#!/usr/bin/python
#-----------------------------------------------------------------------------
# {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#-----------------------------------------------------------------------------
#
# Debug CLI
#

import os
import click
from click_repl import register_repl
import cli
#from tls_txdma_pre_crypto_enc_frontend import *
#from tls_txdma_pre_crypto_dec_frontend import *
import importlib
import sys

from cli_frontend import *

sw_dir = '../..'
nic_dir = sw_dir + '/nic'
if 'P4PLUS_MOD' in os.environ.keys():
    p4plus_module = os.environ['P4PLUS_MOD']
    p4plus_mod_path = nic_dir + '/gen/' + p4plus_module + '/cli'
else:
    print("Missing P4PLUS_MOD_PATH")
    quit()

sys.path.append(p4plus_mod_path)
modl = importlib.import_module(p4plus_module + '_frontend')
if modl is not None:
    print("Loaded Module: [%s]" % (p4plus_module + '_frontend'))

register_repl(dbg_cli)
dbg_cli.add_command(debug)
grpc_server_port = "localhost:50054"

if 'HAL_GRPC_PORT' in os.environ.keys():
    grpc_server_port = "localhost:" + os.environ['HAL_GRPC_PORT']

cli.cli_init(grpc_server_port)

if __name__ == '__main__':
    dbg_cli()
