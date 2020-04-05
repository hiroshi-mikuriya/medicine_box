#pragma once

#include <string>
#include <vector>

class TM1637 {
    int clk_;
    int dio_;
    uint8_t brightness_;

    void delay() const;
    void start() const;
    void stop() const;
    void write_data_cmd() const;
    void write_dsp_ctrl() const;
    void write_byte(uint8_t b) const;
    void write(std::vector<uint8_t> const& segments, uint8_t pos = 0) const;
    uint8_t encode_digit(int digit) const;
    std::vector<uint8_t> encode_string(std::string const& s) const;
    uint8_t encode_char(char c) const;

public:
    explicit TM1637(int clk, int dio, uint8_t br = 7);
    virtual ~TM1637();
    void brightness(uint8_t br);
    uint8_t brightness() const;
    void hex(int val) const;
    void number(int num) const;
    void numbers(int num1, int num2, bool colon = true) const;
    void temperature(int num) const;
    void show(std::string const& s, bool colon = false) const;
};
