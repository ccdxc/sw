#!/usr/bin/python
#-----------------------------------------------------------------------------
# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#-----------------------------------------------------------------------------
#
# Debug CLI
#

import os
import click
from click_repl import register_repl
import cli
from athena_frontend import *
from p4plus_txdma_frontend import *
from p4plus_rxdma_frontend import *

register_repl(dbg_cli)
dbg_cli.add_command(debug)

cli.cli_init(None)

if __name__ == '__main__':
    dbg_cli()
