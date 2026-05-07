// C-OS Ultimate - PS/2 Keyboard Driver
#include "include/types.h"

// External VGA for debug
namespace VGA {
    void puts(const char* s);
    void putc(char c);
    void puthex(u32 n);
}

namespace Keyboard {
    static const u16 PORT_DATA = 0x60;
    static const u16 PORT_STATUS = 0x64;
    
    // Shift key state
    static bool leftShift = false;
    static bool rightShift = false;
    static bool extended = false;  // For 0xE0 extended scancodes (arrows)
    
    bool hasKey() {
        return inb(PORT_STATUS) & 1;
    }
    
    u8 readScancode() {
        return inb(PORT_DATA);
    }
    
    bool isShiftPressed() {
        return leftShift || rightShift;
    }
    
    // Check if scancode is an arrow key (after processing extended byte)
    bool isArrowKey(u8 scancode, u8& arrowCode) {
        if (scancode == 0x48) { arrowCode = 0; return true; } // Up
        if (scancode == 0x50) { arrowCode = 1; return true; } // Down
        if (scancode == 0x4B) { arrowCode = 2; return true; } // Left
        if (scancode == 0x4D) { arrowCode = 3; return true; } // Right
        return false;
    }
    
    // Get arrow key code (0=up, 1=down, 2=left, 3=right)
    // Returns -1 if not an arrow key
    int getArrowKey(u8 scancode) {
        if (extended) {
            if (scancode == 0x48) return 0; // Up
            if (scancode == 0x50) return 1; // Down
            if (scancode == 0x4B) return 2; // Left
            if (scancode == 0x4D) return 3; // Right
        }
        return -1;
    }
    
    // Process scancode and update shift state
    // Returns true if this scancode was a shift key (consumed)
    bool processModifiers(u8 scancode) {
        // Extended scancode prefix
        if (scancode == 0xE0) {
            extended = true;
            return true; // Consumed, not a printable key
        }
        
        // Check for shift press/release
        if (scancode == 0x2A) { leftShift = true; return true; }
        if (scancode == 0x36) { rightShift = true; return true; }
        if (scancode == 0xAA) { leftShift = false; return true; }
        if (scancode == 0xB6) { rightShift = false; return true; }
        
        return false; // Not a modifier
    }
    
    // Convert scancode to ASCII with shift support
    char scancodeToAscii(u8 scancode) {
        // Handle shift state
        bool shift = isShiftPressed();
        // PS/2 Scancode Set 1 to ASCII mapping
        static const char map[] = {
            0,        // 0x00: Error
            0,        // 0x01: ESC
            '1',      // 0x02: '1'
            '2',      // 0x03: '2'
            '3',      // 0x04: '3'
            '4',      // 0x05: '4'
            '5',      // 0x06: '5'
            '6',      // 0x07: '6'
            '7',      // 0x08: '7'
            '8',      // 0x09: '8'
            '9',      // 0x0A: '9'
            '0',      // 0x0B: '0'
            '-',      // 0x0C: '-'
            '=',      // 0x0D: '='
            '\b',     // 0x0E: Backspace
            '\t',     // 0x0F: Tab
            'q',      // 0x10: 'q'
            'w',      // 0x11: 'w'
            'e',      // 0x12: 'e' ← E key (Set 1)
            'r',      // 0x13: 'r'
            't',      // 0x14: 't'
            'y',      // 0x15: 'y'
            'u',      // 0x16: 'u'
            'i',      // 0x17: 'i'
            'o',      // 0x18: 'o'
            'p',      // 0x19: 'p'
            '[',      // 0x1A: '['
            ']',      // 0x1B: ']' ← (not 'J'!)
            '\n',     // 0x1C: Enter
            0,        // 0x1D: Left Ctrl
            'a',      // 0x1E: 'a'
            's',      // 0x1F: 's'
            'd',      // 0x20: 'd' ← D key! (Set 1: 0x20='d')
            'f',      // 0x21: 'f'
            'g',      // 0x22: 'g'
            'h',      // 0x23: 'h' ← H key! (Set 1: 0x23='h')
            'j',      // 0x24: 'j'
            'k',      // 0x25: 'k'
            'l',      // 0x26: 'l'
            ';',      // 0x27: ';'
            '\'',     // 0x28: '''
            '`',      // 0x29: '`'
            0,        // 0x2A: Left Shift
            '\\',     // 0x2B: '\'
            'z',      // 0x2C: 'z'
            'x',      // 0x2D: 'x'
            'c',      // 0x2E: 'c'
            'v',      // 0x2F: 'v'
            'b',      // 0x30: 'b'
            'n',      // 0x31: 'n'
            'm',      // 0x32: 'm'
            ',',      // 0x33: ','
            '.',      // 0x34: '.'
            '/',      // 0x35: '/'
            0,        // 0x36: Right Shift
            '*',      // 0x37: Keypad *
            0,        // 0x38: Left Alt
            ' ',      // 0x39: Space
            0,        // 0x3A: Caps Lock
        };
        if (scancode >= sizeof(map)) return 0;
        
        char c = map[scancode];
        if (c == 0) return 0;
        
        // Apply shift for letters (a-z -> A-Z)
        if (c >= 'a' && c <= 'z' && shift) {
            return c - 32; // Convert to uppercase
        }
        
        // Apply shift for number row symbols
        if (shift) {
            switch (c) {
                case '1': return '!';
                case '2': return '@';
                case '3': return '#';
                case '4': return '$';
                case '5': return '%';
                case '6': return '^';
                case '7': return '&';
                case '8': return '*';
                case '9': return '(';
                case '0': return ')';
                case '-': return '_';
                case '=': return '+';
                case '[': return '{';
                case ']': return '}';
                case '\\': return '|';
                case ';': return ':';
                case '\'': return '"';
                case '`': return '~';
                case ',': return '<';
                case '.': return '>';
                case '/': return '?';
            }
        }
        
        return c;
    }
    
