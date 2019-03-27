#!/usr/bin/python

import sys 
import argparse
import pprint
import os

num_rules = 0
policy_output = ''

if len(sys.argv) > 2:
    num_rules = int(sys.argv[1])
    policy_output = sys.argv[2]
else:
    print("ipsec_policy.py <num_rules> <output_file.json>")
    sys.exit(0)

print("Num Rules: %d" % num_rules)  
filep_policy = open(policy_output, "w")
filep_policy.write("[\n")
filep_policy.write("  {\n")
filep_policy.write("    \"kind\": \"IPSecPolicy\",\n")
filep_policy.write("    \"meta\": {\n")
filep_policy.write("      \"name\": \"ipsec-encrypt-rules\",\n")
filep_policy.write("      \"tenant\": \"default\",\n")
filep_policy.write("      \"namespace\": \"default\",\n")
filep_policy.write("      \"creation-time\": \"1970-01-01T00:00:00Z\",\n")
filep_policy.write("      \"mod-time\": \"1970-01-01T00:00:00Z\"\n")
filep_policy.write("    },\n")
filep_policy.write("    \"spec\": {\n")
filep_policy.write("      \"vrf-name\": \"scale-vrf-0\",\n")
filep_policy.write("      \"attach-tenant\": true,\n")
filep_policy.write("      \"rules\": [\n")
for i in range(num_rules):
    filep_policy.write("        {\n")
    filep_policy.write("          \"source\": {\n")
    filep_policy.write("            \"addresses\": [\n")
    filep_policy.write("              \"10.5.0.0 - 10.5.255.255\"\n")
    filep_policy.write("             ],\n")
    filep_policy.write("            \"app-configs\": [\n")
    filep_policy.write("              {\n")
    filep_policy.write("                \"protocol\": \"tcp\",\n")
    filep_policy.write("                \"port\":" + " \"" + "%d-%d"%((40000+i*10)+1,(40000+(i+1)*10)) + "\"\n") 
    filep_policy.write("              },\n")
    filep_policy.write("              {\n")
    filep_policy.write("                \"protocol\": \"udp\",\n")
    filep_policy.write("                \"port\":" + " \"" + "%d-%d"%((40000+i*10)+1,(40000+(i+1)*10)) + "\"\n") 
    filep_policy.write("              }\n")
    filep_policy.write("           ]\n")
    filep_policy.write("         },\n")
    filep_policy.write("         \"sa-name\": \"ipsec-encrypt-sa" + "%d"%(i+1)+ "\",\n")
    filep_policy.write("         \"sa-type\": \"ENCRYPT\"\n")
    filep_policy.write("        }")
    if (i != num_rules-1):
        filep_policy.write(",") 
    filep_policy.write("\n")
filep_policy.write("      ]\n")
filep_policy.write("    },\n")
filep_policy.write("    \"status\":  {}\n")
filep_policy.write("  },\n")


filep_policy.write("  {\n")
filep_policy.write("    \"kind\": \"IPSecPolicy\",\n")
filep_policy.write("    \"meta\": {\n")
filep_policy.write("      \"name\": \"ipsec-decrypt-rules\",\n")
filep_policy.write("      \"tenant\": \"default\",\n")
filep_policy.write("      \"namespace\": \"default\",\n")
filep_policy.write("      \"creation-time\": \"1970-01-01T00:00:00Z\",\n")
filep_policy.write("      \"mod-time\": \"1970-01-01T00:00:00Z\"\n")
filep_policy.write("    },\n")
filep_policy.write("    \"spec\": {\n")
filep_policy.write("      \"vrf-name\": \"scale-vrf-2\",\n")
filep_policy.write("      \"attach-tenant\": true,\n")
filep_policy.write("      \"rules\": [\n")
for i in range(num_rules):
    filep_policy.write("        {\n")
    filep_policy.write("          \"source\": {\n")
    filep_policy.write("            \"addresses\": [\n")
    filep_policy.write("              \"10.5.0.0 - 10.5.255.255\"\n")
    filep_policy.write("             ]\n")
    filep_policy.write("            },\n")
    filep_policy.write("          \"destination\": {\n")
    filep_policy.write("            \"addresses\": [\n")
    filep_policy.write("              \"10.5.0.0 - 10.5.255.255\"\n")
    filep_policy.write("             ]\n")
    filep_policy.write("            },\n")
    filep_policy.write("         \"sa-name\": \"ipsec-decrypt-sa" + "%d"%(i+1)+ "\",\n")
    filep_policy.write("         \"sa-type\": \"DECRYPT\",\n")
    filep_policy.write("         \"spi\":" + " %d\n"%(i+1)) 
    filep_policy.write("        }")
    if (i != num_rules-1):
        filep_policy.write(",") 
    filep_policy.write("\n")

filep_policy.write("      ]\n")
filep_policy.write("    },\n")
filep_policy.write("    \"status\":  {}\n")
filep_policy.write("  }\n")

filep_policy.write("]\n")
    
