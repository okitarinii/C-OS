// C-OS Ultimate - Memory Management Header
#ifndef MEMORY_H
#define MEMORY_H

#include "include/types.h"

namespace Memory {
    // Initialize memory manager
    void init(u32 totalMemory);
    
    // Page-level functions
    u32 getFreePages();
    u32 getUsedPages();
    
    // Heap functions with guards
    namespace Heap {
        void* malloc(u32 size);
        void free(void* ptr);
        bool validate(void* ptr);
        u32 getUsed();
        u32 getFree();
    }
    
    // Stack protection
    namespace Stack {
        void initCanary();
        u32 getCanary();
        bool checkCanary();
        u32 getStackPointer();
        bool isValidStack(u32 addr, u32 stackTop, u32 stackSize);
    }
    
    // Memory guard check
    void checkMemoryGuards();
}

// Convenience aliases
inline void* kmalloc(u32 size) { return Memory::Heap::malloc(size); }
inline void kfree(void* ptr) { Memory::Heap::free(ptr); }

#endif // MEMORY_H
