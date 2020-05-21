#! /usr/bin/python3
import pdb
import json
import copy
from collections import defaultdict
import ipaddress

from infra.common.logging import logger
import infra.common.objects as objects
from infra.common.glopts  import GlobalOptions

from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base
import apollo.config.objects.host.lif as lif
from apollo.config.objects.port import client as PortClient

import interface_pb2 as interface_pb2
import types_pb2 as types_pb2
import apollo.config.objects.metaswitch.cp_utils as cp_utils
import apollo.test.utils.pdsctl as pdsctl

class InterfaceStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.INTERFACE)
        self.LifId = None
        return

    def Update(self, status):
        uplinkIfStatus = getattr(status, 'UplinkIfStatus', None)
        if uplinkIfStatus:
            self.LifId = uplinkIfStatus.LifId
        return

class InterfaceSpec_:
    pass

class InterfaceObject(base.ConfigObjectBase):
    def __init__(self, node, spec, type):
        super().__init__(api.ObjectTypes.INTERFACE, node)
        super().SetOrigin(getattr(spec, 'origin', None))

        if hasattr(spec, 'iid'):
            self.InterfaceId = spec.iid
        else:
            self.InterfaceId = next(ResmgrClient[node].InterfaceIdAllocator)

        self.Type = type
        if hasattr(spec, 'uuid'):
            self.UUID = spec.uuid
        else:
            self.UUID = utils.PdsUuid(self.InterfaceId, self.ObjType)
        self.IfName = getattr(spec, 'ifname', None)
        self.AdminState = getattr(spec, 'ifadminstatus', None)
        self.MacAddr = getattr(spec, 'macaddress', None)
        self.VPCId = getattr(spec, 'vpcid', None)
        self.VrfName = None
        if hasattr(spec, 'vrfname'):
            self.VrfName = spec.vrfname
        elif self.VPCId:
            self.VrfName = f'Vpc{self.VPCId}'

        # Following attributes are valid only for netagent mode
        self.Speed = ''
        self.MTU = topo.MTU
        self.PauseSpec = InterfaceSpec_()
        self.PauseSpec.Type = 'DISABLE'
        self.PauseSpec.TxPauseEnabled = False
        self.PauseSpec.RxPauseEnabled = False

        self.Status = InterfaceStatus()
        self.Mutable = utils.IsUpdateSupported()
        if self.IfName:
            self.GID(f'{self.IfName}')
        else:
            self.GID(f'{INTFTYPE2STR.get(type)}Interface{self.InterfaceId:08x}')
        return

    def GetInterfaceName(self):
        return self.IfName

    def GetInterfaceMac(self):
        return self.MacAddr

    def GetInterfaceNetwork(self):
        return ''

    def GetInterfaceIpPrefix(self):
        return None

    def GetInterfaceIpPfxStr(self):
        pfx = self.GetInterfaceIpPrefix()
        if pfx:
            return pfx.exploded
        else:
            return ''

    def __repr__(self):
        return "Interface: %s" % (self.GID())

    def Show(self):
        logger.info("InterfaceObject:")
        logger.info(f"- {repr(self)}")
        str = f"Origin: {self.Origin} "
        if self.UUID:
            str += f"{self.UUID} "
        if self.IfName:
            str += f"Name: {self.IfName} "
        if self.MacAddr:
            str += f"MAC: {self.MacAddr} "
        if self.VPCId:
            str += f"VPC: {self.VPCId} "
        if self.VrfName:
            str += f"VrfName: {self.VrfName} "
        logger.info(str)
        return

    def UpdateImplicit(self):
        if not GlobalOptions.netagent:
            return
        if utils.IsDol() and GlobalOptions.dryrun:
            return
        if not self.IsOriginImplicitlyCreated():
            return
        # We need to read info from naples and update the DS
        resp = api.client[self.Node].GetHttp(self.ObjType)
        if not resp:
            return
        for ifinst in resp:
            if self.Type == topo.InterfaceTypes.L3:
                if (not ifinst['spec']['type'] == 'L3'):
                    continue
                riid = ifinst['meta']['name']
                if (self.InterfaceId != int(riid[len(riid)-1])):
                    continue
            elif self.Type == topo.InterfaceTypes.LOOPBACK:
                if (not ifinst['spec']['type'] == 'LOOPBACK'):
                    continue
            elif self.Type == topo.InterfaceTypes.ETH:
                if (not ifinst['spec']['type'] == 'HOST_PF'):
                    continue
                if (ifinst['status']['if-host-status']['host-ifname']
                    != self.IfName):
                    continue
            else:
                continue
            # Found matching interface, get basic info
            uuid_str = ifinst['meta']['uuid']
            self.UUID = utils.PdsUuid(bytes.fromhex(uuid_str.replace('-','')),\
                    self.ObjType)
            self.Tenant = ifinst['meta']['tenant']
            self.Namespace = ifinst['meta']['namespace']
            self.SetIfNameFromAgentData(ifinst['meta']['name'])

            # get ifinfo
            if hasattr(ifinst['spec'], 'ip-address'):
                self.IpPrefix = ipaddress.ip_network(ifinst['spec']['ip-address'],\
                        False)
            if hasattr(ifinst['spec'], 'vrf-name'):
                self.VrfName = ifinst['spec']['vrf-name']
        return

    def SetIfNameFromAgentData(self, ifname):
        self.IfName = ifname
        self.GID(ifname)
        return

    def Dup(self):
        dupObj = copy.copy(self)
        dupObj.InterfaceId = next(ResmgrClient[self.Node].InterfaceIdAllocator) + Resmgr.BaseDuplicateIdAllocator
        dupObj.GID("DupInterface ID:%s"%dupObj.InterfaceId)
        dupObj.UUID = utils.PdsUuid(dupObj.InterfaceId, dupObj.ObjType)
        dupObj.Interim = True
        self.Duplicate = dupObj
        return dupObj

    def CopyObject(self):
        clone = copy.copy(self)
        clone.PauseSpec = copy.copy(self.PauseSpec)
        return clone

    def UpdateAttributes(self, spec):
        self.MacAddr = ResmgrClient[self.Node].DeviceMacAllocator.get()
        self.MTU = self.MTU + 10
        self.Speed = '' #TODO
        self.PauseSpec.Type = 'LINK'
        return

    def RollbackAttributes(self):
        attrlist = ["MacAddr", "MTU", "Speed"]
        self.RollbackMany(attrlist)
        self.PauseSpec = self.GetPrecedent().PauseSpec
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        self.__unimplemented()
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.AdminStatus != interface_pb2.IF_STATUS_UP:
            return False
        return True

    def PopulateAgentJson(self):
        ifname = self.IfName
        if self.Type == topo.InterfaceTypes.LOOPBACK:
            iftype = 'LOOPBACK'
        elif self.Type == topo.InterfaceTypes.L3:
            iftype = 'L3'
        elif self.Type == topo.InterfaceTypes.ETH:
            iftype = 'HOST_PF'
            ifname = self.GetRESTKey()
        else:
            return None
        spec = {
            "kind": "Interface",
            "meta": {
                "name": ifname,
                "namespace": self.Namespace,
                "tenant": self.Tenant,
                "uuid": self.UUID.UuidStr,
                "labels": {
                    "CreatedBy": "Venice"
                },
            },
            "spec": {
                "type": iftype,
                "admin-status": 'UP',
                "vrf-name": self.VrfName,
                "network": self.GetInterfaceNetwork(),
                "ip-address": self.GetInterfaceIpPfxStr(),
                "speed": self.Speed,
                "mtu": self.MTU,
                "pause": {
                    "type": self.PauseSpec.Type,
                    "tx-pause-enabled": self.PauseSpec.TxPauseEnabled,
                    "rx-pause-enabled": self.PauseSpec.RxPauseEnabled,
                }
            }
        }
        return json.dumps(spec)

