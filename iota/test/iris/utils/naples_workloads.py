#! /usr/bin/python3

import iota.harness.api as api
import iota.harness.infra.resmgr as resmgr
import iota.harness.infra.store as store
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.naples_host as naples_host
import iota.test.iris.testcases.drivers.verify as verify
import iota.test.iris.config.netagent.hw_push_config as hw_config
import ipaddress
import re

ip_prefix = 24
mgmtIp = store.GetPrimaryIntNicMgmtIp()
nextIp = re.sub('\.1$','.2',mgmtIp)

IntMgmtIpAllocator = resmgr.IpAddressStep(nextIp, "0.0.0.1")
IntMgmtIpAllocator = resmgr.IpAddressStep("192.169.1.2", "0.0.0.1")
InbandIpAllocator = resmgr.IpAddressStep("192.170.1.2", "0.0.0.1")
OobIpAllocator = resmgr.IpAddressStep("192.171.1.2", "0.0.0.1")

class InterfaceType:
    HOST               = 1
    HOST_MGMT          = 2
    HOST_INTERNAL      = 3
    NAPLES_INT_MGMT    = 4
    NAPLES_IB_100G     = 5
    NAPLES_OOB_1G      = 6

def GetHostMgmtInterfaces(node):
    intfs = []
    intf = host_utils.GetHostMgmtInterface(node)
    api.Logger.debug("HostMgmtInterface for node:%s interface:%s " % (node, intf))
    intfObj = Interface(node, intf, InterfaceType.HOST_MGMT, api.GetNodeOs(node))
    intfs.append(intfObj)
    return intfs

def GetHostInterfaces(node):
    intfs = []
    for intf in api.GetWorkloadNodeHostInterfaces(node):
        api.Logger.debug("HostInterface for node:%s interface:%s " % (node, intf))
        intfObj = Interface(node, intf, InterfaceType.HOST, api.GetNodeOs(node))
        intfs.append(intfObj)
    return intfs

def GetHostInternalMgmtInterfaces(node):
    intfs = []
    for intf in naples_host.GetHostInternalMgmtInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.HOST_INTERNAL, api.GetNodeOs(node))
        intfs.append(intfObj)
    api.Logger.debug("HostInternalMgmtInterfaces for node: ", node, intfs)
    return intfs

def GetNaplesInternalMgmtInterfaces(node):
    if not api.IsNaplesNode(node):
        return []
    intfs = []
    for intf in naples_host.GetNaplesInternalMgmtInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.NAPLES_INT_MGMT, 'linux')
        intfs.append(intfObj)
    api.Logger.debug("NaplesInternalMgmtInterfaces for node: ", node, intfs)
    return intfs

def GetNaplesOobInterfaces(node):
    if not api.IsNaplesNode(node):
        return []
    intfs = []
    for intf in naples_host.GetNaplesOobInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.NAPLES_OOB_1G, 'linux')
        intfs.append(intfObj)
    api.Logger.debug("NaplesOobInterfaces for node: ", node, intfs)
    return intfs

def GetNaplesInbandInterfaces(node):
    if not api.IsNaplesNode(node):
        return []
    intfs = []
    for intf in naples_host.GetNaplesInbandInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.NAPLES_IB_100G, 'linux')
        intfs.append(intfObj)
    api.Logger.debug("NaplesInbandInterfaces for node: ", node, intfs)
    return intfs


