/**
 * Userland
 * 
 * Author: verner002
*/

/**
 * Declarations
*/

uint32_t __get_eip(void);
void __flush_tss(void);
void __exec_kernelmode(uint32_t eip);
void __exec_usermode(uint32_t eip);