#include <stdio.h>
#include <string>

#include <opencv2/opencv.hpp>
#include "common/config.h"

#include "Error.h"
#include "EGLGlobal.h"
#include "GLContext.h"
#include "aaOCVConsumer.h"
#include "aaCamCapture.h"

#include "Window.h"
#include "Thread.h"

#include <Argus/Argus.h>

#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include <gst/gst.h>

#include <inttypes.h>
#include "../common/Queue.h"
#include "../common/aaDebug.h"
#include "../common/aaCircularBuffer.h"

#include <sys/stat.h>


using namespace cv;
using namespace Argus;
using namespace ArgusSamples;


static uint32_t CAPTURE_TIME                        = 4; // In seconds.
static const uint32_t NUMBER_SESSIONS               = 2;
static const int    DEFAULT_FPS                     = 30;
static const int lastFrameCount                     = CAPTURE_TIME * DEFAULT_FPS;

std::vector<aaEglFrameBuffer> eglFrameBufferList;


namespace ArgusSamples
{
// Constants.
static const ARGUSSIZE   PREVIEW_STREAM_SIZE(640, 480);
static const ARGUSSIZE   OCV_STREAM_SIZE(1920, 1080);
static const int32_t     FRAMERATE                  = DEFAULT_FPS;
static const int32_t     BITRATE                    = 20000000;
static const char*       ENCODER                    = "omxh264enc";
static const char*       MUXER                      = "qtmux";
static const char*       OUTPUT                     = "argus_gstvideoencode_out.mp4";
static const uint32_t    LENGTH                     = 10; // in seconds.


// Globals.
EGLDisplayHolder g_display;



/**
 * Class to initialize and control GStreamer video encoding from an EGLStream.
 */
class GstVideoEncoder
{
public:
    GstVideoEncoder()
        : m_state(GST_STATE_NULL)
        , m_pipeline(NULL)
        , m_videoEncoder(NULL)
    {
    }

    ~GstVideoEncoder()
    {
        shutdown(); 
    }

    /**
     * Initialize the GStreamer video encoder pipeline.
     * @param[in] eglStream The EGLStream to consume frames from.
     * @param[in] resolution The resolution of the video.
     * @param[in] framerate The framerate of the video (in frames per second).
     * @param[in] encoder The encoder to use for encoding. Options include:
     *                    avenc_h263, omxh264enc, omxh265enc, omxvp8enc, avenc_mpeg4
     * @param[in] muxer The muxer/container to use. Options include:
     *                    qtmux (MP4), 3gppmux (3GP), avimux (AVI), identity (H265)
     * @param[in] output The filename/path for the encoded output.
     */
    bool initialize(EGLStreamKHR eglStream, ARGUSSIZE resolution,
                    int32_t framerate, int32_t bitrate,
                    const char* encoder, const char* muxer, const char* output)
    {
        // Initialize GStreamer.
        gst_init(NULL, NULL);

        // Create pipeline.
        m_pipeline = gst_pipeline_new("video_pipeline");
        if (!m_pipeline)
            ORIGINATE_ERROR("Failed to create video pipeline");

        // Create EGLStream video source.
        GstElement *videoSource = gst_element_factory_make("nveglstreamsrc", NULL);
        if (!videoSource)
            ORIGINATE_ERROR("Failed to create EGLStream video source");
        if (!gst_bin_add(GST_BIN(m_pipeline), videoSource))
        {
            gst_object_unref(videoSource);
            ORIGINATE_ERROR("Failed to add video source to pipeline");
        }
        g_object_set(G_OBJECT(videoSource), "display", g_display.get(), NULL);
        g_object_set(G_OBJECT(videoSource), "eglstream", eglStream, NULL);

        // Create queue.
        GstElement *queue = gst_element_factory_make("queue", NULL);
        if (!queue)
            ORIGINATE_ERROR("Failed to create queue");
        if (!gst_bin_add(GST_BIN(m_pipeline), queue))
        {
            gst_object_unref(queue);
            ORIGINATE_ERROR("Failed to add queue to pipeline");
        }

        // Create encoder.
        m_videoEncoder = gst_element_factory_make(encoder, NULL);
        if (!m_videoEncoder)
            ORIGINATE_ERROR("Failed to create video encoder");
        if (!gst_bin_add(GST_BIN(m_pipeline), m_videoEncoder))
        {
            gst_object_unref(m_videoEncoder);
            ORIGINATE_ERROR("Failed to add video encoder to pipeline");
        }
        g_object_set(G_OBJECT(m_videoEncoder), "bitrate", bitrate, NULL);

        // Create muxer.
        GstElement *videoMuxer = gst_element_factory_make(muxer, NULL);
        if (!videoMuxer)
            ORIGINATE_ERROR("Failed to create video muxer");
        if (!gst_bin_add(GST_BIN(m_pipeline), videoMuxer))
        {
            gst_object_unref(videoMuxer);
            ORIGINATE_ERROR("Failed to add video muxer to pipeline");
        }

        // Create file sink.
        GstElement *fileSink = gst_element_factory_make("filesink", NULL);
        if (!fileSink)
            ORIGINATE_ERROR("Failed to create file sink");
        if (!gst_bin_add(GST_BIN(m_pipeline), fileSink))
        {
            gst_object_unref(fileSink);
            ORIGINATE_ERROR("Failed to add file sink to pipeline");
        }
        g_object_set(G_OBJECT(fileSink), "location", output, NULL);

        // Create caps filter to describe EGLStream image format.
        GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                            "format", G_TYPE_STRING, "I420",
                                            "width", G_TYPE_INT, RESOLUTIONW,
                                            "height", G_TYPE_INT, RESOLUTIONH,
                                            "framerate", GST_TYPE_FRACTION, framerate, 1,
                                            NULL);
        if (!caps)
            ORIGINATE_ERROR("Failed to create caps");
        GstCapsFeatures *features = gst_caps_features_new("memory:NVMM", NULL);
        if (!features)
        {
            gst_caps_unref(caps);
            ORIGINATE_ERROR("Failed to create caps feature");
        }
        gst_caps_set_features(caps, 0, features);

