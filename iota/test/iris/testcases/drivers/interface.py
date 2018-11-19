#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify


INTF_TEST_TYPE_OOB_1G       = "oob-1g"
INTF_TEST_TYPE_IB_100G      = "ib-100g"
INTF_TEST_TYPE_INT_MGMT     = "int-mgmt"
INTF_TEST_TYPE_HOST         = "host"

ip_prefix = 24

ip_map =  {
    INTF_TEST_TYPE_HOST     : ("1.2.2.2", "1.2.2.3"),
    INTF_TEST_TYPE_OOB_1G   : ("2.2.2.2", "2.2.2.3"),
    INTF_TEST_TYPE_INT_MGMT : ("2.2.2.2", "2.2.2.3"),
    INTF_TEST_TYPE_IB_100G  : ("2.2.2.2", "2.2.2.3"),
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
        intfObj = Interface(node, intf, InterfaceType.HOST)
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
    def __init__(self, node, name, intfType):
        self.__name = name
        self.__node = node
        self.__type = intfType
        self.__ip   = None
        self.__prefix = None
        self.__workload = None

    def Name(self):
        return self.__name

    def IntfType(self):
        return self.__type

    def SetIP(self, ip):
        self.__ip = ip

    def GetIP(self):
        return self.__ip

    def Node(self):
        return self.__node

    def AddCommand(self, req, cmd, background = False):
        Interface.__CMD_WRAPPER[self.__type](req, self.__node, cmd, background = background)

    def ConfigureInterface(self, ip, netmask = 24):
        self.__ip = ip
        self.__prefix = str(netmask)
        return self.ReconfigureInterface()

    def ReconfigureInterface(self):
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        ip = self.GetIP() + "/" + self.__prefix
        ifconfig_cmd = "ifconfig " + self.Name() + " " + ip
        self.AddCommand(req, ifconfig_cmd)
        trig_resp = api.Trigger(req)
        for cmd in trig_resp.commands:
            if cmd.exit_code != 0:
                return api.types.status.FAILURE


        return api.types.status.SUCCESS


class NodeInterface:
    def __init__(self, node):
        self._host_intfs             = GetHostInterfaces(node)
        self._host_int_mgmt_intfs    = []
        self._naples_int_mgmt_intfs  = []
        self._oob_1g_intfs           = []
        self._ib_100g_intfs          = []

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
    ret = tc.intf1.ConfigureInterface(ip1, ip_prefix)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Configure interface failed")
        return api.types.status.FAILURE
    ret = tc.intf2.ConfigureInterface(ip2, ip_prefix)
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
    tc.intf2 = naples_intfs[1]
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
    return __configure_interfaces(tc, INTF_TEST_TYPE_OOB_1G)

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
