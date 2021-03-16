/*
*   DataQueue.h
*
*   Data Queue.
*
*   Created on: 2018-11-13
*   Author:
*   All rights reserved.
*/
#ifndef __DataQueue_H__
#define __DataQueue_H__

namespace Firefly
{
    struct tagDataHead
    {
        unsigned int             nDataSize;
        unsigned short           wIdentifier;
    };

    class DataQueue
    {
    public:
        DataQueue();
        virtual ~DataQueue();

    public:
        bool InsertData( unsigned short wIdentifier, void *pBuffer, unsigned int nDataSize );
        bool DistillData( tagDataHead &DataHead, void *pBuffer, unsigned int wBufferSize );
        void RemoveData( bool bFreeMemroy );
        int Size();

    private:
        bool RectifyBuffer( unsigned int dwNeedSize );

    protected:
        unsigned int        m_dwInsertPos;              //InsertData Position
        unsigned int        m_dwTerminalPos;            //
        unsigned int        m_dwDataQueryPos;           //

    protected:
        unsigned int        m_dwDataSize; 
        unsigned int        m_dwDataPacketCount;

    protected:
        unsigned int        m_dwBufferSize;
        char               *m_pDataQueueBuffer;
    };
}

#endif