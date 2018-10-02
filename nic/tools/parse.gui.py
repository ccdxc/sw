#!/usr/bin/python

import os
import sys
import argparse
import re
import json

# Globals
customer_color="yellow"
default_color  ="red"
defaultColor = "blue"
customer_regexp="babayaga"
if len(sys.argv) == 2:
   customer_regexp  = sys.argv[1];
elif len(sys.argv) == 3:
   customer_regexp  = sys.argv[1];
   customer_color = sys.argv[2];    
nic_dir = os.getcwd()

print "\n\n   parse.gui.py <regex> <color>"
print "   <regex> - patter to look in the model.log file"
print "   <color=red> - color to highlight founded pattern \n\n"
print "This tool generates JSON file from model.log and capri_loader.conf. That JSON file"
print "will be used to generate HTML D3 based graph for easy navigation"
print "all question and requestes please forward to rami@pensando.io\n\n"



print "WS_TOP = ", nic_dir
if nic_dir is None:
    print "WS_TOP is not set!"
    sys.exit(1)


model_log = nic_dir + "/model.log"
model1_log = nic_dir + "/model1.log"
model_csv = nic_dir+"/model.log.csv"
model_json = nic_dir+"/model.json"
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

redColorWords= [\
 "^\[\s+\d+\]\:.*violation.*:"\
,"^\[\s+\d+\]\:.*abort.*:"\
,"^\[\s+\d+\]\:.*error.*:"\
,"^\[\s+\d+\]\:.*err.*:"\
,"^\[\s+\d+\]\:.*fatal.*:"\
];



defaultRegexColor = "(" + ")|(".join(redColorWords) + ")"
inputRegexColor =customer_regexp
print "+ RegExp that will be highlighted:\n " 
print "+ Color : Regex\n "
print "+ "+ default_color + " : "+ defaultRegexColor  
print "+ "+ customer_color + " : "+ inputRegexColor
print "\n+ This tool generates JSON file from model.log and capri_loader.conf. That JSON file"
print "   will be used to generate HTML D3 based graph for easy navigation"
print "   all question and requestes please forward to rami@pensando.io"


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
        elif re.match("^\[(.*)\]: ([0-9a-fA-F]+): ([0-9a-fA-F]+)\s+b\w+\s+.+, 0x([0-9a-fA-F]+)", line, re.I):
#this part added to translate the branch pointers in code
            
	    fields1 = re.split(r'\[(.*)\]: ([0-9a-fA-F]+): ([0-9a-fA-F]+)\s+b\w+\s+.+, 0x([0-9a-fA-F]+)', line)
            key1 = '0x'+fields1[2].upper()
            key2 = '0x'+fields1[4].upper()
         #   print "branch found ", line , "key1 ", key1, " key2 ", key2
            if key1 in symbols:
                line = line.replace(fields1[2], symbols[key1])
            if key2 in symbols:
                line = line.replace(fields1[4], fields1[4]+"("+symbols[key2]+")")
            print linenum, 'Branch to '+fields[2], program
            inscountfile.write("%04d %s %d\n" % (int(fields1[1])+1, program, programline))
        elif re.match("^\[(.*)\]: ([0-9a-fA-F]+):(.*)", line, re.I):
#this part added to mark what jumps were taken
            fields1 = re.split(r'\[(.*)\]: ([0-9a-fA-F]+):(.*)', line)
            key = '0x'+fields1[2].upper()
            if key in symbols:
                line = line.replace(fields1[2], symbols[key])
            print linenum, 'Jump to '+fields[2], program
            inscountfile.write("%03d %s %d\n" % (int(fields1[1])+1, program, programline))
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


def parse_csv():
    print "* Starting CSV Parse....."
    modelfile  = open(model1_log, "r")
    os.remove(model_csv) if os.path.exists(model_csv) else None
    modelcsv  = open(model_csv, "a+")
    combinedLevelIregex = "(" + ")|(".join(levelIregex) + ")"
    combinedLevelIIregex = "(" + ")|(".join(levelIIregex) + ")"
    combinedLevelIIIregex = "(" + ")|(".join(levelIIIregex) + ")"
    combinedRedColor = "(" + ")|(".join(redColorWords) + ")" 
    line = "Level I, Level II, Level III,,,,,,,,\"DATA";
    modelcsv.write(line)
    firstLevelFound=0;
    for linenum, line in enumerate(modelfile):
        if re.match(combinedLevelIregex, line, re.I):
            firstLevelFound=1
	    line = "\"\n"+line.rstrip() + ",.,.,,,,,,,,\"\n"   
        elif re.match(combinedLevelIIregex, line, re.I): 
            line = "\"\n.,"+line.rstrip() + ",.,,,,,,,,\"\n" 
        elif re.match(combinedLevelIIIregex, line, re.I):
            line = "\"\n.,.,"+line.rstrip() + ",,,,,,,,\"\n"
        if firstLevelFound==1:        
            modelcsv.write(line)
    modelfile.close()
    modelcsv.close()
    # prepare sorted inscount file
    return

