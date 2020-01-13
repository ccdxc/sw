#! /usr/bin/python3
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.apulu.testcases.drivers.cmd_builder as cmd_builder
import iota.test.apulu.testcases.drivers.common as common


driver_verify_handlers = {}
def register_tc_handler(handler, key):
    if key in driver_verify_handlers:
        api.Logger.error("Verify handler already registered %s" % key)
        assert(0)
    driver_verify_handlers[key] = handler

def driver_feature_verify(tc):
    if tc.Name() in driver_verify_handlers:
        return driver_verify_handlers[tc.Name()](tc)
    api.Logger.info("No driver feature verification")
    return api.types.status.SUCCESS

def verify_tso(tc):
    api.Logger.info("TSO verification called.")
    return api.types.status.SUCCESS

register_tc_handler(verify_tso, 'DRIVER_FEATURE_TSO')
