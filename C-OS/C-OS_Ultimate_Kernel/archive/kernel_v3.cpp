// C-OS Ultimate Kernel v3.0
// Enhanced with v6_Enhanced Design Features
// Features: High-res, Multi-tasking, Taskbar, Right-click menu, Rounded UI

#include <stdint.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// ===== CATPPUCCIN MOCHA COLOR PALETTE (from v6_Enhanced) =====
namespace Theme {
    // Base colors
    constexpr u32 BASE     = 0xFF1E1E2E;  // #1e1e2e - Background
    constexpr u32 MANTLE   = 0xFF181825;  // #181825 - Darker bg
    constexpr u32 CRUST    = 0xFF11111B;  // #11111b - Darkest
    
    // Text colors
    constexpr u32 TEXT     = 0xFFCDD6F4;  // #cdd6f4 - Primary text
    constexpr u32 SUBTEXT0 = 0xFFA6ADC8;  // #a6adc8 - Secondary
    constexpr u32 SUBTEXT1 = 0xFFB4BEFE;  // #b4befe - Tertiary
    
    // Accent colors
    constexpr u32 LAVENDER = 0xFFB4BEFE;  // #b4befe - Primary accent
    constexpr u32 BLUE     = 0xFF89B4FA;  // #89b4fa
    constexpr u32 SAPPHIRE = 0xFF74C7EC;  // #74c7ec
    constexpr u32 SKY      = 0xFF89DCEB;  // #89dceb
    constexpr u32 TEAL     = 0xFF94E2D5;  // #94e2d5
    constexpr u32 GREEN    = 0xFFA6E3A1;  // #a6e3a1
    constexpr u32 YELLOW   = 0xFFF9E2AF;  // #f9e2af
    constexpr u32 PEACH    = 0xFFFAB387;  // #fab387
    constexpr u32 MAROON   = 0xFFEBA0AC;  // #eba0ac
    constexpr u32 RED      = 0xFFF38BA8;  // #f38ba8
    constexpr u32 MAUVE    = 0xFFCBA6F7;  // #cba6f7
    constexpr u32 PINK     = 0xFFF5C2E7;  // #f5c2e7
    constexpr u32 FLAMINGO = 0xFFF2CDCD;  // #f2cdcd
    constexpr u32 ROSEWATER= 0xFFF5E0DC;  // #f5e0dc
    
    // Surface colors
    constexpr u32 SURFACE0 = 0xFF313244;  // #313244
    constexpr u32 SURFACE1 = 0xFF45475A;  // #45475a
    constexpr u32 SURFACE2 = 0xFF585B70;  // #585b70
    
    // Overlay with alpha (simulated)
    constexpr u32 OVERLAY0 = 0xFF6C7086;  // #6c7086
    constexpr u32 OVERLAY1 = 0xFF7F849C;  // #7f849c
    constexpr u32 OVERLAY2 = 0xFF9399B2;  // #9399b2
}

// ===== VESA HIGH RESOLUTION GRAPHICS =====
namespace VESA {
    static u32* framebuffer = nullptr;
    static int width = 1920;   // High-res
    static int height = 1080;  // Full HD
    static int pitch = 0;
    static bool initialized = false;
    
    bool init(u32* fb, int w, int h, int p) {
        framebuffer = fb ? fb : (u32*)0xE0000000;  // Default framebuffer
        width = w > 0 ? w : 1920;
        height = h > 0 ? h : 1080;
        pitch = p > 0 ? p : width * 4;
        initialized = true;
        return true;
    }
    
