#! /usr/bin/python3
import sys
import os
import pdb
import infra.common.defs            as defs
import infra.common.timeprofiler    as timeprofiler
timeprofiler.TotalTimeProfiler.Start()
timeprofiler.InitTimeProfiler.Start()
paths = [
    '/bazel-genfiles/',
    '/bazel-genfiles/nic/proto/',
    '/bazel-genfiles/nic/proto/hal/',
    '/nic/gen/protobuf/',
    '/nic',
    '/dol/third_party/'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
os.environ['MODEL_SOCK_PATH'] = ws_top + '/nic/'
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import infra.clibs.clibs	    as clibs
import infra.factory.factory    as factory
import infra.engine.engine      as engine
import infra.config.config      as config
import config.generator         as generator
import config.resmgr            as resmgr

# This import will parse all the command line options.
import infra.common.glopts as glopts

from infra.common.logging import logger

import sys

def Main():
    logger.info("Initializing clibs")
    clibs.init()
    resmgr.init()

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

    if glopts.GlobalOptions.cfgonly:
        logger.info("CONFIG Only Run......Stopping.")
        return 0

    return engine.main()


if __name__ == '__main__':
    status = Main()
    print("Runtime:")
    timeprofiler.TotalTimeProfiler.Stop()
    timeprofiler.Show()
    sys.exit(status)

