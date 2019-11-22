#! /usr/bin/python3
import yaml

from infra.common.logging import logger
import infra.config.base as base

import apollo.config.agent.api as api
import apollo.config.utils as utils

import apollo.test.utils.pdsctl as pdsctl

import types_pb2 as types_pb2

class StatusObjectBase(base.StatusObjectBase):
    def __init__(self, objtype):
        super().__init__()
        self.ObjType = objtype
        self.HwId = None
        return

    def GetHwId(self):
        return self.HwId

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

    def ValidateYamlSpec(self, spec):
        logger.error("Method not implemented by class: %s" % self.__class__)
        assert(0)
        return False

    def ValidateYamlStats(self, stats):
        return True

    def ValidateYamlStatus(self, status):
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

class ConfigClientBase(base.ConfigClientBase):
    def __init__(self, objtype):
        super().__init__()
        self.Objs = dict()
        self.ObjType = objtype
        return

    def Objects(self):
        return self.Objs.values()

    def GetNumObjects(self):
        return len(self.Objects())

    def GetObjectByKey(self, key):
        return self.Objs.get(key, None)

    def GetObjectType(self):
        return self.ObjType

    def ShowObjects(self):
        for obj in self.Objects():
            obj.Show()
        return

    def __get_GrpcMsg(self, op):
        grpcreq = api.client.GetGRPCMsgReq(self.ObjType, op)
        if grpcreq is None:
            logger.error("GRPC req method not added for obj:%s op:%s" %(self.ObjType, op))
            assert(0)
        return grpcreq()

    def GetGrpcReadAllMessage(self):
        grpcmsg = self.__get_GrpcMsg(api.ApiOps.GET)
        return grpcmsg

    def ValidateGrpcRead(self, getResp):
        if utils.IsDryRun(): return True
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("GRPC get request failed for ", obj)
                return False
            for resp in obj.Response:
                key = self.GetKeyfromSpec(resp.Spec)
                cfgObj = self.GetObjectByKey(key)
                if not utils.ValidateObject(cfgObj, resp):
                    logger.error("GRPC read validation failed for ", obj)
                    cfgObj.Show()
                    return False
        return True

    def GrpcRead(self):
        # read all via grpc
        msg = self.GetGrpcReadAllMessage()
        resp = api.client.Get(self.ObjType, [msg])
        if not self.ValidateGrpcRead(resp):
            logger.critical("Object validation failed for %s" % (self.ObjType))
            assert(0)
        return

    def ValidatePdsctlRead(self, ret, stdout):
        if utils.IsDryRun(): return True
        if not ret:
            logger.error("pdsctl show cmd failed for ", self.ObjType)
            return False
        # split output per object
        cmdop = stdout.split("---")
        for op in cmdop:
            yamlOp = yaml.load(op, Loader=yaml.FullLoader)
            if not yamlOp:
                continue
            key = yamlOp['spec']['id']
            cfgObj = self.GetObjectByKey(key)
            if not utils.ValidateObject(cfgObj, yamlOp, yaml=True):
                logger.error("GRPC read validation failed for ", op)
                cfgObj.Show()
                return False
        return True

    def PdsctlRead(self):
        # read all via pdsctl
        ret, op = pdsctl.GetObjects(self.ObjType)
        if not self.ValidatePdsctlRead(ret, op):
            logger.critical("Object validation failed for ", self.ObjType, ret, op)
            assert(0)
        return

    def ReadObjects(self):
        self.GrpcRead()
        self.PdsctlRead()
        return