    inline u32 rgb(u8 r, u8 g, u8 b) {
        return 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    
    void drawPixel(int x, int y, u32 color) {
        if (!initialized || x < 0 || x >= width || y < 0 || y >= height) return;
        framebuffer[y * (pitch / 4) + x] = color;
    }
    
    void fillRect(int x, int y, int w, int h, u32 color) {
        for (int row = y; row < y + h && row < height; row++) {
            for (int col = x; col < x + w && col < width; col++) {
                drawPixel(col, row, color);
            }
        }
    }
    
    // Rounded rectangle (v6_Enhanced feature)
    void fillRoundedRect(int x, int y, int w, int h, int radius, u32 color) {
        // Main rectangle
        fillRect(x + radius, y, w - 2*radius, h, color);
        fillRect(x, y + radius, w, h - 2*radius, color);
        
        // Corners (simplified circles)
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    drawPixel(x + radius + dx, y + radius + dy, color);
                    drawPixel(x + w - radius + dx, y + radius + dy, color);
                    drawPixel(x + radius + dx, y + h - radius + dy, color);
                    drawPixel(x + w - radius + dx, y + h - radius + dy, color);
                }
            }
        }
    }
    
    // Acrylic effect (translucent - v6_Enhanced)
    void drawAcrylicPanel(int x, int y, int w, int h, u32 baseColor) {
        // Base with slight transparency simulation
        fillRoundedRect(x, y, w, h, 8, baseColor);
        // Border
        for (int i = 0; i < 2; i++) {
            for (int px = x; px < x + w; px++) {
                drawPixel(px, y + i, Theme::SURFACE2);
                drawPixel(px, y + h - 1 - i, Theme::SURFACE2);
            }
            for (int py = y; py < y + h; py++) {
                drawPixel(x + i, py, Theme::SURFACE2);
                drawPixel(x + w - 1 - i, py, Theme::SURFACE2);
            }
        }
    }
    
    void clear(u32 color = Theme::BASE) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                framebuffer[y * (pitch / 4) + x] = color;
            }
        }
    }
    
    int getWidth() { return width; }
    int getHeight() { return height; }
}

// ===== MULTITASKING SYSTEM =====
namespace TaskManager {
    struct Task {
        u32 id;
        u32 state;  // 0=running, 1=sleeping, 2=terminated
        char name[32];
        u32 priority;
        u32 cpuTime;
    };
    
    static const int MAX_TASKS = 16;
    static Task tasks[MAX_TASKS];
    static int taskCount = 0;
    static int currentTask = -1;
    static u32 nextId = 1;
    
    void init() {
        for (int i = 0; i < MAX_TASKS; i++) {
            tasks[i].state = 2;  // terminated
        }
        taskCount = 0;
        currentTask = -1;
    }
    
    int create(const char* name, u32 priority = 5) {
        for (int i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].state == 2) {
                tasks[i].id = nextId++;
                tasks[i].state = 0;
                tasks[i].priority = priority;
                tasks[i].cpuTime = 0;
                int j = 0;
                while (name[j] && j < 31) {
                    tasks[i].name[j] = name[j];
                    j++;
                }
                tasks[i].name[j] = '\0';
                taskCount++;
                return tasks[i].id;
            }
        }
        return -1;
    }
    
    void terminate(int id) {
        for (int i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].id == id && tasks[i].state != 2) {
                tasks[i].state = 2;
                taskCount--;
                break;
            }
        }
    }
    
    int getCount() { return taskCount; }
    
    Task* getTasks() { return tasks; }
}

// ===== DESKTOP ENVIRONMENT (v6_Enhanced style) =====
namespace Desktop {
    static bool showTaskbar = true;
    static bool showStartMenu = false;
    static int taskbarHeight = 48;
    
    void drawTaskbar() {
        int h = VESA::getHeight();
        int w = VESA::getWidth();
        
        // Taskbar background (acrylic effect)
        VESA::drawAcrylicPanel(0, h - taskbarHeight, w, taskbarHeight, Theme::SURFACE0);
        
        // Start button (rounded)
        VESA::fillRoundedRect(10, h - taskbarHeight + 8, 80, 32, 6, Theme::LAVENDER);
        
        // Task icons area
        int iconX = 110;
        for (int i = 0; i < TaskManager::getCount() && i < 10; i++) {
            VESA::fillRoundedRect(iconX, h - taskbarHeight + 10, 40, 28, 4, 
                i == 0 ? Theme::BLUE : Theme::SURFACE1);
            iconX += 50;
        }
        
        // System tray area
        VESA::fillRoundedRect(w - 200, h - taskbarHeight + 8, 180, 32, 6, Theme::SURFACE1);
    }
    
    void drawDesktopBackground() {
        // Gradient background (simulated)
        int h = VESA::getHeight();
        int w = VESA::getWidth();
        
        for (int y = 0; y < h - taskbarHeight; y++) {
            u32 color = Theme::BASE;
            // Subtle gradient effect
            if (y < h / 3) color = Theme::MANTLE;
            for (int x = 0; x < w; x++) {
                VESA::drawPixel(x, y, color);
            }
        }
    }
    
