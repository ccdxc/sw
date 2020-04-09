#!/usr/bin/python3

import os
import sys
import random

asic = os.environ.get('ASIC', 'capri')

paths = [
    '/mbt/',
    '/nic/build/x86_64/iris/' + asic + '/gen/proto/',
    '/dol/',
    '/dol/third_party/',
]

ws_top = os.path.dirname(sys.argv[0]) + '/../..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top

# set the MBT_V2 for new infra
os.environ['MBT_V2'] = '1'
os.environ['GEN_MAX_VALUE'] = '1'

for path in paths:
    fullpath = ws_top + path
    sys.path.insert(0, fullpath)

import infra.common.parser  as parser
import infra.common.objects as objects
from   infra.common.glopts import GlobalOptions

# If the random seed is set as a command line argument, set it into the
# environment variables now, so that the same seed is used everywhere.
if GlobalOptions.mbtrandomseed:
    os.environ['MBT_RANDOM_SEED'] = GlobalOptions.mbtrandomseed
else:
    os.environ['MBT_RANDOM_SEED'] = str(random.randint(1,10000000))

import grpc_meta.types as grpc_meta_types
from   msg import *
import utils
import mbt_obj_store

grpc_meta_types.set_random_seed()

max_size_val = 0
count        = 0

def max_size_cb(req_msg,
                cb_args,
                ext_refs,
                ext_constraints,
                immutable_objs,
                enums_list,
                field_values):

    global max_size_val
    global count

    size = req_msg.ByteSize()

    print (req_msg)

    if size > max_size_val:
        max_size_val = size

    count += 1

    return True


def max_size(proto_message_type):

    cb_args         = {}
    ext_refs        = {}
    immutable_objs  = {}
    ext_constraints = None
    num_objects     = 1

    mbt_obj_store.set_walk_enum(True)
    mbt_obj_store.set_skip_none_enum(True)

    walk_proto(proto_message_type,
               max_size_cb,
               cb_args,
               num_objects,
               ext_refs,
               ext_constraints,
               immutable_objs)


module_name        = 'fwlog_pb2'
proto_message_name = 'FWEvent'

module             = utils.load_module(module_name)
proto_message_type = getattr(module, proto_message_name)

max_size(proto_message_type)

print ("Max size: " + str(max_size_val))
