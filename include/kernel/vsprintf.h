/**
 * @file vsprintf.h
 * @author verner002
 * @date 09/09/2026
*/

#pragma once

#include <stdarg.h>

int vsprintf(char *buf, char const *fmt, va_list args);