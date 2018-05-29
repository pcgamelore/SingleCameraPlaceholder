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

#include "../common/config.h"
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

#if 1
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



void allocateEglFrame(aaEglFrameBuffer &fb) {
   EGLImageKHR image;
   image = NvEGLImageFromFd(ArgusSamples::g_display.get(), fb.nvBufParams.dmabuf_fd);
   CUresult status;

   cudaFree(0);
   status = cuGraphicsEGLRegisterImage(&(fb.pResource), image, CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE);
   if (status != CUDA_SUCCESS) {
      printf("cuGraphicsEGLRegisterImage failed: %d, cuda process stop\n", status);
      return;
   }

   status = cuGraphicsResourceGetMappedEglFrame(&(fb.eglFrame), fb.pResource, 0, 0);
   if (status != CUDA_SUCCESS) {
      printf("cuGraphicsSubResourceGetMappedArray failed\n");
   }

   status = cuCtxSynchronize();
   if (status != CUDA_SUCCESS) {
      printf("cuCtxSynchronize failed\n");
   }

   NvDestroyEGLImage(ArgusSamples::g_display.get(), image);
}

void deallocateEglFrame(aaEglFrameBuffer &fb) {
   CUresult status;
   status = cuGraphicsUnregisterResource(fb.pResource);
   if (status != CUDA_SUCCESS) {
      printf("cuGraphicsEGLUnRegisterResource failed: %d\n", status);
   }
}


void aaNewOCVConsumerThread::init_members(OutputStream *stream, ARGUSSIZE size, NvEglRenderer *renderer)
{
    m_renderer  = renderer;
}
bool aaNewOCVConsumerThread::threadInitialize()
{
    AANEWOCVCONSUMER_PRINT("Done initializing aaNewOCVConsumerThread\n"); 
    m_pDebugObj   = NULL;
//    m_pDebugObj   = new aaDebug;
    return true;
}

bool aaNewOCVConsumerThread::threadExecute()
{
    int frameCount = 0;
    char *data;
    volatile int tempCount = 0;
 
    AANEWOCVCONSUMER_PRINT("Starting aaNewOCVConsumer Thread Execution\n");
   
    while (true)
    {
       // Fetch nv_buffer to do format conversion
       AANEWOCVCONSUMER_PRINT("Pop request initiated %d\n",m_pcamCapture2NewOCVConsumerMsgQ->isempty());
       if (!m_pcamCapture2NewOCVConsumerMsgQ->isempty())
          break;
         

       while (m_pinputFrameCB->fullness() <= 5){
          tempCount++;
       }

       //AANEWOCVCONSUMER_PRINT("Done with inputFrameQ wait level is  %d \n", m_pinputFrameQ->getsize() );
       aaEglFrameBuffer framedata = m_pinputFrameCB->pop();
       AANEWOCVCONSUMER_PRINT("Pop request granted! Queue Size: %d \n", m_pinputFrameCB->fullness() );

       if (framedata.dataY != NULL) {
          allocateEglFrame(framedata);
          eglFrameBufferList.push_back(framedata);

          cv::Mat imgY = cv::Mat(FRAME_SIZE_Y ,FRAME_SIZE_X, CV_8UC1, framedata.dataY,PITCH_SIZE_X);
          cv::Mat imgU = cv::Mat(FRAME_SIZE_Y/2 ,FRAME_SIZE_X/2, CV_8UC1, framedata.dataU,(PITCH_SIZE_X/2));
          cv::Mat imgV = cv::Mat(FRAME_SIZE_Y/2 ,FRAME_SIZE_X/2, CV_8UC1, framedata.dataV,(PITCH_SIZE_X/2));

       
         /////////////////// Frame processing (on GPU, and then on CPU)happens here///////////////////  
            

         // push buffers in encoder Q. These will be popped by appsrc element in aaDebug.cpp
         if (m_pDebugObj != NULL) m_pDebugObj->ocvConsumer2EncoderFB1.push(framedata);


         m_renderer->render(framedata.framefd);
         if (m_pinputFrameDelQ.getsize() > DELETE_BUFFER_THRESHOLD) {

             aaEglFrameBuffer framedataDelElement = m_pinputFrameDelQ.pop();
             munmap(framedataDelElement.dataY, framedata.fsizeY);
             munmap(framedataDelElement.dataU, framedata.fsizeU);
             munmap(framedataDelElement.dataV, framedata.fsizeV);

             deallocateEglFrame(framedataDelElement);
             eglFrameBufferList.erase(eglFrameBufferList.begin());

	     NvBufferDestroy(framedataDelElement.framefd);


          }
          m_pinputFrameDelQ.push(framedata);
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
    if (m_pDebugObj != NULL)
    delete m_pDebugObj;
    return true;
}



} // namespace ArgusSamples
