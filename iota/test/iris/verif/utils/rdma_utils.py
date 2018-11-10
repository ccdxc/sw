#! /usr/bin/python3
import re

# method to get the device, given output of show_gid
def GetWorkloadDevice(stdout):
    str=re.sub(' +', ' ',stdout)
    return str.split('\t')[0]

# method to get the GID, given output of show_gid
def GetWorkloadGID(stdout):
    str=re.sub(' +', ' ',stdout)
    return str.split('\t')[2]
