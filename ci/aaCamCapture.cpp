#include "aaCamCapture.h"
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

#include "../common/config.h"

#include <EGLStream/EGLStream.h> // FrameConsumer
#include <EGLStream/ArgusCaptureMetadata.h> // EGLStream::IArgusCaptureMetadata
#include <EGLStream/NV/ImageNativeBuffer.h> // EGLStream::NV::IImageNativeBuffer


extern pthread_mutex_t queue_lockGlobal;
extern pthread_cond_t queue_condGlobal;



namespace ArgusSamples
{

#if 0
#define AACAM_CAPTURE_PRINT(...);    printf("Cam Capture CONSUMER: " __VA_ARGS__);
#else
#define AACAM_CAPTURE_PRINT(...);      
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



void aaCamCaptureThread::init_members(OutputStream *stream, Argus::Size size, NvEglRenderer *renderer)
{
    m_stream    = stream;
    m_framesize = size;
    m_renderer  = renderer;


}
bool aaCamCaptureThread::threadInitialize()
{
    // Create the FrameConsumer.
    m_consumer = UniqueObj<FrameConsumer>(FrameConsumer::create(m_stream));
    if (!m_consumer)
        ORIGINATE_ERROR("Failed to create FrameConsumer");

    /*pCamInfo = new aaCamInfo();
    //pCamInfo->camId = 0;
    pCamInfo->currentFrame = 0;*/


    AACAM_CAPTURE_PRINT("Done Initializing aaCamCaptureThread \n");

    return true;
}

bool aaCamCaptureThread::threadExecute()
{
    IStream *iStream = interface_cast<IStream>(m_stream);
    IFrameConsumer *iFrameConsumer = interface_cast<IFrameConsumer>(m_consumer);
    Argus::Status status;
    int ret;

    // Wait until the producer has connected to the stream.
    AACAM_CAPTURE_PRINT("Waiting until producer is connected...%x\n", m_stream);
    if (iStream->waitUntilConnected() != STATUS_OK)
        ORIGINATE_ERROR("Stream failed to connect.");
    AACAM_CAPTURE_PRINT("Producer has connected; continuing. %x\n", iFrameConsumer);

    int frameCount = 0;
 
    while (m_currentFrame < (m_lastFrameCount-2))
    {
        // Acquire a Frame.
        AACAM_CAPTURE_PRINT("1 Starting frame caputre  %d \n",m_currentFrame);
        UniqueObj<Frame> frame(iFrameConsumer->acquireFrame());
        IFrame *iFrame = interface_cast<IFrame>(frame);
        if (!iFrame)
            break;
        // Get the Frame's Image.
        Image *image = iFrame->getImage();
        EGLStream::NV::IImageNativeBuffer *iImageNativeBuffer
              = interface_cast<EGLStream::NV::IImageNativeBuffer>(image);
        TEST_ERROR_RETURN(!iImageNativeBuffer, "Failed to create an IImageNativeBuffer");
#if ENABLETX1
        int fd = iImageNativeBuffer->createNvBuffer(Argus::Size {FRAME_SIZE_X,FRAME_SIZE_Y},
               NvBufferColorFormat_YUV420, NvBufferLayout_Pitch, &status);
#else
        int fd = iImageNativeBuffer->createNvBuffer(Argus::Size2D<uint32_t> {FRAME_SIZE_X,FRAME_SIZE_Y},
               NvBufferColorFormat_YUV420, NvBufferLayout_Pitch, &status);
#endif
        if (status != STATUS_OK)
               TEST_ERROR_RETURN(status != STATUS_OK, "Failed to create a native buffer");

        NvBufferParams params;
        NvBufferGetParams(fd, &params);

        int fsize = params.pitch[0] * params.height[0] ;
	int fsizeU = params.pitch[1] * params.height[1] ;
	int fsizeV = params.pitch[2] * params.height[2];
   
 	struct timeval tp;
    	gettimeofday(&tp, NULL);
    	long start = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        char *m_datamem  = (char *)mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, params.offset[0]);
	char *m_datamemU = (char *)mmap(NULL, fsizeU, PROT_READ | PROT_WRITE, MAP_SHARED, fd, params.offset[1]);
	char *m_datamemV = (char *)mmap(NULL, fsizeV, PROT_READ | PROT_WRITE, MAP_SHARED, fd, params.offset[2]);
        

        if( (m_datamem == MAP_FAILED) || (m_datamemU == MAP_FAILED) || (m_datamemV == MAP_FAILED) )
           ORIGINATE_ERROR("mmap failed : %s\n", strerror(errno));
	
	frameBuffer framedata;
	
	framedata.dataY = m_datamem;
	framedata.dataU = m_datamemU;
	framedata.dataV = m_datamemV;

        // Push input buffer into q for OCV Consumer 
        m_pinputFrameQ->push(framedata);
	m_pinputFrameFdQ->push(fd);

        AACAM_CAPTURE_PRINT("Pushed frame no %d  Queue Size: %d Camera ID : %d\n",m_currentFrame,m_pinputFrameQ->getsize() ,m_pCamInfo->camId);
	m_currentFrame++;
    }



    // send a msg to aaNewOCVConsumer

    frameBuffer endBuffer;

    endBuffer.dataY = nullptr;
    endBuffer.dataU = nullptr; 
    endBuffer.dataV = nullptr;


    m_pinputFrameQ->push(endBuffer);
    m_pinputFrameFdQ->push(0);
    m_pcamCapture2NewOCVConsumerMsgQ->push(10);

    //m_pinputFrameQ->push(endBuffer);
    //m_pinputFrameFdQ->push(0);
    //m_pcamCapture2NewOCVConsumerMsgQ->push(10);

    //m_pinputFrameQ->push(endBuffer);
    //m_pinputFrameFdQ->push(0);
    //m_pcamCapture2NewOCVConsumerMsgQ->push(10);


    PROPAGATE_ERROR(requestShutdown());

    return true;
}

bool aaCamCaptureThread::threadShutdown()
{
    AACAM_CAPTURE_PRINT("aaCamCaptureObject Done.\n");

    return true;
}

int aaCamCaptureThread::getCamCapture2NewOCVConsumerMsgQSize()
{
    return m_pcamCapture2NewOCVConsumerMsgQ->getsize();
}

} // namespace ArgusSamples
