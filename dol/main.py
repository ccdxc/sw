#! /usr/bin/python3
import sys
import os
import pdb

import infra.common.defs as defs

ws_top = os.environ['WS_TOP']

pb2path = ws_top + '/hack/saratk/nic/gen/protobuf/'
sys.path.append(ws_top + '/hack/saratk/nic')
sys.path.append(pb2path)
tp_path = ws_top + '/hack/dol/third_party/'
sys.path.insert(0, tp_path)

import infra.factory.factory    as factory
import infra.engine.engine      as engine
import infra.config.config      as config
import config.generator         as generator

# This import will parse all the command line options.
import infra.common.glopts as glopts

from infra.common.logging import logger

logger.info("Initializing Config Infra")
config.init()

generator.main(glopts.GlobalOptions.topology)

if glopts.GlobalOptions.cfgonly:
    logger.info("CONFIG Only Run......Stopping.")
    sys.exit(0)

logger.info("Initializing Factory.")
factory.init()

logger.info("Initializing Engine.")
engine.init()
engine.main()


