#! /usr/bin/python3

from collections import defaultdict

from infra.common.logging import logger
import infra.config.base as base

import iota.test.apulu.config.agent.api as api
import iota.test.apulu.config.utils as utils

import iota.test.apulu.utils.pdsctl as pdsctl

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
    def __init__(self, objtype):
        super().__init__()
        self.deleted = False
        self.ObjType = objtype
        #TODO: Add node as part of config object
        return

    def __get_GrpcMsg(self, node, op):
        grpcreq = api.client[node].GetGRPCMsgReq(self.ObjType, op)
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

    def GetGrpcCreateMessage(self, node, cookie=0):
        grpcmsg = self.__get_GrpcMsg(node, api.ApiOps.CREATE)
        self.__populate_BatchContext(grpcmsg, cookie)
        self.PopulateSpec(grpcmsg)
        return grpcmsg

    def GetGrpcReadMessage(self, node):
        grpcmsg = self.__get_GrpcMsg(node, api.ApiOps.GET)
        self.PopulateKey(grpcmsg)
        return grpcmsg

    def GetGrpcDeleteMessage(self, node, cookie=0):
        grpcmsg = self.__get_GrpcMsg(node, api.ApiOps.DELETE)
        self.__populate_BatchContext(grpcmsg, cookie)
        self.PopulateKey(grpcmsg)
        return grpcmsg

class ConfigClientBase(base.ConfigClientBase):
    def __init__(self, objtype, maxlimit=0):
        super().__init__()
        self.Objs = defaultdict(dict)
        self.ObjType = objtype
        self.Maxlimit = maxlimit
        return

    def IsValidConfig(self, node):
        count = self.GetNumObjects(node)
        if  count > self.Maxlimit:
            return False, "%s count %d exceeds allowed limit of %d" % \
                          (self.ObjType, count, self.Maxlimit)
        return True, ""

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml: return spec['id']
        return spec.Id

    def Objects(self, node):
        if self.Objs.get(node, None):
            return self.Objs[node].values()
        return []

    def GetNumObjects(self, node):
        return len(self.Objects(node))

    def GetObjectByKey(self, node, key):
        return self.Objs[node].get(key, None)

    def GetObjectType(self):
        return self.ObjType

    def ShowObjects(self, node):
        for obj in self.Objects(node):
            obj.Show()
        return

    def __get_GrpcMsg(self, node, op):
        grpcreq = api.client[node].GetGRPCMsgReq(self.ObjType, op)
        if grpcreq is None:
            logger.error("GRPC req method not added for obj:%s op:%s" %(self.ObjType, op))
            assert(0)
        return grpcreq()

    def GetGrpcReadAllMessage(self, node):
        grpcmsg = self.__get_GrpcMsg(node, api.ApiOps.GET)
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
                if hasattr(cfgObj, 'Status'):
                    cfgObj.Status.Update(resp.Status)
        return True

    def GrpcRead(self, node):
        # read all via grpc
        msg = self.GetGrpcReadAllMessage(node)
        resp = api.client[node].Get(self.ObjType, [msg])
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
            yamlOp = utils.LoadYaml(op)
            if not yamlOp:
                continue
            key = self.GetKeyfromSpec(yamlOp['spec'], yaml=True)
            cfgObj = self.GetObjectByKey(key)
            if not utils.ValidateObject(cfgObj, yamlOp, yaml=True):
                logger.error("pdsctl read validation failed for ", op)
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

    def ReadObjects(self, node):
        logger.info("Reading %s Objects" % (self.ObjType.name))
        self.GrpcRead()
        self.PdsctlRead()
        return

    def CreateObjects(self, node):
        self.ShowObjects(node)
        logger.info("Creating %s Objects in agent" % (self.ObjType.name))
        cookie = utils.GetBatchCookie(node)
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(node, cookie), self.Objects(node)))
        api.client[node].Create(self.ObjType, msgs)
        #TODO: Add validation for create
        return
