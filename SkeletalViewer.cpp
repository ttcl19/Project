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

// Global Variables:
CSkeletalViewerApp	g_CSkeletalViewerApp;	// Application class
HINSTANCE			g_hInst;				// current instance
HWND				g_hWndApp;				// Windows Handle to main application
TCHAR				g_szAppTitle[256];		// Application title



//This function creates the dialogue window 
int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	MSG			msg;
	WNDCLASS	wc;

	// Store the instance handle
	g_hInst=hInstance;

	LoadString(g_hInst,IDS_APPTITLE,g_szAppTitle,sizeof(g_szAppTitle)/sizeof(g_szAppTitle[0]));

	// Dialog custom window class
	ZeroMemory(&wc,sizeof(wc));
	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.cbWndExtra=DLGWINDOWEXTRA;
	wc.hInstance=hInstance;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_SKELETALVIEWER));
	wc.lpfnWndProc=DefDlgProc;
	wc.lpszClassName=SZ_APPDLG_WINDOW_CLASS;
	if(!RegisterClass(&wc))
		return (0);

	// Create main application window
	g_hWndApp=CreateDialogParam(g_hInst,MAKEINTRESOURCE(IDD_APP),NULL,(DLGPROC) CSkeletalViewerApp::WndProc,NULL);

	// Show window
	ShowWindow(g_hWndApp,nCmdShow);
	UpdateWindow(g_hWndApp);

	// Main message loop:
	while(GetMessage(&msg,NULL,0,0)) 
	{
		// If a dialog message
		if(g_hWndApp!=NULL && IsDialogMessage(g_hWndApp,&msg))
			continue;

		// otherwise do default window processing
		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }
	return (msg.wParam);
}


//Handles messeges sent to the window by the system
//Does not accept user input 
LONG CALLBACK CSkeletalViewerApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Initializzes the miain class object g_CSkeletonViewApp
	switch(message)
	{

		/*
		 WM_INITDIALOG arrives before the window appears. In response, SkeletalViewer initializes the NUI 
		API, starts processing data from the Kinect sensor, and sets up the font to display frames per 
		second


		WM_CLOSE arrives when the user clicks the box in the menu bar to close the program. 
		SkeletalViewer simply calls the DestroyWindow function.

		
		WM_DESTROY is sent by the DestroyWindow function. SkeletalViewer uninitializes the NUI API, 
		deletes the font object, and calls the PostQuitMessage function to inform the system that the 
		thread is ready to terminate.	
		*/
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



