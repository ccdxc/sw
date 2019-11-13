#! /usr/bin/python3
from infra.common.logging import logger
import infra.config.base as base

import apollo.config.agent.api as api
import apollo.config.utils as utils

import types_pb2 as types_pb2

class ConfigObjectBase(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.deleted = False
        self.ObjType = api.ObjectTypes.NONE
        return

    def __get_GrpcMsg(self, op):
        grpcreq = api.client.GetGRPCMsgReq(self.ObjType, op)
        if grpcreq is None:
            logger.error("GRPC req method not added for obj:%s op:%s" %(self.ObjType, op))
            assert(0)
        return grpcreq()

    def __populate_BatchContext(self, grpcmsg, cookie):
        grpcmsg.BatchCtxt.BatchCookie = cookie
        return

    def Create(self, spec=None):
        utils.CreateObject(self, self.ObjType)
        return

    def Read(self, expRetCode=types_pb2.API_STATUS_OK):
        return utils.ReadObject(self, self.ObjType, expRetCode)

    def ReadAfterDelete(self, spec=None):
        return self.Read(types_pb2.API_STATUS_NOT_FOUND)

    def Delete(self, spec=None):
        utils.DeleteObject(self, self.ObjType)
        return

    def ValidateSpec(self, spec):
        logger.error("Method not implemented by class: %s" % self.__class__)
        assert(0)
        return False

    def ValidateStats(self, stats):
        return True

    def ValidateStatus(self, status):
        return True

    def Equals(self, obj, spec):
        return True

    def MarkDeleted(self, flag=True):
        self.deleted = flag
        return

    def IsDeleted(self):
        return self.deleted

    def SetBaseClassAttr(self):
        logger.error("Method not implemented by class: %s" % self.__class__)
        assert(0)
        return

    def PopulateKey(self, grpcmsg):
        logger.error("Method not implemented by class: %s" % self.__class__)
        assert(0)
        return

    def PopulateSpec(self, grpcmsg):
        logger.error("Method not implemented by class: %s" % self.__class__)
        assert(0)
        return

    def GetGrpcCreateMessage(self, cookie=0):
        grpcmsg = self.__get_GrpcMsg(api.ApiOps.CREATE)
        self.__populate_BatchContext(grpcmsg, cookie)
        self.PopulateSpec(grpcmsg)
        return grpcmsg

    def GetGrpcReadMessage(self):
        grpcmsg = self.__get_GrpcMsg(api.ApiOps.GET)
        self.PopulateKey(grpcmsg)
        return grpcmsg

    def GetGrpcDeleteMessage(self, cookie=0):
        grpcmsg = self.__get_GrpcMsg(api.ApiOps.DELETE)
        self.__populate_BatchContext(grpcmsg, cookie)
        self.PopulateKey(grpcmsg)
        return grpcmsg
