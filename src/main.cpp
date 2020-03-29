#include "gpio.h"
#include "qr.h"
#include <atomic>
#include <iostream>
#include <thread>

constexpr int LED_RED = 27;
constexpr int LED_GREEN = 17;

namespace {
std::atomic<std::chrono::system_clock::time_point> s_last_rcv_time(
    std::chrono::system_clock::now());
std::atomic_bool s_stop_thread(false);
std::atomic_bool s_is_opened_qr_device(false);

/// led_red_thread
void led_red_thread() {
    gpio_configure(LED_RED, GPIO_OUTPUT);
    gpio_write(LED_RED, 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while (!s_stop_thread) {
        auto now = std::chrono::system_clock::now();
        if (now - s_last_rcv_time.load() < std::chrono::seconds(1)) {
            gpio_write(LED_RED, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            gpio_write(LED_RED, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else {
            gpio_write(LED_RED, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

/// led_green_thread
void led_green_thread() {
    gpio_configure(LED_GREEN, GPIO_OUTPUT);
    gpio_write(LED_GREEN, 0);
    while (!s_stop_thread) {
        if (s_is_opened_qr_device) {
            gpio_write(LED_GREEN, 1);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else {
            gpio_write(LED_GREEN, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            gpio_write(LED_GREEN, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

/// qr_thread
void qr_thread() {
    for (;;) {
        if (qr_open()) {
            std::cerr << "failed to open QR device" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }
        s_is_opened_qr_device = true;
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
        s_is_opened_qr_device = false;
    }
}
} // namespace

int main(int argc, const char* argv[]) {
    try {
        if (gpio_init())
            throw std::runtime_error("failed to init gpio");
        if (qr_init())
            throw std::runtime_error("failed to init USB");
        std::thread th0(led_red_thread);
        std::thread th1(led_green_thread);
        try {
            qr_thread();
        }
        catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
        }
        s_stop_thread = true;
        th0.join();
        th1.join();
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
    }
    qr_deinit();
    gpio_deinit();
    return 1;
}