#include "stdafx.h"
#include "igame_level.h"
#include "igame_persistent.h"

#include "xrSheduler.h"
#include "xr_object_list.h"
#include "std_classes.h"

#include "xr_object.h"
#include "../xrNetServer/net_utils.h"
#include "../../build_config_defines.h"
#include "luaicp_events.h"
#include "lua_tools.h"
#include "CustomHUD.h"

class fClassEQ {
	CLASS_ID cls;
public:
	fClassEQ(CLASS_ID C) : cls(C) {};
	IC bool operator() (CObject* O) { return cls==O->CLS_ID; }
};


// #pragma optimize("gyts", off)

xr_vector<CObject*>		deleted_objects;

void SafeNetDestroy(CObject *O, LPCSTR context)
{
//	__try
//	{ 
		O->net_Destroy(); 
//	}
//	__except (SIMPLE_FILTER) 
//	{  
//		MsgCB("! #EXCEPTION: net_Destroy object %s, context = <%s> ", O->Name_script(), context); 
//	}	
}


bool chk_already_deleted(CObject *O, str_c context)
{
	xr_vector<CObject*>::iterator i = std::find(deleted_objects.begin(), deleted_objects.end(), O);
	if (i != deleted_objects.end())
	{   // ������ ������ ���������, ��������� ������������� ����� ������. 
		// Msg("!WARN: %-15s touch already deleted object 0x%p, ID = %d, Name = %s ", context, O, O->ID(), O->Name_script());
		return true;
	}
	else
		return false;
}

CObjectList::CObjectList	( )
{
	objects_dup_memsz		= 512;
	objects_dup				= xr_alloc	<CObject*>	(objects_dup_memsz);
	crows					= &crows_0	;	
	m_items					= xr_alloc <CObject*> (65536);
	Memory.mem_fill32	    (objects_dup, 0,  objects_dup_memsz);
	Memory.mem_fill32	    (m_items, 0, 65536);
}

CObjectList::~CObjectList	( )
{
	R_ASSERT				( objects_active.empty()	);
	R_ASSERT				( objects_sleeping.empty()	);
	R_ASSERT				( destroy_queue.empty()		);
#ifndef FAST_OBJECT_MAP
	R_ASSERT				( map_NETID.empty()			);
#endif
	xr_free					( objects_dup);
	xr_free					( m_items );
}

CObject*	CObjectList::FindObjectByName	( shared_str name )
{
	for (xr_vector<CObject*>::iterator I=objects_active.begin(); I!=objects_active.end(); I++)
		if ((*I)->cName().equal(name))	return (*I);
	for (xr_vector<CObject*>::iterator I=objects_sleeping.begin(); I!=objects_sleeping.end(); I++)
		if ((*I)->cName().equal(name))	return (*I);
	return	NULL;
}
CObject*	CObjectList::FindObjectByName	( LPCSTR name )
{
	return	FindObjectByName				(shared_str(name));
}

CObject*	CObjectList::FindObjectByCLS_ID	( CLASS_ID cls )
{
	{
		xr_vector<CObject*>::iterator O	= std::find_if(objects_active.begin(),objects_active.end(),fClassEQ(cls));
		if (O!=objects_active.end())	return *O;
	}
	{
		xr_vector<CObject*>::iterator O	= std::find_if(objects_sleeping.begin(),objects_sleeping.end(),fClassEQ(cls));
		if (O!=objects_sleeping.end())	return *O;
	}

	return	NULL;
}

#pragma optimize("gyt", off)

void	CObjectList::o_remove		( xr_vector<CObject*>&	v,  CObject* O)
{
#ifdef LUAICP_COMPAT
	R_ASSERT(O);
	// MsgCB("$#CONTEXT: cl remove [%d][%p] %s, from v = %p, v.size = %d", O->ID(), O, O->Name_script(), &v, v.size());
	process_object_event(EVT_OBJECT_REMOVE | EVT_OBJECT_CLIENT, O->ID(), O, NULL);
#endif
	xr_vector<CObject*>::iterator _i	= std::find(v.begin(),v.end(),O);
	if (_i != v.end())
	__try
	{
		v.erase(_i);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Msg("!#EXCEPTION: strange, but expected in CObjectList::o_remove");
		MsgCB("#DUMP_CONTEXT");
	}
	else
		Msg("!#ERROR: object %s not registered in object list.", O->Name_script());
//.	Msg("---o_remove[%s][%d]", O->Name_script(), O->ID() );
}

