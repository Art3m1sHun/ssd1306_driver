# SSD1306 OLED Driver on Embedded Linux

Custom Linux kernel driver for SSD1306 OLED display running on a Yocto-based Embedded Linux system using Raspberry Pi Zero 2W.

---

# Features

* Custom Embedded Linux image built with Yocto Project
* SSD1306 OLED driver development in Linux kernel space
* I2C communication support
* Device Tree integration and driver matching
* User-space command-line application
* Text rendering and screen control
* Successfully deployed and tested on real hardware

---

# Hardware

* Raspberry Pi Zero 2W
* SSD1306 OLED Display (I2C)
* Linux Embedded Platform

---

# Software Stack

| Component           | Description               |
| ------------------- | ------------------------- |
| Yocto Project       | Custom Linux distribution |
| Linux Kernel Module | SSD1306 driver            |
| Device Tree         | Hardware configuration    |
| I2C Subsystem       | OLED communication        |
| User-space App      | Display control           |

---

# Project Architecture

```text
+----------------------+
| User-space CLI App   |
+----------+-----------+
           |
           v
+----------------------+
| SSD1306 Kernel Driver|
+----------+-----------+
           |
           v
+----------------------+
| Linux I2C Subsystem  |
+----------+-----------+
           |
           v
+----------------------+
| SSD1306 OLED Display |
+----------------------+
```

---

# Implemented Features

## Kernel Driver

* SSD1306 initialization sequence
* Command/data transmission
* Display buffer handling
* Screen clear operation
* Character rendering
* Device Tree compatible matching
* Driver probe/remove support

## User-space Application

* Clear OLED screen
* Print text to display
* Exit and refresh display

---

# Device Tree Configuration

Example Device Tree node:

```dts
&i2c1 {
    status = "okay";

    ssd1306: oled@3c {
        compatible = "ssd1306";
        reg = <0x3c>;
        status = "okay";
        solomon,width = <128>;
        solomon,height = <64>;
        solomon,page-offset = <0>;
    };
};
```

---

# Build and Deployment

## Build Yocto Image

```bash
bitbake core-image-base
```

## Build Kernel Module

```bash
make
```

## Insert Driver

```bash
modprobe ssd1306
```

## Run User-space Application

```bash
./ssd1306_app
```

---

# Debugging

Useful commands:

```bash
dmesg | tail
i2cdetect -y 1
lsmod
```

---

# Demo

* Linux booted successfully from custom Yocto image
* SSD1306 driver matched through Device Tree
* OLED display controlled from user-space application
* Successfully tested on Raspberry Pi Zero 2W hardware

link: https://drive.google.com/file/d/1it0nYKCzolJPGofD7N9QfD40fKJAuyJb/view?usp=sharing

