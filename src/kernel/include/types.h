// C-OS Ultimate - Common Types
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Basic types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// C++ bool is built-in, no typedef needed

// NULL
#define NULL ((void*)0)

// Inline assembly helpers
#define outb(port, value) __asm__ __volatile__ ("outb %%al, %%dx" : : "a"(value), "d"(port))
#define inb(port) ({ u8 _v; __asm__ __volatile__ ("inb %%dx, %%al" : "=a"(_v) : "d"(port)); _v; })
#define cli() __asm__ __volatile__ ("cli")
#define sti() __asm__ __volatile__ ("sti")
#define hlt() __asm__ __volatile__ ("hlt")

#endif
