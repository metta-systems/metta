#!/bin/sh
# Script taken from Pedigree and slightly modified

IMAGEDIR=../../images
BUILDDIR=../../_build_/x86-pc99-release

# Create directory fd
mkdir fd
chmod 0777 fd

# Create empty image
dd if=/dev/zero of=$IMAGEDIR/floppy.img bs=1024 count=1440

# Format the image
mke2fs -F $IMAGEDIR/floppy.img

# Mount the Ext2 image
sudo mount -oloop $IMAGEDIR/floppy.img fd
mkdir -p fd/boot/grub
cp menu.lst stage{1,2} fd/boot/grub
sudo umount fd

# Install grub
grub --batch --no-floppy <<EOT 1>/dev/null  || exit 1
device (fd0) $IMAGEDIR/floppy.img
install (fd0)/boot/grub/stage1 (fd0) (fd0)/boot/grub/stage2 (fd0)/boot/grub/menu.lst
quit
EOT

# Delete directory fd
rmdir fd

# Create an ISO image
mkdir -p iso/boot/grub
cp stage2_eltorito iso/boot/grub
cp menu.lst.cd iso/boot/grub/menu.lst
mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $IMAGEDIR/metta.iso iso
rm -rf iso
