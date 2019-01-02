#! /usr/bin/python3

import iota.harness.api as api
import iota.protos.pygen.types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.common as common

def Main(tc):
    if tc.GetStatus() == api.types.status.SUCCESS:
        return api.types.status.SUCCESS
    api.Logger.info("TC: %s failed. Collecting techsupport." % (tc.Name()))
    nodes = api.GetNaplesHostnames()
    req = api.Trigger_CreateExecuteCommandsRequest()
    tc_dir = tc.GetLogsDir()
    for n in nodes:
        api.Logger.info("Techsupport fron node: %s" % n)
        common.AddPenctlCommand(req, n, "system tech-support")
        resp = api.Trigger(req)
        if resp != api.types.status.SUCCESS:
            api.Logger.error("Failed to execute penctl system tech-support. err: %d" % resp)
            return api.types.status.FAILURE
        # Copy tech support tar out
        resp = api.CopyFromHost(n, [common.DEF_TECH_SUPPORT_FILE], "%s/%s" % (tc_dir, n))
        if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
            api.Logger.error("Failed to copy techsupport file from node: %s" % n)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS
