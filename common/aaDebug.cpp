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
#include "aaDebug.h"


int feedFlag = 0;
static int fno=0;
int imageWidth = 2048;
int imageHeight = 1080;

static void
event_loop (GstElement * pipe)
{
  GstBus *bus;
  GstMessage *message = NULL;
  gboolean running = TRUE;

  bus = gst_element_get_bus (GST_ELEMENT (pipe));

  while (running) {
    message = gst_bus_timed_pop_filtered (bus, -1, GST_MESSAGE_ANY);

    g_assert (message != NULL);

    switch (message->type) {
      case GST_MESSAGE_EOS:
        g_message ("got EOS");
        running = FALSE;
        break;
      case GST_MESSAGE_WARNING:{
        GError *gerror;
        gchar *debug;

        gst_message_parse_warning (message, &gerror, &debug);
        gst_object_default_error (GST_MESSAGE_SRC (message), gerror, debug);
        g_error_free (gerror);
        g_free (debug);
        break;
      }
      case GST_MESSAGE_ERROR:
      {
        GError *gerror;
        gchar *debug;

        gst_message_parse_error (message, &gerror, &debug);
        gst_object_default_error (GST_MESSAGE_SRC (message), gerror, debug);
        g_error_free (gerror);
        g_free (debug);
        running = FALSE;
        break;
      }
      case GST_MESSAGE_STEP_DONE:
      {
        GstFormat format;
        guint64 amount;
        gdouble rate;
        gboolean flush, intermediate;
        guint64 duration;
        gboolean eos;

        gst_message_parse_step_done (message, &format, &amount, &rate,
            &flush, &intermediate, &duration, &eos);

        if (format == GST_FORMAT_DEFAULT) {
          g_message ("step done: %" GST_TIME_FORMAT " skipped in %"
              G_GUINT64_FORMAT " frames", GST_TIME_ARGS (duration), amount);
        } else {
          g_message ("step done: %" GST_TIME_FORMAT " skipped",
              GST_TIME_ARGS (duration));
        }
        break;
      }
      default:
        break;
    }
    gst_message_unref (message);
  }
  gst_object_unref (bus);
}

#if 0
static gboolean read_data(void *ptr)
{
 GstBuffer *m_pgstBuffer;
 GstVideoMeta *m_pgstVideoMeta;
 cv::Mat img = ocvConsumer2EncoderQ.pop();
  gsize           m_offset[3];
  gint           m_stride[3];
    m_offset[0] = m_offset[1] = m_offset[2] = 0;
    m_stride[0] = 2048; // TBD : FIXME : no magic no
    m_stride[1] = 1024; 
    m_stride[2] = 1024;

    int size              = 1920*1080*1.5;
    m_pgstBuffer          = gst_buffer_new_wrapped_full( (GstMemoryFlags)0, (gpointer)(img.data), size, 0, size, NULL, NULL );
    m_pgstVideoMeta       = gst_buffer_add_video_meta_full(m_pgstBuffer,GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_I420, 1920,1080, 3, m_offset, m_stride );

    //ref buffer to give copy to appsrc
    gst_buffer_ref(m_pgstBuffer);

cout << "push buffer\n";

    GstFlowReturn ret;
    ret                  = gst_app_src_push_buffer(((aaDebug *)ptr)->m_pappsrc, m_pgstBuffer);
    if(ret != GST_FLOW_OK)
    {
        g_printerr("could not push buffer\n");
        g_printerr("ret enum: %i\n", ret);
    }

    //dec. ref count so that we can edit data on next run
    gst_buffer_unref(m_pgstBuffer);


 return TRUE;
}
#endif

