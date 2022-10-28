#include "stdafx.h"
#include "../xr_3da/customhud.h"

static u32 m_3dstatic_frame = 0;
static const float m_taa_angle[4] = {0.5f,-0.5f,1.f,0.f};
void CRenderTarget::phase_ui()
{
	RImplementation.b_render_3d_static = false;
	u_setrt(Device.dwWidth, Device.dwHeight, HW.pBaseRT, NULL, NULL, NULL);

	RImplementation.marker++;
	RImplementation.r_pmask(true, true);
	RImplementation.phase = RImplementation.PHASE_NORMAL;

	g_hud->RenderUI();

	if (!RImplementation.b_render_3d_static) return;

	u32		w = Device.dwWidth;
	u32		h = Device.dwHeight;

	// Render 3d ststic
	if (!RImplementation.o.dx10_msaa)
		HW.pContext->ClearDepthStencilView(HW.pBaseZB, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	else
		HW.pContext->ClearDepthStencilView(rt_MSAADepth->pZRT, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	phase_scene_begin();

	// Change projection
	Fmatrix Pold = Device.mProject;
	Fmatrix FTold = Device.mFullTransform;
	Device.mProject.build_projection(
		deg2rad(Device.fFOV*0.35f),
		Device.fASPECT, VIEWPORT_NEAR,
		2.f/*g_pGamePersistent->Environment().CurrentEnv.far_plane*/);

	if (m_3dstatic_frame > 3) m_3dstatic_frame = 0;
	float taa_mult = m_taa_angle[m_3dstatic_frame];
	Device.mProject._31 += taa_mult / w;
	Device.mProject._32 += taa_mult / h;

	Device.mFullTransform.mul(Device.mProject, Device.mView);
	RCache.set_xform_project(Device.mProject);

	// Rendering
	//rmNear();
	RImplementation.r_dsgraph_render_graph(0);

	ref_rt rts[4] = { rt_UI3dStatic_prev0, rt_UI3dStatic_prev1, rt_UI3dStatic_prev2, rt_UI3dStatic_prev3 };

	FLOAT ColorRGBA[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (!RImplementation.o.dx10_msaa)
	{
		HW.pContext->ClearRenderTargetView(rts[m_3dstatic_frame]->pRT, ColorRGBA);
		u_setrt(rts[m_3dstatic_frame], 0, 0, HW.pBaseZB);
	}
	else
	{
		HW.pContext->ClearRenderTargetView(rt_Generic_0_r->pRT, ColorRGBA);
		u_setrt(rt_Generic_0_r, 0, 0, rt_MSAADepth->pZRT);
	}

	{
		u32		Offset = 0;

		// Fill VB
		float	scale_X				= float(w)	/ float(TEX_jitter);
		float	scale_Y				= float(h)  / float(TEX_jitter);

		// Fill vertex buffer
		FVF::TL* pv					= (FVF::TL*)	RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						( -1,  1, 0, 1, 0,		0,	scale_Y	);	pv++;
		pv->set						( -1, -1, 0, 0, 0,		0,		  0	);	pv++;
		pv->set						(  1,  1, 1, 1, 0, scale_X,	scale_Y	);	pv++;
		pv->set						(  1, -1, 1, 0, 0, scale_X,		  0	);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);

		// Draw
		RCache.set_Element			(s_clear_hud->E[2]	);
		RCache.set_Geometry			(g_combine		);

		Fvector		L_clr;	float L_spec;
		L_clr.set(0.8f, 0.8f, 0.8f);
		L_spec = u_diffuse2s(L_clr);

		CEnvDescriptorMixer& envdesc = g_pGamePersistent->Environment().CurrentEnv;
		const float minamb			= 0.001f;
		Fvector4	ambclr			= { _max(envdesc.ambient.x*2,minamb),	_max(envdesc.ambient.y*2,minamb),			_max(envdesc.ambient.z*2,minamb),	0	};
					ambclr.mul		(ps_r2_sun_lumscale_amb);
		Fvector4	envclr;
		if (!ps_r2_ls_flags.test(R2FLAG_HEMI_COP))	envclr			= { envdesc.sky_color.x*2+EPS,	envdesc.sky_color.y*2+EPS,	envdesc.sky_color.z*2+EPS,	envdesc.weight					};
		else										envclr			= { envdesc.hemi_color.x*2+EPS,	envdesc.hemi_color.y*2+EPS,	envdesc.hemi_color.z*2+EPS,	envdesc.weight					};
		envclr.x		*= 2*ps_r2_sun_lumscale_hemi; 
		envclr.y		*= 2*ps_r2_sun_lumscale_hemi; 
		envclr.z		*= 2*ps_r2_sun_lumscale_hemi;

		RCache.set_c				("L_ambient",		ambclr	);
		RCache.set_c				("env_color",		envclr	);
		RCache.set_c				("m_3d_static_light_dir", ps_r1_3d_static_light_dir.x, ps_r1_3d_static_light_dir.y, ps_r1_3d_static_light_dir.z, L_spec);

		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

	RCache.set_CullMode(CULL_CCW);
	RCache.set_Stencil(FALSE);
	RCache.set_ColorWriteEnable();

	RImplementation.r_dsgraph_render_graph(1);
	RImplementation.r_dsgraph_render_sorted();

	// Restore projection
	Device.mProject = Pold;
	Device.mFullTransform = FTold;
	RCache.set_xform_project(Device.mProject);

	if (RImplementation.o.dx10_msaa)
		HW.pContext->ResolveSubresource(rts[m_3dstatic_frame]->pTexture->surface_get(), 0, rt_Generic_0_r->pTexture->surface_get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	// taa
	u_setrt(rt_UI3dStatic, 0, 0, 0);

	{
		u32		Offset = 0;
		u32		w = Device.dwWidth;
		u32		h = Device.dwHeight;

		// Fill VB
		float	scale_X				= float(w)	/ float(TEX_jitter);
		float	scale_Y				= float(h)  / float(TEX_jitter);

		// Fill vertex buffer
		FVF::TL* pv					= (FVF::TL*)	RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						( -1,  1, 0, 1, 0,		0,	scale_Y	);	pv++;
		pv->set						( -1, -1, 0, 0, 0,		0,		  0	);	pv++;
		pv->set						(  1,  1, 1, 1, 0, scale_X,	scale_Y	);	pv++;
		pv->set						(  1, -1, 1, 0, 0, scale_X,		  0	);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);

		// Draw
		RCache.set_Element			(s_clear_hud->E[3]	);
		RCache.set_Geometry			(g_combine		);

		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

	// post
	u_setrt(Device.dwWidth, Device.dwHeight, HW.pBaseRT, NULL, NULL, NULL);

	{
		u32		Offset = 0;
		u32		w = Device.dwWidth;
		u32		h = Device.dwHeight;

		// Fill VB
		float	scale_X				= float(w)	/ float(TEX_jitter);
		float	scale_Y				= float(h)  / float(TEX_jitter);

		// Fill vertex buffer
		FVF::TL* pv					= (FVF::TL*)	RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set						( -1,  1, 0, 1, 0,		0,	scale_Y	);	pv++;
		pv->set						( -1, -1, 0, 0, 0,		0,		  0	);	pv++;
		pv->set						(  1,  1, 1, 1, 0, scale_X,	scale_Y	);	pv++;
		pv->set						(  1, -1, 1, 0, 0, scale_X,		  0	);	pv++;
		RCache.Vertex.Unlock		(4,g_combine->vb_stride);

		// Draw
		RCache.set_Element			(s_clear_hud->E[4]	);
		RCache.set_Geometry			(g_combine		);

		RCache.Render				(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

	if (RImplementation.mapDistort.size()) RImplementation.mapDistort.clear();
	if (RImplementation.mapEmissive.size()) RImplementation.mapEmissive.clear();

	m_3dstatic_frame++;
}
