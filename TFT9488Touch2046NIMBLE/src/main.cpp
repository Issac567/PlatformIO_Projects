/* * =================================================================
 * PROJECT: Demo Uploader
 * HARDWARE: ESP32 + XPT2046 Touch + TFT Display + SD Card(Optional)
 * CONFIGURATION: Dual SPI (Separate Buses)
 * TFT9488 Seperate Bus. Touch2046/SD Card Shared Bus.
 * =================================================================
* * --- TFT DISPLAY (Bus 1: FSPI) ---
 * MOSI/SDI:   13                       -> Master Out Slave In
 * SCLK:       14                       -> Serial Clock
 * MISO/SDO:   -1                       -> (Not used by ILI9488)
 * CS:         10                       -> Chip Select
 * DC:          2                       -> Data/Command
 * RST:        12                       -> Reset 
 * * --- TOUCH SENSOR OR/AND SD Card (Shared Bus 2: HSPI) ---
 * TOUCH_DIN:  4 (MOSI)                 -> Data In to XPT2046
 * TOUCH_CLK:  5 (SCLK)                 -> Clock from ESP32 to XPT2046
 * TOUCH_DO:   6 (MISO)                 -> Data Out from XPT2046
 * TOUCH_CS:   15                       -> Touch Chip Select
 * TOUCH_IRQ:   1                       -> Touch Interrupt (Active LOW)
 * SD_MOSI      4                       -> Data In to SD Card
 * SD_MISO      6                       -> Data Out from SD Card
 * SD_SCK       5                       -> Clock from ESP32 to SD Card
 * SD_CS:      21                       -> SD Card Chip Select
 * * --- POWER & GROUND ---
 * VCC:    3.3V or 5V (Check Screen Regulator)
 * GND:    Common Ground
 * =================================================================
 */
// SD Card not used in this project but tested with it wired with Touch Display

#include <Arduino.h>
#include "globals.h"
#include "display_logic.h"
#include "ble.h"

// TFT and Touch Pins defined in Platformio.ini!

// Define the actual objects here (Allocation)
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
SPIClass touchSPI(HSPI); 

bool wasTouched = false;

void setup() 
{
    Serial.begin(115200);
    delay(2500); // Good delay for the S3 USB Serial to catch up. Can remove if debugging not needed!

    // --- IMPORTANT: QUIET THE SD CARD AND TOUCH ---
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);      // SD Idle
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);   // Touch Idle

    // 1. Initialize TFT (Uses Bus 1: FSPI)
    tft.init();
    tft.setRotation(1); // 480x320 Landscape

    // 2. Initialize Touch SPI (Uses Bus 2: HSPI)
    // Make sure these names match your -D flags in platformio.ini
    touchSPI.begin(TOUCH_SCLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS); 
    if (!touch.begin(touchSPI)) {
        Serial.println("Touchscreen init failed!");
    } else {
        Serial.println("Touchscreen online.");
    }
    touch.setRotation(1); // Keep this matching tft.setRotation

    // 3. INITIALIZE SD CARD (Pass the shared SPI) ---
    // Note: SD.begin takes (SS_PIN, SPI_BUS, FREQUENCY)
    if (!SD.begin(SD_CS, touchSPI, 16000000)) { // 16MHz is stable for S3
        Serial.println("SD Card mount failed!");
    } else {
        Serial.println("SD Card mounted.");
        //uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        //Serial.printf("SD Card Size: %lluMB\n", cardSize);
    }

    // 4. Draw main menu ui
    drawUI();
    
    Serial.println("Issac Engineer Go-Box Ready.");
}

void loop() 
{
    bool isCurrentlyTouched = touch.touched();

    //-------------------------------------------------------------------------------------------------
    // Check for touch
    //-------------------------------------------------------------------------------------------------
    if (isCurrentlyTouched && !wasTouched) {        // Only act if it's a NEW touch
        Serial.println("Handle Touch Pressed");
        handleTouch(); 
    }
    wasTouched = isCurrentlyTouched;                // Update the state

    //-------------------------------------------------------------------------------------------------
    // Do connecting to server
    //-------------------------------------------------------------------------------------------------
    if (doConnect == true && doScan == false) 
    {
        if (bleconnectToServer()) {
            Serial.println("We are now connected to the BLE Server.");
        } else {
            Serial.println("We have failed to connect to the server; there is nothing more we will do.");
            doScan = true; 
        }
    }

    //-------------------------------------------------------------------------------------------------
    // Do BLE scanning and find the device
    //-------------------------------------------------------------------------------------------------
    if (!bleIsConnected() && doConnect == false && doScan == true) 
    {
        Serial.println("BLE is not connected, attempting to reconnect...");
        bleDoScan();
    }
 
    delay(10);
}

