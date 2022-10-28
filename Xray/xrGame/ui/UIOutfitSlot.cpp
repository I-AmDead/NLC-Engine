#include "stdafx.h"
#include "UIOutfitSlot.h"
#include "UI3dStatic.h"
#include "UICellItem.h"
#include "../CustomOutfit.h"
#include "../actor.h"
#include "UIInventoryUtilities.h"

CUIOutfitDragDropList::CUIOutfitDragDropList()
{
	m_background				= xr_new<CUI3dStatic>();
	m_background->SetAutoDelete	(true);
	AttachChild					(m_background);
	//m_default_outfit			= "npc_icon_without_outfit";
}

CUIOutfitDragDropList::~CUIOutfitDragDropList()
{
}

#include "../level.h"
#include "../game_base_space.h"

void CUIOutfitDragDropList::SetOutfit(CUICellItem* itm)
{
	/*
	static Fvector2 fNoOutfit			= pSettings->r_fvector2(m_default_outfit, "full_scale_icon");
	Frect								r;
	r.x1								= fNoOutfit.x*ICON_GRID_WIDTH;
	r.y1								= fNoOutfit.y*ICON_GRID_HEIGHT;
	r.x2								= r.x1+CHAR_ICON_FULL_WIDTH*ICON_GRID_WIDTH;
	r.y2								= r.y1+CHAR_ICON_FULL_HEIGHT*ICON_GRID_HEIGHT;
	*/
	float w = GetWidth();
	float h = GetHeight();
	if (w > h)
	{
		m_background->Init(-w/3.f, 0, w, w);
	}
	else
	{
		m_background->Init(-h/3.f, 0, h, h);
	}
	//m_background->SetStretchTexture		(true);


/*	if ((GameID() != GAME_SINGLE) && !itm)
	{
		CObject *pActor = NULL;

        pActor = smart_cast<CActor*>(Level().CurrentEntity());

		xr_string a;
		if (pActor)
			a = *pActor->cNameVisual();
		else
			a = *m_default_outfit;

		xr_string::iterator it = std::find(a.rbegin(), a.rend(), '\\').base(); 

		// Cut leading full path
		if (it != a.begin())
			a.erase(a.begin(), it);
		// Cut trailing ".ogf"
		R_ASSERT(xr_strlen(a.c_str()) > 4);
		if ('.' == a[a.size() - 4])
			a.erase(a.size() - 4);

		m_background->InitTexture(a.c_str());
	}
	else*/ {
/*		if(itm)
		{
			PIItem _iitem	= (PIItem)itm->m_pData;
			CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(_iitem); VERIFY(pOutfit);

			m_background->InitTexture			(pOutfit->GetFullIconName().c_str());
		}else
		{
			m_background->InitTexture		("npc_icon_without_outfit");
		}*/
		/*
		r.x2			= r.x1+CHAR_ICON_FULL_WIDTH*ICON_GRID_WIDTH;
		r.y2			= r.y1+CHAR_ICON_FULL_HEIGHT*ICON_GRID_HEIGHT;

		m_background->SetShader				(InventoryUtilities::GetCharIconsShader());
        m_background->SetOriginalRect		(r);
		*/

		CObject *pActor = smart_cast<CObject*>(Level().CurrentEntity());
		if (!pActor) return;

/*		IRender_Visual* V = ::Render->model_Create(pActor->cNameVisual().c_str());
		CKinematicsAnimated* ka = V->dcast_PKinematicsAnimated();
		if (ka)
		{
			MotionID M2 = ka->ID_Cycle_Safe("idle");

			R_ASSERT3(M2.valid(), "model has no motion [idle] ", pActor->cNameVisual().c_str());

			u16 root_id = ka->LL_GetBoneRoot();
			CBoneInstance& root_binst = ka->LL_GetBoneInstance(root_id);
			root_binst.set_callback_overwrite(TRUE);
			root_binst.mTransform.identity();

			u16 pc = ka->partitions().count();
			for (u16 pid = 0; pid < pc; ++pid)
			{
				CBlend* B = ka->PlayCycle(pid, M2, false);
				R_ASSERT(B);
			}

			ka->CalculateBones_Invalidate();
		}*/

		m_background->SetGameObject(pActor->Visual(), true);

		m_background->SetRotateY(PI-PI_DIV_6);
		m_background->SetScale(1.35f);
		//if (V) ::Render->model_Delete(V);
	}

	//m_background->TextureAvailable		(true);
	//m_background->TextureOn				();
//	m_background->RescaleRelative2Rect	(r);
}

void CUIOutfitDragDropList::SetDefaultOutfit(LPCSTR default_outfit){
	//m_default_outfit = default_outfit;
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm)
{
	if(itm)	inherited::SetItem			(itm);
	SetOutfit							(itm);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Fvector2 abs_pos)
{
	if(itm)	inherited::SetItem			(itm, abs_pos);
	SetOutfit							(itm);
}

void CUIOutfitDragDropList::SetItem(CUICellItem* itm, Ivector2 cell_pos)
{
	if(itm)	inherited::SetItem			(itm, cell_pos);
	SetOutfit							(itm);
}

CUICellItem* CUIOutfitDragDropList::RemoveItem(CUICellItem* itm, bool force_root)
{
	VERIFY								(!force_root);
	CUICellItem* ci						= inherited::RemoveItem(itm, force_root);
	SetOutfit							(NULL);
	return								ci;
}


void CUIOutfitDragDropList::Draw()
{
	m_background->Draw					();
//.	inherited::Draw						();
}