static void start_feed (GstElement * pipeline, guint size, void *ptr)
{
cout << "1 Start feeding\n";
 if (feedFlag == 0) {

feedFlag = 1;

 GstBuffer *m_pgstBuffer;
 GstVideoMeta *m_pgstVideoMeta;
// cv::Mat img = ((aaDebug *)ptr)->ocvConsumer2EncoderQ.pop();
 cv::Mat img = ((aaDebug *)ptr)->ocvConsumer2EncoderQ.pop();
// frameBuffer framedata = ((aaDebug *)ptr)->ocvConsumer2EncoderFrameBuffer.pop();
//cv::imshow("img",img);
//cv::waitKey(1);	

  gsize           m_offset[3];
  gint           m_stride[3];
    m_offset[0] = 0;
    m_offset[1] = 2048*1080;
    m_offset[2] = 2048*1080+1024*540;
    m_stride[0] = 2048; // TBD : FIXME : no magic no
    m_stride[1] = 1024; 
    m_stride[2] = 1024;

    int size              = 2048*1080*1.5;
    m_pgstBuffer          = gst_buffer_new_wrapped_full( (GstMemoryFlags)0, (gpointer)(img.data), size, 0, size, NULL, NULL );
    m_pgstVideoMeta       = gst_buffer_add_video_meta_full(m_pgstBuffer,GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_I420, imageWidth,imageHeight, 3, m_offset, m_stride );

    //ref buffer to give copy to appsrc
    gst_buffer_ref(m_pgstBuffer);

    GST_BUFFER_DTS(m_pgstBuffer) = 0;
    GST_BUFFER_PTS(m_pgstBuffer) = 0;

#if 0
{
char filename[250];
sprintf(filename,"junk/frame%04d.jpg",fno++);
FILE *fp = fopen(filename,"wb");
if (NULL == fp){
fprintf(stderr,"Unable to open file %s\n",filename);
exit(1);
}
fwrite(framedata.dataY,sizeof(char),size,fp);
fclose(fp);
}
#endif

    GstFlowReturn ret;
    g_signal_emit_by_name (((aaDebug *)ptr)->m_pappsrc, "push-buffer", m_pgstBuffer, &ret);

//    ret                  = gst_app_src_push_buffer(((aaDebug *)ptr)->m_pappsrc, m_pgstBuffer);
    if(ret != GST_FLOW_OK)
    {
        g_printerr("could not push buffer\n");
        g_printerr("ret enum: %i\n", ret);
    }

    //dec. ref count so that we can edit data on next run
    gst_buffer_unref(m_pgstBuffer);


/*if (ocvConsumer2EncoderFrameBuffer.getsize() > 1){
   frameBuffer fd = ocvConsumer2EncoderFrameBuffer.back();
       munmap(fd.dataY, 2048*1080);
       munmap(fd.dataU, 1024*540);
       munmap(fd.dataV, 1024*540);
}*/


cout << "2 Start feeding\n";


feedFlag = 0;
 }
}

static void stop_feed (GstElement * pipeline, void *app)
{
cout << "Stop feeding\n";
 if (feedFlag != 0) {
  GST_DEBUG ("stop feeding");
  g_source_remove (feedFlag);
  feedFlag = 0;
 }
}
 


#if 0
aaDebug::aaDebug()
{
// this is working
//   m_videoWriter.open("appsrc ! autovideoconvert ! omxh265enc ! matroskamux ! filesink location=test.mkv ", 0, (double)30, cv::Size(1920, 1080), true);

//GstCaps *caps1, *caps2;
//caps1 =
//gst_video_format_new_caps (GST_VIDEO_FORMAT_xRGB, 640, 480, 25, 1, 1, 1);

    GstVideoInfo info;
    GstCaps *caps;

    string aaDebugPipeline          = "appsrc name=myappsrc ! autovideoconvert ! omxh265enc ! matroskamux ! filesink location=test.mkv ";    
    m_ppipeline                        = gst_parse_launch(aaDebugPipeline.c_str(),NULL);
    g_assert (m_ppipeline);

    m_pappsrc                          = gst_bin_get_by_name (GST_BIN(m_ppipeline), "myappsrc");
    g_assert (m_pappsrc);

    m_offset[0] = m_offset[1] = m_offset[2] = 0;
    m_stride[0] = 2048; // TBD : FIXME : no magic no
    m_stride[1] = 1024; 
    m_stride[2] = 1024;

#if 0
    gst_video_info_set_format(&info, GST_VIDEO_FORMAT_I420, 1920, 1080);
    caps                            = gst_video_info_to_caps(&info); 

	caps = gst_caps_new_simple ("video/x-raw",
			"format",G_TYPE_STRING,"I420",
//			"bpp",G_TYPE_INT,12,
//			"depth",G_TYPE_INT,8,
			"width", G_TYPE_INT, 1920,
			"height", G_TYPE_INT, 1080,
			"pitch", G_TYPE_INT, 2048,
			"framerate", GST_TYPE_FRACTION, 30, 1,
			NULL);



    gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
#endif
   //m_pgstBuffer        = gst_buffer_new();
//   m_pgstVideoMeta     = gst_buffer_add_video_meta_full(outbuf,GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_RGB,1920,1080, 3, ostrm->offset, ostrm->stride );
 //  m_pgstVideoMeta     = gst_buffer_add_video_meta_full(m_pgstBuffer,GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_I420, 1920,1080, 3, m_offset, m_stride );
     GstStateChangeReturn state_ret = gst_element_set_state((GstElement*)m_ppipeline, GST_STATE_PLAYING);
     g_warning("set state returned %d\n", state_ret);  

#if 1
   m_videoWriter.open(aaDebugPipeline,0, (double)30, cv::Size(1920, 1080), false);
//   m_videoWriter.open("appsrc ! nvvidconv ! 'video/x-raw, format=(string)I420, framerate=(fraction)30/1'  ! omxh265enc ! matroskamux ! filesink location=test.mkv ", 0, (double)30, cv::Size(1920, 1080), false);
   if (!m_videoWriter.isOpened()) {
      REPORT_ERROR("can't create writer\n");
      exit(1);
   }
#endif
}

