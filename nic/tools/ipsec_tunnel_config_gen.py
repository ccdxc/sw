#!/usr/bin/python

import sys 
import argparse
import pprint
import os

num_tunnels = 0
config_enc_output = ''
config_dec_output = ''

if len(sys.argv) > 3:
    num_tunnels = int(sys.argv[1])
    config_enc_output = sys.argv[2]
    config_dec_output = sys.argv[3]
else:
    print("ipsec_config.py <num_tunnels> <output_file.json>")
    sys.exit(0)

print("Num Tunnels: %d" % num_tunnels)  
filep_enc = open(config_enc_output, "w")
filep_enc.write("[\n")
for i in range(num_tunnels):
    filep_enc.write("  {\n")
    filep_enc.write("    \"kind\": \"IPSecSAEncrypt\",\n")
    filep_enc.write("    \"meta\": {\n")
    filep_enc.write("      \"name\": \"ipsec-encrypt-sa%d\",\n"%(i+1))
    filep_enc.write("      \"tenant\": \"default\",\n")
    filep_enc.write("      \"namespace\": \"default\",\n")
    filep_enc.write("      \"creation-time\": \"1970-01-01T00:00:00Z\",\n")
    filep_enc.write("      \"mod-time\": \"1970-01-01T00:00:00Z\"\n")
    filep_enc.write("    },\n")
    filep_enc.write("    \"spec\": {\n")
    filep_enc.write("      \"vrf-name\": \"scale-vrf-0\",\n")
    filep_enc.write("      \"attach-tenant\": true,\n")
    filep_enc.write("      \"protocol\": \"ESP\",\n")
    filep_enc.write("      \"authentication-algorithm\": \"AES_GCM\",\n")
    filep_enc.write("      \"authentication-key\": \"AAAAAAAAAAAAAAAAAAAAAAAAAAAA" + "%04d"%(i+1) + "\",\n")
    filep_enc.write("      \"encryption-algorithm\": \"AES_GCM_256\",\n")
    filep_enc.write("      \"encryption-key\": \"AAAAAAAAAAAAAAAAAAAAAAAAAAAA" + "%04d"%(i+1) + "\",\n")
    filep_enc.write("      \"local-gateway-ip\": \"10.7.6.4\",\n")
    filep_enc.write("      \"remote-gateway-ip\": \"10.7.6.3\",\n")
    filep_enc.write("      \"spi\": " + "%d"%(i+1) + ",\n")
    filep_enc.write("      \"tep-vrf\": \"scale-vrf-2\"\n")
    filep_enc.write("    },\n")
    filep_enc.write("    \"status\":  {}\n")
    filep_enc.write("  }")
    if i != num_tunnels-1:
        filep_enc.write(",")
    filep_enc.write("\n")
filep_enc.write("]\n")
    
filep_dec = open(config_dec_output, "w")
filep_dec.write("[\n")
for i in range(num_tunnels):
    filep_dec.write("  {\n")
    filep_dec.write("    \"kind\": \"IPSecSADecrypt\",\n")
    filep_dec.write("    \"meta\": {\n")
    filep_dec.write("      \"name\": \"ipsec-decrypt-sa%d\",\n"%(i+1))
    filep_dec.write("      \"tenant\": \"default\",\n")
    filep_dec.write("      \"namespace\": \"default\",\n")
    filep_dec.write("      \"creation-time\": \"1970-01-01T00:00:00Z\",\n")
    filep_dec.write("      \"mod-time\": \"1970-01-01T00:00:00Z\"\n")
    filep_dec.write("    },\n")
    filep_dec.write("    \"spec\": {\n")
    filep_dec.write("      \"vrf-name\": \"scale-vrf-2\",\n")
    filep_dec.write("      \"attach-tenant\": true,\n")
    filep_dec.write("      \"protocol\": \"ESP\",\n")
    filep_dec.write("      \"authentication-algorithm\": \"AES_GCM\",\n")
    filep_dec.write("      \"authentication-key\": \"AAAAAAAAAAAAAAAAAAAAAAAAAAAA" + "%04d"%(i+1) + "\",\n")
    filep_dec.write("      \"decryption-algorithm\": \"AES_GCM_256\",\n")
    filep_dec.write("      \"decryption-key\": \"AAAAAAAAAAAAAAAAAAAAAAAAAAAA" + "%04d"%(i+1) + "\",\n")
    filep_dec.write("      \"local-gateway-ip\": \"10.7.6.4\",\n")
    filep_dec.write("      \"remote-gateway-ip\": \"10.7.6.3\",\n")
    filep_dec.write("      \"spi\": " + "%d"%(i+1) + ",\n")
    filep_dec.write("      \"tep-vrf\": \"scale-vrf-0\"\n")
    filep_dec.write("    },\n")
    filep_dec.write("    \"status\":  {}\n")
    filep_dec.write("  }")
    if i != num_tunnels-1:
        filep_dec.write(",")
    filep_dec.write("\n")
filep_dec.write("]\n")