class HostInterfaceObject(InterfaceObject):
    def __init__(self, node, spec):
        super().__init__(node, spec, topo.InterfaceTypes.ETH)
        self.Network = ''
        self.RESTKey = None
        if utils.IsDol() and utils.IsHostLifSupported() and spec.lifns:
            self.obj_helper_lif = lif.LifObjectHelper(node)
            self.__create_lifs(spec)
        self.UpdateImplicit()
        self.Show()
        return

    def GetInterfaceNetwork(self):
        return self.Network

    def SetIfNameFromAgentData(self, ifname):
        # Netagent uses <mac>-<pfid> as key for host interfaces
        # for example 00ae.cd00.ff18-pf-71.
        # Not changing IfName property but keep it for REST purposes
        self.RESTKey = ifname
        return

    def GetRESTKey(self):
        return self.RESTKey

    def GetRESTPath(self):
        return "%s/%s/%s" % (self.Tenant, self.Namespace, self.RESTKey)

    def Show(self):
        super().Show()
        if self.RESTKey:
            logger.info(f"Agent-IFName: {self.RESTKey}")
        return

    def __create_lifs(self, spec):
        self.obj_helper_lif.Generate(spec.lifinfo, spec.lifspec, spec.lifns)
        self.obj_helper_lif.Configure()
        self.lif = self.obj_helper_lif.GetRandomHostLif()
        logger.info(" Selecting %s for Test" % self.lif.GID())
        self.lif.Show()
        return

    def UpdateVrfAndNetwork(self, subnet, dissociate):
        if self.InterfaceId == subnet.HostIfIdx:
            self.VrfName = subnet.Tenant if not dissociate else ""
            self.Network = subnet.GID() if not dissociate else ""
        return

