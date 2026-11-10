/**
 * @file stdio.h
 * @author verner002
 * @date 15/02/2026
*/

#pragma once

#include "valist.h"
#include "types.h"
#include "drivers/cpu.h"
#include "drivers/vga.h"
#include "kernel/mutex.h"
#include "mm/heap.h"
#include "kernel/task.h"
#include "kstdlib/math.h"
#include "kstdlib/errno.h"
#include "kstdlib/file.h"

#define EOF 0
#define FILE_EOF 1 // eof flag

extern FILE
    *stdin,
    *stdout,
    *stderr;

int feof(FILE *stream);
int getc(FILE *stream);
int getchar(void);
int putc(int c, FILE *stream);
int putchar(int c);
int vfprintf(FILE *stream, char const *str, va_list args);
int vprintf(char const *s, va_list args);
int fprintf(FILE *stream, char const *s, ...);
int printf(char const *s, ...);
int puts(char const *s);
void printk(char const *s, ...);