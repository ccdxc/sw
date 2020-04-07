#! /usr/bin/python3
import time
import iota.harness.api as api

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Trigger_AddHostCommand(req, tc.nodes[1], "fio --filename=/dev/{} --direct=1 --rw={} --bs={} --ioengine=libaio --iodepth={} --runtime={} --numjobs={} --time_based --group_reporting --name=throughput-test-job".format(tc.iterators.namespace, tc.iterators.command, tc.iterators.block_size, tc.iterators.io_depth, tc.iterators.runtime, tc.iterators.num_jobs), timeout=125)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    f = open('fio_dump.txt', 'a')
    api.Logger.info("nvme_fio results for the following nodes: {0}".format(tc.nodes))
    for cmd in tc.resp.commands:
        f.write(cmd.stdout)
        f.write(cmd.stderr)
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        f.close()

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
