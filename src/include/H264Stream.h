#ifndef _H264_STREAM_H_
#define _H264_STREAM_H_

#include <unistd.h>
#include <chrono>
#include "H264Encoder.h"
#include "H264Reader.h"
#include "H264RTMPPackager.h"
#include "FilteredVideoSource.h"
#include "VideoSource.h"
#include "PacketQueue.h"
#include "MemoryPool.h"

#define msleep(x) usleep((x) * 1000)

class H264Stream {
public:
    H264Stream(FilteredVideoSource& source, PacketQueue& queue, MemoryPool& pool, int fps, int bitrate, char *file);

    void run();

private:
    std::chrono::milliseconds mInterval;
    FilteredVideoSource &mSource;
    PacketQueue &mQueue;
    MemoryPool &mPool;
    H264Encoder mEncoder;
    H264Reader mReader;
    RTMPPacket mMetadata;
    char mDataBuf[256];
};

#endif
