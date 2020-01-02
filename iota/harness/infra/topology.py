#! /usr/bin/python3
import os
import pdb
import json
import sys
import re
import subprocess
import traceback

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.harness.infra.utils.console import Console

import iota.harness.infra.store as store
import iota.harness.infra.resmgr as resmgr
import iota.harness.infra.types as types
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.testcase as testcase

import iota.protos.pygen.topo_svc_pb2 as topo_pb2
import iota.protos.pygen.iota_types_pb2 as types_pb2

node_log_file = GlobalOptions.logdir + "/nodes.log"

def formatMac(mac: str) -> str:
    mac = re.sub('[.:-]', '', mac).lower()  # remove delimiters and convert to lower case
    mac = ''.join(mac.split())  # remove whitespaces
    mac = ".".join(["%s" % (mac[i:i+4]) for i in range(0, 12, 4)])
    return mac

def GetNodePersonalityByNicType(nic_type):
    if nic_type in ['pensando', 'naples']:
        return topo_pb2.PERSONALITY_NAPLES
    elif nic_type == 'mellanox':
        return topo_pb2.PERSONALITY_MELLANOX
    elif nic_type == 'broadcom':
        return topo_pb2.PERSONALITY_BROADCOM
    elif nic_type == 'intel':
        return topo_pb2.PERSONALITY_INTEL
    elif nic_type == 'pensando-sim':
        return topo_pb2.PERSONALITY_NAPLES_SIM
    else:
        return None


def GetNodeType(role):
    if role in ['PERSONALITY_NAPLES_SIM', 'PERSONALITY_VENICE']:
        return "vm"
    return "bm"

