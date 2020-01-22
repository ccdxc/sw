#! /usr/bin/python3
import sys
import os
import pdb

from infra.common.glopts import GlobalOptions as GlobalOptions
from infra.common.logging import logger as logger

import infra.common.objects as objects
import infra.config.config as config
import infra.engine.engine as engine

from apollo.config.store import EzAccessStore as EzAccessStore

# reusing apollo objects
APULU_CONFIG_TEMPLATE_PATH  = "apollo/config/templates/"
APULU_CONFIG_SPEC_PATH      = "apollo/config/specs/"
APULU_CONFIG_TOPO_SPEC_PATH = "apollo/config/topology/%s" % GlobalOptions.topology
APULU_TEST_PATH             = "apulu/test/"
APULU_TEST_CALLBACK_PATH    = "apollo.test.callbacks"
APULU_PROTO_PATH            = os.environ['WS_TOP'] + '/nic/build/x86_64/apulu/gen/proto/'
sys.path.insert(0, APULU_PROTO_PATH)
import apollo.config.topology    as topology

def InitConfig():
    logger.info("Initializing APULU Config Templates and Specs")
    config.Init(EzAccessStore, APULU_CONFIG_TEMPLATE_PATH, APULU_CONFIG_SPEC_PATH, APULU_CONFIG_TOPO_SPEC_PATH)
    return

def InitEngine():
    engine.Init(APULU_TEST_PATH);
    return

def GenerateConfig():
    topology.Main()
    return

def Init():
    objects.CallbackField.SetPackagePath(APULU_TEST_CALLBACK_PATH)
    return

def Main():
    return engine.main()
