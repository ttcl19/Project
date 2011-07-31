/************************************************************************
*                                                                       *
*   NuiImpl.cpp -- Implementation of CSkeletalViewerApp methods dealing *
*                  with NUI processing                                  *
*                                                                       *
* Copyright (c) Microsoft Corp. All rights reserved.                    *
*                                                                       *
* This code is licensed under the terms of the                          *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research       *
* License Agreement: http://research.microsoft.com/KinectSDK-ToU        *
*                                                                       *
************************************************************************/

#include "stdafx.h"
#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>

static const COLORREF g_JointColorTable[NUI_SKELETON_POSITION_COUNT] = 
{
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_HIP_CENTER
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_SPINE
    RGB(168, 230, 29), // NUI_SKELETON_POSITION_SHOULDER_CENTER
    RGB(200, 0,   0), // NUI_SKELETON_POSITION_HEAD
    RGB(79,  84,  33), // NUI_SKELETON_POSITION_SHOULDER_LEFT
    RGB(84,  33,  42), // NUI_SKELETON_POSITION_ELBOW_LEFT
    RGB(255, 126, 0), // NUI_SKELETON_POSITION_WRIST_LEFT
    RGB(215,  86, 0), // NUI_SKELETON_POSITION_HAND_LEFT
    RGB(33,  79,  84), // NUI_SKELETON_POSITION_SHOULDER_RIGHT
    RGB(33,  33,  84), // NUI_SKELETON_POSITION_ELBOW_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_WRIST_RIGHT
    RGB(37,   69, 243), // NUI_SKELETON_POSITION_HAND_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_HIP_LEFT
    RGB(69,  33,  84), // NUI_SKELETON_POSITION_KNEE_LEFT
    RGB(229, 170, 122), // NUI_SKELETON_POSITION_ANKLE_LEFT
    RGB(255, 126, 0), // NUI_SKELETON_POSITION_FOOT_LEFT
    RGB(181, 165, 213), // NUI_SKELETON_POSITION_HIP_RIGHT
    RGB(71, 222,  76), // NUI_SKELETON_POSITION_KNEE_RIGHT
    RGB(245, 228, 156), // NUI_SKELETON_POSITION_ANKLE_RIGHT
    RGB(77,  109, 243) // NUI_SKELETON_POSITION_FOOT_RIGHT
};




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
	m_numHBox = 6;
	m_numVBox = 4;
	m_boxHeight = 100;
	m_boxWidth = 100;
	m_offset = 80;

	m_p1Index = 0;
	m_p2Index = 0;

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



DWORD WINAPI CSkeletalViewerApp::Nui_ProcessThread(LPVOID pParam)
{
    CSkeletalViewerApp *pthis=(CSkeletalViewerApp *) pParam;
    HANDLE                hEvents[4];
    int                    nEventIdx,t,dt;

    // Configure events to be listened on
    hEvents[0]=pthis->m_hEvNuiProcessStop;
    hEvents[1]=pthis->m_hNextDepthFrameEvent;
    hEvents[2]=pthis->m_hNextVideoFrameEvent;
    hEvents[3]=pthis->m_hNextSkeletonEvent;

    // Main thread loop
    while(1)
    {
        // Wait for an event to be signalled
        nEventIdx=WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

        // If the stop event, stop looping and exit
        if(nEventIdx==0)
            break;            

        // Perform FPS processing
        t = timeGetTime( );
        if( pthis->m_LastFPStime == -1 )
        {
            pthis->m_LastFPStime = t;
            pthis->m_LastFramesTotal = pthis->m_FramesTotal;
        }
        dt = t - pthis->m_LastFPStime;
        if( dt > 1000 )
        {
            pthis->m_LastFPStime = t;
            int FrameDelta = pthis->m_FramesTotal - pthis->m_LastFramesTotal;
            pthis->m_LastFramesTotal = pthis->m_FramesTotal;
            SetDlgItemInt( pthis->m_hWnd, IDC_FPS, FrameDelta,FALSE );
        }

        // Perform skeletal panel blanking
        if( pthis->m_LastSkeletonFoundTime == -1 )
            pthis->m_LastSkeletonFoundTime = t;
        dt = t - pthis->m_LastSkeletonFoundTime;
        if( dt > 250 )
        {
            if( !pthis->m_bScreenBlanked )
            {
                pthis->Nui_BlankSkeletonScreen( GetDlgItem( pthis->m_hWnd, IDC_SKELETALVIEW ) );
                pthis->m_bScreenBlanked = true;
            }
        }

        // Process signal events
        switch(nEventIdx)
        {
            case 1:
                pthis->Nui_GotDepthAlert();
                //pthis->m_FramesTotal++;
                break;

            case 2:
                pthis->Nui_GotVideoAlert();
                break;

            case 3:
                pthis->Nui_GotSkeletonAlert( );
                break;
        }
    }

    return (0);
}

