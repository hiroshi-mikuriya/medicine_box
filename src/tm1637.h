#pragma once

#include <string>
#include <vector>

class TM1637 {
    int clk_;
    int dio_;
    uint8_t brightness_;

    void delay();
    void start();
    void stop();
    void write_data_cmd();
    void write_dsp_ctrl();
    void write_byte(uint8_t b);
    void write(std::vector<uint8_t> const& segments, uint8_t pos = 0);
    uint8_t encode_digit(int digit);
    std::vector<uint8_t> encode_string(std::string const& s);
    uint8_t encode_char(char c);

public:
    explicit TM1637(int clk, int dio, uint8_t br = 7);
    void setBrightness(uint8_t br);
    void hex(int val);
    void number(int num);
    void numbers(int num1, int num2, bool colon = true);
    void temperature(int num);
    void show(std::string const& s, bool colon = false);
};
