#include "stdafx.h"
#include "resource.h"
#include "dedicated_server_only.h"

#ifdef INGAME_EDITOR
#	include "../include/editor/ide.hpp"
#	include "engine_impl.hpp"
#endif // #ifdef INGAME_EDITOR

extern LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#ifdef INGAME_EDITOR
void CRenderDevice::initialize_editor	()
{
    Msg("loading \"editor.dll\"...");
	m_editor_module		= LoadLibrary("editor.dll");
	if (!m_editor_module) {
		Msg				("! cannot load library \"editor.dll\"");
		return;
	}
    Msg("\"editor.dll\" loaded successfully");

	m_editor_initialize	= (initialize_function_ptr)GetProcAddress(m_editor_module, "initialize");
	R_ASSERT			(m_editor_initialize);

	m_editor_finalize	= (finalize_function_ptr)GetProcAddress(m_editor_module, "finalize");
    R_ASSERT(m_editor_finalize);

	m_engine			= xr_new<engine_impl>();
	m_editor_initialize	(m_editor, m_engine);
    R_ASSERT(m_editor);

	m_hWnd				= m_editor->view_handle();
    R_ASSERT(m_hWnd != INVALID_HANDLE_VALUE);
    Msg("editor initialized");
}
#endif // #ifdef INGAME_EDITOR

PROTECT_API void CRenderDevice::Initialize			()
{
	Log("Initializing Engine...");
	TimerGlobal.Start			();
	TimerMM.Start				();

#ifdef INGAME_EDITOR
	if (strstr(Core.Params,"-editor"))
		initialize_editor		();
#endif // #ifdef INGAME_EDITOR

	// Unless a substitute hWnd has been specified, create a window to render into
    if( m_hWnd == NULL)
    {
		const char*	wndclass ="_XRAY_";

        // Register the windows class
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
        WNDCLASS wndClass = { 0, WndProc, 0, 0, hInstance,
                              LoadIcon( hInstance, MAKEINTRESOURCE(IDI_ICON1) ),
                              LoadCursor( NULL, IDC_ARROW ),
                              (HBRUSH)GetStockObject(BLACK_BRUSH),
                              NULL, wndclass };
        RegisterClass( &wndClass );

        // Set the window's initial style
        m_dwWindowStyle = WS_BORDER |WS_DLGFRAME;

        // Set the window's initial width
        RECT rc;
        SetRect			( &rc, 0, 0, 640, 480 );
        AdjustWindowRect( &rc, m_dwWindowStyle, FALSE );

        // Create the render window
		m_hWnd = CreateWindow( wndclass, "S.T.A.L.K.E.R.: Shadow Of Chernobyl", m_dwWindowStyle,
                               /*rc.left, rc.top, */CW_USEDEFAULT, CW_USEDEFAULT,
                               (rc.right-rc.left), (rc.bottom-rc.top), 0L,
                               0, hInstance, 0L );
    }

    // Save window properties
    m_dwWindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
    GetWindowRect	( m_hWnd, &m_rcWindowBounds );
    GetClientRect	( m_hWnd, &m_rcWindowClient );

	/*
	if (strstr(lpCmdLine,"-gpu_sw")!=NULL)		HW.Caps.bForceGPU_SW		= TRUE;
	else										HW.Caps.bForceGPU_SW		= FALSE;
	if (strstr(lpCmdLine,"-gpu_nopure")!=NULL)	HW.Caps.bForceGPU_NonPure	= TRUE;
	else										HW.Caps.bForceGPU_NonPure	= FALSE;
	if (strstr(lpCmdLine,"-gpu_ref")!=NULL)		HW.Caps.bForceGPU_REF		= TRUE;
	else										HW.Caps.bForceGPU_REF		= FALSE;
	*/
}