void CSkeletalViewerApp::Nui_GotVideoAlert( )
{

	if (GetTickCount64() > m_videoDelay && m_videoDelay != 0) {
		m_videoDelay = 0;
	} else {
		return;
	}
	m_FramesTotal++;
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pVideoStreamHandle,
        0,
        &pImageFrame );
    if( FAILED( hr ) )
    {
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {

			BYTE * pBuffer = (BYTE*) LockedRect.pBits;	
			RGBQUAD * pBufferRun = (RGBQUAD*) pBuffer;
			RGBQUAD * pVideoRun = m_videoCache;

			USHORT * pPlayerRun = m_playerMap;
			for( int y = 0 ; y < 480 ; y++ )
			{
				for( int x = 0 ; x < 640 ; x++ )
				{
					//copy kinect video buffer into local.
					*pVideoRun = *pBufferRun;
					pVideoRun++;
					pBufferRun++;
				}
			}
    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}


void CSkeletalViewerApp::Nui_GotDepthAlert( )
{
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        &pImageFrame );

    if( FAILED( hr ) )
    {
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );

    if( LockedRect.Pitch != 0 )
    {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;

		USHORT * pPlayerRun = m_playerMap;
        for( int y = 0 ; y < 480 ; y++ )
        {
            for( int x = 0 ; x < 640 ; x++ )
            {
				*pPlayerRun = 0;
				pPlayerRun++;
			}
		}

        // draw the bits to the bitmap
        RGBQUAD * rgbrun = m_rgbWk;
        USHORT * pBufferRun = (USHORT*) pBuffer;
		USHORT player, depth;
		UINT curP1Index = 0, curP2Index = 0; 
		long colorX = 0, colorY = 0;
		long bigPixel[4];
		long MAX = 640 * 480;
        for( int y = 0 ; y < 240 ; y++ )
        {
            for( int x = 0 ; x < 320 ; x++ )
            {
				depth = *pBufferRun & 0xfff8;
				player = *pBufferRun & 7;

				if (player != 0) 
				{
					if (curP1Index == 0) {
						curP1Index = player;
					} else if (curP2Index == 0) {
						curP2Index = player;
					}

					NuiImageGetColorPixelCoordinatesFromDepthPixel(
						NUI_IMAGE_RESOLUTION_640x480,
						0, x, y, depth, &colorX, &colorY);

						bigPixel[0] = colorY * 640 + colorX;
						bigPixel[1] = bigPixel[0] + 1;
						bigPixel[2] = bigPixel[0] + 640;
						bigPixel[3] = bigPixel[2] + 1;

						for (int i = 0; i < 4; i++) {
							if (bigPixel[i] > MAX) {
								break;
							}
							m_playerMap[bigPixel[i]] = player;
						}
				}

                RGBQUAD quad = Nui_ShortToQuad_Depth( *pBufferRun );
                pBufferRun++;
                *rgbrun = quad;
                rgbrun++;
            }
        }

		// if P1 is not assigned and has dissappeared
		if (m_p1Index == 0 || 
		   (m_p1Index != curP1Index && m_p1Index != curP2Index)) {
			if (curP1Index != 0 && curP1Index != m_p2Index) {
				m_p1Index = curP1Index;
				curP1Index = 0;
			} else if (curP2Index != 0 && curP2Index != m_p2Index) {
				m_p1Index = curP2Index;
				curP2Index = 0;
			}
		}
		
		if (m_p2Index == 0 || 
		   (m_p2Index != curP1Index && m_p2Index != curP2Index)) {
			if (curP1Index != 0 && curP1Index != m_p1Index) {
				m_p2Index = curP1Index;
				curP1Index = 0;
			} else if (curP2Index != 0 && curP2Index != m_p1Index) {
				m_p2Index = curP2Index;
				curP2Index = 0;
			}
		}
		//copy raw video to var for drawing effects (like tetris boxes) on top.
		memcpy(m_videoEffects,m_videoCache,640*480*4);

		//tetris box drawing
		UINT shapeTemplate[24] = {
			0, 0, 0, 0, 0, 0,
			0, 0, 1, 0, 2, 2, 
			0, 1, 1, 0, 2, 0, 
			0, 0, 1, 0, 2, 0
		};
		
		
		RGBQUAD p1Matched		= {0x2f, 0xff, 0xad, 0x00};
		RGBQUAD p1Unmatched		= {0xeb, 0xce, 0x87, 0x00};
		RGBQUAD p1Out			= {0xf0, 0x20, 0x80, 0x00};
		RGBQUAD p2Matched		= {0x82, 0xdd, 0xee, 0x00};
		RGBQUAD p2Unmatched		= {0x47, 0x63, 0xff, 0x00};
		RGBQUAD p2Out			= {0xb4, 0x69, 0xff, 0x00};
		RGBQUAD ignored			= {0xfa, 0xfa, 0xff, 0x00};
		bool p1Passed = true, p2Passed = true;

		int gap = 640 - m_boxWidth * m_numHBox;
		int startX, startY;
		UINT p1Score, p2Score;
		UINT limit = 0.3 * m_boxHeight * m_boxWidth;

		for (UINT i = 0; i < m_numHBox * m_numVBox; i++) {
			startX = (i % m_numHBox) * m_boxWidth + gap / 2;			
			startY = (i / m_numHBox) * m_boxHeight + m_offset;
			pPlayerRun = m_playerMap + startY * 640 + startX;

			p1Score = 0;
			p2Score = 0;
			for (UINT j = 0; j < m_boxHeight; j++) {
				for (UINT k = 0; k < m_boxWidth; k++) {
					if (*pPlayerRun == m_p1Index) {
						p1Score++;
					}

					if (*pPlayerRun == m_p2Index) {
						p2Score++;
					}
					pPlayerRun++;
				}

				pPlayerRun += m_boxWidth * (m_numHBox - 1) + gap;
			}

			if (shapeTemplate[i] == 0) {
				if (p1Score >= limit) {
					p1Passed = false;
					drawBox(i, &p1Out, 0.5);
				} else if (p2Score >= limit) {
					p2Passed = false;
					drawBox(i, &p2Out, 0.5);
				} else {
					drawBox(i, &ignored, 0.25);
				}
			} else if (shapeTemplate[i] == 1) {
				if (p1Score < limit) {
					p1Passed = false;
					drawBox(i, &p1Unmatched, 0.5);
				} else {
					drawBox(i, &p1Matched, 0.25);
				}
			} else if (shapeTemplate[i] == 2) {
				if (p2Score < limit) {
					p2Passed = false;
					drawBox(i, &p2Unmatched, 0.5);
				} else {
					drawBox(i, &p2Matched, 0.25);
				}
			}
		}

		m_DrawVideo.DrawFullRect( (BYTE*) m_videoEffects );

		//picture-in-picture
		double PIPscale = 0.3;
		RECT PIPRect;
		//below random-seeming w/h values are based on how big IDC_VIDEO_VIEW is drawn.
		int width = 548;//640;
		int height = 455;//480;

		//draw RGB Captured
		double CapturedScale = 0.2;
		for (int c = 0; c < m_NumCapturedPictures; c++)
		{
			RECT CapturedRect;
			CapturedRect.left = c*CapturedScale*width;
			CapturedRect.right = (c+1)*CapturedScale*width;
			CapturedRect.top = (long)(height - height*CapturedScale);
			CapturedRect.bottom = height;

			m_DrawVideo.DrawRect( (BYTE*) m_CapturedPictures[c], &CapturedRect);
		}

		m_DrawVideo.FinishedDrawThisFrame(); //expect no more video calls.


        m_DrawDepth.DrawFrame( (BYTE*) m_rgbWk );

		ULONGLONG diff = (ULONGLONG) ((1.0 / m_FramesTotal - 1.0 / 30) * 1000);
		diff = 0;
		m_videoDelay = GetTickCount64() + diff;
    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );
}

