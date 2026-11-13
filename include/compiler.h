/**
 * @file compiler.h
 * @author verner002
 * @date 06/05/2026
*/

#pragma once

#define likely(x)           __builtin_expect(!!(x), 1)
#define unlikely(x)         __builtin_expect(!!(x), 0)