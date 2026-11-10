/**
 * @file kmalloc.h
 * @author verner002
 * @date 09/10/2026
*/

#pragma once

#include <stddef.h>

void kheap_init(void *start, size_t size);
void *kmalloc(size_t size);
void *kzalloc(size_t size);
void kfree(void *ptr);