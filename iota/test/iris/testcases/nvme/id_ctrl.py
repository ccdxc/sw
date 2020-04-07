#! /usr/bin/python3
import iota.harness.api as api
import json

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Looking for Pensando Controller at host {0}".format(tc.nodes[1]))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "nvme id-ctrl /dev/%s --output-format json" %(tc.iterators.ctrl))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    f = open('test/iris/testcases/nvme/nvmf_cfg.json')
    nvmf_cfg = json.load(f)

    api.Logger.info("nvme_id_ctrl results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

        output = json.loads(cmd.stdout)
        if output['mn'] != nvmf_cfg['ctrl_name']:
            result = api.types.status.FAILURE
        if output['subnqn'] != nvmf_cfg['nqn']:
            result = api.types.status.FAILURE
        if output['npss'] != 0:				# As per unh compliance doc
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