        // Link EGLStream source to queue via caps filter.
        if (!gst_element_link_filtered(videoSource, queue, caps))
        {
            gst_caps_unref(caps);
            ORIGINATE_ERROR("Failed to link EGLStream source to queue");
        }
        gst_caps_unref(caps);

        // Link queue to encoder
        if (!gst_element_link(queue, m_videoEncoder))
            ORIGINATE_ERROR("Failed to link queue to encoder");

        // Link encoder to muxer pad.
        if (!gst_element_link_pads(m_videoEncoder, "src", videoMuxer, "video_%u"))
            ORIGINATE_ERROR("Failed to link encoder to muxer pad");

        // Link muxer to sink.
        if (!gst_element_link(videoMuxer, fileSink))
            ORIGINATE_ERROR("Failed to link muxer to sink");

        return true;
    }

    /**
     * Shutdown the GStreamer pipeline.
     */
    void shutdown()
    {
        if (m_state == GST_STATE_PLAYING)
            stopRecording();

        if (m_pipeline)
            gst_object_unref(GST_OBJECT(m_pipeline));
        m_pipeline = NULL;
    }

    /**
     * Start recording video.
     */
    bool startRecording()
    {
        if (!m_pipeline || !m_videoEncoder)
            ORIGINATE_ERROR("Video encoder not initialized");

        if (m_state != GST_STATE_NULL)
            ORIGINATE_ERROR("Video encoder already recording");

        // Start the pipeline.
        if (gst_element_set_state(m_pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
            ORIGINATE_ERROR("Failed to start recording.");

        m_state = GST_STATE_PLAYING;
        return true;
    }

    /**
     * Stop recording video.
     */
    bool stopRecording()
    {
        if (!m_pipeline || !m_videoEncoder)
            ORIGINATE_ERROR("Video encoder not initialized");

        if (m_state != GST_STATE_PLAYING)
            ORIGINATE_ERROR("Video encoder not recording");

        // Send the end-of-stream event.
        GstPad *pad = gst_element_get_static_pad(m_videoEncoder, "sink");
        if (!pad)
            ORIGINATE_ERROR("Failed to get 'sink' pad");
        bool result = gst_pad_send_event(pad, gst_event_new_eos());
        gst_object_unref(pad);
        if (!result)
            ORIGINATE_ERROR("Failed to send end of stream event to encoder");

        // Wait for the event to complete.
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
        if (!bus)
            ORIGINATE_ERROR("Failed to get bus");
        result = gst_bus_poll(bus, GST_MESSAGE_EOS, GST_CLOCK_TIME_NONE);
        gst_object_unref(bus);
        if (!result)
            ORIGINATE_ERROR("Failed to wait for the EOF event");

        // Stop the pipeline.
        if (gst_element_set_state(m_pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE)
            ORIGINATE_ERROR("Failed to stop recording.");

        m_state = GST_STATE_NULL;
        return true;
    }

protected:
    GstState m_state;
    GstElement *m_pipeline;
    GstElement *m_videoEncoder;
};

static int sigint_received = 0;

/* Signal handler for ctrl+c */
static void intHandler(int dummy) {

   /*Enable all termination variables here */
   cout << "Initiating termination\n";
   sigint_received = 1;

}

static sigset_t signal_mask; /* signals to block         */

static void *signal_thread(void *arg) {
   int sig_caught; /* signal caught       */
   int rc;         /* returned code       */

   rc = sigwait(&signal_mask, &sig_caught);
   if (rc != 0) {
      /* handle error */
   }
   switch (sig_caught) {
   case SIGINT: /* process SIGINT  */
      cout << "Initiating termination from SIGINT\n";
      break;
   case SIGTERM: /* process SIGTERM */
      cout << "Initiating termination from SIGTERM\n";
      break;
   default: /* should normally not happen */
      fprintf(stderr, "\nUnexpected signal %d\n", sig_caught);
      break;
   }
}




bool  execute()
{

    using namespace Argus;
    
    // Initialize the preview window and EGL display.    
    Window &window = Window::getInstance(); 
    PROPAGATE_ERROR(g_display.initialize(window.getEGLNativeDisplay()));

    // cntrl c
    signal(SIGINT, intHandler);


    // Initialize the Argus camera provider.
    UniqueObj<CameraProvider> cameraProvider(CameraProvider::create());

    // Get the ICameraProvider interface from the global CameraProvider.
    ICameraProvider *iCameraProvider = interface_cast<ICameraProvider>(cameraProvider);
    if (!iCameraProvider)
        ORIGINATE_ERROR("Failed to get ICameraProvider interface");

    // Get the camera devices.
    std::vector<CameraDevice*> cameraDevices;
    iCameraProvider->getCameraDevices(&cameraDevices);

    if (cameraDevices.size() < 2)
        ORIGINATE_ERROR("Must have at least 2 sensors available");


#if ENABLERECORD
    CameraDevice *cameraDevice = cameraDevices[0];
    ICameraProperties *iCameraProperties = interface_cast<ICameraProperties>(cameraDevice);
    if (!iCameraProperties)
        ORIGINATE_ERROR("Failed to get ICameraProperties interface");
#endif

    // Create the capture session, AutoControl will be based on what the 1st device sees.
    UniqueObj<CaptureSession> captureSessionCamera0(iCameraProvider->createCaptureSession(cameraDevices[0]));
    ICaptureSession *iCaptureSessionCamera0 = interface_cast<ICaptureSession>(captureSessionCamera0);
    if (!iCaptureSessionCamera0)
        ORIGINATE_ERROR("Failed to get capture session interface");


#if  ENABLERECORD
    std::vector<Argus::SensorMode*> sensorModes;

    // Get the sensor mode to determine the video output stream resolution.   
#if ENABLETX1
    iCameraProperties->getSensorModes(&sensorModes);
#else
    iCameraProperties->getAllSensorModes(&sensorModes);
#endif

    if (sensorModes.size() == 0)
	ORIGINATE_ERROR("Failed to get sensor modes");

    ISensorMode *iSensorMode = interface_cast<ISensorMode>(sensorModes[0]);
    if (!iSensorMode)
	ORIGINATE_ERROR("Failed to get sensor mode interface");
#endif


    // Create stream settings object and set settings common to both streams.
    UniqueObj<OutputStreamSettings> streamSettingsCamera0(iCaptureSessionCamera0->createOutputStreamSettings());
    IOutputStreamSettings *iStreamSettingsCamera0 = interface_cast<IOutputStreamSettings>(streamSettingsCamera0);
    if (!iStreamSettingsCamera0)
        ORIGINATE_ERROR("Failed to create OutputStreamSettings");
    iStreamSettingsCamera0->setPixelFormat(PIXEL_FMT_YCbCr_420_888);

#if  ENABLERECORD
    iStreamSettingsCamera0->setEGLDisplay(g_display.get());
#endif
    iStreamSettingsCamera0->setResolution(ARGUSSIZE(FRAME_SIZE_X,FRAME_SIZE_Y));



    // Create egl streams
    iStreamSettingsCamera0->setCameraDevice(cameraDevices[0]);
    UniqueObj<OutputStream> streamLeft(iCaptureSessionCamera0->createOutputStream(streamSettingsCamera0.get()));
    IStream *iStreamLeft = interface_cast<IStream>(streamLeft);
    if (!iStreamLeft)
        ORIGINATE_ERROR("Failed to create left stream");


    UniqueObj<OutputStream> videoStreamCamera0(iCaptureSessionCamera0->createOutputStream(streamSettingsCamera0.get()));
    IStream *iVideoStreamCamera0 = interface_cast<IStream>(videoStreamCamera0);
    if (!iVideoStreamCamera0)
        ORIGINATE_ERROR("Failed to create video stream");


    NvEglRenderer *rendererCamera0 = NULL;
    rendererCamera0 = NvEglRenderer::createEglRenderer("renderer0", 720 , 578  , 0, 0);	 	
    Queue<frameBuffer> inputFrameQ;

    Queue<int   > camCapture2NewOCVConsumerMsgQ;
    aaCircularBuffer<aaEglFrameBuffer>   inputFrameCB(64);

    aaCamCaptureThread aaCamCaptureLeft(streamLeft.get(), rendererCamera0,0,&inputFrameQ,&camCapture2NewOCVConsumerMsgQ,&inputFrameCB,lastFrameCount);
    aaNewOCVConsumerThread aaNewConsumerLeft(streamLeft.get(), rendererCamera0,0,&inputFrameQ,&inputFrameCB,&camCapture2NewOCVConsumerMsgQ);


    PROPAGATE_ERROR(aaCamCaptureLeft.initialize());
    PROPAGATE_ERROR(aaCamCaptureLeft.waitRunning());
    PROPAGATE_ERROR(aaNewConsumerLeft.initialize()); 		

    // Create a request
#if ENABLERECORD
    UniqueObj<Request> requestCamera0(iCaptureSessionCamera0->createRequest(CAPTURE_INTENT_VIDEO_RECORD));
#else  
    UniqueObj<Request> requestCamera0(iCaptureSessionCamera0->createRequest());	
#endif


    IRequest *iRequestCamera0 = interface_cast<IRequest>(requestCamera0);
    if (!iRequestCamera0)
        ORIGINATE_ERROR("Failed to create Request");

#if ENABLERECORD
    if (iRequestCamera0->enableOutputStream(videoStreamCamera0.get()) != STATUS_OK)
        ORIGINATE_ERROR("Failed to enable video stream in Request");
#endif


    // Enable both streams in the request.    
    iRequestCamera0->enableOutputStream(streamLeft.get());
    ISourceSettings *iSourceSettingsCamera0 = interface_cast<ISourceSettings>(iRequestCamera0->getSourceSettings());
    if (!iSourceSettingsCamera0)
        ORIGINATE_ERROR("Failed to get ISourceSettings interface");
    iSourceSettingsCamera0->setFrameDurationRange(Argus::Range<uint64_t>(1e9/DEFAULT_FPS));


#if ENABLERECORD
 // Initialize the GStreamer video encoder consumer.
    GstVideoEncoder gstVideoEncoder;
    string outputFile0 = "imx0.mp4";
    if (!gstVideoEncoder.initialize(iVideoStreamCamera0->getEGLStream(), ARGUSSIZE(FRAME_SIZE_X,FRAME_SIZE_Y) , FRAMERATE, BITRATE, ENCODER, MUXER, outputFile0.c_str()))
	ORIGINATE_ERROR("Failed to initialize GstVideoEncoder EGLStream consumer");

    if (!gstVideoEncoder.startRecording())
        ORIGINATE_ERROR("Failed to start video recording");
#endif


    // Submit capture for the specified time.
    if (iCaptureSessionCamera0->repeat(requestCamera0.get()) != STATUS_OK)
        ORIGINATE_ERROR("Failed to start repeat capture request for preview");

    // wait for cam0 capture to be over
    while (aaCamCaptureLeft.getCamCapture2NewOCVConsumerMsgQSize() == 0);


    // Stop the capture requests and wait until they are complete.
    iCaptureSessionCamera0->stopRepeat();
    iCaptureSessionCamera0->waitForIdle();
    iStreamLeft->disconnect();

    // Disconnect Argus producer from the EGLStreams (and unblock consumer acquire).
#if ENABLERECORD
    // Stop video recording.
    if (!gstVideoEncoder.stopRecording())
        ORIGINATE_ERROR("Failed to stop video recording");
    gstVideoEncoder.shutdown();
#endif


    // Wait for the consumer thread to complete.
    PROPAGATE_ERROR(aaCamCaptureLeft.shutdown());
    PROPAGATE_ERROR(aaNewConsumerLeft.shutdown());

   // Shut down Argus.
   cameraProvider.reset();

   // Cleanup the EGL display
   PROPAGATE_ERROR(g_display.cleanup());

    return true;
}

}; // namespace ends here



int main(int argc, char** argv )
{

 //  signal(SIGINT, intHandler);

//   while (sigint_received == 0) {
//   };



   if (!ArgusSamples::execute())
      return EXIT_FAILURE;
	
   return EXIT_SUCCESS;
}


