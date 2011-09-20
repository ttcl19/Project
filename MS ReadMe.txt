HI
Skeletal Viewer - READ ME 

Copyright (c) Microsoft Corporation. All rights reserved.
This code is licensed under the terms of the Microsoft Kinect for
Windows SDK (Beta) from Microsoft Research License Agreement:
http://research.microsoft.com/KinectSDK-ToU


=============================
OVERVIEW  
.............................
This module provides sample code used to demonstrate Kinect NUI processing such as
capturing depth stream, color video stream and skeletal tracking frames and
displaying them on the screen.
When sample is executed you should be able to see the following:
- the depth stream, showing background in grayscale and different people in different
  colors, darker colors meaning farther distance from camera. Note that people will
  only be detected if their entire body fits within captured frame.
- Tracked NUI skeletons of people detected within frame. Note that NUI skeletons will
  only appear if the entire body of at least one person fits within captured frame.
- Color video stream
- Frame rate at which capture is being delivered to sample application.

NOTE: The image streams will freeze if computer is locked/unlocked, since sample does
      not include the Direct3D code necessary to recover surface lost after user
      pressed Ctrl+Alt+Delete

=============================
SAMPLE LANGUAGE IMPLEMENTATIONS     
.............................
This sample is available in C++


=============================
FILES   
.............................
- DrawDevice.cpp: implementation of DrawDevice helper class used to manage Direct3D device
- DrawDevice.h: declaration of DrawDevice helper class
- NuiImpl.cpp: Implementation of CSkeletalViewerApp methods dealing with NUI processing
- Resource.h: declaration of resource identifiers
- SkeletalViewer.cpp: Application's main function and WndProc
- SkeletalViewer.h: Declaration of CSkeletalViewerApp class
- SkeletalViewer.ico: Application icon used in title bar and task bar
- SkeletalViewer.rc: Declaration of sample application resources
- stdafx.cpp: used to create a pre-compiled header from common includes
- stdafx.h: includes common files needed by sample
- targetver.h: used for windows platform versioning

=============================
BUILDING THE SAMPLE   
.............................

To build the sample using Visual Studio:
-----------------------------------------------------------
1. In Windows Explorer, navigate to the SkeletalViewer\CPP directory.
2. Double-click the icon for the .sln (solution) file to open the file in Visual Studio.
3. In the Build menu, select Build Solution. The application will be built in the default \Debug or \Release directory.


=============================
RUNNING THE SAMPLE   
.............................

To run the sample:
------------------
1. Navigate to the directory that contains the new executable, using the command prompt or Windows Explorer.
2. Type SkeletalViewer.exe at the command line, or double-click the icon for SkeletalViewer.exe to launch it from Windows Explorer.

