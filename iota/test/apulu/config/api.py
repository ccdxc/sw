#! /usr/bin/python3
import ipaddress
import random
#Following come from dol/infra
from apollo.config.generator import ObjectInfo as ObjClient
import apollo.config.objects.device as device
import apollo.config.objects.vpc as vpc
import apollo.config.objects.subnet as subnet
import apollo.config.objects.vnic as vnic
import apollo.config.objects.nexthop as nh
import apollo.config.objects.nexthop_group as nh_group
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.tunnel as tunnel
import apollo.config.objects.interface as interface
import apollo.config.objects.route as route

import iota.harness.api as api


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

def GetObjClient(objname):
    return ObjClient[objname]

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
        for vnic2 in vnics:
            if vnic1 == vnic2:
                continue
            if not __vnics_in_same_segment(vnic1, vnic2):
                continue

            w1 = __findWorkloadByVnic(vnic1)
            w2 = __findWorkloadByVnic(vnic2)
            assert(w1 and w2)
            if wl_pair_type == WORKLOAD_PAIR_TYPE_LOCAL_ONLY and vnic1.Node != vnic2.Node:
                continue
            elif wl_pair_type == WORKLOAD_PAIR_TYPE_REMOTE_ONLY and vnic1.Node == vnic2.Node:
                continue

            wl_pairs.append((w1, w2))

    return wl_pairs

def GetPingableWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy(wl_pair_type=wl_pair_type)

def __getObjects(objtype):
    objs = list()
    naplesHosts = api.GetNaplesHostnames()
    for node in naplesHosts:
        if objtype == 'vnic':
            objs.extend(vnic.client.Objects(node))
        elif objtype == 'vpc':
            objs.extend(vpc.client.Objects(node))
        elif objtype == 'subnet':
            objs.extend(subnet.client.Objects(node))
        elif objtype == 'nexthop':
            objs.extend(nh.client.Objects(node))
        elif objtype == 'tunnel':
            objs.extend(tunnel.client.Objects(node))
        elif objtype == 'nh-group':
            objs.extend(nh_group.client.Objects(node))
        elif objtype == 'lmapping':
            objs.extend(lmapping.client.Objects(node))
        elif objtype == 'rmapping':
            objs.extend(rmapping.client.Objects(node))
        elif objtype == 'interface':
            objs.extend(interface.client.Objects(node))
        elif objtype == 'route':
            objs.extend(route.client.Objects(node))
    return objs

def __getMaxLimit(objtype):
    objs = __getObjects(objtype)
    return len(objs)

def __getRandomSamples(objlist, num = None):
    limit = len(objlist)
    if num is None or num is 0 or limit < num:
        num = limit
    return random.sample(objlist, k=num)

def SetupConfigObjects(oper, objtype):
    select_objs = []
    if oper is None or ((oper != 'delete') and (oper != 'update')):
        return select_objs
    maxlimit = __getMaxLimit(objtype)
    select_count = int(maxlimit / 2) if maxlimit >= 2 else maxlimit
    objs = __getObjects(objtype)
    select_objs = __getRandomSamples(objs, select_count)
    for obj in select_objs:
        if oper == 'delete':
            obj.Delete()
            if obj.Read() is False:
                api.Logger.error(f"Deleting object failed for {obj}")
    return select_objs

def __findVnicObjectByWorkload(wl):
    vnics = vnic.client.Objects(wl.node_name)
    for vnic_ in vnics:
        if str(vnic_.MACAddr.get()) == wl.mac_address:
            return vnic_
    return None

def __getLocalMappingObjectByWorkload(workload):
    lmapping_ = []
    lmapping_.extend(__getObjects('lmapping'))
    for lmp in lmapping_:
        if lmp.AddrFamily == 'IPV4' and lmp.IP == str(workload.ip_address):
            return lmp
        if lmp.AddrFamily == 'IPV6' and lmp.IP == str(workload.ipv6_address):
            return lmp
    return None

def __getRemoteMappingObjectByWorkload(workload):
    rmaps = list()
    rmaps.extend(__getObjects('rmapping'))
    for rmap in rmaps:
        if rmap.AddrFamily == 'IPV4' and rmap.IP == str(workload.ip_address):
            return rmap
        if rmap.AddrFamily == 'IPV6' and rmap.IP == str(workload.ipv6_address):
            return rmap
    return None

def IsAnyConfigDeleted(workload_pair):
    w1 = workload_pair[0]
    w2 = workload_pair[1]

    vnic1 = __findVnicObjectByWorkload(w1)
    vnic2 = __findVnicObjectByWorkload(w2)
    vnics = [ vnic1, vnic2 ]
    for vnic_ in vnics:
        w1 = __findWorkloadByVnic(vnic_)
        nexthop_ = None
        nexthopgroup_ = None
        interface_ = None
        vpc_ = None
        v4routetable_ = None
        v6routetable_ = None
        subnet_ = getattr(vnic_, "SUBNET", None)
        if subnet_:
            vpc_ = getattr(vnic_.SUBNET, "VPC", None)
            v4_rtbid = getattr(subnet_, "V4RouteTableId", None)
            v6_rtbid = getattr(subnet_, "V6RouteTableId", None)
            v4routetable_ = route.client.GetRouteV4Table(w1.node_name, vpc_.VPCId, v4_rtbid)
            v6routetable_ = route.client.GetRouteV6Table(w1.node_name, vpc_.VPCId, v6_rtbid)
        lmapping_ = __getLocalMappingObjectByWorkload(w1)
        rmapping_ = __getRemoteMappingObjectByWorkload(w2)
        tunnel_ = getattr(rmapping_, 'TUNNEL', None)
        if tunnel_ is None:
            pass
        elif tunnel_.IsUnderlayEcmp():
            nexthopgroup_ = getattr(tunnel_, "NEXTHOPGROUP", None)
        elif tunnel_.IsUnderlay():
            nexthop_ = getattr(tunnel_, "NEXTHOP", None)
            interface_ = getattr(nexthop_, "L3Interface", None)
        device_ = getattr(tunnel_, "DEVICE", None)
        objs = [vpc_, subnet_, vnic_, lmapping_, v4routetable_, v6routetable_, device_]
        objs.extend([rmapping_, tunnel_, nexthop_, nexthopgroup_, interface_])
        values = list(map(lambda x: not(x.IsHwHabitant()), list(filter(None, objs))))
        if any(values):
            return True
        w2 = __findWorkloadByVnic(vnic_)
    return False

def ReadConfigObject(obj):
    return obj.Read()

def RestoreConfigObjects(oper, objlist):
    if oper is None or ((oper != 'delete') and (oper != 'update')):
        return True
    else:
        rs = True
        if oper == 'delete':
            for obj in objlist:
                obj.Create()
                if ReadConfigObject(obj) is False:
                    api.Logger.error(f"Read object failed for {obj}")
                    rs = False
    return rs


