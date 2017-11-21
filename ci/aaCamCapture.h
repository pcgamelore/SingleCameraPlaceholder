#ifndef AACAMCAPTURE_H
#define AACAMCAPTURE_H

#include <Argus/Argus.h>
#include <EGLStream/EGLStream.h>
#include "Thread.h"
#include "EGLGlobal.h"
#include "GLContext.h"
#include <NvEglRenderer.h>
#include "../common/Queue.h"
#include "../common/config.h"

namespace ArgusSamples
{

using namespace Argus;
using namespace EGLStream;

/*******************************************************************************
 * OCV Consumer thread:
 *   Creates a FrameConsumer object to consume frames from a stream,
 *   to perform open cv processing
 ******************************************************************************/


class aaCamCaptureThread : public Thread
{
public:

    explicit aaCamCaptureThread()
    {
    }

    explicit aaCamCaptureThread(OutputStream *stream,  NvEglRenderer *renderer, int id , Queue<frameBuffer> *q, Queue<int> * fdq, Queue<int> *msgq, int lastFrame)
        : m_stream(stream),  m_renderer(renderer), m_id(id) , m_pinputFrameQ(q), m_pinputFrameFdQ(fdq), m_pcamCapture2NewOCVConsumerMsgQ(msgq), m_lastFrameCount(lastFrame)
    {
	m_currentFrame = 0;
    }



    ~aaCamCaptureThread()
    {
    }
    void init_members(OutputStream *stream, ARGUSSIZE size, NvEglRenderer *renderer);
    int getCamCapture2NewOCVConsumerMsgQSize();

    //aaCamera   *m_pCamera;

private:


    /** @name Thread methods */
    /**@{*/
    virtual bool threadInitialize();
    virtual bool threadExecute();
    virtual bool threadShutdown();
    /**@}*/

    OutputStream               *m_stream;
    UniqueObj<FrameConsumer>    m_consumer;
    ARGUSSIZE                   m_framesize;
    NvEglRenderer              *m_renderer;
    int m_id;

    Queue<frameBuffer>         *m_pinputFrameQ;
    Queue<int   >              *m_pinputFrameFdQ;
    Queue<int   >              *m_pcamCapture2NewOCVConsumerMsgQ;
    int                         m_lastFrameCount;
    int                         m_currentFrame;	

};



} // namespace ArgusSamples

#endif // OCVCONSUMER