    // Reset extended flag (call after processing each scancode)
    void resetExtended() {
        extended = false;
    }
    
    static void waitForInput() {
        while (inb(PORT_STATUS) & 2); // Wait while input buffer full
    }
    
    static void waitForOutput() {
        while (!(inb(PORT_STATUS) & 1)); // Wait until output buffer has data
    }
    
    void init() {
        VGA::puts("[KBD] Disabling PS/2...\n");
        
        // Disable both PS/2 ports
        waitForInput();
        outb(PORT_STATUS, 0xAD); // Disable keyboard
        waitForInput();
        outb(PORT_STATUS, 0xA7); // Disable mouse (if present)
        
        VGA::puts("[KBD] Clearing buffer...\n");
        // Clear output buffer
        int clearCount = 0;
        while (inb(PORT_STATUS) & 1 && clearCount < 16) {
            inb(PORT_DATA);
            clearCount++;
        }
        
        VGA::puts("[KBD] Reading config...\n");
        // Read configuration byte
        waitForInput();
        outb(PORT_STATUS, 0x20); // Read config command
        waitForOutput();
        u8 config = inb(PORT_DATA);
        VGA::puts("[KBD] Config: 0x"); VGA::puthex(config); VGA::puts("\n");
        
        // Modify config: ENABLE IRQ1 for keyboard, disable translation
        config &= ~(0x40); // Clear bit 6 (disable translation)
        config |= 0x01;    // Set bit 0 (enable keyboard IRQ)
        
        VGA::puts("[KBD] Writing config...\n");
        // Write configuration byte back
        waitForInput();
        outb(PORT_STATUS, 0x60); // Write config command
        waitForInput();
        outb(PORT_DATA, config);
        
        VGA::puts("[KBD] Self-test...\n");
        // Self-test controller
        waitForInput();
        outb(PORT_STATUS, 0xAA); // Self-test
        waitForOutput();
        u8 selfTest = inb(PORT_DATA);
        VGA::puts("[KBD] Self-test result: 0x"); VGA::puthex(selfTest); VGA::puts("\n");
        
        VGA::puts("[KBD] Enabling port...\n");
        // Enable keyboard port
        waitForInput();
        outb(PORT_STATUS, 0xAE); // Enable keyboard
        
        VGA::puts("[KBD] Resetting keyboard...\n");
        // Reset keyboard
        waitForInput();
        outb(PORT_DATA, 0xFF); // Reset command
        waitForOutput();
        u8 resetAck = inb(PORT_DATA); // Should be 0xFA
        VGA::puts("[KBD] Reset ack: 0x"); VGA::puthex(resetAck); VGA::puts("\n");
        waitForOutput();
        u8 resetDone = inb(PORT_DATA); // Should be 0xAA (success) or 0xFC (fail)
        VGA::puts("[KBD] Reset done: 0x"); VGA::puthex(resetDone); VGA::puts("\n");
        
        VGA::puts("[KBD] Setting scancode set 1...\n");
        // Set scancode set 1 (most common)
        // Some keyboards need the command sent twice
        for (int retry = 0; retry < 3; retry++) {
            waitForInput();
            outb(PORT_DATA, 0xF0);   // Set scancode set command
            waitForOutput();
            u8 ack1 = inb(PORT_DATA); // Read ack (should be 0xFA)
            VGA::puts("[KBD] Set cmd ack: 0x"); VGA::puthex(ack1); VGA::puts("\n");
            if (ack1 == 0xFA) break;
        }
        
        waitForInput();
        outb(PORT_DATA, 0x01);   // Select set 1
        waitForOutput();
        u8 ack2 = inb(PORT_DATA); // Read ack (should be 0xFA)
        VGA::puts("[KBD] Set 1 ack: 0x"); VGA::puthex(ack2); VGA::puts("\n");
        VGA::puts("[KBD] Scancode set 1 selected\n");
        
        VGA::puts("[KBD] Enabling scan...\n");
        // Enable scanning
        waitForInput();
        outb(PORT_DATA, 0xF4);   // Enable scanning
        waitForOutput();
        u8 scanAck = inb(PORT_DATA); // Should be 0xFA
        VGA::puts("[KBD] Scan ack: 0x"); VGA::puthex(scanAck); VGA::puts("\n");
        
        VGA::puts("[KBD] Init complete!\n");
    }
}
