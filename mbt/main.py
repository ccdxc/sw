#! /usr/bin/python3
import os,sys
import pdb
import grpc
from base64 import test
import zmq
import random

paths = [
    '/nic/gen/proto/',
    '/nic/gen/proto/hal/',
    '/nic/',
    '/dol/',
    '/dol/third_party/',
]

ws_top = os.path.dirname(sys.argv[0]) + '/..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import infra.common.parser  as parser
import infra.common.utils  as utils
from infra.common.logging import logger
import infra.common.objects as objects
import threading
from infra.common.glopts import GlobalOptions

# If the random seed is set as a command line argument, set it into the 
# environment variables now, so that the same seed is used everywhere.
if GlobalOptions.mbtrandomseed:
    os.environ['MBT_RANDOM_SEED'] = GlobalOptions.mbtrandomseed
else:
    os.environ['MBT_RANDOM_SEED'] = str(random.randint(1,10000000))

print("The random seed(MBT_RANDOM_SEED) being used for this test is %s" %(str(os.environ['MBT_RANDOM_SEED'])))

# This ordering is needed because the random seed is set by the below 2 imports
import grpc_proxy
import config_mgr

objects.CallbackField.SetPackagePath("cfg.callbacks")

def start_zmq_server():
    logger.info("Starting ZMQ server for signaling DOL!")
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("ipc://%s/zmqsockmbt" % ws_top)
    while True:
        #  Wait for next request from client
        message = socket.recv().decode("utf-8")
        print( "Received Config Complete for message type: ", message)
        config_mgr.ModifyConfigFromDol(message)
        socket.send_string ("Proceed")

# Create a thread for zmq signaling with DOL
if GlobalOptions.mbt:
    threading.Thread(target=start_zmq_server).start()

def get_hal_channel():
    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    logger.info("Creating GRPC channel to HAL on port %s" %(port))
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

config_specs = parser.ParseDirectory("../mbt/cfg/specs", "*.spec")
for config_spec in config_specs:
    if config_spec.graphEnabled:
        logger.info("Adding config spec for service : " , config_spec.Service)
        config_mgr.AddConfigSpec(config_spec, hal_channel)

logger.logfile = "./logfile"
logger.info("Building Config dependency information")

if GlobalOptions.mbt:
    # This is blocking.
    grpc_proxy.serve()

test_specs = parser.ParseDirectory("../mbt/mbt_test/specs", "*.spec")
for test_spec in test_specs:
    if not test_spec.Enabled:
        continue
    for step in test_spec.Steps:
        logger.info("Executing Step : ", step.step.op)
        op_name = op_map[step.step.op]
        for cfg_spec in config_mgr.GetOrderedConfigSpecs(rev=step.step.op=="Delete"):
            if not cfg_spec._spec.enabled:
                continue
            logger.info("Executing Step  %s for Config %s and object %s" % (step.step.op, cfg_spec, cfg_spec._service_object.name))
            method = getattr(cfg_spec, op_name)
            try:
                ret = method(test_spec.MaxObjects, step.step.status)
            except Exception as ex:
                logger.critical("Received Exception", ex)
                utils.log_exception(logger)
                ret = False
            if not ret:
                logger.info("Step %s failed for Config %s" % (step.step.op, cfg_spec))
                sys.exit(1)

logger.info("All Config test passed!")
