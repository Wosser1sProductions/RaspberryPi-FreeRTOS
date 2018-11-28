#
#	Makefile for FreeRTOS demo on Raspberry Pi
#
BASE=$(shell pwd)/
BUILD_DIR=$(shell pwd)/build/

MODULE_NAME="RaspberryPi BSP"

TARGETS=kernel.img kernel.list kernel.syms kernel.elf
LINKER_SCRIPT=raspberrypi.ld

-include .dbuild/dbuild.mk


QEMU = /usr/bin/qemu-system-arm

.PHONY: emu
emu: all
	$(QEMU) -kernel kernel.elf -cpu arm1176 -m 512 -M raspi -nographic -M versatilepb
    
.PHONY: debug-emu
debug-emu: all
	$(QEMU) -s -S -kernel kernel.elf -cpu arm1176 -m 512 -M raspi -nographic -M versatilepb


all: kernel.list kernel.img kernel.syms
	@$(SIZE) kernel.elf

kernel.img: kernel.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

kernel.list: kernel.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D -S  kernel.elf > $@

kernel.syms: kernel.elf
	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -t kernel.elf > $@

#kernel.elf: LDFLAGS += -L "/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/lib/gcc/arm-xilinx-eabi/4.6.1/" -lgcc
#kernel.elf: LDFLAGS += -L "/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/arm-xilinx-eabi/lib/" -lc
kernel.elf: CFLAGS += -std=c11 -lm
kernel.elf: LDFLAGS += -L "/usr/lib/gcc/arm-none-eabi/4.9.3/" -lgcc
kernel.elf: LDFLAGS += -L "/usr/lib/arm-none-eabi/lib/" -lc -lm
kernel.elf: $(OBJECTS)
	$(Q)$(LD) -nostartfiles -lnosys $(OBJECTS) -lm -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(LDFLAGS)
