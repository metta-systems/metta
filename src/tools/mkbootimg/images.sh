#!/bin/sh
# Script taken from Pedigree and slightly modified

BUILDDIR=../../_build_/x86-pc99-release
FLOPPY=$BUILDDIR/metta.fd0
CDISO=$BUILDDIR/metta.iso

# Create directory fd
mkdir -p fd
chmod 0777 fd

# Create empty image
dd if=/dev/zero of=$FLOPPY bs=1024 count=1440

# Format the image
/sbin/mke2fs -L Metta -m 0 -F $FLOPPY

# Mount the Ext2 image
sudo mount -oloop $FLOPPY fd
mkdir -p fd/boot/grub
cp stage{1,2} fd/boot/grub
cp menu.lst.fd0 fd/boot/grub/menu.lst
cp $BUILDDIR/vesper/x86/kickstart.bin fd/kickstart
cp $BUILDDIR/vesper/x86/nucleus.bin   fd/nucleus
cp $BUILDDIR/initfs.img               fd/bootcomps
sudo umount fd
rm -rf fd

# Install grub
/sbin/grub --batch --no-floppy <<EOT 1>/dev/null  || exit 1
device (fd0) $FLOPPY
install (fd0)/boot/grub/stage1 (fd0) (fd0)/boot/grub/stage2 (fd0)/boot/grub/menu.lst
quit
EOT

# Create an ISO image
mkdir -p iso/boot/grub
cp stage2_eltorito iso/boot/grub
cp menu.lst.cd iso/boot/grub/menu.lst
cp $BUILDDIR/vesper/x86/kickstart.bin iso/kickstart
cp $BUILDDIR/vesper/x86/nucleus.bin   iso/nucleus
cp $BUILDDIR/initfs.img               iso/bootcomps
mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $CDISO iso
rm -rf iso
