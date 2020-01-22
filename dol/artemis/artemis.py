#! /usr/bin/python3
import sys
import os
import pdb

import infra.common.defs as defs
import infra.common.objects as objects
import infra.config.config as config
import infra.engine.engine as engine

from infra.common.glopts import GlobalOptions as GlobalOptions
from infra.common.logging import logger as logger
from apollo.config.store import EzAccessStore as EzAccessStore

ARTEMIS_CONFIG_TEMPLATE_PATH = "apollo/config/templates/"
ARTEMIS_CONFIG_TOPO_SPEC_PATH = "apollo/config/topology/%s" % GlobalOptions.topology
ARTEMIS_TEST_PATH = "artemis/test/"
ARTEMIS_PROTO_PATH = os.environ['WS_TOP'] + '/nic/build/x86_64/artemis/gen/proto/'
sys.path.insert(0, ARTEMIS_PROTO_PATH)
import apollo.config.topology    as topology

def InitConfig():
    logger.info("Initializing ARTEMIS Config Templates and Specs")
    config.Init(EzAccessStore, ARTEMIS_CONFIG_TEMPLATE_PATH, None, None)
    return

def InitEngine():
    engine.Init(ARTEMIS_TEST_PATH);
    return

def GenerateConfig():
    topology.Main()
    return

def Init():
    objects.CallbackField.SetPackagePath("artemis.test.callbacks")
    return

def Main():
    return engine.main()
