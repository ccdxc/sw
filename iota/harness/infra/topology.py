#! /usr/bin/python3
import os
import pdb
import json
import sys
import re
import socket
import subprocess
import time
import traceback
import ipaddress

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

def GetNodePersonalityByNicType(nic_type, mode = None):
    if nic_type in ['pensando', 'naples']:
        if mode == "dvs":
            return topo_pb2.PERSONALITY_NAPLES_DVS
        return topo_pb2.PERSONALITY_NAPLES
    elif nic_type == 'mellanox':
        if mode == "dvs":
            return topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS
        return topo_pb2.PERSONALITY_MELLANOX
    elif nic_type == 'broadcom':
        if mode == "dvs":
            return topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS
        return topo_pb2.PERSONALITY_BROADCOM
    elif nic_type == 'intel':
        if mode == "dvs":
            return topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS
        return topo_pb2.PERSONALITY_INTEL
    elif nic_type == 'pensando-sim':
        return topo_pb2.PERSONALITY_NAPLES_SIM
    else:
        return None


def GetNodeType(role):
    if role in ['PERSONALITY_NAPLES_SIM', 'PERSONALITY_VENICE', 'PERSONALITY_VCENTER_NODE']:
        return "vm"
    return "bm"

class Node(object):

    class ApcInfo:
        def __init__(self, ip, port, username, password):
            self.__ip = ip
            self.__port = port
            self.__username = username
            self.__password = password

        def GetIp(self):
            return self.__ip

        def GetPort(self):
            return self.__port

        def GetUsername(self):
            return self.__username

        def GetPassword(self):
            return self.__password

    class CimcInfo:
        def __init__(self, ip, ncsi_ip, username, password):
            self.__ip = ip
            self.__ncsi_ip = ncsi_ip
            self.__username = username
            self.__password = password

        def GetIp(self):
            return self.__ip

        def GetNcsiIp(self):
            return self.__ncsi_ip

        def GetUsername(self):
            return self.__username

        def GetPassword(self):
            return self.__password

    class PciInfo:
        def __init__(self, nic, bus, device, function):
            self.__nic = nic
            self.__bus = bus
            self.__device = device
            self.__function = function

        def GetInfo(self):
            return {'nic':self.__nic, 'bus':self.__bus, 'device':self.__device, 'function':self.__function}

    class NicDevice:
        def __init__(self, name):
            self.__name = name
            self.__uuid = None
            self.__mac = ""
            self.__host_intfs = None
            self.__host_if_alloc_idx = 0
            self.__nic_mgmt_ip = None
            self.__nic_console_ip = None
            self.__nic_console_port = None
            self.__nic_mgmt_intf = None
            self.__console_hdl = None

        def HostIntfs(self):
            return self.__host_intfs
        
        def Uuid(self):
            return self.__uuid

        def Name(self):
            return self.__name

        def SetUuid(self, uuid):
            self.__uuid = formatMac(uuid)
        
        def SetMac(self, mac):
            self.__mac = mac

        def GetMac(self):
            return self.__mac

        def SetHostIntfs(self, host_intfs):
            self.__host_intfs = host_intfs

        def AllocateHostInterface(self, device = None):
           if GlobalOptions.dryrun:
               return None
           host_if = self.__host_intfs[self.__host_if_alloc_idx]
           self.__host_if_alloc_idx = (self.__host_if_alloc_idx + 1) % len(self.__host_intfs)
           return host_if

        def GetNicMgmtIP(self):
           return self.__nic_mgmt_ip

        def GetNicIntMgmtIP(self):
           return self.__nic_int_mgmt_ip

        def GetHostNicIntMgmtIP(self):
           mnic_ip = ipaddress.ip_address(self.__nic_int_mgmt_ip)
           return str(mnic_ip + 1)

        def GetNicConsoleIP(self):
           return self.__nic_console_ip

        def GetNicConsolePort(self):
           return self.__nic_console_port

        def GetNicMgmtIntf(self):
            return self.__nic_mgmt_intf

        def GetNicUnderlayIPs(self):
           return self.__nic_underlay_ips

        def SetNicMgmtIP(self, ip):
           self.__nic_mgmt_ip = ip

        def SetNicIntMgmtIP(self, ip):
           self.__nic_int_mgmt_ip = ip

        def SetNicConsoleIP(self, ip):
           self.__nic_console_ip = ip

        def SetNicConsolePort(self, port):
           self.__nic_console_port = port
        
        def SetNicMgmtIntf(self, intf):
            self.__nic_mgmt_intf = intf

        def SetNicUnderlayIPs(self, ips):
           self.__nic_underlay_ips = ips

        def read_from_console(self):
            if self.__nic_console_ip != "" and self.__nic_console_port != "":
                try:
                    ip_read = False
                    with open(node_log_file, 'r') as json_file:
                        data = json.load(json_file)
                        for node in data:
                            for _, dev in node["Devices"].items():
                                if dev["NicConsoleIP"] == self.__nic_console_ip and \
                                    dev["NicConsolePort"] == self.__nic_console_port and \
                                    dev["NicMgmtIP"] not in [ "N/A", "" ] and dev["Mac"] not in [ "N/A", "" ]:
                                    self.__nic_mgmt_ip = dev["NicMgmtIP"]
                                    self.__mac = dev["Mac"]
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

                        output = self.__console_hdl.RunCmdGetOp("ip link | grep oob_mnic0 -A 1 | grep ether")
                        mac_regexp = '(?:[0-9a-fA-F]:?){12}'
                        x = re.findall(mac_regexp.encode(), output)
                        if len(x) > 0:
                            self.__mac = x[0].decode('utf-8')
                            Logger.info("Read oob mac %s %s" % (self.__name, x[0]))
            else:
                Logger.info("Skipping management IP read as no console info %s" % self.__name)

    def __init__(self, topo, spec):
        self.__spec = spec
        self.__topo = topo
        self.__name = spec.name
        self.__node_type = GetNodeType(spec.role)
        self.__node_tag = getattr(spec, "Tag", None)
        self.__inst = store.GetTestbed().AllocateInstance(self.__node_type, self.__node_tag)
        self.__role = self.__get_instance_role(spec.role, getattr(spec, "mode", None))
        self.__node_id = getattr(self.__inst, "ID", 0)

        self.__ip_address = self.__inst.NodeMgmtIP
        self.__os = getattr(self.__inst, "NodeOs", "linux")

        self.__apcInfo = self.__update_apc_info()
        self.__cimcInfo = self.__update_cimc_info()

        self.__dev_index = 1
        self.__devices = {}
        self.__nic_underlay_ips = []


        nics = getattr(self.__inst, "Nics", None)
        if  self.__node_type == "bm":
            if nics != None and len(nics) != 0:
                for nic in nics:
                    name = self.GetNicType() + str(self.__dev_index)
                    device = Node.NicDevice(name)
                    self.__dev_index = self.__dev_index + 1
                    self.__devices[name] = device
                    device.SetNicMgmtIP(getattr(nic, "MgmtIP", None))
                    device.SetNicConsoleIP(getattr(nic, "ConsoleIP", ""))
                    device.SetNicConsolePort(getattr(nic, "ConsolePort", ""))
                    device.SetNicIntMgmtIP(getattr(nic, "IntMgmtIP", api.GetPrimaryIntNicMgmtIp()))
                    device.SetNicMgmtIntf(getattr(nic, "NicMgmtIntf", "oob_mnic0"))
                    device.SetNicUnderlayIPs(getattr(self.__inst, "NicUnderlayIPs", []))
                    for port in getattr(nic, "Ports", []):
                        device.SetMac(port.MAC)
                        break
                    device.read_from_console()       

            else:
                for index in range(1):
                    name = self.GetNicType() + str(self.__dev_index)
                    device = Node.NicDevice(name)
                    self.__dev_index = self.__dev_index + 1
                    self.__devices[name] = device
                    device.SetNicMgmtIP(getattr(self.__inst, "NicMgmtIP", None))
                    device.SetNicConsoleIP(getattr(self.__inst, "NicConsoleIP", ""))
                    device.SetNicConsolePort(getattr(self.__inst, "NicConsolePort", ""))
                    device.SetNicIntMgmtIP(getattr(self.__inst, "NicIntMgmtIP", api.GetPrimaryIntNicMgmtIp()))
                    device.SetNicMgmtIntf(getattr(self.__inst, "NicMgmtIntf", "oob_mnic0"))
                    device.SetNicUnderlayIPs(getattr(self.__inst, "NicUnderlayIPs", []))
                    device.read_from_console()

        self.__nic_pci_info = {}
        self.__nic_info = {}
        self.__vmUser = getattr(self.__inst, "Username", "vm")
        self.__vmPassword = getattr(self.__inst, "Password", "vm")
        self.ssh_host = "%s@%s" % (self.__vmUser, self.__ip_address) 
        self.ssh_pfx = "sshpass -p %s ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no " % self.__vmPassword
        self.__control_ip = resmgr.ControlIpAllocator.Alloc()
        self.__control_intf = "eth1"
        self.__cimc_ip = getattr(self.__inst, "NodeCimcIP", None)
        self.__cimc_ncsi_ip = getattr(self.__inst, "NodeCimcNcsiIP", None)
        self.__cimc_username = getattr(self.__inst, "NodeCimcUsername", None)
        self.__cimc_password = getattr(self.__inst, "NodeCimcPassword", None)
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
            if not osDetail:
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

    def __update_apc_info(self):
        try:
            res = self._Node__inst.Resource
            if getattr(res,"ApcIP",None):
                return Node.ApcInfo(res.ApcIP, res.ApcPort, res.ApcUsername, res.ApcPassword)
        except:
            Logger.debug("failed to parse apc info. error was: {0}".format(traceback.format_exc()))
        return None

    def __update_cimc_info(self):
        try:
            cimc_ip = getattr(self._Node__inst,"NodeCimcIP","")
            if cimc_ip == "":
                return None
            node = Node.CimcInfo(cimc_ip, 
                                 getattr(self._Node__inst.Resource,"NodeCimcNcsiIP", ""),
                                 getattr(self._Node__inst,"NodeCimcUsername","admin"),
                                 getattr(self._Node__inst,"NodeCimcPassword","N0isystem$"))
            return node
        except:
            Logger.debug("failed to parse cimc info. error was: {0}".format(traceback.format_exc()))
        return None

    def __get_instance_role(self, role, mode=None):
        if role != 'auto':
            return topo_pb2.PersonalityType.Value(role)

        if getattr(self.__inst.Resource, "DataNicType", None):
            return topo_pb2.PERSONALITY_NAPLES_BITW

        nic_type = self.__inst.Resource.NICType

        role = GetNodePersonalityByNicType(nic_type, mode)
        if role is None:
            os.system("cp /warmd.json '%s/iota/logs" % GlobalOptions.topdir)
            os.system("cat /warmd.json")
            Logger.error("Unknown NIC Type : %s %s" % (nic_type, role))
            sys.exit(1)
        return role

    def GetApcInfo(self):
        return self.__apcInfo

    def GetCimcInfo(self):
        return self.__cimcInfo

    def IpmiPowerCycle(self):
        cimc = self.GetCimcInfo()
        if not cimc:
            raise Exception('no cimc info for node {0} in warmd.json'.format(self.__name))
        cmd="ipmitool -I lanplus -H %s -U %s -P %s power cycle" %\
              (cimc.GetIp(), cimc.GetUsername(), cimc.GetPassword())
        subprocess.check_call(cmd, shell=True)
        time.sleep(30)

    def ApcPowerCycle(self):
        self.PowerOffNode()
        time.sleep(20)
        self.PowerOnNode()
        time.sleep(20)

    def PowerOffNode(self):
        apcInfo = self.GetApcInfo()
        if not apcInfo:
            raise Exception('no apc info for node {0} in warmd.json'.format(self.__name))
        apcctrl = ApcControl(host=apcInfo.GetIp(), username=apcInfo.GetUsername(),
                                password=apcInfo.GetPassword())
        apcctrl.portOff(apcInfo.GetPort())

    def PowerOnNode(self):
        apcInfo = self.GetApcInfo()
        if not apcInfo:
            raise Exception('no apc info for node {0} in warmd.json'.format(self.__name))
        apcctrl = ApcControl(host=apcInfo.GetIp(), username=apcInfo.GetUsername(),
                                password=apcInfo.GetPassword())
        apcctrl.portOn(apcInfo.GetPort())

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

    def GetNicMgmtIP(self, device = None):
        dev = self.__get_device(device)
        return dev.GetNicMgmtIP()

    def GetNicIntMgmtIP(self, device = None):
        dev = self.__get_device(device)
        return dev.GetNicIntMgmtIP()

    def GetHostNicIntMgmtIP(self, device = None):
        dev = self.__get_device(device)
        return dev.GetHostNicIntMgmtIP()

    def GetNicConsoleIP(self, device = None):
        dev = self.__get_device(device)
        return dev.GetNicConsoleIP()

    def GetNicConsolePort(self, device = None):
        dev = self.__get_device(device)
        return dev.GetNicConsolePort()

    def GetNicUnderlayIPs(self, device = None):
        dev = self.__get_device(device)
        return dev.GetNicUnderlayIPs()

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
        return self.__role in[ topo_pb2.PERSONALITY_NAPLES, topo_pb2.PERSONALITY_NAPLES_DVS]
    def IsNaplesHwWithBumpInTheWire(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES_BITW
    def IsNaplesHwWithBumpInTheWirePerf(self):
        return self.__role == topo_pb2.PERSONALITY_NAPLES_BITW_PERF
    def IsNaplesCloudPipeline(self):
        return GlobalOptions.pipeline in [ "apulu" ]

    def IsOrchestratorNode(self):
        return self.__role == topo_pb2.PERSONALITY_VCENTER_NODE

    def IsNaples(self):
        return self.IsNaplesSim() or self.IsNaplesHw() or self.IsNaplesHwWithBumpInTheWire()

    def IsMellanox(self):
        return self.__role in [ topo_pb2.PERSONALITY_MELLANOX, topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS]
    def IsBroadcom(self):
        return self.__role in [ topo_pb2.PERSONALITY_BROADCOM, topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS]
    def IsIntel(self):
        return self.__role in [ topo_pb2.PERSONALITY_INTEL, topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS]
    def IsThirdParty(self):
        return self.IsMellanox() or self.IsBroadcom() or self.IsIntel()
    def IsWorkloadNode(self):
        return self.__role != topo_pb2.PERSONALITY_VENICE and self.__role != topo_pb2.PERSONALITY_VCENTER_NODE

    def GetDevicesNames(self):
        return sorted(self.__devices.keys())

    def GetDevices(self):
        return self.__devices

    def __get_device(self, device = None):
        key = ""
        if device is None:
            devices = list(self.__devices.keys())
            devices.sort()
            key = devices[0]
        else:
            key = device
        return self.__devices[key]

    def GetDefaultDeivce(self):
        return self.__get_device(None)

    def UUID(self, device = None):
        if self.IsThirdParty():
            return self.Name()
        dev = self.__get_device(device)
        return dev.Uuid()

    def HostInterfaces(self, device = None):
        dev = self.__get_device(device)
        return dev.HostIntfs()

    def AllocateHostInterface(self, device = None):
        dev = self.__get_device(device)
        return dev.AllocateHostInterface()

    def ControlIpAddress(self):
        return self.__control_ip

    def MgmtIpAddress(self):
        if self.__os== 'esx':
            return self.__esx_ctrl_vm_ip
        return self.__ip_address

    def EsxHostIpAddress(self):
        if self.__os == 'esx':
            return self.__ip_address
        return None

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
            if getattr(self.__spec, "mode", None) == "dvs":
                msg.type = topo_pb2.PERSONALITY_THIRD_PARTY_NIC_DVS

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

        if self.Role() == topo_pb2.PERSONALITY_VCENTER_NODE:
            msg.vcenter_config.pvlan_start = int(self.__spec.vlan_start)
            msg.vcenter_config.pvlan_end  = int(self.__spec.vlan_end)
            self.__topo.vlan_start = int(self.__spec.vlan_start)
            self.__topo.vlan_end = int(self.__spec.vlan_end)
            managed_nodes = self.__spec.managednodes
            for node in managed_nodes:
                esx_config = msg.vcenter_config.esx_configs.add()
                esx_config.username = getattr(self.__tb_params, "EsxUsername", "")
                esx_config.password = getattr(self.__tb_params, "EsxPassword", "")
                esx_config.ip_address = self.__topo.GetMgmtIPAddress(node)
                esx_config.name = node
            msg.vcenter_config.dc_name = GlobalOptions.dc_name
            msg.vcenter_config.cluster_name = GlobalOptions.cluster_name
            msg.vcenter_config.distributed_switch = GlobalOptions.distributed_switch

        elif self.Role() == topo_pb2.PERSONALITY_VENICE:
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
                if not self.IsNaplesHw() and not self.IsThirdParty():
                    msg.image = os.path.basename(testsuite.GetImages().naples)
                for _, device in self.__devices.items():
                    naples_config = msg.naples_configs.configs.add()
                    naples_config.nic_type = self.GetNicType()
                    naples_config.control_intf = self.__control_intf
                    naples_config.control_ip = str(self.__control_ip)
                    naples_config.name = device.Name()
                    naples_config.naples_username = "root"
                    naples_config.naples_password = "pen123"
                    #Enable this with Brad's PR
                    naples_config.nic_hint = device.GetMac()

                    if device.GetNicIntMgmtIP() == "N/A" or self.IsNaplesCloudPipeline():
                        naples_config.naples_ip_address = device.GetNicMgmtIP()
                    #else:
                    #    #naples_config.naples_ip_address = device.GetNicIntMgmtIP()

                    for n in topology.Nodes():
                        if n.Role() != topo_pb2.PERSONALITY_VENICE: continue
                        naples_config.venice_ips.append(str(n.ControlIpAddress()))
                    for data_intf in self.__data_intfs:
                        naples_config.data_intfs.append(data_intf)
                    try:
                        self.DetermineNicPciInfo(data_intf)
                    except:
                        Logger.debug('failed to get pci info for node {0} nic {1}. error was: {2}'.format(self.__name, data_intf, traceback.format_exc()))

            if self.IsNaplesHwWithBumpInTheWire() or self.IsNaplesHwWithBumpInTheWirePerf():
                #make sure to use actual management
                msg.naples_config.naples_ip_address = self.__nic_mgmt_ip
                msg.naples_config.nic_type = self.GetNicType()

            host_entity = msg.entities.add()
            host_entity.type = topo_pb2.ENTITY_TYPE_HOST
            host_entity.name = self.__name + "_host"
            if self.IsNaples():
                for _, device in self.__devices.items():
                    nic_entity = msg.entities.add()
                    nic_entity.type = topo_pb2.ENTITY_TYPE_NAPLES
                    nic_entity.name = device.Name()

        script = self.GetStartUpScript()
        if script is not None:
            msg.startup_script = script

        return types.status.SUCCESS

    def ProcessResponse(self, resp):

        if self.IsOrchestratorNode():
            Logger.info("Setting orch node as %s" % (self.Name()))
            self.__topo.SetOrchNode(self)

        if self.IsNaples():
            for naples_config in resp.naples_configs.configs:
                device = self.__get_device(naples_config.name)
                assert(device)
                device.SetUuid(naples_config.node_uuid)
                device.SetHostIntfs(naples_config.host_intfs)
                device.SetNicIntMgmtIP(naples_config.naples_ip_address)
                Logger.info("Nic: %s UUID: %s" % (naples_config.name, naples_config.node_uuid))
                self.__host_intfs.extend(naples_config.host_intfs)
        elif self.IsThirdParty():
            if GlobalOptions.dryrun:
                self.__host_intfs = []
            else:
                device = self.GetDefaultDeivce()
                assert(device)
                device.SetHostIntfs(resp.third_party_nic_config.host_intfs)
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
            "InstanceID" : self.__node_id,
            "Devices" : {}
        }
        for _, device in self.__devices.items():
            dev_info = {
                "Name" :  device.Name(),
                "NicMgmtIP" : device.GetNicMgmtIP(),
                "NicConsoleIP" : device.GetNicConsoleIP(),
                "NicConsolePort" : device.GetNicConsolePort(),
                "Mac" : device.GetMac()
            }
            info["Devices"][device.Name()] = dev_info
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

    def WaitForHost(self, port=22):
        Logger.debug("waiting for host {0} to be up".format(self.__ip_address))
        for retry in range(60):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            ret = sock.connect_ex(('%s' % self.__ip_address, port))
            sock.settimeout(10)
            if not ret:
                return True
            time.sleep(5)
        raise Exception("host {0} not up".format(self.__ip_address))


