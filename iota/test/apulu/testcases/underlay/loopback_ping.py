#! /usr/bin/python3
import time

import ipaddress
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import apollo.config.objects.device as device
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):

    req = None
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tc.cmd_cookies = []
    interval = "0.01"

    naplesHosts = api.GetNaplesHostnames()
    for node1 in naplesHosts:
        for node2 in naplesHosts:
            if node1 == node2:
                continue

            objs = device.client.Objects(node1)
            device1 = next(iter(objs))
            objs = device.client.Objects(node2)
            device2 = next(iter(objs))
            cmd_cookie = "%s --> %s" %\
                         (device1.IP, device2.IP)
            api.Trigger_AddNaplesCommand(req, node1, \
                                         "ping -i %s -c 20 -s %d %s" % \
                                         (interval, tc.iterators.pktsize, \
                                          device2.IP))
            api.Logger.info("Loopback ping test from %s" % (cmd_cookie))
            tc.cmd_cookies.append(cmd_cookie)

        tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
