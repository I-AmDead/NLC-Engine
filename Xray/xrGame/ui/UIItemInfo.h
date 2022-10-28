#pragma once
#include "uiwindow.h"


class CInventoryItem;
class CUIStatic;
class CUIScrollView;
class CUIProgressBar;
class CUIWpnParams;
class CUIArtefactParams;
class CUI3dStatic;

extern const char * const 		fieldsCaptionColor;

class CUIItemInfo: public CUIWindow
{
private:
	typedef CUIWindow inherited;
	struct _desc_info
	{
		CGameFont*			pDescFont;
		u32					uDescClr;
		bool				bShowDescrText;
	};
	_desc_info				m_desc_info;
	CInventoryItem* m_pInvItem;
public:
						CUIItemInfo			();
	virtual				~CUIItemInfo		();

	void				Init				(float x, float y, float width, float height, LPCSTR xml_name);
	void				Init				(LPCSTR xml_name);
	void				InitItem			(CInventoryItem* pInvItem);
	void				TryAddWpnInfo		(const shared_str& wpn_section);
	void				TryAddArtefactInfo	(const shared_str& af_section);
	virtual bool 		OnDbClick			();
	virtual bool		OnMouseDown			(int mouse_btn);
	virtual void		Update				();
	virtual void		Draw				();
	bool				m_b_force_drawing;
	CUIStatic*			UIName;
	CUIStatic*			UIWeight;
	CUIStatic*			UICost;
	CUIStatic*			UICondition;
	CUIScrollView*		UIDesc;
	CUIProgressBar*		UICondProgresBar;
	CUIWpnParams*		UIWpnParams;
	CUIArtefactParams*	UIArtefactParams;

	//Fvector2			UIItemImageSize;
	//CUIStatic*			UIItemImage;

	Fvector2			UIItem3dPos;
	CUI3dStatic*		UIItem3d;
private:
	float				f_3d_static_time;
	float				f_3d_static_time_delta;
	bool				b_3d_static_mouse_down;
	bool				b_3d_static_db_click;
};