#! /usr/bin/python3
import sys
import os
import pdb

asic = os.environ.get('ASIC', 'capri')
paths = [
    '/dol/',
    '/dol/third_party/'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top


for path in paths:
    fullpath = ws_top + path
    #print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)


APOLLO_PROTO_PATH = os.environ['WS_TOP'] + '/nic/build/x86_64/apollo/' + asic + '/gen/proto/'

sys.path.insert(0, APOLLO_PROTO_PATH)
