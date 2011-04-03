#!/bin/sh
# Script taken from Pedigree and slightly modified

BUILDDIR=../../_build_/x86-pc99-release
#FLOPPY=$BUILDDIR/metta.fd0
CDISO=$BUILDDIR/metta.iso

# Create directory fd
#mkdir -p fd
#chmod 0777 fd

# Create empty image
#dd if=/dev/zero of=$FLOPPY bs=1024 count=1440

# Format the image
#/sbin/mke2fs -L Metta -m 0 -F $FLOPPY

# Mount the Ext2 image
#sudo mount -o loop -t ext2 $FLOPPY fd
#mkdir -p fd/boot/grub
#cp stage{1,2} fd/boot/grub
#cp $BUILDDIR/tools/mkbootimg/menu.lst.fd0  fd/boot/grub/menu.lst
#cp $BUILDDIR/kickstart.sys      fd/kickstart
#cp $BUILDDIR/kernel-startup.sys fd/kernel-startup
#cp $BUILDDIR/init.img           fd/system-bootimage
#sudo umount fd
#rm -rf fd

# Install grub
#/sbin/grub --batch --no-floppy <<EOT || exit 1
#device (fd0) $FLOPPY
#install (fd0)/boot/grub/stage1 (fd0) (fd0)/boot/grub/stage2 (fd0)/boot/grub/menu.lst
#quit
#EOT

# Create an ISO image
mkdir -p iso/boot/grub
cp stage2_eltorito iso/boot/grub
cp $BUILDDIR/tools/mkbootimg/menu.lst.cd   iso/boot/grub/menu.lst
cp $BUILDDIR/kickstart.sys      iso/kickstart
cp $BUILDDIR/kernel-startup.sys iso/kernel-startup
cp $BUILDDIR/init.img           iso/system-bootimage
mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $CDISO iso
rm -rf iso
