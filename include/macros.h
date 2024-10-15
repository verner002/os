/**
 * Macros
 * 
 * Author: verner002
*/

#pragma once

/**
 * Constants
*/

#define lambda(lambda_ret, lambda_args, lambda_body) {  \
    lambda_ret lambda_func lambda_args lambda_body      \
    &lambda_func;                                       \
}