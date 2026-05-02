#ifndef BLE_H
#define BLE_H

#include <NimBLEDevice.h>   // This replaces ALL old BLE headers

// Functions to be called from main.cpp
bool bleconnectToServer();
bool bleIsConnected();
void bleDisconnect();
void bleDoScan();

//extern bool connected;
extern bool doConnect;
extern bool doScan;

// Device + service tracking
extern NimBLEAdvertisedDevice* myDevice;

#endif