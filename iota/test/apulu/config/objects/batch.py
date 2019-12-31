#! /usr/bin/python3
import enum
import pdb
import infra.config.base as base
import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.utils as utils
import iota.test.apulu.config.agent.api as api
import batch_pb2 as batch_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from iota.test.apulu.config.store import Store

INVALID_BATCH_COOKIE = 0

class BatchObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.GID('Batch')
        self.epoch = next(resmgr.EpochAllocator)
        self.cookie = INVALID_BATCH_COOKIE
        return

    def __repr__(self):
        return "Batch/epoch:%d/cookie:0x%x" % (self.epoch, self.cookie)

    def GetBatchSpec(self):
        batchspec = batch_pb2.BatchSpec()
        batchspec.epoch = self.epoch
        return batchspec

    def GetInvalidBatchSpec(self):
        batchspec = batch_pb2.BatchSpec()
        batchspec.epoch = 0
        return batchspec

    def GetBatchContext(self):
        batchctx = types_pb2.BatchCtxt()
        batchctx.BatchCookie = self.cookie
        return batchctx

    def GetBatchCookie(self):
        return self.cookie

    def SetBatchCookie(self, batchCookie):
        self.cookie = batchCookie

    def SetNextEpoch(self):
        self.epoch += 1
        return

    def Show(self):
        logger.info("Batch Object:", self)
        logger.info("- %s" % repr(self))
        return

class BatchObjectClient:
    def __init__(self):
        self.Objs = dict()
        # Temporary fix for artemis to generate flows and sessions for the
        # created mappings
        if utils.IsFlowInstallationNeeded():
            self.__commit_for_flows = True
        else:
            self.__commit_for_flows = False
        return

    def GetObjectByKey(self, key):
        return self.Objs.get(key, None)

    def Objects(self):
        return self.Objs.values()

    def GenerateObjects(self, node):
        obj = BatchObject()
        obj.Show()
        self.Objs.update({node: obj})
        return

    def __updateObject(self, node, batchStatus):
        if batchStatus is None:
            cookie = INVALID_BATCH_COOKIE
        else:
            cookie = batchStatus[0].BatchContext.BatchCookie
        logger.info("Setting Batch cookie to ", cookie)
        self.GetObjectByKey(node).SetBatchCookie(cookie)

    def Start(self, node):
        self.GetObjectByKey(node).SetNextEpoch()
        status = api.client[node].Start(api.ObjectTypes.BATCH, self.GetObjectByKey(node).GetBatchSpec())
        # update batch context
        self.__updateObject(node, status)
        return status

    def Commit(self, node):
        if self.__commit_for_flows:
            api.client.Start(api.ObjectTypes.BATCH, self.GetObjectByKey(node).GetInvalidBatchSpec())
            self.__commit_for_flows = False
        api.client[node].Commit(api.ObjectTypes.BATCH, self.GetObjectByKey(node).GetBatchContext())
        # invalidate batch context
        self.__updateObject(node, None)
        return

client = BatchObjectClient()
Store.SetBatchClient(client)
