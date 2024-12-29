/**
 * Userland
 * 
 * Author: verner002
*/

/**
 * Declarations
*/

unsigned int __get_eip(void);
void __flush_tss(void);
void __exec_kernelmode(unsigned int eip);
void __exec_usermode(unsigned int eip);