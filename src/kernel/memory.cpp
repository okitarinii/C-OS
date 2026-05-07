// C-OS Ultimate - Memory Management
// Phase 2: Memory Manager Implementation + Memory Guards

#include "include/types.h"

namespace VGA {
    void puts(const char* s);
    void puthex(u32 n);
}

namespace Memory {

// ===== MEMORY GUARD CONSTANTS =====
static const u32 GUARD_POISON_ALLOCATED = 0xDEADBEEF;  // Allocated memory pattern
static const u32 GUARD_POISON_FREED = 0xFEEEFEEE;      // Freed memory pattern
static const u32 GUARD_CANARY_VALUE = 0xC0FFEE00;      // Stack canary value
static const u32 GUARD_HEADER_MAGIC = 0x4D454D00;     // 'MEM' + version
static const u32 GUARD_FENCE_MAGIC = 0x46454E43;      // 'FENC' fence magic

// ===== MEMORY GUARD STRUCTURES =====
struct AllocHeader {
    u32 magic;           // GUARD_HEADER_MAGIC
    u32 size;            // Size of allocation (excluding header)
    u32 fence;           // GUARD_FENCE_MAGIC
};

struct AllocFooter {
    u32 fence;           // GUARD_FENCE_MAGIC
    u32 checksum;        // XOR checksum of header
};

// ===== PHYSICAL MEMORY MANAGER =====
// Manages 4KB pages using bitmap
namespace PMM {
    static const u32 PAGE_SIZE = 4096;
    static const u32 MAX_PAGES = 1024 * 1024; // 4GB max
    
    static u8* bitmap = nullptr;
    static u32 totalPages = 0;
    static u32 usedPages = 0;
    static u32 freePages = 0;
    
    // Allocation statistics
    static u32 totalAllocations = 0;
    static u32 totalFrees = 0;
    static u32 guardViolations = 0;
    
    void setPage(u32 page, bool used) {
        u32 byte = page / 8;
        u32 bit = page % 8;
        
        if (used) {
            bitmap[byte] |= (1 << bit);
        } else {
            bitmap[byte] &= ~(1 << bit);
        }
    }
    
    void init(u32 totalMemory) {
        totalPages = totalMemory / PAGE_SIZE;
        // Place bitmap right after kernel (1MB mark)
        bitmap = (u8*)0x100000;
        
        // Poison the bitmap area for debugging
        for (u32 i = 0; i < (totalPages / 8); i++) {
            bitmap[i] = 0xFF;  // Mark all as used initially
        }
        
        // Just set stats, skip actual bitmap init for now
        usedPages = 256;
        freePages = totalPages - 256;
        
        VGA::puts("[MM] PMM initialized with memory guards\n");
    }
    
    // Get allocation statistics
    void getStats(u32& allocs, u32& frees, u32& violations) {
        allocs = totalAllocations;
        frees = totalFrees;
        violations = guardViolations;
    }
    
    bool isPageUsed(u32 page) {
        u32 byte = page / 8;
        u32 bit = page % 8;
        return (bitmap[byte] >> bit) & 1;
    }
    
    // Poison a memory region
    void poisonRegion(void* ptr, u32 size, u32 pattern) {
        u32* p = (u32*)ptr;
        u32 count = size / 4;
        for (u32 i = 0; i < count; i++) {
            p[i] = pattern;
        }
    }
    
    void* allocPage() {
        for (u32 i = 256; i < totalPages; i++) {
            if (!isPageUsed(i)) {
                setPage(i, true);
                usedPages++;
                freePages--;
                totalAllocations++;
                void* ptr = (void*)(i * PAGE_SIZE);
                poisonRegion(ptr, PAGE_SIZE, GUARD_POISON_ALLOCATED);
                return ptr;
                return (void*)(i * PAGE_SIZE);
            }
        }
        return nullptr; // Out of memory
    }
    
    void freePage(void* addr) {
        u32 page = (u32)addr / PAGE_SIZE;
        if (page >= 256 && page < totalPages) {
            setPage(page, false);
            usedPages--;
            freePages++;
        }
    }
    
    u32 getFreePages() { return freePages; }
    u32 getUsedPages() { return usedPages; }
}

// ===== SIMPLE HEAP ALLOCATOR =====
// Bump allocator for small objects
namespace Heap {
    static u8* heapStart = (u8*)0x200000; // 2MB mark
    static u8* heapEnd = (u8*)0x400000;   // 4MB mark
    static u8* heapPtr = heapStart;
    
    void init() {
        heapPtr = heapStart;
    }
    
