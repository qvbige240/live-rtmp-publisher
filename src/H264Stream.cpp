#include "H264Stream.h"
#include <iostream>

H264Stream::H264Stream(FilteredVideoSource& source, PacketQueue& queue, MemoryPool& pool, int fps, int bitrate, char *file): 
    mInterval(1000 / fps), mSource(source), mQueue(queue), mPool(pool),
    mEncoder(source.getWidth(), source.getHeight(), fps, bitrate), mReader(file) {}
	/*static int time_sub(struct timeval *next, struct timeval *prev, struct timeval *result)
	{
		if (prev->tv_sec > next->tv_sec) return -1;
		if (prev->tv_sec == next->tv_sec && prev->tv_usec > next->tv_usec) return -1;

		result->tv_sec = next->tv_sec - prev->tv_sec;
		result->tv_usec = next->tv_usec - prev->tv_usec;
		if (result->tv_usec < 0)
		{
			result->tv_sec--;
			result->tv_usec += 1000000;
		}

		return 0;
	}*/

#ifdef USE_H264_READER
void H264Stream::run() {
    if (!mSource.isOpened()) {
        std::cout << "video device is not open" << std::endl;
        return;
    }

    char *buf;
    RTMPPacket packet;
    H264RTMPPackager packager;
    std::chrono::milliseconds duration;
    auto result = mReader.getMetadata();

    mMetadata = packager.metadata(mDataBuf, result.second, result.first);
    //mSource.getNextFrame(); // warm-up

    auto last = std::chrono::system_clock::now();

    while (1) {
		result = mReader.readnal();
		if (result.first == 0) {
			std::cout << "===== read end, result.first" << result.first << std::endl;
			break;
		}

		if (H264RTMPPackager::isPPS(result.second) || H264RTMPPackager::isSPS(result.second)
			 || H264RTMPPackager::isDPC(result.second)) {
			continue;
		}

		if (H264RTMPPackager::isKeyFrame(result.second)) {
            std::cout << "===== isKeyFrame" << std::endl;
            mQueue.push(mMetadata, true);
        }
        buf = mPool.getChunk(packager.getBodyLength(result.first));
        packet = packager.pack(buf, result.second, result.first);
        mQueue.push(packet);

        duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last);

        if (duration < mInterval) {
            msleep((mInterval - duration).count());
        }
        last = std::chrono::system_clock::now();
    }
}

#else

void H264Stream::run() {
    if (!mSource.isOpened()) {
        std::cout << "video device is not open" << std::endl;
        return;
    }

    char *buf;
    RTMPPacket packet;
    H264RTMPPackager packager;
    std::chrono::milliseconds duration;
    char *frame;
    auto result = mEncoder.getMetadata();

    mMetadata = packager.metadata(mDataBuf, result.second, result.first);
    mSource.getNextFrame(); // warm-up

    auto last = std::chrono::system_clock::now();

    while ((frame = mSource.getNextFrame()) != NULL) {
        result = mEncoder.encode(frame);
    
        if (H264RTMPPackager::isKeyFrame(result.second)) {
            mQueue.push(mMetadata, true);
        }
        buf = mPool.getChunk(packager.getBodyLength(result.first));
        packet = packager.pack(buf, result.second, result.first);
        mQueue.push(packet);

        duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last);

        if (duration < mInterval) {
            msleep((mInterval - duration).count());
        }
        last = std::chrono::system_clock::now();
    }
}
#endif
