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

#ifndef AAOCVCONSUMER_H
#define AAOCVCONSUMER_H

#include <Argus/Argus.h>
#include <EGLStream/EGLStream.h>
#include "Thread.h"
#include "EGLGlobal.h"
#include "GLContext.h"
#include <NvEglRenderer.h>
#include "../common/config.h"
#include "../common/Queue.h"
#include "../common/aaDebug.h"



namespace ArgusSamples
{

using namespace Argus;
using namespace EGLStream;


/*******************************************************************************
 * OCV Consumer thread:
 *   Creates a FrameConsumer object to consume frames from a stream,
 *   to perform open cv processing
 ******************************************************************************/


class aaNewOCVConsumerThread : public Thread
{
public:

    explicit aaNewOCVConsumerThread()
    {
    }

  aaNewOCVConsumerThread(OutputStream *stream,  NvEglRenderer *renderer, int id, Queue<frameBuffer> *q,Queue<int> * fdq, Queue<int> *msgq)
        : m_stream(stream),  m_renderer(renderer), m_id(id) , m_pinputFrameQ(q), m_pinputFrameFdQ(fdq),m_pcamCapture2NewOCVConsumerMsgQ(msgq)
    {
    }

    ~aaNewOCVConsumerThread()
    {
    }
    void init_members(OutputStream *stream, Argus::Size size, NvEglRenderer *renderer);

private:


    /** @name Thread methods */
    /**@{*/
    virtual bool threadInitialize();
    virtual bool threadExecute();
    virtual bool threadShutdown();
    /**@}*/

    OutputStream* m_stream;
    UniqueObj<FrameConsumer> m_consumer;
    Argus::Size m_framesize;
    NvEglRenderer *m_renderer;
    int m_id;


    Queue<frameBuffer> *m_pinputFrameQ;
    Queue<int   > *m_pinputFrameFdQ;
    Queue<int   > *m_pcamCapture2NewOCVConsumerMsgQ;

    aaDebug       *m_pDebugObj;

};



} // namespace ArgusSamples

#endif // OCVCONSUMER
