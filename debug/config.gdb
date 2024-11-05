target remote localhost:1234
set disassembly-flavor intel
set architecture i8086
break *0x7c00
c

# use to step by single instruction
define s
    si
    x/i $eip
    end

# use to step over call and int instruction
define n
    x/2i $eip
    tbreak *$_
    c
    end