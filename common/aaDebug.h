#ifndef AADEBUG_H
#define AADEBUG_H

#include <opencv2/opencv.hpp>


#include "../common/config.h"
#include "../common/Queue.h"

#include "Error.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gst/video/video.h>
//#include <gdk-pixbuf/gdk-pixbuf.h>


#include <Argus/Argus.h>
#include <EGLStream/EGLStream.h>
#include "Thread.h"
#include "EGLGlobal.h"
#include "GLContext.h"
#include <NvEglRenderer.h>
#include "../common/config.h"
#include "../common/Queue.h"




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

class aaDebug 
{

public :
   aaDebug();
   ~aaDebug();
   void pushFrame(cv::Mat & img);
  //VideoWriter m_videoWriter;

//private:
//    GstElement *m_pappsrc;
    GstAppSrc *m_pappsrc;
//    GstPipeline *m_ppipeline;
    GstElement *m_ppipeline;
    GstElement *m_pvideoConvert;
    GstElement *m_pencoder;
    GstElement *m_pmux;
    GstElement *m_pfsink;
  GstVideoInfo m_info;
  GstCaps      *m_pcaps;

  GstBuffer    *m_pgstBuffer;
  GstVideoMeta *m_pgstVideoMeta;
  gsize         m_offset[3];
  gint          m_stride[3];

  Queue<frameBuffer  > ocvConsumer2EncoderFrameBuffer;
  Queue<frameBuffer  > encoder2OcvConsumerFrameBuffer;
  Queue<cv::Mat  >     ocvConsumer2EncoderQ;

};



#endif
