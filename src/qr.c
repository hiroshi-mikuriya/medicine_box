#include "qr.h"
#include <libusb1.0/libusb.h>
#include <stdio.h>

#define bInterfaceNumber 0
#define VENDOR_ID 0x076d
#define PRODUCT_ID 0x0001
#define EP_IN 0x83

static libusb_context* s_ctx = 0;
static libusb_device_handle* s_handle = 0;

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_init(void) {
    libusb_version const* v = libusb_get_version();
    printf("libusb: version %hu.%hu.%hu.%hu%s\n", v->major, v->minor, v->micro, v->nano, v->rc);
    int res = libusb_init(&s_ctx);
    if (res < 0) {
        return res;
    }
    libusb_set_option(0, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);
    return 0;
}

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_deinit(void) { return libusb_exit(s_ctx); }

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_open(void) {
    s_handle = libusb_open_device_with_vid_pid(s_ctx, VENDOR_ID, PRODUCT_ID);
    if (s_handle == 0) {
        return EFAULT;
    }
    if (libusb_kernel_driver_active(s_handle, bInterfaceNumber)) {
        libusb_detach_kernel_driver(s_handle, bInterfaceNumber);
    }
    int res = libusb_claim_interface(s_handle, bInterfaceNumber);
    if (res < 0) {
        libusb_reset_device(s_handle);
        libusb_close(s_handle);
        s_handle = 0;
        return res;
    }
    return 0;
}

/**
 * @retval 0 成功
 * @retval errno
 */
int qr_close(void) {
    int res = libusb_release_interface(s_handle, bInterfaceNumber);
    if (res < 0) {
        return res;
    }
    // libusb_reset_device(s_handle); // USB抜け時に固まるのでこの処理はしない
    res = libusb_close(s_handle);
    s_handle = 0;
    return 0;
}

/**
 * @param[out] buf buffer of 8 bytes or more
 * @param[out] size
 * @retval 0 成功
 * @retval errno
 */
int qr_read(uint8_t* buf, uint32_t* size) {
    const int timeout = 100;
    int actual_length = 0;
    int res = libusb_interrupt_transfer(s_handle, EP_IN, buf, 8, actual_length, timeout);
    if (res == LIBUSB_TRANSFER_COMPLETED) {
        *size = actual_length;
        return 0;
    }
    if (res == LIBUSB_TRANSFER_TIMED_OUT) {
        *size = 0;
        return 0;
    }
    fprintf(stderr, "libusb_interrupt_transfer returns %s in function qr_read\n",
        libusb_error_name(res));
    return EFAULT;
}
