# ESP32 ILI9488 JPEG Slideshow (SD Card + TFT_eSPI)

This project runs a JPEG slideshow on an ESP32 S3 using an ILI9488 TFT display and an SD card, with both devices running on **separate SPI buses** for maximum stability and performance.

Built and tested using **PlatformIO in VS Code**.

---

## 🚀 Features

- ESP32 S3-based JPEG slideshow
- ILI9488 480x320 TFT display
- SD card JPEG loading
- Separate SPI buses (TFT + SD independent)
- Automatic image sequencing:
  - `/1.jpg`
  - `/2.jpg`
  - `/3.jpg`
  - ...etc
- Auto loop when reaching last image
- Optimized JPEG rendering using MCU blocks
- High-quality full-screen image display

---

## 🧠 Hardware Used

- ESP32 S3 (any compatible board)
- ILI9488 TFT display (480x320)
- MicroSD card module
- SD card (FAT32 formatted)

---

## 🔌 SPI Configuration

### TFT Display (SPI1 / default bus)
Handled by `TFT_eSPI` library configuration (`Platformio.ini`)

### SD Card (Separate SPI bus)
Custom SPI instance:

SPIClass sdSPI(HSPI);
sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

This prevents SPI conflicts between TFT and SD card.

---

## 📁 SD Card File Format

Images must be:

- Format: JPEG only (.jpg)
- Resolution: 320x480 (recommended)
- Naming format:
/1.jpg
/2.jpg
/3.jpg
...

### ⚠️ Important
- Use baseline JPEG
- Avoid progressive JPEG (may fail decoding)
- Keep file size ~100KB–200KB for best performance

---

## 🖼️ Image Preparation

Use any online resizer:
* https://www.photopea.com/

Recommended settings:

- Resize: 320x480
- Quality: 90–100%
- Format: JPG (baseline)

---

## ⚙️ How It Works
ESP32 S3 initializes TFT display
SD card is mounted using separate SPI bus
Program checks /1.jpg
Loads images sequentially:
checks file exists
decodes JPEG
renders MCU blocks to TFT
When no next image → resets to /1.jpg

---

## 🧩 Libraries Used
* TFT_eSPI
* JPEGDecoder
* SD
* SPI (ESP32 core)

---



