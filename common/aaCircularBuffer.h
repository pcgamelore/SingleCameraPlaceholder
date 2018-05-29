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


#ifndef __AACIRCULARBUFFERH__
#define __AACIRCULARBUFFERH__

#include <stdlib.h>
#include <stddef.h>


// Optimization Note : replace all modulo operation in this file with AND operation if size 
//                     is power of two. Note that modulo is expensive operation, spanning several cycles.
//                     Whereas AND will take at the most single cycle
// CAVEAT            : note that this class is NOT thread safe
template <class T>
class aaCircularBuffer {
public:
    aaCircularBuffer(size_t size) :
        m_pBuf(new T[size]),
        m_Size(size), m_Head(0),m_Tail(0)
    {
        // size must be power of 2
        // TBD : assert (!( size & (size-1) ));
    }

    ~aaCircularBuffer() {
        delete m_pBuf;
    }

    inline void push(T  item){
        m_pBuf[m_Head]  = item;
        m_Head          = (m_Head+1) % m_Size;
    } 

    inline T peek(void) {
        return m_pBuf[m_Tail];
    }

    inline T pop(void) {
        T ret = m_pBuf[m_Tail];
        m_Tail          = (m_Tail+1) % m_Size;
        return ret;
    } 

    inline T peek_minus_n(int n) {
        return m_pBuf[ (m_Tail - n) & (m_Size - 1) ];
    }

    inline int fullness(void) {
        return (m_Head >= m_Tail) ? (m_Head - m_Tail) : m_Size - (m_Tail - m_Head);
    } 

    inline float getMean() {
       register int fullness = (m_Head >= m_Tail) ? (m_Head - m_Tail) : m_Size - (m_Tail - m_Head);
       register float  sum   = 0; 
       register int count    = 0;
       register int i        = m_Tail;
       while(count++ < fullness){
          sum += m_pBuf[i];
          i    = (i + 1) % m_Size;
       }
       return sum/(float)fullness;
    }

    inline float getVariance() {
       register int fullness   = (m_Head >= m_Tail) ? (m_Head - m_Tail) : m_Size - (m_Tail - m_Head);
       register float  sum     = 0;
       register int count      = 0;
       register int i          = m_Tail;
       while(count++ < fullness){
          sum         += m_pBuf[i];
          i           = (i + 1) % m_Size;
       }
       sum            /= (float) fullness;
       float variance = 0;
       i              = m_Tail; 
       count          = 0;
       while(count++ < fullness){
          variance    += ((m_pBuf[i] - sum) * (m_pBuf[i] - sum));
          i            = (i + 1) % m_Size;
       }

       return variance;
    }


private:

    T *                m_pBuf;
    size_t             m_Head;
    size_t             m_Tail;
    size_t             m_Size;
};

#endif

