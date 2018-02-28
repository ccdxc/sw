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

try:
    os.makedirs(svc_gen_dir)
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

def genSvcFiles(suffix):
    tenjin_prefix = "//::"

    template = template_dir + 'svc_gen_' + suffix + '_template.py'

    fullpath = proto_gen_dir
    sys.path.insert(0, fullpath)

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

        with open(out_file, "w") as of:
            render_template(of, template, dic, './', prefix=tenjin_prefix)

        of.close()

genSvcFiles('hpp')
genSvcFiles('cc')
