#pragma once

#include "inventory_item_object.h"
//#include "night_vision_effector.h"
#include "hudsound.h"
#include "script_export_space.h"

class CLAItem;
class CMonsterEffector;

class CTorch : public CInventoryItemObject {
private:
    typedef	CInventoryItemObject	inherited;

protected:
	float			fBrightness;
	CLAItem*		lanim;
	float			time2hide;

	u16				guid_bone;
	shared_str		light_trace_bone;

	float			m_delta_h;
	Fvector2		m_prev_hp;

public:
	bool			m_switched_on;
protected:
	bool			b_lastState;
	ref_light		light_render;
	ref_light		light_omni;
	ref_glow		glow_render;
	Fvector			m_focus;
private:
	inline	bool	can_use_dynamic_lights	();

public:
					CTorch				(void);
	virtual			~CTorch				(void);

	virtual void	Load				(LPCSTR section);
	virtual BOOL	net_Spawn			(CSE_Abstract* DC);
	virtual void	net_Destroy			();
	virtual void	net_Export			(NET_Packet& P);				// export to server
	virtual void	net_Import			(NET_Packet& P);				// import from server

	virtual void	OnH_A_Chield		();
	virtual void	OnH_B_Independent	(bool just_before_destroy);

	virtual void	UpdateCL			();

			void	Switch				();
			void	Switch				(bool light_on);

	virtual bool	can_be_attached		() const;
 
protected:
	HUD_SOUND_COLLECTION	m_sounds;

	enum EStats{
		eTorchActive				= (1<<0),
		eNightVisionActive			= (1<<1),
		eAttached					= (1<<2)
	};

public:

	virtual bool			use_parent_ai_locations	() const
	{
		return				(!H_Parent());
	}
	virtual void	create_physic_shell		();
	virtual void	activate_physic_shell	();
	virtual void	setup_physic_shell		();

	virtual void	afterDetach				();
	virtual void	renderable_Render		();

	// alpet: ���������� ������ ������
	IRender_Light  *GetLight(int target = 0);

	void			SetAnimation(LPCSTR name);
	void			SetBrightness (float brightness);
	void			SetColor(const Fcolor &color, int target = 0);
	void			SetRGB(float r, float g, float b, int target = 0);
	void			SetAngle(float angle, int target = 0);
	void			SetRange(float range, int target = 0);
	void			SetTexture(LPCSTR texture, int target = 0);
	void			SetVirtualSize(float size, int target = 0);


	DECLARE_SCRIPT_REGISTER_FUNCTION
};

CTorch *get_torch(CScriptGameObject *script_obj); // alpet: ��� �������� � ������ CScriptGameObject

add_to_type_list(CTorch)
#undef script_type_list
#define script_type_list save_type_list(CTorch)
