rem thanks http://chdk.wikia.com/wiki/GPL_Disassembling
del kernel.elf
P:\prog\devkitPPC\bin\powerpc-eabi-objcopy -I binary -O elf32-powerpc -B powerpc --change-addresses=0xffe00000 --set-section-flags .data=code kernel.out kernel.elf

P:\prog\devkitPPC\bin\powerpc-eabi-objdump -d kernel.elf >kernel_disasm.txt