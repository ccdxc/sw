#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base

import oper_pb2 as oper_pb2

class TechSupportStatus(base.StatusObjectBase):
    def __init__(self):
        self.FilePath = None
        return

    def Update(self, status):
        self.FilePath = getattr(status, 'FilePath', None)
        return

    def GetTechSupportFile(self):
        return self.FilePath

class TechSupportObject(base.ConfigObjectBase):
    def __init__(self, node):
        super().__init__(api.ObjectTypes.OPER, node)
        self.SetSingleton(True)
        self.GID("TechSupport")
        ############## PUBLIC ATTRIBUTES OF TechSupport OBJECT #################
        self.SkipCores = False
        ############## PRIVATE ATTRIBUTES OF TechSupport OBJECT ################
        self.Status = TechSupportStatus()
        self.Show()
        return

    def __repr__(self):
        return "TechSupport"

    def Show(self):
        logger.info(f"Oper Object: {self}")
        logger.info(f" - SkipCores:{self.SkipCores}")
        return

    def PopulateRequest(self, grpcmsg):
        spec = grpcmsg.Request
        spec.SkipCores = self.SkipCores
        return

    def ValidateResponse(self, resps):
        if utils.IsDryRun(): return None
        for r in resps:
            if not utils.ValidateGrpcResponse(r):
                logger.error(f"TechSupport request failed with {r}")
                continue
            status = r.Response
            self.Status.Update(status)
        return self.Status.GetTechSupportFile()

    def GetGrpcTechSupportCollectMessage(self):
        grpcmsg = oper_pb2.TechSupportRequest()
        self.PopulateRequest(grpcmsg)
        return grpcmsg

    def SetSkipCores(self, skipCores):
        self.SkipCores = skipCores

    def Collect(self):
        msg = self.GetGrpcTechSupportCollectMessage()
        resp = api.client[self.Node].Request(self.ObjType, 'TechSupportCollect', [msg])
        return self.ValidateResponse(resp)

class OperObjectsClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.OPER, Resmgr.MAX_OPER)
        self.TechSupportObjs = dict()

    def IsReadSupported(self):
        return False

    def GenerateTechSupportObjects(self, node):
        obj = TechSupportObject(node)
        self.TechSupportObjs[node] = obj
        self.Objs[node].update({obj.GID: obj})

    def GenerateObjects(self, node):
        self.GenerateTechSupportObjects(node)

    def GetTechSupportObject(self, node):
        return self.TechSupportObjs[node]

    def GetTechSupport(self, node):
        return self.TechSupportObjs[node].Collect()

client = OperObjectsClient()
