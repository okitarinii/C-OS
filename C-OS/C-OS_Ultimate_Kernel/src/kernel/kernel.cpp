// C-OS Ultimate Kernel - Modular Version
// Clean split: kernel.cpp (main) + vga.cpp + memory.cpp

#include "include/types.h"

// Language system
namespace Lang {
    static bool isJapanese = false;
    
    void setJapanese(bool val) { isJapanese = val; }
    bool getJapanese() { return isJapanese; }
}

// VGA forward declaration for Secrets
namespace VGA {
    void clear();
    void setColor(u8 fg, u8 bg);
    void puts(const char* s);
}

// Hidden command constants
namespace Secrets {
    const char* CMD_ARTIST = "Teaching aetist";
    const char* CMD_MAERZ = "Encho Guihub maerz";
    
    void showArtist() {
        VGA::clear();
        VGA::setColor(15, 0);  // White on black
        VGA::puts("  K   K   TTTTT\n");
        VGA::puts("  K  K      T  \n");
        VGA::puts("  K K       T  \n");
        VGA::puts("  KK        T  \n");
        VGA::puts("  K K       T  \n");
        VGA::puts("  K  K      T  \n");
        VGA::puts("  K   K     T  \n\n");
    }
    
    void showMaerz() {
        VGA::clear();
        VGA::setColor(15, 1);  // White on blue
        if (Lang::getJapanese()) {
            VGA::puts("=====================================\n");
            VGA::puts("  やあ、みんな！\n");
            VGA::puts("  このコマンドに気づいた君は優秀だね！\n");
            VGA::puts("  私の作ったOSはまだまだ未熟だが\n");
            VGA::puts("  楽しんでくれ！\n");
            VGA::puts("=====================================\n");
        } else {
            VGA::puts("=====================================\n");
            VGA::puts("  Hey everyone!\n");
            VGA::puts("  You noticed this command - clever!\n");
            VGA::puts("  My OS is still immature but\n");
            VGA::puts("  enjoy using it!\n");
            VGA::puts("=====================================\n");
        }
    }
}

// Forward declarations
namespace Mouse {
    void onInterrupt();
}

// Mouse handler called by boot.asm (irq12_handler)
extern "C" void mouse_handler() {
    Mouse::onInterrupt();
}

// IRQ12 handler defined in boot.asm
extern "C" void irq12_handler();

// External module declarations
namespace VGA {
    void clear();
    void setColor(u8 fg, u8 bg);
    void putc(char c);
    void puts(const char* s);
    void puthex(u32 n);
    void putdec(u32 n);
    void setCursor(int x, int y);
    void writeChar(int x, int y, char c, u8 attr);
    void fillRect(int x, int y, int w, int h, char c, u8 attr);
    u16 readChar(int x, int y);
}

namespace Memory {
    void init(u32 totalMemory);
    void* allocPage();
    void freePage(void* addr);
    u32 getFreePages();
    u32 getUsedPages();
    
    // Heap with guards
    namespace Heap {
        void* malloc(u32 size);
        void free(void* ptr);
        bool validate(void* ptr);
    }
    
    // Stack protection
    namespace Stack {
        void initCanary();
        bool checkCanary();
    }
}

namespace Mouse {
    void init();
    void onInterrupt();
    int getX();
    int getY();
    u8 getButtons();
    bool hasPacket();
    void clearPacket();
}

namespace GUI {
    void init();
    int createWindow(int x, int y, int width, int height, const char* title);
    void closeWindow(int id);
    void drawText(int windowId, int x, int y, const char* text);
    void drawButton(int windowId, int x, int y, int width, const char* text, bool pressed);
    int getActiveWindow();
    void showMouse(bool show);
    void getMousePos(int& x, int& y);
    void update();
}

namespace Keyboard {
    void init();
    bool hasKey();
    u8 readScancode();
    char scancodeToAscii(u8 scancode);
    bool isShiftPressed();
    int getArrowKey(u8 scancode);
    bool processModifiers(u8 scancode);
    void resetExtended();
}

namespace Mouse {
    void init();
    void onInterrupt();
    int getX();
    int getY();
    u8 getButtons();
}

namespace VFS {
    void init();
}

namespace Scheduler {
    void init();
    void start();
    u32 createTask(void (*entryPoint)(), const char* name, u32 priority);
    void yield();
    bool isMultitasking();
}

namespace GUI {
    void init();
    void showMouse(bool show);
}

// ===== GDT (Global Descriptor Table) =====
namespace GDT {
    struct Entry {
        u16 limit_low;
        u16 base_low;
        u8 base_middle;
        u8 access;
        u8 granularity;
        u8 base_high;
    } __attribute__((packed));
    
