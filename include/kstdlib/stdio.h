/**
 * Standard Input/Output
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "types.h"
#include "drivers/cpu.h"
#include "drivers/vga.h"
#include "kernel/mutex.h"
#include "mm/heap.h"
#include "kernel/task.h"
#include "kstdlib/math.h"
#include "kstdlib/errno.h"
#include "kstdlib/file.h"

/**
 * Constants
*/

#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

#define EOF 0
#define FILE_EOF 1 // eof flag

/**
 * Types Definitions
*/

typedef struct __dir DIR;
typedef __builtin_va_list va_list;

/**
 * Structures
*/

struct __dir {
    char *__buffer; // buffer base
    char *__dname; // dirname
};

/**
 * Global Variables
*/

extern FILE
    *stdin,
    *stdout,
    *stderr;

/**
 * Declarations
*/

bool feof(FILE *stream);
int getc(FILE *stream);
int getchar(void);
int putc(int c, FILE *stream);
int putchar(int c);
int vfprintf(FILE *stream, char const *s, va_list args);
int vprintf(char const *s, va_list args);
int fprintf(FILE *stream, char const *s, ...);
int printf(char const *s, ...);
int puts(char const *s);
void printk(char const *s, ...);