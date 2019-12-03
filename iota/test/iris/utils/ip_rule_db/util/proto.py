#!/usr/bin/python3

import random

PROTO_MIN = 0
PROTO_MAX = 256

l3_proto_dict = {
    "icmp" : 1,
    "tcp"  : 6,
    "udp"  : 17,
    "any"  : 256
}

def validate(value):
    return isinstance(value, int) and \
        PROTO_MIN <= value <= PROTO_MAX

class Proto:

    def __init__(self, string):
        try:
            self.l3Proto = int(string)
            self.disp = string
            return None
        except:
            pass

        try:
            self.l3Proto = l3_proto_dict[string.lower()]
            self.disp = string
        except:
            raise ValueError("Invalid l3 protocol %s"%string)

    @property
    def l3Proto(self):
        return self._l3Proto

    @l3Proto.setter
    def l3Proto(self, value):
        if not validate(value):
            raise ValueError("Invalid l3 protocol %s"%value)
        self._l3Proto = value

    def __eq__(self, obj):
        return isinstance(obj, Proto) and \
            self.l3Proto == obj.l3Proto

    def __contains__(self, protocol):
        try:
            protocol = int(protocol)
            return isinstance(protocol, int) and \
                validate(protocol) and \
                (self.isAny() or \
                 protocol == self.l3Proto)
        except:
            return False

    def isAny(self):
        return self.l3Proto == l3_proto_dict["any"]

    def __str__(self):
        return str(self.disp)

    def getRandom(self, seed=None):
        return  l3_proto_dict["icmp"] if self.isAny() else self.l3Proto

    @staticmethod
    def getStrFromL3Proto(l3Proto):
        if isinstance(l3Proto, str):
            l3Proto = int(l3Proto)

        if not isinstance(l3Proto, int):
            return ""

        for k,v in l3_proto_dict.items():
            if v == l3Proto:
                return k

        return ""
