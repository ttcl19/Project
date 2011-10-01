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

	m_p1Index = -1;
	m_p2Index = -1;
	
	srand((UINT) GetTickCount64());
	m_selectedShape = NULL;

	m_timeLimit = 1;
	m_timeLimit = 10;
	m_timeAvailable = 100 * 1000; //HACK make 10 seconds later.
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
			printf("Framerate: %i\n",FrameDelta);
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
	//GetTickCount64 gets the time in milliseconds since the initiation of the program 
	if (GetTickCount64() > m_videoDelay && m_videoDelay != 0) {
		m_videoDelay = 0;
	} else {
		return;
	}
	m_FramesTotal++;
	//The pImageFrame member of NUI_IMAGE_FRAME is of type NuiImageBuffer 
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

//Assigns the colours and effects of each respective case 
RGBQUAD p1Matched		= {0x32, 0xcd, 0x32, 0x00};
RGBQUAD p1Unmatched		= {0xeb, 0xce, 0x87, 0x00};
RGBQUAD p1Out			= {0xf0, 0x20, 0x80, 0x00};
RGBQUAD p1InWrongBox    = {0x47, 0xff, 0xff, 0x00};

RGBQUAD p2Matched		= {0x32, 0xcd, 0x32, 0x00};//{0x82, 0xdd, 0xee, 0x00};
RGBQUAD p2Unmatched		= {0x47, 0x63, 0xff, 0x00};
RGBQUAD p2Out			= {0xb4, 0x69, 0xff, 0x00};
RGBQUAD p2InWrongBox    = {0xeb, 0xce, 0xff, 0x00};

RGBQUAD ignored			= {0xfa, 0xfa, 0xff, 0x00};
//InWrongBox colours are for p2 is in p1's boxes, and vice versa.

RGBQUAD progressBox		= {0xff, 0xff, 0xff, 0x00};


const int FRAMES_FOR_MATCH = 15;
int p1MatchProgress = 0;
int p2MatchProgress = 0;

