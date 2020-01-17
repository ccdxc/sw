#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify
import iota.test.utils.naples_host as utils
import ipaddress
import iota.harness.infra.store as store
import re

INTF_TEST_TYPE_OOB_1G       = "oob-1g"
INTF_TEST_TYPE_IB_100G      = "inb-100g"
INTF_TEST_TYPE_INT_MGMT     = "int-mgmt"
INTF_TEST_TYPE_HOST         = "host"

ip_prefix = 24

mgmtIp = api.GetPrimaryIntNicMgmtIp()
nextIp = api.GetPrimaryIntNicMgmtIpNext()

ip_map =  {
    INTF_TEST_TYPE_HOST: ("1.1.1.1", "1.1.1.2"),
    INTF_TEST_TYPE_OOB_1G: ("2.2.2.1", "2.2.2.2"),
    INTF_TEST_TYPE_INT_MGMT: (nextIp, mgmtIp),
    INTF_TEST_TYPE_IB_100G: ("4.4.4.1", "4.4.4.2"),
}

class InterfaceType:
    HOST               = 1
    HOST_INTERNAL      = 2
    NAPLES_INT_MGMT    = 3
    NAPLES_IB_100G     = 4
    NAPLES_OOB_1G      = 5

def GetHostInterfaces(node):
    intfs = []
    for intf in api.GetWorkloadNodeHostInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.HOST, api.GetNodeOs(node))
        intfs.append(intfObj)
    return intfs

def GetHostInternalMgmtInterfaces(node):
    intfs = []
    for intf in utils.GetHostInternalMgmtInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.HOST_INTERNAL, api.GetNodeOs(node))
        intfs.append(intfObj)
    return intfs

def GetNaplesInternalMgmtInterfaces(node):
    intfs = []
    for intf in utils.GetNaplesInternalMgmtInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.NAPLES_INT_MGMT, 'linux')
        intfs.append(intfObj)
    return intfs

def GetNaplesOobInterfaces(node):
    intfs = []
    for intf in utils.GetNaplesOobInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.NAPLES_OOB_1G, 'linux')
        intfs.append(intfObj)
    return intfs

def GetNaplesInbandInterfaces(node):
    intfs = []
    for intf in utils.GetNaplesInbandInterfaces(node):
        intfObj = Interface(node, intf, InterfaceType.NAPLES_IB_100G, 'linux')
        intfs.append(intfObj)
    return intfs



class Interface:
    __CMD_WRAPPER = {
        InterfaceType.HOST             : api.Trigger_AddHostCommand,
        InterfaceType.HOST_INTERNAL    : api.Trigger_AddHostCommand,
        InterfaceType.NAPLES_INT_MGMT  : api.Trigger_AddNaplesCommand,
        InterfaceType.NAPLES_IB_100G   : api.Trigger_AddNaplesCommand,
        InterfaceType.NAPLES_OOB_1G    : api.Trigger_AddNaplesCommand,
    }
    def __init__(self, node, name, intfType, os_type):
        self.__name = name
        self.__node = node
        self.__type = intfType
        self.__ip   = None
        self.__prefix = None
        self.__workload = None
        self.__os_type = os_type

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


class NodeInterface:
    def __init__(self, node):
        self._host_intfs             = GetHostInterfaces(node)
        self._host_int_mgmt_intfs    = GetHostInternalMgmtInterfaces(node)
        self._naples_int_mgmt_intfs  = GetNaplesInternalMgmtInterfaces(node)
        self._oob_1g_intfs           = GetNaplesOobInterfaces(node)
        self._ib_100g_intfs          = GetNaplesInbandInterfaces(node)

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

def __configure_interfaces(tc, tc_type):
    ip1 = ip_map[tc_type][0]
    ip2 = ip_map[tc_type][1]
   
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    if ipproto == 'v6':
        ip1 = ipaddress.IPv6Address('2002::' + ip1).compressed
        ip2 = ipaddress.IPv6Address('2002::' + ip2).compressed
        
        api.Logger.info("IPV6:", ip1)
        api.Logger.info("IPV6:", ip2)

    ret = tc.intf1.ConfigureInterface(ip1, ip_prefix, ipproto)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Configure interface failed")
        return api.types.status.FAILURE
    ret = tc.intf2.ConfigureInterface(ip2, ip_prefix, ipproto)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Configure interface failed")
        return api.types.status.FAILURE
    tc.intf1.SetIP(ip1)
    tc.intf2.SetIP(ip2)

    return api.types.status.SUCCESS

def __setup_host_interface_test(tc):
    host_intfs = tc.node_intfs[tc.nodes[0]].HostIntfs()
    host_intfs1 = tc.node_intfs[tc.nodes[1]].HostIntfs()
    tc.intf1 = host_intfs[0]
    tc.intf2 = host_intfs1[0]
    return __configure_interfaces(tc, INTF_TEST_TYPE_HOST)

def __setup_int_mgmt_interface_test(tc):
    host_intfs = tc.node_intfs[tc.nodes[0]].HostIntIntfs()
    naples_intfs = tc.node_intfs[tc.nodes[0]].NaplesIntMgmtIntfs()
    tc.intf1 = host_intfs[0]
    tc.intf2 = naples_intfs[0]
    return __configure_interfaces(tc, INTF_TEST_TYPE_INT_MGMT)

def __setup_oob_1g_interface_test(tc):
    intfs = tc.node_intfs[tc.nodes[0]].Oob1GIntfs()
    intfs1 = tc.node_intfs[tc.nodes[1]].Oob1GIntfs()
    tc.intf1 = intfs[0]
    tc.intf2 = intfs1[0]
    return __configure_interfaces(tc, INTF_TEST_TYPE_OOB_1G)

def __setup_inb_100g_inteface_test(tc):
    intfs = tc.node_intfs[tc.nodes[0]].Inb100GIntfs()
    intfs1 = tc.node_intfs[tc.nodes[1]].Inb100GIntfs()
    tc.intf1 = intfs[0]
    tc.intf2 = intfs1[1]
    return __configure_interfaces(tc, INTF_TEST_TYPE_IB_100G)

def ConfigureInterfaces(tc, test_type = INTF_TEST_TYPE_HOST):
    if test_type == INTF_TEST_TYPE_HOST:
        ret = __setup_host_interface_test(tc)
    elif test_type == INTF_TEST_TYPE_INT_MGMT:
        ret = __setup_int_mgmt_interface_test(tc)
    elif test_type == INTF_TEST_TYPE_OOB_1G:
        ret = __setup_oob_1g_interface_test(tc)
    elif test_type == INTF_TEST_TYPE_IB_100G:
        ret = __setup_inb_100g_inteface_test(tc)
    else:
        api.Logger.error("Invalid intf test type : ", test_type)
        return api.types.status.FAILURE

    tc.test_intfs = [tc.intf1, tc.intf2]

    return ret

def RestoreIntMmgmtInterfaceConfig():
    nodes = api.GetNaplesHostnames()
    node_intfs = {}
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    mgmtIp = store.GetPrimaryIntNicMgmtIp()
    nextIp = api.GetPrimaryIntNicMgmtIpNext()
    for node in nodes:
        node_if_info = GetNodeInterface(node)
        for intf in node_if_info.HostIntIntfs():
            api.Trigger_AddHostCommand(req, node, "ifconfig %s " + nextIp + "/24" % intf.Name())
    resp = api.Trigger(req)
    if resp == None:
        api.Abort()
    return api.types.status.FAILURE
