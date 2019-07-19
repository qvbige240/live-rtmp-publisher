#ifndef _H264_READER_H_
#define _H264_READER_H_

#include <cstdint>
#include <cstring>
#include <utility>


class H264Reader {
public:
    H264Reader(const char* file);

    ~H264Reader();

    std::pair<int, char*> getMetadata();

    std::pair<int, char*> readnal();
private:
    //int mPts;
    //int mLumaSize;
    //int mChromaSize;
    //x264_t *mHandle;
    //x264_nal_t *mNal;
    //x264_picture_t mPicture;

	void*	mData;
	size_t	mLength;
	size_t	mPos;
	char	mMetadata[256];

    char    mBuffer[1024 << 8];
};

#endif