void CSkeletalViewerApp::Nui_BlankSkeletonScreen(HWND hWnd)
{
    HDC hdc = GetDC( hWnd );
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;
    PatBlt( hdc, 0, 0, width, height, BLACKNESS );
    ReleaseDC( hWnd, hdc );
}

void CSkeletalViewerApp::Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int numJoints, ... )
{
    va_list vl;
    va_start(vl,numJoints);
    POINT segmentPositions[NUI_SKELETON_POSITION_COUNT];

    for (int iJoint = 0; iJoint < numJoints; iJoint++)
    {
        NUI_SKELETON_POSITION_INDEX jointIndex = va_arg(vl,NUI_SKELETON_POSITION_INDEX);
        segmentPositions[iJoint].x = m_Points[jointIndex].x;
        segmentPositions[iJoint].y = m_Points[jointIndex].y;
    }

    Polyline(m_SkeletonDC, segmentPositions, numJoints);

    va_end(vl);
}

void CSkeletalViewerApp::Nui_DrawSkeleton( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
{
    HGDIOBJ hOldObj = SelectObject(m_SkeletonDC,m_Pen[WhichSkeletonColor % m_PensTotal]);
    
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;

    if( m_Pen[0] == NULL )
    {
        m_Pen[0] = CreatePen( PS_SOLID, width / 80, RGB(255, 0, 0) );
        m_Pen[1] = CreatePen( PS_SOLID, width / 80, RGB( 0, 255, 0 ) );
        m_Pen[2] = CreatePen( PS_SOLID, width / 80, RGB( 64, 255, 255 ) );
        m_Pen[3] = CreatePen( PS_SOLID, width / 80, RGB(255, 255, 64 ) );
        m_Pen[4] = CreatePen( PS_SOLID, width / 80, RGB( 255, 64, 255 ) );
        m_Pen[5] = CreatePen( PS_SOLID, width / 80, RGB( 128, 128, 255 ) );
    }

    if( bBlank )
    {
        PatBlt( m_SkeletonDC, 0, 0, width, height, BLACKNESS );
    }

    int scaleX = width; //scaling up to image coordinates
    int scaleY = height;
    float fx=0,fy=0;
    int i;
    for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
    {
        NuiTransformSkeletonToDepthImageF( pSkel->SkeletonPositions[i], &fx, &fy );
        m_Points[i].x = (int) ( fx * scaleX + 0.5f );
        m_Points[i].y = (int) ( fy * scaleY + 0.5f );
    }

    SelectObject(m_SkeletonDC,m_Pen[WhichSkeletonColor%m_PensTotal]);
    
    Nui_DrawSkeletonSegment(pSkel,4,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);
    
    // Draw the joints in a different color
    for (i = 0; i < NUI_SKELETON_POSITION_COUNT ; i++)
    {
        HPEN hJointPen;
        
        hJointPen=CreatePen(PS_SOLID,9, g_JointColorTable[i]);
        hOldObj=SelectObject(m_SkeletonDC,hJointPen);

        MoveToEx( m_SkeletonDC, m_Points[i].x, m_Points[i].y, NULL );
        LineTo( m_SkeletonDC, m_Points[i].x, m_Points[i].y );

        SelectObject( m_SkeletonDC, hOldObj );
        DeleteObject(hJointPen);
    }

    return;

}

void CSkeletalViewerApp::drawBox(int boxIndex, RGBQUAD * color, double opacity) {
	int gap = 640 - m_boxWidth * m_numHBox;
	int startX, startY;
	UINT borderWidth = 1;
	RGBQUAD borderColor = {0x00, 0x00, 0x00, 0x00};

	startX = (boxIndex % m_numHBox) * m_boxWidth + gap / 2;			
	startY = (boxIndex / m_numHBox) * m_boxHeight + m_offset;
	RGBQUAD * pixel = m_videoEffects + startY * 640 + startX;
	// top border
	for (UINT j = 0; j < borderWidth; j++) {
		for (UINT k = 0; k < m_boxWidth; k++) {
			*pixel = borderColor;
			pixel++;
		}
		pixel += m_boxWidth * (m_numHBox - 1) + gap;
	}

	for (UINT j = 0; j < m_boxHeight - borderWidth * 2; j++) {
		// left border
		for (UINT k = 0; k < borderWidth; k++) {
			*pixel = borderColor;
			pixel++;
		}

		for (UINT k = 0; k < m_boxWidth - borderWidth * 2; k++) {
			pixel->rgbBlue = (BYTE) color->rgbBlue * opacity + pixel->rgbBlue * (1 - opacity);
			pixel->rgbGreen = (BYTE) color->rgbGreen * opacity + pixel->rgbGreen * (1 - opacity);
			pixel->rgbRed = (BYTE) color->rgbRed * opacity + pixel->rgbRed * (1 - opacity);

			pixel++;
		}

		// right border
		for (UINT k = 0; k < borderWidth; k++) {
			*pixel = borderColor;
			pixel++;
		}

		pixel += m_boxWidth * (m_numHBox - 1) + gap;
	}

	// bottom border
	for (UINT j = 0; j < borderWidth; j++) {
		for (UINT k = 0; k < m_boxWidth; k++) {
			*pixel = borderColor;
			pixel++;
		}
		pixel += m_boxWidth * (m_numHBox - 1) + gap;
	}
}

void CSkeletalViewerApp::Nui_DoDoubleBuffer(HWND hWnd,HDC hDC)
{
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;

    HDC hdc = GetDC( hWnd );

    BitBlt( hdc, 0, 0, width, height, hDC, 0, 0, SRCCOPY );

    ReleaseDC( hWnd, hdc );

}

void CSkeletalViewerApp::Nui_GotSkeletonAlert( )
{
    NUI_SKELETON_FRAME SkeletonFrame;

    HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );

    bool bFoundSkeleton = true;
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
        {
            bFoundSkeleton = false;
        }
    }

    // no skeletons!
    //
    if( bFoundSkeleton )
    {
        return;
    }

    // smooth out the skeleton data
    NuiTransformSmooth(&SkeletonFrame,NULL);

    // we found a skeleton, re-start the timer
    m_bScreenBlanked = false;
    m_LastSkeletonFoundTime = -1;

    // draw each skeleton color according to the slot within they are found.
    //
    bool bBlank = true;
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )
        {
            Nui_DrawSkeleton( bBlank, &SkeletonFrame.SkeletonData[i], GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), i );
            bBlank = false;
        }
    }

    Nui_DoDoubleBuffer(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), m_SkeletonDC);
}

