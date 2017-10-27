#! /usr/bin/python3
''' Tool to parse log files for register writes and output a binary file compatible
    with capri_load_config()
'''
import pdb
import struct
import re
import argparse
import itertools
import os
import sys

#https://stackoverflow.com/questions/4322705/split-a-list-into-nested-lists-on-a-value
def isplit(iterable,splitter):
    return [list(g) for k,g in itertools.groupby(iterable,lambda x:splitter and splitter in x) if not k]


def process_logfile(logfile, bindir, end_marker, csr_name):

    with open(logfile, 'r') as logf:
        loglines = logf.readlines()

    for i, lines in enumerate(isplit(loglines, end_marker)):
        binfile = os.path.join(bindir + '_%d_bin' % i, csr_name + '.bin')
        os.makedirs(os.path.dirname(binfile), exist_ok=True)
        with open(binfile, 'wb') as binf:
            for l in lines:
                if end_marker and end_marker in l:
                    return
                if l.startswith('csr'):
                    splits = [x.strip() for x in l.split(':')]

                    op = splits[2]
                
                    if op != 'write byte':
                        continue
                    
                    name = splits[4].split()[0]
                    if csr_name not in name:
                        continue

                    addr = splits[4].split()[2]
                    data = splits[-1]

                    addr = int(addr,0)
                    data = int(data,0)
                    #binf.write('0x%08x 0x%08x\n' % (addr, data))
                    binf.write(struct.pack('II', addr, data))


def main():
    nic_dir = os.path.dirname(sys.argv[0]) + '/../'
    nic_dir = os.path.abspath(nic_dir)
    parser = argparse.ArgumentParser()
    parser.add_argument("-l", "--logfile", help="Log file to parse")
    parser.add_argument("-b", "--bindir", help="Bin directory prefix", 
            default = os.path.join(nic_dir, 'conf', 'init'))
    parser.add_argument("-m", "--marker", help="Marker string", default = None)
    parser.add_argument("-c", "--csr", help="CSR name", default = 'pbc')

    args = parser.parse_args()

    process_logfile(args.logfile, args.bindir, args.marker, args.csr)


if __name__ == "__main__":
    main()
