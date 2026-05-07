// C-OS Ultimate Kernel v4.0
// Windows XP Edition - Full GUI Desktop OS
// Features: VESA Graphics, Keyboard Input, Virtual File System, XP UI

#include <stdint.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// ===== WINDOWS XP COLOR PALETTE =====
namespace Theme {
    // XP Luna Blue Theme
    constexpr u32 XP_BLUE       = 0xFF0054E3;  // Title bar blue
    constexpr u32 XP_BLUE_LIGHT = 0xFF3D95FF;  // Gradient light
    constexpr u32 XP_BLUE_DARK  = 0xFF003D99;  // Gradient dark
    constexpr u32 XP_GREEN      = 0xFF3D9400;  // Start button
    constexpr u32 XP_GRAY       = 0xFFE0E0E0;  // Window background
    constexpr u32 XP_GRAY_DARK  = 0xFF808080;  // Border
    constexpr u32 XP_WHITE      = 0xFFFFFFFF;  // Text background
    constexpr u32 XP_BLACK      = 0xFF000000;  // Text
    constexpr u32 XP_DESKTOP    = 0xFF3A6EA5;  // Desktop teal-blue
    
    // Taskbar
    constexpr u32 TASKBAR_BG    = 0xFF2368D3;
    constexpr u32 START_BTN     = 0xFF3D9400;
    
    // Window chrome
    constexpr u32 WINDOW_ACTIVE = 0xFF0054E3;
    constexpr u32 WINDOW_INACTIVE = 0xFF7A96DF;
}

// ===== VESA BIOS EXTENSIONS (VBE) =====
namespace VESA {
    static volatile u32* framebuffer = (u32*)0xE0000000;
    static int width = 1024;
    static int height = 768;
    static int pitch = 1024 * 4;
    static bool initialized = false;
    
    // VBE Mode Info Structure
    struct VbeModeInfo {
        u16 attributes;
        u8 winA, winB;
        u16 granularity;
        u16 winsize;
        u16 segmentA, segmentB;
        u32 realFctPtr;
        u16 pitch;
        u16 width, height;
        u8 wChar, yChar;
        u8 planes;
        u8 bpp;
        u8 banks;
        u8 memory_model;
        u8 bank_size;
        u8 image_pages;
        u8 reserved0;
        u8 red_mask, red_position;
        u8 green_mask, green_position;
        u8 blue_mask, blue_position;
        u8 rsv_mask, rsv_position;
        u8 directcolor_attributes;
        u32 framebuffer;
        u32 offscreen;
        u16 offscreensize;
        u8 reserved1[206];
    } __attribute__((packed));
    
    void initFromMultiboot(u32* mboot) {
        // Parse multiboot2 framebuffer tag
        if (mboot && mboot[0] == 0x36d76289) {
            u32* tag = mboot + 2;
            while (*tag != 0) {
                u32 type = tag[0];
                u32 size = tag[1];
                
                if (type == 5) { // Framebuffer tag
                    framebuffer = (u32*)tag[8];
                    width = tag[2];
                    height = tag[3];
                    pitch = tag[4];
                    initialized = true;
                    break;
                }
                tag = (u32*)((u8*)tag + ((size + 7) & ~7));
            }
        }
        
        // Fallback to default
        if (!initialized) {
            framebuffer = (u32*)0xE0000000;
            width = 1024;
            height = 768;
            pitch = 4096;
            initialized = true;
        }
    }
    
    inline void putPixel(int x, int y, u32 color) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        framebuffer[y * (pitch / 4) + x] = color;
    }
    
    void fillRect(int x, int y, int w, int h, u32 color) {
        for (int row = y; row < y + h && row < height; row++) {
            for (int col = x; col < x + w && col < width; col++) {
                putPixel(col, row, color);
            }
        }
    }
    
    void drawRect(int x, int y, int w, int h, u32 color) {
        for (int i = x; i < x + w; i++) {
            putPixel(i, y, color);
            putPixel(i, y + h - 1, color);
        }
        for (int i = y; i < y + h; i++) {
            putPixel(x, i, color);
            putPixel(x + w - 1, i, color);
        }
    }
    
    void clear(u32 color = Theme::XP_DESKTOP) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                putPixel(x, y, color);
            }
        }
    }
    
    int getWidth() { return width; }
    int getHeight() { return height; }
}

// ===== PS/2 KEYBOARD DRIVER =====
namespace Keyboard {
    static const u16 PORT_DATA = 0x60;
    static const u16 PORT_STATUS = 0x64;
    
    static volatile u8 lastKey = 0;
    static volatile bool keyPressed = false;
    
    // Simple scancode to ASCII
    static char scancodeToAscii(u8 scancode) {
        static const char map[] = {
            0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
            0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
            0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
            '*', 0, ' ', 0
        };
        if (scancode < sizeof(map)) return map[scancode];
        return 0;
    }
    
    void init() {
        // Clear buffer
        while (inb(PORT_STATUS) & 1) {
            inb(PORT_DATA);
        }
    }
    