class Interface:
    __CMD_WRAPPER = {
        InterfaceType.HOST             : api.Trigger_AddHostCommand,
        InterfaceType.HOST_MGMT        : api.Trigger_AddHostCommand,
        InterfaceType.HOST_INTERNAL    : api.Trigger_AddHostCommand,
        InterfaceType.NAPLES_INT_MGMT  : api.Trigger_AddNaplesCommand,
        InterfaceType.NAPLES_IB_100G   : api.Trigger_AddNaplesCommand,
        InterfaceType.NAPLES_OOB_1G    : api.Trigger_AddNaplesCommand,
    }

    __IP_CMD_WRAPPER = {
        InterfaceType.HOST             : host_utils.GetIPAddress,
        InterfaceType.HOST_MGMT        : host_utils.GetIPAddress,
        InterfaceType.HOST_INTERNAL    : host_utils.GetIPAddress,
        InterfaceType.NAPLES_INT_MGMT  : naples_host.GetIPAddress,
        InterfaceType.NAPLES_IB_100G   : naples_host.GetIPAddress,
        InterfaceType.NAPLES_OOB_1G    : naples_host.GetIPAddress,
    }

    def __init__(self, node, name, intfType, os_type):
        self.__name = name
        self.__node = node
        self.__type = intfType
        self.__ip   = None
        self.__prefix = None
        self.__workload = None
        self.__os_type = os_type
        self.__ip = self.GetConfiguredIP()

    def Name(self):
        return self.__name

    def IntfType(self):
        return self.__type

    def OsType(self):
        return self.__os_type

    def SetIP(self, ip):
        self.__ip = ip

    def GetIP(self):
        return self.__ip

    def Node(self):
        return self.__node

    def AddCommand(self, req, cmd, background = False):
        Interface.__CMD_WRAPPER[self.__type](req, self.__node, cmd, background = background)

    def ConfigureInterface(self, ip, netmask = 24, ipproto = 'v4'):
        self.__ip = ip
        if ipproto == 'v4':
            self.__prefix = '255.255.255.0'
        return self.ReconfigureInterface(ipproto)

    def ReconfigureInterface(self, ipproto):
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

        if ipproto == 'v6':
            ip = self.GetIP() + "/64"
            ifconfig_cmd = "ifconfig " + self.Name() + " " + "inet6 add " + ip
        else:
            ip = self.GetIP() + " netmask " + self.__prefix + " up"
            ifconfig_cmd = "ifconfig " + self.Name() + " " + ip
        api.Logger.info ("ifconfig: ", ifconfig_cmd)

        self.AddCommand(req, ifconfig_cmd)
        trig_resp = api.Trigger(req)
        for cmd in trig_resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0 and  "SIOCSIFADDR: File exists" not in cmd.stderr:
                return api.types.status.FAILURE

        return api.types.status.SUCCESS

    def GetConfiguredIP(self):
        return Interface.__IP_CMD_WRAPPER[self.__type](self.__node, self.__name)


class NodeInterface:
    def __init__(self, node):
        self._host_intfs             = GetHostInterfaces(node)
        self._host_mgmt_intfs        = GetHostMgmtInterfaces(node)
        self._host_int_mgmt_intfs    = GetHostInternalMgmtInterfaces(node)
        self._naples_int_mgmt_intfs  = GetNaplesInternalMgmtInterfaces(node)
        self._oob_1g_intfs           = GetNaplesOobInterfaces(node)
        self._ib_100g_intfs          = GetNaplesInbandInterfaces(node)

    def HostMgmtIntfs(self):
        return self._host_mgmt_intfs

    def HostIntfs(self):
        return self._host_intfs

    def HostIntIntfs(self):
        return self._host_int_mgmt_intfs

    def NaplesIntMgmtIntfs(self):
        return self._naples_int_mgmt_intfs

    def Oob1GIntfs(self):
        return self._oob_1g_intfs

    def Inb100GIntfs(self):
        return self._ib_100g_intfs

def GetNodeInterface(node):
    return NodeInterface(node)


__MGMT_WORKLOAD_TYPE = "mgmt"
__INT_MGMT_WORKLOAD_HOST_TYPE = "int-mgmt-host"
__INT_MGMT_WORKLOAD_NAPLES_TYPE = "int-mgmt-naples"
__IB_MGMT_WORKLOAD_TYPE = "inband"
__OOB_MGMT_WORKLOAD_TYPE = "oob"

class NaplesWorkload(store.Workload):
    def __init__(self, type, intf):
        self.__type = type
        self.__intf = intf
        workload_name = type + "-" + intf.Node() + "-" + intf.Name()
        self.init(workload_name, intf.Node(), intf.GetIP(), intf.Name())

    def GetType(self):
        return self.__type

    def AddCommand(self, req, cmd, background = False):
        return self.__intf.AddCommand(req, cmd, background)

def AddMgmtWorkloads(node_if_info):
    for intf in node_if_info.HostMgmtIntfs():
        ip = intf.GetIP()
        if not ip:
            api.Logger.error("No ipaddress found for interface ", intf.Name())
            return api.types.status.FAILURE
        wl = NaplesWorkload(__MGMT_WORKLOAD_TYPE, intf)
        wl.skip_node_push = True
        api.AddNaplesWorkload(wl.GetType(), wl)
    return api.types.status.SUCCESS


