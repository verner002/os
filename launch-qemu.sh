#!/bin/bash

qemu-system-x86_64 \
    -no-reboot -no-shutdown \
    -boot order=a \
    -fda ./fdd.img \
    -drive file=disk.img,format=raw,if=ide,index=0 \
    -d int,cpu_reset -D qemu.log \
    -debugcon file:/tmp/qemu_e9 \
    -serial tcp:127.0.0.1:4444,server,nowait
    #-device piix3-ide,id=ide \
    #-drive id=disk,file=disk.img,format=raw,if=none \
    #-device ide-hd,drive=disk,bus=ide.0 #\
    #-netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
    #-device e1000,netdev=net0,mac=12:34:56:78:9a:bc \
    #-object filter-dump,id=dump0,netdev=net0,file=net.pcap

#qemu-system-x86_64 -fda ./fdd.img -device piix3-ide,id=ide -drive id=disk,file=disk.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -nic user,model=e1000,mac=12:34:56:78:9a:bc
