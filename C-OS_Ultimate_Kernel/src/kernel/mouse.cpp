// C-OS Ultimate - PS/2 Mouse Driver
#include "include/types.h"

// External VGA for cursor drawing
namespace VGA {
    void puts(const char* s);
    void putc(char c);
    void puthex(u32 n);
    void putPixel(int x, int y, u8 color);
    void clearScreen();
}

namespace Mouse {
    static const u16 PORT_DATA = 0x60;
    static const u16 PORT_STATUS = 0x64;
    static const u16 PORT_CMD = 0x64;
    
    // Mouse state
    static int mouseX = 40;  // Center of 80x25 screen (roughly)
    static int mouseY = 12;
    static u8 buttons = 0;
    static bool packetReady = false;
    static u8 packet[3];
    static u8 packetIndex = 0;
    
    // Mouse cursor character (inverse video block)
    static const char CURSOR_CHAR = 219; // Full block
    
    void waitForInput() {
        while (inb(PORT_STATUS) & 2);
    }
    
    void waitForOutput() {
        while (!(inb(PORT_STATUS) & 1));
    }
    
    void sendCommand(u8 cmd) {
        waitForInput();
        outb(PORT_CMD, 0xD4);  // Tell controller we're sending to mouse
        waitForInput();
        outb(PORT_DATA, cmd);
    }
    
    u8 readData() {
        // Timeout after 100000 iterations to prevent infinite hang
        for (int timeout = 0; timeout < 100000; timeout++) {
            if ((inb(PORT_STATUS) & 1) == 1) {  // Data available
                return inb(PORT_DATA);
            }
        }
        return 0xFF;  // Timeout error
    }
    
    // Non-blocking read with timeout
    u8 readDataWithTimeout(bool& success) {
        for (int timeout = 0; timeout < 100000; timeout++) {
            if ((inb(PORT_STATUS) & 1) == 1) {
                success = true;
                return inb(PORT_DATA);
            }
        }
        success = false;
        return 0xFF;
    }
    
    void init() {
        VGA::puts("[MOUSE] Init start...\n");
        
        // Clear any pending data
        for (int i = 0; i < 8; i++) {
            if (inb(PORT_STATUS) & 1) {
                inb(PORT_DATA);
            }
        }
        
        // Enable mouse port (no wait)
        outb(PORT_CMD, 0xA8);  // Enable second PS/2 port
        
        // Enable mouse interrupts
        bool success;
        outb(PORT_CMD, 0x20);  // Read config
        u8 config = readDataWithTimeout(success);
        if (!success) {
            VGA::puts("[MOUSE] Timeout reading config\n");
            return;
        }
        
        config |= 0x02;  // Enable IRQ12
        outb(PORT_CMD, 0x60);  // Write config
        outb(PORT_DATA, config);
        
        VGA::puts("[MOUSE] Init done\n");
        outb(PORT_CMD, 0x60);  // Write configuration byte
        waitForInput();
        outb(PORT_DATA, config);
        VGA::puts("[MOUSE] IRQ12 enabled\n");
        
        // Note: Full mouse reset sequence disabled to prevent hang
        // Basic mouse functionality should still work with PS/2 emulation
        
        // VGA::puts("[OK] Mouse initialized\n");
    }
    
    // Process mouse interrupt (IRQ12) - No debug output to avoid screen corruption
    void onInterrupt() {
        u8 data = inb(PORT_DATA);
        
        // Check if this is a valid packet byte
        if (packetIndex == 0) {
            // First byte should have bit 3 set
            if ((data & 0x08) == 0) {
                // Not a valid first byte, ignore
                return;
            }
        }
        
        packet[packetIndex] = data;
        packetIndex++;
        
        if (packetIndex >= 3) {
            // Packet complete
            buttons = packet[0] & 0x07;
            
            // X movement (signed)
            int dx = (signed char)packet[1];
            if (packet[0] & 0x10) dx -= 256;  // Sign extension
            
            // Y movement (signed, inverted)
            int dy = (signed char)packet[2];
            if (packet[0] & 0x20) dy -= 256;  // Sign extension
            dy = -dy;  // Invert Y (screen coordinates go down)
            
            // Update position (with bounds checking for 80x25 text mode)
            mouseX += dx / 8;  // Convert to character cells
            mouseY += dy / 16;
            
            if (mouseX < 0) mouseX = 0;
            if (mouseX > 79) mouseX = 79;
            if (mouseY < 0) mouseY = 0;
            if (mouseY > 24) mouseY = 24;
            
            packetReady = true;
            packetIndex = 0;
        }
    }
    
    void drawCursor() {
        // Draw mouse cursor at current position
        // In text mode, we can use a special character
        // For now, just print position on screen corner
        // TODO: Implement actual cursor drawing in VGA text mode
    }
    
    int getX() { return mouseX; }
    int getY() { return mouseY; }
    u8 getButtons() { return buttons; }
    bool hasPacket() { return packetReady; }
    void clearPacket() { packetReady = false; }
}
