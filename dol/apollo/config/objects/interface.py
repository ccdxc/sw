#! /usr/bin/python3
import pdb
import copy

from infra.common.logging import logger
import infra.common.objects as objects

from apollo.config.store import EzAccessStore

import apollo.config.resmgr as resmgr
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
            self.LifId = status.UplinkStatus.LifId
        return

class InterfaceSpec_:
    pass

class InterfaceInfoObject(base.ConfigObjectBase):
    def __init__(self, iftype, spec):
        self.__type = iftype
        if (iftype == topo.InterfaceTypes.UPLINK):
            self.port_num = getattr(spec, 'port', None)
        elif (iftype == topo.InterfaceTypes.UPLINKPC):
            self.port_bmap = getattr(spec, 'portbmp', None)
        elif (iftype == topo.InterfaceTypes.L3):
            self.VPC = getattr(spec, 'vpc', 0)
            self.ip_prefix = getattr(spec, 'ippfx', None)
            self.port_num = getattr(spec, 'port', -1)
            self.encap = getattr(spec, 'encap', None)
            self.macaddr = getattr(spec, 'MACAddr', None)

    def Show(self):
        if (self.__type == topo.InterfaceTypes.UPLINK):
            res = str("port num : %d" % int(self.port_num))
        elif (self.__type == topo.InterfaceTypes.UPLINKPC):
            res = str("port_bmap: %s" % self.port_bmap)
        elif (self.__type == topo.InterfaceTypes.L3):
            res = str("VPC:%d|ip:%s|port_num:%d|encap:%s|mac:%s"% \
                    (self.VPC, self.ip_prefix, self.port_num, self.encap, \
                    self.macaddr))
        else:
            return
        logger.info("- %s" % res)

class InterfaceObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__(api.ObjectTypes.INTERFACE)
        ################# PUBLIC ATTRIBUTES OF INTERFACE OBJECT #####################
        self.InterfaceId = next(resmgr.InterfaceIdAllocator)
        self.Ifname = spec.id
        self.Type = topo.MODE2INTF_TBL.get(spec.mode)
        self.AdminState = spec.status
        info = None
        self.lifns = getattr(spec, 'lifns', None)
        if utils.IsHostLifSupported() and self.lifns:
            self.obj_helper_lif = lif.LifObjectHelper()
            self.__create_lifs(spec)
        info = InterfaceInfoObject(self.Type, spec)
        self.IfInfo = info
        self.Status = InterfaceStatus()
        self.GID("Interface ID:%s"%self.InterfaceId)
        self.Mutable = utils.IsUpdateSupported()

        ################# PRIVATE ATTRIBUTES OF INTERFACE OBJECT #####################
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "InterfaceId:%d|Ifname:%s|Type:%d|AdminState:%s" % \
                (self.InterfaceId, self.Ifname, self.Type, self.AdminState)

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
        self.IfInfo.macaddr = resmgr.DeviceMacAllocator.get()
        return

    def RollbackAttributes(self):
        self.IfInfo.macaddr = self.GetPrecedent().IfInfo.macaddr
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.InterfaceId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.InterfaceId
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        if self.Type == topo.InterfaceTypes.L3:
            spec.Type = interface_pb2.IF_TYPE_L3
            spec.L3IfSpec.PortId = self.IfInfo.port_num
            spec.L3IfSpec.MACAddress = self.IfInfo.macaddr.getnum()
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.InterfaceId:
            return False
        if spec.AdminStatus != interface_pb2.IF_STATUS_UP:
            return False
        if self.Type == topo.InterfaceTypes.L3:
            if spec.Type != interface_pb2.IF_TYPE_L3:
                return False
            if spec.L3IfSpec.PortId != self.IfInfo.port_num:
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
        self.__uplinkl3ifs = dict()
        self.__uplinkl3ifs_iter = None
        self.__hostifs = dict()
        self.__hostifs_iter = None
        return

    def GetInterfaceObject(self, infid):
        return self.GetObjectByKey(infid)

    def GetHostInterface(self):
        Interface = None
        if self.__hostifs:
            try:
                Interface = next(self.__hostifs_iter)
            except:
                Interface = None
        return Interface

    def GetL3UplinkInterface(self):
        if self.__uplinkl3ifs:
            return self.__uplinkl3ifs_iter.rrnext()
        return None

    def __generate_host_interfaces(self, ifspec):
        if not ifspec:
            return
        spec = InterfaceSpec_()
        spec.port = 0
        spec.status = 'UP'
        spec.mode = 'host'
        spec.lifspec = ifspec.lif.Get(EzAccessStore)
        for obj in resmgr.HostIfs.values():
            spec.id = obj.IfName
            spec.ifinfo = obj
            lifstart = obj.LifBase
            lifend = lifstart + obj.LifCount - 1
            spec.lifns = objects.TemplateFieldObject("range/%d/%d" % (lifstart, lifend))
            ifobj = InterfaceObject(spec)
            self.Objs.update({ifobj.InterfaceId: ifobj})
            self.__hostifs.update({ifobj.InterfaceId: ifobj})

        if self.__hostifs:
            self.__hostifs_iter = iter(self.__hostifs.values())
        return

    def __generate_l3_uplink_interfaces(self):
        uplink_ports = PortClient.Objects()
        if not uplink_ports:
            return
        for port in uplink_ports:
            spec = InterfaceSpec_()
            spec.mode = 'l3'
            spec.port = port.Port - 1
            spec.id = 'Uplink%d' % spec.port
            spec.status = port.AdminState
            spec.MACAddr = resmgr.DeviceMacAllocator.get()
            ifobj = InterfaceObject(spec)
            self.Objs.update({ifobj.InterfaceId: ifobj})
            self.__uplinkl3ifs.update({ifobj.InterfaceId: ifobj})

        if self.__uplinkl3ifs:
            self.__uplinkl3ifs_iter = utils.rrobiniter(self.__uplinkl3ifs.values())
        return

    def GenerateObjects(self, topospec):
        if utils.IsInterfaceSupported() is False:
            return
        iflist = getattr(topospec, 'interface', None)
        if iflist:
            hostifspec = getattr(iflist, 'host', None)
            self.__generate_host_interfaces(hostifspec)
        if utils.IsL3InterfaceSupported():
            # generate l3 if for uplink interface
            self.__generate_l3_uplink_interfaces()
        return

    def CreateObjects(self):
        cookie = utils.GetBatchCookie()
        if utils.IsL3InterfaceSupported():
            # create l3 if for uplink interface
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__uplinkl3ifs.values()))
            api.client.Create(api.ObjectTypes.INTERFACE, msgs)
        return

    def ReadObjects(self):
        msg = self.GetGrpcReadAllMessage()
        resp = api.client.Get(api.ObjectTypes.INTERFACE, [msg])
        result = self.ValidateObjects(resp)
        if result is False:
            logger.critical("INTERFACE object validation failed!!!")
            assert(0)
        return

    def ValidateObjects(self, getResp):
        if utils.IsDryRun(): return True
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("INTERFACE get request failed for ", obj)
                return False
            for resp in obj.Response:
                spec = resp.Spec
                inf = self.GetInterfaceObject(spec.Id)
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

