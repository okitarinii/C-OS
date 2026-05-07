/*
 * scheduler.h - Cooperative/Preemptive Multitasking Scheduler
 * Version: 1.0
 * Features: Round-robin scheduling, context switch, task management
 */

#pragma once

#include "types.h"

namespace Scheduler {
    // Task states
    enum TaskState {
        TASK_EMPTY = 0,      // Slot not used
        TASK_READY,          // Ready to run
        TASK_RUNNING,        // Currently executing
        TASK_BLOCKED,        // Waiting for something
        TASK_TERMINATED      // Finished execution
    };
    
    // Task structure
    struct Task {
        u32 id;              // Task ID
        TaskState state;     // Current state
        u32 esp;             // Stack pointer
        u32 ebp;             // Base pointer
        u32 eip;             // Instruction pointer
        u32 eax, ebx, ecx, edx;  // General registers
        u32 esi, edi;        // Index registers
        u32 eflags;          // Flags
        u32 cr3;             // Page directory (for future virtual memory)
        u32 stackBase;       // Stack base address
        u32 stackSize;       // Stack size
        u32 priority;        // Task priority (0-15, higher = more priority)
        u32 timeSlice;       // Remaining time slice (in timer ticks)
        u32 totalTicks;      // Total CPU ticks consumed
        char name[32];       // Task name for debugging
    };
    
    // Constants
    static const u32 MAX_TASKS = 16;
    static const u32 DEFAULT_STACK_SIZE = 8192;  // 8KB per task
    static const u32 DEFAULT_TIME_SLICE = 10;    // 10ms per task
    static const u32 KERNEL_STACK_TOP = 0x100000; // 1MB (after kernel)
    
    // Core functions
    void init();                                    // Initialize scheduler
    void start();                                   // Start multitasking
    void stop();                                    // Stop scheduler
    
    // Task management
    u32 createTask(void (*entryPoint)(), const char* name, u32 priority = 8);  // Create new task
    void terminateTask(u32 taskId);                 // Terminate specific task
    void terminateCurrentTask();                    // Terminate current task
    void blockTask(u32 taskId);                     // Block a task
    void unblockTask(u32 taskId);                   // Unblock a task
    void yield();                                   // Give up CPU time
    void sleep(u32 ms);                             // Sleep for milliseconds
    
    // Scheduler control
    void schedule();                                // Main scheduler function (called by timer)
    void switchTask(Task* next);                    // Context switch
    Task* getCurrentTask();                         // Get currently running task
    Task* getTask(u32 id);                          // Get task by ID
    u32 getTaskCount();                             // Get number of active tasks
    u32 getCurrentTaskId();                         // Get current task ID
    
    // Debug/Info
    void listTasks();                               // Print task list for debugging
    bool isMultitasking();                          // Check if scheduler is active
    
    // Assembly helper functions
    extern "C" {
        void contextSwitch(u32* oldEsp, u32 newEsp);  // Assembly context switch
        void firstTaskStart(u32 esp, u32 eip);        // Start first task
    }
}
