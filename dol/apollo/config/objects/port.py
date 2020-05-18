#! /usr/bin/python3
import pdb
import enum

from infra.common.logging import logger

from apollo.config.store import client as EzAccessStoreClient

from apollo.config.resmgr import client as ResmgrClient

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.objects.base as base

from apollo.oper.alerts import client as AlertsClient

import apollo.test.utils.pdsctl as pdsctl

import alerts_pb2 as alerts_pb2

class UplinkPorts(enum.IntEnum):
    # In DOL, it starts with 1
    UplinkPort0 = 1
    UplinkPort1 = 2

class PortObject(base.ConfigObjectBase):
    def __init__(self, node, port, mode, state='UP'):
        ################# PUBLIC ATTRIBUTES OF PORT OBJECT #####################
        self.Node = node
        self.PortId = next(ResmgrClient[node].PortIdAllocator)
        self.GID("Port ID:%s"%self.PortId)
        self.Port = port
        self.EthIfIndex = topo.PortToEthIfIdx(self.PortId)
        self.UUID = utils.PdsUuid(self.EthIfIndex)
        self.Mode = mode
        self.AdminState = state
        ################# PRIVATE ATTRIBUTES OF PORT OBJECT #####################
        self.Show()
        return

    def __repr__(self):
        return f"PortId:{self.PortId}|UUID:{self.UUID}"

    def Show(self):
        logger.info("PortObject:")
        logger.info("- %s" % repr(self))
        logger.info(f"- Mode: {self.Mode}")
        logger.info(f"- Port: {self.Port}")
        logger.info(f"- EthIfIndex: 0x{self.EthIfIndex:0x}")
        logger.info(f"- AdminState: {self.AdminState}")
        return

    def IsHostPort(self):
        return self.Mode == topo.PortTypes.HOST

    def IsSwitchPort(self):
        return self.Mode == topo.PortTypes.SWITCH

    def LinkDown(self, spec=None):
        self.AdminState = 'DOWN'
        cmd = f" port -p {self.UUID.String()} -a down "
        return pdsctl.UpdatePort(cmd)

    def LinkUp(self, spec=None):
        self.AdminState = 'UP'
        cmd = f" port -p {self.UUID.String()} -a up "
        return pdsctl.UpdatePort(cmd)

    def __validate_link_alert(self, alert):
        logger.info(f"Validating link alert {alert} against {self}")
        if not alert or not utils.ValidateGrpcResponse(alert):
            return False
        spec = alert.Response
        if spec.Name != f"LINK_{self.AdminState}":
            return False
        if spec.Category != "Network":
            return False
        if spec.Severity != alerts_pb2.INFO:
            return False
        if spec.Description != f"Port link is {self.AdminState.lower()}":
            return False
        if not self.UUID.String() in spec.Message:
            return False
        return True

    def VerifyLinkAlert(self, spec=None):
        alertsObj = AlertsClient.Objects(self.Node)
        try:
            alert = next(alertsObj.GetAlerts())
        except:
            logger.error(f"Got no alerts from {self.Node} for {self}")
            return True if utils.IsDryRun() else False
        return self.__validate_link_alert(alert)

    def SetupTestcaseConfig(self, obj):
        obj.root = self
        obj.alert = AlertsClient.Objects(self.Node)
        return

class PortObjectClient:
    def __init__(self):
        self.__objs = dict()
        return

    def Objects(self):
        return self.__objs.values()

    def GenerateObjects(self, node, topospec):
        def __get_port_mode(port, mode='auto'):
            if mode == 'switch':
                return topo.PortTypes.SWITCH
            elif mode == 'host':
                return topo.PortTypes.HOST
            if EzAccessStoreClient[node].IsHostMode():
                return topo.PortTypes.SWITCH
            elif EzAccessStoreClient[node].IsBitwMode():
                if port == UplinkPorts.UplinkPort0:
                    return topo.PortTypes.HOST
                elif port == UplinkPorts.UplinkPort1:
                    return topo.PortTypes.SWITCH
            return topo.PortTypes.NONE

        portlist = getattr(topospec, 'uplink', None)
        if portlist is None:
            return
        for spec in portlist:
            entryspec = spec.entry
            port = getattr(entryspec, 'port')
            mode = __get_port_mode(port, getattr(entryspec, 'mode', 'auto'))
            obj = PortObject(node, port, mode)
            self.__objs.update({obj.PortId: obj})
            if obj.IsHostPort():
                EzAccessStoreClient[node].SetHostPort(obj.Port)
            elif obj.IsSwitchPort():
                EzAccessStoreClient[node].SetSwitchPort(obj.Port)
        return

client = PortObjectClient()

def GetMatchingObjects(selectors):
    return client.Objects()
