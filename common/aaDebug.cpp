#include "aaDebug.h"


int feedFlag            = 0;
static int fno          = 0;
int imageWidth          = 2048;
int imageHeight         = 1080;
GstClockTime timestamp  = 0;

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

static void start_feed (GstElement * pipeline, guint size, void *ptr)
{
 if (feedFlag == 0) {

    feedFlag = 1;

    GstBuffer *m_pgstBuffer;
    GstVideoMeta *m_pgstVideoMeta;
//  cv::Mat img = ((aaDebug *)ptr)->ocvConsumer2EncoderQ.pop();
//  cv::Mat img = ((aaDebug *)ptr)->ocvConsumer2EncoderQ.pop();
    frameBuffer framedata = ((aaDebug *)ptr)->ocvConsumer2EncoderFrameBuffer.pop();

#if 0
    FILE *fp ;
    char fname[256];
    sprintf(fname,"yuvframes/frame-%04d",fno++);
    if ( (fp = fopen(fname,"wb")) == NULL ){
       g_printerr("Unable to open file\n");
       exit(1);
    }
    fwrite(img.data,sizeof(char),2048*1080+1024*540+1024*540, fp);
    fclose(fp);
#endif

    gsize          m_offset[3];
    gint           m_stride[3];
    m_offset[0]    = framedata.nvBuffParams.offset[0];
    m_offset[1]    = framedata.nvBuffParams.offset[1];
    m_offset[2]    = framedata.nvBuffParams.offset[2];
    m_stride[0]    = framedata.nvBuffParams.pitch[0]; 
    m_stride[1]    = framedata.nvBuffParams.pitch[1]; 
    m_stride[2]    = framedata.nvBuffParams.pitch[2];


    int size              = imageWidth * imageHeight * 1.5;
    m_pgstBuffer          = gst_buffer_new_wrapped_full( (GstMemoryFlags)0, (gpointer)(framedata.dataY), size, 0, size, NULL, NULL );
    m_pgstVideoMeta       = gst_buffer_add_video_meta_full(m_pgstBuffer,GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_I420, imageWidth,imageHeight, 3, m_offset, m_stride );

    //ref buffer to give copy to appsrc
    gst_buffer_ref(m_pgstBuffer);

    //GST_BUFFER_DTS(m_pgstBuffer) = 0;
    GST_BUFFER_PTS(m_pgstBuffer) = timestamp;

    GstFlowReturn ret;
    g_signal_emit_by_name (((aaDebug *)ptr)->m_pappsrc, "push-buffer", m_pgstBuffer, &ret);

    if(ret != GST_FLOW_OK)
    {
        g_printerr("could not push buffer\n");
        g_printerr("ret enum: %i\n", ret);
    }

    //dec. ref count so that we can edit data on next run
    gst_buffer_unref(m_pgstBuffer);
    feedFlag = 0;
    timestamp += 33333;
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
 



aaDebug::aaDebug()
{
    GstStateChangeReturn state_ret;

    gst_init(NULL, NULL);

    m_offset[0] = m_offset[1] = m_offset[2] = 0;
    m_stride[0] = 2048; // TBD : FIXME : no magic no
    m_stride[1] = 1024; 
    m_stride[2] = 1024;

    m_ppipeline            = gst_pipeline_new("mypipeline");
    m_pappsrc              = (GstAppSrc*)gst_element_factory_make("appsrc", "aa-appsrc");
    m_pencoder             = gst_element_factory_make("omxh265enc", "aa-videoencoder");
    m_pmux                 = gst_element_factory_make("matroskamux", "aa-mux");
    m_pfsink               = gst_element_factory_make("filesink", "aa-filesink");


    g_assert(m_ppipeline);
    g_assert(m_pappsrc);
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

    g_object_set(m_pappsrc,"format" ,      GST_FORMAT_TIME,NULL);
    g_object_set( G_OBJECT( m_pfsink ),   "location", "test2.mp4", NULL ); 
    g_object_set( G_OBJECT( m_pencoder ), "bitrate", 40000000, NULL ); 
    gst_bin_add_many(GST_BIN(m_ppipeline), (GstElement*)m_pappsrc, m_pencoder, m_pmux, m_pfsink, NULL);

    if(!gst_element_link_many((GstElement*)m_pappsrc, m_pencoder, m_pmux, m_pfsink,NULL)){
       g_warning("failed to link appsrc, autovideoconvert, encoder, muxer, and filesink");
       gst_object_unref (m_ppipeline);
       return ;
    }

  /* go to the PAUSED state and wait for preroll */
  //g_message ("prerolling first frame");
  //gst_element_set_state (m_ppipeline, GST_STATE_PAUSED);
  //gst_element_get_state (m_ppipeline, NULL, NULL, -1);

  gst_element_set_state (m_ppipeline, GST_STATE_PLAYING);
  /* Run event loop listening for bus messages until EOS or ERROR */
  // event_loop (m_ppipeline);
  g_message ("finished constructing pipeline");

}


aaDebug::~aaDebug()
{
  GstStateChangeReturn state_ret;
state_ret = gst_element_set_state((GstElement*)m_ppipeline, GST_STATE_NULL);
 g_warning("set state null returned %d\n", state_ret);


    gst_object_unref(GST_OBJECT(m_ppipeline));
}