void	CObjectList::o_activate		( CObject*		O		)
{	
	xr_vector<CObject*>::iterator _i	= std::find(objects_active.begin(), objects_active.end(), O);
	if (_i != objects_active.end())
		return; // already activated

	// chk_already_deleted(O, "o_activate");
	VERIFY						(O && O->processing_enabled());
	o_remove					(objects_sleeping,O);
	objects_active.push_back	(O);
	O->MakeMeCrow				();
#ifdef LUAICP_COMPAT
	process_object_event(EVT_OBJECT_ACTIVATE | EVT_OBJECT_CLIENT, O->ID(), O, NULL);
#endif
}
void	CObjectList::o_sleep		( CObject*		O		)
{
	// chk_already_deleted(O, "o_sleep");	

	VERIFY	(O && !O->processing_enabled());
	o_remove					(objects_active,O);
	objects_sleeping.push_back	(O);
	O->MakeMeCrow				();
#ifdef LUAICP_COMPAT
	process_object_event(EVT_OBJECT_SLEEP  | EVT_OBJECT_CLIENT, O->ID(), O, NULL);
#endif
}

void	CObjectList::SingleUpdate	(CObject* O)
{
	if (O->processing_enabled() && (Device.dwFrame != O->dwFrame_UpdateCL))
	{
		R_ASSERT3(O != O->H_Parent(), " object.parent == object!!! ", O->Name_script());

		if (O->H_Parent())		SingleUpdate(O->H_Parent());
		Device.Statistic->UpdateClient_updated	++;
		O->dwFrame_UpdateCL		=				Device.dwFrame;
		O->IAmNotACrowAnyMore	()				;
		O->UpdateCL				()				;
		VERIFY3					(O->dbg_update_cl == Device.dwFrame, "Broken sequence of calls to 'UpdateCL'",*O->cName());
//		if (O->getDestroy())
//		{
//			destroy_queue.push_back(O);
//.			Msg				("- destroy_queue.push_back %s[%d] frame [%d]",O->Name_script(), O->ID(), Device.dwFrame);
//		}
//		else
		if (O->H_Parent() && (O->H_Parent()->getDestroy() || O->H_Root()->getDestroy()) )	
		{
			// Push to destroy-queue if it isn't here already
			Msg	("! ERROR: incorrect destroy sequence for object[%d:%s], section[%s], parent[%d:%s]",O->ID(),*O->cName(),*O->cNameSect(),O->H_Parent()->ID(),*O->H_Parent()->cName());
//			if (std::find(destroy_queue.begin(),destroy_queue.end(),O)==destroy_queue.end())
//				destroy_queue.push_back	(O);
		}
	}
	if (O->getDestroy() && (Device.dwFrame != O->dwFrame_UpdateCL))
	{
//		destroy_queue.push_back(O);
		Msg				("- !!!processing_enabled ->destroy_queue.push_back %s[%d] frame [%d]",O->Name_script(), O->ID(), Device.dwFrame);
	}
}

void clear_crow_vec	(xr_vector<CObject*>& o)
{
	for (u32 _it=0; _it<o.size(); _it++)	o[_it]->IAmNotACrowAnyMore();
	o.clear_not_free();
}

