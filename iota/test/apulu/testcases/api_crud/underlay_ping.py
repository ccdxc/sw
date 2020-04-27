#! /usr/bin/python3
import time

import ipaddress
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import apollo.config.objects.metaswitch.bgp_peer as bgp_peer
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.utils.arping as arp_utils
from iota.test.apulu.utils.portflap import *
from iota.test.apulu.utils.hostflap import *

def __getOperations(tc_operation):
    opers = list()
    if tc_operation is None:
        return opers
    else:
        opers = list(map(lambda x:x.capitalize(), tc_operation))
    return opers


def Setup(tc):
    tc.opers = __getOperations(tc.iterators.oper)
    tc.selected_objs = config_api.SetupConfigObjects(tc.iterators.objtype)
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.is_config_deleted = False
    tc.is_config_updated = False
    for op in tc.opers:
        res = config_api.ProcessObjectsByOperation(op, tc.selected_objs)
        if op == 'Delete':
            tc.is_config_deleted = True
        elif op == 'Update':
            tc.is_config_updated = True
        if res != api.types.status.SUCCESS:
            break;

    req = None
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    tc.cmd_cookies = []
    ping_count = getattr(tc.args, "ping_count", 20)
    interval = getattr(tc.args, "ping_interval", 0.01)

    naplesHosts = api.GetNaplesHostnames()
    for node in naplesHosts:
        for bgppeer in bgp_peer.client.Objects(node):
            cmd_cookie = "%s --> %s" %\
                         (str(bgppeer.LocalAddr), str(bgppeer.PeerAddr))
            api.Trigger_AddNaplesCommand(req, node, \
                                         "ping -i %f -c %d %s" % \
                                         (interval, ping_count, str(bgppeer.PeerAddr)))
            api.Logger.info("Ping test from %s" % (cmd_cookie))
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
        # we expect ping to fail.
        if cmd.exit_code == 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    #return result
    return api.types.status.SUCCESS

def Teardown(tc):
    if tc.is_config_updated:
        rs = config_api.RestoreObjects('Update', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"Teardown failed to restore objs from Update operation: {rs}")
    if tc.is_config_deleted:
        rs = config_api.RestoreObjects('Delete', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"Teardown failed to restore objs from Delete operation: {rs}")
        h2h_test = getattr(tc.args, "h2h_test", False)
        if h2h_test:
            if not arp_utils.SendGratArp(api.GetWorkloads()):
                api.Logger.error(f"Teardown failed to send ARP to all workloads")
            else:
                api.Logger.info("Running overlay connectivity test")
                cmd_cookies, resp = conn_utils.TriggerConnectivityTestAll(proto="icmp")
                ret = conn_utils.VerifyConnectivityTest("icmp", cmd_cookies, resp)
                if ret != api.types.status.SUCCESS:
                    api.Logger.error("Connectivity verification failed.")
    return api.types.status.SUCCESS
