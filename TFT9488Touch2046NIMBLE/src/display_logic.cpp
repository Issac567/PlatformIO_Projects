#include "globals.h"
#include "display_logic.h"
#include "ble.h"
#include "sdcard.h"

MenuState currentMenu = MAIN;
bool wasTouched = false;

void initDisplaySystem() 
{
    // --- IMPORTANT: QUIET THE SD CARH ---
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
}

// --- TOUCH HANDLER ---
void handleTouch() 
{
    bool isCurrentlyTouched = touch.touched();

    if (isCurrentlyTouched && !wasTouched) 
    {   // Only act if it's a NEW touch
        Serial.println("Handle Touch Pressed");

        TS_Point p = touch.getPoint();
    
        // X is fine (Left to Right)
        int x = map(p.x, 300, 3800, 0, 480);
        
        // Y is FLIPPED (Top to Bottom)
        // By putting 3800 first, we tell the ESP32 that the 
        // "high" signal is actually the "top" of the screen.
        int y = map(p.y, 3800, 300, 0, 320); 

        if (currentMenu == MAIN) {
            if (x > 20 && x < 230 && y > 60 && y < 170)    changeMenu(FLASH);
            if (x > 250 && x < 460 && y > 60 && y < 170)   changeMenu(BT_CHECK);
            if (x > 20 && x < 230 && y > 190 && y < 300)   changeMenu(SYSTEM);
            if (x > 250 && x < 460 && y > 190 && y < 300)  changeMenu(ABOUT);
        } else if (currentMenu == FLASH) {
            if (x > 250 && x < 350 && y > 260 && y < 305)  changeMenu(MAIN);        // Flash button. Temp using back!
            if (x > 360 && x < 460 && y > 260 && y < 305)  changeMenu(MAIN);        // Back button
        } else {
            if (x > 360 && x < 460 && y > 260 && y < 305)  changeMenu(MAIN);        // Back button
        }
        // Debounce: wait for finger lift so we don't spam state changes
        while (touch.touched()) { delay(10); }
    }
    
    // Update the state
    wasTouched = isCurrentlyTouched;                
}

// --- CORE UI CONTROLLER ---
void drawUI() 
{
    tft.fillScreen(TFT_BLACK);
    
    switch (currentMenu) 
    {
        case MAIN:     drawMainMenu();    break;
        case FLASH:    drawFlashMenu();   break;
        case BT_CHECK: drawBTMenu();      break;
        case SYSTEM:   drawSystemMenu();  break;
        case ABOUT:    drawAboutMenu();   break;
    }
}

// --- MAIN MENU ---
void drawMainMenu() 
{
    tft.setTextColor(TFT_YELLOW);
    tft.drawCentreString("ISSAC ENGINEER UPLOADER", 240, 10, 4);
    
    // Grid: [Flash] [BT] / [System] [About]
    createButton(20,  60,  210, 110, TFT_RED,    "FLASH");
    createButton(250, 60,  210, 110, TFT_BLUE,   "CHECK BT");
    createButton(20,  190, 210, 110, TFT_GREEN,  "CHECK_SYS");
    createButton(250, 190, 210, 110, TFT_ORANGE, "ABOUT");
}