class Node(object):

    class PciInfo:
        def __init__(self, nic, bus, device, function):
            self.__nic = nic
            self.__bus = bus
            self.__device = device
            self.__function = function

        def GetInfo(self):
            return {'nic':self.__nic, 'bus':self.__bus, 'device':self.__device, 'function':self.__function}

    def __init__(self, spec):
        self.__spec = spec
        self.__name = spec.name
        self.__node_type = GetNodeType(spec.role)
        self.__inst = store.GetTestbed().AllocateInstance(self.__node_type)
        self.__role = self.__get_instance_role(spec.role)
        self.__node_id = getattr(self.__inst, "ID", 0)

        self.__ip_address = self.__inst.NodeMgmtIP
        self.__os = getattr(self.__inst, "NodeOs", "linux")
        self.__nic_mgmt_ip = getattr(self.__inst, "NicMgmtIP", None)
        #self.__nic_int_mgmt_ip = getattr(self.__inst, "NicIntMgmtIP", "169.254.0.1")
        self.__nic_int_mgmt_ip = getattr(self.__inst, "NicIntMgmtIP", api.GetPrimaryIntNicMgmtIp())
        self.__nic_console_ip = getattr(self.__inst, "NicConsoleIP", "")
        self.__nic_console_port = getattr(self.__inst, "NicConsolePort", "")
        self.__nic_mgmt_intf = getattr(self.__inst, "NicMgmtIntf", "oob_mnic0")
        self.__console_hdl = None
        self.__read_mgmt_ip_from_console()

        self.__nic_pci_info = {}
        self.__vmUser = getattr(self.__inst, "Username", "vm")
        self.__vmPassword = getattr(self.__inst, "Password", "vm")
        self.ssh_host = "%s@%s" % (self.__vmUser, self.__ip_address) 
        self.ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.__vmPassword
        self.__control_ip = resmgr.ControlIpAllocator.Alloc()
        self.__control_intf = "eth1"

        self.__data_intfs = [ "eth2", "eth3" ]
        self.__host_intfs = []
        self.__host_if_alloc_idx = 0
        self.__tb_params = store.GetTestbed().GetProvisionParams()
        if self.__tb_params:
            self.__esx_username = getattr(self.__tb_params, "EsxUsername", "")
            self.__esx_password = getattr(self.__tb_params, "EsxPassword", "")
        self.__esx_ctrl_vm_ip = getattr(self.__inst, "esx_ctrl_vm_ip", "")

        if self.IsWorkloadNode():
            osDetail = getattr(spec.workloads, self.__os,  None)
            if not os:
                Logger.error("Workload type for OS  : %s not found" % (self.__os))
                sys.exit(1)
            self.__workload_type = topo_pb2.WorkloadType.Value(osDetail.type)
            self.__workload_image = osDetail.image
            self.__ncpu  = int(getattr(osDetail, "ncpu", 1))
            self.__memory  = int(getattr(osDetail, "memory", 2))

            self.__concurrent_workloads = getattr(osDetail, "concurrent", sys.maxsize)
        Logger.info("- New Node: %s: %s (%s)" %\
                    (spec.name, self.__ip_address, topo_pb2.PersonalityType.Name(self.__role)))
        return

    def __get_instance_role(self, role):
        if role != 'auto':
            return topo_pb2.PersonalityType.Value(role)

        if getattr(self.__inst.Resource, "DataNicType", None):
            return topo_pb2.PERSONALITY_NAPLES_BITW

        nic_type = self.__inst.Resource.NICType

        role = GetNodePersonalityByNicType(nic_type)
        if role == None:
            os.system("cp /warmd.json '%s/iota/logs" % GlobalOptions.topdir)
            os.system("cat /warmd.json")
            Logger.error("Unknown NIC Type : %s %s" % (nic_type, role))
            sys.exit(1)
        return role

    def __read_mgmt_ip_from_console(self):
        if self.__nic_console_ip != "" and self.__nic_console_port != "":
            try:
                ip_read = False
                with open(node_log_file, 'r') as json_file:
                    data = json.load(json_file)
                    for node in data:
                        if node["NicConsoleIP"] == self.__nic_console_ip and \
                            node["NicConsolePort"] == self.__nic_console_port and \
                            node["NicMgmtIP"] not in [ "N/A", "" ]:
                            self.__nic_mgmt_ip = node["NicMgmtIP"]
                            ip_read = True
                if not ip_read:
                    raise
            except:
                    self.__console_hdl = Console(self.__nic_console_ip, self.__nic_console_port, disable_log=True)
                    output = self.__console_hdl.RunCmdGetOp("ifconfig " + self.__nic_mgmt_intf)
                    ifconfig_regexp = "addr:(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})"
                    x = re.findall(ifconfig_regexp, str(output))
                    if len(x) > 0:
                        Logger.info("Read management IP %s %s" % (self.__name, x[0]))
                        self.__nic_mgmt_ip = x[0]
        else:
            Logger.info("Skipping management IP read as no console info %s" % self.__name)

    def GetNicPciInfo(self,nic):
        if nic not in self.__nic_pci_info:
            raise Exception('nic {0} not found on node {1}'.format(nic, self.__name))
        return self.__nic_pci_info[nic].GetInfo()

    def DetermineNicPciInfo(self,nic):
        bus = 'na'
        device = 'na'
        function = 'na'
        cmd = ''
        reText = ''
        if self.__os == 'linux':
            cmd = "ethtool -i " + nic + " | awk -F ' ' '/bus-info/ { print $2}'"
            reText = '([\d]+):([\d]+):([\d]+\.([\d]+))'
        elif self.__os == 'freebsd':
            cmd = ''
            reText = ''
        if not cmd == '':
            try:
                Logger.debug('sending cmd to get pci info: {0}'.format(cmd))
                output = self.RunSshCmd(cmd)
                Logger.debug('cmd returned: {0}'.format(output))
                found = re.search(reText, output)
                if found:
                    domain,bus,device,function = found.groups()
            except:
                Logger.debug('failed to run determine pci info. error was: {0}'.format(traceback.format_exc()))
        Logger.debug('pci info for node {0}: nic={1}, bus={2}, device={3}, function={4}'.format(self.__name, nic, bus, device, function))
        self.__nic_pci_info[nic] = Node.PciInfo(nic=nic, bus=bus, device=device, function=function)
        return self.__nic_pci_info[nic]

    def GetNicType(self):
        if getattr(self.__inst, "Resource", None):
            if getattr(self.__inst.Resource, "DataNicType", None):
                return self.__inst.Resource.DataNicType
            return getattr(self.__inst.Resource, "NICType", "")
        #Simenv does not have nic type
        return ""

    def GetNetwork(self):
        if getattr(self.__inst, "Resource", None):
            return getattr(self.__inst.Resource, "Network", None)
        #Simenv does not have nic type
        return ""

    def GetDataNetworks(self):
        return self.__inst.DataNetworks

    def GetNicMgmtIP(self):
        return self.__nic_mgmt_ip

    def GetNicIntMgmtIP(self):
        return self.__nic_int_mgmt_ip

    def GetNicConsoleIP(self):
        return self.__nic_console_ip

    def GetNicConsolePort(self):
        return self.__nic_console_port

    def Name(self):
        return self.__name
    def Role(self):
        return self.__role
    def GetOs(self):
        return self.__os
    def IsVenice(self):
        return self.__role == topo_pb2.PERSONALITY_VENICE
    def IsNaplesSim(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES_SIM
    def IsNaplesMultiSim(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES_MULTI_SIM
    def IsNaplesHw(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES
    def IsNaplesHwWithBumpInTheWire(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES_BITW
    def IsNaplesHwWithBumpInTheWirePerf(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES_BITW_PERF
    def IsNaplesCloudPipeline(self):
        return GlobalOptions.pipeline in [ "apulu" ]

    def IsNaples(self):
        return self.IsNaplesSim() or self.IsNaplesHw() or self.IsNaplesHwWithBumpInTheWire()

    def IsMellanox(self):
        return self.__role == topo_pb2.PERSONALITY_MELLANOX
    def IsBroadcom(self):
        return self.__role == topo_pb2.PERSONALITY_BROADCOM
    def IsIntel(self):
        return self.__role == topo_pb2.PERSONALITY_INTEL
    def IsThirdParty(self):
        return self.IsMellanox() or self.IsBroadcom() or self.IsIntel()
    def IsWorkloadNode(self):
        return self.__role != topo_pb2.PERSONALITY_VENICE

    def UUID(self):
        if self.IsThirdParty():
            return self.Name()
        return self.__uuid

    def HostInterfaces(self):
        return self.__host_intfs

    def AllocateHostInterface(self):
        host_if = self.__host_intfs[self.__host_if_alloc_idx]
        self.__host_if_alloc_idx = (self.__host_if_alloc_idx + 1) % len(self.__host_intfs)
        return host_if

    def ControlIpAddress(self):
        return self.__control_ip

    def MgmtIpAddress(self):
        if self.__os== 'esx':
            return self.__esx_ctrl_vm_ip
        return self.__ip_address

    def WorkloadType(self):
        return self.__workload_type

    def WorkloadImage(self):
        return self.__workload_image

    def WorkloadCpus(self):
        return self.__ncpu

    def WorkloadMemory(self):
        return self.__memory

    def GetMaxConcurrentWorkloads(self):
        return self.__concurrent_workloads

    def AddToNodeMsg(self, msg, topology, testsuite):
        if self.IsThirdParty():
            msg.type = topo_pb2.PERSONALITY_THIRD_PARTY_NIC
        else:
            msg.type = self.__role
        msg.image = ""
        msg.ip_address = self.__ip_address
        msg.name = self.__name

        if self.__os== 'esx':
            msg.os = topo_pb2.TESTBED_NODE_OS_ESX
            msg.esx_config.username = self.__esx_username
            msg.esx_config.password = self.__esx_password
            msg.esx_config.ip_address = self.__ip_address
        elif self.__os == 'linux':
            msg.os = topo_pb2.TESTBED_NODE_OS_LINUX
        elif self.__os == 'freebsd':
            msg.os = topo_pb2.TESTBED_NODE_OS_FREEBSD

        if self.Role() == topo_pb2.PERSONALITY_VENICE:
            msg.venice_config.control_intf = self.__control_intf
            msg.venice_config.control_ip = str(self.__control_ip)
            msg.image = os.path.basename(testsuite.GetImages().venice)
            for n in topology.Nodes():
                if n.Role() != topo_pb2.PERSONALITY_VENICE: continue
                peer_msg = msg.venice_config.venice_peers.add()
                peer_msg.host_name = n.Name()
                if api.IsSimulation():
                    peer_msg.ip_address = str(n.ControlIpAddress())
                else:
                    peer_msg.ip_address = str(n.MgmtIpAddress())
        else:
            if self.IsThirdParty() and not self.IsNaplesHwWithBumpInTheWire():
                msg.third_party_nic_config.nic_type = self.GetNicType()
            elif self.Role() == topo_pb2.PERSONALITY_NAPLES_MULTI_SIM:
                msg.naples_multi_sim_config.num_instances = 3
                msg.naples_multi_sim_config.network = self.GetNetwork().address
                msg.naples_multi_sim_config.gateway = self.GetNetwork().gateway
                msg.naples_multi_sim_config.nic_type = self.GetNicType()
                msg.image = os.path.basename(testsuite.GetImages().naples_sim)
                #Just test code
                msg.naples_multi_sim_config.venice_ips.append("1.2.3.4")
            else:
                msg.naples_config.nic_type = self.GetNicType()
                msg.naples_config.control_intf = self.__control_intf
                msg.naples_config.control_ip = str(self.__control_ip)
                if not self.IsNaplesHw() and not self.IsThirdParty():
                    msg.image = os.path.basename(testsuite.GetImages().naples)
                for data_intf in self.__data_intfs:
                    msg.naples_config.data_intfs.append(data_intf)
                    try:
                        self.DetermineNicPciInfo(data_intf)
                    except:
                        Logger.debug('failed to get pci info for node {0} nic {1}. error was: {2}'.format(self.__name, data_intf, traceback.format_exc()))
                for n in topology.Nodes():
                    if n.Role() != topo_pb2.PERSONALITY_VENICE: continue
                    msg.naples_config.venice_ips.append(str(n.ControlIpAddress()))

                #msg.naples_config.naples_ip_address = self.__nic_mgmt_ip
                if self.__nic_int_mgmt_ip == "N/A" or self.IsNaplesCloudPipeline():
                    msg.naples_config.naples_ip_address = self.__nic_mgmt_ip
                else:
                    msg.naples_config.naples_ip_address = self.__nic_int_mgmt_ip

                msg.naples_config.naples_username = "root"
                msg.naples_config.naples_password = "pen123"

            if self.IsNaplesHwWithBumpInTheWire() or self.IsNaplesHwWithBumpInTheWirePerf():
                #make sure to use actual management
                msg.naples_config.naples_ip_address = self.__nic_mgmt_ip
                msg.naples_config.nic_type = self.GetNicType()

            host_entity = msg.entities.add()
            host_entity.type = topo_pb2.ENTITY_TYPE_HOST
            host_entity.name = self.__name + "_host"
            if self.IsNaples():
                nic_entity = msg.entities.add()
                nic_entity.type = topo_pb2.ENTITY_TYPE_NAPLES
                nic_entity.name = self.__name + "_naples"

        script = self.GetStartUpScript()
        if script != None:
            msg.startup_script = script

        return types.status.SUCCESS

    def ProcessResponse(self, resp):
        self.__uuid = resp.node_uuid
        if self.IsNaples():
            self.__host_intfs = resp.naples_config.host_intfs
            self.__uuid = formatMac(self.__uuid)
            Logger.info("Node: %s UUID: %s" % (self.__name, self.__uuid))
        elif self.IsThirdParty():
            if GlobalOptions.dryrun:
                self.__host_intfs = []
            else:
                self.__host_intfs = resp.third_party_nic_config.host_intfs
        Logger.info("Node: %s Host Interfaces: %s" % (self.__name, self.__host_intfs))
        if len(self.__host_intfs) == 0 and  not self.IsVenice() and self.__role not in [topo_pb2.PERSONALITY_NAPLES_BITW, topo_pb2.PERSONALITY_NAPLES_BITW_PERF]:
            if GlobalOptions.dryrun:
                self.__host_intfs = ["dummy_intf0", "dummy_intf1"]
            else:
                Logger.error("Interfaces not found on Host: ", self.MgmtIpAddress())
                if self.IsNaples():
                    if not GlobalOptions.skip_host_intf_check:
                        Logger.error("Check if IONIC driver is installed.")
                        sys.exit(1)
                    else:
                        Logger.error("Ignoring Host interface check")

        return

    def GetStartUpScript(self):
        if self.IsNaplesHw():
            return api.HOST_NAPLES_DIR + "/" + "nodeinit.sh"
        return None

    def GetNodeInfo(self):
        info = {
            "Name" : self.__name,
            "NicMgmtIP" : self.__nic_mgmt_ip,
            "NicConsoleIP" : self.__nic_console_ip,
            "NicConsolePort" : self.__nic_console_port,
            "InstanceID" : self.__node_id
        }
        return info

    def RunConsoleCmd(self,cmd):
        if not self.__nic_console_ip:
            raise ValueError('no nic console ip configured at time of call to RunConsoleCmd()')
        console_hdl = Console(self.__nic_console_ip, self.__nic_console_port, disable_log=True)
        output = console_hdl.RunCmdGetOp(cmd)
        return re.split(cmd,output.decode("utf-8"),1)[1]

    def RunSshCmd(self, cmd):
        cmd = "%s %s \"%s\"" % (self.ssh_pfx, self.ssh_host, cmd)
        output = subprocess.check_output(cmd,shell=True,stderr=subprocess.STDOUT)
        return output.decode("utf-8")


class Topology(object):
    def __init__(self, spec):
        self.__nodes = {}

        assert(spec)
        Logger.info("Parsing Topology: %s" % spec)
        self.__dirname = os.path.dirname(spec)
        self.__spec = parser.YmlParse(spec)
        self.__parse_nodes()
        return

    def GetDirectory(self):
        return self.__dirname

    def __parse_nodes(self):
        for node_spec in self.__spec.nodes:
            node = Node(node_spec)
            self.__nodes[node.Name()] = node
        return

    def Nodes(self):
        return self.__nodes.values()

    def RestartNodes(self, node_names):
        req = topo_pb2.ReloadMsg()

        for node_name in node_names:
            if node_name not in self.__nodes:
                Logger.error("Node %s not found" % node_name)
                return types.status.FAILURE
            msg = req.node_msg.nodes.add()
            msg.name = node_name


        resp = api.ReloadNodes(req)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE

        return types.status.SUCCESS

    def Switches(self):
        switch_ips = {}
        req = topo_pb2.SwitchMsg()
        for node_name in self.__nodes:
            data_networks = self.__nodes[node_name].GetDataNetworks()
            for nw in data_networks:
                switch_ctx = switch_ips.get(nw.SwitchIP, None)
                if not switch_ctx:
                    switch_ctx = req.data_switches.add()
                    switch_ips[nw.SwitchIP] = switch_ctx
                switch_ctx.username = nw.SwitchUsername
                switch_ctx.password = nw.SwitchPassword
                switch_ctx.ip = nw.SwitchIP
                switch_ctx.ports.append(nw.Name)
        #Just return the switch IPs for now
        return switch_ips.keys()

    def FlapDataPorts(self, node_names, num_ports_per_node = 1, down_time = 5,
        flap_count = 1, interval = 5):
        req = topo_pb2.SwitchMsg()

        req.op = topo_pb2.FLAP_PORTS
        req.flap_info.count = flap_count
        req.flap_info.interval = interval
        req.flap_info.down_time = down_time
        switch_ips = {}
        for node_name in node_names:
            if node_name not in self.__nodes:
                Logger.error("Node %s not found to flap port" % node_name)
                return types.status.FAILURE
            data_networks = self.__nodes[node_name].GetDataNetworks()
            ports_added = 0
            for nw in data_networks:
                switch_ctx = switch_ips.get(nw.SwitchIP, None)
                if not switch_ctx:
                    switch_ctx = req.data_switches.add()
                    switch_ips[nw.SwitchIP] = switch_ctx
                switch_ctx.username = nw.SwitchUsername
                switch_ctx.password = nw.SwitchPassword
                switch_ctx.ip = nw.SwitchIP
                switch_ctx.ports.append(nw.Name)
                #This should from testsuite eventually or each testcase should be able to set
                ports_added = ports_added +  1
                if ports_added >= num_ports_per_node:
                    break

        resp = api.DoSwitchOperation(req)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE

        return types.status.SUCCESS


    def Setup(self, testsuite):
        Logger.info("Adding Nodes:")
        req = topo_pb2.NodeMsg()
        req.node_op = topo_pb2.ADD

        for name,node in self.__nodes.items():
            msg = req.nodes.add()
            ret = node.AddToNodeMsg(msg, self, testsuite)
            assert(ret == types.status.SUCCESS)

        resp = api.AddNodes(req)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE

        for node_resp in resp.nodes:
            node = self.__nodes[node_resp.name]
            node.ProcessResponse(node_resp)

        #save node info for future ref
        node_infos = []
        for n in self.__nodes.values():
            node_infos.append(n.GetNodeInfo())

        with open(node_log_file, 'w') as outfile:
            json.dump(node_infos, outfile, indent=4)

        return types.status.SUCCESS

    def Build(self, testsuite):
        Logger.info("Getting Nodes:")
        req = topo_pb2.NodeMsg()
        req.node_op = topo_pb2.ADD

        for name,node in self.__nodes.items():
            msg = req.nodes.add()
            ret = node.AddToNodeMsg(msg, self, testsuite)
            assert(ret == types.status.SUCCESS)

        resp = api.GetAddedNodes(req)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE

        for node_resp in resp.nodes:
            node = self.__nodes[node_resp.name]
            node.ProcessResponse(node_resp)

        return types.status.SUCCESS

    def __convert_to_roles(self, nics):
        roles = []
        for nic_type in nics:
            roles.append(GetNodePersonalityByNicType(nic_type))
        return roles

    def ValidateNics(self, nics):
        roles = self.__convert_to_roles(nics)
        for n in self.__nodes.values():
            if not n.IsVenice() and n.Role() not in roles:
                return False
        return True

    def GetVeniceMgmtIpAddresses(self):
        ips = []
        for n in self.__nodes.values():
            if n.Role() == topo_pb2.PERSONALITY_VENICE:
                ips.append(n.MgmtIpAddress())
        return ips

    def GetNaplesMgmtIpAddresses(self):
        ips = []
        for n in self.__nodes.values():
            if n.IsNaples():
                ips.append(n.MgmtIpAddress())
        return ips

    def GetMgmtIPAddress(self, node_name):
        return self.__nodes[node_name].MgmtIpAddress()

    def GetMgmtIPAddress(self, node_name):
         return self.__nodes[node_name].MgmtIpAddress()

    def GetNaplesUuidMap(self):
        uuid_map = {}
        for n in self.__nodes.values():
            if n.IsWorkloadNode():
                uuid_map[n.Name()] = n.UUID()
        return uuid_map

    def GetVeniceHostnames(self):
        ips = []
        for n in self.__nodes.values():
            if n.Role() == topo_pb2.PERSONALITY_VENICE:
                ips.append(n.Name())
        return ips

    def GetNaplesHostnames(self):
        ips = []
        for n in self.__nodes.values():
            if n.IsNaples():
                ips.append(n.Name())
        return ips

    def GetNaplesHostInterfaces(self, name):
        return self.__nodes[name].HostInterfaces()

    def GetWorkloadNodeHostnames(self):
        ips = []
        for n in self.__nodes.values():
            if n.IsWorkloadNode():
                ips.append(n.Name())
        return ips

    def GetWorkloadNodeHostInterfaces(self, node_name):
        return self.__nodes[node_name].HostInterfaces()

    def GetWorkloadTypeForNode(self, node_name):
        return self.__nodes[node_name].WorkloadType()

    def GetWorkloadImageForNode(self, node_name):
        return self.__nodes[node_name].WorkloadImage()

    def GetWorkloadCpusForNode(self, node_name):
        return self.__nodes[node_name].WorkloadCpus()

    def GetWorkloadMemoryForNode(self, node_name):
        return self.__nodes[node_name].WorkloadMemory()

    def AllocateHostInterfaceForNode(self, node_name):
        return self.__nodes[node_name].AllocateHostInterface()

    def GetNodeOs(self, node_name):
        return self.__nodes[node_name].GetOs()

    def GetNaplesMgmtIP(self, node_name):
        if self.__nodes[node_name].IsNaples():
            return self.__nodes[node_name].MgmtIpAddress()

    def GetNicMgmtIP(self, node_name):
        return self.__nodes[node_name].GetNicMgmtIP()

    def GetNicIntMgmtIP(self, node_name):
        return self.__nodes[node_name].GetNicIntMgmtIP()

    def GetNicConsoleIP(self, node_name):
        return self.__nodes[node_name].GetNicConsoleIP()

    def GetNicConsolePort(self, node_name):
        return self.__nodes[node_name].GetNicConsolePort()

    def GetNicIntMgmtIP(self, node_name):
        return self.__nodes[node_name].GetNicIntMgmtIP()

    def GetMaxConcurrentWorkloads(self, node_name):
        return self.__nodes[node_name].GetMaxConcurrentWorkloads()

    def GetNicType(self, node_name):
        return self.__nodes[node_name].GetNicType()

    def GetNodes(self):
        return list(self.__nodes.values())

    def SetUpTestBedInHostToHostNetworkMode():
        return store.GetTestbed().SetUpTestBedInHostToHostNetworkMode()

    def SetupTestBedNetwork():
        return store.GetTestbed().SetupTestBedNetwork()
