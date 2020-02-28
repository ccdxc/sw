#! /usr/bin/python3
import pdb

from infra.common.logging import logger

from apollo.config.resmgr import client as ResmgrClient
from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.utils as utils
import apollo.config.objects.base as base
from apollo.config.store import EzAccessStore
import types_pb2 as types_pb2
import policer_pb2 as policer_pb2

class PolicerObject(base.ConfigObjectBase):
    def __init__(self, node, spec):
        super().__init__(api.ObjectTypes.POLICER, node)
        super().SetOrigin(spec)
        ############### PUBLIC ATTRIBUTES OF POLICER OBJECT ###################
        if hasattr(spec, 'id'):
            self.PolicerId = spec.id
        else:
            self.PolicerId = next(ResmgrClient[node].PolicerIdAllocator)
        self.type = spec.type
        self.direction = spec.direction
        self.rate = spec.rate
        self.burst = spec.burst
        self.GID('Policer%d'%self.PolicerId)
        self.UUID = utils.PdsUuid(self.PolicerId, api.ObjectTypes.POLICER)
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "Policer: %d|Type:%s|Dir:%s|Rate:%d|Burst:%d" %\
               (self.PolicerId, self.type, self.direction, self.rate, self.burst)

    def Show(self):
        logger.info("Policer object:", self)
        logger.info("- UUID: %s" % (self.UUID))
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        if (self.direction == 'ingress'):
            spec.Direction = policer_pb2.POLICER_DIR_INGRESS
        else:
            spec.Direction = policer_pb2.POLICER_DIR_EGRESS
        if (self.type == 'bps'):
            spec.BPSPolicer.BytesPerSecond = self.rate
            spec.BPSPolicer.Burst = self.burst
        else:
            spec.PPSPolicer.PacketsPerSecond = self.rate
            spec.PPSPolicer.Burst = self.burst
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if (self.direction == 'ingress'):
            if spec.Direction != policer_pb2.POLICER_DIR_INGRESS:
                return False
        else:
            if spec.Direction != policer_pb2.POLICER_DIR_EGRESS:
                return False
        if (self.type == 'bps'):
            if not hasattr(spec, 'BPSPolicer'):
                return False
            if ((spec.BPSPolicer.BytesPerSecond != self.rate) or\
                (spec.BPSPolicer.Burst != self.burst)):
                return False
        else:
            if not hasattr(spec, 'PPSPolicer'):
                return False
            if ((spec.PPSPolicer.PacketsPerSecond != self.rate) or\
                (spec.PPSPolicer.Burst != self.burst)):
                return False
        return True

    def ValidateYamlSpec(self, spec):
        if (utils.GetYamlSpecAttr(spec, 'id') != self.GetKey()):
            return False
        if spec['direction'] != self.direction:
            return False
        if spec['type'] != self.type:
            return False
        if spec['rate'] != spec.rate:
            return False
        if spec['burst'] != spec.burst:
            return False
        return True

class PolicerObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.POLICER, (2 * Resmgr.MAX_POLICER_PER_DIRECTION))
        self.rxpolicercount = 0
        self.txpolicercount = 0
        return

    def GenerateObjects(self, node, topospecobj):
        if not hasattr(topospecobj, 'policer'):
            return

        for policer_spec_obj in topospecobj.policer:
            obj = PolicerObject(node, policer_spec_obj)
            self.Objs[node].update({obj.PolicerId: obj})
            if (policer_spec_obj.direction == 'ingress'):
                self.rxpolicercount += 1
            else:
                self.txpolicercount += 1
        return

    def GetPolicerObject(self, node, policerid):
        resp = self.GetObjectByKey(node, policerid)
        return resp

    def IsValidConfig(self, node):
        if self.rxpolicercount > Resmgr.MAX_POLICER_PER_DIRECTION:
            return False, "Ingress policer count has exceeded limit of %d" %\
                                (Resmgr.MAX_POLICER_PER_DIRECTION)
        if self.txpolicercount > Resmgr.MAX_POLICER_PER_DIRECTION:
            return False, "Egress policer count has exceeded limit of %d" %\
                                (Resmgr.MAX_POLICER_PER_DIRECTION)
        return True, ""

client = PolicerObjectClient()
