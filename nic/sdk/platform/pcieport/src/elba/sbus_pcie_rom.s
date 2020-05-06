
/*
 * Copyright (c) 2018,2020, Pensando Systems Inc.
 */

        .section ".rodata"
        .globl  sbus_pcie_rom_start

        .globl  sbus_pcie_rom_0x10AA_2347_start
        .balign 32
sbus_pcie_rom_start:
sbus_pcie_rom_0x10AA_2347_start:
        .incbin "serdes.0x10AA_2347.bin"
