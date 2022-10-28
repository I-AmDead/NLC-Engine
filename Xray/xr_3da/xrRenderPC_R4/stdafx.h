// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#pragma warning(disable:4995)
#include "../xr_3da/stdafx.h"
#pragma warning(disable:4995)
#include <d3dx9.h>
#pragma warning(default:4995)
#pragma warning(disable:4714)
#pragma warning( 4 : 4018 )
#pragma warning( 4 : 4244 )
#pragma warning(disable:4237)

#include <D3D11.h>
#include <D3Dx11core.h>
#include <D3DCompiler.h>

#include "../xrRender/xrD3DDefs.h"

#include "../xrRenderDX10/Debug/dxPixEventWrapper.h"

#define		R_R1	1
#define		R_R2	2
#define		R_R3	3
#define		R_R4	4
#define		RENDER	R_R4

#include "../../xr_3da/psystem.h"

#include "../xr_3da/HW.h"
#include "../xr_3da/Shader.h"
#include "../xr_3da/R_Backend.h"
#include "../xr_3da/R_Backend_Runtime.h"

#include "../xr_3da/resourcemanager.h"

#include "../../xr_3da/vis_common.h"
#include "../../xr_3da/render.h"
#include "../../xr_3da/_d3d_extensions.h"
#include "../../xr_3da/igame_level.h"
#include "../xr_3da/blenders\blender.h"
#include "../xr_3da/blenders\blender_clsid.h"
#include "../xrRender/xrRender_console.h"
#include "r4.h"

IC	void	jitter(CBlender_Compile& C)
{
//	C.r_Sampler	("jitter0",	JITTER(0), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
//	C.r_Sampler	("jitter1",	JITTER(1), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
//	C.r_Sampler	("jitter2",	JITTER(2), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
//	C.r_Sampler	("jitter3",	JITTER(3), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
	C.r_dx10Texture	("jitter0",	JITTER(0));
	C.r_dx10Texture	("jitter1",	JITTER(1));
	C.r_dx10Texture	("jitter2",	JITTER(2));
	C.r_dx10Texture	("jitter3",	JITTER(3));
	C.r_dx10Texture	("jitter4",	JITTER(4));
	C.r_dx10Texture	("jitterMipped",	r2_jitter_mipped);
	C.r_dx10Sampler	("smp_jitter");
}
