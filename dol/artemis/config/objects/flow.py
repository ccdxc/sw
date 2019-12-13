#! /usr/bin/python3
import pdb
import copy

import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.route as routetable
import apollo.config.utils as utils
import apollo.config.topo as topo
import tunnel_pb2 as tunnel_pb2

from infra.common.logging import logger
from apollo.config.store import Store

# Flow based on Local and Remote mapping
class FlowMapObject(base.ConfigObjectBase):
    def __init__(self, lobj, robj, fwdmode, routeobj):
        super().__init__()
        self.Clone(Store.templates.Get('FLOW'))
        self.FlowMapId = next(resmgr.FlowIdAllocator)
        self.GID('FlowMap%d'%self.FlowMapId)
        self.FwdMode = fwdmode
        self.__lobj = lobj
        self.__robj = robj
        self.__dev = Store.GetDevice()
        self.__routeobj = routeobj
        self.Show()
        return

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.localmapping = self.__lobj
        obj.remotemapping = self.__robj
        obj.devicecfg = self.__dev
        obj.route = self.__routeobj
        obj.hostport = topo.PortTypes.HOST
        obj.switchport = topo.PortTypes.SWITCH
        return

    def __repr__(self):
        return "FlowMapId:%d" %(self.FlowMapId)

    def SetMeterStats(self, tc,  step):
        port = -1
        rxbytes = 0
        txbytes = 0
        if step.trigger.packets == None:
            return

        # TODO, Need to add mirror case and other cases
        for p in step.trigger.packets:
            if p.packet == None: return
            port = p.ports[0]
            # Received on host port and send out on switch port
            # So it is counted in transmit
            if port == tc.config.hostport:
                txbytes = len(p.packet.rawbytes)
                break

        for p in step.expect.packets:
            if p.packet == None: return
            port = p.ports[0]
            # Received on switch port and send out on host port
            # So it is counted in receive
            if port == tc.config.hostport:
                rxbytes = len(p.packet.rawbytes) - utils.GetVlanHeaderSize(p.packet) # Remove VLAN header
                break

        if tc.config.localmapping.AddrFamily == 'IPV6':
            tc.pvtdata.meterstats.IncrMeterStats(
                tc.connfig.localmapping.VNIC.V6MeterId, rxbytes, txbytes)
        else:
            tc.pvtdata.meterstats.IncrMeterStats(
                tc.config.localmapping.VNIC.V4MeterId, rxbytes, txbytes)


    def Show(self):
        def __show_flow_object(obj):
            if obj is not None:
                obj.Show()
            return
        logger.info("")
        logger.info("FlowMap Object: %s" % self)
        __show_flow_object(self.__lobj)
        if self.__robj:
            __show_flow_object(self.__robj)
        elif self.__routeobj:
            __show_flow_object(self.__routeobj)
        return

class FlowMapObjectHelper:
    def __init__(self):
        return

    def __is_lmapping_match(self, routetblobj, lobj):
        if lobj.AddrFamily == 'IPV4':
            return lobj.AddrFamily == routetblobj.AddrFamily and\
               lobj.VNIC.SUBNET.V4RouteTableId == routetblobj.RouteTblId
        if lobj.AddrFamily == 'IPV6':
            return lobj.AddrFamily == routetblobj.AddrFamily and\
               lobj.VNIC.SUBNET.V6RouteTableId == routetblobj.RouteTblId

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        fwdmode = None
        mapsel = selectors
        key = 'FwdMode'

        # Consider it only if TEST is for MAPPING
        if mapsel.flow.GetValueByKey('FlType') != 'MAPPING':
            return objs

        # Get the forwarding mode, fwdmode is not applicable for local & remote
        fwdmode = mapsel.flow.GetValueByKey(key)
        mapsel.flow.filters.remove((key, fwdmode))
        rmapsel = copy.deepcopy(mapsel)

        assert (fwdmode == 'VNET' or fwdmode == 'IGW' or fwdmode == 'SVCTUN' or\
                fwdmode == 'SVCTUN_REMOTE') == True

        if fwdmode == 'VNET':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for robj in rmapping.GetMatchingObjects(rmapsel):

                    # Ignore VPC-ID 1 , Reserved for substrate
                    if lobj.VNIC.SUBNET.VPC.VPCId == 1  or robj.SUBNET.VPC.VPCId == 1:
                        continue

                    if lobj.VNIC.SUBNET.VPC.VPCId != robj.SUBNET.VPC.VPCId:
                        continue

                    obj = FlowMapObject(lobj, robj, fwdmode, None)
                    objs.append(obj)

            return utils.GetFilteredObjects(objs, selectors.maxlimits)

        elif fwdmode == 'IGW':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                    if not self.__is_lmapping_match(routetblobj, lobj):
                        continue
                    obj = FlowMapObject(lobj, None, fwdmode, routetblobj)
                    objs.append(obj)

            return utils.GetFilteredObjects(objs, selectors.maxlimits)

        elif fwdmode == 'SVCTUN' or fwdmode == 'SVCTUN_REMOTE':
            for lobj in lmapping.GetMatchingObjects(mapsel):
                vpc = lobj.VNIC.SUBNET.VPC
                if vpc.Nat46_pfx is not None:
                    for routetblobj in routetable.GetAllMatchingObjects(mapsel):
                        if fwdmode == 'SVCTUN_REMOTE':
                            if routetblobj.TUNNEL.Remote is False:
                                continue
                        obj = FlowMapObject(lobj, None, fwdmode, routetblobj)
                        objs.append(obj)
            return utils.GetFilteredObjects(objs, selectors.maxlimits)

        else:
            assert 0

FlowMapHelper = FlowMapObjectHelper()

def GetMatchingObjects(selectors):
    return FlowMapHelper.GetMatchingConfigObjects(selectors)
