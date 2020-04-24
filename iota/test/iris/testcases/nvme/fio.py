#! /usr/bin/python3
import time
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Trigger_AddHostCommand(req, tc.nodes[1], "mkdir -p /naples/fio_logs")
    api.Trigger_AddHostCommand(req, tc.nodes[1], "fio --filename=/dev/{} --rw={} --bs={} --iodepth={} --numjobs={} --write_bw_log=/naples/fio_logs/{}_{}_{}qd_{}j_bw.log --write_iops_log=/naples/fio_logs/{}_{}_{}qd_{}j_iops.log /naples/nvme.fio".format(tc.iterators.namespace, tc.iterators.command, tc.iterators.block_size, tc.iterators.io_depth, tc.iterators.num_jobs, tc.iterators.block_size, tc.iterators.command, tc.iterators.io_depth, tc.iterators.num_jobs, tc.iterators.block_size, tc.iterators.command, tc.iterators.io_depth, tc.iterators.num_jobs), timeout=125)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("nvme_fio results for the following nodes: {0}".format(tc.nodes))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
