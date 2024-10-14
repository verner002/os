/**
 * Standard Input/Output
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "kstd/types.h"
#include "kstd/vga.h"

/**
 * Constants
*/

#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

/**
 * Types Definitions
*/

typedef unsigned int FILE;
typedef __builtin_va_list va_list;

/**
 * Declarations
*/

int putc(int c, FILE *stream);
int putchar(int c);
int vfprintf(FILE *stream, char const *s, va_list args);
int vprintf(char const *s, va_list args);
int fprintf(FILE *stream, char const *s, ...);
int printf(char const *s, ...);