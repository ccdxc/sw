#! /usr/bin/python3

import sys
import os
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.config      as config
import infra.engine.engine      as engine

import iris.config.generator    as generator

from infra.common.glopts    import GlobalOptions as GlobalOptions
from infra.common.logging   import logger as logger

from iris.config.store      import Store as Store

IRIS_CONFIG_TEMPLATE_PATH   = "iris/config/templates/"
IRIS_CONFIG_SPEC_PATH       = "iris/config/specs/"
IRIS_CONFIG_TOPO_SPEC_PATH  = "iris/config/topology/%s" % GlobalOptions.topology
IRIS_TEST_PATH              = "iris/test/"

def InitConfig():
    logger.info("Initializing IRIS Config Templates and Specs")    
    config.Init(Store, IRIS_CONFIG_TEMPLATE_PATH,
                IRIS_CONFIG_SPEC_PATH, IRIS_CONFIG_TOPO_SPEC_PATH)
    return

def InitEngine():
    engine.Init(IRIS_TEST_PATH);
    return

def GenerateConfig():
    logger.info("Initializing Topology.")
    topo = GlobalOptions.topology
    topofile = '%s/%s.topo' % (topo, topo)
    generator.main(topofile)
    return

def Init():
    objects.CallbackField.SetPackagePath("iris.test.callbacks")
    return

def Main():
    return engine.main()
