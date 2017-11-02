#! /usr/bin/python3
import os,sys
import pdb
import grpc
from base64 import test
paths = [
    '/nic/gen/proto/',
    '/nic/gen/proto/hal/',
    '/nic',
    '/dol/',
    '/dol/third_party/'
]

ws_top = os.path.dirname(sys.argv[0]) + '/../..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import config_mgr
import infra.common.parser  as parser
import infra.common.utils  as utils
from infra.common.logging import logger
import infra.common.objects as objects

objects.CallbackField.SetPackagePath("config_validator.cfg.callbacks")

def get_hal_channel():
    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    logger.info("Creating GRPC channel to HAL")
    server = 'localhost:' + port
    hal_channel = grpc.insecure_channel(server)
    logger.info("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(hal_channel).result()
    logger.info("Connected to HAL!")
    return hal_channel

op_map = {
        "Create"   : "CreateConfigs",
        "ReCreate" : "ReCreateConfigs",
        "Get"      : "VerifyConfigs",
        "Update"   : "UpdateConfigs",
        "Delete"   : "DeleteConfigs"
    }

hal_channel = get_hal_channel()

config_specs = parser.ParseDirectory("config_validator/cfg/specs", "*.spec")
for config_spec in config_specs:
    if config_spec.enabled:
        logger.info("Adding config spec for service : " , config_spec.Service)
        config_mgr.AddConfigSpec(config_spec, hal_channel)

logger.info("Building Config dependencey information")
config_mgr.BuildConfigDeps()
    
test_specs = parser.ParseDirectory("config_validator/test/specs", "*.spec")
for test_spec in test_specs:
    if not test_spec.Enabled:
        continue
    config_mgr.ResetConfigs()
    for step in test_spec.Steps:
        logger.info("Executing Step : ", step.step.op)
        op_name = op_map[step.step.op]
        for cfg_spec in config_mgr.GetOrderedConfigSpecs(rev=step.step.op=="Delete"):
            logger.info("Executing Step  %s for Conifg %s" % (step.step.op, cfg_spec))
            method = getattr(cfg_spec, op_name)
            try:
                ret = method(test_spec.MaxObjects, step.step.status)
            except Exception as ex:
                logger.critical("Received Exception", ex)
                utils.log_exception(logger)
                ret = False
            if not ret:
                logger.info("Step %s failed for Conifg %s" % (step.step.op, cfg_spec))
                sys.exit(1)

logger.info("All Config test passed!")