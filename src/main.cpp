#include "gpio.h"
#include "qr.h"
#include <iostream>
#include <thread>

constexpr int LED_RED = 27;
constexpr int LED_GREEN = 17;

namespace {
class gpio_initializer {
public:
    gpio_initializer() {
        if (gpio_init()) {
            throw std::runtime_error("failed to init gpio");
        }
    }
    ~gpio_initializer() {
        if (gpio_deinit()) {
            throw std::runtime_error("failed to deinit gpio");
        }
    }
};

class qr_initializer {
public:
    qr_initializer() {
        if (qr_init()) {
            throw std::runtime_error("failed to init qr");
        }
    }
    ~qr_initializer() {
        if (qr_deinit()) {
            throw std::runtime_error("failed to deinit qr");
        }
    }
};

bool s_stop_led_thread = false;
void led_thread() {
    while (!s_stop_led_thread) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        gpio_write(LED_RED, 0);
        gpio_write(LED_GREEN, 1);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        gpio_write(LED_RED, 1);
        gpio_write(LED_GREEN, 0);
    }
}

void qr_thread() {
    for (;;) {
        if (qr_open()) {
            std::cerr << "failed to open QR device" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }
        std::cerr << "opened QR device" << std::endl;
        uint8_t buf[8] = { 0 };
        uint32_t size = 0;
        for (;;) {
            if (qr_read(buf, &size)) {
                break;
            }
            if (size == 0) {
                continue;
            }
            std::printf("read %d bytes\n", size);
        }
        if (qr_close()) {
            std::cerr << "failed to close QR device" << std::endl;
        }
        else {
            std::cerr << "closed QR device" << std::endl;
        }
    }
}
} // namespace

int main(int argc, const char* argv[]) {
    try {
        gpio_initializer ginit;
        qr_initializer qinit;
        if (gpio_configure(LED_RED, GPIO_OUTPUT)) {
            throw std::runtime_error("failed to configure RED LED");
        }
        if (gpio_configure(LED_GREEN, GPIO_OUTPUT)) {
            throw std::runtime_error("failed to configure RED GREEN");
        }
        std::thread th(led_thread);
        try {
            qr_thread();
        }
        catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
        }
        s_stop_led_thread = true;
        th.join();
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}