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
from gft_frontend import *
from common_txdma_actions_frontend import *
from common_rxdma_actions_frontend import *

register_repl(dbg_cli)
dbg_cli.add_command(debug)
grpc_server_port = "localhost:50054"

if 'HAL_GRPC_PORT' in os.environ.keys():
    grpc_server_port = "localhost:" + os.environ['HAL_GRPC_PORT']

cli.cli_init(grpc_server_port)

if __name__ == '__main__':
    dbg_cli()
