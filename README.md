Medicine Box
===

## ライブラリインストール

### apt

```
$ sudo apt-get install -y wiringpi mpg321 libtool libudev-dev ruby cmake clang-format
```

### libusb

```
$ cd ~
$ git clone https://github.com/libusb/libusb
$ cd libusb
$ ./autogen.sh
$ ./configure 
$ make
$ sudo make install
```

## USB DAC有効化

```
$ sudo vi /etc/modprobe.d/alsa-base.conf
options snd slots=snd_usb_audio,snd_bcm2835
options snd_usb_audio index=0
options snd_bcm2835 index=1
$ sudo reboot
$ cat /proc/asound/modules
 0 snd_usb_audio
 1 snd_bcm2835
```

## RTC有効化

```
$ sudo apt-get purge fake-hwclock
$ sudo vi /boot/config.txt
# 最後に次を追加
dtoverlay=i2c-rtc,ds3231
$ sudo vi /etc/udev/rules.d/85-hwclock.rules
KERNEL=="rtc0", RUN+="/sbin/hwclock --rtc=$root/$name --hctosys"
```

## 本アプリインストール

```
$ cd ~
$ git clone https://github.com/hiroshi-mikuriya/medicine_box
$ cd medicine_box
$ ruby install
```

## HW構成

```
Raspberry Pi
├── USB
│   ├── QM30-SU（QRコードリーダー）
│   └── DAC（DuKabel USB オーディオ 変換アダプタ）
└── GPIO
    ├── DS1307（RTC）
    ├── GREEN LED
    └── DiyStudio 5色 TM1637 4桁LED 0.56インチ（7 Segment LED）
```

### GPIOマッピング

|PIN|GPIO|Assign|
|:---|:---|:---|
|1|3.3V|RTC|
|2|5V|RTC|
|3|SDA|RTC|
|4|5V|RTC|
|5|SCL|RTC|
|6|GND|RTC|
|7|GPIO4|RTC|
|8|TXD0|RTC|
|9|GND|RTC|
|10|RXD0|RTC|
|11|GPIO17|TM1637 DIO|
|13|GPIO27|TM1637 CLK|
|15|GPIO22|GREEN LED|
