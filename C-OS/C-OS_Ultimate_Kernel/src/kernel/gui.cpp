// C-OS Ultimate - GUI Window System
// Text-mode GUI with mouse support

#include "include/types.h"

namespace VGA {
    void clear();
    void putc(char c);
    void puts(const char* s);
    void puthex(u32 n);
    void putdec(u32 n);
    void setCursor(int x, int y);
    void setColor(u8 fg, u8 bg);
    void writeChar(int x, int y, char c, u8 attr);
    u16 readChar(int x, int y);
    void fillRect(int x, int y, int w, int h, char c, u8 attr);
}

namespace Mouse {
    
    int getX();
    int getY();
    u8 getButtons();
    bool hasPacket();
    void clearPacket();
}

namespace GUI {
    // Forward declarations
    struct Window;
    void drawWindow(Window& w);
    void clearWindow(Window& w);
    
    // Screen dimensions (80x25 text mode)
    static const int SCREEN_WIDTH = 80;
    static const int SCREEN_HEIGHT = 25;
    
    // Colors
    static const u8 COLOR_BLACK = 0;
    static const u8 COLOR_BLUE = 1;
    static const u8 COLOR_GREEN = 2;
    static const u8 COLOR_CYAN = 3;
    static const u8 COLOR_RED = 4;
    static const u8 COLOR_MAGENTA = 5;
    static const u8 COLOR_BROWN = 6;
    static const u8 COLOR_LIGHT_GRAY = 7;
    static const u8 COLOR_DARK_GRAY = 8;
    static const u8 COLOR_LIGHT_BLUE = 9;
    static const u8 COLOR_LIGHT_GREEN = 10;
    static const u8 COLOR_LIGHT_CYAN = 11;
    static const u8 COLOR_LIGHT_RED = 12;
    static const u8 COLOR_LIGHT_MAGENTA = 13;
    static const u8 COLOR_YELLOW = 14;
    static const u8 COLOR_WHITE = 15;
    
    // Mouse cursor position (in character cells)
    static int mouseX = 40;
    static int mouseY = 12;
    static int prevMouseX = 40;
    static int prevMouseY = 12;
    static u8 prevMouseChar = ' ';
    static u8 prevMouseAttr = 0x07;
    static bool mouseVisible = false;
    static bool mouseLeftButton = false;
    static bool mouseLeftButtonPrev = false;
    
    // Window dragging state
    static bool isDragging = false;
    static int dragWindowIndex = -1;
    static int dragOffsetX = 0;
    static int dragOffsetY = 0;
    
    // Window structure
    struct Window {
        int x, y;
        int width, height;
        char title[32];
        bool visible;
        bool active;
        u8 bgColor;
        u8 fgColor;
        u8 borderColor;
        bool hasBorder;
    };
    
    static const int MAX_WINDOWS = 8;
    static Window windows[MAX_WINDOWS];
    static int windowCount = 0;
    static int activeWindow = -1;
    
    // Initialize GUI system (Windows style desktop)
    void init() {
        // Clear screen with Windows 95 style blue desktop
        VGA::clear();
        
        // Clear windows
        for (int i = 0; i < MAX_WINDOWS; i++) {
            windows[i].visible = false;
            windows[i].active = false;
        }
        windowCount = 0;
        activeWindow = -1;
        
        // Draw desktop background
        u8 desktopAttr = (COLOR_BLUE << 4) | COLOR_WHITE;
        for (int y = 0; y < SCREEN_HEIGHT - 1; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                VGA::writeChar(x, y, ' ', desktopAttr);
            }
        }
        
