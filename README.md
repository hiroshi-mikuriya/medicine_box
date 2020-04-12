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
