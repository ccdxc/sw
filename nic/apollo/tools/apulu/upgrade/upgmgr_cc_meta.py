#!/usr/bin/python

# {C} Copyright 2020 Pensando Systems Inc. All rights reserved\n

import sys
import json
import os

# compare manifest from the upgrade image and current
# running image

if len(sys.argv) != 4:
    print("Usage : <command> <running_meta.json> <running_img> <new_meta.json>")
    exit -1

running_meta = sys.argv[1]
running_img  = sys.argv[2]
new_meta     = sys.argv[3]

fields_cc    =  ["nicmgr_compat_version", "kernel_compat_version", "pcie_compat_version"]
fields_cc    += ["dev_conf_compat_version"]
fields_build =  ["build_date", "base_version", "software_version"]

upgrade_to_same_check_file = '/data/upgrade_to_same_firmware_allowed'

def is_upgrade_image_same(rdata, ndata):
    match = 0
    fields = fields_build + fields_cc

    for e in fields:
        print("Field %s run-version %s new-version %s" \
              %(e, rdata[running_img]['system_image'][e], ndata[e]))
        if rdata[running_img]['system_image'][e] == ndata[e]:
            match = match + 1

    if match == len(fields):
        print("Compat check, trying to upgrade to same")
        return True

    return False

def is_upgrade_to_same_allowed():
    if not os.path.isfile(upgrade_to_same_check_file):
        print("Compat check failed, upgrading to same image not allowed")
        return False
    return True

def is_upgrade_ok(rdata, ndata):
    match = 0

    for e in fields_cc:
        if rdata[running_img]['system_image'][e] == ndata[e]:
            match = match + 1

    if match != len(fields_cc):
        print("Compat check failed, version mismatch in %s" %(e))
        return False

    return True

def main():
    try:
        with open(running_meta, 'r') as f:
            rdata = json.load(f)

        with open(new_meta, 'r') as f:
            ndata = json.load(f)

        if is_upgrade_image_same(rdata, ndata):
            if is_upgrade_to_same_allowed():
                print("Compat check successful")
                return 0
        elif is_upgrade_ok(rdata, ndata):
            print("Compat check successful")
            return 0

        print("Compat check failed")
        return -1
    except Exception as e:
        print("Compat check failed, due to exception")
        print(e)
        return -1

if __name__ == '__main__':
    exit(main())
