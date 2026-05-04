#ifndef DISPLAY_LOGIC_H
#define DISPLAY_LOGIC_H

#include <Arduino.h>

// UI Prototypes
void initDisplaySystem();
void handleTouch();
void drawUI();
void drawMainMenu();
void drawFlashMenu();
void drawBTMenu();
void drawSystemMenu();
void drawAboutMenu();
void createButton(int x, int y, int w, int h, uint32_t color, const char *label);
void drawBackButton();
void drawFlashButton();
void changeMenu(MenuState next);
bool checkTouchMount();
bool checkSDMount();

#endif