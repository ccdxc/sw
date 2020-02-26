#! /usr/bin/python3
import ipaddress
import random
#Following come from dol/infra
from apollo.config.generator import ObjectInfo as ObjClient
from apollo.config.agent.api import ObjectTypes as APIObjTypes
import apollo.config.objects.vnic as vnic
import apollo.config.objects.metaswitch.bgp_peer as bgp_peer
import apollo.config.objects.lmapping as lmapping
import iota.harness.api as api


WORKLOAD_PAIR_TYPE_LOCAL_ONLY    = 1
WORKLOAD_PAIR_TYPE_REMOTE_ONLY   = 2
WORKLOAD_PAIR_TYPE_IGW_ONLY      = 3

class Endpoint:
    def __init__(self, vnic_inst, ip_addresses):
        self.name = vnic_inst.GID()
        self.macaddr = vnic_inst.MACAddr.get()
        self.vlan = vnic_inst.VlanId
        self.ip_addresses = ip_addresses
        self.node_name = vnic_inst.Node
        self.interface = GetObjClient('interface').GetHostInterfaceName(vnic_inst.Node, vnic_inst.SUBNET.HostIfIdx)

class VnicRoute:
    def __init__(self, vnic_inst, ip_addresses):
        self.routes = vnic_inst.RemoteRoutes
        self.gw = vnic_inst.SUBNET.VirtualRouterIPAddr[1]
        self.node_name = vnic_inst.Node
        if ip_addresses:
            self.vnic_ip = ip_addresses[0]
        else:
            self.vnic_ip = None

def GetEndpoints():
    naplesHosts = api.GetNaplesHostnames()
    eps = []
    for node in naplesHosts:
        vnics = vnic.client.Objects(node)
        for vnic_inst in vnics:
            vnic_addresses = lmapping.client.GetVnicAddresses(vnic_inst)
            ep = Endpoint(vnic_inst, vnic_addresses)
            eps.append(ep)

    return eps

def GetVnicRoutes():
    naplesHosts = api.GetNaplesHostnames()
    vnic_routes = []
    for node in naplesHosts:
        vnics = vnic.client.Objects(node)
        for vnic_inst in vnics:
            vnic_addresses = lmapping.client.GetVnicAddresses(vnic_inst)
            if vnic_inst.RemoteRoutes:
                route = VnicRoute(vnic_inst, vnic_addresses)
                vnic_routes.append(route)
    return vnic_routes


def GetObjClient(objname):
    return ObjClient[objname]

def __vnics_in_same_segment(vnic1, vnic2):
    if vnic1.SUBNET.GID() == vnic2.SUBNET.GID():
        return True
    return False

def __vnics_are_local_to_igw_pair(vnic1, vnic2):
    if vnic1.Node == vnic2.Node:
        return False
    if vnic1.LocalVnic == True and vnic2.IgwVnic == True:
        return True
    return False


def __findWorkloadByVnic(vnic_inst):
    wloads = api.GetWorkloads()
    vnic_mac = vnic_inst.MACAddr.get()
    for wload in wloads:
        if wload.mac_address == vnic_mac:
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
            if wl_pair_type == WORKLOAD_PAIR_TYPE_LOCAL_ONLY and vnic1.Node != vnic2.Node:
                continue
            elif wl_pair_type == WORKLOAD_PAIR_TYPE_REMOTE_ONLY and vnic1.Node == vnic2.Node:
                continue
            elif wl_pair_type == WORKLOAD_PAIR_TYPE_IGW_ONLY and not __vnics_are_local_to_igw_pair(vnic1, vnic2):
                continue

            find_in_same_segment = \
                    (wl_pair_type == WORKLOAD_PAIR_TYPE_LOCAL_ONLY or \
                    wl_pair_type == WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
            # TODO: do we need this here?
            if find_in_same_segment and not __vnics_in_same_segment(vnic1, vnic2):
                continue

            w1 = __findWorkloadByVnic(vnic1)
            w2 = __findWorkloadByVnic(vnic2)
            assert(w1 and w2)
            wl_pairs.append((w1, w2))

    return wl_pairs

def GetPingableWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_REMOTE_ONLY):
    return __getWorkloadPairsBy(wl_pair_type=wl_pair_type)

def __getObjects(objtype):
    objs = list()
    naplesHosts = api.GetNaplesHostnames()
    objClient = GetObjClient(objtype)
    for node in naplesHosts:
        if objtype == 'nexthop':
            objs.extend(objClient.GetUnderlayNexthops(node))
        else:
            objs.extend(objClient.Objects(node))
    return objs

def __getMaxLimit(objtype):
    objs = __getObjects(objtype)
    return len(objs)

def __getRandomSamples(objlist, num = None):
    limit = len(objlist)
    if num == None or num == 0 or limit < num:
        num = limit
    return random.sample(objlist, k=num)

def SetupConfigObjects(objtype):
    maxlimit = __getMaxLimit(objtype)
    select_count = int(maxlimit / 2) if maxlimit >= 2 else maxlimit
    select_objs = __getRandomSamples(__getObjects(objtype), select_count)
    api.Logger.info(f"selected_objs: {select_objs}")
    return select_objs

def ProcessObjectsByOperation(oper, select_objs):
    supported_ops = [ 'Create', 'Read', 'Delete', 'Update' ]
    res = api.types.status.SUCCESS
    if oper is None or oper not in supported_ops:
        return res
    for obj in select_objs:
        getattr(obj, oper)()
        if not getattr(obj, 'Read')():
            api.Logger.error(f"{oper} failed for object: {obj}")
            res = api.types.status.FAILURE
    return res

def __findVnicObjectByWorkload(wl):
    vnics = vnic.client.Objects(wl.node_name)
    for vnic_ in vnics:
        if vnic_.MACAddr.get() == wl.mac_address:
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
            v4routetable_ = subnet_.V4RouteTable
            v6routetable_ = subnet_.V6RouteTable
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
        objs = [vpc_, subnet_, vnic_, lmapping_, v4routetable_, v6routetable_ ]
        objs.extend([rmapping_, tunnel_, nexthop_, nexthopgroup_, interface_])
        objs.extend(GetObjClient('device').Objects(w1.node_name))
        values = list(map(lambda x: not(x.IsHwHabitant()), list(filter(None, objs))))
        if any(values):
            return True
        w2 = __findWorkloadByVnic(vnic_)
    return False

def ReadConfigObject(obj):
    return obj.Read()

def RestoreObjects(oper, objlist):
    supported_ops = [ 'Delete', 'Update' ]
    if oper is None or oper not in supported_ops:
        return True
    else:
        rs = True
        if oper == 'Delete':
            for obj in objlist:
                obj.Create()
                if ReadConfigObject(obj) is False:
                    api.Logger.error(f"RestoreObjects:Read object failed for {obj} after {oper} operation")
                    rs = False
        elif oper == 'Update':
            for obj in objlist:
                obj.RollbackUpdate()
                if ReadConfigObject(obj) is False:
                    api.Logger.error(f"RestoreObjects:Read object failed for {obj} after {oper} operation")
                    rs = False
    return rs

def GetPolicyObjectsByWorkload(wl):
    return GetObjClient(APIObjTypes.POLICY.name.lower()).Objects(wl.node_name)

def GetUnderlayWorkloadPairs():
    naplesHosts = api.GetNaplesHostnames()
    workloads = []
    bgppeers = []
    for node in naplesHosts:
        bgppeers = bgp_peer.client.Objects(node)
        #TODO - get workloads from bgp peer objects
        return workloads
