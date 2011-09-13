/************************************************************************
*                                                                       *
*   SkeletalViewer.h -- Declares of CSkeletalViewerApp class            *
*                                                                       *
* Copyright (c) Microsoft Corp. All rights reserved.                    *
*                                                                       *
* This code is licensed under the terms of the                          *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research       *
* License Agreement: http://research.microsoft.com/KinectSDK-ToU        *
*                                                                       *
************************************************************************/

#pragma once

#include "ShapeLibrary.h"

#include "resource.h"
#include "MSR_NuiApi.h"
#include "DrawDevice.h"


#define SZ_APPDLG_WINDOW_CLASS        _T("SkeletalViewerAppDlgWndClass")

class CSkeletalViewerApp
{
public:
    HRESULT Nui_Init();
    void                    Nui_UnInit( );
    void                    Nui_GotDepthAlert( );
    void                    Nui_GotVideoAlert( );
    void                    Nui_GotSkeletonAlert( );
    void                    Nui_Zero();
    void                    Nui_BlankSkeletonScreen( HWND hWnd );
    void                    Nui_DoDoubleBuffer(HWND hWnd,HDC hDC);
    void                    Nui_DrawSkeleton( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor );
    void                    Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int numJoints, ... );

    void					drawBox(int boxIndex, RGBQUAD * color);
    void					drawBox(int boxIndex, RGBQUAD * color,double opacity);
	RGBQUAD                 Nui_ShortToQuad_Depth( USHORT s );

    static LONG CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
	HFONT					m_hFontFPS;

	void					KeyboardInput(WPARAM keyCode);
	void					CapturePicture();
	int						(*TweetPicture)(int shape, int orientation,int w, int h,void* ptr);
    void*					(*globalAlloc)(int size);

	int						TwitterPost(int shape, int orientation, int X1, int Y1, int X2, int Y2);
	int						numericCommand(int cmd);

	void					newShape(int ShapeIndex);
	int						newRandomShape();

private:
    static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);


    // NUI and draw stuff
    DrawDevice    m_DrawDepth;
    DrawDevice    m_DrawVideo;

    // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;

    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_hNextVideoFrameEvent;
    HANDLE        m_hNextSkeletonEvent;
    HANDLE        m_pDepthStreamHandle;
    HANDLE        m_pVideoStreamHandle;
    HPEN          m_Pen[6];
    HDC           m_SkeletonDC;
    HBITMAP       m_SkeletonBMP;
    HGDIOBJ       m_SkeletonOldObj;
    int           m_PensTotal;
    POINT         m_Points[NUI_SKELETON_POSITION_COUNT];
    RGBQUAD       m_rgbWk[640*480];
	ULONGLONG	  m_videoDelay;
	RGBQUAD		  m_videoCache[640*480]; //raw video input
	RGBQUAD		  m_videoEffects[640*480]; //for drawing special effects on top.

	USHORT		  m_playerMap[640*480];
	UINT		  m_numHBox;
	UINT		  m_numVBox;
	UINT		  m_boxHeight;
	UINT		  m_boxWidth;
	UINT		  m_offset;
	int			  m_p1Index;
	int			  m_p2Index;
	int *		  m_selectedShape;

	//for TwitterPost use 
	int			  ShapeIndex;
	int			  ori;   //What is this? Dustin


	ULONGLONG	  m_timeLimit;
	ULONGLONG	  m_timeAvailable; //Before Round ends

    int           m_LastSkeletonFoundTime;
    bool          m_bScreenBlanked;
    int           m_FramesTotal;
    int           m_LastFPStime;
    int           m_LastFramesTotal;

	//captured pictures
	RGBQUAD  	  m_CapturedPictures[100][640*480];
	int			  m_NumCapturedPictures;
};

int MessageBoxResource(HWND hwnd,UINT nID,UINT nType);