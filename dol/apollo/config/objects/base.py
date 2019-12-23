#! /usr/bin/python3

from collections import defaultdict

from infra.common.logging import logger
import infra.config.base as base

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.topo as topo

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
    def __init__(self, objtype):
        super().__init__()
        self.Origin = topo.OriginTypes.FIXED
        self.HwHabitant = True
        self.ObjType = objtype
        self.Children = []
        self.Deps = defaultdict(list)
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

    def AddChild(self, child):
        self.Children.append(child);

    def GetDependees(self):
        # returns the list of dependees
        dependees = []
        return dependees

    def BuildDependency(self):
        dependees = self.GetDependees()
        for dependee in dependees:
            # add ourself as an dependent to dependee
            dependee.AddDependent(self)
        return

    def AddDependent(self, dep):
        self.Deps[dep.ObjType].append(dep)

    def DeriveOperInfo(self):
        self.BuildDependency()
        return

    def SetHwHabitant(self, value):
        self.HwHabitant = value

    def IsHwHabitant(self):
        return self.HwHabitant

    def SetOrigin(self, origintype):
        self.Origin = origintype

    def IsOriginFixed(self):
        return True if (self.Origin == topo.OriginTypes.FIXED) else False

    def Create(self, spec=None):
        utils.CreateObject(self)
        return

    def Read(self, spec=None):
        return utils.ReadObject(self)

    def Update(self, spec=None):
        return utils.UpdateObject(self)

    def Delete(self, spec=None):
        utils.DeleteObject(self)
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

    def GetGrpcUpdateMessage(self, cookie=0):
        grpcmsg = self.__get_GrpcMsg(api.ApiOps.UPDATE)
        self.__populate_BatchContext(grpcmsg, cookie)
        self.PopulateSpec(grpcmsg)
        return grpcmsg

    def GetGrpcDeleteMessage(self, cookie=0):
        grpcmsg = self.__get_GrpcMsg(api.ApiOps.DELETE)
        self.__populate_BatchContext(grpcmsg, cookie)
        self.PopulateKey(grpcmsg)
        return grpcmsg

class ConfigClientBase(base.ConfigClientBase):
    def __init__(self, objtype, maxlimit=0):
        super().__init__()
        self.Objs = dict()
        self.ObjType = objtype
        self.Maxlimit = maxlimit
        return

    def IsValidConfig(self):
        count = self.GetNumObjects()
        if  count > self.Maxlimit:
            return False, "%s count %d exceeds allowed limit of %d" % \
                          (self.ObjType, count, self.Maxlimit)
        return True, ""

    def GetKeyfromSpec(self, spec, yaml=False):
        if yaml: return spec['id']
        return spec.Id

    def Objects(self):
        return self.Objs.values()

    def GetNumHwObjects(self):
        count = len(self.Objects())
        # TODO can be improved, if object has a reference to gen object
        for obj in self.Objects():
            if (obj.HwHabitant == False):
                count = count - 1
        return count

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
        numObjs = 0
        for obj in getResp:
            if not utils.ValidateGrpcResponse(obj):
                logger.error("GRPC get request failed for ", obj)
                return False
            for resp in obj.Response:
                numObjs += 1
                key = self.GetKeyfromSpec(resp.Spec)
                cfgObj = self.GetObjectByKey(key)
                if not utils.ValidateObject(cfgObj, resp):
                    logger.error("GRPC read validation failed for ", obj)
                    cfgObj.Show()
                    return False
                if hasattr(cfgObj, 'Status'):
                    cfgObj.Status.Update(resp.Status)

        assert(numObjs == self.GetNumHwObjects())
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
        assert((len(cmdop) - 1) == self.GetNumHwObjects())
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

    def ReadObjects(self):
        logger.info("Reading %s Objects" % (self.ObjType.name))
        self.GrpcRead()
        self.PdsctlRead()
        return

    def CreateObjects(self):
        fixed, discovered = [], []
        for obj in self.Objects():
            (fixed if obj.IsOriginFixed() else discovered).append(obj)

        logger.info("%s objects: fixed: %d discovered %d" %(self.ObjType.name, len(fixed), len(discovered)))
        # set HwHabitant to false for discovered objects
        for obj in discovered:
            obj.SetHwHabitant(False)

        # return if there is no fixed object
        if len(fixed) == 0:
            return

        self.ShowObjects()
        logger.info("Creating %s Objects in agent" % (self.ObjType.name))
        cookie = utils.GetBatchCookie()
        msgs = list(map(lambda x: x.GetGrpcCreateMessage(cookie), fixed))
        api.client.Create(self.ObjType, msgs)
        #TODO: Add validation for create
        return
