#include "gpio.h"
#include "qr.h"
#include <atomic>
#include <iostream>
#include <thread>

constexpr int LED_RED = 27;
constexpr int LED_GREEN = 17;

namespace {
std::atomic<std::chrono::system_clock::time_point> s_last_rcv_time =
    std::chrono::system_clock::now();
std::atomic_bool s_stop_thread = false;
void led_thread() {
    gpio_configure(LED_RED, GPIO_OUTPUT);
    gpio_configure(LED_GREEN, GPIO_OUTPUT);
    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        gpio_write(LED_RED, 0);
        gpio_write(LED_GREEN, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        gpio_write(LED_RED, 1);
        gpio_write(LED_GREEN, 0);
    }
    gpio_write(LED_RED, 0);
    gpio_write(LED_GREEN, 0);
    while (!s_stop_thread) {
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
        while (qr_read(buf, &size) == 0) {
            if (size == sizeof(buf))
                s_last_rcv_time = std::chrono::system_clock::now();
        }
        if (qr_close())
            std::cerr << "failed to close QR device" << std::endl;
        else
            std::cerr << "closed QR device" << std::endl;
    }
}
} // namespace

int main(int argc, const char* argv[]) {
    try {
        if (gpio_init())
            throw std::runtime_error("failed to init gpio");
        if (qr_init())
            throw std::runtime_error("failed to init USB");
        std::thread th(led_thread);
        try {
            qr_thread();
        }
        catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
        }
        s_stop_thread = true;
        th.join();
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
    }
    qr_deinit();
    gpio_deinit();
    return 1;
}