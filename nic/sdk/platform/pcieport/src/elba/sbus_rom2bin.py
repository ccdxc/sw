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
nw = (l + 2) / 3
rom_out = bytearray(struct.pack("<II", SBUS_ROM_MAGIC, nw))

for i in range(0, l, 3):
    n = min(l - i, 3)
    h = rom_in[i:i+n]
    rom_out += bytearray(struct.pack("<I",
                                     (n << 30) |
                                     sum([int(h[j], 16) << (j * 10)
                                          for j in range(n)])))

with open(dst_path, 'wb') as f:
    f.write(rom_out)

