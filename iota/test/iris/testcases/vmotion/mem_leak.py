#! /usr/bin/python3
import os
import time
import threading
import iota.harness.api as api
from iota.test.iris.testcases.alg.alg_utils import *
import iota.test.iris.testcases.vmotion.vm_utils as vm_utils 
import pdb

def Setup(tc):
    tc.Nodes    = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []
    tc.memleak     = 0

    memreq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    api.Logger.info("Starting MemLeak test after vMotion suite")

    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(memreq, node, "/nic/bin/halctl clear session")
        api.Trigger_AddNaplesCommand(memreq, node, "sleep 30", timeout=40)
        api.Trigger_AddNaplesCommand(memreq, node, "/nic/bin/halctl show system memory slab --yaml")
        api.Trigger_AddNaplesCommand(memreq, node, "/nic/bin/halctl show system memory mtrack --yaml")

    # Trigger the commands
    trig_resp = api.Trigger(memreq)

    cmd_cnt = 0
    for node in tc.Nodes:
        # Increment 2 for first two commands (clear && sleep)
        cmd_cnt += 2

        # Find any leak from memslab command output
        memslab_cmd = trig_resp.commands[cmd_cnt]
        api.PrintCommandResults(memslab_cmd)

        alg_meminfo = get_meminfo(memslab_cmd, 'alg')
        for info in alg_meminfo:
           if (info['inuse'] != 0 or info['allocs'] != info['frees']):
               tc.memleak = 1

        cmd_cnt += 1

        # Find any leak from mtrack command output
        memtrack_cmd = trig_resp.commands[cmd_cnt]
        api.PrintCommandResults(memtrack_cmd)

        # Alloc ID 90 & 91 is for VMOTION
        vm_memtrack = []
        vm_memtrack.append(vm_utils.get_memtrack(memtrack_cmd, 90))
        vm_memtrack.append(vm_utils.get_memtrack(memtrack_cmd, 91))

        # Subtract -4 in vMotion Init, 4 chunks of vMotion alloc will be done
        for info in vm_memtrack:
           if 'allocs' in info and 'frees' in info:
               if ((info['allocs'] - 4) != info['frees']):
                   tc.memleak = 1

        cmd_cnt += 1

    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    tc.resp   = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    if tc.memleak == 1:
       api.Logger.info("Memleak failure detected")
       return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    api.Logger.info("Results for vMotion Memleak test")
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
                result = api.types.status.FAILURE
        cookie_idx += 1       
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

