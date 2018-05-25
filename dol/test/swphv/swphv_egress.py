#! /usr/bin/python3
# Test Module

import pdb
import model_sim.src.model_wrap as model_wrap
from infra.common.glopts import GlobalOptions
import config.hal.defs          as haldefs
import config.hal.api           as halapi
import internal_pb2             as internal_pb2

def swphv_inject():
    stub = internal_pb2.InternalStub(halapi.HalChannel)
    req_msg = internal_pb2.SoftwarePhvInjectMsg()
    req_spec = req_msg.request.add()
    req_spec.pipeline = internal_pb2.SOFTWARE_PHV_EGRESS
    resp_msg = stub.SoftwarePhvInject(req_msg)

def swphv_get_state():
    stub = internal_pb2.InternalStub(halapi.HalChannel)
    req_msg = internal_pb2.SoftwarePhvGetRequestMsg()
    req_spec = req_msg.request.add()
    req_spec.pipeline = internal_pb2.SOFTWARE_PHV_EGRESS
    resp_msg = stub.SoftwarePhvGet(req_msg)
    # FIXME: remove this debug message
    print("Got swphv response: ")
    print(resp_msg)
    num_resp_specs = len(resp_msg.response)
    if num_resp_specs != 1:
        assert(0)

    resp_status = resp_msg.response[0].status
    return resp_status

def Setup(infra, module):
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    print("Triggering txdma sw phv test")
    swphv_inject()
    return

def TestCaseTrigger(tc):
    if (GlobalOptions.dryrun):
        return
    return

def TestCaseVerify(tc):
    print("Verifying egress sw phv test")

    # get the state
    resp = swphv_get_state()

    # verify the phv injection is done
    if resp.done != True:
        return False

    # Verify the counter was incremented
    if resp.inject_cntr != 1:
        return False

    return True

def TestCaseTeardown(tc):
    return