void CSkeletalViewerApp::Nui_GotDepthAlert( ) //This is the event where most of the interaction happens.
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

		//printf("time remaining: %i\n",m_timeLimit - GetTickCount64());
		//current time
		int timeRemaining = (int)(m_timeLimit - GetTickCount64());
		if (timeRemaining/1000 != lastCountdownMessageSent)
		{
			Countdown(timeRemaining);
			lastCountdownMessageSent = timeRemaining/1000;
		}
		if (GetTickCount64() < m_timeLimit) {
			
			//Copying the array to PlayerRun pointer 
			USHORT * pPlayerRun = m_playerMap;
			for( int y = 0 ; y < 480 ; y++ )
			{
				for( int x = 0 ; x < 640 ; x++ )
				{
					//Zeroing the array 
					*pPlayerRun = 0;
					pPlayerRun++;
				}
			}

			// draw the bits to the bitmap
			RGBQUAD * rgbrun = m_rgbWk;
			USHORT * pBufferRun = (USHORT*) pBuffer;
			USHORT player, depth;

			//Player indices 
			int curP1Index = -1, curP2Index = -1; 
			long colorX = 0, colorY = 0;
			long bigPixel[4];
			long MAX = 640 * 480;

			//Half of the screen 
			for( int y = 0 ; y < 240 ; y++ )
			{
				for( int x = 0 ; x < 320 ; x++ )
				{
					depth = *pBufferRun & 0xfff8;
					player = *pBufferRun & 7;

					//If the pixel is occupied by the player, then it would be 1 
					if (player != 0) 
					{
						if (curP1Index == -1) {
							curP1Index = player;
							//Occurence of the second player 
						} else if (curP2Index == -1 && player != curP1Index) {
							curP2Index = player;
						}

						//Matching colour coordinates with depth 
						//Returned values are directly saved in two of its parameters 
						//(colorx and color y) 
						NuiImageGetColorPixelCoordinatesFromDepthPixel(
							NUI_IMAGE_RESOLUTION_640x480,
							0, x, y, depth, &colorX, &colorY);
							//bigger pixels for the tetris boxes 
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

			// if P1 is not assigned or has dissappeared
			if (m_p1Index == -1 || 
			   (m_p1Index != curP1Index && m_p1Index != curP2Index)) {
				if (curP1Index != -1 && curP1Index != m_p2Index) {
					m_p1Index = curP1Index;
					curP1Index = -1;
				} else if (curP2Index != -1 && curP2Index != m_p2Index) {
					m_p1Index = curP2Index;
					curP2Index = -1;
				}
			}
		
			if (m_p2Index == -1 || 
			   (m_p2Index != curP1Index && m_p2Index != curP2Index)) {
				if (curP1Index != -1 && curP1Index != m_p1Index) {
					m_p2Index = curP1Index;
					curP1Index = -1;
				} else if (curP2Index != -1 && curP2Index != m_p1Index) {
					m_p2Index = curP2Index;
					curP2Index = -1;
				}
			}
		
			//copy raw video to var for drawing effects (like tetris boxes) on top.
			memcpy(m_videoEffects,m_videoCache,640*480*4);

			//tetris box drawing
			bool p1Passed = true, p2Passed = true;
			int p1scoreCount = 0, p2scoreCount = 0;

			int gap = 640 - m_boxWidth * m_numHBox;
			int startX, startY;
			UINT p1Score, p2Score;
			UINT limit = 0.3 * m_boxHeight * m_boxWidth;

			//this loop iterates through each of the boxes in the Tetris grid, calculating scores and drawing.
			for (UINT i = 0; i < m_numHBox * m_numVBox; i++) {
				startX = (i % m_numHBox) * m_boxWidth + gap / 2;			
				startY = (i / m_numHBox) * m_boxHeight + y_box_offset;
				pPlayerRun = m_playerMap + startY * 640 + startX;

				//Calculates the scores 
				p1Score = 0;
				p2Score = 0;
				//m_p1Index was initialized to be -1 
				//If pPlayerRun == m_p1Index then the player occupies that pixel 
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

				//colours the box in if the player occupies the threshold score 
				//m_selectedshape is defined in the header file(SkeletalViewer.h)
				//and it contains information about the selected shape, where it was
				//coloured and such 
				//the if statements here basically checks if there exists a number(an 
				//indicator for the colour the box needs to be coloured in) and colours
				//them appropriately
				//m_selectedshape points to an array containing the bitmap of the desire of 
				//the desired shape with 0, 1 and 2. etc. 
				if (m_selectedShape == NULL || 
					m_selectedShape[i] == 0) {
					if (p1Score >= limit) {
						p1Passed = false;
						drawBox(i, &p1Out, 0.5);
					} else if (p2Score >= limit) {
						p2Passed = false;
						drawBox(i, &p2Out, 0.5);
					} else {
						drawBox(i, &ignored, 0.25);
					}
				} else if (m_selectedShape[i] == 1) {
					if (p1Score < limit) {
						p1Passed = false;
						if (p2Score> limit) {
							drawBox(i, &p2InWrongBox, 0.5);						
						} else {
							drawBox(i, &p1Unmatched, 0.5);
						}
					} else {
						drawBox(i, &p1Matched, 0.4);
						p1scoreCount++;
					}
				} else if (m_selectedShape[i] == 2) {
					if (p2Score < limit) {
						p2Passed = false;
						if (p1Score> limit) {
							drawBox(i, &p1InWrongBox, 0.5);						
						} else {
							drawBox(i, &p2Unmatched, 0.5);
						}
					} else {
						drawBox(i, &p2Matched, 0.4);
						p2scoreCount++;
					}
				}
			}

			ShapeStatus(p1scoreCount,p2scoreCount); //OSC

			if (p1Passed) {
				if (p1MatchProgress == 0)
					Holding(1); //first time holding
				p1MatchProgress++;
			} else {
				if (p1MatchProgress > 0)
					HoldFail(1); //first time failing
				p1MatchProgress = 0;
			}
			drawRect(x_box_offset,x_box_offset + m_boxWidth*3*p1MatchProgress/FRAMES_FOR_MATCH,
				y_box_offset-40,y_box_offset,&progressBox,1.0);

			if (p2Passed) {
				if (p2MatchProgress == 0)
					Holding(2); //first time holding
				p2MatchProgress++;
			} else {
				if (p2MatchProgress > 0)
					HoldFail(2); //first time failing
				p2MatchProgress = 0;
			}
			drawRect(x_box_offset+ m_boxWidth*3,x_box_offset + m_boxWidth*3 + m_boxWidth*3*p2MatchProgress/FRAMES_FOR_MATCH,
				y_box_offset-40,y_box_offset,&progressBox,1.0);

			bool p1Win = p1MatchProgress >= FRAMES_FOR_MATCH;
			bool p2Win = p2MatchProgress >= FRAMES_FOR_MATCH;

			if (ShapeIndex > 0) { //if shape is being displayed.

				if (p1Win || p2Win) //Someone won!
				{
					if (p1Win && p2Win) {
						bool chooseP1 = (rand() % 2 == 0);
						p1Win = chooseP1;
						p2Win = !chooseP1;
					}

					int pWinNum = p1Win ? 1 : 2;

					ShapeCompleted(pWinNum);

					TwitterPost(ShapeIndex, ori, Shapes::X1(ShapeIndex,ori,pWinNum)*m_boxWidth +x_box_offset, 
						Shapes::Y1(ShapeIndex,ori,pWinNum)*m_boxHeight+y_box_offset, 
						Shapes::X2(ShapeIndex,ori,pWinNum)*m_boxWidth+x_box_offset,
						Shapes::Y2(ShapeIndex,ori,pWinNum)*m_boxHeight+y_box_offset);

					//image captured - hide shape.

					//TODO "flash" effect or something awesome.

					//HACK m_timeLimit = 0; //This line would end gameplay if you wanted.

					// new shape
					newRandomShape();
				}
			}
			m_DrawVideo.DrawFullRect( (BYTE*) m_videoEffects );

			m_DrawVideo.FinishedDrawThisFrame(); //expect no more video calls.
		
		} else {
			//out of time
			if (m_timeLimit != 0)
			{
				Timeout();
				m_timeLimit = 0;
			}
			m_DrawVideo.DrawFrame( (BYTE*) m_videoCache);
		}

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