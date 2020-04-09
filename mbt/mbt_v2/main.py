#! /usr/bin/python3
import os
import sys
import grpc
from   base64 import test
import zmq
import random
from   subprocess import call

asic = os.environ.get('ASIC', 'capri')

paths = [
    '/mbt/',
    '/nic/build/x86_64/iris/' + asic + '/gen/proto/',
    '/nic/',
    '/dol/',
    '/dol/third_party/',
]

ws_top = os.path.dirname(sys.argv[0]) + '/../..'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top

# set the MBT_V2 for new infra
os.environ['MBT_V2'] = '1'

for path in paths:
    fullpath = ws_top + path
    sys.path.insert(0, fullpath)

import infra.common.parser  as parser
import infra.common.objects as objects
import threading
from   infra.common.glopts import GlobalOptions
import mbt_obj_store

# If the random seed is set as a command line argument, set it into the 
# environment variables now, so that the same seed is used everywhere.
if GlobalOptions.mbtrandomseed:
    os.environ['MBT_RANDOM_SEED'] = GlobalOptions.mbtrandomseed
else:
    os.environ['MBT_RANDOM_SEED'] = str(random.randint(1,10000000))

print("The random seed(MBT_RANDOM_SEED) being used for this test is %s" %(str(os.environ['MBT_RANDOM_SEED'])))

import grpc_meta.types as grpc_meta_types
from   msg import *
import config_object

grpc_meta_types.set_random_seed()

