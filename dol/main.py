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
os.environ['ZMQ_SOC_DIR'] = ws_top + '/nic/'

for path in paths:
    fullpath = ws_top + path
    #print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

# This import will parse all the command line options.
import infra.common.glopts as glopts
import infra.common.loader as loader
from infra.common.logging import logger

glopts.ValidateGlopts()

import infra.factory.factory    as factory
import infra.e2e.main           as e2e

from infra.asic.pktcollector    import PacketCollector

def GetPipeline():
    pipeline = loader.ImportModule(glopts.GlobalOptions.pipeline,
                                   glopts.GlobalOptions.pipeline)
    return pipeline

def Main():
    pipeline = GetPipeline()
    logger.info("Initializing Pipeline")
    pipeline.Init()

    logger.info("Initializing Config for Pipeline")
    pipeline.InitConfig()

    logger.info("Initializing Factory.")
    factory.init()

    logger.info("Initializing Engine for Pipeline.")
    pipeline.InitEngine()

    timeprofiler.InitTimeProfiler.Stop()

    pipeline.GenerateConfig()
   
    if glopts.GlobalOptions.e2e:
        #Start E2E 
        cfg_file = ws_top + '/nic/' + glopts.GlobalOptions.cfgjson
        e2e.Start(cfg_file)

    if glopts.GlobalOptions.cfgonly:
        logger.info("CONFIG Only Run......Stopping.")
        return 0

    status = pipeline.Main()
    
    if glopts.GlobalOptions.e2e:
        e2e.Stop()
   
    if glopts.GlobalOptions.savepcap:
        PacketCollector.Write()

    return status


if __name__ == '__main__':
    status = Main()
    print("Runtime:")
    timeprofiler.TotalTimeProfiler.Stop()
    timeprofiler.Show()
    sys.exit(status)

