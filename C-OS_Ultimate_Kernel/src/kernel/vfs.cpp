// C-OS Ultimate - Virtual File System (Windows XP Structure)
#include "include/types.h"

namespace VFS {
    struct Node {
        char name[32];
        u32 type; // 0=file, 1=dir
        u32 size;
        Node* parent;
        Node* child;
        Node* next;
    };
    
    static Node root;
    static Node* currentDir;
    static int nodeCount = 0;
    
    static void strcpy(char* dst, const char* src) {
        while (*src) *dst++ = *src++;
        *dst = 0;
    }
    
    static Node* createNode(const char* name, u32 type) {
        Node* node = (Node*)0x300000; // Simple static allocation
        node = (Node*)((u8*)node + nodeCount * sizeof(Node));
        nodeCount++;
        
        for (int i = 0; i < 32; i++) node->name[i] = 0;
        strcpy(node->name, name);
        node->type = type;
        node->size = 0;
        node->parent = &root;
        node->child = nullptr;
        node->next = nullptr;
        return node;
    }
    
    void init() {
        // Root: C:
        for (int i = 0; i < 32; i++) root.name[i] = 0;
        root.name[0] = 'C'; root.name[1] = ':';
        root.type = 1;
        root.size = 0;
        root.parent = &root;
        root.child = nullptr;
        root.next = nullptr;
        
        // WINDOWS
        Node* windows = createNode("WINDOWS", 1);
        root.child = windows;
        
        // System32
        Node* system32 = createNode("System32", 1);
        windows->child = system32;
        
        // Documents and Settings
        Node* docs = createNode("Documents and Settings", 1);
        windows->next = docs;
        
        // Program Files
        Node* progfiles = createNode("Program Files", 1);
        docs->next = progfiles;
        
        currentDir = &root;
    }
}
