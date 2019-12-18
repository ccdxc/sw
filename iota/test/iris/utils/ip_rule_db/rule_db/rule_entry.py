#!/usr/bin/python3

import app.app as app

from util.ip import *
from util.port import *
from util.proto import *
from enum import Enum, auto

HINT_MATCH_SIP = 1
HINT_MATCH_DIP = 2
HINT_MATCH_PROTO = 4

class ACTION(Enum):
    PERMIT = auto()
    DENY = auto()
    REJECT = auto()
    LOG = auto()

class RuleEntry:
    def __init__(self, sip="any", dip="any", \
                 proto="any", identifier=0,
                 action="PERMIT", **kwargs):

        # statistics
        self.hint = 0
        self.counter = 0
        self.identifier = identifier
        self._app = None

        # Match fields
        self.sip = sip
        self.dip = dip
        self.proto = proto
        self.appName = kwargs.get('appName', None)
        self.action = action

        self.findApp(**kwargs)

    @property
    def sip(self):
        return self._sip

    @sip.setter
    def sip(self, param):
        try:
            value = IP(param)
            if value.ipType == IPType.IP_ANY:
                self.hint &= ~HINT_MATCH_SIP
            else:
                self.hint |= HINT_MATCH_SIP
            self._sip = value
        except:
            raise ValueError("Invalid src ip %s"%param)

    @property
    def dip(self):
        return self._dip

    @dip.setter
    def dip(self, param):
        try:
            value = IP(param)
            if value.ipType == IPType.IP_ANY:
                self.hint &= ~HINT_MATCH_DIP
            else:
                self.hint |= HINT_MATCH_DIP

            self._dip = value
        except:
            raise ValueError("Invalid dst ip %s"%param)

    @property
    def proto(self):
        return self._proto

    @proto.setter
    def proto(self, param):
        try:
            value = Proto(param)

            if value.isAny():
                self.hint &= ~HINT_MATCH_PROTO
            else:
                self.hint |= HINT_MATCH_PROTO

            self._proto = value
        except:
            raise ValueError("Invalid protocol %s"%param)

    @property
    def action(self):
        return self._action

    @action.setter
    def action(self, param):
        try:
            self._action = ACTION[param]
        except:
            raise ValueError("Invalid action %s"%param)

    @property
    def identifier(self):
        return self._identifier

    @identifier.setter
    def identifier(self, value):
        if not isinstance(value, int):
            raise ValueError("Invalid identifier %s"%value)
        self._identifier = value

    @property
    def appName(self):
        return self._appName

    @appName.setter
    def appName(self, name):
        self._appName = name

    def findApp(self, **kwargs):
        self._app = app.getAppInstance(self.appName, self.proto, **kwargs)

    def __eq__(self, obj):
        return  self.hint == obj.hint and \
            self.sip == obj.sip and \
            self.dip == obj.dip and\
            self.proto == obj.proto and \
            self.appName == self.appName and\
            self._app == obj._app and \
            self.action == obj.action

    def __str__(self):
        return "[%s], sip='%s', dip='%s', proto='%s', app=%s [%s], action='%s', hint=%s, counter=%s"%\
            (self.identifier, self.sip, self.dip, self.proto, self.appName, self._app, \
             self.action.name, self.printHint(), self.counter)

    def printHint(self):
        hintList = []

        if self.hint & HINT_MATCH_SIP:
            hintList.append("SIP")

        if self.hint & HINT_MATCH_DIP:
            hintList.append("DIP")

        if self.hint & HINT_MATCH_PROTO:
            hintList.append("PROTO")

        return '[' + ', '.join(hintList) + ']'

    def match(self, **kwargs):
        sip = kwargs.get("sip", None)
        dip = kwargs.get("dip", None)
        proto = kwargs.get("proto", None)

        if not sip:
            logger.warning("While matching Rule Entry, Source IP field 'sip' is must")
            return False

        if not dip:
            logger.warning("While matching Rule Entry, Destination IP field 'dip' is must")
            return False

        if not isinstance(proto, int):
            logger.warning("While matching Rule Entry, Protocol field 'proto' is must")
            return False

        if self.hint & HINT_MATCH_SIP and \
           sip not in self.sip:
            return False

        if self.hint & HINT_MATCH_DIP and \
           dip not in self.dip:
            return False

        if self.hint & HINT_MATCH_PROTO and \
           proto not in self.proto:
            return False

        if self._app and not self._app.match(**kwargs):
            return False

        self.counter += 1

        return True

    def applyFilter(self, **kwarg):
        for k,v in kwarg.items():

            if k == "sip" and v not in self.sip:
                return False

            if k == "dip" and v not in self.dip:
                return False

            if k == "proto" and v not in self.proto:
                return False

            if k == "action" and v != self.action.name:
                return False

        if self._app and not self._app.applyFilter(**kwarg):
            return False

        return True

    def getRandom(self, seed=None):
        rand = {
            "sip"   : self.sip.getRandom(seed=seed),
            "dip"   : self.dip.getRandom(seed=seed),
            "proto" : self.proto.getRandom(seed=seed)
        }

        if self._app:
            rand.update(self._app.getRandom(seed=seed))

        return rand

    @staticmethod
    def parseFromDict(ruleEntryDict, appDict=None):
        '''
        refer : nic/agent/protos/generated/restclient/docs/swagger/sgpolicy.swagger.json
        '''

        srcProtoPort = []
        dstProtoPort = []
        kwargsList   = []
        sip = "any"
        dip = "any"

        source      = ruleEntryDict.get("source", None)
        destination = ruleEntryDict.get('destination', None)
        action      = ruleEntryDict.get("action", "PERMIT")
        identifier  = int(ruleEntryDict.get("rule-id", 0))
        appName     = ruleEntryDict.get("app-name", "")

        if len(appName) != 0:
            raise ValueError("APPs are not supported in current release")

        if source:
            sip = ','.join(source.get("addresses", ["any"]))
            src_app_configs = source.get("proto-ports", [])
            for config in src_app_configs:
                srcProtoPort.append((config.get("protocol", "any"),
                                     config.get("port", "any")))

        if destination:
            dip = ','.join(destination.get("addresses", ["any"]))
            dst_app_configs = destination.get("proto-ports", [])
            for config in dst_app_configs:
                dstProtoPort.append((config.get("protocol", "any"),
                                     config.get("port", "any")))

        if len(srcProtoPort) == 0 and len(dstProtoPort) == 0:
            kwargsList.append({
                "sip":sip,       "dip":dip,
                "action":action, "identifier":identifier
            })

        elif len(srcProtoPort) and len(dstProtoPort) == 0:
            for proto, sp in srcProtoPort:
                kwargsList.append({
                    "sip":sip,       "dip":dip,
                    "proto":proto,   "sp": sp,
                    "action":action, "identifier":identifier
                })

        elif len(srcProtoPort) == 0 and len(dstProtoPort):
            for proto, dp in dstProtoPort:
                kwargsList.append({
                    "sip":sip,       "dip":dip,
                    "proto":proto,   "dp": dp,
                    "action":action, "identifier":identifier
                })
        else:
            for _,sp in srcProtoPort:
                for proto,dp in dstProtoPort:
                    kwargsList.append({
                        "sip":sip,       "dip":dip,
                        "sp":sp,         "dp": dp,
                        "proto":proto,
                        "action":action, "identifier":identifier
                    })
        return [RuleEntry(**kwarg) for kwarg in kwargsList]