    void drawIcons() {
        // Desktop icons (rounded squares)
        int startX = 20;
        int startY = 20;
        int spacing = 100;
        
        const char* icons[] = {
            "Terminal", "Files", "Editor", "Browser",
            "Settings", "Calc", "Snake", "Paint", "Music"
        };
        const u32 colors[] = {
            Theme::GREEN, Theme::BLUE, Theme::TEAL, Theme::SAPPHIRE,
            Theme::LAVENDER, Theme::YELLOW, Theme::PEACH, Theme::PINK, Theme::MAUVE
        };
        
        for (int i = 0; i < 9; i++) {
            int x = startX + (i % 3) * spacing;
            int y = startY + (i / 3) * spacing;
            
            // Icon background (rounded)
            VESA::fillRoundedRect(x, y, 64, 64, 12, colors[i]);
            
            // Selection indicator
            if (i == 0) {
                for (int b = 0; b < 3; b++) {
                    for (int px = x - b; px < x + 64 + b; px++) {
                        VESA::drawPixel(px, y - b, Theme::LAVENDER);
                        VESA::drawPixel(px, y + 64 + b, Theme::LAVENDER);
                    }
                    for (int py = y - b; py < y + 64 + b; py++) {
                        VESA::drawPixel(x - b, py, Theme::LAVENDER);
                        VESA::drawPixel(x + 64 + b, py, Theme::LAVENDER);
                    }
                }
            }
        }
    }
    
    void drawNotificationCenter() {
        // Right-side notification panel (slide-in)
        int w = VESA::getWidth();
        VESA::drawAcrylicPanel(w - 320, 50, 300, 400, Theme::SURFACE0);
        
        // Notifications
        VESA::fillRoundedRect(w - 300, 70, 260, 60, 8, Theme::SURFACE1);
        VESA::fillRoundedRect(w - 300, 140, 260, 60, 8, Theme::SURFACE1);
    }
    
    void drawQuickSettings() {
        // Quick settings panel
        int w = VESA::getWidth();
        int h = VESA::getHeight();
        
        VESA::drawAcrylicPanel(w - 280, h - taskbarHeight - 200, 260, 180, Theme::SURFACE0);
        
        // Toggle buttons (rounded)
        VESA::fillRoundedRect(w - 260, h - taskbarHeight - 180, 100, 40, 20, Theme::GREEN);
        VESA::fillRoundedRect(w - 150, h - taskbarHeight - 180, 100, 40, 20, Theme::BLUE);
    }
    
    void render() {
        drawDesktopBackground();
        drawIcons();
        drawTaskbar();
        // drawNotificationCenter();  // Optional
        // drawQuickSettings();       // Optional
    }
}

// ===== RIGHT-CLICK CONTEXT MENU =====
namespace ContextMenu {
    static bool visible = false;
    static int x = 0, y = 0;
    static int width = 200;
    static int itemHeight = 32;
    static const char* items[] = {
        "Open", "Copy", "Cut", "Paste", "Delete", "Rename", "Properties"
    };
    static const int itemCount = 7;
    
    void show(int mx, int my) {
        x = mx;
        y = my;
        visible = true;
    }
    
    void hide() {
        visible = false;
    }
    
    void draw() {
        if (!visible) return;
        
        // Menu background (acrylic)
        VESA::drawAcrylicPanel(x, y, width, itemCount * itemHeight + 16, Theme::SURFACE0);
        
        // Menu items
        for (int i = 0; i < itemCount; i++) {
            int itemY = y + 8 + i * itemHeight;
            
            // Hover effect (simulated on first item)
            if (i == 0) {
                VESA::fillRoundedRect(x + 4, itemY, width - 8, itemHeight - 4, 4, Theme::SURFACE1);
            }
            
            // Separator line
            if (i == 4) {
                for (int px = x + 10; px < x + width - 10; px++) {
                    VESA::drawPixel(px, itemY, Theme::OVERLAY0);
                }
            }
        }
    }
}

// ===== MOUSE INPUT =====
namespace Mouse {
    static int x = 960, y = 540;
    static bool leftDown = false;
    static bool rightDown = false;
    
    void init() {
        x = VESA::getWidth() / 2;
        y = VESA::getHeight() / 2;
    }
    
    void update(int dx, int dy, bool left, bool right) {
        x += dx;
        y += dy;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= VESA::getWidth()) x = VESA::getWidth() - 1;
        if (y >= VESA::getHeight()) y = VESA::getHeight() - 1;
        
        leftDown = left;
        rightDown = right;
        
        // Right-click detection
        if (right && !rightDown) {
            ContextMenu::show(x, y);
        }
    }
    
    void draw() {
        // Cursor (pointer style)
        for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 12; col++) {
                // Simple cursor shape
                if (col < row / 2 + 2) {
                    VESA::drawPixel(x + col, y + row, Theme::TEXT);
                }
            }
        }
    }
}

