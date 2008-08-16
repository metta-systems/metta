#!/bin/bash

echo "Updating bootable image, enter your sudo password to continue:"
mkdir -p mountpoint
sudo mount -o loop floppy.img mountpoint
sudo cp -f build/default/vesper mountpoint/kernel
sudo umount mountpoint
rmdir mountpoint
touch floppy.img