// --- SUB-MENUS ---
void drawFlashMenu() 
{
    // 1. Get the data from SD first
    String status = GetConfigInfo();

    if (status== "OK")
    {
        // 2. Setup Display
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Yellow for Headers
        
        // Header
        tft.setCursor(10, 10);
        tft.print("CONFIG: ");
        tft.println(myConfig.strPicName);
        tft.drawFastHLine(0, 30, 480, TFT_WHITE); // Separator line

        tft.setTextColor(TFT_WHITE, TFT_BLACK); // White for Data
        tft.setTextSize(1); // Drop size slightly to fit more info

        // Column 1 - Memory Info
        String strData;
        char buffer[64]; // Make sure it's big enough for the text
        sprintf(buffer, "Start Addr:  0x%04X", myConfig.intStartAddrFlash);  
        strData = String(buffer);
        tft.drawString(strData, 10, 45, 2);
        sprintf(buffer, "End Addr:  0x%04X", myConfig.intEndAddrFlash);  
        strData = String(buffer);
        tft.drawString(strData, 10, 65, 2);
        sprintf(buffer, "Empty Val:  0x%02X", myConfig.intEmptyFlashValue);  
        strData = String(buffer);
        tft.drawString(strData, 10, 85, 2);

        // Column 2 - Protocol Details
        int col2 = 240; 
        sprintf(buffer, "Pkt Size:  %d", myConfig.intInstructionPacket);  
        strData = String(buffer);
        tft.drawString(strData, col2, 45, 2);
        sprintf(buffer, "Pkt Delay:  %dms", myConfig.intPacketDelayMS);  
        strData = String(buffer);
        tft.drawString(strData, col2, 65, 2);
        sprintf(buffer, "H-Shake:  %dms", myConfig.intHandShakeDelayMS);  
        strData = String(buffer);
        tft.drawString(strData, col2, 85, 2);

        // Timing & UART
        sprintf(buffer, "Stop Bits:  %d", myConfig.intStopBit);  
        strData = String(buffer);
        tft.drawString(strData, 10, 115, 2);
        sprintf(buffer, "Exp Bytes:  %u", myConfig.intExpectedFirmwareBytes);  
        strData = String(buffer);
        tft.drawString(strData, 240, 115, 2);

        // 1. Burst Mode
        sprintf(buffer, "Burst: [%s]", myConfig.blnUseWriteBurst ? "ON" : "OFF");
        tft.drawString(buffer, 10, 145, 2);

        // 2. DoubleHex (Moved to 160 for better spacing on the 480px width)
        sprintf(buffer, "DoubleHex: [%s]", myConfig.blnUseDoubleHexAddr ? "ON" : "OFF");
        tft.drawString(buffer, 160, 145, 2);

        // 3. 4Padding (Moved to 320 to keep the columns even)
        sprintf(buffer, "4Padding: [%s]", myConfig.blnUse4Padding ? "ON" : "OFF");
        tft.drawString(buffer, 320, 145, 2);

        // Notes Section
        // 1. Draw Separator Line
        tft.drawFastHLine(0, 170, 480, TFT_DARKGREY);

        // 2. Draw Label
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString("NOTES:", 10, 180, 2); 

        // 3. Draw The Content
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(myConfig.strNotes, 10, 200, 2);

        drawFlashButton();
    } else {
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW, TFT_RED); 
        
        // Header
        tft.setCursor(10, 10);
        tft.print(status);
        tft.setTextSize(1);
    } 

    drawBackButton();
}

void drawBTMenu() 
{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    //tft.setTextSize(2); // Multiplies the size of the font you call next
    tft.drawCentreString("BLUETOOTH STATUS", 240, 20, 4);
    tft.setTextColor(TFT_CYAN);
    
    if (bleIsConnected()) 
    {
        // Update your ILI9488 display to show a green icon or "Online"
        tft.drawCentreString("BT Module: CONNECTED", 240, 120, 2);
        // DEVICE NAME
        String dName = myDevice->getName().c_str();
        if (dName == "") dName = "Unknown Device";
        tft.drawCentreString("Device: " + dName, 240, 140, 2);
        // MAC ADDRESS
        String macAddr = myDevice->getAddress().toString().c_str();
        tft.drawCentreString("MAC: " + macAddr, 240, 160, 2);
        // UUID
        String dUUID = myDevice->getServiceUUID().toString().c_str();
        tft.drawCentreString("Service UUID: " + dUUID, 240, 180, 2);

    } else {
        // Show "Offline" or "Scanning..."
         tft.drawCentreString("BT Module: DISCONNECTED", 240, 150, 2);
    }
    //tft.setTextSize(1); 
    drawBackButton();
}

void drawSystemMenu() 
{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("System Check", 240, 20, 4);
 
    // SD Card Check
    if (checkSDMount() == true)
    {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("[OK] SD Card Mount", 5, 80, 2);
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("[Fail] SD Card Mount", 5, 80, 2);
    }

    // Touch 2046 Check
    if (checkTouchMount() == true)
    {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("[OK] Touch 2046", 5, 100, 2);
    } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("[Fail] Touch 2046", 5, 100, 2);
    }
    
    drawBackButton();
    
}

void drawAboutMenu() 
{
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("ABOUT", 240, 20, 4);
    tft.drawString("Firmware: v1.0", 5, 80, 2);
    tft.drawString("Engineer: Issac Engineer", 5, 100, 2);
    tft.drawString("Hardware: ESP32 + ILI9488", 5, 120, 2);
    drawBackButton();
}

// --- UI HELPERS ---
void createButton(int x, int y, int w, int h, uint32_t color, const char *label) 
{
    tft.fillRoundRect(x, y, w, h, 10, color);
    tft.drawRoundRect(x, y, w, h, 10, TFT_WHITE);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString(label, x + (w / 2), y + (h / 2) - 10, 4);
}

void drawBackButton() 
{
    createButton(360, 260, 100, 45, TFT_DARKGREY, "BACK");
}

void drawFlashButton() 
{
    createButton(250, 260, 100, 45, TFT_DARKGREY, "FLASH");
}

void changeMenu(MenuState next) 
{
    if (currentMenu == next) return;  // prevents redraw spam
    currentMenu = next;
    drawUI();
}

// --- SYSTEM CHECKS ---
bool checkTouchMount()
{
    if (!touch.begin(touchSPI)) {
        return false;
    } else {
        return true;
    }
}
 
bool checkSDMount()
 {
    if (!SD.begin(SD_CS, touchSPI, 16000000)) { 
        return false;
    } else {
        return true;
    }
 }