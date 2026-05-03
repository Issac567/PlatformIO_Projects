#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 21

// Menu States
enum MenuState { MAIN, FLASH, BT_CHECK, CALIB, ABOUT };

// External references so other files can see them
extern TFT_eSPI tft;
extern XPT2046_Touchscreen touch;
extern SPIClass touchSPI;

#endif