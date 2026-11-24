/**
 * @file paging.h
 * @author verner002
 * @date 19/09/2026
*/

#pragma once

#define PAGE_SIZE       4096u

#define PAGE_P          1u
#define PAGE_RW         2u
#define PAGE_US         4u
#define PAGE_WT         8u
#define PAGE_CD         16u

int kmap(void *dir, void *virt, void *phys, int flags);
size_t copy_from_user(void *dst, void const *src, size_t size);
size_t copy_to_user(void *dst, void const *src, size_t size);