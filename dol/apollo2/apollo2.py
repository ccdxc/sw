#! /usr/bin/python3

import sys
import os
import pdb

import infra.common.defs            as defs
import infra.common.objects         as objects
import infra.config.config          as config
import infra.engine.engine          as engine
import apollo2.config.generator     as generator

from infra.common.glopts    import GlobalOptions as GlobalOptions
from infra.common.logging   import logger as logger
from infra.config.store     import ConfigStore as ConfigStore

APOLLO2_CONFIG_TEMPLATE_PATH   = "apollo2/config/templates/"
APOLLO2_CONFIG_SPEC_PATH       = "apollo2/config/specs/"
APOLLO2_CONFIG_TOPO_SPEC_PATH  = "apollo2/config/topology/%s" % GlobalOptions.topology
APOLLO2_TEST_PATH              = "apollo2/test/"

def InitConfig():
    logger.info("Initializing APOLLO2 Config Templates and Specs")    
    config.Init(ConfigStore,
                APOLLO2_CONFIG_TEMPLATE_PATH,
                APOLLO2_CONFIG_SPEC_PATH,
                APOLLO2_CONFIG_TOPO_SPEC_PATH)
    return

def InitEngine():
    engine.Init(APOLLO2_TEST_PATH);
    return

def GenerateConfig():
    logger.info("Initializing Topology.")
    topo = GlobalOptions.topology
    topofile = '%s/%s.topo' % (topo, topo)
    generator.Main(topofile)
    return

def Init():
    objects.CallbackField.SetPackagePath("apollo2.test.callbacks")
    return

def Main():
    return engine.main()