def AddNaplesWorkloads(node_if_info):
    for intf in node_if_info.HostIntIntfs():
        ip = intf.GetIP()
        if ip is None or ip == "":
            #IP not assigned
            ip = IntMgmtIpAllocator.Alloc()
            intf.ConfigureInterface(str(ip))
            ip = intf.GetIP()
        wl = NaplesWorkload(__INT_MGMT_WORKLOAD_HOST_TYPE, intf)
        api.AddNaplesWorkload(wl.GetType(),wl)

    for intf in node_if_info.NaplesIntMgmtIntfs():
        ip = intf.GetIP()
        if ip is None or ip == "":
            #Naples should have int mgmt IP assigned!
            assert(0)
        wl = NaplesWorkload(__INT_MGMT_WORKLOAD_NAPLES_TYPE, intf)
        wl.skip_node_push = True
        api.AddNaplesWorkload(wl.GetType(),wl)

    for intf in node_if_info.Oob1GIntfs():
        ip = intf.GetIP()
        if ip is None or ip == "":
            #IP not assigned
            ip = OobIpAllocator.Alloc()
            intf.ConfigureInterface(str(ip))
            ip = intf.GetIP()
        wl = NaplesWorkload(__OOB_MGMT_WORKLOAD_TYPE, intf)
        wl.skip_node_push = True
        api.AddNaplesWorkload(wl.GetType(),wl)

    for intf in node_if_info.Inb100GIntfs():
        ip = intf.GetIP()
        if ip is None or ip == "":
            #IP not assigned
            ip = InbandIpAllocator.Alloc()
            intf.ConfigureInterface(str(ip))
            ip = intf.GetIP()
        wl = NaplesWorkload(__IB_MGMT_WORKLOAD_TYPE, intf)
        wl.skip_node_push = True
        api.AddNaplesWorkload(wl.GetType(),wl)

def Main(tc):
    nodes = api.GetWorkloadNodeHostnames()
    for node in nodes:
        api.Logger.debug("Creating NodeInterface for node: %s" % node)
        node_if_info = GetNodeInterface(node)
        api.Logger.debug("Adding MgmtWorkloads for node: %s" % node)
        ret = AddMgmtWorkloads(node_if_info)
        if ret != api.types.status.SUCCESS:
            api.Logger.debug("Failed to add MgmtWorkloads for node: %s" % node)
            return api.types.status.FAILURE
        if api.IsNaplesNode(node):
            api.Logger.debug("Adding NaplesWorkloads for node: %s" % node)
            AddNaplesWorkloads(node_if_info)
    return api.types.status.SUCCESS


def __GetWorkloadPairs(type, remote=False):
    pairs = []
    for w1 in api.GetNaplesWorkloads(type):
        for w2 in api.GetNaplesWorkloads(type):
            if id(w1) == id(w2): continue
            if (not remote and w1.node_name == w2.node_name) or \
                (remote and w1.node_name != w2.node_name):
                pairs.append((w1, w2))
    return pairs

def __GetWorkloads(type):
    wloads = []
    for w1 in api.GetNaplesWorkloads(type):
        wloads.append(w1)
    return wloads


def GetMgmtWorkloadPairs():
    return __GetWorkloadPairs(__MGMT_WORKLOAD_TYPE, remote=True)

def GetIntMgmtHostWorkloadPairs():
    return __GetWorkloadPairs(__INT_MGMT_WORKLOAD_HOST_TYPE)

def GetIntMgmtNaplesWorkloadPairs():
    return __GetWorkloadPairs(__INT_MGMT_WORKLOAD_NAPLES_TYPE)

def GetInbandMgmtWorkloadPairs():
    return __GetWorkloadPairs(__IB_MGMT_WORKLOAD_TYPE)

def GetOobMgmtWorkloadPairs():
    return __GetWorkloadPairs(__OOB_MGMT_WORKLOAD_TYPE)

def GetIntMgmtHostWorkloads():
    return __GetWorkloads(__INT_MGMT_WORKLOAD_HOST_TYPE)

def GetIntMgmtNaplestWorkloads():
    return __GetWorkloads(__INT_MGMT_WORKLOAD_NAPLES_TYPE)

def GetInbandMgmtRemoteWorkloadPairs():
    return __GetWorkloadPairs(__IB_MGMT_WORKLOAD_TYPE, remote=True)

def GetOobMgmtRemoteWorkloadPairs():
    return __GetWorkloadPairs(__OOB_MGMT_WORKLOAD_TYPE, remote=True)
