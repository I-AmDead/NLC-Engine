#pragma once
#include "inventory_item_object.h"
#include <ComplexVar.h>

class CCartridge 
{
public:
	CCartridge();
	void Load(LPCSTR section, u8 LocalAmmoType);

	shared_str	m_ammoSect;
	enum{
		cfTracer				= (1<<0),
		cfRicochet				= (1<<1),
		cfCanBeUnlimited		= (1<<2),
		cfExplosive				= (1<<3),
	};
	float	m_kDist, m_kDisp, m_kHit, m_kImpulse, m_kPierce, m_kAP, m_kAirRes;
	int		m_buckShot;
	float	m_impair;
	float	fWallmarkSize;
	
	u8		m_u8ColorID;
	u8		m_LocalAmmoType;

	
	u16		bullet_material_idx;
	Flags8	m_flags;

	shared_str	m_InvShortName;
};

class CWeaponAmmo :	public CInventoryItemObject {
	typedef CInventoryItemObject		inherited;
protected:
	CComplexVarInt					*m_pBoxCurr;
public:
									CWeaponAmmo			(void);
	virtual							~CWeaponAmmo		(void);

	virtual CWeaponAmmo				*cast_weapon_ammo	()	{return this;}
	virtual void					Load				(LPCSTR section);
	virtual BOOL					net_Spawn			(CSE_Abstract* DC);
	virtual void					net_Destroy			();
	virtual void					net_Export			(NET_Packet& P);
	virtual void					net_Import			(NET_Packet& P);
	virtual void					OnH_B_Chield		();
	virtual void					OnH_B_Independent	(bool just_before_destroy);
	virtual void					UpdateCL			();
	virtual void					renderable_Render	();

	virtual bool					Useful				() const;
	virtual float					Weight				() const;

	virtual u32						Cost				() const;
	bool							Get					(CCartridge &cartridge);
	u16								get_box_curr		() const;
	void							set_box_curr		(u16 value);

	float		m_kDist, m_kDisp, m_kHit, m_kImpulse, m_kPierce, m_kAP, m_kAirRes;
	int			m_buckShot;
	float		m_impair;
	float		fWallmarkSize;
	u8			m_u8ColorID;

	u16				m_boxSize;
	
	bool			m_tracer;
	

public:
	virtual CInventoryItem *can_make_killing	(const CInventory *inventory) const;
};
