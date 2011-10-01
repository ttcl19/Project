#include "stdafx.h"
#include "SkeletalViewer.h"

#include "resource.h"
#include <mmsystem.h>



void CSkeletalViewerApp::Nui_Zero()
{
    m_hNextDepthFrameEvent = NULL;
    m_hNextVideoFrameEvent = NULL;
    m_hNextSkeletonEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_pVideoStreamHandle = NULL;
    m_hThNuiProcess=NULL;
    m_hEvNuiProcessStop=NULL;
    ZeroMemory(m_Pen,sizeof(m_Pen));
    m_SkeletonDC = NULL;
    m_SkeletonBMP = NULL;
    m_SkeletonOldObj = NULL;
    m_PensTotal = 6;
    ZeroMemory(m_Points,sizeof(m_Points));
    m_LastSkeletonFoundTime = -1;
    m_bScreenBlanked = false;
    m_FramesTotal = 0;
    m_LastFPStime = -1;
    m_LastFramesTotal = 0;
}

HRESULT CSkeletalViewerApp::Nui_Init()
{
	printf("NuiInit\n");

	m_numHBox = 6;
	m_numVBox = 4;
	m_boxHeight = 100;
	m_boxWidth = 100;
	y_box_offset = 80;

	winningShapeStatus = new int[4*6];

	p1Points = 0; p2Points = 0; pMaxPoints = 10; lastPointWinner = 0;

	m_p1Index = -1;
	m_p2Index = -1;
	
	srand((UINT) GetTickCount64());
	m_selectedShape = NULL;

	m_timeLimit = 1;
	m_timeLimit = 10;
	m_timeAvailable = 10 * 1000; //HACK make 10 seconds later.
	lastCountdownMessageSent = -1;

	m_NumCapturedPictures = 0;

    HRESULT                hr;
    RECT                rc;

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    GetWindowRect(GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rc );
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    HDC hdc = GetDC(GetDlgItem( m_hWnd, IDC_SKELETALVIEW));
    m_SkeletonBMP = CreateCompatibleBitmap( hdc, width, height );
    m_SkeletonDC = CreateCompatibleDC( hdc );
    ::ReleaseDC(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), hdc );
    m_SkeletonOldObj = SelectObject( m_SkeletonDC, m_SkeletonBMP );

    hr = m_DrawDepth.CreateDevice( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ) );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DCREATE,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = m_DrawDepth.SetVideoType( 320, 240, 320 * 4 );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DVIDEOTYPE,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = m_DrawVideo.CreateDevice( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ) );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DCREATE,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = m_DrawVideo.SetVideoType( 640, 480, 640 * 4 );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DVIDEOTYPE,MB_OK | MB_ICONHAND);
        return hr;
    }
    
    hr = NuiInitialize( 
        NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_NUIINIT,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_SKELETONTRACKING,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextVideoFrameEvent,
        &m_pVideoStreamHandle );
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_VIDEOSTREAM,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
        NUI_IMAGE_RESOLUTION_320x240,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_DEPTHSTREAM,MB_OK | MB_ICONHAND);
        return hr;
    }

    // Start the Nui processing thread
    m_hEvNuiProcessStop=CreateEvent(NULL,FALSE,FALSE,NULL);
    m_hThNuiProcess=CreateThread(NULL,0,Nui_ProcessThread,this,0,NULL);

    return hr;
}

void CSkeletalViewerApp::Nui_UnInit( )
{
    ::SelectObject( m_SkeletonDC, m_SkeletonOldObj );
    DeleteDC( m_SkeletonDC );
    DeleteObject( m_SkeletonBMP );

    if( m_Pen[0] != NULL )
    {
        DeleteObject(m_Pen[0]);
        DeleteObject(m_Pen[1]);
        DeleteObject(m_Pen[2]);
        DeleteObject(m_Pen[3]);
        DeleteObject(m_Pen[4]);
        DeleteObject(m_Pen[5]);
        ZeroMemory(m_Pen,sizeof(m_Pen));
    }

    // Stop the Nui processing thread
    if(m_hEvNuiProcessStop!=NULL)
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if(m_hThNuiProcess!=NULL)
        {
            WaitForSingleObject(m_hThNuiProcess,INFINITE);
            CloseHandle(m_hThNuiProcess);
        }
        CloseHandle(m_hEvNuiProcessStop);
    }

    NuiShutdown( );
    if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }
    m_DrawDepth.DestroyDevice( );
    m_DrawVideo.DestroyDevice( );
}