RGBQUAD CSkeletalViewerApp::Nui_ShortToQuad_Depth( USHORT s )
{
    USHORT RealDepth = (s & 0xfff8) >> 3;
    USHORT Player = s & 7;

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

    RGBQUAD q;
    q.rgbRed = q.rgbBlue = q.rgbGreen = 0;

    switch( Player )
    {
    case 0:
        q.rgbRed = l / 2;
        q.rgbBlue = l / 2;
        q.rgbGreen = l / 2;
        break;
    case 1:
        q.rgbRed = l;
        break;
    case 2:
        q.rgbGreen = l;
        break;
    case 3:
        q.rgbRed = l / 4;
        q.rgbGreen = l;
        q.rgbBlue = l;
        break;
    case 4:
        q.rgbRed = l;
        q.rgbGreen = l;
        q.rgbBlue = l / 4;
        break;
    case 5:
        q.rgbRed = l;
        q.rgbGreen = l / 4;
        q.rgbBlue = l;
        break;
    case 6:
        q.rgbRed = l / 2;
        q.rgbGreen = l / 2;
        q.rgbBlue = l;
        break;
    case 7:
        q.rgbRed = 255 - ( l / 2 );
        q.rgbGreen = 255 - ( l / 2 );
        q.rgbBlue = 255 - ( l / 2 );
    }

    return q;
}