#include "stdafx.h"
#pragma hdrstop

#include "Blender_light_spot.h"

CBlender_accum_spot::CBlender_accum_spot	()	{	description.CLS		= 0;	}
CBlender_accum_spot::~CBlender_accum_spot	()	{	}

void	CBlender_accum_spot::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

//	BOOL		b_HW_smap	= RImplementation.o.HW_smap;
//	BOOL		b_HW_PCF	= RImplementation.o.HW_smap_PCF;
	BOOL		blend		= RImplementation.o.fp16_blend;
	D3DBLEND	dest		= blend?D3DBLEND_ONE:D3DBLEND_ZERO;

	switch (C.iElement)
	{
	case SE_L_FILL:			// masking
		C.r_Pass			("stub_notransform","copy",						false,	FALSE,	FALSE);
		//C.r_Sampler			("s_base",			C.L_textures[0]);
		C.r_dx10Texture		("s_base",			C.L_textures[0]);
		C.r_dx10Sampler		("smp_nofilter");
		C.r_End				();
		break;
	case SE_L_UNSHADOWED:	// unshadowed
		C.r_Pass			("accum_volume",	"accum_spot_unshadowed",	false,	FALSE,FALSE,blend,D3DBLEND_ONE,dest);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_lmap",			C.L_textures[0]);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum		);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");

		C.r_End				();
		break;
	case SE_L_NORMAL:		// normal
		C.r_Pass			("accum_volume",	"accum_spot_normal",		false,	FALSE,FALSE,blend,D3DBLEND_ONE,dest);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_lmap",			C.L_textures[0]);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");

		C.r_End				();
		break;
	case SE_L_FULLSIZE:		// normal-fullsize
		C.r_Pass			("accum_volume",	"accum_spot_fullsize",		false,	FALSE,FALSE,blend,D3DBLEND_ONE,dest);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_lmap",			C.L_textures[0]);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");
		C.r_End				();
		break;
	case SE_L_TRANSLUENT:	// shadowed + transluency
		C.r_Pass			("accum_volume",	"accum_spot_fullsize",		false,	FALSE,FALSE,blend,D3DBLEND_ONE,dest);

		C.r_dx10Texture		("s_position",		r2_RT_P);
		C.r_dx10Texture		("s_material",		r2_material);
		C.r_dx10Texture		("s_lmap",			C.L_textures[0]);
		C.r_dx10Texture		("s_smap",			r2_RT_smap_depth);
		C.r_dx10Texture		("s_accumulator",	r2_RT_accum);

		C.r_dx10Sampler		("smp_nofilter");
		C.r_dx10Sampler		("smp_material");
		C.r_dx10Sampler		("smp_rtlinear");
		jitter				(C);
		C.r_dx10Sampler		("smp_smap");
		C.r_End				();
		break;
	}
}
