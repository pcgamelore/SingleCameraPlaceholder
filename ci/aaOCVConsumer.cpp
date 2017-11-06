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

#include "aaOCVConsumer.h"
#include "Error.h"

#include <Argus/Argus.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include <sys/time.h>


#include <nvbuf_utils.h>
#include <NvUtils.h>

#include "NvBuffer.h"
#include "NvLogging.h"

#include <cstring>
#include <errno.h>
#include <sys/mman.h>
#include <libv4l2.h>


#include <EGLStream/EGLStream.h> // FrameConsumer
#include <EGLStream/ArgusCaptureMetadata.h> // EGLStream::IArgusCaptureMetadata
#include <EGLStream/NV/ImageNativeBuffer.h> // EGLStream::NV::IImageNativeBuffer


namespace ArgusSamples
{

#if 0
#define AANEWOCVCONSUMER_PRINT(...);    printf("NEW OCV CONSUMER: " __VA_ARGS__);
#else
#define AANEWOCVCONSUMER_PRINT(...);      
#endif


#ifdef ANDROID
#define JPEG_PREFIX "/sdcard/DCIM/Argus_"
#else
#define JPEG_PREFIX "Argus_"
#endif


#define TEST_ERROR_RETURN(cond, _str, ...) \
    do { \
        if (cond) { \
            fprintf(stderr, "[ERROR] %s (%s:%d) : ", __FUNCTION__, __FILE__, __LINE__); \
            fprintf(stderr, _str "\n", ##__VA_ARGS__); \
            return false; \
}} while(0)



void aaNewOCVConsumerThread::init_members(OutputStream *stream, Argus::Size size, NvEglRenderer *renderer)
{
    m_renderer  = renderer;
}
bool aaNewOCVConsumerThread::threadInitialize()
{
    AANEWOCVCONSUMER_PRINT("Done initializing aaNewOCVConsumerThread\n"); 
    m_pDebugObj   = new aaDebug;
    return true;
}

bool aaNewOCVConsumerThread::threadExecute()
{
    int frameCount = 0;
    char *data;
 
    AANEWOCVCONSUMER_PRINT("Starting aaNewOCVConsumer Thread Execution\n");
   
    while (true)
    {
       // Fetch nv_buffer to do format conversion
       AANEWOCVCONSUMER_PRINT("Pop request initiated %d\n",m_pcamCapture2NewOCVConsumerMsgQ->isempty());
       if (!m_pcamCapture2NewOCVConsumerMsgQ->isempty())
          break;
         
       frameBuffer framedata = m_pinputFrameQ->pop();
       int framefd = m_pinputFrameFdQ->pop();
 
       AANEWOCVCONSUMER_PRINT("Pop request granted! Queue Size: %d Camera ID : %d\n", m_pinputFrameQ->getsize() ,m_pCamInfo->camId );

       if (framedata.dataY != NULL) {
          cv::Mat imgY = cv::Mat(FRAME_SIZE_Y ,FRAME_SIZE_X, CV_8UC1, framedata.dataY,PITCH_SIZE_X);
          cv::Mat imgU = cv::Mat(FRAME_SIZE_Y/2 ,FRAME_SIZE_X/2, CV_8UC1, framedata.dataU,(PITCH_SIZE_X/2));
          cv::Mat imgV = cv::Mat(FRAME_SIZE_Y/2 ,FRAME_SIZE_X/2, CV_8UC1, framedata.dataV,(PITCH_SIZE_X/2));

       
          /////////////////// Frame processing (on GPU, and then on CPU)happens here///////////////////  
            

            // push buffers in encoder Q. These will be popped by appsrc element in aaDebug.cpp
            m_pDebugObj->ocvConsumer2EncoderQ.push(imgY);

            // temporarily disable unmap to debug encoding of imgY 
//          munmap(framedata.dataY, PITCH_SIZE_X*FRAME_SIZE_Y);
//          munmap(framedata.dataU, (PITCH_SIZE_X/2)*(FRAME_SIZE_Y/2));
//          munmap(framedata.dataV, (PITCH_SIZE_X/2)*(FRAME_SIZE_Y/2));

	  m_renderer->render(framefd);
  
//	  NvBufferDestroy(framefd);
       }
       AANEWOCVCONSUMER_PRINT("1 Popped frame no %d from Q\n",m_pcamCapture2NewOCVConsumerMsgQ->isempty());
       if (!m_pcamCapture2NewOCVConsumerMsgQ->isempty())
          break;

       AANEWOCVCONSUMER_PRINT("2 Popped frame no %d from Q\n",frameCount++);
    }

    AANEWOCVCONSUMER_PRINT("No more frames. Cleaning up. Camera id : %d\n", m_id);
    PROPAGATE_ERROR(requestShutdown());

    return true;
}

bool aaNewOCVConsumerThread::threadShutdown()
{
    AANEWOCVCONSUMER_PRINT("aaNewOCVConsumer Done.\n");
    delete m_pDebugObj;
    return true;
}



} // namespace ArgusSamples
