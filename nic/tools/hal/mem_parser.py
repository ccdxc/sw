#!/usr/bin/python
# {C} Copyright 2018 Pensando Systems Inc. All rights reserved\n

import json
import sys
import os
import re
import pdb

# Name header
name = "MEM_REGION_"

# Headers and footers for the output file
hdr="""
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved\n
#ifndef __%sHPP__\n#define __%sHPP__
// This is an auto generated file

#include "platform/utils/mcache_pipe.hpp"

#ifdef  __cplusplus
namespace mem {
namespace regions {
#endif

"""%(name, name)

ftr="""
#ifdef  __cplusplus
} // namespace mem
} // namespace regions

#endif //__cplusplus
#endif
"""

cache_pipes = {
    'p4ig': "MEM_REGION_CACHE_PIPE_P4IG",
    'p4eg': "MEM_REGION_CACHE_PIPE_P4EG",
    'p4plus-txdma': "MEM_REGION_CACHE_PIPE_P4PLUS_TXDMA",
    'p4plus-rxdma': "MEM_REGION_CACHE_PIPE_P4PLUS_RXDMA",
    'p4plus-all': "MEM_REGION_CACHE_PIPE_P4PLUS_ALL" }

cc_region_key = 'cc'
regular_region_key = 'regular'

# Dictionary of:
# key: cache_pipe,         value: list of regions for a cache pipe
# key: cc_region_key,      value: list of cache coherent regions
# key: regular_region_key, value: list of remaining regions
cache_regions = {}
MAX_CACHE_REGS_PER_REGION_TYPE = 8

fd = sys.stdout
pipeline = None

cache_size = 6 # in powers of 2

cache_size_roundoff_mask = 0
for i in range(cache_size):
    cache_size_roundoff_mask |= (1 << i)

# region kind stats
static_mem = 0
upgrade_mem = 0
cfgtbl_mem = 0
opertbl_mem = 0
kind_rgns = 0 # kind should be present in all regions
static_off = 0 # break if static regions are not adjascent

def parse_kind(e, off, size):
    global static_mem, upgrade_mem, cfgtbl_mem, opertbl_mem, kind_rgns, static_off
    if 'kind' in e:
        kind_rgns = kind_rgns + 1
        if e['kind'] == "static":
            static_mem += size
        elif e['kind'] == "upgrade":
            upgrade_mem += size
        elif e['kind'] == "opertbl":
            opertbl_mem += size
        elif e['kind'] == "cfgtbl":
            cfgtbl_mem += size
        else:
            return -1
    return 0

# if key is present, append to existing value
# else add the value as a list
def cache_regions_add(key, region_id, region):
    if not key in cache_regions:
        cache_regions[key] = []
    cache_regions[key].append({region_id:region})