class InbandMgmtInterfaceObject(InterfaceObject):
    def __init__(self, node, spec):
        super().__init__(node, spec, topo.InterfaceTypes.MGMT)
        self.Show()
        return

class ControlInterfaceObject(InterfaceObject):
    def __init__(self, node, spec):
        super().__init__(node, spec, topo.InterfaceTypes.CONTROL)
        if hasattr(spec, 'ipprefix'):
            self.IpPrefix = ipaddress.ip_network(spec.ipprefix.replace('\\', '/'), False)
            self.IfIpPrefix= ipaddress.ip_interface(spec.ipprefix.replace('\\', '/'))
        else:
            self.IpPrefix = next(ResmgrClient[node].L3InterfaceIPv4PfxPool)
            self.IfIpPrefix = next(ResmgrClient[node].L3InterfaceIPv4PfxPool)
        self.Gateway = None
        if hasattr(spec, 'gateway'):
            self.Gateway = ipaddress.IPv4Address(spec.gateway)
        self.Show()
        return

    def Show(self):
        super().Show()
        str = f"IP-Pfx: {self.IpPrefix} IF-IP-Pfx: {self.IfIpPrefix} "
        if self.Gateway:
            str += f"Gw: {self.Gateway}"
        logger.info(str)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        spec.Type = interface_pb2.IF_TYPE_CONTROL
        utils.GetRpcIfIPPrefix(self.IfIpPrefix, spec.ControlIfSpec.Prefix)
        spec.ControlIfSpec.MACAddress = self.MacAddr.getnum()
        if self.Gateway:
            spec.ControlIfSpec.Gateway.Af = types_pb2.IP_AF_INET
            spec.ControlIfSpec.Gateway.V4Addr = int(self.Gateway)
        return

    def ValidateSpec(self, spec):
        if not super().ValidateSpec(spec):
            return False
        if spec.Type != interface_pb2.IF_TYPE_CONTROL:
            return False
        return True

