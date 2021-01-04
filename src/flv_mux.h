
/**
 * History:
 * ================================================================
 * 2020-12-29 qing.zou created
 *
 */

#ifndef FLV_MUX_H
#define FLV_MUX_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool flv_meta_data(unsigned char **output, size_t *size, bool write_header);

#ifdef __cplusplus
}
#endif

#endif // FLV_MUX_H