void CObjectList::Update		(bool bForce)
{
	if ( ! (Device.Paused() && !bForce) )
	{
		// Clients
		if (Device.fTimeDelta>EPS_S || bForce)			
		{
			// Select Crow-Mode
			Device.Statistic->UpdateClient_updated	= 0;
			Device.Statistic->UpdateClient_crows	= crows->size	();
			xr_vector<CObject*>*		workload	= 0;
			if (!psDeviceFlags.test(rsDisableObjectsAsCrows))	
			{
				workload = crows			;
				if (crows==&crows_0)		crows=&crows_1;
				else						crows=&crows_0;
				clear_crow_vec				(*crows);
			} else 
			{
				workload	= &objects_active;
				clear_crow_vec				(crows_0);
				clear_crow_vec				(crows_1);
			}

			Device.Statistic->UpdateClient.Begin		();
			Device.Statistic->UpdateClient_active		= objects_active.size	();
			Device.Statistic->UpdateClient_total		= objects_active.size	() + objects_sleeping.size();

			u32 objects_count	= workload->size();
			if (objects_count > objects_dup_memsz)	
			{
				// realloc
				while (objects_count > objects_dup_memsz)	objects_dup_memsz	+= 32;
				objects_dup	= (CObject**)xr_realloc(objects_dup,objects_dup_memsz*sizeof(CObject*));
			}
			CopyMemory	(objects_dup,&*workload->begin(),objects_count*sizeof(CObject*));
			for (u32 O=0; O<objects_count; O++) 
				SingleUpdate	(objects_dup[O]);

			Device.Statistic->UpdateClient.End		();
		}
	}

	// Destroy
	if (!destroy_queue.empty()) 
	{
		// Info
		for (xr_vector<CObject*>::iterator oit=objects_active.begin(); oit!=objects_active.end(); oit++)
			for (int it = destroy_queue.size()-1; it>=0; it--){	
				(*oit)->net_Relcase		(destroy_queue[it]);
			}
		for (xr_vector<CObject*>::iterator oit=objects_sleeping.begin(); oit!=objects_sleeping.end(); oit++)
			for (int it = destroy_queue.size()-1; it>=0; it--)	(*oit)->net_Relcase	(destroy_queue[it]);

		for (int it = destroy_queue.size()-1; it>=0; it--)	Sound->object_relcase	(destroy_queue[it]);
		
		CCustomHUD			&hud = *g_pGameLevel->pHUD;
		RELCASE_CALLBACK_VEC::iterator It	= m_relcase_callbacks.begin();
		RELCASE_CALLBACK_VEC::iterator Ite	= m_relcase_callbacks.end();
		for(;It!=Ite; ++It)	{
			VERIFY			(*(*It).m_ID==(It-m_relcase_callbacks.begin()));
			xr_vector<CObject*>::iterator dIt	= destroy_queue.begin();
			xr_vector<CObject*>::iterator dIte	= destroy_queue.end();
			for (;dIt!=dIte; ++dIt) {
				(*It).m_Callback(*dIt);
				hud.net_Relcase	(*dIt);
			}
		}

		// Destroy
		for (int it = destroy_queue.size()-1; it>=0; it--)
		{
			CObject*		O	= destroy_queue[it];
//			Msg				("Object [%x]", O);
#ifdef DEBUG
			Msg				("Destroying object[%x] [%d][%s] frame[%d]",O, O->ID(),*O->cName(), Device.dwFrame);
#endif // DEBUG
			SafeNetDestroy(O, "Update");			
			Destroy			(O);
		}
		destroy_queue.clear	();
	}
}

void CObjectList::net_Register		(CObject* O)
{
	// chk_already_deleted(O, "net_Register");		
	R_ASSERT		(O);
#ifdef FAST_OBJECT_MAP
	if (0 == O->ID())
		__asm nop;
	m_items[O->ID()] = O;
#ifdef LUAICP_COMPAT
	process_object_event(EVT_OBJECT_SPAWN | EVT_OBJECT_CLIENT, O->ID(), O, NULL);
#endif
#else
	map_NETID.insert(mk_pair(O->ID(),O));
#endif
	//Msg			("-------------------------------- Register: %s",O->cName());
}

void CObjectList::net_Unregister	(CObject* O)
{
#ifdef FAST_OBJECT_MAP
	m_items[O->ID()] = NULL;
#ifdef LUAICP_COMPAT
	process_object_event(EVT_OBJECT_REMOVE  | EVT_OBJECT_CLIENT, O->ID(), NULL, NULL);
#endif
#else
	xr_map<u32,CObject*>::iterator	it = map_NETID.find(O->ID());
	if ((it!=map_NETID.end()) && (it->second == O))	{
		// Msg			("-------------------------------- Unregster: %s",O->cName());
		map_NETID.erase(it);
	}
#endif
}

