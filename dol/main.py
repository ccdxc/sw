#! /usr/bin/python3
import sys
import os
import pdb
import infra.common.defs            as defs
import infra.common.timeprofiler    as timeprofiler
timeprofiler.TotalTimeProfiler.Start()
timeprofiler.InitTimeProfiler.Start()
paths = [
    '/nic/gen/proto/',
    '/nic/gen/proto/hal/',
    '/mbt/',
    '/nic',
    '/dol/third_party/'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
os.environ['MODEL_SOCK_PATH'] = ws_top + '/nic/'
os.environ['MODEL_DATA_OUT_DIR'] = ws_top + '/nic/coverage/asm_out_all'
os.environ['MODEL_DATA_DEST_DIR'] = ws_top + '/nic/coverage/asm_out_final'
for path in paths:
    fullpath = ws_top + path
    #print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

# This import will parse all the command line options.
import infra.common.glopts as glopts
from infra.common.logging import logger
import sys

glopts.ValidateGlopts()

import infra.factory.factory    as factory
import infra.engine.engine      as engine
import infra.config.config      as config
import config.generator         as generator
import infra.e2e.main           as e2e

def Main():
    logger.info("Initializing Config Infra")
    config.init()

    logger.info("Initializing Factory.")
    factory.init()

    logger.info("Initializing Engine.")
    engine.init()

    logger.info("Initializing Topology.")
    topo = glopts.GlobalOptions.topology
    topofile = '%s/%s.topo' % (topo, topo)
    timeprofiler.InitTimeProfiler.Stop()
    generator.main(topofile)

    if glopts.GlobalOptions.cfgjson:
        #Dump the configuration to file.
        cfg_file = ws_top + '/nic/' + glopts.GlobalOptions.cfgjson
        generator.dump_configuration(cfg_file)
    
    if glopts.GlobalOptions.e2e:
        #Start E2E 
        cfg_file = ws_top + '/nic/' + glopts.GlobalOptions.cfgjson
        e2e.Start(cfg_file)

    if glopts.GlobalOptions.cfgonly:
        logger.info("CONFIG Only Run......Stopping.")
        return 0

    status = engine.main()
    
    if glopts.GlobalOptions.e2e:
        e2e.Stop()
    
    return status


if __name__ == '__main__':
    status = Main()
    print("Runtime:")
    timeprofiler.TotalTimeProfiler.Stop()
    timeprofiler.Show()
    sys.exit(status)

