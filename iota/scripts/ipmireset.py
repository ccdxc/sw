#! /usr/bin/python3
import argparse
import sys
import os
import pdb

topdir = os.path.dirname(sys.argv[0]) + '/../../'
topdir = os.path.abspath(topdir)
sys.path.insert(0, topdir)
import iota.harness.infra.utils.parser as parser

parser = argparse.ArgumentParser(description='IPMI Reset Script')
parser.add_argument('--testbed', dest='testbed_json', help='Testbed JSON file')
GlobalOptions = parser.parse_args()

def IpmiReset(ip):
    os.system("ipmitool -I lanplus -H %s -U admin -P N0isystem$ power cycle" % ip)
    return

tbspec = parser.JsonParse(GlobalOptions.testbed_json)
ips = []
for instance in tbspec.Instances:
    IpmiReset(instance.NodeCimcIP)
