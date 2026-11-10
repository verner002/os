/**
 * @file 82077aa.h
 * @author verner002
 * @date 05/11/2025
*/

#pragma once

int fdc_init(void);
int fdc_read_func(int minor, unsigned long lba, unsigned long count, void *buffer);