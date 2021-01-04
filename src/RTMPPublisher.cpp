#include "RTMPPublisher.h"
#include <iostream>
#include "flv_mux.h"

RTMPPublisher::RTMPPublisher(PacketQueue& queue, MemoryPool& pool): mQueue(queue), mPool(pool) {
    mRTMP = RTMP_Alloc();
    RTMP_Init(mRTMP);
}

RTMPPublisher::~RTMPPublisher() {
    if (mRTMP) {
        if (RTMP_IsConnected(mRTMP)) {
            RTMP_Close(mRTMP);
        }
        RTMP_Free(mRTMP);
    }
}

bool RTMPPublisher::connect(char *url) {
    if (!RTMP_SetupURL(mRTMP, url)) {
        return false;
    }

    RTMP_EnableWrite(mRTMP);

    if (!RTMP_Connect(mRTMP, NULL)) {
        return false;
    }

    if (!RTMP_ConnectStream(mRTMP, 0)) {
        RTMP_Close(mRTMP);
        return false;
    }

    return true;
}

bool RTMPPublisher::meta_data()
{
    bool ret;
    uint8_t *meta_data;
    size_t meta_data_size;
    // bool    success = true;
    // PrivInfo *thiz = p->priv;

    if (!RTMP_IsConnected(mRTMP)) {
        std::cout << "can not connect to server" << std::endl;
        return false;
    }

    ret = flv_meta_data(&meta_data, &meta_data_size, false);

    if (ret)
    {
        ret = RTMP_Write(mRTMP, (char *)meta_data, (int)meta_data_size) >= 0;
        free(meta_data);
    }
    return ret;
}

void RTMPPublisher::run() {
    meta_data();

    while (true) {
        RTMPPacket &packet = mQueue.front();
        packet.m_nInfoField2 = mRTMP->m_stream_id;
        packet.m_nTimeStamp = RTMP_GetTime() & 0xffffff;

        if (!RTMP_IsConnected(mRTMP)) {
            std::cout << "can not connect to server" << std::endl;
            return;
        }

        if (!RTMP_SendPacket(mRTMP, &packet, 1)) {
            std::cout << "fail to send packet" << std::endl;
            return;
        }

        int bytes = packet.m_nBodySize + RTMP_MAX_HEADER_SIZE;
        char *body = packet.m_body - RTMP_MAX_HEADER_SIZE;

        if (!mQueue.pop()) {
            mPool.putChunk(bytes, body);
        }
    }
}