        // Draw simple taskbar at bottom
        u8 taskbarAttr = (COLOR_LIGHT_GRAY << 4) | COLOR_BLACK;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            VGA::writeChar(x, SCREEN_HEIGHT - 1, ' ', taskbarAttr);
        }
        // Start button
        VGA::writeChar(0, SCREEN_HEIGHT - 1, 'S', (COLOR_DARK_GRAY << 4) | COLOR_WHITE);
        VGA::writeChar(1, SCREEN_HEIGHT - 1, 't', (COLOR_DARK_GRAY << 4) | COLOR_WHITE);
        VGA::writeChar(2, SCREEN_HEIGHT - 1, 'a', (COLOR_DARK_GRAY << 4) | COLOR_WHITE);
        VGA::writeChar(3, SCREEN_HEIGHT - 1, 'r', (COLOR_DARK_GRAY << 4) | COLOR_WHITE);
        VGA::writeChar(4, SCREEN_HEIGHT - 1, 't', (COLOR_DARK_GRAY << 4) | COLOR_WHITE);
    }
    
    // Draw window shadow (Windows style)
    void drawShadow(Window& w) {
        u8 shadowAttr = (COLOR_BLACK << 4) | COLOR_BLACK;
        // Right shadow
        for (int j = 1; j < w.height; j++) {
            if (w.x + w.width < SCREEN_WIDTH && w.y + j < SCREEN_HEIGHT)
                VGA::writeChar(w.x + w.width, w.y + j, 176, shadowAttr);
        }
        // Bottom shadow
        for (int i = 1; i <= w.width && (w.x + i) < SCREEN_WIDTH; i++) {
            if (w.y + w.height < SCREEN_HEIGHT)
                VGA::writeChar(w.x + i, w.y + w.height, 176, shadowAttr);
        }
    }
    
    // Draw window border and title (Windows style with box drawing chars)
    void drawWindow(Window& w) {
        if (!w.visible) return;
        
        // Draw shadow first
        drawShadow(w);
        
        // Title bar colors
        u8 titleBarAttr = w.active ? 
            ((COLOR_BLUE << 4) | COLOR_WHITE) : 
            ((COLOR_DARK_GRAY << 4) | COLOR_LIGHT_GRAY);
        u8 borderAttr = (w.borderColor << 4) | w.fgColor;
        
        // Draw top-left corner
        VGA::writeChar(w.x, w.y, 201, borderAttr);  // ╔
        
        // Draw title bar
        int titleLen = 0;
        while (w.title[titleLen] && titleLen < w.width - 4) titleLen++;
        
        for (int i = 1; i < w.width - 1; i++) {
            char c = ' ';
            if (i == w.width - 2) {
                c = 'X';  // Close button
            } else if (i >= 1 && i <= titleLen) {
                c = w.title[i-1];
            }
            VGA::writeChar(w.x + i, w.y, c, titleBarAttr);
        }
        
        // Draw top-right corner
        VGA::writeChar(w.x + w.width - 1, w.y, 187, borderAttr);  // ╗
        
        // Draw side borders
        for (int j = 1; j < w.height - 1; j++) {
            VGA::writeChar(w.x, w.y + j, 186, borderAttr);  // ║
            VGA::writeChar(w.x + w.width - 1, w.y + j, 186, borderAttr);  // ║
        }
        
        // Draw bottom corners and border
        VGA::writeChar(w.x, w.y + w.height - 1, 200, borderAttr);  // ╚
        for (int i = 1; i < w.width - 1; i++) {
            VGA::writeChar(w.x + i, w.y + w.height - 1, 205, borderAttr);  // ═
        }
        VGA::writeChar(w.x + w.width - 1, w.y + w.height - 1, 188, borderAttr);  // ╝
    }
    
    // Erase window (restore desktop background)
    void eraseWindow(Window& w) {
        if (!w.visible) return;
        
        // Fill entire window area with desktop background color
        u8 desktopAttr = (COLOR_BLUE << 4) | COLOR_WHITE;
        for (int j = 0; j < w.height; j++) {
            for (int i = 0; i < w.width; i++) {
                VGA::writeChar(w.x + i, w.y + j, ' ', desktopAttr);
            }
        }
        
        // Also erase shadow if exists (1 char right and bottom)
        if (w.x + w.width < SCREEN_WIDTH) {
            for (int j = 1; j < w.height + 1 && (w.y + j) < SCREEN_HEIGHT; j++) {
                VGA::writeChar(w.x + w.width, w.y + j, ' ', desktopAttr);
            }
        }
        if (w.y + w.height < SCREEN_HEIGHT) {
            for (int i = 0; i < w.width + 1 && (w.x + i) < SCREEN_WIDTH; i++) {
                VGA::writeChar(w.x + i, w.y + w.height, ' ', desktopAttr);
            }
        }
    }
    
    // Clear window content area
    void clearWindow(Window& w) {
        if (!w.visible) return;
        
        u8 bgAttr = (w.bgColor << 4) | w.fgColor;
        for (int j = 1; j < w.height - 1; j++) {
            for (int i = 1; i < w.width - 1; i++) {
                VGA::writeChar(w.x + i, w.y + j, ' ', bgAttr);
            }
        }
    }
    
    // Create window (Windows style)
    int createWindow(int x, int y, int w, int h, const char* title) {
        if (windowCount >= MAX_WINDOWS) return -1;
        
        int id = windowCount++;
        Window& win = windows[id];
        
        win.x = x;
        win.y = y;
        win.width = w;
        win.height = h;
        win.visible = true;
        win.active = (activeWindow == -1);
        win.bgColor = COLOR_LIGHT_GRAY;
        win.fgColor = COLOR_BLACK;
        win.borderColor = COLOR_BLUE;
        win.hasBorder = true;
        
        // Copy title
        int i = 0;
        while (title[i] && i < 31) {
            win.title[i] = title[i];
            i++;
        }
        win.title[i] = '\0';
        
        if (win.active) activeWindow = id;
        
        drawWindow(win);
        clearWindow(win);
        
        return id;
    }
    
    // Draw mouse cursor - bright blinking cursor
    void drawMouseCursor() {
        if (!mouseVisible) return;
        
        // Bounds check
        if (mouseX < 0 || mouseX >= SCREEN_WIDTH || 
            mouseY < 0 || mouseY >= SCREEN_HEIGHT) return;
        
        // Draw cursor as BRIGHT WHITE block on BLACK background (maximum contrast)
        // Use character 219 (█) for solid block - very visible
        u8 cursorAttr = (COLOR_BLACK << 4) | (COLOR_WHITE | 0x08);  // Bright white on black
        VGA::writeChar(mouseX, mouseY, 219, cursorAttr);  // █ - full solid block
    }
    
    // Erase previous mouse cursor position
    void eraseMouseCursor() {
        if (prevMouseX >= 0 && prevMouseX < SCREEN_WIDTH &&
            prevMouseY >= 0 && prevMouseY < SCREEN_HEIGHT) {
            // Read what was actually there before we drew cursor
            u16 cell = VGA::readChar(prevMouseX, prevMouseY);
            char c = cell & 0xFF;
            u8 attr = (cell >> 8) & 0xFF;
            
            // If it was our cursor (black bg, bright white char, block), restore desktop
            if (c == 219 && attr == ((COLOR_BLACK << 4) | (COLOR_WHITE | 0x08))) {
                // Restore desktop background (blue with white text)
                VGA::writeChar(prevMouseX, prevMouseY, ' ', (COLOR_BLUE << 4) | COLOR_WHITE);
            }
            // Otherwise restore original content
            else {
                VGA::writeChar(prevMouseX, prevMouseY, c, attr);
            }
        }
    }
    
    // Update mouse position
    void updateMouse() {
        if (!Mouse::hasPacket()) return;
        
        // Convert mouse pixel coordinates to character cells
        // Mouse returns relative movement, we maintain absolute position
        int rawX = Mouse::getX();
        int rawY = Mouse::getY();
        
        // Convert to character cells (8x16 font assumed)
        int newX = rawX / 8;
        int newY = rawY / 16;
        
        // Clamp to screen
        if (newX < 0) newX = 0;
        if (newX >= SCREEN_WIDTH) newX = SCREEN_WIDTH - 1;
        if (newY < 0) newY = 0;
        if (newY >= SCREEN_HEIGHT) newY = SCREEN_HEIGHT - 1;
        
        // Only redraw if position changed
        if (newX != mouseX || newY != mouseY) {
            eraseMouseCursor();
            prevMouseX = mouseX;
            prevMouseY = mouseY;
            mouseX = newX;
            mouseY = newY;
            drawMouseCursor();
        }
        
        // Handle mouse buttons
        u8 buttons = Mouse::getButtons();
        mouseLeftButton = (buttons & 0x01) != 0;
        
        // Button just pressed
        if (mouseLeftButton && !mouseLeftButtonPrev) {
            // Check for window title bar click (top line of window)
            for (int i = windowCount - 1; i >= 0; i--) {
                Window& w = windows[i];
                if (w.visible && 
                    mouseX >= w.x && mouseX < w.x + w.width &&
                    mouseY == w.y) {  // Title bar is at y position
                    
                    // Activate window
                    if (activeWindow != i) {
                        if (activeWindow >= 0) {
                            windows[activeWindow].active = false;
                            drawWindow(windows[activeWindow]);
                        }
                        activeWindow = i;
                        w.active = true;
                        drawWindow(w);
                    }
                    
                    // Start dragging
                    isDragging = true;
                    dragWindowIndex = i;
                    dragOffsetX = mouseX - w.x;
                    dragOffsetY = mouseY - w.y;
                    break;
                }
            }
        }
        
        // Button released
        if (!mouseLeftButton && mouseLeftButtonPrev) {
            isDragging = false;
            dragWindowIndex = -1;
        }
        
        // Update previous state
        mouseLeftButtonPrev = mouseLeftButton;
        
        // Handle dragging
        if (isDragging && dragWindowIndex >= 0) {
            Window& w = windows[dragWindowIndex];
            
            // Calculate new position
            int newWinX = mouseX - dragOffsetX;
            int newWinY = mouseY - dragOffsetY;
            
            // Clamp to screen bounds
            if (newWinX < 0) newWinX = 0;
            if (newWinX + w.width > SCREEN_WIDTH) newWinX = SCREEN_WIDTH - w.width;
            if (newWinY < 0) newWinY = 0;
            if (newWinY + w.height > SCREEN_HEIGHT) newWinY = SCREEN_HEIGHT - w.height;
            
            // Only redraw if position changed
            if (newWinX != w.x || newWinY != w.y) {
                // Erase old window (redraw background)
                eraseWindow(w);
                
                // Update position
                w.x = newWinX;
                w.y = newWinY;
                
                // Draw at new position
                drawWindow(w);
            }
        }
        
        Mouse::clearPacket();
    }
    
    // Draw text in window
    void drawText(int windowId, int x, int y, const char* text) {
        if (windowId < 0 || windowId >= windowCount) return;
        Window& w = windows[windowId];
        if (!w.visible) return;
        
        u8 attr = (w.bgColor << 4) | w.fgColor;
        int cx = w.x + 1 + x;
        int cy = w.y + 1 + y;
        
        while (*text && cx < w.x + w.width - 1) {
            VGA::writeChar(cx, cy, *text, attr);
            text++;
            cx++;
        }
    }
    
    // Draw button in window
    void drawButton(int windowId, int x, int y, int w, const char* text, bool pressed) {
        if (windowId < 0 || windowId >= windowCount) return;
        Window& win = windows[windowId];
        if (!win.visible) return;
        
        u8 bg = pressed ? COLOR_DARK_GRAY : COLOR_LIGHT_GRAY;
        u8 fg = COLOR_BLACK;
        u8 attr = (bg << 4) | fg;
        
        int cx = win.x + 1 + x;
        int cy = win.y + 1 + y;
        
        // Draw button border
        for (int i = 0; i < w; i++) {
            VGA::writeChar(cx + i, cy, pressed ? 223 : 220, attr);  // Top/Bottom
        }
        
        // Draw text (centered)
        int textLen = 0;
        while (text[textLen]) textLen++;
        int textX = cx + (w - textLen) / 2;
        
        for (int i = 0; text[i] && i < w - 2; i++) {
            VGA::writeChar(textX + i, cy - 1, text[i], attr);
        }
    }
    
    // Main GUI update loop
    void update() {
        updateMouse();
    }
    
    // Show/hide mouse cursor
    void showMouse(bool show) {
        if (show && !mouseVisible) {
            mouseVisible = true;
            drawMouseCursor();
        } else if (!show && mouseVisible) {
            eraseMouseCursor();
            mouseVisible = false;
        }
    }
    void getMousePos(int& x, int& y) {
        x = mouseX;
        y = mouseY;
    }
    
    // Get active window
    int getActiveWindow() {
        return activeWindow;
    }
    
    // Close window
    void closeWindow(int id) {
        if (id < 0 || id >= windowCount) return;
        windows[id].visible = false;
        // Redraw area (simplified: just clear)
        VGA::clear();
        // Redraw remaining windows
        for (int i = 0; i < windowCount; i++) {
            if (windows[i].visible) {
                drawWindow(windows[i]);
                clearWindow(windows[i]);
            }
        }
    }
}