class L3InterfaceObject(InterfaceObject):
    def __init__(self, node, spec):
        super().__init__(node, spec, topo.InterfaceTypes.L3)
        self.IpPrefix = None
        # In IOTA, get L3 interface IPs from testbed json file if present.
        # If not, then we'll use the one in the cfgyml.
        if EzAccessStoreClient[node].GetUnderlayIPs():
            ifname = None
            if self.InterfaceId == 1:
                ifname = "Uplink0"
            elif self.InterfaceId == 2:
                ifname = "Uplink1"

            if ifname:
                self.IpPrefix = ipaddress.ip_network(EzAccessStoreClient[node].GetUnderlayIp(ifname) + "/" + \
                                                     EzAccessStoreClient[node].GetUnderlayMaskLen(ifname), False)
                self.IfIpPrefix = ipaddress.ip_interface(EzAccessStoreClient[node].GetUnderlayIp(ifname) + "/" + \
                                                       EzAccessStoreClient[node].GetUnderlayMaskLen(ifname))
                logger.info(f"Configuring L3 Interface Id: {self.InterfaceId}"
                            f"IP Prefix: {self.IpPrefix} IF IP Prefix: {self.IfIpPrefix}")
        if not self.IpPrefix:
            if hasattr(spec, 'ipprefix'):
                self.IpPrefix = ipaddress.ip_network(spec.ipprefix.replace('\\', '/'), False)
                self.IfIpPrefix = ipaddress.ip_interface(spec.ipprefix.replace('\\', '/'))
            else:
                self.IpPrefix = next(ResmgrClient[node].L3InterfaceIPv4PfxPool)
                self.IfIpPrefix = next(ResmgrClient[node].L3InterfaceIPv4PfxPool)
        self.EthIfIdx = getattr(spec, 'ethifidx', -1)
        if utils.IsDol():
            node_uuid = None
        else:
            node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        self.Port = utils.PdsUuid(self.EthIfIdx, node_uuid=node_uuid)
        self.PortNum = getattr(spec, 'port', -1)
        self.Encap = getattr(spec, 'encap', None)
        self.UpdateImplicit()
        self.Show()
        return

    def Show(self):
        super().Show()
        str = f"IP-Pfx: {self.IpPrefix} IF-IP-Pfx: {self.IfIpPrefix} "\
              f"Eth-ifindex: {self.EthIfIdx} Port: {self.Port} Portnum: {self.PortNum} "
        if self.Encap:
            str += f"Encap: {self.Encap}"
        logger.info(str)
        return

    def GetInterfaceIpPrefix(self):
        return self.IpPrefix

    def GetInterfaceEthIfIndex(self):
        return self.EthIfIdx

    # TODO: Move this to port object and link interface obj to port obj
    def LinkDown(self, port):
        cmd = " debug port -p "
        cmd += utils.PdsUuid.GetUuidString(self.Port.GetUuid())
        cmd += " -a down"
        pdsctl.ExecutePdsctlCommand(cmd, None, False)
        return

    def LinkUp(self, port):
        cmd = " debug port -p "
        cmd += utils.PdsUuid.GetUuidString(self.Port.GetUuid())
        cmd += " -a up"
        pdsctl.ExecutePdsctlCommand(cmd, None, False)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        spec.Type = interface_pb2.IF_TYPE_L3
        spec.L3IfSpec.PortId = self.Port.GetUuid()
        spec.L3IfSpec.MACAddress = self.MacAddr.getnum()
        spec.L3IfSpec.VpcId = utils.PdsUuid.GetUUIDfromId(self.VPCId, api.ObjectTypes.VPC)
        utils.GetRpcIPPrefix(self.IpPrefix, spec.L3IfSpec.Prefix)
        utils.GetRpcIfIPPrefix(self.IfIpPrefix, spec.L3IfSpec.Prefix)
        return

    def ValidateSpec(self, spec):
        if not super().ValidateSpec(spec):
            return False
        if spec.Type != interface_pb2.IF_TYPE_L3:
            return False
        if spec.L3IfSpec.PortId != self.Port.GetUuid():
            return False
        # TODO: Enable once device delete is fixed. MAC is also \
        # overwritten with 0 on deleting device config.
        #if spec.L3IfSpec.MACAddress != self.MacAddr.getnum():
        #    return False
        return True

