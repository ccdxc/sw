#! /usr/bin/python3
import pdb
import copy
from collections import defaultdict
import ipaddress

from infra.common.logging import logger
import infra.common.objects as objects

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

class InterfaceStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.INTERFACE)
        self.LifId = None
        return

    def Update(self, iftype, status):
        if iftype == topo.InterfaceTypes.UPLINK:
            self.LifId = status.UplinkIfStatus.LifId
        return

class InterfaceSpec_:
    pass

class InterfaceInfoObject(base.ConfigObjectBase):
    def __init__(self, iftype, spec, ifspec):
        self.__type = iftype
        if (iftype == topo.InterfaceTypes.UPLINK):
            self.port_num = getattr(spec, 'port', None)
        elif (iftype == topo.InterfaceTypes.UPLINKPC):
            self.port_bmap = getattr(spec, 'portbmp', None)
        elif (iftype == topo.InterfaceTypes.L3):
            if (hasattr(spec, 'vpcid')):
                self.VpcId = spec.vpcid
            else:
                self.VpcId = getattr(spec, 'vpc', 0)
            if (hasattr(ifspec, 'ipprefix')):
                self.ip_prefix = ipaddress.ip_network(ifspec.ipprefix.replace('\\', '/'), False)
            else:
                self.ip_prefix = next(ResmgrClient[node].L3InterfaceIPv4PfxPool)
            self.ethifidx = getattr(spec, 'ethifidx', -1)
            self.port_num = getattr(spec, 'port', -1)
            self.encap = getattr(spec, 'encap', None)
            self.macaddr = getattr(spec, 'MACAddr', None)

    def Show(self):
        if (self.__type == topo.InterfaceTypes.UPLINK):
            res = str("port num : %d" % int(self.port_num))
        elif (self.__type == topo.InterfaceTypes.UPLINKPC):
            res = str("port_bmap: %s" % self.port_bmap)
        elif (self.__type == topo.InterfaceTypes.L3):
            res = str("VPC:%d|ip:%s|ethifidx:%d|encap:%s|mac:%s"% \
                    (self.VpcId, self.ip_prefix, self.ethifidx, self.encap, \
                    self.macaddr.getnum()))
        else:
            return
        logger.info("- %s" % res)

class InterfaceObject(base.ConfigObjectBase):
    def __init__(self, spec, ifspec, node=None):
        super().__init__(api.ObjectTypes.INTERFACE, node)
        ################# PUBLIC ATTRIBUTES OF INTERFACE OBJECT #####################
        if (hasattr(ifspec, 'iid')):
            self.InterfaceId = ifspec.iid
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
        info = InterfaceInfoObject(self.Type, spec, ifspec)
        self.IfInfo = info
        self.Status = InterfaceStatus()
        self.GID("Interface ID:%s"%self.InterfaceId)
        self.UUID = utils.PdsUuid(self.InterfaceId)
        self.Mutable = utils.IsUpdateSupported()

        ################# PRIVATE ATTRIBUTES OF INTERFACE OBJECT #####################
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "Interface: %s |Ifname:%s|Type:%d|AdminState:%s" % \
                (self.UUID, self.Ifname, self.Type, self.AdminState)

    def Show(self):
        logger.info("InterfaceObject:")
        logger.info("- %s" % repr(self))
        if self.IfInfo:
            self.IfInfo.Show()
        return

    def CopyObject(self):
        clone = copy.copy(self)
        clone.IfInfo = copy.copy(self.IfInfo)
        return clone

    def UpdateAttributes(self):
        self.IfInfo.macaddr = ResmgrClient[node].DeviceMacAllocator.get()
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
            spec.L3IfSpec.EthIfIndex = self.IfInfo.ethifidx
            spec.L3IfSpec.MACAddress = self.IfInfo.macaddr.getnum()
            spec.L3IfSpec.VpcId = utils.PdsUuid.GetUUIDfromId(self.IfInfo.VpcId)
            utils.GetRpcIPPrefix(self.IfInfo.ip_prefix, spec.L3IfSpec.Prefix)
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.AdminStatus != interface_pb2.IF_STATUS_UP:
            return False
        if self.Type == topo.InterfaceTypes.L3:
            if spec.Type != interface_pb2.IF_TYPE_L3:
                return False
            if spec.L3IfSpec.EthIfIndex != self.IfInfo.ethifidx:
                return False
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

class InterfaceObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.INTERFACE)
        self.__uplinkl3ifs = defaultdict(dict)
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
            self.Objs[node].update({ifobj.InterfaceId: ifobj})
            self.__hostifs[node].update({ifobj.InterfaceId: ifobj})

        if self.__hostifs[node]:
            self.__hostifs_iter[node] = utils.rrobiniter(self.__hostifs[node].values())
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
        hostifspec = getattr(topospec, 'hostinterface', None)
        if not hostifspec:
            return
        self.__generate_host_interfaces(node, hostifspec)
        return

    def GenerateObjects(self, node, parent, topospec):
        if not utils.IsL3InterfaceSupported():
            return
        iflist = getattr(topospec, 'interface', [])
        self.__generate_l3_uplink_interfaces(node, parent, iflist)
        return

    def CreateObjects(self, node):
        cookie = utils.GetBatchCookie(node)
        if utils.IsL3InterfaceSupported():
            # create l3 if for uplink interface
            logger.info(f"Creating {len(self.__uplinkl3ifs[node])} L3 {self.ObjType.name} Objects in {node}")
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__uplinkl3ifs[node].values()))
            api.client[node].Create(api.ObjectTypes.INTERFACE, msgs)
        return

    def ReadObjects(self, node):
        msg = self.GetGrpcReadAllMessage(node)
        resp = api.client[node].Get(api.ObjectTypes.INTERFACE, [msg])
        result = self.ValidateObjects(resp, node)
        if result is False:
            logger.critical("INTERFACE object validation failed!!!")
            assert(0)
        return

    def ValidateObjects(self, getResp, node):
        if utils.IsDryRun(): return True
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("INTERFACE get request failed for ", obj)
                return False
            for resp in obj.Response:
                key = self.GetKeyfromSpec(resp.Spec)
                inf = self.GetInterfaceObject(node, key)
                if inf is not None:
                    if not utils.ValidateObject(inf, resp):
                        logger.error("INTERFACE validation failed for ", resp.Spec)
                        inf.Show()
                        return False
                    # update status for this interface object
                    inf.Status.Update(inf.Type, resp.Status)
        return True


client = InterfaceObjectClient()
def GetMatchingObjects(selectors):
    return client.Objects()

