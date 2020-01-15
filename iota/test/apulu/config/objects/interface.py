#! /usr/bin/python3
import pdb
from collections import defaultdict

from infra.common.logging import logger
import infra.common.objects as objects

from iota.test.apulu.config.store import Store

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.objects.base as base
import iota.test.apulu.config.objects.host.lif as lif
from iota.test.apulu.config.objects.port import client as PortClient
import ipaddress

import interface_pb2 as interface_pb2

class InterfaceStatus(base.StatusObjectBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.INTERFACE)
        self.LifId = None
        return

    def Update(self, iftype, status):
        if iftype == utils.InterfaceTypes.UPLINK:
            self.LifId = status.UplinkIfStatus.LifId
        return

class InterfaceSpec_:
    pass

class InterfaceInfoObject(base.ConfigObjectBase):
    def __init__(self, iftype, spec, ifspec):
        self.__type = iftype
        if (iftype == utils.InterfaceTypes.UPLINK):
            self.port_num = getattr(spec, 'port', None)
        elif (iftype == utils.InterfaceTypes.UPLINKPC):
            self.port_bmap = getattr(spec, 'portbmp', None)
        elif (iftype == utils.InterfaceTypes.L3):
            if (hasattr(spec, 'vpcid')):
                self.VPC = spec.vpcid
            else:
                self.VPC = getattr(spec, 'vpc', 0)
            if (hasattr(ifspec, 'ipprefix')):
                self.ip_prefix = ipaddress.ip_network(ifspec.ipprefix.replace('\\', '/'), False)
            else:
                self.ip_prefix = getattr(spec, 'ippfx', None)
            self.ethifidx = getattr(spec, 'ethifidx', -1)
            self.port_num = getattr(spec, 'port', -1)
            self.encap = getattr(spec, 'encap', None)
            self.macaddr = getattr(spec, 'MACAddr', None)

    def Show(self):
        if (self.__type == utils.InterfaceTypes.UPLINK):
            res = str("port num : %d" % int(self.port_num))
        elif (self.__type == utils.InterfaceTypes.UPLINKPC):
            res = str("port_bmap: %s" % self.port_bmap)
        elif (self.__type == utils.InterfaceTypes.L3):
            res = str("VPC:%d|ip:%s|ethifidx:%d|encap:%s|mac:%s"% \
                    (self.VPC, self.ip_prefix, self.ethifidx, self.encap, \
                     self.macaddr.get()))
        else:
            return
        logger.info("- %s" % res)

class InterfaceObject(base.ConfigObjectBase):
    def __init__(self, spec, ifspec):
        super().__init__(api.ObjectTypes.INTERFACE)
        ################# PUBLIC ATTRIBUTES OF INTERFACE OBJECT #####################
        if (hasattr(ifspec, 'iid')):
            self.InterfaceId = ifspec.iid
        else:
            self.InterfaceId = next(resmgr.InterfaceIdAllocator)
        self.Ifname = spec.id
        self.Type = utils.MODE2INTF_TBL.get(spec.mode)
        self.AdminState = spec.status
        info = None
        self.lifns = getattr(spec, 'lifns', None)
        if utils.IsHostLifSupported() and self.lifns:
            self.obj_helper_lif = lif.LifObjectHelper()
            self.__create_lifs(spec)
        info = InterfaceInfoObject(self.Type, spec, ifspec)
        self.IfInfo = info
        self.Status = InterfaceStatus()
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

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(str.encode(str(self.InterfaceId)))
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = str.encode(str(self.InterfaceId))
        spec.AdminStatus = interface_pb2.IF_STATUS_UP
        if self.Type == utils.InterfaceTypes.L3:
            spec.Type = interface_pb2.IF_TYPE_L3
            spec.L3IfSpec.EthIfIndex = self.IfInfo.ethifidx
            spec.L3IfSpec.MACAddress = self.IfInfo.macaddr.getnum()
        return

    def ValidateSpec(self, spec):
        if int(spec.Id) != self.InterfaceId:
            return False
        if spec.AdminStatus != interface_pb2.IF_STATUS_UP:
            return False
        if self.Type == utils.InterfaceTypes.L3:
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

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml:
            return utils.GetYamlSpecAttr(spec, 'id')
        return int(spec.Id)

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
        spec.lifspec = ifspec.lif.Get(Store)
        for obj in resmgr.HostIfs.values():
            spec.id = obj.IfName
            spec.ifinfo = obj
            lifstart = obj.LifBase
            lifend = lifstart + obj.LifCount - 1
            spec.lifns = objects.TemplateFieldObject("range/%d/%d" % (lifstart, lifend))
            ifobj = InterfaceObject(spec)
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
            spec.ethifidx = utils.PortToEthIfIdx(port.Port)
            spec.id = 'Uplink%d' % spec.port
            spec.status = port.AdminState
            for ifspec in iflist:
               if ifspec.portid == port.Port:
                  if (hasattr(ifspec, 'macaddress')):
                      spec.MACAddr = ifspec.macaddress
                  else:
                      spec.MACAddr = resmgr.DeviceMacAllocator.get()
                  spec.vpcid = parent.VPCId
                  ifobj = InterfaceObject(spec, ifspec)
                  self.Objs[node].update({ifobj.InterfaceId: ifobj})
                  self.__uplinkl3ifs[node].update({ifobj.InterfaceId: ifobj})

        if self.__uplinkl3ifs[node]:
            logger.info(" In uplinkl3ifs generate-objects node %s size %d" % (node, len(self.__uplinkl3ifs[node])))
            self.__uplinkl3ifs_iter[node] = utils.rrobiniter(self.__uplinkl3ifs[node].values())
        return

    def GenerateObjects(self, node, parent, topospec):
        if utils.IsInterfaceSupported() is False:
            return
        iflist = getattr(topospec, 'interface', None)
        if iflist:
            hostifspec = getattr(iflist, 'host', None)
            self.__generate_host_interfaces(node, hostifspec)
            if utils.IsL3InterfaceSupported():
                 # generate l3 if for uplink interface
                self.__generate_l3_uplink_interfaces(node, parent, iflist)
        return

    def CreateObjects(self, node):
        cookie = utils.GetBatchCookie(node)
        if utils.IsL3InterfaceSupported():
            # create l3 if for uplink interface
            logger.info(" Interface createObjects node %s: count %d" % (node, len(self.__uplinkl3ifs[node])))
            logger.info("Creating L3 interface Objects in agent")
            msgs = list(map(lambda x: x.GetGrpcCreateMessage(node, cookie), self.__uplinkl3ifs[node].values()))
            api.client[node].Create(api.ObjectTypes.INTERFACE, msgs)
        return

    def ReadObjects(self, node):
        msg = self.GetGrpcReadAllMessage(node)
        resp = api.client[node].Get(api.ObjectTypes.INTERFACE, [msg])
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

