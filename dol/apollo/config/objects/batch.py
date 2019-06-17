#! /usr/bin/python3
import enum
import pdb
import infra.config.base as base
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils
import apollo.config.agent.api as api
import batch_pb2 as batch_pb2
import types_pb2 as types_pb2

from infra.common.logging import logger
from apollo.config.store import Store

class BatchObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.GID('Batch')
        self.epoch = next(resmgr.EpochAllocator)
        return

    def __repr__(self):
        return "Batch/epoch:%d" % self.epoch

    def GetBatchSpec(self):
        batchspec = batch_pb2.BatchSpec()
        batchspec.epoch = self.epoch
        return batchspec

    def GetInvalidBatchSpec(self):
        batchspec = batch_pb2.BatchSpec()
        batchspec.epoch = 0
        return batchspec

    def SetNextEpoch(self):
        self.epoch += 1
        return

    def Show(self):
        logger.info("Batch Object:", self)
        logger.info("- %s" % repr(self))
        return
    
class BatchObjectClient:
    def __init__(self):
        self.__obj = None
        # Temporary fix for artemis to generate flows and sessions for the
        # created mappings
        if utils.IsPipelineArtemis():
            self.__commit_for_flows = True
        else:
            self.__commit_for_flows = False
        return

    def Objects(self):
        return self.__obj

    def GenerateObjects(self, topospec):
        self.__obj = BatchObject()
        self.__obj.Show()
        return

    def Start(self):
        self.__obj.SetNextEpoch()
        api.client.Start(api.ObjectTypes.BATCH, self.__obj.GetBatchSpec())
        return

    def Commit(self):
        if self.__commit_for_flows:
            api.client.Start(api.ObjectTypes.BATCH, self.__obj.GetInvalidBatchSpec())
            self.__commit_for_flows = False
        api.client.Commit(api.ObjectTypes.BATCH, self.__obj.GetBatchSpec())
        return

client = BatchObjectClient()
