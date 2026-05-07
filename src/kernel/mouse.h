// C-OS Ultimate - PS/2 Mouse Driver Header
#ifndef MOUSE_H
#define MOUSE_H

#include "include/types.h"

namespace Mouse {
    void init();
    void onInterrupt();  // IRQ12 handler
    void drawCursor();
    int getX();
    int getY();
    u8 getButtons();
    bool hasPacket();
    void clearPacket();
}

#endif // MOUSE_H
