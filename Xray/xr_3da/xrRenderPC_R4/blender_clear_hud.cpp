#include "stdafx.h"
#pragma hdrstop

#include "blender_clear_hud.h"

CBlender_ClearHUD::CBlender_ClearHUD	()	{	description.CLS		= 0;	}
CBlender_ClearHUD::~CBlender_ClearHUD	()	{	}

void	CBlender_ClearHUD::Compile			(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass			("clear_hud",		"dumb",	FALSE,	FALSE,	TRUE);
		C.r_Stencil			(TRUE, D3DCMP_EQUAL, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_ZERO, D3DSTENCILOP_KEEP);
		C.r_StencilRef		(0x02);
		C.r_ColorWriteEnable(false, false, false, false);
		C.r_End				();
		break;
	case 1:
		C.r_Pass			("clear_hud",		"dumb",	FALSE,	FALSE, FALSE);
		C.r_Stencil			(TRUE, D3DCMP_EQUAL, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_DECRSAT, D3DSTENCILOP_KEEP);
		C.r_StencilRef		(0x02);
		C.r_ColorWriteEnable(false, false, false, false);
		C.r_End				();
		break;
	case 2:		// UI Light
		C.r_Pass			("combine_1",		"ui_3d_static_light",	FALSE,	FALSE,	FALSE);
		C.r_Stencil			(TRUE,D3DCMP_LESSEQUAL,0xff,0x00);	// stencil should be >= 1
		C.r_StencilRef		(0x01);
		C.r_dx10Texture		("s_position",		r2_RT_P				);
		C.r_dx10Texture		("s_diffuse",		r2_RT_albedo		);
		C.r_dx10Texture		("s_material",		r2_material			);
		C.r_dx10Texture		("env_s0",			r2_T_envs0			);
		C.r_dx10Texture		("env_s1",			r2_T_envs1			);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 3:		// UI combine
		C.r_Pass			("combine_1",		"ui_3d_static_taa",	FALSE,	FALSE,	FALSE,  FALSE);
		C.r_dx10Texture		("s_image0",		r2_RT_3dstatic_prev0);
		C.r_dx10Texture		("s_image1",		r2_RT_3dstatic_prev1);
		C.r_dx10Texture		("s_image2",		r2_RT_3dstatic_prev2);
		C.r_dx10Texture		("s_image3",		r2_RT_3dstatic_prev3);
		//C.r_dx10Texture		("s_image_taa",		"$user$3dstatic_taa");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 4:		// UI post
		C.r_Pass			("combine_1",		"ui_3d_static_post",	FALSE,	FALSE,	FALSE, TRUE, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA);
		C.r_dx10Texture		("s_3dstatic",		r2_RT_3dstatic);
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	case 5:	// post-processing
		C.r_Pass			("combine_1",		"combine_taa",		FALSE,	FALSE,	FALSE, FALSE );
		//C.r_Stencil			(TRUE,D3DCMP_LESSEQUAL,0xff,0x00);	// stencil should be >= 1
		//C.r_StencilRef		(0x01);
		C.r_dx10Texture		("s_generic_prev0",			r2_RT_generic_prev0);
		C.r_dx10Texture		("s_generic_prev1",			r2_RT_generic_prev1);
		C.r_dx10Texture		("s_hud_prev0",			r2_RT_hud_prev0);
		C.r_dx10Texture		("s_hud_prev1",			r2_RT_hud_prev1);
		C.r_dx10Texture		("s_position",		r2_RT_P				);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_rtlinear");
		C.r_End				();
		break;
	}
}
