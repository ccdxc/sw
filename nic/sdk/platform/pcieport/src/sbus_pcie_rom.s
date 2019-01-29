
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

        .section ".rodata"
        .globl  sbus_pcie_rom_start
        .globl  sbus_pcie_rom_end
sbus_pcie_rom_start:
        .incbin "sbus_pcie_rom.bin"
sbus_pcie_rom_end:
