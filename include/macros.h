/**
 * Macros
 * 
 * Author: verner002
*/

#pragma once

/**
 * Includes
*/

#include "kstdlib/stdio.h"

/**
 * Macros
*/

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define lambda(lambda_ret, lambda_args, lambda_body) {  \
    lambda_ret lambda_func lambda_args lambda_body      \
    &lambda_func;                                       \
}

#define INFO(__string) do { printf("\033[1m\033[36m%s:%s:%d: %s\033[0m\n", __FILE__, __func__, __LINE__, __string); } while (0);
#define FATAL(__string) do  { printf("\033[1m\033[31m%s:%s:%d: %s\033[0m\n", __FILE__, __func__, __LINE__, __string); /*exit(-1);*/ while (1); } while (0);

#define TODO FATAL("reached todo");
#define UNREACHABLE FATAL("should be unreachable");