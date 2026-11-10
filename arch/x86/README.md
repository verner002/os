# Initialization Modules
Initialization modules are independent units designed for each architecture or system configuration creating an abstraction layer for the kernel. This makes the kernel code portable while init modules can be optimized for different systems. 

## Compilation


## x86 Initialization Module
### GCC x86 Options
The source code of the x86 init module is built with specific `gcc` options:
- `-mtune=i486` - tells compiler to optimize the code for Intel i486 CPU,
- `-m32` - tells compiler to set `int`, `long` and pointer types to 32 bits and to generate code that runs on any i386 system.

## Custom Initialization Module