/*
 * scheduler.cpp - Cooperative/Preemptive Multitasking Implementation
 * Version: 1.0
 */
 
#include "scheduler.h"
#include "memory.h"
 
namespace Scheduler {
    // Task table and current state
    static Task tasks[MAX_TASKS];
    static u32 currentTaskIndex = 0;
    static u32 nextTaskId = 1;
    static u32 taskCount = 0;
    static bool multitaskingActive = false;
    static u32 kernelStack = 0;
    static Task* currentTask = nullptr;
    
    // C-linkage exports for assembly
    extern "C" {
        bool* getMultitaskingFlag() { return &multitaskingActive; }
        Task** getCurrentTaskPtr() { return &currentTask; }
    }
    
    // Get current task
    Task* getCurrentTask() { return currentTask; }
    
    // Initialize scheduler
    void init() {
        for (u32 i = 0; i < MAX_TASKS; i++) {
            tasks[i].id = 0;
            tasks[i].state = TASK_EMPTY;
            tasks[i].esp = 0;
            tasks[i].ebp = 0;
            tasks[i].eip = 0;
            tasks[i].eax = tasks[i].ebx = tasks[i].ecx = tasks[i].edx = 0;
            tasks[i].esi = tasks[i].edi = 0;
            tasks[i].eflags = 0;
            tasks[i].cr3 = 0;
            tasks[i].stackBase = 0;
            tasks[i].stackSize = 0;
            tasks[i].priority = 0;
            tasks[i].timeSlice = 0;
            tasks[i].totalTicks = 0;
            tasks[i].name[0] = '\0';
        }
        currentTaskIndex = 0;
        nextTaskId = 1;
        taskCount = 0;
        multitaskingActive = false;
        currentTask = nullptr;
    }
    
    // Create a new task
    u32 createTask(void (*entryPoint)(), const char* name, u32 priority) {
        u32 slot = 0;
        for (; slot < MAX_TASKS; slot++) {
            if (tasks[slot].state == TASK_EMPTY) break;
        }
        if (slot >= MAX_TASKS) return 0;
        
        u32 stackSize = DEFAULT_STACK_SIZE;
        u32 stackBase = (u32)Memory::Heap::malloc(stackSize);
        if (!stackBase) return 0;
        
        Task* task = &tasks[slot];
        task->id = nextTaskId++;
        task->state = TASK_READY;
        task->stackBase = stackBase;
        task->stackSize = stackSize;
        task->priority = (priority > 15) ? 15 : priority;
        task->timeSlice = DEFAULT_TIME_SLICE;
        task->totalTicks = 0;
        
        int i = 0;
        while (name[i] && i < 31) {
            task->name[i] = name[i];
            i++;
        }
        task->name[i] = '\0';
        
        task->esp = stackBase + stackSize - 4;
        task->ebp = task->esp;
        task->eip = (u32)entryPoint;
        task->eflags = 0x202;
        
        task->eax = task->ebx = task->ecx = task->edx = 0;
        task->esi = task->edi = 0;
        
        taskCount++;
        return task->id;
    }
    
    // Get task by ID
    Task* getTask(u32 id) {
        for (u32 i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].id == id && tasks[i].state != TASK_EMPTY) {
                return &tasks[i];
            }
        }
        return nullptr;
    }
    
    // Get task count
    u32 getTaskCount() { return taskCount; }
    
    // Get current task ID
    u32 getCurrentTaskId() { return currentTask ? currentTask->id : 0; }
    
    // Check if multitasking is active
    bool isMultitasking() { return multitaskingActive; }
    
    // Terminate current task
    void terminateCurrentTask() {
        if (currentTask && currentTask->state != TASK_EMPTY) {
            currentTask->state = TASK_TERMINATED;
            taskCount--;
            if (currentTask->stackBase) {
                Memory::Heap::free((void*)currentTask->stackBase);
            }
            yield();
        }
    }
    
    // Terminate specific task
    void terminateTask(u32 taskId) {
        if (currentTask && currentTask->id == taskId) {
            terminateCurrentTask();
            return;
        }
        Task* task = getTask(taskId);
        if (task && task->state != TASK_EMPTY) {
            task->state = TASK_TERMINATED;
            taskCount--;
            if (task->stackBase) {
                Memory::Heap::free((void*)task->stackBase);
            }
        }
    }
    
    // Block a task
    void blockTask(u32 taskId) {
        Task* task = getTask(taskId);
        if (task && task->state == TASK_READY) {
            task->state = TASK_BLOCKED;
        }
    }
    
    // Unblock a task
    void unblockTask(u32 taskId) {
        Task* task = getTask(taskId);
        if (task && task->state == TASK_BLOCKED) {
            task->state = TASK_READY;
        }
    }
    
    // Yield CPU
    void yield() {
        if (!multitaskingActive || taskCount <= 1) return;
    }
    
    // Simple sleep
    void sleep(u32 ms) {
        u32 end = ms * 10000;
        for (volatile u32 i = 0; i < end; i++);
    }
    
    // Find next ready task
    static u32 findNextReadyTask() {
        if (taskCount == 0) return (u32)-1;
        u32 startIndex = currentTaskIndex;
        u32 index = (startIndex + 1) % MAX_TASKS;
        for (u32 checked = 0; checked < MAX_TASKS; checked++) {
            if (tasks[index].state == TASK_READY || 
                (index == startIndex && tasks[index].state == TASK_RUNNING)) {
                return index;
            }
            index = (index + 1) % MAX_TASKS;
        }
        return (u32)-1;
    }
    
    // Main scheduler
    void schedule() {
        if (!multitaskingActive || taskCount == 0) return;
        if (currentTask && currentTask->timeSlice > 0) {
            currentTask->timeSlice--;
            currentTask->totalTicks++;
            if (currentTask->timeSlice > 0 && currentTask->state == TASK_RUNNING) {
                return;
            }
        }
        u32 nextIndex = findNextReadyTask();
        if (nextIndex == (u32)-1 || nextIndex == currentTaskIndex) {
            if (currentTask) {
                currentTask->timeSlice = DEFAULT_TIME_SLICE;
            }
            return;
        }
        Task* oldTask = currentTask;
        Task* nextTask = &tasks[nextIndex];
        if (oldTask && oldTask->state == TASK_RUNNING) {
            oldTask->state = TASK_READY;
            oldTask->timeSlice = DEFAULT_TIME_SLICE;
        }
        nextTask->state = TASK_RUNNING;
        nextTask->timeSlice = DEFAULT_TIME_SLICE;
        currentTask = nextTask;
        currentTaskIndex = nextIndex;
    }
    
    // Start scheduler
    void start() {
        if (taskCount == 0) return;
        for (u32 i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].state == TASK_READY) {
                currentTaskIndex = i;
                currentTask = &tasks[i];
                tasks[i].state = TASK_RUNNING;
                tasks[i].timeSlice = DEFAULT_TIME_SLICE;
                multitaskingActive = true;
                return;
            }
        }
    }
    
    // Stop scheduler
    void stop() {
        multitaskingActive = false;
    }
    
    // List tasks
    void listTasks() {
    }
    
    // Switch task
    void switchTask(Task* next) {
        currentTask = next;
        currentTaskIndex = 0;
        for (u32 i = 0; i < MAX_TASKS; i++) {
            if (&tasks[i] == next) {
                currentTaskIndex = i;
                break;
            }
        }
    }
}
