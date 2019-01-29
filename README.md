# FreeRTOS Ported to Raspberry Pi

This provides a very basic port of FreeRTOS to Raspberry pi.

## Howto Build

Type make! -- If you get an error then:

```shell
$ cd .dbuild/pretty
$ chmod +x *.py
```

Currently the makefile expect an arm-none-eabi- toolchain in the path. Either export the path to yours or
modify the TOOLCHAIN variable in dbuild.config.mk file.

You may also need to modify the library locations in the Makefile:

```makefile
kernel.elf: LDFLAGS += -L"/usr/lib/gcc/arm-none-eabi/4.9.3/" -lgcc
kernel.elf: LDFLAGS += -L"/usr/lib/arm-none-eabi/lib/" -lc -lm
```

The build system also expects find your python interpreter by using /usr/bin/env python,
if this doesn't work you will get problems.

To resolve this, modify the #! lines in the .dbuild/pretty/*.py files.

Hope this helps.