def validate_regions(data):
    cc_region_id = 0
    static_region_id = 0
    oper_region_id = 0
    region_id = 0
    # as the number of cache address/size registers for P4 regions are fixed(8),
    # regions of specific cache regions should be specified adjacent.
    #
    # hbm owners should consider this while specifiying the regions.
    #
    # for upgrade safe static/state should be extended to the bottom. so
    # this cannot be mandated. only config tables can be sorted
    for region in data['regions']:
        region_id = region_id + 1
        if cc_region_key in region and region[cc_region_key] == "true":
            if cc_region_id == 0 : cc_region_id = region_id
            if region_id != cc_region_id:
                print "Warning - cc region %s not adjacent" %(region['name'])
            cc_region_id = region_id + 1

        if 'kind' in region and region['kind'] == "static":
            if static_region_id == 0 : static_region_id = region_id
            if region_id != static_region_id:
                print "Warning - static region %s not adjacent" %(region['name'])
            static_region_id = region_id + 1

        if 'kind' in region and region['kind'] == "opertbl":
            if oper_region_id == 0 : oper_region_id = region_id
            if region_id != oper_region_id:
                print "Warning - oper table %s not adjacent" %(region['name'])
            oper_region_id = region_id + 1

        if True:
            # group all cache regions together
            if 'cache' in region:
                cache_pipe = region['cache']
                if cache_pipe in cache_pipes:
                    cache_regions_add(cache_pipe, region_id, region)
                else:
                    print "Invalid cache pipe speciciation : " + cache_pipe
                    return -1
                continue

            # group all cache coherent regions
            if cc_region_key in region and region[cc_region_key] == "true":
                cache_regions_add(cc_region_key, region_id, region)
                continue

            # remaining regions
            cache_regions_add(regular_region_key, region_id, region)

    # list of region types to be generated in that order
    region_types = [cc_region_key, regular_region_key]
    region_types.extend(cache_pipes.keys())

    # calculate the number of cache registers required for each region
    # using region-id to check whether adjascent or not
    num_regs = {}
    for region_type in region_types:
        num_regs[region_type] = 0
        if region_type in cache_regions:
            #print "%s" %(region_type)
            rid = 0
            num_regs[region_type] = 1
            for v in cache_regions[region_type]:
                for region_id in v:
                    #print "id %u name %s" %(region_id, v[region_id]['name'])
                    if rid == 0:rid = region_id
                    if rid != region_id:
                        num_regs[region_type] = num_regs[region_type] + 1
                    rid = region_id + 1
        print "Num regs for region %s is %u" %(region_type, num_regs[region_type])

    # p4plus-all should be added to rxdma & txdma
    num_regs['p4plus-txdma'] = num_regs['p4plus-txdma'] + num_regs['p4plus-all']
    num_regs['p4plus-rxdma'] = num_regs['p4plus-rxdma'] + num_regs['p4plus-all']
    for region_type in region_types:
        if region_type == 'regular':
            continue
        if num_regs[region_type] > MAX_CACHE_REGS_PER_REGION_TYPE:
            print "Num registers exceeded"
            return -1
    return 0


def size_str_to_bytes(size_str):
    if "G" in size_str:
        mf = 1024 * 1024 * 1024
    elif "M" in size_str:
        mf = 1024 * 1024
    elif "K" in size_str:
        mf = 1024
    elif "B" in size_str:
        mf = 1
    else:
        return -1

    value = size_str[:-1]
    if value.isdecimal():
        mv = long(size_str[:-1])
    else:
        return -1

    size = mf * mv

    # roundoff size to nearest cache size
    size = (size + pow(2, cache_size-1)) & (~cache_size_roundoff_mask)
    return size


def parse_inputs():
    global fd
    global pipeline

    if len(sys.argv) != 4:
        print "Usage : mem_parser.py <json-input> <out-file> <pipeline>"
        return -1;
    # Input file name
    fname_in = sys.argv[1]

    # Output file name
    fname_out = sys.argv[2]
    try:
        d = os.path.dirname(fname_out)
        os.makedirs(d)
    except:
        pass

    try:
        fd = open(fname_out, "w+")
    except:
        print "File open failed : " + fname_out
        return -1

    pipeline = sys.argv[3]
    return 0

def check_max_usage(sz):
    if True:
        if pipeline == "iris":
            if sz > (2 << 30): # 2 Gigabyte
                return -1
        elif pipeline == "apollo":
            if sz > (6 << 30): # 6 Gigabyte
                return -1
        elif pipeline == "artemis":
            if sz > (6 << 30): # 6 Gigabyte
                return -1
        elif pipeline == "apulu":
            if sz > (2 << 30): # 2 Gigabyte
                return -1
        else:
            print "Unknown pipeline, not enforcing size check"
    return 0

def convert_size(sz):
    munits = [[30, 'G'], [20, 'M'], [10, 'K'], [0, 'B']]
    v = ""
    for b, u in munits:
        res = sz / (1 << b)
        sz = sz - (res * (1 << b))
        if res > 0:
            v = v + "%d%s " %(res, u)
    return v[:-1]