# jsoncsv parse
def parse_json_advance():
	print "* Starting JSON Parse....."
	modelfile  = open(model1_log, "r")
	os.remove(model_json) if os.path.exists(model_json) else None
	modeljson  = open(model_json, "a+")
	combinedLevel0regex = "(" + ")|(".join(levelIregex) + ")"
	combinedLevel1regex = "(" + ")|(".join(levelIIregex) + ")"
	combinedLevel2regex = "(" + ")|(".join(levelIIIregex) + ")"
	combinedLevel3regex = "(" + ")|(".join(levelIIIIregex) + ")"
	heads=[None]*10;
	root={};
	root["name"]="non"
	root["children"]=[];
        line =json.dumps(root, ensure_ascii=False)
	currentLevel=0;
	newlineIndex=0;
	for linenum, line in enumerate(modelfile):
		colorMatch=""
		debugLine="."
		item={};
		if re.match(combinedLevel0regex, line, re.I):
			newlineIndex=0;
                        debugLine="+"
                elif re.match(combinedLevel1regex, line, re.I):
			debugLine="+"
                        newlineIndex=1;
                elif re.match(combinedLevel2regex, line, re.I):
                        debugLine="++"
                        newlineIndex=2;
                elif re.match(combinedLevel3regex, line, re.I):
                        debugLine="+++"
                        newlineIndex=3;
		else:
			newlineIndex=-1; #nothing
			continue;
		if re.match(defaultRegexColor, line, re.I):
			colorMatch=default_color;
                if re.match(inputRegexColor, line, re.I):
                        colorMatch=customer_color;

		item["name"]=line
		item["line"]=newlineIndex
                item["size"]=0.5
		item["children"]=[]
		item["colorCode"]=defaultColor #ADD SUPPORT FOR CUSTMER CODES
		if (newlineIndex!=currentLevel):
			for i in range (0,newlineIndex):
				if (heads[i]==None):
					titem={};
					titem["name"]="EMPTY LEVEL"+str(i);
                                        titem["line"]="EMPTY"+str(i);
                                        titem["size"]=0.5;
                                        titem["colorCode"]=defaultColor;
                                        titem["children"]=[];
					heads[i]=titem;
					if (i!=0):
						heads[i-1]["children"].append(titem);
					else:
						root["children"].append(titem);
						
					heads[i]=titem
		heads[newlineIndex]=item;
                if (newlineIndex!=0) :
			heads[newlineIndex-1]["children"].append(item);
		elif (newlineIndex==0) :
			root["children"].append(item);
		currentLevel=newlineIndex;
		if (colorMatch!=""):
                        item["colorCode"]=colorMatch
			for i in range (0,newlineIndex):
				if (heads[i]!=None):
					heads[i]["colorCode"]=colorMatch
                                        heads[i]["size"]+=0.5

		
#for
#	print root  
	line =json.dumps(root, ensure_ascii=False)
	modeljson.write(line)
	modeljson.close()
	modelfile.close()
	# prepare sorted inscount file
	print "+ JSON file was generated successfully. Please enable HTTP server through the following command:"
	print "      python -m SimpleHTTPServer XXYY &"
	print "\n\n PLEASE USE CHROME to browse the resutls, as other browser does not support correct color scheme"
	import socket
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.connect(("www.google.com", 80))
	ip = s.getsockname()[0]
	s.close()
	print "   http://"+ip+":XXYY/"
	return

def parse_json():
    print "* Starting JSON Parse....."
    modelfile  = open(model1_log, "r")
    os.remove(model_json) if os.path.exists(model_json) else None
    modeljson  = open(model_json, "a+")
    combinedLevelIregex = "(" + ")|(".join(levelIregex) + ")"
    combinedLevelIIregex = "(" + ")|(".join(levelIIregex) + ")"
    combinedLevelIIIregex = "(" + ")|(".join(levelIIIregex) + ")"
    combinedLevelIIIIregex = "(" + ")|(".join(levelIIIIregex) + ")"
    combinedRedColor = "(" + ")|(".join(redColorWords) + ")"
    combinedRegExp = "(" + ")|(".join(customer_regexp) + ")"
    combinedRedColor = combinedRedColor+"|"+combinedRegExp
    firstLevelFound=0;
    lev0Color = 0
    lev1Color = 0
    lev2Color = 0
    lev3Color = 0
    lev4Color = 0
    rawLevel0={}
    rawLevel0["name"]="All stages"
    rawLevel0["line"]="0"
    rawLevel0["size"]="3"
    rawLevel0["children"]=[]
