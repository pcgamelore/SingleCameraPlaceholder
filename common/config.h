//
//GAMELORE INC CONFIDENTIAL
//Unpublished Copyright (c) 2016-2020 GameLore Inc, All Rights Reserved.
//
//NOTICE:  All information contained herein is, and remains the property of GameLore Inc. The intellectual and technical concepts contained
//herein are proprietary to GameLore Inc and may be covered by U.S. and Foreign Patents, patents in process, and are protected by trade secret or copyright law.
// Dissemination of this information or reproduction of this material is strictly forbidden unless prior written permission is obtained
// from GameLore Inc.  Access to the source code contained herein is hereby forbidden to anyone except current GameLore employees, managers or contractors who have executed 
// Confidentiality and Non-disclosure agreements explicitly covering such access.
//
//The copyright notice above does not evidence any actual or intended publication or disclosure  of  this source code, which includes  
// information that is confidential and/or proprietary, and is a trade secret, of  GameLore Inc.   ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC  PERFORMANCE, 
// OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS  SOURCE CODE  WITHOUT  THE EXPRESS WRITTEN CONSENT OF GAMELORE INC IS STRICTLY PROHIBITED, AND IN VIOLATION OF APPLICABLE 
// LAWS AND INTERNATIONAL TREATIES.  THE RECEIPT OR POSSESSION OF  THIS SOURCE CODE AND/OR RELATED INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS  
// TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE, OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR IN PART.                
//


#ifndef __CONFIGH__
#define __CONFIGH__

#include <string>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <vector>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <sys/time.h>


#include <opencv2/core/core.hpp>


using namespace std;
using namespace cv;

#if 0
#define PRINT(...);    printf("" __VA_ARGS__);
#else
#define PRINT(...);   
#endif

#define ENABLETX1 1
#define ENABLERECORD 1
#define ENABLECUDA 1

#define USEPOSITIONONLY 1

#define FRAME_SIZE_X 1920
#define FRAME_SIZE_Y 1080

#define PITCH_SIZE_X 2048
#define PITCH_SIZE_Y 1024


struct frameBuffer
{
	char *dataY;
	char *dataU;
	char *dataV;
};



#endif
