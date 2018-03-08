#!/usr/bin/python3
import os
import sys
import importlib
import errno

paths = [
    '/nic/cli/', # To get tenjin_wrapper.
]

ws_top = os.getcwd() + '/../'

svc_gen_dir   = ws_top + '/nic/gen/hal/svc/'
proto_gen_dir = ws_top + '/nic/gen/proto/hal/'
template_dir  = ws_top + '/nic/tools/hal/'
api_stats_dir = ws_top + '/nic/gen/hal/include/'

try:
    os.makedirs(svc_gen_dir)
except OSError as e:
    if e.errno != errno.EEXIST:
        raise

try:
    os.makedirs(api_stats_dir)
except OSError as e:
    if e.errno != errno.EEXIST:
        raise

os.environ['WS_TOP'] = ws_top

for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

from tenjin import *
from tenjin_wrapper import *

def file_skip(name):
    skip_list = ['port', 'quiesce', 'debug', 'barco_rings', 'event', 'interface_pb2.py', 'proxy_pb2.py', 'wring', 'rdma', 'telemetry_pb2.py', 'system', 'session_pb2.py', 'rawrcb']

    if not name.endswith(".py") or "grpc" in name:
        return True

    for file_name in skip_list:
        if file_name in name:
            return True

    return False

enumCount = 0

def createStatsFile(name):
    filepath = api_stats_dir + name
    f = open(filepath, "w+")
    global enumCount

    enumCount = 0
    copyright = "// {C} Copyright 2017 Pensando Systems Inc. All rights reserved\r\n\r\n";
    hdrs = "//Auto-Generated file\r\n#ifndef __HAL_API_STATS_HPP__\r\n#define __HAL_API_STATS_HPP__\r\n\r\n#include \"nic/include/base.h\"\r\n#include \"nic/include/hal_state.hpp\"\r\n\r\n"
    namespace = "namespace hal {\r\n"
    comments = ("//------------------------------------------------------------------------------\r\n"
               "// Counters for API calls/successes/fails\r\n"
               "//------------------------------------------------------------------------------\r\n\r\n");
    enums = "#define HAL_API_ENTRIES(ENTRY)    \\\r\n";
    f.write(copyright + hdrs + namespace + comments + enums);
    f.close()

def closeStatsFile(name):
    filepath = api_stats_dir + name
    f = open(filepath, "a")
    global enumCount
    max_entry = "ENTRY(HAL_API_MAX,"
    enumC = str(enumCount) + ","
    final_text = ("DEFINE_ENUM(hal_api_t, HAL_API_ENTRIES)\r\n"
                  "#undef HAL_API_ENTRIES\r\n\r\n"
                  "extern hal_state *g_hal_state;\r\n\r\n"
                  "#define HAL_API_STATS_INC(ctr)            (hal::g_hal_state->set_api_stats(ctr, hal::g_hal_state->api_stats(ctr) + 1))\r\n"
                  "#define HAL_API_STATS_ADD(ctr, cnt)       (hal::g_hal_state->set_api_stats(ctr, hal::g_hal_state->api_stats(ctr) + cnt))\r\n\r\n"
                  "}    // namespace hal\r\n\r\n"
                  "#endif    // __HAL_STATE_HPP__\r\n")
    f.write("\t%-69s%-4s %-50s\r\n\r\n" % (max_entry, enumC, "\"max\")"))
    f.write(final_text)
    f.close()

def genSvcFiles(suffix):
    tenjin_prefix = "//::"

    template = template_dir + 'svc_gen_' + suffix + '_template.py'

    fullpath = proto_gen_dir
    sys.path.insert(0, fullpath)

    global enumCount

    for fileName in os.listdir(fullpath):
        if file_skip(fileName) == True:
                continue

        fileModule = importlib.import_module(fileName[:-3])

        if not fileModule.DESCRIPTOR.services_by_name.items():
            continue

        print ('File: ' + fileName)

        out_file = svc_gen_dir + fileName.replace('_pb2.py', '') + '_svc_gen.' + suffix

        dic = {}
        dic['fileName'] = fileName;
        dic['enumCount'] = enumCount;
        

        with open(out_file, "w") as of:
            render_template(of, template, dic, './', prefix=tenjin_prefix)

        for service in fileModule.DESCRIPTOR.services_by_name.items():
            for method in service[1].methods_by_name.items():
                enumCount = enumCount + 3
            
        of.close()

genSvcFiles('hpp')
createStatsFile('hal_api_stats.hpp')
genSvcFiles('cc')
closeStatsFile('hal_api_stats.hpp')
