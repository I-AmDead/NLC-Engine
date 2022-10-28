#pragma once


#include "ui/UIStatic.h"

class CActor;
class CUICustomMap;
//////////////////////////////////////////////////////////////////////////


class CUIZoneMap
{
//public:
	bool						visible;

//private:
	CUICustomMap*				m_activeMap;
	float						m_fScale;

	CUIStatic					m_background;
	CUIStatic					m_center;
	CUIStatic					m_compass;
	CUIStatic					m_clipFrame;
	CUIStatic					m_pointerDistanceText;
	u8							m_current_map_idx;

public:
								CUIZoneMap		();
	virtual						~CUIZoneMap		();

	void						SetHeading		(float angle);
	void						Init			();

	void						Render			();
	void						UpdateRadar		(Fvector pos);

	void						SetScale		(float s)							{m_fScale = s;}
	float						GetScale		()									{return m_fScale;}

	bool						ZoomIn			();
	bool						ZoomOut			();


	CUIStatic&					Background		()									{return m_background;};
	CUIStatic&					ClipFrame		()									{return m_clipFrame; }; // alpet: для экспорта в скрипты
	CUIStatic&					Compass			()									{return m_compass; }; // alpet: для экспорта в скрипты

	void						SetupCurrentMap	();
	void						OnSectorChanged	(int sector);
};

