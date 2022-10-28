#include "stdafx.h"
#include "Text_Console.h"
#include "line_editor.h"

extern char const * const		ioc_prompt;
extern char const * const		ch_cursor;
int g_svTextConsoleUpdateRate = 1;

CTextConsole::CTextConsole()
{
	m_pMainWnd    = NULL;
	m_hConsoleWnd = NULL;
	m_hLogWnd     = NULL;
	m_hLogWndFont = NULL;

	m_bScrollLog  = true;
	m_dwStartLine = 0;

	m_bNeedUpdate      = false;
	m_dwLastUpdateTime = Device.dwTimeGlobal;
	m_last_time        = Device.dwTimeGlobal;
}

CTextConsole::~CTextConsole()
{
	m_pMainWnd = NULL;
}

//-------------------------------------------------------------------------------------------
LRESULT CALLBACK TextConsole_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void	CTextConsole::CreateConsoleWnd()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	//----------------------------------
	RECT cRc;
	GetClientRect(*m_pMainWnd, &cRc);
	INT lX = cRc.left;
	INT lY = cRc.top;
	INT lWidth = cRc.right - cRc.left;
	INT lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	const char*	wndclass ="TEXT_CONSOLE";

	// Register the windows class
	WNDCLASS wndClass = { 0, TextConsole_WndProc, 0, 0, hInstance,
		NULL,
		LoadCursor( hInstance, IDC_ARROW ),
		GetStockBrush(GRAY_BRUSH),
		NULL, wndclass };
	RegisterClass( &wndClass );

	// Set the window's initial style
	u32 dwWindowStyle = WS_OVERLAPPED | WS_CHILD | WS_VISIBLE;// | WS_CLIPSIBLINGS;// | WS_CLIPCHILDREN;

	// Set the window's initial width
	RECT rc;
	SetRect			( &rc, lX, lY, lWidth, lHeight );
//	AdjustWindowRect( &rc, dwWindowStyle, FALSE );

	// Create the render window
	m_hConsoleWnd = CreateWindow( wndclass, "XRAY Text Console", dwWindowStyle,
		lX, lY,
		lWidth, lHeight, *m_pMainWnd,
		0, hInstance, 0L );
	//---------------------------------------------------------------------------
	R_ASSERT2(m_hConsoleWnd, "Unable to Create TextConsole Window!");
};
//-------------------------------------------------------------------------------------------
LRESULT CALLBACK TextConsole_LogWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
void	CTextConsole::CreateLogWnd()
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	//----------------------------------
	RECT cRc;
	GetClientRect(m_hConsoleWnd, &cRc);
	INT lX = cRc.left;
	INT lY = cRc.top;
	INT lWidth = cRc.right - cRc.left;
	INT lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	const char*	wndclass ="TEXT_CONSOLE_LOG_WND";

	// Register the windows class
	WNDCLASS wndClass = { 0, TextConsole_LogWndProc, 0, 0, hInstance,
		NULL,
		LoadCursor( NULL, IDC_ARROW ),
		GetStockBrush(BLACK_BRUSH),
		NULL, wndclass };
	RegisterClass( &wndClass );

	// Set the window's initial style
	u32 dwWindowStyle = WS_OVERLAPPED | WS_CHILD | WS_VISIBLE;// | WS_CLIPSIBLINGS;
//	u32 dwWindowStyleEx = WS_EX_CLIENTEDGE;

	// Set the window's initial width
	RECT rc;
	SetRect			( &rc, lX, lY, lWidth, lHeight );
