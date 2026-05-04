#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <NimBLEDevice.h>   // This replaces ALL old BLE headers

// Functions to be called from main.cpp
bool bleconnectToServer();
bool bleIsConnected();
void bleDisconnect();
void bleDoScan();
void handleConnection();
void handleBleScan();

extern NimBLEAdvertisedDevice* myDevice;        // display_logic.cpp needs access to info

#endif