#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.host as host
import json

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    f = open('test/iris/testcases/nvme/nvmf_cfg.json')
    nvmf_cfg = json.load(f)
    api.Logger.info("Setting up target {0} for spdk".format(tc.nodes[0]))
    api.Trigger_AddHostCommand(req, tc.nodes[0], "python /naples/target_malloc.py --intf " + nvmf_cfg['intf'] + " --ip " + nvmf_cfg['tgt_ip'] + " --prefixlen " + nvmf_cfg['prefixlen'] + " --mtu " + nvmf_cfg['mtu'] + " --hmem " + nvmf_cfg['hmem'] + " --spdk_dir " + nvmf_cfg['spdk_dir'] + " --cpu_mask " + nvmf_cfg['cpu_mask'] + " --mdev_name " + nvmf_cfg['mdev_name'] + " --block_size " + nvmf_cfg['block_size'] + " --num_block " + nvmf_cfg['num_block'] + " --ctrl_name " + nvmf_cfg['ctrl_name'] + " --nqn " + nvmf_cfg['nqn'])

    api.Logger.info("Setting up host {0} for nvme over fabrics".format(tc.nodes[1]))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "python /naples/initiator.py --intf " + nvmf_cfg['intf'] + " --ip " + nvmf_cfg['ini_ip'] + " --prefixlen " + nvmf_cfg['prefixlen'] + " --r_ip " + nvmf_cfg['tgt_ip'] + " --mtu " + nvmf_cfg['mtu'] +" --nqn " + nvmf_cfg['nqn'])

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("nvmeof_setup results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
