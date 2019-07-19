#include <iostream>
#include "H264Encoder.h"

H264Encoder::H264Encoder(int width, int height, int fps, int bitrate): mPts(0) {
    x264_param_t param;

    mLumaSize = width * height;
    mChromaSize = mLumaSize / 4;

    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    x264_param_apply_profile(&param, "baseline");

    param.i_log_level = X264_LOG_NONE;
    param.i_csp = X264_CSP_I420;
    param.i_width = width;
    param.i_height = height;
    param.i_fps_den = 1;
    param.i_fps_num = fps;
    param.i_keyint_max = 10*fps;

    param.rc.i_rc_method = X264_RC_ABR;
    param.rc.i_bitrate = bitrate;

    param.b_repeat_headers = 0; // add sps and pps manually

#ifndef H264_AVCC
    param.b_annexb = 1; // for the convenience of packing
#else
    param.b_annexb = 0; // for the convenience of packing
#endif

    x264_picture_alloc(&mPicture, param.i_csp, param.i_width, param.i_height);

    mHandle = x264_encoder_open(&param);
}

H264Encoder::~H264Encoder() {
    x264_encoder_close(mHandle);
    x264_picture_clean(&mPicture);
}

#include <stdio.h>

typedef struct _PrivInfo
{
    // test..
    FILE        *fp;
    size_t      offset;
    int         file_size;
    unsigned    buf_size;
    char        *buf;
} PrivInfo;

static int vpk_file_save(const char *filename, void *data, size_t size)
{
    FILE *fp = 0;
    size_t ret = 0;
    //return_val_if_fail(filename != NULL && data != NULL, -1);

    fp = fopen(filename, "a+");
    if (fp != NULL && data)
    {
        ret = fwrite(data, 1, size, fp);
        fclose(fp);
    }
    if (ret != size)
        printf("fwrite size(%ld != %ld) incorrect!", ret, size);

    return ret;
}

#ifndef H264_AVCC
static inline size_t find_start_code(unsigned char *buf)
{
	if (buf[0] != 0x00 || buf[1] != 0x00 ) {
		return 0;
	}

	if (buf[2] == 0x01) {
		return 3;
	} else if (buf[2] == 0x00 && buf[3] == 0x01) {
		return 4;
	}

	return 0;
}
#endif

std::pair<int, char*> H264Encoder::encode(char* frame) {
    int temp, size;
    x264_picture_t out;

    memcpy(mPicture.img.plane[0], frame, mLumaSize);
    memcpy(mPicture.img.plane[1], frame + mLumaSize, mChromaSize);
    memcpy(mPicture.img.plane[2], frame + mLumaSize + mChromaSize, mChromaSize);

    mPicture.i_pts = mPts++;

    size = x264_encoder_encode(mHandle, &mNal, &temp, &mPicture, &out);

#ifdef _DEBUG
    printf("## nalu type(%d) len(%d)\n", mNal->i_type, size);
#endif

#ifdef _SAVE_H264
    unsigned char *p = (unsigned char *)mNal->p_payload;
    std::cout << "nal size: " << size << std::endl;
    int i = 0;
    for (i = 0; i < 32; i++)
        printf(" %02x", p[i]);
    printf("\n");

#ifndef H264_AVCC
    vpk_file_save("local.h264", p, size);
#else
    char nalu_head[4] = {0x00, 0x00, 0x00, 0x01};
    vpk_file_save("local.h264", nalu_head, 4);
    vpk_file_save("local.h264", p, size);
#endif //H264_AVCC

#endif

    return std::make_pair(size, reinterpret_cast<char*>(mNal->p_payload));
}

std::pair<int, char*> H264Encoder::getMetadata() {
    int temp, size;

    x264_encoder_headers(mHandle, &mNal, &temp);
    size = mNal[0].i_payload + mNal[1].i_payload;

#ifdef _DEBUG
    std::cout << "metadata: " << size << std::endl;
#endif

#ifdef _SAVE_H264
    unsigned char *p = (unsigned char *)mNal->p_payload;

    int i = 0;
    for (i = 0; i < size; i++)
        printf("%02x%s", p[i], i % 16 == 15 ? "\n" : " ");
    printf("\n");

#ifndef H264_AVCC
    vpk_file_save("local.h264", p, size);
    size_t pos = 0, start = 0;
    size_t offset = find_start_code(p);
    while (pos < size)
    {
        start = find_start_code(p + offset + pos);
        if (start) break;
        pos++;
    }

    p[2] = pos >> 8 & 0xff;
    p[3] = pos & 0xff;

    p[4+pos+2] = (size - pos - 8) >> 8 & 0xff;
    p[4+pos+3] = (size - pos - 8) & 0xff;
#else
    int len = (p[2] << 8) | p[3];
    int pps = (p[4 + len + 2] << 8) | p[4 + len + 3];
    printf("sps len: %d\n", len);
    printf("pps len: %d\n", pps);

    //std::cout<<std::hex;
    //int a = 40;
    //std::cout<<a;
    //for (int i = 0; i < size; i++)
    //	std::cout <<std::hex << p[i] << " ";
    //std::cout<<std::endl;

    char nalu_head[4] = {0x00, 0x00, 0x00, 0x01};
    // sps
    vpk_file_save("local.h264", nalu_head, 4);
    vpk_file_save("local.h264", p + 4, len);

    // pps
    vpk_file_save("local.h264", nalu_head, 4);
    vpk_file_save("local.h264", p + 8 + len, size - len - 8);
#endif //H264_AVCC

#endif

    return std::make_pair(size, reinterpret_cast<char*>(mNal->p_payload));
}
