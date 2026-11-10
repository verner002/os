/**
 * @file valist.h
 * @author verner002
 * @date 14/05/2026
*/

#pragma once

#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l) __builtin_va_arg(v, l)
#define va_end(v) __builtin_va_end(v)

typedef __builtin_va_list va_list;