int	g_Dump_Export_Obj = 0;

u32	CObjectList::net_Export			(NET_Packet* _Packet,	u32 start, u32 max_object_size	)
{
	if (g_Dump_Export_Obj) Msg("---- net_export --- ");

	NET_Packet& Packet	= *_Packet;
	u32			position;
	for (; start<objects_active.size() + objects_sleeping.size(); start++)			{
		CObject* P = (start<objects_active.size()) ? objects_active[start] : objects_sleeping[start-objects_active.size()];
		if (P->net_Relevant() && !P->getDestroy())	{			
			Packet.w_u16			(u16(P->ID())	);
			Packet.w_chunk_open8	(position);
			//Msg						("cl_export: %d '%s'",P->ID(),*P->cName());
			P->net_Export			(Packet);

#ifdef DEBUG
			u32 size				= u32		(Packet.w_tell()-position)-sizeof(u8);
			if				(size>=256)			{
				Debug.fatal	(DEBUG_INFO,"Object [%s][%d] exceed network-data limit\n size=%d, Pend=%d, Pstart=%d",
					*P->cName(), P->ID(), size, Packet.w_tell(), position);
			}
#endif
			if (g_Dump_Export_Obj)
			{
				u32 size				= u32		(Packet.w_tell()-position)-sizeof(u8);
				Msg("* %s : %d", *(P->cNameSect()), size);
			}
			Packet.w_chunk_close8	(position);
//			if (0==(--count))		
//				break;
			if (max_object_size > (NET_PacketSizeLimit - Packet.w_tell()))
				break;
		}
	}
	if (g_Dump_Export_Obj) Msg("------------------- ");
	return	start+1;
}

int	g_Dump_Import_Obj = 0;

void CObjectList::net_Import		(NET_Packet* Packet)
{
	if (g_Dump_Import_Obj) Msg("---- net_import --- ");

	while (!Packet->r_eof())
	{
		u16 ID;		Packet->r_u16	(ID);
		u8  size;	Packet->r_u8	(size);
		CObject* P  = net_Find		(u32(ID));
		if (P)		
		{

			u32 rsize = Packet->r_tell();			
			
			P->net_Import	(*Packet);

			if (g_Dump_Import_Obj) Msg("* %s : %d - %d", *(P->cNameSect()), size, Packet->r_tell() - rsize);

		}
		else		Packet->r_advance(size);
	}

	if (g_Dump_Import_Obj) Msg("------------------- ");
}

CObject* CObjectList::net_Find			(u32 ID)
{
#ifdef FAST_OBJECT_MAP
	R_ASSERT		(m_items);
	return m_items [(u16)ID];
#else
	xr_map<u32,CObject*>::iterator	it = map_NETID.find(ID);
	return (it==map_NETID.end())?0:it->second;
#endif
}

void CObjectList::Load		()
{
	
#ifndef FAST_OBJECT_MAP
	R_ASSERT				(map_NETID.empty());
#endif
	R_ASSERT				(m_items && objects_active.empty() && destroy_queue.empty() && objects_sleeping.empty());

#ifdef LUAICP_COMPAT
#ifdef FAST_OBJECT_MAP
	LogXrayOffset("GameLevel.m_items",			g_pGameLevel, &this->m_items);
	LogXrayOffset("GameLevel.map_NETID",		g_pGameLevel, g_pGameLevel);
#else
	LogXrayOffset("GameLevel.m_items",			g_pGameLevel, g_pGameLevel);
	LogXrayOffset("GameLevel.map_NETID",		g_pGameLevel, &this->map_NETID);	
#endif
	// ����� ���������� g_pGameLevel ��� �� ������ ���� NULL
	LogXrayOffset("GameLevel.ObjectList",		g_pGameLevel, this);
	LogXrayOffset("GameLevel.destroy_queue",	g_pGameLevel, &this->destroy_queue);
	LogXrayOffset("GameLevel.objects_active",	g_pGameLevel, &this->objects_active);
	LogXrayOffset("GameLevel.objects_sleeping", g_pGameLevel, &this->objects_sleeping);
	LogXrayOffset("GameLevel.crows_0",			g_pGameLevel, &this->crows_0);
	LogXrayOffset("GameLevel.crows_1",			g_pGameLevel, &this->crows_1);
	LogXrayOffset("GameLevel.crows",			g_pGameLevel, &this->crows);

	LogXrayOffset("xr_vector.first",			&this->objects_active, &objects_active._Myfirst);
	LogXrayOffset("xr_vector.last",				&this->objects_active, &objects_active._Mylast);
#endif 
}

