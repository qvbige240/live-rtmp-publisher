#include "H264RTMPPackager.h"

RTMPPacket H264RTMPPackager::pack(char* buf, const char* data, int length) const {
    char *body = buf + RTMP_MAX_HEADER_SIZE;

    RTMPPacket packet;
    packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet.m_nChannel = 0x04;
    packet.m_hasAbsTimestamp = 0;
#ifdef H264_AVCC
    packet.m_nBodySize = length + 5;
#else
    packet.m_nBodySize = length + 5 + 4;
#endif
    packet.m_body = body;

#ifdef H264_AVCC
    *(body++) = (data[4] & 0x1f) == 0x05 ? 0x17 : 0x27;
#else
    *(body++) = (data[0] & 0x1f) == 0x05 ? 0x17 : 0x27;
#endif
    *(body++) = 0x01;
    *(body++) = 0x00;
    *(body++) = 0x00;
    *(body++) = 0x00;
#ifndef H264_AVCC
    *(body++) = length >> 24 & 0xff;
    *(body++) = length >> 16 & 0xff;
    *(body++) = length >> 8 & 0xff;
    *(body++) = length & 0xff;
#endif
    memcpy(body, data, length);

    return packet;
}

RTMPPacket H264RTMPPackager::metadata(char* buf, const char* data, int length) const {
    char *body = buf + RTMP_MAX_HEADER_SIZE;

    RTMPPacket packet;
    packet.m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet.m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet.m_nChannel = 0x04;
    packet.m_hasAbsTimestamp = 0;
    packet.m_nBodySize = 8 + length;
    packet.m_body = body;

    *(body++) = 0x17; // 1-keyframe, 7-AVC
    *(body++) = 0x00;
    *(body++) = 0x00;
    *(body++) = 0x00;
    *(body++) = 0x00;

    // AVCDecoderConfigurationRecord

    *(body++) = 0x01; // configurationVersion
    *(body++) = data[5]; // AVCProfileIndication
    *(body++) = data[6]; // profile_compatibility
    *(body++) = data[7]; // AVCLevelIndication
    *(body++) = 0xff; // 111111(reserved) + lengthSizeMinusOne

    int len = (data[2] << 8) | data[3];

    *(body++) = 0xe1; // 111(reserved) + numOfSequenceParameterSets
    memcpy(body, data + 2, len + 2);

    body += (len + 2);
    *(body++) = 0x01; // numOfPictureParameterSets
    memcpy(body, data + len + 6, length - len - 6);

    return packet;
}
