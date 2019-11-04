#! /usr/bin/python3
import pdb

from infra.common.logging import logger
import infra.common.objects as objects

from apollo.config.store import Store

import apollo.config.resmgr as resmgr
import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base
import apollo.config.objects.host.lif as lif
from apollo.config.objects.port import client as PortClient

import interface_pb2 as interface_pb2

class InterfaceSpec_:
    pass

class InterfaceInfoObject(base.ConfigObjectBase):
    def __init__(self, iftype, spec):
        super().__init__()
        self.__type = iftype
        if (iftype == utils.InterfaceTypes.UPLINK):
            self.port_num = getattr(spec, 'port', None)
        elif (iftype == utils.InterfaceTypes.UPLINKPC):
            self.port_bmap = getattr(spec, 'portbmp', None)
        elif (iftype == utils.InterfaceTypes.L3):
            self.VPC = getattr(spec, 'vpc', 0)
            self.ip_prefix = getattr(spec, 'ippfx', None)
            self.port_num = getattr(spec, 'port', -1)
            self.encap = getattr(spec, 'encap', None)
            self.macaddr = getattr(spec, 'MACAddr', None)

    def Show(self):
        if (self.__type == utils.InterfaceTypes.UPLINK):
            res = str("port num : %d" % int(self.port_num))
        elif (self.__type == utils.InterfaceTypes.UPLINKPC):
            res = str("port_bmap: %s" % self.port_bmap)
        elif (self.__type == utils.InterfaceTypes.L3):
            res = str("VPC:%d|ip:%s|port_num:%d|encap:%s|mac:%s"% \
                    (self.VPC, self.ip_prefix, self.port_num, self.encap, \
                    self.macaddr))
        else:
            return
        logger.info("- %s" % res)

class InterfaceObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.SetBaseClassAttr()
        ################# PUBLIC ATTRIBUTES OF INTERFACE OBJECT #####################
        self.InterfaceId = next(resmgr.InterfaceIdAllocator)
        self.Ifname = spec.id
        self.Type = utils.MODE2INTF_TBL.get(spec.mode)
        self.AdminState = spec.status
        info = None
        self.lifns = getattr(spec, 'lifns', None)
        if utils.IsHostLifSupported() and self.lifns:
            self.obj_helper_lif = lif.LifObjectHelper()
            self.__create_lifs(spec)
        info = InterfaceInfoObject(self.Type, spec)
        # TODO: Add support for UPLINKPC and L3_INTERFACE
        self.IfInfo = info
        self.GID("Interface ID:%s"%self.InterfaceId)
        ################# PRIVATE ATTRIBUTES OF INTERFACE OBJECT #####################
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

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.INTERFACE
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.InterfaceId)
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.InterfaceId
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        if self.Type == utils.InterfaceTypes.L3:
            spec.Type = interface_pb2.IF_TYPE_L3
            spec.L3IfSpec.PortId = self.IfInfo.port_num
            spec.L3IfSpec.MACAddress = self.IfInfo.macaddr.getnum()
        return

    def __create_lifs(self, spec):
        self.obj_helper_lif.Generate(spec.ifinfo, spec.lifspec, self.lifns)
        self.obj_helper_lif.Configure()
        self.lif = self.obj_helper_lif.GetRandomHostLif()
        logger.info(" Selecting %s for Test" % self.lif.GID())
        self.lif.Show()
        return

class InterfaceObjectClient:
    def __init__(self):
        self.__objs = dict()
        self.__uplinkl3ifs = dict()
        self.__uplinkl3ifs_iter = None
        self.__hostifs = dict()
        self.__hostifs_iter = None
        return

    def Objects(self):
        return self.__objs.values()

    def GetHostInterface(self):
        if self.__hostifs:
            return self.__hostifs_iter.rrnext()
        return None

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
        spec.lifspec = ifspec.lif.Get(Store)
        for obj in resmgr.HostIfs.values():
            spec.id = obj.IfName
            spec.ifinfo = obj
            lifstart = obj.LifBase
            lifend = lifstart + obj.LifCount - 1
            spec.lifns = objects.TemplateFieldObject("range/%d/%d" % (lifstart, lifend))
            ifobj = InterfaceObject(spec)
            self.__objs.update({ifobj.InterfaceId: ifobj})
            self.__hostifs.update({ifobj.InterfaceId: ifobj})

        if self.__hostifs:
            self.__hostifs_iter = utils.rrobiniter(self.__hostifs.values())
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
            self.__objs.update({ifobj.InterfaceId: ifobj})
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

    def GetGrpcReadAllMessage(self):
        grpcmsg = interface_pb2.InterfaceGetRequest()
        return grpcmsg

    def CreateObjects(self):
        if utils.IsInterfaceSupported() is False:
            return
        cookie = utils.GetBatchCookie()
        if utils.IsL3InterfaceSupported():
            # create l3 if for uplink interface
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), self.__uplinkl3ifs.values()))
            api.client.Create(api.ObjectTypes.INTERFACE, msgs)
        return

    def ReadObjects(self):
        if utils.IsInterfaceSupported() is False:
            return
        msg = self.GetGrpcReadAllMessage()
        api.client.Get(api.ObjectTypes.INTERFACE, [msg])
        return

client = InterfaceObjectClient()
def GetMatchingObjects(selectors):
    return client.Objects()

