#include "stdafx.h"

#ifdef DEBUG
	ECORE_API BOOL bDebug	= FALSE;
	
#endif

// Video
//. u32			psCurrentMode		= 1024;
u32			psCurrentVidMode[2] = {1024,768};
u32			psCurrentBPP		= 32;
// release version always has "mt_*" enabled
Flags32		psDeviceFlags		= {rsFullscreen|rsDetails|mtPhysics|mtSound|mtNetwork|rsDrawStatic|rsDrawDynamic|rsRefresh60hz};

Flags32		ps_r2_engine_flags = {
	R1FLAG_DETAIL_TEXTURES
	| R2FLAG_DETAIL_BUMP
	| R3FLAG_CLEAR_HUD
	| R1FLAG_3DSTATIC
};

// textures 
int			psTextureLOD		= 1;
