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
#include "nvbuf_utils.h"

using namespace std;
using namespace cv;

#if 0
#define PRINT(...);    printf("" __VA_ARGS__);
#else
#define PRINT(...);   
#endif

#define ENABLETX1 1
#define ENABLERECORD 0
#define ENABLECUDA 0

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

    NvBufferParams nvBuffParams;

};



#endif
