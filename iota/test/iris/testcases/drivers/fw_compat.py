#! /usr/bin/python3
import time
import pdb
import os
import json

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.utils.compat as compat
import iota.test.iris.config.netagent.hw_push_config as hw_config

OS_TYPE_LINUX = "linux"
OS_TYPE_BSD   = "freebsd"
OS_TYPE_ESX   = "esx"

def Setup(tc):
    api.Logger.info ("Fw compat test")
    if api.IsDryrun(): return api.types.status.SUCCESS

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    tc.skip = False
    if tc.os == OS_TYPE_BSD: # Not supportig BSD & ESXi right now
        tc.skip = True
        return api.types.status.SUCCESS

    # Intention to test locally built FW with target-version driver
    tc.target_version = getattr(tc.iterators, 'release', 'latest')

    if tc.target_version == 'latest':
        api.Logger.info('Target version is latest - nothing to change')
        return

    tc.resp = api.DownloadAssets(tc.args.asset_name, 
                                 parent_dir = os.path.join(api.GetTopDir(), 'images'),
                                 release_version = tc.target_version)
    if not api.IsApiResponseOk(tc.resp):
        api.Logger.error("Failed to download assets for %s" % tc.target_version)
        return tc.resp

    manifest_file = os.path.join(api.GetTopDir(), 'images', tc.target_version + '.json')

    # this is required to bring the testbed into operation state
    # after driver unload interfaces need to be initialized
    for node in tc.nodes:
        if compat.LoadFirmware(tc, node, manifest_file) == api.types.status.SUCCESS:
            api.Logger.info("Loaded %s Fw on %s" % (tc.target_version, node))
        else:
            return api.types.status.FAILURE

    if tc.os == OS_TYPE_LINUX:
        for node in tc.nodes:
            # this is required to bring the testbed into operation state
            # after driver unload interfaces need to be initialized
            hw_config.ReAddWorkloads(node)

    if getattr(tc.args, 'type', 'local_only') == 'local_only': 
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else: 
        tc.workload_pairs = api.GetRemoteWorkloadPairs() 

    if len(tc.workload_pairs) == 0: 
        api.Logger.info("Skipping ping part of testcase due to no workload pairs.") 
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    req = None
    interval = "0.2"
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        interval = "3"
    tc.cmd_cookies = []

    if tc.os == OS_TYPE_LINUX:
        for node in tc.nodes:
            cmd_cookie = "Driver-FW-Version"
            cmd = "/naples/nodeinit.sh --version"
            api.Trigger_AddHostCommand(req, node, cmd)
            tc.cmd_cookies.append(cmd_cookie)

    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if tc.args.ipaf == 'ipv6':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping6 -i %s -c 20 -s %d %s" % (interval, tc.args.pktsize, w2.ipv6_address))
        else:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                         (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                                   "ping -i %s -c 20 -s %d %s" % (interval, tc.args.pktsize, w2.ip_address))
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            if tc.cmd_cookies[cookie_idx] == "Driver-FW-Version":
                continue # ifnore failure for now
            if api.GetNicMode() == 'hostpin' and tc.args.pktsize > 1024:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    if tc.skip: return api.types.status.SUCCESS

    if not tc.fw_changed:
        return api.types.status.SUCCESS

    # Restore the workspace and testbed to continue
    manifest_file = os.path.join(api.GetTopDir(), 'images', 'latest.json')

    # this is required to bring the testbed into operation state
    # after driver unload interfaces need to be initialized
    for node in tc.nodes:
        if compat.LoadFirmware(tc, node, manifest_file) == api.types.status.SUCCESS:
            api.Logger.info("Loaded latest Fw on %s" % node)
        else:
            api.Logger.error("Failed to load latest Fw on %s" % node)
            return api.types.status.FAILURE

    if tc.os == OS_TYPE_LINUX:
        for node in tc.nodes:
            # this is required to bring the testbed into operation state
            # after driver unload interfaces need to be initialized
            hw_config.ReAddWorkloads(node)

    return api.types.status.SUCCESS
