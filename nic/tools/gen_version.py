#!/bin/python3

import os
import re
import subprocess
import json
import argparse

###########################
##### CMDLINE PARSING #####
###########################

parser = argparse.ArgumentParser()

parser.add_argument('--pipeline', dest='pipeline',
                    default='iris',
                    help='asic pipeline')

parser.add_argument('--output-dir', dest='output_dir',
                    default='.',
                    help='output dir to generate version file')

args = parser.parse_args()

output_dir      = args.output_dir
output_filename = output_dir + '/VERSION.json'
version_dict    = {}

#############################
# Generate version for nic
#############################

nic_version    = subprocess.run(['git', 'describe', '--tags', '--dirty', '--always'], stdout=subprocess.PIPE).stdout.decode('utf-8').strip('\n')
nic_build_time = subprocess.run(['date'], stdout=subprocess.PIPE).stdout.decode('utf-8').strip('\n')
nic_sha        = subprocess.run(['git', 'log', '-1', '--pretty=format:\'%H\''], stdout=subprocess.PIPE).stdout.decode('utf-8').strip('\n')

# Parse build meta data file, that is construction when docker build contiainer got started.
def parse_build_meta():
    data = dict();

    data['host_name'] = ""
    data['host_ws'] = ""

    build_meta_file="/usr/build_host_meta_data"
    if os.path.isfile(build_meta_file):
       file = open(build_meta_file, "r")
       lines = file.readlines()

       for line in lines:
           line = line.strip()
           if re.match("HOST_HOSTNAME=", line):
               host_name_fields = re.split("=", line)
               data['host_name'] = host_name_fields[1]
           elif re.match("HOST_WORKSPACE=", line):
               host_workspace_fields = re.split("=", line)
               data['host_ws'] = host_workspace_fields[1]
    else:
       print ("File '%s' not found. "
              " Build host meta data will be missed in VERSION.json file. Continuing..." % (build_meta_file))

    return(data)


meta_data = parse_build_meta()

version_dict["sw"]               = {}
version_dict["sw"]["pipeline"]   = args.pipeline
version_dict["sw"]["version"]    = nic_version
version_dict["sw"]["tag"]        = nic_version
version_dict["sw"]["sha"]        = nic_sha
if (meta_data['host_name'] != ""):
    version_dict["sw"]["build_host"]      = meta_data['host_name']
if (meta_data['host_ws'] != ""):
    version_dict["sw"]["host_workspace"]  = meta_data['host_ws']
version_dict["sw"]["build_time"] = nic_build_time

#########################
# Generate for submodules
#########################

def get_submodule_short_name(submodule_full_name):
    return submodule_full_name.split('/')[-1]

def get_submodule_tag(submodule_tag):
    return submodule_tag.strip('(').strip(')')

version_dict["sw"]["modules"] = {}
submodules = subprocess.run(['git', 'submodule', 'status'], stdout=subprocess.PIPE)

for submodule in submodules.stdout.decode('utf-8').split('\n'):
    # ignore line containing only whitespace or newline
    if len(submodule) == 0:
        continue

    # strip leading whitespace
    submodule = submodule.lstrip()

    submodule_status_arr = submodule.split(' ')

    sha  = submodule_status_arr[0]
    name = get_submodule_short_name(submodule_status_arr[1])
    tag  = get_submodule_tag(submodule_status_arr[2])

    # store in dict to dump to json
    version_dict["sw"]["modules"][name]            = {}
    version_dict["sw"]["modules"][name]["version"] = tag
    version_dict["sw"]["modules"][name]["tag"]     = tag
    version_dict["sw"]["modules"][name]["sha"]     = sha

########################
# Dump info to json file
########################

with open(output_filename, 'w') as outfile:
    # json.dump(version_dict, outfile, indent=4, sort_keys=True)
    json.dump(version_dict, outfile, indent=4)
