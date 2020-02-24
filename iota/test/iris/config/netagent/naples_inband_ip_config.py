#! /usr/bin/python3
import json
import time
import datetime
import ipaddress as ip
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common

def Main(step):
    res = api.types.status.SUCCESS
    workloads = api.GetLocalWorkloadPairs(True)
    for wl in workloads:
        w1 = wl[0]
        w2 = wl[1]

        if (w1.uplink_vlan != 0):
            continue

        ip_mask = w1.ip_prefix.split('/')[1]
        ip_addr = ip.IPv4Interface(w1.ip_prefix)

        #For inband collector config, agent always resolves thru first ip addr on the inband mgmt i/f.
        #so forcing to use 10.x.x.x network as inband mgmt ip for baremetal workloads
        if api.IsBareMetalWorkloadType(w1.node_name):
            if w1.ip_address != "10.255.0.1":
                continue

        #chose +100 as the IP address for host in-band mgmt ip
        new_naples_inband_ip = ip.IPv4Interface(ip_addr.ip+100)
        new_controller_ip = ip.IPv4Interface(ip_addr.ip+101)

        new_naples_inband_ip_str =str(new_naples_inband_ip.ip)+"/"+str(ip_mask)

        node = w1.node_name
        api.Logger.info("Configuring Naples Inband IP {}".format(new_naples_inband_ip_str))
        ret = common.SetNaplesModeInband_Static(node, str(new_controller_ip.ip), new_naples_inband_ip_str)
        if ret == None:
            api.Logger.info("Failed to set Naples Inband mode IP: {}".format(new_naples_inband_ip_str))
            res = api.types.status.FAILURE
        #api.Logger.info("Naples {}".format(common.PenctlGetNaplesMgtmIp(node)))
        break

    return res
