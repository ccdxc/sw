#!/usr/bin/python

import struct
import sys

SBUS_ROM_MAGIC = 0x53554253

if len(sys.argv) < 3:
    print >> sys.stderr, "usage: %s infile outfile" % sys.argv[0]
    exit(1)

src_path = sys.argv[1]
dst_path = sys.argv[2]

with open(src_path, 'r') as f:
    rom_in = f.read().splitlines()

l = len(rom_in)
rom_out = bytearray(struct.pack("<II", SBUS_ROM_MAGIC, l))

for i in range(0, l, 4):
    sz = min(l - i, 4)
    b = [0] + [int(rom_in[i + j], 16) for j in range(sz)] + [0] * (5 - sz)
    rom_out += bytearray([((b[j] & (0xff >> (8 - 2 * j))) << 8 - (2 * j)) |
                           (b[j + 1] >> (2 + 2 * j)) for j in range(5)])

with open(dst_path, 'wb') as f:
    f.write(rom_out)