    struct Ptr {
        u16 limit;
        u32 base;
    } __attribute__((packed));
    
    static Entry entries[3];
    static Ptr gdtPtr;
    
    void setEntry(int num, u32 base, u32 limit, u8 access, u8 gran) {
        entries[num].base_low = base & 0xFFFF;
        entries[num].base_middle = (base >> 16) & 0xFF;
        entries[num].base_high = (base >> 24) & 0xFF;
        entries[num].limit_low = limit & 0xFFFF;
        entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
        entries[num].access = access;
    }
    
    void init() {
        setEntry(0, 0, 0, 0, 0);           // Null
        setEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code
        setEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data
        
        gdtPtr.limit = sizeof(entries) - 1;
        gdtPtr.base = (u32)&entries;
        
        __asm__ __volatile__ ("lgdt %0" : : "m"(gdtPtr));
        VGA::puts("[OK] GDT initialized\n");
    }
}

// Utility: Compare strings
bool compareStrings(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++;
        b++;
    }
    return *a == *b;
}

// ===== IDT (Interrupt Descriptor Table) =====
namespace IDT {
    struct Entry {
        u16 offset_low;
        u16 selector;
        u8 zero;
        u8 type_attr;
        u16 offset_high;
    } __attribute__((packed));
    
    struct Ptr {
        u16 limit;
        u32 base;
    } __attribute__((packed));
    
    static Entry entries[256];
    static Ptr idtPtr;
    
    void setEntry(u8 num, u32 handler, u16 sel, u8 flags) {
        entries[num].offset_low = handler & 0xFFFF;
        entries[num].offset_high = (handler >> 16) & 0xFFFF;
        entries[num].selector = sel;
        entries[num].zero = 0;
        entries[num].type_attr = flags;
    }
    
    void init() {
        idtPtr.limit = sizeof(entries) - 1;
        idtPtr.base = (u32)&entries;
        
        for (int i = 0; i < 256; i++) {
            setEntry(i, 0, 0, 0);
        }
        
        // Register IRQ12 handler for mouse (0x08 = kernel code seg, 0x8E = interrupt gate)
        setEntry(0x2C, (u32)irq12_handler, 0x08, 0x8E);
        VGA::puts("[OK] IRQ12 handler registered for mouse\n");
        
        __asm__ __volatile__ ("lidt %0" : : "m"(idtPtr));
        VGA::puts("[OK] IDT initialized\n");
    }
}

// ===== PIC (8259 Programmable Interrupt Controller) =====
namespace PIC {
    static const u16 PIC1 = 0x20;
    static const u16 PIC2 = 0xA0;
    static const u16 PIC1_CMD = PIC1;
    static const u16 PIC1_DATA = PIC1 + 1;
    static const u16 PIC2_CMD = PIC2;
    static const u16 PIC2_DATA = PIC2 + 1;
    
    void init() {
        // ICW1: Start initialization
        outb(PIC1_CMD, 0x11);
        outb(PIC2_CMD, 0x11);
        
        // ICW2: Vector offsets (remap IRQs)
        outb(PIC1_DATA, 0x20);  // Master: IRQ0-7 -> 0x20-0x27
        outb(PIC2_DATA, 0x28);  // Slave: IRQ8-15 -> 0x28-0x2F
        
        // ICW3: Cascade configuration
        outb(PIC1_DATA, 0x04);  // Master has slave at IRQ2
        outb(PIC2_DATA, 0x02);  // Slave is at IRQ2
        
        // ICW4: 8086 mode
        outb(PIC1_DATA, 0x01);
        outb(PIC2_DATA, 0x01);
        
        // Mask all IRQs initially
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);
        
        VGA::puts("[OK] PIC initialized\n");
    }
    
    void enableIRQ(u8 irq) {
        u16 port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
        u8 mask = inb(port);
        mask &= ~(1 << (irq & 7));
        outb(port, mask);
    }
}

