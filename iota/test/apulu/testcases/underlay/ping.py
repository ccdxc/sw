#! /usr/bin/python3
import time

import ipaddress
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import apollo.config.objects.metaswitch.bgp_peer as bgp_peer
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.flow as flow_utils
from iota.test.apulu.utils.portflap import *
from iota.test.apulu.utils.hostflap import *
import apollo.config.objects.device as device
import apollo.config.utils as utils

def Setup(tc): 
    #naplesHosts = api.GetNaplesHostnames()
    #setDataPortState(naplesHosts, "up")
    api.Logger.info("Setup Complete")
    return api.types.status.SUCCESS

def Trigger(tc):

    req = None
    req = api.Trigger_CreateExecuteCommandsRequest()
    tc.cmd_cookies = []
    ping_count = getattr(tc.args, "ping_count", 20)
    interval = getattr(tc.args, "ping_interval", 0.01)
    connectivity = tc.iterators.connectivity
    naplesHosts = api.GetNaplesHostnames()
    tc_intf = (tc.iterators.interface).capitalize()

    if tc_intf in ['Uplink0', 'Uplink1', 'Uplinks']:
       setDataPortStatePerUplink(naplesHosts, tc.iterators.port_status, tc_intf)   
    if tc_intf in ['Switchport0', 'Switchport1', 'Switchports']:
        switchPortOp(naplesHosts, tc.iterators.port_status, tc_intf)
    
    if connectivity == 'bgp_peer': 
        for node in naplesHosts:
            for bgppeer in bgp_peer.client.Objects(node):
                cmd_cookie = "%s --> %s" %\
                             (str(bgppeer.LocalAddr), str(bgppeer.PeerAddr))
                api.Trigger_AddNaplesCommand(req, node, \
                                     "ping -i %f -c %d -s %d %s" % \
                                     (interval, ping_count, tc.iterators.pktsize, \
                                             str(bgppeer.PeerAddr)))
                api.Logger.info("Ping test from %s" % (cmd_cookie))
                tc.cmd_cookies.append(cmd_cookie)

            tc.resp = api.Trigger(req)

    else:
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
                                       "ping -i %f -c %d -s %d %s" % \
                                        (interval, ping_count, tc.iterators.pktsize, \
                                              device2.IP))
                api.Logger.info("Loopback ping test from %s" % (cmd_cookie))
                tc.cmd_cookies.append(cmd_cookie)

            tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    
    tc_intf = (tc.iterators.interface).capitalize()
    intersection_exists = 0
    all_uplink_down = 0
    ip_list = [] #impacted underlay ip list
    naplesHosts = api.GetNaplesHostnames()
    if tc.iterators.port_status == 'down' and\
        tc_intf in ['Uplink0', 'Uplink1', 'Switchport0', 'Switchport1']:
        for node in naplesHosts:
            if tc_intf in ['Switchport0', 'Switchport1']:
                shut_if = getFirstOperDownPort(node)
            else:
                shut_if = tc_intf
            ip = utils.GetNodeUnderlayIp(node, shut_if)
            ip_list.append(ip)

    if tc.iterators.port_status == 'down' and tc_intf in ['Uplinks', 'Switchports']:
        all_uplink_down = 1
        for node in naplesHosts:
            ip = utils.GetNodeUnderlayIp(node, "Uplink0")
            ip_list.append(ip)
            ip = utils.GetNodeUnderlayIp(node, "Uplink1")
            ip_list.append(ip)
    
    result = api.types.status.SUCCESS
    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        ip_str = tc.cmd_cookies[cookie_idx]
        ip_pair = ip_str.split(" --> ")
        ip_pair[0] = ipaddress.ip_address(ip_pair[0])
        ip_pair[1] = ipaddress.ip_address(ip_pair[1])
        if (set(ip_pair) & set(ip_list)):
            intersection_exists = 1
        else:
            intersection_exists = 0

        api.Logger.info(f"UNDERLAY_PING:ip pair {ip_pair} impacted ip {ip_list} intersect {intersection_exists}")
        if cmd.exit_code != 0 and not intersection_exists and not all_uplink_down == 1:
            api.Logger.info(f"UNDERLAY_PING: PING FAILURE {cmd.exit_code}")
            result = api.types.status.FAILURE
        if cmd.exit_code == 0 and (intersection_exists == 1):
            api.Logger.info(f"UNDERLAY_PING: UNEXPECTED PING SUCCESS {cmd.exit_code}")
            result = api.types.status.FAILURE
        
        cookie_idx += 1
    return result

def Teardown(tc):
    naples_nodes = api.GetNaplesHostnames()
    setDataPortState(naples_nodes, adminUp)
    switchPortOp(naples_nodes, 'up', 'Switchports') 
    ret = verifyDataPortState(naples_nodes, adminUp, operUp)
    api.Logger.info("Reset post Teardown Complete")
    return ret
