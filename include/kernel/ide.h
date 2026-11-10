/**
 * @file ide.h
 * @author verner002
 * @date 04/09/2025
*/

#pragma once

void reqinfo(void);
int ide_read_func(int minor, unsigned long lba, unsigned long count, char *buffer);