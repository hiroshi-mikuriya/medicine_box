#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_init(void);

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_deinit(void);

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_open(void);

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_close(void);

/**
 * @param[out] buf buffer or 8 bytes or more
 * @param[out] size
 * @retval 0 成功
 * @retval errno
 */
int qr_read(uint8_t* buf, uint32_t* size);

#ifdef __cplusplus
}
#endif // __cplusplus