// ===== VGA TEXT MODE (Temporary - until VESA is fixed) =====
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
    
    void putc(char c) {
        if (c == '\n') {
            row++;
            col = 0;
        } else if (c == '\r') {
            col = 0;
        } else {
            if (row >= ROWS) {
                // Scroll up
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
        while (*s) {
            putc(*s++);
        }
    }
}

// ===== VESA Initialization from Multiboot =====
namespace VESA {
    static u32* framebuffer = (u32*)0xE0000000;
    static int width = 1024;
    static int height = 768;
    static int pitch = 4096;
    static bool initialized = false;
    
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
    
    void clear(u32 color) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                putPixel(x, y, color);
            }
        }
    }
    
    bool isInitialized() { return initialized; }
    int getWidth() { return width; }
    int getHeight() { return height; }
}

// ===== PS/2 KEYBOARD =====
namespace Keyboard {
    static inline u8 inb(u16 port) {
        u8 result;
        __asm__ __volatile__ ("inb %1, %0" : "=a"(result) : "Nd"(port));
        return result;
    }
    
    bool hasKey() {
        return inb(0x64) & 1;
    }
    
    u8 readScancode() {
        return inb(0x60);
    }
}

// ===== VIRTUAL FILE SYSTEM =====
namespace VFS {
    static const char* folders[] = {
        "C:\\",
        "C:\\WINDOWS",
        "C:\\WINDOWS\\System32",
        "C:\\Documents and Settings",
        "C:\\Program Files",
        "C:\\Program Files\\Accessories",
        "C:\\Program Files\\Games"
    };
    static const int folderCount = 7;
    
    void init() {
        // Virtual folders initialized
    }
    
    const char** getFolders() { return folders; }
    int getFolderCount() { return folderCount; }
}

// ===== GUI COMPONENTS =====
namespace GUI {
    // Windows XP Colors
    constexpr u32 XP_BLUE = 0xFF0054E3;
    constexpr u32 XP_BLUE_LIGHT = 0xFF3D95FF;
    constexpr u32 XP_DESKTOP = 0xFF3A6EA5;
    constexpr u32 XP_GRAY = 0xFFE0E0E0;
    constexpr u32 XP_TASKBAR = 0xFF2368D3;
    constexpr u32 XP_START_BTN = 0xFF3D9400;
    constexpr u32 XP_WHITE = 0xFFFFFFFF;
    constexpr u32 XP_BLACK = 0xFF000000;
    
    void drawDesktop() {
        // Clear with XP desktop color
        VESA::clear(XP_DESKTOP);
        
        // Draw XP logo area (center)
        int cx = VESA::getWidth() / 2;
        int cy = VESA::getHeight() / 2;
        
        // XP style "Windows" logo simulation
        VESA::fillRect(cx - 100, cy - 60, 80, 50, XP_BLUE);
        VESA::fillRect(cx + 20, cy - 60, 80, 50, XP_BLUE_LIGHT);
        VESA::fillRect(cx - 100, cy + 10, 80, 50, XP_BLUE_LIGHT);
        VESA::fillRect(cx + 20, cy + 10, 80, 50, XP_BLUE);
        
        // Welcome text (drawn as white pixels for simplicity)
        VESA::fillRect(cx - 80, cy + 80, 160, 30, XP_WHITE);
    }
    
    void drawTaskbar() {
        int h = VESA::getHeight();
        int w = VESA::getWidth();
        
        // Taskbar
        VESA::fillRect(0, h - 30, w, 30, XP_TASKBAR);
        
        // Start button
        VESA::fillRect(5, h - 25, 70, 20, XP_START_BTN);
        VESA::fillRect(7, h - 23, 66, 16, XP_BLUE_LIGHT);
    }
}

// ===== KERNEL MAIN =====
extern "C" void kernel_main(u32 magic, u32* mboot) {
    // Initialize VESA from multiboot
    VESA::initFromMultiboot(mboot);
    
    // Check if VESA mode is active
    if (VESA::isInitialized()) {
        // Use VESA graphics mode
        VFS::init();
        GUI::drawDesktop();
        GUI::drawTaskbar();
        
        // Main loop with keyboard
        while (true) {
            if (Keyboard::hasKey()) {
                u8 scancode = Keyboard::readScancode();
                // Handle keyboard input here
                // For now, just acknowledge
            }
            __asm__ __volatile__ ("hlt");
        }
    } else {
        // Fallback to VGA text mode
        VGA::clear();
        VGA::puts("C-OS Ultimate Kernel v3.0\n");
        VGA::puts("VESA mode not available, using VGA text\n");
        
        while (true) {
            __asm__ __volatile__ ("hlt");
        }
    }
}
