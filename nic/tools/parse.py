#!/usr/bin/python

import os
import sys
import argparse
import re
import json
import pprint

pp = pprint.PrettyPrinter(indent=4)

# Globals
if len(sys.argv) > 1:
   nic_dir = os.path.dirname(sys.argv[1])
else:
   nic_dir = os.getcwd()

print "WS_TOP = ", nic_dir
if nic_dir is None:
    print "WS_TOP is not set!"
    sys.exit(1)

skip_config = 0

model_log = nic_dir + "/model.log"
model1_log = nic_dir + "/model1.log"
model_csv = nic_dir+"/model.log.csv"
model_json = nic_dir+"/model.json"
inscount_log = nic_dir + "/inscount.log"
inscount_sort_log = nic_dir + "/inscount_sort.log"
sym_log = nic_dir + "/gen/capri_loader.conf"
sym_log2 = nic_dir + "/capri_loader.conf"
sym_log3 = nic_dir + "/conf/gen/mpu_prog_info.json"
dol_log = nic_dir + "/dol.log"
print "- Model Log file: " + model_log
print "- Model Log Output file: " + model1_log
print "- SYM Log file: " + sym_log
print "- DOL Log file: " + dol_log
symbols = { 'address': 'name' }

def parse_loader_conf(loader_conf):
    with open(loader_conf, 'r') as f:
        data = json.load(f)
        for p in data['programs']:
            pgm_or_label = p['name']
            start_pc = p['base_addr']
            start_pc_hex = p['base_addr_hex']
            #print str(pgm_or_label), str(start_pc_hex)
            yield str(pgm_or_label), str(start_pc_hex)
            for s in p['symbols']:
                pgm_or_label = s['name']
                if 'Lfunc' == s['name'][:5]:
                   #print "skipped"
                   continue
                s_pc = int(start_pc) + int(s['addr'])
                #print str(pgm_or_label) + '.LAB', hex(s_pc)
                yield str(pgm_or_label) + '.LAB', hex(s_pc)

# build_symbols
def build_symbols():
    print "* Building symbols....."
    if os.path.isfile(sym_log3):
       for pgm_or_label, start_pc in parse_loader_conf(sym_log3):
             #print pgm_or_label
             symbols[start_pc] = pgm_or_label[0:-4]
       #pp.pprint(symbols)
       return
    else:
        print "Cannot find symbol file"
        sys.exit()

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
    tc_start_linenum = 0

    if skip_config:
        for tc_start_linenum, line in enumerate(modelfile):
            if 'Starting Testcase TC' in line:
                print "\'Found Starting Testcase TC\' at linenum: ", tc_start_linenum
                break
            else:
                continue

    for linenum, line in enumerate(modelfile, tc_start_linenum - 1):
        if re.match("(.*) Setting PC to 0x([0-9a-fA-F]+)(.*)", line, re.I):
            fields = re.split(r'(.*) Setting PC to 0x([0-9a-fA-F]+)(.*)', line)
            key = '0x'+fields[2].lower()
            if not key in symbols:
               print "key not found" + key
               print linenum, '0x'+fields[2]
               model1file.write(line)
               continue
            program = symbols[key]
            programline = linenum
            print linenum, '0x'+fields[2], program
            line = line.replace('0x'+fields[2], program)
        elif re.match("^\[(.*)\]: ([0-9a-fA-F]+): ([0-9a-fA-F]+)\s+b\w+\s+.+, 0x([0-9a-fA-F]+)", line, re.I):
#this part added to translate the branch pointers in code
            
	    fields1 = re.split(r'\[(.*)\]: ([0-9a-fA-F]+): ([0-9a-fA-F]+)\s+b\w+\s+.+, 0x([0-9a-fA-F]+)', line)
            key1 = '0x'+fields1[2].lower()
            key2 = '0x'+fields1[4].lower()
         #   print "branch found ", line , "key1 ", key1, " key2 ", key2
            if key1 in symbols:
                line = line.replace(fields1[2], symbols[key1])
            if key2 in symbols:
                line = line.replace(fields1[4], fields1[4]+"("+symbols[key2]+")")
         #   print linenum, 'Jump to '+fields[2], program
            inscountfile.write("%04d %s %d\n" % (int(fields1[1])+1, program, programline))
        elif re.match("^\[(.*)\]: ([0-9a-fA-F]+):(.*)", line, re.I):
#this part added to mark what jumps were taken
            fields1 = re.split(r'\[(.*)\]: ([0-9a-fA-F]+):(.*)', line)
            key = '0x'+fields1[2].lower()
            if key in symbols:
                line = line.replace(fields1[2], symbols[key])
        #    print linenum, 'Jump to '+fields[2], program
            inscountfile.write("%03d %s %d\n" % (int(fields1[1])+1, program, programline))
        elif re.match(".* PC_ADDR=0x(.*) INST=0x", line, re.I):
            fields = re.split(r'.* PC_ADDR=0x(.*) INST=0x', line)
            key = '0x'+fields[1].lower()
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
            key = '0x10'+fields[6].lower()
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

# csv parse

levelIregex=[\
 "^INFO \:\: WA-MODEL\: RING DOORBELL"\
,"^INFO \:\: PBC-MODEL\: RECEIVE PACKET ON PORT"\
,"^INFO \:\: PBC-MODEL\: SENDING PACKET ON PORT"\
];

levelIIregex= [\
 ".*LAUNCH TABLE ID"\
,".*Setting PC to"\
,"^INFO \:\: PBC-MODEL\: dump packet \:"\
,".*Sending request to MPU\:"\
];

levelIIIregex= [\
# "^MSG \:\: STAGE"\
"^MSG \:\: stg \: flit\: "\
,"^INFO \:\:\s*\d"\
,"^DBG_DECODE \>\>\>         FULL_KEY\:\d\:"\
,"^INFO \:\: PICT\: "\
,"^\[\s+\d+\]\:"\
,".*mpu_req_itf_t"\
];

levelIIIIregex= [\
"^DBG_DECODE \>\>\>                BYTE\:"\
,"^\# "\
];

# main()
def main():
    build_symbols()
    parse_logs()
    
if __name__ == "__main__":
    main()
