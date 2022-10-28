#ifndef	STDAFX_3DA
#define STDAFX_3DA

#pragma once

#ifdef _EDITOR
	#include "..\editors\ECore\stdafx.h"
#else

#include "../xrCore/xrCore.h"

#ifdef _DEBUG
	#define D3D_DEBUG_INFO
#endif

#pragma warning(disable:4995)
#include <d3d9.h>
#ifdef USE_DX10
//#include <d3dx9core.h>
#include <d3d10_1.h>
#include <d3dx10core.h>
#endif
#ifdef USE_DX11
#include <d3d11.h>
#include <D3Dx11core.h>
#include <D3DCompiler.h>
#endif
//#include <dplay8.h>
#pragma warning(default:4995)

// you must define ENGINE_BUILD then building the engine itself
// and not define it if you are about to build DLL
#ifndef NO_ENGINE_API
	#ifdef	ENGINE_BUILD
		#define DLL_API			__declspec(dllimport)
		#define ENGINE_API		__declspec(dllexport)
	#else
		#define DLL_API			__declspec(dllexport)
		#define ENGINE_API		__declspec(dllimport)
	#endif
#else
	#define ENGINE_API
	#define DLL_API
#endif // NO_ENGINE_API

#define ECORE_API

// Our headers
#include "engine.h"
#include "defines.h"
#ifndef NO_XRLOG
#include "../xrCore/log.h"
#endif
#include "device.h"
//#include "fs.h"
#include "xrD3DDefs.h"

#include "xrXRC.h"

#include "../xrSound/sound.h"

extern ENGINE_API CInifile *pGameIni;

#pragma comment( lib, "xrCore.lib"	)
#pragma comment( lib, "xrCDB.lib"	)
#pragma comment( lib, "xrSound.lib"	)
#pragma comment( lib, "lua5.1.lib"	)
#pragma comment( lib, "luabind.lib"	)

#pragma comment( lib, "winmm.lib"		)

#pragma comment( lib, "d3d9.lib"		)
#pragma comment( lib, "d3dx9.lib"		)
#pragma comment( lib, "dinput8.lib"		)
#pragma comment( lib, "dxguid.lib"		)
#ifdef USE_DX10
#pragma comment( lib, "d3d10_1.lib"		)
#pragma comment( lib, "d3dx10.lib"		)
//#pragma comment( lib, "d3dcompiler.lib"		)
#endif
#ifdef USE_DX11
#pragma comment( lib, "d3d11.lib"		)
#pragma comment( lib, "d3dx11.lib"		)
#pragma comment( lib, "d3dcompiler.lib"		)
#endif
#pragma comment( lib, "dxgi.lib"		)
#pragma comment( lib, "d3d10.lib"		)

#ifndef DEBUG
#define LUABIND_NO_ERROR_CHECKING2
#endif




#if	!defined(DEBUG) || defined(FORCE_NO_EXCEPTIONS)
	// release: no error checking, no exceptions
	#define LUABIND_NO_EXCEPTIONS
	#define BOOST_THROW_EXCEPTION_HPP_INCLUDED
	namespace std	{	class exception; }
	namespace boost {	ENGINE_API	void throw_exception(const std::exception &A);	};
#endif
#define LUABIND_DONT_COPY_STRINGS

#endif // !M_BORLAND
#endif // !defined STDAFX_3DA