// ===== MAIN =====
extern "C" void kernel_main(u32 magic, u32* mboot) {
    // Phase 1: Basic output
    VGA::clear();
    VGA::puts("C-OS Ultimate Kernel\n");
    VGA::puts("====================\n\n");
    VGA::puts("[OK] Boot successful!\n");
    VGA::puts("Magic: 0x"); VGA::puthex(magic); VGA::puts("\n\n");
    
    // Phase 2: Memory Manager
    Memory::init(mboot[1]);  // Use memory info from multiboot
    
    // Test memory guards silently
    void* testPtr = Memory::Heap::malloc(64);
    if (testPtr) Memory::Heap::free(testPtr);
    
    // Initialize stack canary
    Memory::Stack::initCanary();
    
    // Phase 3: GDT & IDT & PIC
    GDT::init();
    IDT::init();
    PIC::init();
    
    // Enable IRQ12 for mouse
    PIC::enableIRQ(12);
    
    // Initialize mouse driver
    Mouse::init();
    
    // Enable interrupts
    sti();
    
    // Phase 4: Scheduler (Multitasking)
    Scheduler::init();
    // Note: Tasks will be created later, scheduler starts after all init
    
    // Phase 6: VFS
    VFS::init();
    
    // Phase 7: GUI Mode (Windows style)
    GUI::init();
    GUI::showMouse(true);
    
    // Test: Draw static mouse cursor at center to verify visibility
    // If this shows but mouse doesn't move, problem is in mouse driver
    // If this doesn't show, problem is in VGA output
    VGA::writeChar(40, 12, 219, 0x0F);  // White block at center
    
    // Create sample windows
    int win1 = GUI::createWindow(2, 2, 30, 10, "Terminal");
    GUI::drawText(win1, 1, 1, "C-OS Ultimate GUI v1.0");
    GUI::drawText(win1, 1, 3, "Commands: help, echo, clear");
    GUI::drawText(win1, 1, 5, "Mouse: Left click to focus");
    
    int win2 = GUI::createWindow(35, 5, 25, 8, "System Info");
    GUI::drawText(win2, 1, 1, "Memory:");
    GUI::drawText(win2, 1, 2, "Kernel: OK");
    GUI::drawText(win2, 1, 3, "Mouse: IRQ12");
    GUI::drawText(win2, 1, 5, "Press 's' for shell");
    
    // GUI Mode - wait for 's' key to switch to shell
    
    // GUI event loop
    while (true) {
        GUI::update();  // Update mouse, etc.
        
        // Debug: Show mouse coordinates at bottom right
        int mx = Mouse::getX();
        int my = Mouse::getY();
        VGA::setColor(15, 1);  // White on blue
        VGA::setCursor(60, 24);
        VGA::puts("M:");
        VGA::putdec(mx);
        VGA::puts(",");
        VGA::putdec(my);
        VGA::puts("  ");
        
        // Check for 's' key to switch to shell mode
        if (Keyboard::hasKey()) {
            u8 scancode = Keyboard::readScancode();
            if (scancode == 0x1F) {  // 's' key pressed
                break;  // Exit GUI mode, go to shell
            }
        }
        
        // Small delay to prevent busy-waiting
        for (volatile int i = 0; i < 1000; i++);
    }
    
    // Clear screen for shell mode
    VGA::clear();
    GUI::showMouse(false);
    
    VGA::puts("\n=== Shell Mode ===\n");
    VGA::puts("Commands: help, echo, clear, about, shutdown\n\n");
    
    // Shell variables
    static char lineBuffer[256];
    int linePos = 0;
    int cursorPos = 0;  // For arrow key support
    
    while (true) {
        VGA::puts("> ");  // Prompt
        
        // Read a line
        while (true) {
            // Wait for key
            while (!Keyboard::hasKey()) {
                for (volatile int i = 0; i < 1000; i++);
            }
            
            u8 scancode = Keyboard::readScancode();
            
            // Handle extended scancode prefix (0xE0 for arrows)
            if (scancode == 0xE0) {
                // Wait for next scancode
                while (!Keyboard::hasKey()) {
                    for (volatile int i = 0; i < 100; i++);
                }
                u8 extScancode = Keyboard::readScancode();
                
                // Arrow keys
                if (extScancode == 0x48) { /* Up - ignore for now */ }
                else if (extScancode == 0x50) { /* Down - ignore for now */ }
                else if (extScancode == 0x4B) { // Left
                    if (cursorPos > 0) {
                        cursorPos--;
                        VGA::puts("\x1B[D");  // ANSI left (if supported) or use backspace
                        VGA::putc('\b');
                    }
                }
                else if (extScancode == 0x4D) { // Right
                    if (cursorPos < linePos) {
                        VGA::putc(lineBuffer[cursorPos]);
                        cursorPos++;
                    }
                }
                continue;
            }
            
            // Handle modifiers (shift, etc.)
            if (Keyboard::processModifiers(scancode)) {
                continue;
            }
            
            // Handle key release (ignore for now, but reset extended flag)
            if (scancode >= 0x80) {
                if (scancode == 0xAA || scancode == 0xB6) {
                    Keyboard::processModifiers(scancode);
                }
                continue;
            }
            
            // Special keys
            if (scancode == 0x1C) {  // Enter
                lineBuffer[linePos] = '\0';
                VGA::putc('\n');
                break;
            }
            else if (scancode == 0x0E) {  // Backspace
                if (cursorPos > 0) {
                    // Remove char at cursorPos-1
                    for (int i = cursorPos - 1; i < linePos - 1; i++) {
                        lineBuffer[i] = lineBuffer[i + 1];
                    }
                    linePos--;
                    cursorPos--;
                    // Redraw
                    VGA::putc('\b');
                    for (int i = cursorPos; i < linePos; i++) {
                        VGA::putc(lineBuffer[i]);
                    }
                    VGA::putc(' ');
                    for (int i = cursorPos; i <= linePos; i++) {
                        VGA::putc('\b');
                    }
                }
            }
            else {
                // Regular character
                char c = Keyboard::scancodeToAscii(scancode);
                if (c && linePos < 255) {
                    // Insert at cursor position
                    for (int i = linePos; i > cursorPos; i--) {
                        lineBuffer[i] = lineBuffer[i - 1];
                    }
                    lineBuffer[cursorPos] = c;
                    linePos++;
                    VGA::putc(c);
                    cursorPos++;
                    // Redraw rest of line
                    for (int i = cursorPos; i < linePos; i++) {
                        VGA::putc(lineBuffer[i]);
                    }
                    for (int i = cursorPos; i < linePos; i++) {
                        VGA::putc('\b');
                    }
                }
            }
        }
        
        // Parse and execute command
        if (linePos == 0) continue;  // Empty line
        
        // Simple command parser
        if (lineBuffer[0] == '\0') {
            // Empty
        }
        else if (lineBuffer[0] == 'h' && lineBuffer[1] == 'e' && 
                 lineBuffer[2] == 'l' && lineBuffer[3] == 'p' &&
                 (lineBuffer[4] == '\0' || lineBuffer[4] == ' ')) {
            VGA::puts("Available commands:\n");
            VGA::puts("  help     - Show this help\n");
            VGA::puts("  echo     - Echo text back\n");
            VGA::puts("  clear    - Clear screen\n");
            VGA::puts("  about    - Show system info\n");
            VGA::puts("  shutdown - Shutdown system\n");
        }
        else if (lineBuffer[0] == 'e' && lineBuffer[1] == 'c' &&
                 lineBuffer[2] == 'h' && lineBuffer[3] == 'o' &&
                 (lineBuffer[4] == '\0' || lineBuffer[4] == ' ')) {
            // Echo command
            if (lineBuffer[4] == ' ') {
                VGA::puts(&lineBuffer[5]);
            }
            VGA::putc('\n');
        }
        else if (lineBuffer[0] == 'c' && lineBuffer[1] == 'l' &&
                 lineBuffer[2] == 'e' && lineBuffer[3] == 'a' &&
                 lineBuffer[4] == 'r' && lineBuffer[5] == '\0') {
            // Clear screen
            VGA::clear();
        }
        else if (lineBuffer[0] == 'a' && lineBuffer[1] == 'b' &&
                 lineBuffer[2] == 'o' && lineBuffer[3] == 'u' &&
                 lineBuffer[4] == 't' && lineBuffer[5] == '\0') {
            VGA::puts("C-OS Ultimate v1.0\n");
            VGA::puts("A simple OS written in C++\n");
            VGA::puts("Features: Keyboard, VGA, GDT, IDT, PIC\n");
        }
        else if (lineBuffer[0] == 's' && lineBuffer[1] == 'h' &&
                 lineBuffer[2] == 'u' && lineBuffer[3] == 't' &&
                 lineBuffer[4] == 'd' && lineBuffer[5] == 'o' &&
                 lineBuffer[6] == 'w' && lineBuffer[7] == 'n' &&
                 lineBuffer[8] == '\0') {
            VGA::puts("Shutting down...\n");
            break;
        }
        // Hidden command: Language switch
        else if (compareStrings(lineBuffer, "Change language Japanese")) {
            Lang::setJapanese(true);
            VGA::puts("Language set to Japanese\n");
        }
        // Hidden command: Artist signature
        else if (compareStrings(lineBuffer, Secrets::CMD_ARTIST)) {
            Secrets::showArtist();
        }
        // Hidden command: Maerz message
        else if (compareStrings(lineBuffer, Secrets::CMD_MAERZ)) {
            Secrets::showMaerz();
        }
        else {
            VGA::puts("Unknown command: ");
            VGA::puts(lineBuffer);
            VGA::puts("\nType 'help' for available commands\n");
        }
        
        linePos = 0;
        cursorPos = 0;
    }
    
    while (true) hlt();
}
