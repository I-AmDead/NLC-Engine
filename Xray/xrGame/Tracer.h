#pragma once

class CBulletManager;

class CTracer
{
	friend CBulletManager;
protected:
	ref_shader			sh_Tracer;
	ref_geom			sh_Geom;
	xr_vector<u32>		m_aColors;
	float				m_circle_size_k;
public:
						CTracer		();
	void				Render		(FVF::LIT*&verts, const Fvector& pos, 
										const Fvector& center, 
										const Fvector& dir, 
										float length, 
										float width, 
										u8 colorID, 
										float speed, 
										bool bActor);
};