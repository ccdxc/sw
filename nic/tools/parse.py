#!/usr/bin/python

import os
import sys
import argparse
import re

# Globals
if len(sys.argv) > 1:
   nic_dir = os.path.dirname(sys.argv[1])
else:
   nic_dir = os.getcwd()

print "WS_TOP = ", nic_dir
if nic_dir is None:
    print "WS_TOP is not set!"
    sys.exit(1)

model_log = nic_dir + "/model.log"
model1_log = nic_dir + "/model1.log"
hal_log = nic_dir + "/hal.log"
dol_log = nic_dir + "/dol.log"
print "- Model Log file: " + model_log
print "- Model Log Output file: " + model1_log
print "- HAL Log file: " + hal_log
print "- DOL Log file: " + dol_log
symbols = { 'address': 'name' }

# build_symbols
def build_symbols():
    print "* Building symbols....."
    hal_file  = open(hal_log, "r")
    for linenum, line in enumerate(hal_file):
        if re.match("(.*) Successfully resolved program: name (.*)", line, re.I):
            fields = re.split(r'(.*) Successfully resolved program: name (.*)\.bin, base address 0x(.*), size(.*)', line)
            symbols['0x'+fields[3].upper()] =  fields[2]
        elif re.match("(.*) label: name (.*)", line, re.I):
            fields = re.split(r'(.*) label: name (.*) addr 0x(.*)', line)
            symbols['0x'+fields[3].upper()] =  fields[2]
    return

# parse_logs and create a new file with symbols resolved
def parse_logs():
    print "* Starting Parse....."
    modelfile  = open(model_log, "r")
    os.remove(model1_log) if os.path.exists(model1_log) else None
    model1file  = open(model1_log, "a+")
    for linenum, line in enumerate(modelfile):
        if re.match("(.*) Setting PC to 0x(.*)\[ (.*)", line, re.I):
            fields = re.split(r'(.*) Setting PC to 0x(.*)\[ (.*)', line)
            print linenum, '0x'+fields[2], symbols['0x'+fields[2].upper()]
            line = line.replace('0x'+fields[2], symbols['0x'+fields[2].upper()])
        model1file.write(line)
    modelfile.close()
    model1file.close()
    return

# main()
def main():
    build_symbols()
    parse_logs()

if __name__ == "__main__":
    main()
