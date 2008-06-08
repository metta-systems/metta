#!/bin/bash

mkdir -p mountpoint
sudo mount -o loop floppy.img mountpoint
sudo cp -f kernel.bin mountpoint/kernel
sudo umount mountpoint
rmdir mountpoint
