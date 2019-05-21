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
from artemis_frontend import *

from artemis_txdma_frontend import *
from artemis_rxdma_frontend import *
#from common_txdma_actions_frontend import *
#from common_rxdma_actions_frontend import *

register_repl(dbg_cli)
dbg_cli.add_command(debug)

cli.cli_init(None)

if __name__ == '__main__':
    dbg_cli()
