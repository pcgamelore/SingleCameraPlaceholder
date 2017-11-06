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
