#! /usr/bin/python3
import pdb
from infra.common.logging import logger
import infra.common.objects as objects

from apollo.config.resmgr import Resmgr

import apollo.config.agent.api as api
import apollo.config.objects.base as base
import apollo.config.topo as topo
import apollo.config.utils as utils

import types_pb2 as types_pb2

class SecurityProfileObject(base.ConfigObjectBase):
    def __init__(self, node, spec):
        super().__init__(api.ObjectTypes.SECURITY_PROFILE, node)
        if hasattr(spec, 'origin'):
            self.SetOrigin(spec.origin)
        ############### PUBLIC ATTRIBUTES OF SECURITY PROFILE OBJECT ###############
        self.SetSingleton(True)
        if (hasattr(spec, 'id')):
            self.SecurityProfileId = spec.id
        else:
            self.SecurityProfileId = 1
        self.GID('SecurityProfile%d'%self.SecurityProfileId)
        self.UUID = utils.PdsUuid(self.SecurityProfileId, self.ObjType)
        self.ConnTrackEn = getattr(spec, 'conntrack', False)
        self.DefaultFWAction = getattr(spec, 'deffwaction', 
                                       topo.SecurityRuleActionType.ALLOW)
        self.TCPIdleTimeout = getattr(spec, 'tcpidletimeout', 600)
        self.UDPIdleTimeout = getattr(spec, 'udpidletimeout', 120)
        self.ICMPIdleTimeout = getattr(spec, 'icmpidletimeout', 15)
        self.OtherIdleTimeout = getattr(spec, 'otheridletimeout', 90)
        self.TCPCnxnSetupTimeout = getattr(spec, 'tcpsyntimeout', 10)
        self.TCPHalfCloseTimeout = getattr(spec, 'tcphalfclosetimeout', 120)
        self.TCPCloseTimeout = getattr(spec, 'tcpclosetimeout', 15)
        self.TCPDropTimeout = getattr(spec, 'tcpdroptimeout', 90)
        self.UDPDropTimeout = getattr(spec, 'udpdroptimeout', 60)
        self.ICMPDropTimeout = getattr(spec, 'icmpdroptimeout', 30)
        self.OtherDropTimeout = getattr(spec, 'otherdroptimeout', 60)

       ############### PRIVATE ATTRIBUTES OF SECURITY_PROFILE OBJECT ###############
        self.__spec = spec
        self.DeriveOperInfo()
        self.Show()
        return

    def __repr__(self):
        return "SecurityProfile1: |ConnTrackEn: %s |DefaultFWAction: %s "\
               "|TCPIdleTimeout: %s |UDPIdleTimeout: %s |ICMPIdleTimeout: %s "\
               "|OtherIdleTimeout: %s |TCPCnxnSetupTimeout: %s |TCPHalfCloseTimeout: %s "\
               "|TCPCloseTimeout: %s |TCPDropTimeout: %s |UDPDropTimeout: %s "\
               "|ICMPDropTimeout: %s |OtherDropTimeout %s" %\
               (self.ConnTrackEn, self.DefaultFWAction, self.TCPIdleTimeout,
                self.UDPIdleTimeout, self.ICMPIdleTimeout, self.OtherIdleTimeout,
                self.TCPCnxnSetupTimeout, self.TCPHalfCloseTimeout, self.TCPCloseTimeout,
                self.TCPDropTimeout, self.UDPDropTimeout, self.ICMPDropTimeout,
                self.OtherDropTimeout)

    def Show(self):
        logger.info("Security Profile Object: %s" % self)
        logger.info("- %s" % repr(self))
        return

    def PopulateKey(self, grpcmsg):
        grpcmsg.Id.append(self.GetKey())
        return

    def PopulateSpec(self, grpcmsg):
        spec = grpcmsg.Request.add()
        spec.Id = self.GetKey()
        spec.ConnTrackEn = self.ConnTrackEn
        spec.DefaultFWAction = self.DefaultFWAction
        spec.TCPIdleTimeout = self.TCPIdleTimeout
        spec.UDPIdleTimeout = self.UDPIdleTimeout
        spec.ICMPIdleTimeout = self.ICMPIdleTimeout
        spec.OtherIdleTimeout = self.OtherIdleTimeout
        spec.TCPCnxnSetupTimeout = self.TCPCnxnSetupTimeout
        spec.TCPHalfCloseTimeout = self.TCPHalfCloseTimeout
        spec.TCPCloseTimeout = self.TCPCloseTimeout
        spec.TCPDropTimeout = self.TCPDropTimeout
        spec.UDPDropTimeout = self.UDPDropTimeout
        spec.ICMPDropTimeout = self.ICMPDropTimeout
        spec.OtherDropTimeout = self.OtherDropTimeout
        return

    def ValidateSpec(self, spec):
        if spec.Id != self.GetKey():
            return False
        if spec.ConnTrackEn != self.ConnTrackEn:
            return False
        if spec.DefaultFWAction != self.DefaultFWAction:
            return False
        if spec.TCPIdleTimeout != self.TCPIdleTimeout:
            return False
        if spec.UDPIdleTimeout != self.UDPIdleTimeout:
            return False
        if spec.ICMPIdleTimeout != self.ICMPIdleTimeout:
            return False
        if spec.OtherIdleTimeout != self.OtherIdleTimeout:
            return False
        if spec.TCPCnxnSetupTimeout != self.TCPCnxnSetupTimeout:
            return False
        if spec.TCPHalfCloseTimeout != self.TCPHalfCloseTimeout:
            return False
        if spec.TCPCloseTimeout != self.TCPCloseTimeout:
            return False
        if spec.TCPDropTimeout != self.TCPDropTimeout:
            return False
        if spec.UDPDropTimeout != self.UDPDropTimeout:
            return False
        if spec.ICMPDropTimeout != self.ICMPDropTimeout:
            return False
        if spec.OtherDropTimeout != self.OtherDropTimeout:
            return False
        return True

    def ValidateYamlSpec(self, spec):
        if utils.GetYamlSpecAttr(spec) != self.GetKey():
            return False
        if spec[ 'ConnTrackEn' ] != self.ConnTrackEn:
            return False
        if spec[ 'DefaultFWAction' ] != self.DefaultFWAction:
            return False
        if spec[ 'TCPIdleTimeout' ] != self.TCPIdleTimeout:
            return False
        if spec[ 'UDPIdleTimeout' ] != self.UDPIdleTimeout:
            return False
        if spec[ 'ICMPIdleTimeout' ] != self.ICMPIdleTimeout:
            return False
        if spec[ 'OtherIdleTimeout' ] != self.OtherIdleTimeout:
            return False
        if spec[ 'TCPCnxnSetupTimeout' ] != self.TCPCnxnSetupTimeout:
            return False
        if spec[ 'TCPHalfCloseTimeout' ] != self.TCPHalfCloseTimeout:
            return False
        if spec[ 'TCPCloseTimeout' ] != self.TCPCloseTimeout:
            return False
        if spec[ 'TCPDropTimeout' ] != self.TCPDropTimeout:
            return False
        if spec[ 'UDPDropTimeout' ] != self.UDPDropTimeout:
            return False
        if spec[ 'ICMPDropTimeout' ] != self.ICMPDropTimeout:
            return False
        if spec[ 'OtherDropTimeout' ] != self.OtherDropTimeout:
            return False
        return True

    def GetGrpcReadMessage(self):
        grpcmsg = types_pb2.Empty()
        return grpcmsg

class SecurityProfileObjectClient(base.ConfigClientBase):
    def __init__(self):
        super().__init__(api.ObjectTypes.SECURITY_PROFILE, Resmgr.MAX_SECURITY_PROFILE)
        return

    def IsReadSupported(self):
        # Enable once SecurityPolicySvcImpl::SecurityProfileGet is implemented
        return False

    def GenerateObjects(self, node, topospec):
        if (hasattr(topospec, 'security_profile')):
            obj = SecurityProfileObject(node, topospec.security_profile)
            self.Objs[node].update({0: obj})
        return

    def GetGrpcReadAllMessage(self, node):
        grpcmsg = types_pb2.Empty()
        return grpcmsg

client = SecurityProfileObjectClient()
