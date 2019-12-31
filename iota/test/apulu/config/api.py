#! /usr/bin/python3
import pdb
import os
import json
import ipaddress
from collections import defaultdict


import iota.test.apulu.config.init as init

#Following come from dol/infra
import infra.common.defs as defs
import infra.common.utils as utils
import infra.common.parser as parser
import infra.common.timeprofiler as timeprofiler

import iota.test.apulu.config.agent.api as agentapi
import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as agentapi

#import apulu.config.objects.batch as batch
import iota.test.apulu.config.objects.device as device
import iota.test.apulu.config.objects.vpc as vpc
import iota.test.apulu.config.objects.route as route
import iota.test.apulu.config.objects.policy as policy
import iota.test.apulu.config.objects.subnet as subnet
import iota.test.apulu.config.objects.vnic as vnic
import iota.test.apulu.config.objects.tunnel as tunnel
import iota.test.apulu.config.objects.lmapping as lmapping
#import apulu.config.objects.mirror as mirror
from infra.common.logging import logger as logger

import iota.harness.api as api
from iota.test.apulu.config.store import Store
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions


WORKLOAD_PAIR_TYPE_LOCAL_ONLY    = 1
WORKLOAD_PAIR_TYPE_REMOTE_ONLY   = 2
WORKLOAD_PAIR_TYPE_ANY           = 3

class Endpoint:
    def __init__(self, vnic_inst, ip_addresses):
        self.name = vnic_inst.GID()
        self.macaddr = vnic_inst.MACAddr.get()
        self.vlan = vnic_inst.VlanId
        self.ip_addresses = ip_addresses
        self.node_name = vnic_inst.Node

def __get_vnic_ip_address(vnic_addresses, iptype):
    for ipaddr in vnic_addresses:
        ippfx = ipaddress.ip_interface(ipaddr).network
        if isinstance(ippfx, iptype):
            return ipaddr
    return None

def __get_vnic_addresses(vnic_addresses):
    ip_addresses = []
    ip_addresses.append(__get_vnic_ip_address(vnic_addresses, ipaddress.IPv4Network))
    ip_addresses.append(__get_vnic_ip_address(vnic_addresses, ipaddress.IPv6Network))
    return ip_addresses

def GetEndpoints():
    naplesHosts = api.GetNaplesHostnames()
    eps = []
    for node in naplesHosts:
        vnics = vnic.client.Objects(node)
        for vnic_inst in vnics:
            vnic_addresses = lmapping.client.GetVnicAddresses(vnic_inst)
            ip_addresses = __get_vnic_addresses(vnic_addresses)
            ep = Endpoint(vnic_inst, ip_addresses)
            eps.append(ep)

    return eps

def __findWorkloadsByIP(ip):
    wloads = api.GetWorkloads()
    for wload in wloads:
        if wload.ip_address == ip or\
           wload.ipv6_address == ip:
            return wload
    api.Logger.error("Workload {} not found".format(ip))
    return None

def __vnics_in_same_segment(vnic1, vnic2):
    vnic1_ip_addresses = lmapping.client.GetVnicAddresses(vnic1)
    for vnic1_ip in vnic1_ip_addresses:
        vnic2_ip_addresses = lmapping.client.GetVnicAddresses(vnic2)
        for vnic2_ip in vnic2_ip_addresses:
            if ipaddress.ip_interface(vnic1_ip).network == ipaddress.ip_interface(vnic2_ip).network:
                return True

    return False


def __findWorkloadByVnic(vnic_inst):
    wloads = api.GetWorkloads()
    vnic1_ip_addresses = lmapping.client.GetVnicAddresses(vnic_inst)
    for wload in wloads:
        if wload.ip_prefix in vnic1_ip_addresses:
            return wload
        elif wload.ipv6_prefix in vnic1_ip_addresses:
            return wload
    return None

def __getWorkloadPairsBy(wl_pair_type):
    wl_pairs = []
    naplesHosts = api.GetNaplesHostnames()
    vnics = []
    for node in naplesHosts:
        vnics.extend(vnic.client.Objects(node))

    for vnic1 in vnics:
        api.Logger.error(" vnic1 ", vnic1)
        for vnic2 in vnics:
            if vnic1 == vnic2:
                continue
            if not __vnics_in_same_segment(vnic1, vnic2):
                continue

            w1 = __findWorkloadByVnic(vnic1)
            w2 = __findWorkloadByVnic(vnic2)
            vnic1.Show()
            vnic2.Show()
            assert(w1 and w2)
            if wl_pair_type == WORKLOAD_PAIR_TYPE_LOCAL_ONLY and vnic1.Node != vnic2.Node:
                continue
            elif wl_pair_type == WORKLOAD_PAIR_TYPE_REMOTE_ONLY and vnic1.Node == vnic2.Node:
                continue

            wl_pairs.append((w1, w2))

    return wl_pairs



def GetPingableWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy(wl_pair_type=wl_pair_type)
