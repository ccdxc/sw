#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.objects.base as base

import types_pb2 as types_pb2

class AlertsObject(base.ConfigObjectBase):
    def __init__(self, node):
        super().__init__(api.PenOperObjectTypes.ALERTS, node)
        self.SetSingleton(True)
        self.GID(f"Alerts-{self.Node}")
        self.Stream = None
        self.Show()
        return

    def __repr__(self):
        return self.GID

    def Show(self):
        logger.info(f"Alerts Object: {self}")
        return

    def GetGrpcAlertsMessage(self):
        grpcmsg = types_pb2.Empty()
        return grpcmsg

    def RequestAlertsStream(self):
        msg = self.GetGrpcAlertsMessage()
        response = api.penOperClient[self.Node].Request(self.ObjType, 'AlertsGet', [msg])
        self.Stream = response[0] if response else None
        logger.info(f"received alerts stream {self.Stream} from {self.Node}")

    def GetAlerts(self):
        if not self.Stream:
            return None
        for alert in self.Stream:
            yield alert

    def DrainAlerts(self, spec=None):
        try:
            alerts = self.GetAlerts()
            while True:
                logger.info(f"Draining old alerts, {next(alerts)}")
        except:
            pass
        return True

class AlertsObjectsClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.PenOperObjectTypes.ALERTS, Resmgr.MAX_OPER)
        # one alert object per node
        self.Objs = dict()

    def Objects(self, node):
        return self.Objs.get(node, None)

    def IsReadSupported(self):
        return False

    def GenerateObjects(self, node):
        obj = AlertsObject(node)
        self.Objs[node] = obj

    def CreateObjects(self, node):
        logger.info(f"Starting {self.ObjType.name} stream from {node}")
        # start alert streams request
        obj = self.Objects(node)
        if not obj:
            return False
        obj.RequestAlertsStream()
        return True

client = AlertsObjectsClient()
