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
void __exec_kernelmode(uint32_t eip, uint32_t esp, uint32_t ebp);
void __exec_usermode(uint32_t eip);