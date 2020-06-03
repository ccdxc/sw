#!/usr/bin/python

import struct
import sys
import parser
def auto_int(x):
    return int(x, 0)

SBUS_ROM_MAGIC = 0x53554253

import argparse
parser = argparse.ArgumentParser()
parser.add_argument( '-s', dest="src_path", type=str,help="source rom file", required=True)
parser.add_argument('-d', dest="dest_path", type=str,help="destination bin file", required=True)
parser.add_argument("--build_id",type=auto_int, help="build id value to check", default=0, required=False)
parser.add_argument("--rev_id", type=auto_int, help="rev id value to check", default=0, required=False)


args = parser.parse_args()

src_path =  args.src_path
dst_path =  args.dest_path
build_id = args.build_id
rev_id = args.rev_id

print(src_path, dst_path, build_id, rev_id);

with open(src_path, 'r') as f:
    rom_in = f.read().splitlines()

l = len(rom_in)
nw = ((l + 7)/8)*3
rom_out = bytearray(struct.pack("<II", SBUS_ROM_MAGIC, nw))
rom_out += bytearray(struct.pack("<I", build_id))
rom_out += bytearray(struct.pack("<I", rev_id))

for i in range(0, l, 8):
    n = min(l - i, 8)
    data = rom_in[i:i+n]
    data = [int('0x' + j, 16) for j in data] 
    

    imem_data0 = data[0] | (data[1] << 12) | ((data[2] & 0xff) << 24);
    imem_data1 = ((data[2] >> 8) & 0xf) | (data[3] << 4) | (data[4] << 16) |  ((data[5] & 0xf) << 28);
    imem_data2 = ((data[5] >> 4) & 0xff) | (data[6] << 8) | ((data[7] & 0xfff) << 20);
    
    rom_out += bytearray(struct.pack("<I", imem_data0)); 
    rom_out += bytearray(struct.pack("<I", imem_data1)); 
    rom_out += bytearray(struct.pack("<I", imem_data2)); 


with open(dst_path, 'wb') as f:
    f.write(rom_out)

