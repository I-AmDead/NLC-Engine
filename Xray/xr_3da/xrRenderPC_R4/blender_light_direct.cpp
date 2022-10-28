#include "stdafx.h"
#pragma hdrstop

#include "Blender_light_direct.h"

CBlender_accum_direct::CBlender_accum_direct	()	{ description.CLS		= 0;	}
CBlender_accum_direct::~CBlender_accum_direct	()	{	}

void	CBlender_accum_direct::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

//	BOOL	b_HW_smap		= RImplementation.o.HW_smap;
//	BOOL	b_HW_PCF		= RImplementation.o.HW_smap_PCF;
	BOOL		blend		= FALSE;	//RImplementation.o.fp16_blend;
	D3DBLEND	dest		= blend?D3DBLEND_ONE:D3DBLEND_ZERO;
	if (RImplementation.o.sunfilter)	{ blend = FALSE; dest = D3DBLEND_ZERO; }

	switch (C.iElement)
	{
	case SE_SUN_NEAR:		// near pass - enable Z-test to perform depth-clipping
	case SE_SUN_MIDDLE:		// middle pass - enable Z-test to perform depth-clipping
		//	FVF::TL2uv
		C.r_Pass			("accum_sun","accum_sun_near",	false,	TRUE,	FALSE,blend,D3DBLEND_ONE,dest);

		C.r_CullMode		(D3DCULL_NONE);
		C.PassSET_ZB		(TRUE,FALSE,TRUE	);	// force inverted Z-Buffer

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;
	case SE_SUN_FAR:		// far pass, only stencil clipping performed
		//	FVF::TL2uv
		//C.r_Pass			("null",			"accum_sun_far",	false,	TRUE,	FALSE,blend,D3DBLEND_ONE,dest);
		C.r_Pass			("accum_sun","accum_sun_far",	false,	TRUE,	FALSE,blend,D3DBLEND_ONE,dest);
		C.r_CullMode		(D3DCULL_NONE);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		{
			u32 s = C.r_dx10Sampler("smp_smap");
			C.i_dx10Address		(s, D3DTADDRESS_BORDER);
			C.i_dx10BorderColor	(s, D3DCOLOR_ARGB(255, 255, 255, 255));
		}

		C.r_End				();
		break;
	case SE_SUN_LUMINANCE:	// luminance pass
		//C.r_Pass			("null",			"accum_sun",		false,	FALSE,	FALSE);
		C.r_Pass			("stub_notransform_aa_AA","accum_sun",		false,	FALSE,	FALSE);
		C.r_CullMode		(D3DCULL_NONE);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_smap",			r2_RT_generic0);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		jitter				(C);
		C.r_End				();
		break;
		/*
	//	SE_SUN_FAR for min/max
	case 5:		// far pass, only stencil clipping performed
		//	FVF::TL2uv
		C.r_Pass			("stub_notransform_2uv","accum_sun_far",	false,	TRUE,	FALSE,blend,D3DBLEND_ONE,dest);
		C.r_CullMode		(D3DCULL_NONE);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_normal",		r2_RT_N);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);
		C.r_dx10Texture		("s_lmap",			r2_sunmask);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_linear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;
		*/
	}
}
