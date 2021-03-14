#include "DataQueue.h"
#include <algorithm>
#include <assert.h>
#include <memory.h>

namespace Firefly
{
    DataQueue::DataQueue()
    {
        m_dwInsertPos = 0;
        m_dwTerminalPos = 0;
        m_dwDataQueryPos = 0;
        m_dwDataSize = 0;
        m_dwDataPacketCount = 0;
        m_dwBufferSize = 0L;
        m_pDataQueueBuffer = nullptr;
    }

    DataQueue::~DataQueue()
    {
        delete m_pDataQueueBuffer;
        m_pDataQueueBuffer = nullptr;
        return;
    }

    int DataQueue::Size()
    {
        return m_dwDataPacketCount;
    }

    bool DataQueue::InsertData(unsigned short wIdentifier, void* pBuffer, unsigned int wDataSize)
    {
        tagDataHead DataHead;
        memset(&DataHead, 0, sizeof(DataHead));
        DataHead.wDataSize = wDataSize;
        DataHead.wIdentifier = wIdentifier;

        if (RectifyBuffer(sizeof(DataHead) + DataHead.wDataSize) == false)
        {
            assert(false);
            return false;
        }

        try
        {
            memcpy(m_pDataQueueBuffer + m_dwInsertPos, &DataHead, sizeof(DataHead));
            if (wDataSize > 0)
            {
                assert(pBuffer != NULL);
                memcpy(m_pDataQueueBuffer + m_dwInsertPos + sizeof(DataHead), pBuffer, wDataSize);
            }

            m_dwDataPacketCount++;
            m_dwDataSize += sizeof(DataHead) + wDataSize;
            m_dwInsertPos += sizeof(DataHead) + wDataSize;
            m_dwTerminalPos = std::max(m_dwTerminalPos, m_dwInsertPos);
            return true;
        }
        catch (...)
        {
            assert(false);
            return false;
        }

        return false;
    }

    bool DataQueue::DistillData(tagDataHead& DataHead, void* pBuffer, unsigned int wBufferSize)
    {
        assert(m_dwDataSize > 0L);
        assert(m_dwDataPacketCount > 0);
        assert(m_pDataQueueBuffer != NULL);

        if (m_dwDataSize == 0L) return false;
        if (m_dwDataPacketCount == 0L) return false;

        if (m_dwDataQueryPos == m_dwTerminalPos)
        {
            m_dwDataQueryPos = 0L;
            m_dwTerminalPos = m_dwInsertPos;
        }

        assert(m_dwBufferSize >= (m_dwDataQueryPos + sizeof(tagDataHead)));
        tagDataHead* pDataHead = (tagDataHead*)(m_pDataQueueBuffer + m_dwDataQueryPos);
        assert(wBufferSize >= pDataHead->wDataSize);
        unsigned int wPacketSize = sizeof(DataHead) + pDataHead->wDataSize;
        assert(m_dwBufferSize >= (m_dwDataQueryPos + wPacketSize));
        assert(wBufferSize >= pDataHead->wDataSize);
        DataHead = *pDataHead;

        if (DataHead.wDataSize > 0)
        {
            if (wBufferSize < pDataHead->wDataSize) DataHead.wDataSize = 0;
            else memcpy(pBuffer, pDataHead + 1, DataHead.wDataSize);
        }

        assert(wPacketSize <= m_dwDataSize);
        assert(m_dwBufferSize >= (m_dwDataQueryPos + wPacketSize));
        m_dwDataPacketCount--;
        m_dwDataSize -= wPacketSize;
        m_dwDataQueryPos += wPacketSize;
        return true;
    }

    void DataQueue::RemoveData(bool bFreeMemroy)
    {
        m_dwDataSize = 0L;
        m_dwInsertPos = 0L;
        m_dwTerminalPos = 0L;
        m_dwDataQueryPos = 0L;
        m_dwDataPacketCount = 0L;

        if (bFreeMemroy == true)
        {
            m_dwBufferSize = 0;
            delete m_pDataQueueBuffer;
            m_pDataQueueBuffer = nullptr;
        }
    }

    bool DataQueue::RectifyBuffer(unsigned int dwNeedSize)
    {
        try
        {
            if ((m_dwDataSize + dwNeedSize) > m_dwBufferSize) throw 0;
            if ((m_dwInsertPos == m_dwTerminalPos) && ((m_dwInsertPos + dwNeedSize) > m_dwBufferSize))
            {
                if (m_dwDataQueryPos >= dwNeedSize) m_dwInsertPos = 0;
                else throw 0;
            }

            if ((m_dwInsertPos < m_dwTerminalPos) && ((m_dwInsertPos + dwNeedSize) > m_dwDataQueryPos)) throw 0;
        }
        catch (...)
        {
            try
            {
                unsigned int dwRiseSize = std::max(m_dwBufferSize / 2L, dwNeedSize * 10L);
                char* pNewQueueServiceBuffer = new char[m_dwBufferSize + dwRiseSize];
                assert(pNewQueueServiceBuffer != NULL);
                if (pNewQueueServiceBuffer == NULL) return false;

                if (m_pDataQueueBuffer != NULL)
                {
                    assert(m_dwTerminalPos >= m_dwDataSize);
                    assert(m_dwTerminalPos >= m_dwDataQueryPos);
                    unsigned int dwPartOneSize = m_dwTerminalPos - m_dwDataQueryPos;
                    if (dwPartOneSize > 0L) memcpy(pNewQueueServiceBuffer, m_pDataQueueBuffer + m_dwDataQueryPos, dwPartOneSize);
                    if (m_dwDataSize > dwPartOneSize)
                    {
                        assert((m_dwInsertPos + dwPartOneSize) == m_dwDataSize);
                        memcpy(pNewQueueServiceBuffer + dwPartOneSize, m_pDataQueueBuffer, m_dwInsertPos);
                    }
                }

                m_dwDataQueryPos = 0L;
                m_dwInsertPos = m_dwDataSize;
                m_dwTerminalPos = m_dwDataSize;
                m_dwBufferSize = m_dwBufferSize + dwRiseSize;
                delete m_pDataQueueBuffer;
                m_pDataQueueBuffer = pNewQueueServiceBuffer;
            }
            catch (...)
            {
                return false;
            }
        }

        return true;
    }
}