//	AdjustWindowRect( &rc, dwWindowStyle, FALSE );

	// Create the render window
	m_hLogWnd = CreateWindow(wndclass, "XRAY Text Console Log", dwWindowStyle,
		lX, lY,
		lWidth, lHeight, m_hConsoleWnd,
		0, hInstance, 0L );
	//---------------------------------------------------------------------------
	R_ASSERT2(m_hLogWnd, "Unable to Create TextConsole Window!");
	//---------------------------------------------------------------------------
	ShowWindow(m_hLogWnd, SW_SHOW); 
	UpdateWindow(m_hLogWnd);
	//-----------------------------------------------
	LOGFONT lf; 
	lf.lfHeight = -12; 
	lf.lfWidth = 0;
	lf.lfEscapement = 0; 
	lf.lfOrientation = 0; 
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = 0; 
	lf.lfUnderline = 0; 
	lf.lfStrikeOut = 0; 
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_STRING_PRECIS;
	lf.lfClipPrecision = CLIP_STROKE_PRECIS;	
	lf.lfQuality = DRAFT_QUALITY;
	lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	sprintf_s(lf.lfFaceName,sizeof(lf.lfFaceName),"");

	m_hLogWndFont = CreateFontIndirect(&lf);
	R_ASSERT2(m_hLogWndFont, "Unable to Create Font for Log Window");
	//------------------------------------------------
	m_hDC_LogWnd = GetDC(m_hLogWnd);
	R_ASSERT2(m_hDC_LogWnd, "Unable to Get DC for Log Window!");
	//------------------------------------------------
	m_hDC_LogWnd_BackBuffer = CreateCompatibleDC(m_hDC_LogWnd);
	R_ASSERT2(m_hDC_LogWnd_BackBuffer, "Unable to Create Compatible DC for Log Window!");
	//------------------------------------------------
	GetClientRect(m_hLogWnd, &cRc);
	lWidth = cRc.right - cRc.left;
	lHeight = cRc.bottom - cRc.top;
	//----------------------------------
	m_hBB_BM = CreateCompatibleBitmap(m_hDC_LogWnd, lWidth, lHeight);
	R_ASSERT2(m_hBB_BM, "Unable to Create Compatible Bitmap for Log Window!");
	//------------------------------------------------
	m_hOld_BM = (HBITMAP)SelectObject(m_hDC_LogWnd_BackBuffer, m_hBB_BM);
	//------------------------------------------------
	m_hPrevFont = (HFONT)SelectObject(m_hDC_LogWnd_BackBuffer, m_hLogWndFont);
	//------------------------------------------------
	SetTextColor(m_hDC_LogWnd_BackBuffer, RGB(255, 255, 255));
	SetBkColor(m_hDC_LogWnd_BackBuffer, RGB(1, 1, 1));
	//------------------------------------------------
	m_hBackGroundBrush = GetStockBrush(BLACK_BRUSH);
}

void CTextConsole::Initialize()
{
	inherited::Initialize();
	
	m_pMainWnd         = &Device.m_hWnd;
	m_dwLastUpdateTime = Device.dwTimeGlobal;
	m_last_time        = Device.dwTimeGlobal;

	CreateConsoleWnd();
	CreateLogWnd();

	ShowWindow( m_hConsoleWnd, SW_SHOW );
	UpdateWindow( m_hConsoleWnd );	

	m_server_info.ResetData();
}

void CTextConsole::Destroy()
{
	inherited::Destroy();	

	SelectObject( m_hDC_LogWnd_BackBuffer, m_hPrevFont );
	SelectObject( m_hDC_LogWnd_BackBuffer, m_hOld_BM );

	if ( m_hBB_BM )           DeleteObject( m_hBB_BM );
	if ( m_hOld_BM )          DeleteObject( m_hOld_BM );
	if ( m_hLogWndFont )      DeleteObject( m_hLogWndFont );
	if ( m_hPrevFont )        DeleteObject( m_hPrevFont );
	if ( m_hBackGroundBrush ) DeleteObject( m_hBackGroundBrush );

	ReleaseDC( m_hLogWnd, m_hDC_LogWnd_BackBuffer );
	ReleaseDC( m_hLogWnd, m_hDC_LogWnd );

	DestroyWindow( m_hLogWnd );
	DestroyWindow( m_hConsoleWnd );
}

void CTextConsole::OnRender() {} //disable �Console::OnRender()

void CTextConsole::OnPaint()
{
	RECT wRC;
	PAINTSTRUCT ps;
	BeginPaint( m_hLogWnd, &ps );

	if (Device.dwFrame % 2 )
	{
		GetClientRect( m_hLogWnd, &wRC );
	}
	else
	{
		wRC = ps.rcPaint;
	}
	
	
	BitBlt(	m_hDC_LogWnd,
			wRC.left, wRC.top,
			wRC.right - wRC.left, wRC.bottom - wRC.top,
			m_hDC_LogWnd_BackBuffer,
			wRC.left, wRC.top,
			SRCCOPY);
	EndPaint( m_hLogWnd, &ps );
}

void CTextConsole::OnFrame()
{
	inherited::OnFrame();
	InvalidateRect( m_hConsoleWnd, NULL, FALSE );
	SetCursor( LoadCursor( NULL, IDC_ARROW ) );
}
