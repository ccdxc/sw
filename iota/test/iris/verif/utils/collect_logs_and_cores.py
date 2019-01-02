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
        api.Logger.info("Techsupport for node: %s" % n)
        common.AddPenctlCommand(req, n, "system tech-support -b %s-tech-support" % (n))

    tc.resp = api.Trigger(req)

    result = api.types.status.SUCCESS
    for n,cmd in zip(nodes,tc.resp.commands):
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to execute penctl system tech-support on node: %s. err: %d" % (n, cmd.exit_code))
            result = api.types.status.FAILURE
            continue
        # Copy tech support tar out
        # TAR files are created at: pensando/iota/entities/node1_host/<test_case>
        resp = api.CopyFromHost(n, ["%s-tech-support.tar.gz" % (n)], tc_dir)
        if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
            api.Logger.error("Failed to copy techsupport file from node: %s" % n)
            result = api.types.status.FAILURE
            continue
    return result
