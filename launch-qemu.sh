#!/bin/bash

qemu-system-x86_64 \
    -machine isapc \
    -cpu 486 \
    -m 32M \
    -accel  tcg,tb-size=1 \
    -no-reboot -no-shutdown \
    -boot order=a \
    -fda ./fdd.img \
    -drive file=disk.img,format=raw,if=ide,index=0 \
    -d guest_errors,unimp,int,cpu_reset -D qemu.log \
    -debugcon file:/tmp/qemu_e9 \
    -serial tcp:127.0.0.1:4444,server,nowait \
    -nic none
    #-icount shift=2 \
    #-device piix3-ide,id=ide \
    #-drive id=disk,file=disk.img,format=raw,if=none \
    #-device ide-hd,drive=disk,bus=ide.0 #\
    #-netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
    #-device e1000,netdev=net0,mac=12:34:56:78:9a:bc \
    #-object filter-dump,id=dump0,netdev=net0,file=net.pcap

#qemu-system-x86_64 -fda ./fdd.img -device piix3-ide,id=ide -drive id=disk,file=disk.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -nic user,model=e1000,mac=12:34:56:78:9a:bc