class Topology(object):

    RestartMethodAuto = ''
    RestartMethodApc = 'apc'
    RestartMethodIpmi = 'ipmi'
    RestartMethodReboot = 'reboot'
    RestartMethods = [RestartMethodAuto, RestartMethodApc, RestartMethodIpmi, RestartMethodReboot]

    IpmiMethodCycle = "cycle"
    IpmiMethodOn = "on"
    IpmiMethodOff = "off"
    IpmiMethodReset = "reset"
    IpmiMethodSoft = "soft"
    IpmiMethods = [IpmiMethodCycle, IpmiMethodOn, IpmiMethodOff, IpmiMethodReset, IpmiMethodSoft]

    def __init__(self, spec):
        self.__nodes = {}
        self.__orch_node = None

        assert(spec)
        Logger.info("Parsing Topology: %s" % spec)
        self.__dirname = os.path.dirname(spec)
        self.__spec = parser.YmlParse(spec)
        self.__parse_nodes()
        self.vlan_start = 0
        self.vlan_end = 0
        return

    def GetDirectory(self):
        return self.__dirname

    def SetOrchNode(self, node):
        self.__orch_node = node

    def __parse_nodes(self):
        for node_spec in self.__spec.nodes:
            node = Node(self, node_spec)
            self.__nodes[node.Name()] = node
        return

    def Nodes(self):
        return self.__nodes.values()

    def IpmiNodes(self, node_names, ipmiMethod, useNcsi=False):
        if ipmiMethod not in self.IpmiMethods:
            raise ValueError('ipmiMethod must be one of {0}'.format(self.IpmiMethods))
        req = topo_pb2.ReloadMsg()
        req.restart_method = ipmiMethod
        req.use_ncsi = useNcsi
        for node_name in node_names:
            if node_name not in self.__nodes:
                Logger.error("Node %s not found" % node_name)
                return types.status.FAILURE
            node = self.__nodes[node_name]
            msg = req.node_msg.nodes.add()
            msg.name = node_name

        resp = api.IpmiNodeAction(req)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE

        return types.status.SUCCESS

    def RestartNodes(self, node_names, restartMethod=RestartMethodAuto, useNcsi=False):
        if GlobalOptions.dryrun:
            return types.status.SUCCESS

        if restartMethod not in self.RestartMethods:
            raise ValueError('restartMethod must be one of {0}'.format(self.RestartMethods))
        req = topo_pb2.ReloadMsg()
        req.restart_method = restartMethod
        req.use_ncsi = useNcsi
        for node_name in node_names:
            if node_name not in self.__nodes:
                Logger.error("Node %s not found" % node_name)
                return types.status.FAILURE
            node = self.__nodes[node_name]
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

    def __doPortConfig(self, node_names, req):
        switch_ips = {}
        for node_name in node_names:
            if node_name not in self.__nodes:
                Logger.error("Node %s not found to flap port" % node_name)
                return types.status.FAILURE
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
        resp = api.DoSwitchOperation(req)
        if not api.IsApiResponseOk(resp):
            return types.status.FAILURE
        return types.status.SUCCESS

    def DisablePfcPorts(self, node_names):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_PFC_CONFIG
        req.port_pfc.enable = False
        return self.__doPortConfig(node_names, req)

    def EnablePfcPorts(self, node_names):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_PFC_CONFIG
        req.port_pfc.enable = True
        return self.__doPortConfig(node_names, req)

    def DisablePausePorts(self, node_names):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_PAUSE_CONFIG
        req.port_pause.enable = False
        return self.__doPortConfig(node_names, req)

    def EnablePausePorts(self, node_names):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_PAUSE_CONFIG
        req.port_pause.enable = True
        return self.__doPortConfig(node_names, req)

    def DisableQosPorts(self, node_names, params):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_QOS_CONFIG
        req.port_qos.enable = False
        req.port_qos.params = params
        return self.__doPortConfig(node_names, req)

    def EnableQosPorts(self, node_names, params):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_QOS_CONFIG
        req.port_qos.enable = True
        req.port_qos.params = params
        return self.__doPortConfig(node_names, req)

    def DisableQueuingPorts(self, node_names, params):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_QUEUING_CONFIG
        req.port_queuing.enable = False
        req.port_queuing.params = params
        return self.__doPortConfig(node_names, req)

    def EnableQueuingPorts(self, node_names, params):
        req = topo_pb2.SwitchMsg()
        req.op = topo_pb2.PORT_QUEUING_CONFIG
        req.port_queuing.enable = True
        req.port_queuing.params = params
        return self.__doPortConfig(node_names, req)

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

        if resp.allocated_vlans:
            tbvlans = []
            for vlan in resp.allocated_vlans:
                tbvlans.append(vlan)
            store.GetTestbed().SetVlans(tbvlans)

        return types.status.SUCCESS

    def __convert_to_roles(self, nics, mode=None):
        roles = []
        for nic_type in nics:
            roles.append(GetNodePersonalityByNicType(nic_type, mode))
        return roles

    def ValidateNics(self, nics):
        roles = self.__convert_to_roles(nics, getattr(self.__spec.meta, "mode", None))
        for n in self.__nodes.values():
            if not n.IsVenice() and not n.IsOrchestratorNode() and n.Role() not in roles:
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

    def GetNaplesUuidMap(self):
        uuid_map = {}
        for n in self.__nodes.values():
            if n.IsWorkloadNode():
                for _, device in n.GetDevices().items():
                    uuid_map[device.Name()] = device.Uuid()
                #Also set default to first
                device = n.GetDefaultDeivce()
                assert(device)
                uuid_map[n.Name()] = device.Uuid()
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

    def GetVlanStart(self):
        return self.vlan_start

    def GetVlanEnd(self):
        return self.vlan_end

    def GetNaplesMgmtIP(self, node_name):
        if self.__nodes[node_name].IsNaples():
            return self.__nodes[node_name].MgmtIpAddress()

    def GetNicMgmtIP(self, node_name, device = None):
        return self.__nodes[node_name].GetNicMgmtIP(device)

    def GetNicIntMgmtIP(self, node_name, device = None):
        return self.__nodes[node_name].GetNicIntMgmtIP(device)

    def GetNicConsoleIP(self, node_name, device = None):
        return self.__nodes[node_name].GetNicConsoleIP(device)

    def GetNicConsolePort(self, node_name, device = None):
        return self.__nodes[node_name].GetNicConsolePort(device)

    def GetNicIntMgmtIP(self, node_name, device = None):
        return self.__nodes[node_name].GetNicIntMgmtIP(device)

    def GetHostNicIntMgmtIP(self, node_name, device = None):
        return self.__nodes[node_name].GetHostNicIntMgmtIP(device)

    def GetNicUnderlayIPs(self, node_name, device = None):
        return self.__nodes[node_name].GetNicUnderlayIPs(device)

    def GetEsxHostIpAddress(self, node_name):
        return self.__nodes[node_name].EsxHostIpAddress()

    def GetMaxConcurrentWorkloads(self, node_name):
        return self.__nodes[node_name].GetMaxConcurrentWorkloads()

    def GetNicType(self, node_name):
        return self.__nodes[node_name].GetNicType()

    def GetNodes(self):
        return list(self.__nodes.values())

    def GetOrchestratorNode(self):
        return self.__orch_node.Name()

    def SetUpTestBedInHostToHostNetworkMode(self):
        return store.GetTestbed().SetUpTestBedInHostToHostNetworkMode()

    def SetupTestBedNetwork(self):
        return store.GetTestbed().SetupTestBedNetwork()

    def GetDefaultDeivce(self, node_name):
        return self.__nodes[node_name].GetDefaultDeivce()
    
    def GetDefaultNaples(self, node_name):
        device = self.GetDefaultDeivce(node_name)
        assert(device)
        return device.Name()
