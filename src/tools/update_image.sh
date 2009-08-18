#!/bin/bash

echo "Updating bootable image, enter your sudo password to continue:"
mkdir -p mountpoint
sudo mount -o loop floppy.img mountpoint
sudo cp -f _build_/x86-pc99-release/vesper/bootloader mountpoint/bootloader
#sudo cp -f _build_/x86-pc99-release/vesper/nucleus mountpoint/kernel
sudo cp -f _build_/x86-pc99-release/vesper/initcomp   mountpoint/initcomp
sudo cp -f _build_/x86-pc99-release/vesper/initfs.img mountpoint/initfs
sudo umount mountpoint
rmdir mountpoint
touch floppy.img
