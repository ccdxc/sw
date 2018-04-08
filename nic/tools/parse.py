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
inscount_log = nic_dir + "/inscount.log"
inscount_sort_log = nic_dir + "/inscount_sort.log"
sym_log = nic_dir + "/gen/capri_loader.conf"
sym_log2 = nic_dir + "/capri_loader.conf"
dol_log = nic_dir + "/dol.log"
print "- Model Log file: " + model_log
print "- Model Log Output file: " + model1_log
print "- SYM Log file: " + sym_log
print "- DOL Log file: " + dol_log
symbols = { 'address': 'name' }

# build_symbols
def build_symbols():
    print "* Building symbols....."
    if os.path.isfile(sym_log):
        sym_file  = open(sym_log, "r")
    elif os.path.isfile(sym_log2):
        sym_file  = open(sym_log2, "r")
    else:
        print "Cannot find symbol file"
        sys.exit()
    for line in sym_file:
        fields = line.strip().split(",")
        if len(fields) < 2:
            continue
        symbols['0x'+fields[1].upper()] =  fields[0][0:-4]
    return

# parse_logs and create a new file with symbols resolved
def parse_logs():
    print "* Starting Parse....."
    modelfile  = open(model_log, "r")
    os.remove(model1_log) if os.path.exists(model1_log) else None
    model1file  = open(model1_log, "a+")
    os.remove(inscount_log) if os.path.exists(inscount_log) else None
    inscountfile  = open(inscount_log, "a+")
    os.remove(inscount_sort_log) if os.path.exists(inscount_sort_log) else None
    inscount_sort_file  = open(inscount_sort_log, "a+")
    program = ""
    programline = 0
    for linenum, line in enumerate(modelfile):
        if re.match("(.*) Setting PC to 0x([0-9a-fA-F]+)(.*)", line, re.I):
            fields = re.split(r'(.*) Setting PC to 0x([0-9a-fA-F]+)(.*)', line)
            key = '0x'+fields[2].upper()
            if not key in symbols:
                print linenum, '0x'+fields[2]
                model1file.write(line)
                continue
            program = symbols[key]
            programline = linenum
            print linenum, '0x'+fields[2], program
            line = line.replace('0x'+fields[2], program)
        elif re.match(".* PC_ADDR=0x(.*) INST=0x", line, re.I):
            fields = re.split(r'.* PC_ADDR=0x(.*) INST=0x', line)
            key = '0x'+fields[1].upper()
            if not key in symbols:
                model1file.write(line)
                continue
            program = symbols[key]
            line = line.replace('0x'+fields[1], program)
        elif re.match("^\[(.*)\]: (.*)\.e(.*)", line, re.I):
            fields1 = re.split(r'\[(.*)\]: (.*)\.e(.*)', line)
            inscountfile.write("%03d %s %d\n" % (int(fields1[1])+1, program, programline))
            #print(fields1[1], program, programline, linenum)
        elif 'Starting Testcase TC' in line:
            print '\n' + line.strip()
        elif 'Ending Testcase TC' in line:
            print '\n' + line.strip()
        elif re.match("(.*)[sp]g(.*)mpu(.*)load(.*)pkt_id ([0-9]*) entry pc byte addr=0x([0-9a-fA-F]+)(.*)", line, re.I):
            fields = re.split(r'(.*)[sp]g(.*)mpu(.*)load(.*)pkt_id ([0-9]*) entry pc byte addr=0x([0-9a-fA-F]+)(.*)', line)
            key = '0x10'+fields[6].upper()
            if not key in symbols:
                print 'RTL ', linenum, 'pkt_id ', fields[5], ' 0x'+fields[6]
                model1file.write(line)
                continue
            program = symbols[key]
            programline = linenum
            print 'RTL ', linenum, 'pkt_id ', fields[5], '0x'+fields[6], program
            line = line.replace('0x'+fields[6], program)
        model1file.write(line)
    modelfile.close()
    model1file.close()
    inscountfile.close()

    # prepare sorted inscount file
    with open(inscount_log) as f:
        sorted_file = sorted(f, reverse=True)
    inscount_sort_file.writelines(sorted_file)
    inscount_sort_file.close()
    return

# main()
def main():
    build_symbols()
    parse_logs()

if __name__ == "__main__":
    main()