    bool hasKey() {
        return inb(PORT_STATUS) & 1;
    }
    
    u8 readScancode() {
        return inb(PORT_DATA);
    }
    
    static inline u8 inb(u16 port) {
        u8 result;
        __asm__ __volatile__ ("inb %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
}

// ===== VIRTUAL FILE SYSTEM (Windows XP Structure) =====
namespace VFS {
    struct FileNode {
        char name[256];
        u32 type;  // 0=file, 1=directory
        u32 size;
        u8* data;
        FileNode* parent;
        FileNode* children;
        FileNode* next;
    };
    
    static FileNode* root = nullptr;
    
    void init() {
        // Create Windows XP directory structure
        root = new FileNode{"C:", 1, 0, nullptr, nullptr, nullptr, nullptr};
        
        // WINDOWS folder
        auto windows = new FileNode{"WINDOWS", 1, 0, nullptr, root, nullptr, root->children};
        root->children = windows;
        
        // WINDOWS\System32
        auto system32 = new FileNode{"System32", 1, 0, nullptr, windows, nullptr, windows->children};
        windows->children = system32;
        
        // Documents and Settings
        auto docs = new FileNode{"Documents and Settings", 1, 0, nullptr, root, nullptr, root->children};
        root->children = docs;
        
        // Administrator folder
        auto admin = new FileNode{"Administrator", 1, 0, nullptr, docs, nullptr, docs->children};
        docs->children = admin;
        
        // Desktop
        auto desktop = new FileNode{"Desktop", 1, 0, nullptr, admin, nullptr, admin->children};
        admin->children = desktop;
        
        // My Documents
        auto mydocs = new FileNode{"My Documents", 1, 0, nullptr, admin, desktop, admin->children};
        admin->children = mydocs;
        
        // Program Files
        auto progfiles = new FileNode{"Program Files", 1, 0, nullptr, root, docs, root->children};
        root->children = progfiles;
        
        // Add some sample files
        auto notepad = new FileNode{"notepad.exe", 0, 1024, nullptr, progfiles, nullptr, progfiles->children};
        progfiles->children = notepad;
    }
}

// ===== GUI COMPONENTS =====
namespace GUI {
    void drawDesktop() {
        // Clear with XP desktop color
        VESA::clear(Theme::XP_DESKTOP);
        
        // Draw desktop icons
        drawIcon(20, 20, "My Computer", Theme::XP_BLUE);
        drawIcon(20, 100, "My Documents", Theme::XP_BLUE);
        drawIcon(20, 180, "Recycle Bin", Theme::XP_GREEN);
        drawIcon(20, 260, "Notepad", Theme::XP_BLUE_LIGHT);
    }
    
    void drawIcon(int x, int y, const char* label, u32 color) {
        // Icon background (32x32)
        VESA::fillRect(x, y, 48, 48, Theme::XP_GRAY);
        VESA::drawRect(x, y, 48, 48, Theme::XP_GRAY_DARK);
        
        // Label (placeholder)
        // In real implementation, draw text here
    }
    
    void drawTaskbar() {
        int h = VESA::getHeight();
        int w = VESA::getWidth();
        
        // Taskbar background
        VESA::fillRect(0, h - 30, w, 30, Theme::TASKBAR_BG);
        
        // Start button
        VESA::fillRect(5, h - 25, 70, 20, Theme::START_BTN);
        VESA::drawRect(5, h - 25, 70, 20, Theme::XP_WHITE);
        
        // System tray area
        VESA::fillRect(w - 100, h - 25, 95, 20, Theme::TASKBAR_BG);
    }
    
    void drawWindow(int x, int y, int w, int h, const char* title, bool active) {
        u32 titleColor = active ? Theme::WINDOW_ACTIVE : Theme::WINDOW_INACTIVE;
        
        // Window frame
        VESA::drawRect(x - 2, y - 2, w + 4, h + 4, Theme::XP_GRAY_DARK);
        
        // Title bar (gradient simulation)
        for (int i = 0; i < 25; i++) {
            u32 gradient = (i < 12) ? Theme::XP_BLUE_LIGHT : Theme::XP_BLUE;
            VESA::fillRect(x, y + i, w, 1, gradient);
        }
        
        // Window content area
        VESA::fillRect(x, y + 25, w, h - 25, Theme::XP_GRAY);
    }
}

// ===== MAIN =====
extern "C" void kernel_main(u32 magic, u32* mboot) {
    // Initialize VESA graphics
    VESA::initFromMultiboot(mboot);
    
    // Initialize file system
    VFS::init();
    
    // Initialize keyboard
    Keyboard::init();
    
    // Draw initial desktop
    GUI::drawDesktop();
    GUI::drawTaskbar();
    
    // Main loop
    while (true) {
        // Check keyboard input
        if (Keyboard::hasKey()) {
            u8 scancode = Keyboard::readScancode();
            // Handle key input here
            // Redraw if needed
        }
        
        // Idle
        __asm__ __volatile__ ("hlt");
    }
}
