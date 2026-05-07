// C-OS Ultimate - VGA Text Mode Driver
#include "include/types.h"

namespace VGA {
    static u16* buffer = (u16*)0xB8000;
    static int row = 0;
    static int col = 0;
    static const int ROWS = 25;
    static const int COLS = 80;
    
    void clear() {
        for (int i = 0; i < ROWS * COLS; i++) {
            buffer[i] = (0x1F << 8) | ' ';
        }
        row = 0;
        col = 0;
    }
    
    // Current color attribute
    static u8 currentAttr = 0x1F;  // White on blue
    
    void setColor(u8 fg, u8 bg) {
        currentAttr = (bg << 4) | fg;
    }
    
    void putc(char c) {
        if (c == '\n') {
            row++;
            col = 0;
        } else if (c == '\r') {
            col = 0;
        } else {
            if (row >= ROWS) {
                for (int i = 0; i < (ROWS-1) * COLS; i++) {
                    buffer[i] = buffer[i + COLS];
                }
                for (int i = (ROWS-1) * COLS; i < ROWS * COLS; i++) {
                    buffer[i] = (0x1F << 8) | ' ';
                }
                row = ROWS - 1;
                col = 0;
            }
            buffer[row * COLS + col] = (0x1F << 8) | c;
            col++;
            if (col >= COLS) {
                col = 0;
                row++;
            }
        }
    }
    
    void puts(const char* s) {
        while (*s) putc(*s++);
    }
    
    void puthex(u32 n) {
        const char* hex = "0123456789ABCDEF";
        for (int i = 28; i >= 0; i -= 4) {
            putc(hex[(n >> i) & 0xF]);
        }
    }
    
    void putdec(u32 n) {
        if (n == 0) {
            putc('0');
            return;
        }
        char buf[12];
        int i = 0;
        while (n > 0) {
            buf[i++] = '0' + (n % 10);
            n /= 10;
        }
        while (i > 0) {
            putc(buf[--i]);
        }
    }
    
    // Direct character write with attribute (for GUI)
    void writeChar(int x, int y, char c, u8 attr) {
        if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return;
        buffer[y * COLS + x] = (attr << 8) | (u8)c;
    }
    
    // Read character at position
    u16 readChar(int x, int y) {
        if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return 0;
        return buffer[y * COLS + x];
    }
    
    // Fill rectangle with character
    void fillRect(int x, int y, int w, int h, char c, u8 attr) {
        for (int row = y; row < y + h && row < ROWS; row++) {
            for (int col = x; col < x + w && col < COLS; col++) {
                if (col >= 0 && row >= 0) {
                    writeChar(col, row, c, attr);
                }
            }
        }
    }
    
    // Set cursor position
    void setCursor(int x, int y) {
        col = x;
        row = y;
    }
}
