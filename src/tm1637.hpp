#pragma once
#include <cstdio>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <wiringPi.h>

constexpr uint8_t TM1637_CMD1 = 0x40;   ///< 0x40 data command
constexpr uint8_t TM1637_CMD2 = 0xc0;   ///< 0xC0 address command
constexpr uint8_t TM1637_CMD3 = 0x80;   ///< 0x80 display control command
constexpr uint8_t TM1637_DSP_ON = 0x08; ///< 0x08 display on
/// msb is the decimal point or the colon depending on your display
constexpr uint8_t TM1637_MSB = 0x80;

// 0-9, a-z, blank, dash, star
constexpr uint8_t SEGMENTS[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77,
    0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x06, 0x1E, 0x76, 0x38, 0x55, 0x54, 0x3F, 0x73, 0x67,
    0x50, 0x6D, 0x78, 0x3E, 0x1C, 0x2A, 0x76, 0x6E, 0x5B, 0x00, 0x40, 0x63 };

class TM1637 {
    int clk_;
    int dio_;
    uint8_t brightness_;

    void delay() {
        // 10us delay between clk/dio pulses
        std::this_thread::sleep_for(std::chrono::microseconds(5));
    }

public:
    /*Library for quad 7-segment LED modules based on the TM1637 LED driver.*/
    explicit TM1637(int clk, int dio, int br = 7) : clk_(clk), dio_(dio), brightness_(br) {
        if (brightness_ < 0 || 7 < brightness_)
            throw std::runtime_error("Brightness out of range");

        pinMode(clk_, OUTPUT);
        pinMode(dio_, OUTPUT);
        digitalWrite(clk_, 0);
        digitalWrite(dio_, 0);

        delay();

        write_data_cmd();
        write_dsp_ctrl();
    }

    void start() {
        digitalWrite(dio_, 1);
        digitalWrite(clk_, 1);
        delay();
        digitalWrite(dio_, 0);
    }

    void stop() {
        digitalWrite(clk_, 0);
        delay();
        digitalWrite(dio_, 0);
        delay();
        digitalWrite(clk_, 1);
        delay();
        digitalWrite(dio_, 1);
    }

    void write_data_cmd() {
        // automatic address increment, normal mode
        start();
        write_byte(TM1637_CMD1);
        stop();
    }

    void write_dsp_ctrl() {
        // display on, set brightness
        start();
        write_byte(TM1637_CMD3 | TM1637_DSP_ON | brightness_);
        stop();
    }

    void write_byte(uint8_t b) {
        for (int i = 0; i < 8; ++i) {
            digitalWrite(clk_, 0);
            delay();
            digitalWrite(dio_, b & 1 ? 1 : 0);
            delay();
            digitalWrite(clk_, 1);
            delay();
            b >>= 1;
        }
        digitalWrite(clk_, 0);
        delay();
        digitalWrite(clk_, 1);
        delay();
        digitalWrite(clk_, 0);
    }

    void setBrightness(uint8_t br) {
        /*Set the display brightness 0-7.*/
        // brightness 0 = 1/16th pulse width
        // brightness 7 = 14/16th pulse width
        if (7 < br)
            throw std::runtime_error("Brightness out of range");
        brightness_ = br;
        write_data_cmd();
        write_dsp_ctrl();
    }

    void write(std::vector<uint8_t> const& segments, int pos = 0) {
        /* Display up to 6 segments moving right from a given position.
         The MSB in the 2nd segment controls the colon between the 2nd
         and 3rd segments.*/
        if (pos < 0 || 4 < pos)
            throw std::runtime_error("Position out of range");
        write_data_cmd();
        start();

        write_byte(TM1637_CMD2 | pos);
        for (uint8_t seg : segments)
            write_byte(seg);
        stop();
        write_dsp_ctrl();
    }

    uint8_t encode_digit(int digit) { /* Convert a character 0-9, a-f to a segment.*/
        return SEGMENTS[digit & 0x0f];
    }

    std::vector<uint8_t> encode_string(std::string const& s) {
        /* Convert an up to 4 character length string containing 0-9, a-z,
        space, dash, star to an array of segments, matching the length of the
        source string.*/
        std::vector<uint8_t> dst;
        dst.reserve(s.size());
        for (auto c : s)
            dst.push_back(encode_char(c));
        return dst;
    }

    uint8_t encode_char(char c) {
        /* Convert a character 0-9, a-z, space, dash or star to a segment.*/
        if (c == 32)
            return SEGMENTS[36]; // space
        if (c == 42)
            return SEGMENTS[38]; // star/degrees
        if (c == 45)
            return SEGMENTS[37]; // dash
        if (65 <= c && c <= 90)
            return SEGMENTS[c - 55]; // uppercase A-Z
        if (97 <= c && c <= 122)
            return SEGMENTS[c - 87]; // lowercase a-z
        if (48 <= c && c <= 57)
            return SEGMENTS[c - 48]; // 0-9
        throw std::runtime_error("Character out of range");
    }

    void hex(int val) {
        /* Display a hex value 0x0000 through 0xffff, right aligned.*/
        char s[5] = { 0 };
        snprintf(s, sizeof(s), "%04X", val);
        write(encode_string(s));
    }

    void number(int num) {
        /* Display a numeric value -999 through 9999, right aligned.*/
        // limit to range -999 to 9999
        num = std::max(-999, std::min(num, 9999));
        char s[5] = { 0 };
        snprintf(s, sizeof(s), "%4d", num);
        write(encode_string(s));
    }

    void numbers(int num1, int num2, bool colon = true) {
        /* Display two numeric values -9 through 99, with leading zeros
        and separated by a colon.*/
        num1 = std::max(-9, std::min(num1, 99));
        num2 = std::max(-9, std::min(num2, 99));
        char s[5] = { 0 };
        snprintf(s, sizeof(s), "%2d%2d", num1, num2);
        auto segments = encode_string(s);
        if (colon)
            segments[1] |= 0x80; // colon on
        write(segments);
    }

    void temperature(int num) {
        if (num < -9) {
            show("lo");
        }
        else if (99 < num) {
            show("hi");
        }
        else {
            char s[5] = { 0 };
            snprintf(s, sizeof(s), "%2d", num);
            write(encode_string(s));
            write({ SEGMENTS[38], SEGMENTS[12] }, 2); // degrees C
        }
    }

    void show(std::string const& s, bool colon = false) {
        auto segments = encode_string(s);
        if (!segments.empty() && colon)
            segments[1] |= 0x80;
        write(segments);
    }
};
