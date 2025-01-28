#!/bin/bash

# Create directories.
mkdir -p isodir/boot/grub

# Copy kernel and GRUB configuration to the appropriate locations.
cp build/Luna.bin isodir/boot/tinyos.bin
cp grub.cfg isodir/boot/grub/grub.cfg

# Create the ISO image with xorriso.
xorriso -as mkisofs -R -b boot/grub/i386-pc/eltorito.img -no-emul-boot -boot-load-size 4 -boot-info-table -o Luna.iso isodir