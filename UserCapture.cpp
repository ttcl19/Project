//this file holds a bunch of methods for capturing (a picture of) the user

#include "stdafx.h"
#include "SkeletalViewer.h"

void CSkeletalViewerApp::CapturePicture()
{
	memcpy(m_CapturedPictures[m_NumCapturedPictures],
		m_videoCache,
		640*480*sizeof(RGBQUAD));

	m_NumCapturedPictures++;

	//TODO save these in files so they can be used later in case of a crash.
}

