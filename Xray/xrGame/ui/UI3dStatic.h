// UI3dStatic.h: класс статического элемента, который рендерит 
// 3d объект в себя
//////////////////////////////////////////////////////////////////////

#ifndef _UI_3D_STATIC_H_
#define _UI_3D_STATIC_H_

#pragma once


#include "uiwindow.h"

class CGameObject;

class CUI3dStatic : public CUIWindow  
{
public:
	CUI3dStatic();
	virtual ~ CUI3dStatic();
	
	void	SetRotate			(float x, float y, float z)		{ m_x_angle = x; /*m_y_angle = y;*/ m_z_angle = z; }
	void	SetRotateY			(float y)		{ m_y_angle = y; }
	void	SetScale			(float s)		{ m_scale = s; }
	float	GetScale			()				{ return m_scale; }
	void	SetGameObject		(IRender_Visual* V, bool is_actor);
	
	//прорисовка окна
	virtual void Draw		();



protected:
	float		m_x_angle, m_y_angle, m_z_angle;
	float		m_scale;
	Fsphere		m_sphere;

	//перевод из координат экрана в координаты той плоскости
	//где находиться объект
	void FromScreenToItem(float x_screen, float y_screen, float& x_item, float& y_item);

	IRender_Visual* m_pCurrentVisual;
	
private:
	float		GetScaleX() { return float(::Render->getTarget()->get_width()) / float(UI_BASE_WIDTH); }
	float		GetScaleY() { return float(::Render->getTarget()->get_height()) / float(UI_BASE_HEIGHT); }
	bool		bIsActor;
};

#endif // _UI_3D_STATIC_H_
