#PhenixPro_Devkit_Platform

## File/Dir Introduction

-   BOOT.bin:             ZYNQ boot file
-   create_boot.sh:     script that create ZYNQ boot file
-   devicetree.dtb:      Linux devicetree
-   uImage:                  Linux kernel 
-   filesystem:             include ramdisk and ubuntu core 
-   fpga/:                      fpga image
-   fsbl.elf:                   first stage bootloader 
-   output.bif:             format file of BOOT.bin, input file of Xilinx Bootgen utility 
-   test/:                       test programs 
-   u-boot.elf:             uboot image 
-   uEnv.txt:                uboot parameters 
-   vivado_prj.tar.bz2:   vivado project

## Quick Start

PC Develop environment Ubuntu 16.04

### Ubuntu Core

1. Divide the SD card into two partitions, Boot and rootfs for example

2. Copy these files to Boot partition:
   - BOOT.bin
   - devicetree.dtb
   - filesystem/ubuntu-core/uEnv.txt
   - uImage

3. Install ubuntu core filesystem to SD Card:

   ```
   sudo tar -xpf filesystem/ubuntu-core-16.04-robsense.tar.bz2 -C /media/[PC username]/rootfs
   umount /media/[PC username]/*
   ```

There is a pre-build Ardupilot in ubuntu core filesystem, and it will start automatically after ubuntu core boot-up.

**PS: ubuntu core default user: robsense, password: robsense**

### Ramdisk

1. Copy these files to SD Card:
   - BOOT.bin
   - devicetree.dtb
   - uImage
   - filesystem/ramdisk/uramdisk.image.gz
   - filesystem/ramdisk/uEnv.txt

There is a pre-build Ardupilot in ramdisk filesystem, and it will start automatically after Linux boot-up.

