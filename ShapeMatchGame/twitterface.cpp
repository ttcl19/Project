#include "stdafx.h"
#include "SkeletalViewer.h"
#include <iostream>
#include <sstream>

bool PICTURE_TAKING = true;

int CSkeletalViewerApp::TwitterPost(int shape, int orientation, int X1, int Y1, int X2, int Y2) {
	int w = X2 - X1;
	int h = Y2 - Y1;
	
	//printf("TwitterPost shape: %i X1: %i Y1: %i X2: %i Y2: %i\n", shape, X1, Y1, X2, Y2);

	if (!PICTURE_TAKING)
	{
		//printf("Picture taking repressed.\n");
		return 1;
	}

	unsigned char * imageBits =  (unsigned char*)globalAlloc(w*h*3); 
	
	//fill imageBits
	for (int x = X1; x < X2; x++)
	{
		for (int y = Y1; y< Y2; y++)
		{
			
			int bitsIndex = ((x-X1) + (y-Y1)*w)*3;
			int imageIndex = x + y*640;
			imageBits[bitsIndex  ] = m_videoCache[imageIndex].rgbRed;
			imageBits[bitsIndex + 1 ] = m_videoCache[imageIndex].rgbGreen;
			imageBits[bitsIndex + 2 ] = m_videoCache[imageIndex].rgbBlue;
		}
	}

	TweetPicture( shape, orientation, w, h, m_boxHeight, (void*)imageBits); //to C#

	return 0;
}
