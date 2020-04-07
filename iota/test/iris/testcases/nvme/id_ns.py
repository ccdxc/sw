#! /usr/bin/python3
import iota.harness.api as api
import json

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Looking for Pensando Namespace at host {0}".format(tc.nodes[1]))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "nvme id-ns /dev/%s --output-format json" %(tc.iterators.namespace))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    f = open('test/iris/testcases/nvme/nvmf_cfg.json')
    nvmf_cfg = json.load(f)

    api.Logger.info("nvme_id_ns results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

        output = json.loads(cmd.stdout)
        if output['nsze'] != (int(nvmf_cfg['block_size']) * int(nvmf_cfg['num_block']) * 4):
            result = api.types.status.FAILURE
        if output['nguid'] != "00000000000000000000000000000000":
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
