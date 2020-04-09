#! /usr/bin/python3

import sys
import os
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.config      as config
import infra.engine.engine      as engine

from infra.common.glopts    import GlobalOptions as GlobalOptions
from infra.common.logging   import logger as logger

from iris.config.store      import Store as Store

asic = os.environ.get('ASIC', 'capri')
IRIS_CONFIG_TEMPLATE_PATH   = "iris/config/templates/"
IRIS_CONFIG_SPEC_PATH       = "iris/config/specs/"
IRIS_CONFIG_TOPO_SPEC_PATH  = "iris/config/topology/%s" % GlobalOptions.topology
IRIS_TEST_PATH              = "iris/test/"
IRIS_PROTO_PATH = os.environ['WS_TOP'] + '/nic/build/x86_64/iris/' + asic + '/gen/proto/'
GFT_PROTO_PATH = os.environ['WS_TOP'] + '/nic/build/x86_64/gft/' + asic + '/gen/proto/'

sys.path.insert(0, IRIS_PROTO_PATH)
sys.path.insert(0, GFT_PROTO_PATH)
import iris.config.generator    as generator

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

    if GlobalOptions.cfgjson:
        #Dump the configuration to file.
        cfg_file = os.environ['WS_TOP'] + '/nic/' + GlobalOptions.cfgjson
        generator.SaveStoreToJson(cfg_file)
    return

def Init():
    objects.CallbackField.SetPackagePath("iris.test.callbacks")
    return

def Main():
    return engine.main()