#    print combinedRedColor
#    print combinedRegExp
    rawLevelI={}
    rawLevelII={}
    rawLevelIII={}
    rawLevelIIII={}
    for linenum, line in enumerate(modelfile):
        if re.match(combinedLevelIregex, line, re.I):
            if rawLevelI!={} :
                if lev1Color==1:
                    rawLevelI["colorCode"]=customer_color;
                rawLevel0["children"].append(rawLevelI);
                rawLevelII={}
                rawLevelIII={}
                rawLevelI={}
                lev1Color = 0; #reset the color
            rawLevelI["name"]=line
            rawLevelI["line"]=linenum
            rawLevelI["size"]=2 
        elif re.match(combinedLevelIIregex, line, re.I):
            if rawLevelII!={} :
                if "children" not in rawLevelI :
                    rawLevelI["children"]=[] 
		if lev2Color==1:
		    rawLevelI["colorCode"]=customer_color;
		    rawLevelII["colorCode"]=customer_color;
                lev1Color=lev2Color|lev1Color;
 		rawLevelI["children"].append(rawLevelII);
                print "closing II", rawLevelII["name"]
		rawLevelII={}
                lev2Color=0
		rawLevelIII={}
            rawLevelII["name"]=line
            rawLevelII["size"]=1
	    if re.match(combinedRedColor, line, re.I):
		lev2Color=1
            	rawLevelII["colorCode"]=customer_color
            rawLevelII["line"]=linenum
            print "++Level 2 " ,rawLevelII["name"];
        elif re.match(combinedLevelIIIregex, line, re.I):
            if rawLevelIII!={} :
                if rawLevelII!={} :
                    if lev3Color==1:
                       rawLevelIII["colorCode"]=customer_color;
		       rawLevelII["colorCode"]=customer_color;
                    lev2Color=lev3Color|lev2Color;
                    if "children" not in rawLevelII :
                        rawLevelII["children"]=[]
                    rawLevelII["children"].append(rawLevelIII);
                else :
                    if "children" not in rawLevelI :
                        rawLevelI["children"]=[]
                    rawLevelI["children"].append(rawLevelIII);
		lev3Color=0
	        print "closing III", rawLevelIII["name"]
		rawLevelIII={}
            rawLevelIII["name"]=line
            rawLevelIII["size"]=1
            if re.match(combinedRedColor, line, re.I):
                lev3Color=1
                rawLevelIII["colorCode"]=customer_color
            rawLevelIII["line"]=linenum
            print "+++Level 3 " ,rawLevelIII
        elif re.match(combinedLevelIIIIregex, line, re.I):
            if rawLevelIIII!={} :
                if rawLevelIII!={} :
                    if lev4Color==1:
                        rawLevelIIII["colorCode"]=customer_color;
			rawLevelIII["colorCode"]=customer_color;
                    lev3Color=lev4Color|lev3Color;
                    if "children" not in rawLevelIII :
                        rawLevelIII["children"]=[]
                    rawLevelIII["children"].append(rawLevelIIII);
                else :
                    if rawLevelI["children"] is None :
                        rawLevelI["children"]=[]
                    rawLevelI["children"].append(rawLevelIIII);
                lev4Color=0
		rawLevelIIII={}
            rawLevelIIII["name"]=line
            rawLevelIIII["line"]=linenum
            rawLevelIIII["size"]=1
            if re.match(combinedRedColor, line, re.I):
                lev4Color=1
                rawLevelIIII["colorCode"]=customer_color
    if rawLevelIIII!={} :
        if rawLevelIII!={} :
            rawLevelIIII["colorCode"]= customer_color if lev4Color else "";
	    lev3Color=lev4Color|lev3Color
            if "children" not in rawLevelIII :
                rawLevelIII["children"]=[]
            rawLevelIII["children"].append(rawLevelIIII);
    if rawLevelIII!={} :
        if rawLevelII!={} :
            rawLevelIII["colorCode"]=customer_color if lev3Color else "";
	    lev2Color=lev2Color|lev3Color
            if "children" not in rawLevelII :
                rawLevelII["children"]=[]
            rawLevelII["children"].append(rawLevelIII);

    if rawLevelII!={} :
        rawLevelII["colorCode"]=customer_color if lev2Color else "";
	lev1Color=lev1Color|lev2Color
        if "children" not in rawLevelI :
            rawLevelI["children"]=[]
        rawLevelI["children"].append(rawLevelII);
    if rawLevelI!={} :
        rawLevelI["colorCode"]=customer_color if lev1Color else "";
        rawLevel0["children"].append(rawLevelI);
        rawLevelI={}
    line =json.dumps(rawLevel0, ensure_ascii=False)
    modeljson.write(line)
    modeljson.close()
    modelfile.close()

    # prepare sorted inscount file
    print "+ JSON file was generated successfully. Please enable HTTP server through the following command:"
    print "      python -m SimpleHTTPServer XXYY &"
    print "\n\n PLEASE USE CHROME to browse the resutls, as other browser does not support correct color scheme"
    import socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("www.google.com", 80))
    ip = s.getsockname()[0]
    s.close()
    print "   http://"+ip+":XXYY/"
    return

# main()
def main():
    build_symbols()
    parse_logs()
    parse_json_advance() 
#    parse_json()
if __name__ == "__main__":
    main()
