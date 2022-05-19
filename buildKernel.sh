#!/bin/bash
pwd
#Build and install modules
sudo make -j 4 Image modules dtbs
sudo make -j 4 modules_install
sleep 5
# Copy files to system
sudo cp arch/arm64/boot/dts/broadcom/*.dtb /boot/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* /boot/overlays/
sudo cp arch/arm64/boot/dts/overlays/README /boot/overlays/
sudo cp arch/arm64/boot/Image /boot/rtospkernel.img
# Reboot to apply changes
reboot

