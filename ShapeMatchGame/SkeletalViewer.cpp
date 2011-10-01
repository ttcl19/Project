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

#include <iostream>
#include <sstream>

// Global Variables:
CSkeletalViewerApp	g_CSkeletalViewerApp;	// Application class
HINSTANCE			g_hInst;				// current instance
HWND				g_hWndApp;				// Windows Handle to main application
TCHAR				g_szAppTitle[256];		// Application title

extern "C" _declspec(dllexport) void openKinectWindow();
extern "C" _declspec(dllexport) int setTweetback(void* (*globalAlloc)(int size), int (*TweetPicture)(int shape, int orientation, int w, int h, int squareSize, void* ptr));

extern "C" _declspec(dllexport) int setOSCEvents(
	void (*RoundStart)(int goalShape, int orientation, int* goalShapeStatus), 
	void (*Countdown)(int timeRemaining), 
	void (*Holding)(int player), 
	void (*HoldFail)(int player), 
	void (*Timeout)(), 
	void (*ShapeCompleted)(int winner), 
	void (*ShapeStatus)(int shape1, int shape2),
	void (*PlayerStatus)(int* players)
	);

extern "C" _declspec(dllexport) int numericCommand(int cmd);

int numericCommand(int cmd) {
	return g_CSkeletalViewerApp.numericCommand(cmd);
}

int CSkeletalViewerApp::numericCommand(int cmd)
{
	if (cmd >= 1 && cmd <= 7)
	{
		newShape(cmd);
	} else {
		switch(cmd)
		{
			case -1: //random shape
				newRandomShape();
				break;

			case 0: //force tweet
				TwitterPost(0,0,0,0,640,480);
				break;
		}
	}
	
	return 0;
}

int CSkeletalViewerApp::newRandomShape()
{
	int shape = rand() % 8;
	newShape(shape);
	return shape;
}

void CSkeletalViewerApp::newShape(int newShapeIndex)
{
	ori = 0;

	switch(newShapeIndex)
	{
		case 1:
			ori = rand() % Shapes::ori[0];
			m_selectedShape = Shapes::I[ori];
			//make note of the selected shape as a number
			//to be passed onto TwitterPost 
			ShapeIndex = 1; 
			break;
		case 2:
			ori = rand() % Shapes::ori[1];
			m_selectedShape = Shapes::J[ori];
			ShapeIndex = 2; 
			break;
		case 3:
			ori = rand() % Shapes::ori[2];
			m_selectedShape = Shapes::L[ori];
			ShapeIndex = 3; 
			break;
		case 4:
			ori = rand() % Shapes::ori[3];
			m_selectedShape = Shapes::O[ori];
			ShapeIndex = 4; 
			break;
		case 5:
			ori = rand() % Shapes::ori[4];
			m_selectedShape = Shapes::Z[ori];
			ShapeIndex = 5; 
			break;
		case 6:
			ori = rand() % Shapes::ori[5];
			m_selectedShape = Shapes::T[ori];
			ShapeIndex = 6; 
			break;
		case 7:
			ori = rand() % Shapes::ori[6];
			m_selectedShape = Shapes::S[ori];
			ShapeIndex = 7; 
			break;
	}

	m_timeLimit = GetTickCount64() + m_timeAvailable;
	printf("m_timeLimit %i\n",m_timeLimit);
	
	if (m_selectedShape != 0)
	{
		RoundStart(ShapeIndex,ori,m_selectedShape);
	}

}

void CSkeletalViewerApp::KeyboardInput(WPARAM keyCode)
{
	//Codes here:
	//http://msdn.microsoft.com/en-us/library/dd375731(v=vs.85).aspx

	int ori = 0;

	switch(keyCode)
	{
		case 0x31 : //key 1
			//the remainder here will always be less than the number of orientations 
			//for each respective shape 
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
			return;
		case 0x54: //key T.
			TwitterPost(0,0, 0, 0, 640, 480);
			break;

	}

	m_timeLimit = GetTickCount64() + m_timeAvailable;
	printf("m_timeLimit %i",m_timeLimit);

}

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

			g_CSkeletalViewerApp.newRandomShape(); //Start the game!
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

int setTweetback(void* (*globalAlloc)(int size), int (*TweetPicture)(int shape, int orientation,int w, int h, int squareSize, void* ptr))
{
	g_CSkeletalViewerApp.globalAlloc = globalAlloc;
	g_CSkeletalViewerApp.TweetPicture = TweetPicture;
	return 0;
}

int setOSCEvents(
	void (*RoundStart)(int goalShape, int orientation, int* goalShapeStatus), 
	void (*Countdown)(int timeRemaining), 
	void (*Holding)(int player), 
	void (*HoldFail)(int player), 
	void (*Timeout)(), 
	void (*ShapeCompleted)(int winner), 
	void (*ShapeStatus)(int shape1, int shape2),
	void (*PlayerStatus)(int* players)
	)
{
	g_CSkeletalViewerApp.RoundStart = RoundStart;
	g_CSkeletalViewerApp.goalShapeStatus = (int*)g_CSkeletalViewerApp.globalAlloc(4*6*sizeof(int));


	g_CSkeletalViewerApp.Countdown = Countdown;
	g_CSkeletalViewerApp.Holding = Holding;
	g_CSkeletalViewerApp.HoldFail = HoldFail;
	g_CSkeletalViewerApp.Timeout = Timeout;
	g_CSkeletalViewerApp.ShapeCompleted = ShapeCompleted;
	g_CSkeletalViewerApp.ShapeStatus = ShapeStatus;
	g_CSkeletalViewerApp.PlayerStatus = PlayerStatus;
	g_CSkeletalViewerApp.players = (int*)g_CSkeletalViewerApp.globalAlloc(4*6*sizeof(int));

	return 0;
}

void openKinectWindow()
{
	MSG			msg;
	WNDCLASS	wc;

	LoadString(g_hInst,IDS_APPTITLE,g_szAppTitle,sizeof(g_szAppTitle)/sizeof(g_szAppTitle[0]));

	// Dialog custom window class
	ZeroMemory(&wc,sizeof(wc));
	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.cbWndExtra=DLGWINDOWEXTRA;
	wc.hInstance=g_hInst;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hIcon=LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_SKELETALVIEWER));
	wc.lpfnWndProc=DefDlgProc;
	wc.lpszClassName=SZ_APPDLG_WINDOW_CLASS;
	if(!RegisterClass(&wc))
		return;

	// Create main application window
	g_hWndApp=CreateDialogParam(g_hInst,MAKEINTRESOURCE(IDD_APP),NULL,(DLGPROC) CSkeletalViewerApp::WndProc,NULL);

	// Show window
	ShowWindow(g_hWndApp,SW_SHOW);
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
}

int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	//cURL stuff
	//curl_global_init(CURL_GLOBAL_ALL);



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



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	//HMODULE is equivalent to HINSTANCE
	
	switch ( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
		// A process is loading the DLL.
		break;
		case DLL_THREAD_ATTACH:
		// A process is creating a new thread.
		break;
		case DLL_THREAD_DETACH:
		// A thread exits normally.
		break;
		case DLL_PROCESS_DETACH:
		// A process unloads the DLL.
		break;
	}
	
	g_hInst = hModule;

	return TRUE;
}





