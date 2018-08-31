# Quiesce support routines
import pdb

import types_pb2                as types_pb2

from iris.config.store               import Store
from infra.common.objects       import ObjectDatabase as ObjectDatabase
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
import infra.config.base        as base
import iris.config.hal.defs          as haldefs
import iris.config.hal.api           as halapi
import internal_pb2              as internal_pb2

def quiesce_start():
    stub = internal_pb2.InternalStub(halapi.HalChannel)
    req_msg = internal_pb2.EmptyRequest()
    req_msg.request.extend([types_pb2.Empty()])
    print("Invoking Quiesce API: QuiesceStart");
    stub.QuiesceStart(req_msg)
    return True

def quiesce_stop():
    stub = internal_pb2.InternalStub(halapi.HalChannel)
    req_msg = internal_pb2.EmptyRequest()
    req_msg.request.extend([types_pb2.Empty()])
    print("Invoking Quiesce API: QuiesceStop");
    stub.QuiesceStop(req_msg)
    return True

def Setup(infra, module):
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    print("Quiesce API: %s" % tc.module.args.api)

    if halapi.IsHalDisabled():
        return

    if tc.module.args.api == "QUIESCE_START":
        quiesce_start()
    elif tc.module.args.api == "QUIESCE_STOP":
        quiesce_stop()
    else:
        print("Invalid API in module definition")
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return
