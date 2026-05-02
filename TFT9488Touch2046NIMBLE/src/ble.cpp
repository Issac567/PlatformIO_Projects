#include <Arduino.h>
//#include <NimBLEDevice.h>
#include "ble.h"

// Shared variables (extern from header)
bool doConnect = false;
bool doScan = false;

NimBLEAdvertisedDevice* myDevice = nullptr;

// Internal (file-local) variables
static String targetDeviceName = "HM10";
static NimBLEUUID charUUID((uint16_t)0xFFE1);
static NimBLEUUID serviceUUID("0000ffe0-0000-1000-8000-00805f9b34fb");
static NimBLERemoteCharacteristic* pRemoteCharacteristic = nullptr;
static NimBLEClient* pClient = nullptr;

//-----------------------------------------------------------
// Callback function to handle notifications
//-----------------------------------------------------------
static void notifyCallback(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) 
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pRemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    // NimBLE is very efficient; pData points directly to the received buffer
    Serial.write(pData, length);
    Serial.println();
}

//-----------------------------------------------------------
//Connect/Disconnect event
//-----------------------------------------------------------
class MyClientCallback : public NimBLEClientCallbacks 
{
    void onConnect(NimBLEClient* pClient) 
    {
        Serial.println(" - onConnect");
    }

    void onDisconnect(NimBLEClient* pClient) 
    {
        Serial.println(" - onDisconnect");
    }
};

//-----------------------------------------------------------
// * Scan for BLE servers and find the first one that advertises the service we are looking for.
//-----------------------------------------------------------
class MyAdvertisedDeviceCallbacks : public NimBLEScanCallbacks {
public:
    
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        Serial.println("------------------------------");
        Serial.println("BLE Advertised Device found:");

        if (advertisedDevice->haveName()) 
        {
            Serial.println(advertisedDevice->getName().c_str());
        } else {
            Serial.println("(none)");
        }

        Serial.println(advertisedDevice->getAddress().toString().c_str());
        Serial.println(advertisedDevice->getRSSI());

        bool foundByUUID = advertisedDevice->isAdvertisingService(serviceUUID);
        bool foundByName = false;
        if (advertisedDevice->haveName()) {
            foundByName = (String(advertisedDevice->getName().c_str()) == targetDeviceName);
        }

        if (foundByUUID || foundByName) 
        {
            NimBLEDevice::getScan()->stop();
            
            if (myDevice) 
            {
                delete myDevice;
                myDevice = nullptr;
            }

            myDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
            doScan = true;
            doConnect = true;
            Serial.println("Ready to connect...");
        }
    }
};

//-----------------------------------------------------------
//Connection to server
//-----------------------------------------------------------
bool connectToServer() 
{
    doConnect = false;
    
    // 1. Important to delete client or it will crash anytime you reconnect.
    if (pClient != nullptr) 
    {
        Serial.println("Cleaning up old client memory...");
        pClient->disconnect();
        NimBLEDevice::deleteClient(pClient); 
        pClient = nullptr; 
    }
        
    Serial.println("Starting Pre-Flight Check...");
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    // 2. NimBLE uses NimBLEDevice
    pClient = NimBLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback()); // 'true' handles automatic deletion

    // 3. Connect to the remote server. 
    // pClient->setConnectionParams(12, 24, 0, 400);
    if (!pClient->connect(myDevice)) 
    {
        Serial.println(" - Failed to connect to server");
        NimBLEDevice::deleteClient(pClient);
        return false;
    }
    Serial.println(" - Connected to server");
    
    // 4. Obtain a reference to the service
    NimBLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) 
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our service");
    
    // 5. Obtain a reference to the characteristic
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) 
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }

    // 6. Subscription Logic
    if (pRemoteCharacteristic->canNotify()) 
    {
        // NimBLE uses subscribe() instead of registerForNotify()
        // First param is true for notifications, false for indications
        if (pRemoteCharacteristic->subscribe(true, notifyCallback)) 
        
        {
        Serial.println(" - Subscribed to notifications");
        } 
        else 
        {
        Serial.println(" - Failed to subscribe");
        pClient->disconnect();
        return false;
        }
    }
    else
    {
        Serial.println(" - Characteristic does not support notifications");
        pClient->disconnect();
        return false;
    }

    return true;
}

//-----------------------------------------------------------
//Setup
//-----------------------------------------------------------
void bleDoScan() 
{
    doScan = false;

    Serial.println("Starting Arduino NimBLE Client application...");
    NimBLEDevice::init("");

    NimBLEScan* pBLEScan = NimBLEDevice::getScan();

    pBLEScan->setScanCallbacks(new MyAdvertisedDeviceCallbacks(), true);

    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(90);

    pBLEScan->start(0, false);
}

//-----------------------------------------------------------
//Check connection
//-----------------------------------------------------------
bool bleIsConnected()
{
    if (pClient && pClient->isConnected())
        return true;

    return false;
}

//-----------------------------------------------------------
//Call Disconnect
//-----------------------------------------------------------
void bleDisconnect()
{
    if (pClient) {
        pClient->disconnect();
    }
}
