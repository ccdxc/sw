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

class InterfaceInfoObject(base.ConfigObjectBase):
    def __init__(self, node, iftype, spec, ifspec):
        self.Node = node
        self.__type = iftype
        self.VrfName = ''
        self.Network = ''
        self.PfxStr = ''
        if (iftype == topo.InterfaceTypes.UPLINK):
            self.port_num = getattr(spec, 'port', None)
            self.VrfName = 'underlay-vpc'
        elif iftype == topo.InterfaceTypes.MGMT:
            self.VrfName = 'default'
        elif (iftype == topo.InterfaceTypes.UPLINKPC):
            self.port_bmap = getattr(spec, 'portbmp', None)
        elif (iftype == topo.InterfaceTypes.L3):
            if (hasattr(spec, 'vpcid')):
                self.VpcId = spec.vpcid
                self.VrfName = f'Vpc{spec.vpcid}'
            if (hasattr(ifspec, 'ipprefix')):
                self.ip_prefix = ipaddress.ip_network(ifspec.ipprefix.replace('\\', '/'), False)
            else:
                self.ip_prefix = next(ResmgrClient[node].L3InterfaceIPv4PfxPool)
            self.ethifidx = getattr(spec, 'ethifidx', -1)
            if utils.IsDol():
                node_uuid = None
            else:
                node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
            self.Port = utils.PdsUuid(self.ethifidx, node_uuid=node_uuid)
            self.port_num = getattr(spec, 'port', -1)
            self.encap = getattr(spec, 'encap', None)
            self.macaddr = getattr(spec, 'MACAddr', None)
        elif (iftype == topo.InterfaceTypes.LOOPBACK):
            # If loopback ip exists in testbed json, use that,
            # else use from cfgyaml
            self.ip_prefix = utils.GetNodeLoopbackPrefix(node)
            if not self.ip_prefix and hasattr(ifspec, 'ipprefix'):
                self.ip_prefix = ipaddress.ip_network(ifspec.ipprefix.replace('\\', '/'), False)
        if hasattr(self, "ip_prefix"):
            self.PfxStr = self.ip_prefix.exploded

    def Show(self):
        if (self.__type == topo.InterfaceTypes.UPLINK):
            res = str("port num : %d" % int(self.port_num))
        elif (self.__type == topo.InterfaceTypes.UPLINKPC):
            res = str("port_bmap: %s" % self.port_bmap)
        elif (self.__type == topo.InterfaceTypes.L3):
            res = str("VPC:%s|ip:%s|port: %s|encap:%s|mac:%s"% \
                    (self.VrfName, self.ip_prefix, self.Port, self.encap, \
                    self.macaddr.get()))
        elif (self.__type == topo.InterfaceTypes.LOOPBACK):
            res = f"ip:{self.ip_prefix}"
        else:
            return
        logger.info("- %s" % res)

