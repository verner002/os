/**
 * @file paging.h
 * @author verner002
 * @date 19/09/2026
*/

#pragma once

#define PAGE_SIZE       4096

#define PAGE_P          1
#define PAGE_RW         2
#define PAGE_US         4
#define PAGE_WT         8
#define PAGE_CD         16

int kmap(void *dir, void *virt, void *phys, int flags);