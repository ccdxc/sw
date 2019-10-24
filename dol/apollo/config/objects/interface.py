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

import interface_pb2 as interface_pb2

class InterfaceSpec_:
    pass

class InterfaceInfoObject(base.ConfigObjectBase):
    def __init__(self, iftype, ifinfo):
        super().__init__()
        self.__type = iftype
        if (iftype == utils.InterfaceTypes.UPLINK):
            self.port_num = ifinfo
        elif (iftype == utils.InterfaceTypes.UPLINKPC):
            self.port_bmap = ifinfo
        elif (iftype == utils.InterfaceTypes.L3INTERFACE):
            self.VPC = ifinfo.VPC
            self.ip_prefix = ifinfo.ip_prefix
            self.port_num = ifinfo.port_num
            self.encap = ifinfo.encap
            self.macaddr = ifinfo.macaddr

    def __repr__(self):
        return "Interface InfoType:%d" % (self.__type)

    def Show(self):
        res = ""
        logger.info("- %s" % repr(self))
        if (self.__type == utils.InterfaceTypes.UPLINK):
            res = str("port num : %d" % int(self.port_num))
        elif (self.__type == utils.InterfaceTypes.UPLINKPC):
            res = str("port_bmap: %s" % self.port_bmap)
        elif (self.__type == utils.InterfaceTypes.L3INTERFACE):
            res = str("VPC:%d|ip:%s|port_num:%d|encap:%d|mac:%s"% \
                    (self.VPC, self.ip_prefix, self.port_num, self.encap, \
                    self.macaddr))
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
        port = getattr(spec, 'port', None)
        if port:
            info = InterfaceInfoObject(self.Type, port)
        # TODO: Add support for UPLINKPC and L3_INTERFACE
        self.IfInfo = info
        self.GID("Interface ID:%s"%self.InterfaceId)
        ################# PRIVATE ATTRIBUTES OF INTERFACE OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return "InterfaceId:%d|Type:%d|AdminState:%s" % \
                (self.InterfaceId, self.Type, self.AdminState)

    def Show(self):
        logger.info("InterfaceObject:")
        logger.info("- %s" % repr(self))
        if self.IfInfo:
            logger.info("- %s" % self.IfInfo.Show())
        return

    def SetBaseClassAttr(self):
        self.ObjType = api.ObjectTypes.INTERFACE
        return

    def __create_lifs(self, spec):
        self.obj_helper_lif.Generate(spec.devcmd_addr, spec.lifspec, self.lifns)
        self.obj_helper_lif.Configure()
        self.lif = self.obj_helper_lif.GetRandomHostLif()
        logger.info(" Selecting %s for Test" % self.lif.GID())
        self.lif.Show()
        return

class InterfaceObjectClient:
    def __init__(self):
        self.__objs = dict()
        self.__hostifs = dict()
        self.__hostifs_iter = None
        return

    def Objects(self):
        return self.__objs.values()

    def GetHostInterface(self):
        if self.__hostifs:
            return self.__hostifs_iter.rrnext()
        return None

    def __generate_host_interfaces(self, ifspec):
        if not ifspec:
            return
        spec = InterfaceSpec_()
        spec.port = 0
        spec.status = 'UP'
        spec.mode = 'host'
        spec.lifspec = ifspec.lif.Get(Store)
        lifbase = ifspec.lifbase
        lifcount = ifspec.lifcount
        for i in range(ifspec.count):
            spec.id = "eth%d" % (i)
            spec.lifbase = lifbase
            spec.lifcount =lifcount
            spec.lifns = objects.TemplateFieldObject("range/%d/%d"%(lifbase, lifbase+lifcount-1))
            lifbase = lifbase + lifcount
            spec.devcmd_addr = resmgr.HostIntf2DevCmdAddrMap.get(spec.id, None)
            ifobj = InterfaceObject(spec)
            self.__objs.update({ifobj.InterfaceId: ifobj})
            self.__hostifs.update({ifobj.InterfaceId: ifobj})

        if self.__hostifs:
            self.__hostifs_iter = utils.rrobiniter(self.__hostifs.values())
        return

    def GenerateObjects(self, topospec):
        if utils.IsInterfaceSupported() is False:
            return
        iflist = getattr(topospec, 'interface', None)
        if iflist:
            hostifspec = getattr(iflist, 'host', None)
            self.__generate_host_interfaces(hostifspec)
        return

    def GetGrpcReadAllMessage(self):
        grpcmsg = interface_pb2.InterfaceGetRequest()
        return grpcmsg

    def CreateObjects(self):
        if utils.IsInterfaceSupported() is False:
            return
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(), self.__objs.values()))
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

