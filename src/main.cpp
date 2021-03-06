#include "qr.h"
#include "tm1637.h"
#include <atomic>
#include <ctime>
#include <iostream>
#include <thread>
#include <wiringPi.h>

constexpr int TM1637_DIO_PIN = 0; // PIN11
constexpr int TM1637_CLK_PIN = 2; // PIN13
constexpr int LED_PIN = 3;        // PIN15

namespace {
std::atomic<std::chrono::system_clock::time_point> s_last_rcv_time(
    std::chrono::system_clock::now());
std::atomic_bool s_stop_thread(false);
std::atomic_bool s_is_opened_qr_device(false);

/// clock_thread
void clock_thread(TM1637& tm) {
    while (!s_stop_thread) {
        time_t now = std::time(nullptr);
        std::tm* t = std::localtime(&now);
        tm.numbers(t->tm_hour, t->tm_min, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        tm.numbers(t->tm_hour, t->tm_min, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

/// sound_thread
void sound_thread() {
    while (!s_stop_thread) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto now = std::chrono::system_clock::now();
        if (now - s_last_rcv_time.load() < std::chrono::milliseconds(110)) {
            system("mpg321 sound/Cash_Register-Beep01-1.mp3");
        }
    }
}

/// led_thread
void led_thread() {
    auto last_blink_time = std::chrono::system_clock::now();
    while (!s_stop_thread) {
        if (!s_is_opened_qr_device) {
            digitalWrite(LED_PIN, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            digitalWrite(LED_PIN, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        auto now = std::chrono::system_clock::now();
        if (now - s_last_rcv_time.load() < std::chrono::milliseconds(400) &&
            std::chrono::seconds(3) < now - last_blink_time) {
            digitalWrite(LED_PIN, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            digitalWrite(LED_PIN, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            last_blink_time = now;
        }
        else {
            digitalWrite(LED_PIN, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

/// qr_thread
void qr_thread() {
    while (!s_stop_thread) {
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
        if (wiringPiSetup() < 0)
            throw std::runtime_error("failed to init wiringPi");
        if (qr_init())
            throw std::runtime_error("failed to init USB");
        pinMode(LED_PIN, OUTPUT);
        TM1637 tm(TM1637_CLK_PIN, TM1637_DIO_PIN);
        std::thread th0(led_thread);
        std::thread th1(clock_thread, std::ref(tm));
        std::thread th2(sound_thread);
        try {
            qr_thread();
        }
        catch (std::exception const& e) {
            std::cerr << e.what() << std::endl;
        }
        s_stop_thread = true;
        th0.join();
        th1.join();
        th2.join();
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
    }
    qr_deinit();
    return 1;
}
