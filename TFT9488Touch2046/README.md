# 📺 ESP32-S3 TFT + Touch + SD Project

**Multi-Protocol Support:** Prepared for TTL-USB, Bluetooth (HC-05 / DT-06), and BLE.

---

## 🛠 Hardware Configuration

* **MCU:** ESP32-S3
* **Display:** ILI9488 3.5" TFT
* **Touch:** XPT2046 (Resistive)
* **Storage:** MicroSD via SPI (Optional)

---

## 🔌 Pin Mapping (ESP32-S3)

| Function   | Pin | Bus       |
| ---------- | --- | --------- |
| TFT_MOSI   | 13  | SPI Bus 1 |
| TFT_SCLK   | 14  | SPI Bus 1 |
| TFT_MISO   | -1  | Not Used  |
| TFT_CS     | 10  | SPI Bus 1 |
| TFT_DC     | 11  | SPI Bus 1 |
| TFT_RST     | 12  | SPI Bus 1 |
| TOUCH_CS   | 15  | SPI Bus 2 |
| TOUCH_SCLK | 5   | SPI Bus 2 |
| TOUCH_MISO | 6   | SPI Bus 2 |
| TOUCH_MOSI | 4   | SPI Bus 2 |
| TOUCH_IRQ  | 1   | Interrupt |
| SD_CS      | 21  | SPI Bus 2 |

![Image](DeviceImage.jpg)

---

## 💻 Software Stack

* **Framework:** Arduino / PlatformIO
* **Graphics:** https://github.com/Bodmer/TFT_eSPI
* **Touch:** https://github.com/PaulStoffregen/XPT2046_Touchscreen
* **IDE:** VS Code + PlatformIO

---

## ⚙️ Installation & Build

1. Clone the repository
2. Open in VS Code with PlatformIO