    void* malloc(u32 size) {
        // Add header and footer for memory guards
        u32 totalSize = size + sizeof(AllocHeader) + sizeof(AllocFooter);
        
        // Align to 16 bytes
        totalSize = (totalSize + 15) & ~15;
        
        if (heapPtr + totalSize > heapEnd) {
            VGA::puts("[MM] malloc: Out of memory!\n");
            return nullptr; // Out of heap space
        }
        
        // Set up guard header
        AllocHeader* header = (AllocHeader*)heapPtr;
        header->magic = GUARD_HEADER_MAGIC;
        header->size = size;
        header->fence = GUARD_FENCE_MAGIC;
        
        // Set up guard footer
        AllocFooter* footer = (AllocFooter*)(heapPtr + sizeof(AllocHeader) + size);
        footer->fence = GUARD_FENCE_MAGIC;
        footer->checksum = header->magic ^ header->size ^ header->fence;
        
        // Poison the allocated memory
        u32* dataPtr = (u32*)(heapPtr + sizeof(AllocHeader));
        u32 dataWords = size / 4;
        for (u32 i = 0; i < dataWords; i++) {
            dataPtr[i] = GUARD_POISON_ALLOCATED;
        }
        
        void* userPtr = heapPtr + sizeof(AllocHeader);
        heapPtr += totalSize;
        
        VGA::puts("[MM] malloc: 0x"); VGA::puthex((u32)userPtr); 
        VGA::puts(" size="); VGA::puthex(size); VGA::puts("\n");
        
        return userPtr;
    }
    
    void free(void* ptr) {
        if (ptr == nullptr) return;
        
        // Check guard header
        AllocHeader* header = (AllocHeader*)((u8*)ptr - sizeof(AllocHeader));
        
        if (header->magic != GUARD_HEADER_MAGIC) {
            VGA::puts("[MM] ERROR: Heap corruption detected (bad magic)!\n");
            PMM::guardViolations++;
            return;
        }
        
        if (header->fence != GUARD_FENCE_MAGIC) {
            VGA::puts("[MM] ERROR: Heap underflow detected!\n");
            PMM::guardViolations++;
            return;
        }
        
        // Check guard footer
        AllocFooter* footer = (AllocFooter*)((u8*)ptr + header->size);
        
        if (footer->fence != GUARD_FENCE_MAGIC) {
            VGA::puts("[MM] ERROR: Heap overflow detected!\n");
            PMM::guardViolations++;
            return;
        }
        
        u32 checksum = header->magic ^ header->size ^ header->fence;
        if (footer->checksum != checksum) {
            VGA::puts("[MM] ERROR: Heap checksum mismatch!\n");
            PMM::guardViolations++;
            return;
        }
        
        // Poison the memory before "freeing"
        u32* dataPtr = (u32*)ptr;
        u32 dataWords = header->size / 4;
        for (u32 i = 0; i < dataWords; i++) {
            dataPtr[i] = GUARD_POISON_FREED;
        }
        
        header->magic = GUARD_POISON_FREED;  // Invalidate
        
        VGA::puts("[MM] free: 0x"); VGA::puthex((u32)ptr); VGA::puts("\n");
        
        // Note: Simple bump allocator doesn't actually reclaim memory
        PMM::totalFrees++;
    }
    
    u32 getUsed() { return heapPtr - heapStart; }
    u32 getFree() { return heapEnd - heapPtr; }
    
    // Validate a heap pointer
    bool validate(void* ptr) {
        if (ptr == nullptr) return false;
        
        AllocHeader* header = (AllocHeader*)((u8*)ptr - sizeof(AllocHeader));
        
        if (header->magic != GUARD_HEADER_MAGIC) return false;
        if (header->fence != GUARD_FENCE_MAGIC) return false;
        
        AllocFooter* footer = (AllocFooter*)((u8*)ptr + header->size);
        
        if (footer->fence != GUARD_FENCE_MAGIC) return false;
        
        u32 checksum = header->magic ^ header->size ^ header->fence;
        if (footer->checksum != checksum) return false;
        
        return true;
    }
}

// ===== Public Interface =====
void init(u32 totalMemory) {
    PMM::init(totalMemory);
    Heap::init();
}

u32 getFreePages() { return PMM::getFreePages(); }
u32 getUsedPages() { return PMM::getUsedPages(); }

// ===== STACK PROTECTION =====
namespace Stack {
    // Simple stack canary protection
    static u32 stackCanary = GUARD_CANARY_VALUE;
    
    void initCanary() {
        // Randomize canary value (simple XOR with timer or similar)
        // For now, use a fixed pattern
        stackCanary = GUARD_CANARY_VALUE;
    }
    
    u32 getCanary() {
        return stackCanary;
    }
    
    bool checkCanary() {
        return stackCanary == GUARD_CANARY_VALUE;
    }
    
    // Get current stack pointer
    u32 getStackPointer() {
        u32 sp;
        __asm__ __volatile__ ("mov %%esp, %0" : "=r" (sp));
        return sp;
    }
    
    // Check if address is within stack bounds
    bool isValidStack(u32 addr, u32 stackTop, u32 stackSize) {
        return (addr >= stackTop - stackSize && addr < stackTop);
    }
}

// ===== PUBLIC GUARD INTERFACE =====
void checkMemoryGuards() {
    u32 allocs, frees, violations;
    PMM::getStats(allocs, frees, violations);
    
    VGA::puts("[MM] Stats: allocs="); VGA::puthex(allocs);
    VGA::puts(" frees="); VGA::puthex(frees);
    VGA::puts(" violations="); VGA::puthex(violations);
    VGA::puts("\n");
    
    if (violations > 0) {
        VGA::puts("[MM] WARNING: Memory guard violations detected!\n");
    }
    
    if (!Stack::checkCanary()) {
        VGA::puts("[MM] CRITICAL: Stack canary corrupted!\n");
    }
}

}