class LoopbackInterfaceObject(InterfaceObject):
    def __init__(self, node, spec):
        super().__init__(node, spec, topo.InterfaceTypes.LOOPBACK)
        # If loopback ip exists in testbed json, use that,
        # else use from cfgyaml
        self.IpPrefix = utils.GetNodeLoopbackPrefix(node)
        if not self.IpPrefix and hasattr(spec, 'ipprefix'):
            self.IpPrefix = ipaddress.ip_network(spec.ipprefix.replace('\\', '/'), False)
        self.UpdateImplicit()
        self.Show()
        return

    def Show(self):
        super().Show()
        if self.IpPrefix:
            logger.info(f"IP-Pfx: {self.IpPrefix}")
        return

    def GetInterfaceIpPrefix(self):
        return self.IpPrefix

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        spec.Type = interface_pb2.IF_TYPE_LOOPBACK
        utils.GetRpcIPPrefix(self.IpPrefix, spec.LoopbackIfSpec.Prefix)
        return

class InterfaceObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.INTERFACE, Resmgr.MAX_INTERFACE)
        self.__uplinkl3ifs = defaultdict(dict)
        self.__loopback_if = defaultdict(dict)
        self.__control_if = defaultdict(dict)
        self.__uplinkl3ifs_iter = defaultdict(dict)
        self.__hostifs = defaultdict(dict)
        self.__hostifs_iter = defaultdict(dict)
        self.__inband_mgmt_ifs = defaultdict(dict)
        return

    def GetInterfaceObject(self, node, infid):
        return self.GetObjectByKey(node, infid)

    def GetHostInterface(self, node):
        if self.__hostifs[node]:
            return self.__hostifs_iter[node].rrnext()
        return None

    def FindHostInterface(self, node, hostifindex):
        if self.__hostifs[node]:
            return self.__hostifs[node].get(hostifindex, None)
        return None

    def GetNHInterfaceMac(self, node, ifname):
        if self.__inband_mgmt_ifs[node]:
            intf = self.__inband_mgmt_ifs[node].get(ifname, None)
            if intf:
                return intf.GetInterfaceMac()
        return None

    def GetL3UplinkInterface(self, node):
        if self.__uplinkl3ifs[node]:
            return self.__uplinkl3ifs_iter[node].rrnext()
        return None

    def __generate_host_interfaces_dol(self, node, ifspec):
        if not ifspec:
            return
        spec = InterfaceSpec_()
        spec.ifadminstatus = 'UP'
        spec.lifspec = ifspec.lif.Get(EzAccessStoreClient[node])
        for obj in ResmgrClient[node].HostIfs.values():
            spec = utils.CopySpec(spec, ifspec)
            spec.ifname = obj.IfName
            spec.lifinfo = obj
            lifstart = obj.LifBase
            lifend = lifstart + obj.LifCount - 1
            spec.lifns = objects.TemplateFieldObject("range/%d/%d" % (lifstart, lifend))

            spec = utils.CopySpec(spec, ifspec)
            hostif_obj = HostInterfaceObject(node, spec)
            self.__hostifs[node].update({hostif_obj.InterfaceId: hostif_obj})

        if self.__hostifs[node]:
            self.__hostifs_iter[node] = utils.rrobiniter(self.__hostifs[node].values())
        return

    def __generate_host_interfaces_iota(self, node):
        cmd_op = self.ReadLifs(node)

        if utils.IsDryRun():
            return
        for lif in cmd_op:
            lif_yaml = utils.LoadYaml(lif)
            if not lif_yaml:
                continue
            lif_spec = lif_yaml['spec']
            lif_status = lif_yaml['status']
            intf_type = lif_spec['type']
            spec = InterfaceSpec_()
            spec.iid = lif_status['ifindex']
            spec.uuid = utils.PdsUuid(lif_spec['id'])
            spec.ifname = lif_status['name']
            spec.macaddress = objects.MacAddressBase(integer=lif_spec['macaddress'])
            spec.origin = 'implicitly-created'

            if intf_type == types_pb2.LIF_TYPE_INBAND_MGMT:
                inb_mgmtif = InbandMgmtInterfaceObject(node, spec)
                self.__inband_mgmt_ifs[node].update({inb_mgmtif.GetInterfaceName(): inb_mgmtif})
            elif intf_type == types_pb2.LIF_TYPE_HOST:
                hostif = HostInterfaceObject(node, spec)
                self.__hostifs[node].update({hostif.InterfaceId: hostif})
        if self.__hostifs[node]:
            self.__hostifs_iter[node] = utils.rrobiniter(sorted(self.__hostifs[node].keys()))
        return

    def GenerateHostInterfaces(self, node, topospec):
        if not utils.IsInterfaceSupported():
            return
        if not utils.IsDol():
            self.__generate_host_interfaces_iota(node)
        else:
            if hasattr(topospec, 'hostinterface'):
                self.__generate_host_interfaces_dol(node, topospec.hostinterface)
        return

    def __generate_l3_uplink_interfaces(self, node, parent, iflist):
        uplink_ports = PortClient.Objects()
        if not uplink_ports:
            return

        for port in uplink_ports:
            spec = InterfaceSpec_()
            spec.vpcid = parent.VPCId
            spec.port = port.Port - 1
            spec.ethifidx = topo.PortToEthIfIdx(port.Port)
            spec.ifname = 'Uplink%d' % spec.port
            spec.ifadminstatus = port.AdminState
            for ifspec in iflist:
                if ifspec.iftype != 'l3':
                    continue
                if ifspec.portid == port.Port:
                    spec = utils.CopySpec(spec, ifspec)
                    if not hasattr(ifspec, 'macaddress'):
                        spec.macaddress = ResmgrClient[node].DeviceMacAllocator.get()
                    ifobj = L3InterfaceObject(node, spec)
                    self.Objs[node].update({ifobj.InterfaceId: ifobj})
                    self.__uplinkl3ifs[node].update({ifobj.InterfaceId: ifobj})

        if self.__uplinkl3ifs[node]:
            self.__uplinkl3ifs_iter[node] = utils.rrobiniter(self.__uplinkl3ifs[node].values())
        return

    def __generate_loopback_interfaces(self, node, iflist):
        for ifspec in iflist:
            if ifspec.iftype != 'loopback':
                continue
            spec = InterfaceSpec_()
            spec = utils.CopySpec(spec, ifspec)
            spec.ifname = 'Loopback%d' % next(ResmgrClient[node].LoopbackIfIdAllocator)
            ifobj = LoopbackInterfaceObject(node, spec)
            self.Objs[node].update({ifobj.InterfaceId: ifobj})
            self.__loopback_if[node] = ifobj
        return

    def __generate_control_interfaces(self, node, parent, iflist):
        for ifspec in iflist:
            if ifspec.iftype != 'control':
                continue
            spec = InterfaceSpec_()
            spec = utils.CopySpec(spec, ifspec)
            spec.ifname = 'ctrl0'
            spec.vpcid = parent.VPCId
            if not hasattr(ifspec, 'macaddress'):
                spec.macaddress = ResmgrClient[node].DeviceMacAllocator.get()
            ifobj = ControlInterfaceObject(node, spec)
            self.Objs[node].update({ifobj.InterfaceId: ifobj})
            self.__control_if[node] = ifobj

    def GenerateObjects(self, node, parent, topospec):
        if not utils.IsL3InterfaceSupported():
            return
        iflist = getattr(topospec, 'interface', [])
        self.__generate_l3_uplink_interfaces(node, parent, iflist)
        self.__generate_loopback_interfaces(node, iflist)
        self.__generate_control_interfaces(node, parent, iflist)
        return

    def AddObjToDict(self, obj):
        self.Objs[obj.Node].update({obj.InterfaceId: obj})
        return

    def DeleteObjFromDict(self, obj):
        self.Objs[obj.Node].pop(obj.InterfaceId, None)
        return

    def UpdateHostInterfaces(self, node, subnets, dissociate=False):
        if utils.IsDryRun() or not utils.IsNetAgentMode():
            return
        for subnet in subnets:
            hostif = self.FindHostInterface(node, subnet.HostIfIdx)
            if hostif:
                hostif.UpdateVrfAndNetwork(subnet, dissociate)
                hostif.Show()
                api.client[node].Update(api.ObjectTypes.INTERFACE, [hostif])
        print("after update pf")
        resp = api.client[node].GetHttp(api.ObjectTypes.INTERFACE)
        for r in resp:
            print(r)
        return

    def CreateObjects(self, node):
        if not GlobalOptions.netagent:
            cookie = utils.GetBatchCookie(node)
            if utils.IsL3InterfaceSupported():
                cfgObjects = self.__uplinkl3ifs[node].values()
                # create l3 if for uplink interface
                logger.info(f"Creating {len(cfgObjects)} L3 {self.ObjType.name} Objects in {node}")
                msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), cfgObjects))
                api.client[node].Create(api.ObjectTypes.INTERFACE, msgs)
                list(map(lambda x: x.SetHwHabitant(True), cfgObjects))
            if ((EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled() and utils.IsDol()) or (not EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled() and not utils.IsDol())):
                # create loopback interface
                lo_obj = self.__loopback_if[node]
                if lo_obj:
                    logger.info(f"Creating 1 Loopback {self.ObjType.name} {lo_obj} Objects in {node}")
                    lo_obj.Show()
                    msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), [lo_obj]))
                    api.client[node].Create(api.ObjectTypes.INTERFACE, msgs)

                # create control interface
                ctrl_obj = self.__control_if[node]
                if ctrl_obj:
                    logger.info(f"Creating 1 Control {self.ObjType.name} {ctrl_obj} Objects in {node}")
                    ctrl_obj.Show()
                    msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), [ctrl_obj]))
                    api.client[node].Create(api.ObjectTypes.INTERFACE, msgs)
                    list(map(lambda x: x.SetHwHabitant(True), [ctrl_obj]))

        else:
            obj = self.__loopback_if[node]
            obj.Show()
            api.client[node].Update(api.ObjectTypes.INTERFACE, [obj])
        return

    def ReadLifs(self, node):
        if utils.IsDryRun(): return
        ret, op = utils.RunPdsctlShowCmd(node, "lif", yaml=True)
        if not ret:
            logger.error(f"show lif failed for node {node}")
            return False
        cmdop = op.split("---")
        return cmdop

    def ReadAgentInterfaces(self, node):
        if utils.IsDryRun(): return
        if not GlobalOptions.netagent:
            return
        resp = api.client[node].GetHttp(api.ObjectTypes.INTERFACE)
        return resp

    def ReadObjects(self, node):
        logger.info(f"Reading {self.ObjType.name} Objects from {node}")
        msg = self.GetGrpcReadAllMessage(node)
        resp = api.client[node].Get(api.ObjectTypes.INTERFACE, [msg])
        result = self.ValidateObjects(resp, node)
        if result is False:
            logger.critical("INTERFACE object validation failed!!!")
            return False
        return True

    def ValidateObjects(self, getResp, node):
        if utils.IsDryRun(): return True
        numObjs = 0
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("INTERFACE get request failed for ", obj)
                continue
            for resp in obj.Response:
                key = self.GetKeyfromSpec(resp.Spec)
                inf = self.GetInterfaceObject(node, key)
                if inf is not None:
                    numObjs += 1
                    if not utils.ValidateObject(inf, resp):
                        logger.error("INTERFACE validation failed for ", resp.Spec)
                        inf.Show()
                        return False
                    # update status for this interface object
                    inf.Status.Update(resp.Status)
        logger.info(f"GRPC read count {numObjs} for {self.ObjType.name} in {node}")
        return (numObjs == self.GetNumHwObjects(node))


client = InterfaceObjectClient()
def GetMatchingObjects(selectors):
    return client.Objects()
