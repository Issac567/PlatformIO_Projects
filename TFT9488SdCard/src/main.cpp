/* * =================================================================
 * PROJECT: Image Slide Show
 * HARDWARE: ESP32 + XPT2046 Touch + TFT Display
 * CONFIGURATION: Dual SPI (Separate Buses)
 * =================================================================
* * --- TFT DISPLAY (Bus 1: FSPI) ---
 * MOSI/SDI:   13                       -> Master Out Slave In
 * SCLK:       14                       -> Serial Clock
 * MISO/SDO:   -1                       -> (Not used by ILI9488)
 * CS:         10                       -> Chip Select
 * DC:          2                       -> Data/Command
 * RST:        12                       -> Reset 
 * * --- TOUCH SENSOR OR/AND SD Card (Shared Bus 2: HSPI) ---
 * TOUCH_DIN:  4 (MOSI)                 -> Data In to XPT2046/SD Card
 * TOUCH_CLK:  5 (SCLK)                 -> Clock from ESP32 to XPT2046/SD Card
 * TOUCH_DO:   6 (MISO)                 -> Data Out from XPT2046/SD Card
 * TOUCH_CS:   15                       -> Touch Chip Select
 * TOUCH_IRQ:   1                       -> Touch Interrupt (Active LOW)
 * SD_CS:      21                       -> SD Card Chip Select
 * * --- POWER & GROUND ---
 * VCC:    3.3V or 5V (Check Screen Regulator)
 * GND:    Common Ground
 * =================================================================
 */
// Touch not used in this Demo!
 

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <JPEGDecoder.h>  // JPEG decoder library
#include <SD.h>

// ---------- PROTOTYPES ----------
void drawSdJpeg(const char *filename, int xpos, int ypos);
void renderJPEG(int xpos, int ypos);
void jpegInfo();

// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

// --- SD Card Pins (Separate Bus / SPI3) ---
// Use the safe pins we chose above
#define SD_MOSI 4
#define SD_MISO 6
#define SD_SCK  5
#define SD_CS   21

// TFT Pins defined in Platformio.ini!

TFT_eSPI tft = TFT_eSPI(); 
SPIClass sdSPI(HSPI); // Create a separate SPI instance for the SD card

int fileIndex = 1;

//####################################################################################################
// Setup
//####################################################################################################
void setup() {
  Serial.begin(115200);
  delay(2500);


  // 1. Initialize TFT (Uses SPI2 as configured in User_Setup.h)
  tft.init();
  tft.setRotation(0); // 0 = Portrait, 1 = Landscape
  tft.fillScreen(TFT_BLACK);
  
  // If your screen colors look like a negative/reverse, uncomment this:
  // tft.invertDisplay(true);

  // 2. Initialize the SECOND SPI bus for the SD card
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // 3. Start SD using the specific SPI instance
  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("SD Card Mount Failed!");
    tft.setTextColor(TFT_RED);
    tft.println("SD Error");
    return;
  }
  
  Serial.println("SD Card Initialized on HSPI.");
}

void loop() 
{
  char filename[32];
  
  // Format the name based on the current index
  sprintf(filename, "/%d.jpg", fileIndex);

  if (SD.exists(filename)) {
    Serial.print("Loading: "); Serial.println(filename);
    
    // Clear the screen or draw background before the next image
    // tft.fillScreen(TFT_BLACK); 

    drawSdJpeg(filename, 0, 0); 
    
    fileIndex++;     // Increment for next loop
    delay(5000);     // View time
  } 
  else {
    // If we reach 11.jpg and it doesn't exist, go back to 1.jpg
    Serial.println("Resetting index to 1...");
    fileIndex = 1;
    
    // If even 1.jpg is missing, don't spam the SD card
    if (!SD.exists("/1.jpg")) {
      Serial.println("Error: No images found!");
      delay(5000); 
    }
  }
}

void drawSdJpeg(const char *filename, int xpos, int ypos) {
  File jpgFile = SD.open(filename);
  if (!jpgFile) {
    Serial.print("File not found: "); Serial.println(filename);
    return;
  }

  // Use the SD-specific decode function
  bool decoded = JpegDec.decodeSdFile(jpgFile);

  if (decoded) {
    renderJPEG(xpos, ypos);
  }
  jpgFile.close();
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void renderJPEG(int xpos, int ypos) {

  // retrieve information about the image
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // read each MCU block until there are no more
  while (JpegDec.read()) {
	  
    // save a pointer to the image block
    pImg = JpegDec.pImage ;

    // calculate where the image block should be drawn on the screen
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;  // Calculate coordinates of top left corner of current MCU
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;

    tft.startWrite();

    // draw image MCU block only if it will fit on the screen
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
    {

      // Now set a MCU bounding window on the TFT to push pixels into (x, y, x + width - 1, y + height - 1)
      tft.setAddrWindow(mcu_x, mcu_y, win_w, win_h);

      // Write all MCU pixels to the TFT window
      while (mcu_pixels--) {
        // Push each pixel to the TFT MCU area
        tft.pushColor(*pImg++);
      }

    }
    else if ( (mcu_y + win_h) >= tft.height()) JpegDec.abort(); // Image has run off bottom of screen so abort decoding

    tft.endWrite();
  }

  // calculate how long it took to draw the image
  drawTime = millis() - drawTime;

  // print the results to the serial port
  Serial.print(F(  "Total render time was    : ")); Serial.print(drawTime); Serial.println(F(" ms"));
  Serial.println(F(""));
}

//####################################################################################################
// Print image information to the serial port (optional)
//####################################################################################################
void jpegInfo() {
  Serial.println(F("==============="));
  Serial.println(F("JPEG image info"));
  Serial.println(F("==============="));
  Serial.print(F(  "Width      :")); Serial.println(JpegDec.width);
  Serial.print(F(  "Height     :")); Serial.println(JpegDec.height);
  Serial.print(F(  "Components :")); Serial.println(JpegDec.comps);
  Serial.print(F(  "MCU / row  :")); Serial.println(JpegDec.MCUSPerRow);
  Serial.print(F(  "MCU / col  :")); Serial.println(JpegDec.MCUSPerCol);
  Serial.print(F(  "Scan type  :")); Serial.println(JpegDec.scanType);
  Serial.print(F(  "MCU width  :")); Serial.println(JpegDec.MCUWidth);
  Serial.print(F(  "MCU height :")); Serial.println(JpegDec.MCUHeight);
  Serial.println(F("==============="));
}


//####################################################################################################
// WARNING: for UNO/AVR legacy reasons printing text to the screen with the Mega might not work for
// sketch sizes greater than ~70KBytes because 16-bit address pointers are used in some libraries.

