// C-OS Ultimate - GUI Window System Header
#ifndef GUI_H
#define GUI_H

#include "include/types.h"

namespace GUI {
    // Initialize GUI system
    void init();
    
    // Window management
    int createWindow(int x, int y, int width, int height, const char* title);
    void closeWindow(int id);
    void drawText(int windowId, int x, int y, const char* text);
    void drawButton(int windowId, int x, int y, int width, const char* text, bool pressed);
    int getActiveWindow();
    
    // Mouse cursor
    void showMouse(bool show);
    void getMousePos(int& x, int& y);
    
    // Main update (call regularly)
    void update();
}

#endif // GUI_H
