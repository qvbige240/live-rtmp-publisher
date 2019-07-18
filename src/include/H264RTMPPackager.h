#ifndef _H264_RTMP_Packager_H_
#define _H264_RTMP_Packager_H_

#include <cstring>
#include "RTMPPackager.h"

class H264RTMPPackager: public RTMPPackager {
public:
    virtual RTMPPacket pack(char* buf, const char* data, int length) const;

    virtual RTMPPacket metadata(char* buf, const char* data, int length) const;
#ifdef USE_H264_READER

#ifdef H264_AVCC
    virtual int getBodyLength(int length) const { return length + 5 + RTMP_MAX_HEADER_SIZE; }

    static bool isKeyFrame(char *data) { return ((data[4] & 0x1f) == 0x05); }
    static bool isSPS(char *data) { return ((data[4] & 0x1f) == 0x07); }
    static bool isPPS(char *data) { return ((data[4] & 0x1f) == 0x08); }
    static bool isDPC(char *data) { return ((data[4] & 0x1f) == 0x03); }
    static bool isAUD(char *data) { return ((data[4] & 0x1f) == 0x09); }
#else
    virtual int getBodyLength(int length) const { return length + 5 + 4 + RTMP_MAX_HEADER_SIZE; }

    static bool isKeyFrame(char *data) { return ((data[0] & 0x1f) == 0x05); }
    static bool isSPS(char *data) { return ((data[0] & 0x1f) == 0x07); }
    static bool isPPS(char *data) { return ((data[0] & 0x1f) == 0x08); }
    static bool isDPC(char *data) { return ((data[0] & 0x1f) == 0x03); }
    static bool isAUD(char *data) { return ((data[0] & 0x1f) == 0x09); }
#endif

#else
    virtual int getBodyLength(int length) const { return length + 5 + RTMP_MAX_HEADER_SIZE; }

    static bool isKeyFrame(char* data) { return (data[4] & 0x1f); }
#endif
};

#endif
