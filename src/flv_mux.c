
/**
 * History:
 * ================================================================
 * 2020-12-29 qing.zou created
 *
 */

#include <string.h>
#include <stdarg.h>
#include "c99defs.h"

#include "util/array-serializer.h"
#include "rtmp_helpers.h"

#include "flv_mux.h"

static bool build_flv_meta_data(unsigned char **output, size_t *size)
{
    char buf[1024];
    char *enc = buf;
    char *end = enc+sizeof(buf);

    enc_str(&enc, end, "onMetaData");

    *enc++ = AMF_ECMA_ARRAY;
    enc    = AMF_EncodeInt32(enc, end, 8);

    enc_num_val(&enc, end, "duration", 0.0);
    enc_num_val(&enc, end, "fileSize", 0.0);

    enc_num_val(&enc, end, "width", 1024.0);
    enc_num_val(&enc, end, "height", 600.0);

    enc_str_val(&enc, end, "videocodecid", "avc1");
    // enc_num_val(&enc, end, "videodatarate", 1024*600*8.0);
    enc_num_val(&enc, end, "videodatarate", 2500.0);
    enc_num_val(&enc, end, "framerate", 25);

    enc_str_val(&enc, end, "encoder", "king");

    *enc++ = 0;
    *enc++ = 0;
    *enc++ = AMF_OBJECT_END;

    *size = enc - buf;
    *output = calloc(1, *size);
    memcpy(*output, buf, *size);

    return true;
}

bool flv_meta_data(unsigned char **output, size_t *size, bool write_header)
{
    struct array_output_data data;
    struct serializer s;
    uint8_t *meta_data = NULL;
    size_t meta_data_size;
    uint32_t start_pos;

    array_output_serializer_init(&s, &data);
    if (!build_flv_meta_data(&meta_data, &meta_data_size))
    {
        free(meta_data);
        return false;
    }

    if (write_header)
    {
        s_write(&s, "FLV", 3);
        s_w8(&s, 1);
        s_w8(&s, 1);  // 1 video, 4 audio,  5 av
        s_wb32(&s, 9);
        s_wb32(&s, 0);
    }

    start_pos = serializer_get_pos(&s);
    
    s_w8(&s, RTMP_PACKET_TYPE_INFO);

    s_wb24(&s, (uint32_t)meta_data_size);
    s_wb32(&s, 0);
    s_wb24(&s, 0);

    s_write(&s, meta_data, meta_data_size);

    s_wb32(&s, (uint32_t)serializer_get_pos(&s) - start_pos -1);

    *output = data.bytes.array;
    *size   = data.bytes.num;

    free(meta_data);

    return true;
}
