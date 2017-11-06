/*
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
    void init_members(OutputStream *stream, Argus::Size size, NvEglRenderer *renderer);
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
    Argus::Size                 m_framesize;
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