#else
aaDebug::aaDebug()
{
    GstStateChangeReturn state_ret;

    gst_init(NULL, NULL);

    m_offset[0] = m_offset[1] = m_offset[2] = 0;
    m_stride[0] = 2048; // TBD : FIXME : no magic no
    m_stride[1] = 1024; 
    m_stride[2] = 1024;

//    m_ppipeline            = (GstPipeline*)gst_pipeline_new("mypipeline");
    m_ppipeline            = gst_pipeline_new("mypipeline");
    m_pappsrc              = (GstAppSrc*)gst_element_factory_make("appsrc", "aa-appsrc");
    m_pvideoConvert        = gst_element_factory_make("autovideoconvert", "aa-videoconvert");
    m_pencoder             = gst_element_factory_make("omxh265enc", "aa-videoencoder");
    m_pmux                 = gst_element_factory_make("matroskamux", "aa-mux");
    m_pfsink               = gst_element_factory_make("filesink", "aa-filesink");


    g_assert(m_ppipeline);
    g_assert(m_pappsrc);
    g_assert(m_pvideoConvert);
    g_assert(m_pencoder);
    g_assert(m_pmux);
    g_assert(m_pfsink);



  m_pcaps = gst_caps_new_simple ("video/x-raw",
			"format",G_TYPE_STRING,"I420",
			"width", G_TYPE_INT, imageWidth,
			"height", G_TYPE_INT, imageHeight,
			"framerate", GST_TYPE_FRACTION, 30, 1,
			NULL);


  /* configure appsrc */
  gst_app_src_set_caps(GST_APP_SRC(m_pappsrc), m_pcaps);
    g_signal_connect(m_pappsrc, "need-data", G_CALLBACK(start_feed), this);
    g_signal_connect(m_pappsrc, "enough-data", G_CALLBACK(stop_feed), this);

    g_object_set(m_pappsrc,"format" , GST_FORMAT_TIME,NULL);
    g_object_set( G_OBJECT( m_pfsink ), "location", "test2.mp4", NULL ); 

    g_object_set( G_OBJECT( m_pencoder ), "bitrate", 40000000, NULL ); 

    gst_bin_add_many(GST_BIN(m_ppipeline), (GstElement*)m_pappsrc, m_pvideoConvert, m_pencoder, m_pmux, m_pfsink, NULL);

#if 1
    if(!gst_element_link_many((GstElement*)m_pappsrc, m_pvideoConvert, m_pencoder, m_pmux, m_pfsink,NULL)){
       g_warning("failed to link appsrc, autovideoconvert, encoder, muxer, and filesink");
       gst_object_unref (m_ppipeline);
       return ;
    }
#else
    if(!gst_element_link((GstElement*)m_pappsrc, m_pvideoConvert)){
       g_warning("failed to link appsrc, autovideoconvert");
    }
    if(!gst_element_link(m_pencoder, m_pmux)){
       g_warning("failed to link encoder and mux");
    }
    if(!gst_element_link(m_pmux,m_pfsink)){
       g_warning("failed to link mux and sink");
    }

#endif

  /* go to the PAUSED state and wait for preroll */
  //g_message ("prerolling first frame");
  //gst_element_set_state (m_ppipeline, GST_STATE_PAUSED);
  //gst_element_get_state (m_ppipeline, NULL, NULL, -1);

  gst_element_set_state (m_ppipeline, GST_STATE_PLAYING);
  /* Run event loop listening for bus messages until EOS or ERROR */
  // event_loop (m_ppipeline);
  g_message ("finished constructing pipeline");

//    state_ret = gst_element_set_state((GstElement*)m_ppipeline, GST_STATE_PLAYING);
//    g_warning("set state returned %d\n", state_ret);

}
#endif


aaDebug::~aaDebug()
{
  GstStateChangeReturn state_ret;
state_ret = gst_element_set_state((GstElement*)m_ppipeline, GST_STATE_NULL);
 g_warning("set state null returned %d\n", state_ret);


    gst_object_unref(GST_OBJECT(m_ppipeline));
}

void aaDebug::pushFrame(cv::Mat & img)
{
cout << "Pushing frame \n";
    int size              = 1920*1080*1.5;
    m_pgstBuffer          = gst_buffer_new_wrapped_full( (GstMemoryFlags)0, (gpointer)(img.data), size, 0, size, NULL, NULL );
    m_pgstVideoMeta       = gst_buffer_add_video_meta_full(m_pgstBuffer,GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_I420, 1920,1080, 3, m_offset, m_stride );

    //ref buffer to give copy to appsrc
    gst_buffer_ref(m_pgstBuffer);


    GstFlowReturn ret;
    ret                  = gst_app_src_push_buffer((GstAppSrc*)m_pappsrc, m_pgstBuffer);
    if(ret != GST_FLOW_OK)
    {
        g_printerr("could not push buffer\n");
        g_printerr("ret enum: %i\n", ret);
    }

    //dec. ref count so that we can edit data on next run
    gst_buffer_unref(m_pgstBuffer);

}