# parse a single region
# returns the size of the region
def parse_region(e, start_offset):
    print >> fd, ""

    n = e['name']
    mf = long(1)
    mv = long(0)
    mem_type = e['size']

    if e.get('base_region') is not None:
        s = 0
    else:
        s = size_str_to_bytes(mem_type)

    if s == -1:
        print('Invalid size specified for region %s', n)
        return -1

    if e.get('block_size') is None:
        bs = -1
    else:
        bs = size_str_to_bytes(e['block_size'])

    if e.get('block_count') is None:
        bc = 1
    else:
        bc = size_str_to_bytes(e['block_count'])

    # parse kind
    if parse_kind(e, start_offset, s) == -1:
        print('Invalid kind specified for region %s', n)
        return -1

    # Derive the basename for the macros
    nbase = name + (re.sub("[ -]", "_", n)).upper() + "_"

    # Update name and size
    print >> fd, "#define %-60s \"%s\"" %(nbase + "NAME", n)
    print >> fd, "#define %-60s %ld" %(nbase + "SIZE", s)
    print >> fd, "#define %-60s %ld" %(nbase + "BLOCK_SIZE", bs)
    print >> fd, "#define %-60s %ld" %(nbase + "BLOCK_COUNT", bc)

    # Update start offset
    print >> fd, "#define %-60s 0x%lxUL" %(nbase + "START_OFFSET", start_offset);

    # Update cache pipe
    cv = "MEM_REGION_CACHE_PIPE_NONE"
    if 'cache' in e:
        v = e['cache']
        if v in cache_pipes:
            cv = str(cache_pipes[v])
        else:
            print "Invalid cache pipe speciciation : " + v
            return -1
    print >> fd, "#define %-60s %s" %(nbase + "CACHE_PIPE", cv)

    # Update reset
    reset = 0
    if 'reset' in e and str(e['reset']).lower() == "true":
        reset = 1
    print >> fd, "#define %-60s %d" %(nbase + "RESET", reset)

    return s



def parse_file():

    idx = 0

    with open(sys.argv[1], 'r') as f:
        data = json.load(f)

    if True:
        off = 0
        if validate_regions(data) == -1:
            return -1
        for region in data['regions']:
            s = parse_region(region, off)
            if s == -1:
                return s
            off = long(off) + s
            idx += 1

    print >> fd, "\n/////////////////////////////////////////////////////////\n"
    # Error, if table memories exceeds the upgrade reserved
    if upgrade_mem and upgrade_mem < (cfgtbl_mem + opertbl_mem):
        print('Insufficient memory for upgrade, required %ld configured %ld',
                    cfgtbl_mem + opertbl_mem, upgrade_mem)
        return -1
    # Error, if kind is not specified for all regions (either all or none)
    if kind_rgns and kind_rgns != idx:
        print('Kind is not specified for some regions')
        return -1

    # Total regions count
    print >> fd, "#define %-60s %d" %(name + "COUNT" , idx)

    # Max memory used
    print >> fd, "#define %-60s %ld" %("MAX_MEMORY_USED_IN_BYTES" , off)
    print >> fd, "#define %-60s \"%s\"" %("MAX_MEMORY_USED_IN_UNITS" , convert_size(off))

    if check_max_usage(off) != 0:
        print "Max memory usage exceeded for pipeline : " + pipeline
        return -1

    print >> fd, "\n/////////////////////////////////////////////////////////\n"
    # memory splits
    if upgrade_mem:
        print >> fd, "\n#define %-60s %ld" %("CFGTBL_MEMORY_USED_IN_BYTES" , cfgtbl_mem)
        print >> fd, "#define %-60s \"%s\"" %("CFGTBL_MEMORY_USED_IN_UNITS" , convert_size(cfgtbl_mem))
        print >> fd, "#define %-60s %ld" %("OPERTBL_MEMORY_USED_IN_BYTES" , opertbl_mem)
        print >> fd, "#define %-60s \"%s\"" %("OPERTBL_MEMORY_USED_IN_UNITS" , convert_size(opertbl_mem))
        print >> fd, "#define %-60s %ld" %("STATIC_MEMORY_USED_IN_BYTES" , static_mem)
        print >> fd, "#define %-60s \"%s\"" %("STATIC_MEMORY_USED_IN_UNITS" , convert_size(static_mem))
        print >> fd, "#define %-60s %ld" %("UPGRADE_MEMORY_USED_IN_BYTES" , upgrade_mem)
        print >> fd, "#define %-60s \"%s\"" %("UPGRADE_MEMORY_USED_IN_UNITS" , convert_size(upgrade_mem))
        print >> fd, "\n/////////////////////////////////////////////////////////\n"

    return 0

def main():
    try:
        if parse_inputs() != 0:
            return -1
        print >> fd, hdr
        rv = parse_file()
        print >> fd, ftr
        return rv
    except Exception,e:
        print "Error in opening the file, "  + sys.argv[1]
        print e
        return -1

if __name__ == '__main__':
    exit(main())
