#ifndef DISPLAY_LOGIC_H
#define DISPLAY_LOGIC_H

// UI Prototypes
void handleTouch();
void drawUI();
void drawMainMenu();
void drawFlashMenu();
void drawBTMenu();
void drawCalibMenu();
void drawAboutMenu();
void createButton(int x, int y, int w, int h, uint32_t color, const char *label);
void drawBackButton();
void changeMenu(MenuState next);

#endif