def get_hal_channel():
    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    print("Creating GRPC channel to HAL on port %s" %(port))
    server = 'localhost:' + port
    hal_channel = grpc.insecure_channel(server)
    print("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(hal_channel).result()
    print("Connected to HAL!")
    return hal_channel


objects.CallbackField.SetPackagePath("cfg.callbacks")

op_map = {
        "Create"   : "create",
        "Get"      : "get",
        "Update"   : "update",
        "Delete"   : "delete"
}

hal_channel = get_hal_channel()

config_specs = parser.ParseDirectory("../mbt/cfg/specs", "*.spec")
for config_spec in config_specs:
    for cfg_spec_obj_var in config_spec.objects:
        cfg_spec_obj = cfg_spec_obj_var.object

        print ("Registering: " + config_spec.Service + "::" + cfg_spec_obj.name)

        cfg_spec_obj_inst = config_object.ConfigSpecObjectWrapper(config_spec, cfg_spec_obj, hal_channel)

        mbt_obj_store.cfg_spec_obj_store_insert(config_spec.Service + '_' + cfg_spec_obj.name, cfg_spec_obj_inst)

        if 'key_handle' in dir(cfg_spec_obj):
            mbt_obj_store.cfg_spec_obj_store_kh_insert(cfg_spec_obj.key_handle, cfg_spec_obj_inst)

def handle_pdb(sig, frame):
    print ('Signal handler called with signal', sig)
    import pdb
    pdb.set_trace()
    print ("DONE SIGNAL")

import signal
print ('Registering signal handler')
signal.signal(signal.SIGUSR1, handle_pdb)

def create_reference_object(ref_object_spec):
    expected_api_status = 'API_STATUS_OK'

    cfg_spec_obj_inst = mbt_obj_store.cfg_spec_obj_store_kh(ref_object_spec.key_handle)

    print ("Reference Object: " + ref_object_spec.key_handle)

    constraints = None
    if ref_object_spec.constraints:
        constraints = GrpcReqRspMsg.extract_constraints(ref_object_spec.constraints)[0]

    ext_refs       = {}
    immutable_objs = {}

    print ("Creating with constraints: " + str(constraints))
    (mbt_status, api_status, mbt_handle, rsp_msg) = cfg_spec_obj_inst.create_with_constraints(ext_refs, constraints, immutable_objs)

    if expected_api_status != api_status:
        print ("Expected: " + expected_api_status + ", Got: " + api_status)
        assert False

    mbt_obj_store.ref_obj_store_insert((ref_object_spec, mbt_handle))

reference_spec = parser.ParseFile("../mbt/cfg/references", "references.spec")
for object in reference_spec.objects:
    print("Reference object being created is " + object.object.service + " Name is " + object.object.name)
    create_reference_object(object.object)

test_specs = parser.ParseDirectory("../mbt/mbt_test/specs", "*.spec")
for test_spec in test_specs:

    if 'Enabled_v2' in dir(test_spec):
        Enabled = test_spec.Enabled_v2
    else:
        Enabled = test_spec.Enabled

    if not Enabled:
        continue

    mbt_obj_store.set_walk_enum(True)

    for loop_count in range(test_spec.LoopCount):
        print ("Iteration: " + str(loop_count))

        # key:   cfg_spec_obj name
        # value: list of mbt_handles
        mbt_handle_store = {}

        for step in test_spec.Steps:
            op_name = op_map[step.step.op]

            for cfg_spec_obj in mbt_obj_store.cfg_spec_obj_list():
                if not cfg_spec_obj._config_spec.enabled:
                    continue

                # Ignore Get after Delete for L2Segment since
                # other object delete would delete L2Segment internally
                if int(step.step.num) == 6                        \
                   and cfg_spec_obj.service_name() == 'L2Segment' \
                   and cfg_spec_obj.name() == 'L2Segment':
                    # continue
                    pass

                max_objects = cfg_spec_obj.max_objects()

                print("Executing Step %s for Service %s, object %s," \
                      " max_objects %s"                              \
                       % (step.step.op, cfg_spec_obj.service_name(), \
                       cfg_spec_obj.name(), str(max_objects)))

                cfg_spec_obj.indent_reset()

                method = getattr(cfg_spec_obj, op_name)

                try:
                    if op_name == 'create':
                        ret = method(step.step.status, max_objects)

                        if ret is not None:
                            count = 0
                            for (mbt_status, api_status, mbt_handle, rsp_msg) in ret:
                                if mbt_status == mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED:
                                    api_status = 'API_STATUS_OK'
                                    mbt_handle = cfg_spec_obj.get_config_object(0)

                                if step.step.status != api_status:
                                    print("Step %s failed for Config %s" % (step.step.op, cfg_spec_obj.name()))
                                    print("Expected: " + step.step.status + ", Got: " + api_status)
                                    assert False

                                if cfg_spec_obj.name() in mbt_handle_store:
                                    mbt_handle_store[cfg_spec_obj.name()].append(mbt_handle)
                                else:
                                    mbt_handle_store[cfg_spec_obj.name()] = [mbt_handle]

                                count += 1

                            print("Step %s for Config %s, count %s" % (step.step.op, cfg_spec_obj.name(), str(count)))
                    else:
                        if cfg_spec_obj.name() in mbt_handle_store:
                            count = 0

                            mbt_handle_set = set(mbt_handle_store[cfg_spec_obj.name()])

                            if cfg_spec_obj.name() == 'L2Segment':
                                mbt_handle_set.update(cfg_spec_obj.get_config_object_list())

                            for mbt_handle in mbt_handle_set:

                                (api_status, rsp_msg) = method(mbt_handle)

                                # method is ignored
                                if api_status == 'API_STATUS_OK' and rsp_msg is None:
                                    continue

                                expected_status = step.step.status

                                # special cases
                                if step.step.op == 'Get':
                                   if cfg_spec_obj.name() == 'SecurityGroup':
                                        expected_status = 'API_STATUS_OK'

                                if expected_status != api_status:
                                    print("Step %s failed for Config %s" % (step.step.op, cfg_spec_obj.name()))
                                    print("Expected: " + step.step.status + ", Got: " + api_status)
                                    assert False

                                count += 1

                            print("Step %s for Config %s, count %s" % (step.step.op, cfg_spec_obj.name(), str(count)))
                        else:
                            print ("Could not find mbt_handle for operation: " + step.step.op)
                except Exception as ex:
                    print("Received Exception", ex)
                    raise
                call(["kill -10 `pgrep -x hal`"], shell=True)

print("All Config test passed!")
