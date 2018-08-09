#! /usr/bin/python3
import pdb
import os
import json
from collections import defaultdict 

import infra.common.defs            as defs
import infra.common.utils           as utils
import infra.common.parser          as parser
import infra.common.timeprofiler    as timeprofiler

import apollo2.config.hal.api       as halapi
import apollo2.config.resmgr        as resmgr

from infra.common.logging       import logger as logger
from infra.config.store         import ConfigStore as ConfigStore
from infra.common.glopts        import GlobalOptions as GlobalOptions

def __process_topology(topospec):
    return

def Main(topofile):
    timeprofiler.ConfigTimeProfiler.Start()
    halapi.Init()

    logger.info("Initializing Resmgr")
    resmgr.Init()
    
    logger.info("Generating Config Objects for Topology = %s" % topofile)
    topospec = parser.ParseFile('apollo2/config/topology/', topofile)

    if topospec:
        __process_topology(topospec)
    timeprofiler.ConfigTimeProfiler.Stop()
    return

def SaveStoreToJson(conf_file):
    config_dict = defaultdict(lambda:{})
    for cfg_object in Store.objects.GetAll():
        config_dict[cfg_object.__class__.__name__][cfg_object.GID()] = cfg_object.ToJson()
    with open(conf_file, 'w') as fp:
        json.dump(config_dict, fp, indent=4)
    logger.info("Dumped configuration to file %s" % conf_file)
    