class InterfaceObject(base.ConfigObjectBase):
    def __init__(self, spec, ifspec, node, spec_json=None, type=topo.InterfaceTypes.NONE):
        super().__init__(api.ObjectTypes.INTERFACE, node)
        super().SetOrigin(getattr(ifspec, 'origin', None))
        print(spec_json)
        if type == topo.InterfaceTypes.ETH:
            self.InterfaceId = next(ResmgrClient[node].InterfaceIdAllocator)
            self.Ifname = spec_json['meta']['name']
            self.Tenant = spec_json['meta']['tenant']
            self.Namespace = spec_json['meta']['namespace']
            self.Type = 0
            if spec_json['spec']['type'] == "HOST_PF":
                self.Type = topo.InterfaceTypes.ETH
            else:
                logger.error("Unhandled if type")
            self.AdminState = spec_json['spec']['admin-status']
            info = InterfaceInfoObject(node, topo.InterfaceTypes.ETH, None, None)
            self.IfInfo = info
            self.IfInfo.VrfName = 'Vpc1'
            self.IfInfo.Network = 'Subnet1'
            self.IfInfo.ip_prefix = ''
            self.Status = InterfaceStatus()
            self.GID(spec_json['meta']['name'])
            uuid_str = spec_json['meta']['uuid']
            self.HostIfIdx = spec_json['status']['id']
            self.UUID = utils.PdsUuid(bytes.fromhex(uuid_str.replace('-','')),\
                    self.ObjType)
            self.DeriveOperInfo()
            self.Show()
            return
        ################# PUBLIC ATTRIBUTES OF INTERFACE OBJECT #####################
        if (hasattr(ifspec, 'iid')):
            self.InterfaceId = int(ifspec.iid)
        else:
            self.InterfaceId = next(ResmgrClient[node].InterfaceIdAllocator)
        self.Ifname = spec.id
        self.Type = topo.MODE2INTF_TBL.get(spec.mode)
        self.AdminState = spec.status
        info = None
        self.lifns = getattr(spec, 'lifns', None)
        if utils.IsHostLifSupported() and self.lifns:
            self.obj_helper_lif = lif.LifObjectHelper(node)
            self.__create_lifs(spec)
        info = InterfaceInfoObject(node, self.Type, spec, ifspec)
        self.IfInfo = info
        self.Status = InterfaceStatus()
        self.GID("Interface ID:%d"%self.InterfaceId)
        self.UUID = utils.PdsUuid(self.InterfaceId, self.ObjType)
        self.Mutable = utils.IsUpdateSupported()
        self.UpdateImplicit()

        ################# PRIVATE ATTRIBUTES OF INTERFACE OBJECT #####################
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "Interface: %s|Origin:%d|Ifname:%s|Type:%d|AdminState:%s" % \
                (self.UUID, self.Origin, self.Ifname, self.Type, self.AdminState)

    def Show(self):
        logger.info("InterfaceObject:")
        logger.info("- %s" % repr(self))
        if self.IfInfo:
            self.IfInfo.Show()
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
            else:
                continue
            # Found matching interface, get basic info
            uuid_str = ifinst['meta']['uuid']
            self.UUID = utils.PdsUuid(bytes.fromhex(uuid_str.replace('-','')),\
                    self.ObjType)
            self.Tenant = ifinst['meta']['tenant']
            self.Namespace = ifinst['meta']['namespace']
            self.GID(ifinst['meta']['name'])

            # get ifinfo
            if hasattr(ifinst['spec'], 'ip-address'):
                self.IfInfo.ip_prefix = ipaddress.ip_network(ifinst['spec']['ip-address'],\
                        False)
            if hasattr(ifinst['spec'], 'vrf-name'):
                self.IfInfo.VrfName = ifinst['spec']['vrf-name']
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
        clone.IfInfo = copy.copy(self.IfInfo)
        return clone

    def UpdateAttributes(self):
        self.IfInfo.macaddr = ResmgrClient[self.Node].DeviceMacAllocator.get()
        return

    def RollbackAttributes(self):
        self.IfInfo.macaddr = self.GetPrecedent().IfInfo.macaddr
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        if self.Type == topo.InterfaceTypes.L3:
            spec.Type = interface_pb2.IF_TYPE_L3
            spec.L3IfSpec.PortId = self.IfInfo.Port.GetUuid()
            spec.L3IfSpec.MACAddress = self.IfInfo.macaddr.getnum()
            spec.L3IfSpec.VpcId = utils.PdsUuid.GetUUIDfromId(self.IfInfo.VpcId, api.ObjectTypes.VPC)
            utils.GetRpcIPPrefix(self.IfInfo.ip_prefix, spec.L3IfSpec.Prefix)
            # Metaswitch treats IPv4 address as an array - so requires it in Network order
            cp_utils.IPv4EndianReverse(spec.L3IfSpec.Prefix.Addr)

        if self.Type == topo.InterfaceTypes.LOOPBACK:
            spec.Type = interface_pb2.IF_TYPE_LOOPBACK
            utils.GetRpcIPPrefix(self.IfInfo.ip_prefix, spec.LoopbackIfSpec.Prefix)
            # Metaswitch treats IPv4 address as an array - so requires it in Network order
            cp_utils.IPv4EndianReverse(spec.LoopbackIfSpec.Prefix.Addr)
        return

    def PopulateAgentJson(self):
        if self.Type == topo.InterfaceTypes.LOOPBACK:
            iftype = 'LOOPBACK'
        elif self.Type == topo.InterfaceTypes.L3:
            iftype = 'L3'
        elif self.Type == topo.InterfaceTypes.ETH:
            iftype = 'HOST_PF'
        else:
            return None
        spec = {
            "kind": "Interface",
            "meta": {
                "name": self.GID(),
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
                "vrf-name": self.IfInfo.VrfName,
                "network": self.IfInfo.Network,
                "ip-address": self.IfInfo.PfxStr,
            }
        }
        return json.dumps(spec)

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.AdminStatus != interface_pb2.IF_STATUS_UP:
            return False
        if self.Type == topo.InterfaceTypes.L3:
            if spec.Type != interface_pb2.IF_TYPE_L3:
                return False
            if spec.L3IfSpec.PortId != self.IfInfo.Port.GetUuid():
                return False
            # TODO: Enable once device delete is fixed. MAC is also \
            # overwritten with 0 on deleting device config.
            #if spec.L3IfSpec.MACAddress != self.IfInfo.macaddr.getnum():
            #    return False
        return True

    def __create_lifs(self, spec):
        self.obj_helper_lif.Generate(spec.ifinfo, spec.lifspec, self.lifns)
        self.obj_helper_lif.Configure()
        self.lif = self.obj_helper_lif.GetRandomHostLif()
        logger.info(" Selecting %s for Test" % self.lif.GID())
        self.lif.Show()
        return

    def UpdateVrfAndNetwork(self, subnets):
        for subnet in subnets:
            if self.HostIfIdx == subnet.HostIfIdx:
                self.IfInfo.VrfName = subnet.VPC.GID()
                self.IfInfo.Network = subnet.GID()
                return True
        return False

class InterfaceObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.INTERFACE, Resmgr.MAX_INTERFACE)
        self.__uplinkl3ifs = defaultdict(dict)
        self.__loopback_if = defaultdict(dict)
        self.__uplinkl3ifs_iter = defaultdict(dict)
        self.__hostifs = defaultdict(dict)
        self.__hostifs_iter = defaultdict(dict)
        return

    def GetInterfaceObject(self, node, infid):
        return self.GetObjectByKey(node, infid)

    def GetHostInterface(self, node):
        if self.__hostifs[node]:
            return self.__hostifs_iter[node].rrnext()
        return None

    def GetHostInterfaceName(self, node, hostifindex):
        if self.__hostifs[node]:
            return self.__hostifs[node].get(hostifindex, None)
        return None

    def GetL3UplinkInterface(self, node):
        if self.__uplinkl3ifs[node]:
            return self.__uplinkl3ifs_iter[node].rrnext()
        return None

    def __generate_host_interfaces(self, node, ifspec):
        if not ifspec:
            return
        spec = InterfaceSpec_()
        spec.port = 0
        spec.ethifidx = 0
        spec.status = 'UP'
        spec.mode = 'host'
        spec.lifspec = ifspec.lif.Get(EzAccessStoreClient[node])
        for obj in ResmgrClient[node].HostIfs.values():
            spec.id = obj.IfName
            spec.ifinfo = obj
            lifstart = obj.LifBase
            lifend = lifstart + obj.LifCount - 1
            spec.lifns = objects.TemplateFieldObject("range/%d/%d" % (lifstart, lifend))
            ifobj = InterfaceObject(spec, ifspec, node=node)
            self.__hostifs[node].update({ifobj.InterfaceId: ifobj})

        if self.__hostifs[node]:
            self.__hostifs_iter[node] = utils.rrobiniter(self.__hostifs[node].values())
        return

    def __generate_iota_host_interfaces_map(self, node):
        resps = self.ReadLifs(node)

        if utils.IsDryRun():
            return
        for r in resps:
            if not utils.ValidateGrpcResponse(r):
                logger.error(f"INTERFACE LIF get request failed with {r}")
                continue
            for lif in r.Response:
                intf_type = lif.Spec.Type
                if intf_type != types_pb2.LIF_TYPE_HOST:
                    continue
                key = self.GetKeyfromSpec(lif.Spec)
                intf_name = lif.Status.Name
                self.__hostifs[node].update({key: intf_name})
        return

    def __generate_l3_uplink_interfaces(self, node, parent, iflist):
        uplink_ports = PortClient.Objects()
        if not uplink_ports:
            return

        for port in uplink_ports:
            spec = InterfaceSpec_()
            spec.mode = 'l3'
            spec.port = port.Port - 1
            spec.ethifidx = topo.PortToEthIfIdx(port.Port)
            spec.id = 'Uplink%d' % spec.port
            spec.status = port.AdminState
            for ifspec in iflist:
                if ifspec.iftype != 'l3':
                    continue
                if ifspec.portid == port.Port:
                    if (hasattr(ifspec, 'macaddress')):
                        spec.MACAddr = ifspec.macaddress
                    else:
                        spec.MACAddr = ResmgrClient[node].DeviceMacAllocator.get()
                    spec.vpcid = parent.VPCId
                    ifobj = InterfaceObject(spec, ifspec, node=node)
                    self.Objs[node].update({ifobj.InterfaceId: ifobj})
                    self.__uplinkl3ifs[node].update({ifobj.InterfaceId: ifobj})

        if self.__uplinkl3ifs[node]:
            self.__uplinkl3ifs_iter[node] = utils.rrobiniter(self.__uplinkl3ifs[node].values())
        return

    def GenerateHostInterfaces(self, node, topospec):
        if not utils.IsInterfaceSupported():
            return
        if not utils.IsDol():
            self.__generate_iota_host_interfaces_map(node)
            return
        hostifspec = getattr(topospec, 'hostinterface', None)
        if not hostifspec:
            return
        self.__generate_host_interfaces(node, hostifspec)
        return

    def __read_agent_loopback(self, node):
        resp = self.ReadAgentInterfaces(node)
        if utils.IsDryRun():
            return
        if not resp:
            return None
        for r in resp:
            if r['spec']['type'] == 'LOOPBACK':
                return r
        return

    def __generate_loopback_interfaces(self, node, parent, iflist):
        spec = InterfaceSpec_()
        spec.mode = 'loopback'
        spec.id = 0
        for ifspec in iflist:
            if ifspec.iftype != 'loopback':
                continue
            rdata = None
            if GlobalOptions.netagent:
                rdata = self.__read_agent_loopback(node)
                if rdata:
                    spec.id = rdata['meta']['name']
            if not rdata:
                # if netagent read fail, or we're in pds agent mode, use allocator
                spec.id = 'Loopback%d' % next(ResmgrClient[node].LoopbackIfIdAllocator)

            spec.status = ifspec.ifadminstatus
            ifobj = InterfaceObject(spec, ifspec, node=node, spec_json=rdata)
            self.Objs[node].update({ifobj.InterfaceId: ifobj})
            self.__loopback_if[node] = ifobj

    def GenerateObjects(self, node, parent, topospec):
        if not utils.IsL3InterfaceSupported():
            return
        iflist = getattr(topospec, 'interface', [])
        self.__generate_l3_uplink_interfaces(node, parent, iflist)
        self.__generate_loopback_interfaces(node, parent, iflist)
        return

    def AddObjToDict(self, obj, node):
        self.Objs[node].update({obj.InterfaceId: obj})
        return

    def DeleteObjFromDict(self, obj, node):
        self.Objs[node].pop(obj.InterfaceId, None)
        return

    def __get_first_host_if(self, node):
        resp = self.ReadAgentInterfaces(node)
        #if utils.IsDryRun():
            #return
        if not resp:
            return None
        for r in resp:
            if r['spec']['type'] == 'HOST_PF':
                utils.dump(r)
                return r
        return None

    def UpdateHostInterfaces(self, node, subnets):
        if GlobalOptions.dryrun:
            return 
        resp = api.client[node].GetHttp(api.ObjectTypes.INTERFACE)
        if not resp:
            return None
        for r in resp:
            if r['spec']['type'] == 'HOST_PF' and hasattr(r['status'], 'id'):
                ifspec = InterfaceSpec_()
                ifinfo = InterfaceSpec_()
                ifspec.origin = 'fixed'
                obj = InterfaceObject(None, ifspec, spec_json=r, node=node, \
                                      type=topo.InterfaceTypes.ETH)
                self.Objs[node].update({obj.InterfaceId: obj})
                if obj.UpdateVrfAndNetwork(subnets):
                    obj.Show()
                    api.client[node].Update(api.ObjectTypes.INTERFACE, [obj])
        print("after update pf")
        resp = api.client[node].GetHttp(api.ObjectTypes.INTERFACE)
        for r in resp:
            print(r)

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
            if EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled() and utils.IsDol():
                # create loopback interface
                lo_obj = self.__loopback_if[node]
                logger.info(f"Creating 1 Loopback {self.ObjType.name} Objects in {node}")
                lo_obj.Show()
                msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), [lo_obj]))
                api.client[node].Create(api.ObjectTypes.INTERFACE, msgs)
        else:
            obj = self.__loopback_if[node]
            obj.Show()
            api.client[node].Update(api.ObjectTypes.INTERFACE, [obj])
        return

    def GetGrpcReadAllLifMessage(self):
        grpcmsg = interface_pb2.LifGetRequest()
        return grpcmsg

    def ReadLifs(self, node):
        if utils.IsDryRun(): return
        msg = self.GetGrpcReadAllLifMessage()
        resp = api.client[node].Request(api.ObjectTypes.INTERFACE, 'LifGet', [msg])
        return resp

    def ReadAgentInterfaces(self, node):
        if utils.IsDryRun(): return
        if not GlobalOptions.netagent:
            return
        resp = api.client[node].GetHttp(api.ObjectTypes.INTERFACE)
        return resp

    def ReadObjects(self, node):
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
        return (numObjs == self.GetNumHwObjects(node))


client = InterfaceObjectClient()
def GetMatchingObjects(selectors):
    return client.Objects()

