/************************************************************************
*                                                                       *
*   SkeletalViewer.cpp -- This module provides sample code used to      *
*                         demonstrate Kinect NUI processing             *
*                                                                       *
* Copyright (c) Microsoft Corp. All rights reserved.                    *
*                                                                       *
* This code is licensed under the terms of the                          *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research       *
* License Agreement: http://research.microsoft.com/KinectSDK-ToU        *
*                                                                       *
************************************************************************/

// Note: 
//  1. DirectX SDK must be installed before compiling. 
//  2. DirectX SDK include path should be added after the VC include
//     path, because strsafe.h in DirectX SDK may be older.
//  3. platform SDK lib path should be added before the VC lib
//     path, because uuid.lib in VC lib path may be older

#include "stdafx.h"
#include "SkeletalViewer.h"
#include "resource.h"
#include "ShapeLibrary.h"

// Global Variables:
CSkeletalViewerApp	g_CSkeletalViewerApp;	// Application class
HINSTANCE			g_hInst;				// current instance
HWND				g_hWndApp;				// Windows Handle to main application
TCHAR				g_szAppTitle[256];		// Application title

void CSkeletalViewerApp::KeyboardInput(WPARAM keyCode)
{
	//Codes here:
	//http://msdn.microsoft.com/en-us/library/dd375731(v=vs.85).aspx

	int ori = 0;

	switch(keyCode)
	{
		case 0x31 : //key 1
			ori = rand() % Shapes::ori[0];
			m_selectedShape = Shapes::I[ori];
			break;
		case 0x32 : //key 2
			ori = rand() % Shapes::ori[1];
			m_selectedShape = Shapes::J[ori];
			break;
		case 0x33 : //key 3
			ori = rand() % Shapes::ori[2];
			m_selectedShape = Shapes::L[ori];
			break;
		case 0x34 : //key 4
			ori = rand() % Shapes::ori[3];
			m_selectedShape = Shapes::O[ori];
			break;
		case 0x35 : //key 5
			ori = rand() % Shapes::ori[4];
			m_selectedShape = Shapes::Z[ori];
			break;
		case 0x36 : //key 6
			ori = rand() % Shapes::ori[5];
			m_selectedShape = Shapes::T[ori];
			break;
		case 0x37 : //key 7
			ori = rand() % Shapes::ori[6];
			m_selectedShape = Shapes::S[ori];
			break;
		case 0x43 : //key C
			CapturePicture();
			break;
	}
}


/*****************************************************************************/
/* int MessageBoxResourceV(HWND hwnd,UINT nID,UINT nType, ... )
/*
/* Superset of MessageBox functionality allowing a user to specify a String
/* table loaded string 
/*****************************************************************************/
int MessageBoxResource(HWND hwnd,UINT nID,UINT nType)
{
static TCHAR szRes[512];
int         nRet;

LoadString(g_hInst,nID,szRes,sizeof(szRes)/sizeof(szRes[0]));
nRet=::MessageBox(hwnd,szRes,g_szAppTitle,nType);
return (nRet);
}

/*LONG*/ LRESULT  CALLBACK CSkeletalViewerApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch(message)
	{
	    case WM_CREATE:

			break;
		case WM_INITDIALOG:
			{
			LOGFONT lf;

			// Clean state the class
			g_CSkeletalViewerApp.Nui_Zero();

			// Bind application window handle
			g_CSkeletalViewerApp.m_hWnd=hWnd;

			// Initialize and start NUI processing
			g_CSkeletalViewerApp.Nui_Init();

			// Set the font for Frames Per Second
			GetObject((HFONT) GetStockObject(DEFAULT_GUI_FONT),sizeof(lf),&lf);
			lf.lfHeight*=4;
			g_CSkeletalViewerApp.m_hFontFPS=CreateFontIndirect(&lf);
			SendDlgItemMessage(hWnd,IDC_FPS,WM_SETFONT,(WPARAM) g_CSkeletalViewerApp.m_hFontFPS,0);
			}
			break;

		// If the titlebar X is clicked destroy app
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			// Uninitialize NUI
			g_CSkeletalViewerApp.Nui_UnInit();

			// Other cleanup
			DeleteObject(g_CSkeletalViewerApp.m_hFontFPS);

			// Quit the main message pump
			PostQuitMessage(0);
			break;
	}
	return (FALSE);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	LoadString(g_hInst,IDS_APPTITLE,g_szAppTitle,sizeof(g_szAppTitle)/sizeof(g_szAppTitle[0]));

	// Dialog custom window class
	WNDCLASSEX 	wc;

	ZeroMemory(&wc,sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	//wc.lpfnWndProc    = WndProc;             //wnd Procedure pointer
	wc.cbWndExtra     = DLGWINDOWEXTRA;
	wc.hInstance      = hInstance; 
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_SKELETALVIEWER));
	wc.lpfnWndProc=DefDlgProc;
	wc.lpszClassName=SZ_APPDLG_WINDOW_CLASS;
	if(!RegisterClassEx(&wc))
		return (0);

	// Store the instance handle
	g_hInst=hInstance;

	// Create main application window
	//g_hWndApp=CreateDialog(g_hInst,(LPCTSTR)IDD_APP,0,0);
	g_hWndApp=CreateDialogParam(g_hInst,MAKEINTRESOURCE(IDD_APP),NULL,(DLGPROC) CSkeletalViewerApp::WndProc,NULL);

	/*g_hWndApp=CreateWindow(
		SZ_APPDLG_WINDOW_CLASS,
		g_szAppTitle,
		DS_SETFONT | DS_FIXEDSYS | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		1024,700, 
		NULL,
		NULL,
		hInstance,
		NULL);*/

	if (!g_hWndApp)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Win32 Guided Tour"),
            NULL);

        return 1;
    }

	// Show window
	ShowWindow(g_hWndApp,nCmdShow);
	UpdateWindow(g_hWndApp);

	SetFocus(g_hWndApp);

	// Main message loop:
	MSG			msg;
	int		lastmsg  = 0;
	while(GetMessage(&msg,NULL,0,0)) 
	{
		// If a dialog message
		/*if(g_hWndApp!=NULL && IsDialogMessage(g_hWndApp,&msg))
			continue;*/

		switch (msg.message)
		{
			case WM_KEYDOWN:
				lastmsg = msg.message;

				g_CSkeletalViewerApp.KeyboardInput(msg.wParam);

				break;
		}

		// otherwise do default window processing
		TranslateMessage(&msg);
		DispatchMessage(&msg);

    }
	return (msg.wParam);
}