void CObjectList::Unload	( )
{
	if (objects_sleeping.size() || objects_active.size())
		Msg			("! objects-leaked: %d",objects_sleeping.size() + objects_active.size());

	// Destroy objects
	while (objects_sleeping.size())
	{
		CObject*	O	= objects_sleeping.back	();
		Msg				("! [%x] s[%4d]-[%s]-[%s]", O, O->ID(), *O->cNameSect(), *O->cName());
		O->setDestroy	( TRUE );
		
#ifdef DEBUG
		Msg				("Destroying object [%d][%s]",O->ID(),*O->cName());
#endif
		SafeNetDestroy(O, "Unload sleeping");
		Destroy			( O );
	}
	while (objects_active.size())
	{
		CObject*	O	= objects_active.back	();
		Msg				("! [%x] a[%4d]-[%s]-[%s]", O, O->ID(), *O->cNameSect(), *O->cName());
		O->setDestroy	( TRUE );

#ifdef DEBUG
		Msg				("Destroying object [%d][%s]",O->ID(),*O->cName());
#endif
		SafeNetDestroy  (O, "Unload active");
		Destroy			( O );
	}
}

CObject*	CObjectList::Create				( LPCSTR	name	)
{
	CObject*	O				= g_pGamePersistent->ObjectPool.create(name);
//	Msg("CObjectList::Create [%x]%s", O, name);
	process_object_event(EVT_OBJECT_ADD | EVT_OBJECT_CLIENT, O->ID(), O, NULL);	 
	objects_sleeping.push_back	(O);
	return						O;
}

int  find_remove_object(xr_vector<CObject*> &from, CObject *O)
{
	xr_vector<CObject*>::iterator _i		= std::find(from.begin(),from.end(), O);
	if (_i != from.end())
	{
		from.erase(_i);
		return 1;
	}
	else
		return 0;

}


void		CObjectList::Destroy			( CObject*	O		)
{
	if (0==O)								return;
//	__try
//	{
		R_ASSERT(!IsBadReadPtr(O, 64));
		net_Unregister(O);
		// crows	
		find_remove_object(crows_0, O);
		find_remove_object(crows_1, O);
		// active/inactive
		int remove_set = 0;
		remove_set += find_remove_object(objects_active, O)   * 0x1001;
		remove_set += find_remove_object(objects_sleeping, O) * 0x1002;

		/*
		xr_vector<CObject*>::iterator _i1		= std::find(crows_1.begin(),crows_1.end(),O);
		if	(_i1!=crows_1.end())				crows_1.erase	(_i1);


		xr_vector<CObject*>::iterator _i		= std::find(objects_active.begin(),objects_active.end(),O);
		if	(_i!=objects_active.end())			objects_active.erase	(_i);
		else {
		xr_vector<CObject*>::iterator _ii	= std::find(objects_sleeping.begin(),objects_sleeping.end(),O);
		if	(_ii!=objects_sleeping.end())	objects_sleeping.erase	(_ii);
		else	FATAL						("! Unregistered object being destroyed");
		}
		*/

		if (0 == remove_set)
			FATAL("! Unregistered object being destroyed");
		if (remove_set > 0x2000)
			Msg("! #ERROR: Destroying object '%s' remove_set = 0x%x ", O->Name_script(), remove_set);

		process_object_event(EVT_OBJECT_DESTROY | EVT_OBJECT_CLIENT, O->ID(), O, NULL, 1);
		deleted_objects.push_back(O);
		g_pGamePersistent->ObjectPool.destroy(O);
//	}
//	__except (SIMPLE_FILTER)
//	{
//		string4096 info;
//		GetObjectInfo(O, info, NULL);
//		Msg("! #EXCEPTION: in CObjectList::Destroy, object = %s", info);
//	}

}

