// UI3dStatic.cpp: класс статического элемента, который рендерит 
// 3d объект в себя
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ui3dstatic.h"
#include "../gameobject.h"
#include "../HUDManager.h"
#include "../../xr_3da/fbasicvisual.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CUI3dStatic:: CUI3dStatic()
{
	m_x_angle = m_y_angle = m_z_angle = 0;
	m_scale	= 1.f;
	Enable(false);
	bIsActor = false;
}

 CUI3dStatic::~ CUI3dStatic()
{

}


//расстояние от камеры до вещи, перед глазами
#define DIST  (VIEWPORT_NEAR + 0.2f)


void CUI3dStatic::FromScreenToItem(float x_screen, float y_screen,
								   float& x_item, float& y_item)
{
		float x = x_screen;
		float y = y_screen;


		float halfwidth  = (float)Device.dwWidth/2.f;
		float halfheight = (float)Device.dwHeight/2.f;

	    float size_y = VIEWPORT_NEAR * tanf( deg2rad( Device.fFOV*0.35f ) * 0.5f);
        float size_x = size_y / (Device.fASPECT);

        float r_pt      = float(x-halfwidth) * size_x / (float) halfwidth;
        float u_pt      = float(halfheight-y) * size_y / (float) halfheight;

		x_item  = r_pt * DIST / VIEWPORT_NEAR;
		y_item = u_pt * DIST / VIEWPORT_NEAR;
}


//прорисовка
void  CUI3dStatic::Draw()
{
	if(m_pCurrentVisual)
	{
		if ( !bIsActor ) m_sphere = m_pCurrentVisual->vis.sphere;

		Frect rect;
		GetAbsoluteRect(rect);
		// Apply scale
		rect.top	= rect.top * GetScaleY();
		rect.left	= rect.left * GetScaleX();
		rect.bottom	= rect.bottom * GetScaleY();
		rect.right	= rect.right * GetScaleX();

		Fmatrix translate_matrix;
		Fmatrix scale_matrix;
		
		Fmatrix rx_m; 
		Fmatrix ry_m; 
		Fmatrix rz_m; 

		Fmatrix matrix;
		matrix.identity();


		//поместить объект в центр сферы
		translate_matrix.identity();
		translate_matrix.translate( - m_sphere.P.x, 
			 					    - m_sphere.P.y, 
								    - m_sphere.P.z);

		matrix.mulA_44(translate_matrix);


		rx_m.identity();
		rx_m.rotateX(m_x_angle);
		ry_m.identity();
		ry_m.rotateY(m_y_angle);
		rz_m.identity();
		rz_m.rotateZ(m_z_angle);


		matrix.mulA_44(rx_m);
		matrix.mulA_44(ry_m);
		matrix.mulA_44(rz_m);
		

		
		float x1, y1, x2, y2;

		FromScreenToItem(rect.left, rect.top, x1, y1);
		FromScreenToItem(rect.right, rect.bottom, x2, y2);

		float normal_size;
		normal_size =_abs(x2-x1)<_abs(y2-y1)?_abs(x2-x1):_abs(y2-y1);
		
				
		float radius = m_sphere.R;

		float scale = normal_size/(radius*2.f) * m_scale;

		scale_matrix.identity();
		scale_matrix.scale(scale, scale, scale);

		matrix.mulA_44(scale_matrix);
        

		float right_item_offset, up_item_offset;

		
		///////////////////////////////	
		
		FromScreenToItem(rect.left + (GetWidth()/2.f * GetScaleX()),
						 rect.top + (GetHeight()/2.f * GetScaleY()), 
						 right_item_offset, up_item_offset);

		translate_matrix.identity();
		translate_matrix.translate(right_item_offset,
								   up_item_offset,
								   DIST);

		matrix.mulA_44(translate_matrix);

		Fmatrix camera_matrix;
		camera_matrix.identity();
		camera_matrix = Device.mView;
		camera_matrix.invert();

		matrix.mulA_44(camera_matrix);

		::Render->set_Object(NULL); 
		::Render->set_Transform(&matrix);
		::Render->add_Visual(m_pCurrentVisual);
		::Render->flush_3d_static();
	}
}

void CUI3dStatic::SetGameObject(IRender_Visual* V, bool is_actor)
{
	m_pCurrentVisual = V;
	m_sphere = m_pCurrentVisual->vis.sphere;
	m_scale = 1.f;
	bIsActor = is_actor;
}