void CObjectList::relcase_register		(RELCASE_CALLBACK cb, int *ID)
{
#ifdef DEBUG
	RELCASE_CALLBACK_VEC::iterator It = std::find(	m_relcase_callbacks.begin(),
													m_relcase_callbacks.end(),
													cb);
	VERIFY(It==m_relcase_callbacks.end());
#endif
	*ID								= m_relcase_callbacks.size();
	m_relcase_callbacks.push_back	(SRelcasePair(ID,cb));
}

void CObjectList::relcase_unregister	(int* ID)
{
	VERIFY							(m_relcase_callbacks[*ID].m_ID==ID);
	m_relcase_callbacks[*ID]		= m_relcase_callbacks.back();
	*m_relcase_callbacks.back().m_ID= *ID;
	m_relcase_callbacks.pop_back	();
}

void dump_list(xr_vector<CObject*>& v, LPCSTR reason)
{
	xr_vector<CObject*>::iterator it = v.begin();
	xr_vector<CObject*>::iterator it_e = v.end();
	Msg("----------------dump_list [%s]",reason);
	for(;it!=it_e;++it)
		Msg("%x - name [%s] ID[%d] parent[%s] getDestroy()=[%s]", 
			(*it),
			(*it)->Name_script(), 
			(*it)->ID(), 
			((*it)->H_Parent())?(*it)->H_Parent()->Name_script():"", 
			((*it)->getDestroy())?"yes":"no" );
}

bool CObjectList::dump_all_objects()
{
	dump_list(destroy_queue,"destroy_queue");
	dump_list(objects_active,"objects_active");
	dump_list(objects_sleeping,"objects_sleeping");

	dump_list(crows_0,"crows_0");
	dump_list(crows_1,"crows_1");
	return false;
}

void CObjectList::register_object_to_destroy(CObject *object_to_destroy)
{
	VERIFY					(!registered_object_to_destroy(object_to_destroy));
	destroy_queue.push_back	(object_to_destroy);
	if (IsDebuggerPresent() && object_to_destroy->cNameSect() == "zone_ice")  // 
	{
		LPCSTR tb = GetLuaTraceback();
		Msg("##DBG: planed to destroy %s from %s", object_to_destroy->Name_script(), tb);
	}


	xr_vector<CObject*>::iterator it	= objects_active.begin();
	xr_vector<CObject*>::iterator it_e	= objects_active.end();
	for(;it!=it_e;++it)
	{
		CObject* O = *it;
		if(!O->getDestroy() && O->H_Parent()==object_to_destroy)
		{
			Msg("setDestroy called, but not-destroyed child found parent[%d] child[%d]",object_to_destroy->ID(), O->ID(), Device.dwFrame);
			O->setDestroy(TRUE);
		}
	}

	it		= objects_sleeping.begin();
	it_e	= objects_sleeping.end();
	for(;it!=it_e;++it)
	{
		CObject* O = *it;
		if(!O->getDestroy() && O->H_Parent()==object_to_destroy)
		{
			Msg("setDestroy called, but not-destroyed child found parent[%d] child[%d]",object_to_destroy->ID(), O->ID(), Device.dwFrame);
			O->setDestroy(TRUE);
		}
	}
}

#ifdef DEBUG
bool CObjectList::registered_object_to_destroy	(const CObject *object_to_destroy) const
{
	return					(
		std::find(
			destroy_queue.begin(),
			destroy_queue.end(),
			object_to_destroy
		) != 
		destroy_queue.end()
	);
}
#endif // DEBUG