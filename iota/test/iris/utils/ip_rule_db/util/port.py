#!/usr/bin/python3

import random
from enum import Enum, auto

PORT_MIN = 0
PORT_MAX = 65535


def validate(value):
    return  isinstance(value, int) and PORT_MIN <= value <= PORT_MAX

class PORT_TYPE(Enum):
    PORT_NUM = auto()
    PORT_RANGE = auto()
    PORT_ANY = auto()

def Port(string):
    try:
        return PortRange(string)
    except:
        pass

    try:
        return PortNum(string)
    except:
        pass

    try:
        return PortAny(string)
    except:
        pass

    raise ValueError("Invalid Port %s"%string)

class _PortBase:
    def __init__(self, portType=None):
        self.portType = portType

    @property
    def portType(self):
        return self._portType

    @portType.setter
    def portType(self, value):
        if not value or not isinstance(value, PORT_TYPE):
            raise ValueError("Invalid port type %s"%(value))
        self._portType = value

    def isSame(self, obj):
        raise NotImplementedError("Base method")

    def match(self, port):
        raise NotImplementedError("Base method")

    def getRandom(self, seed=None):
        raise NotImplementedError("Base method")

    def __contains__(self, port):
        try:
            return self.match(port)
        except:
            return NotImplemented

    def __eq__(self, obj):
        try:
            return self.isSame(obj)
        except:
            return NotImplemented

class PortAny(_PortBase):

    def __init__(self, string):
        if not string or \
           not isinstance(string, str) or \
           string.lower() not in ["any","*"]:
            raise ValueError("Invalid Port any %s"%string)
        _PortBase.__init__(self, PORT_TYPE.PORT_ANY)

    def __str__(self):
        return "any"

    def isSame(self, obj):
        return self.portType == obj.portType

    def match(self, port):
        try:
            port = int(port)
            return validate(port)
        except:
            return False

    def getRandom(self, seed=None):
        random.seed(seed)
        return random.randrange(PORT_MAX+1)

class PortNum(_PortBase):

    def __init__(self, string):
        self.port = int(string)
        _PortBase.__init__(self, PORT_TYPE.PORT_NUM)

    @property
    def port(self):
        return self._port

    @port.setter
    def port(self, value):
        if not isinstance(value, int) or \
           not validate(value):
            raise ValueError("Invalid Port num %s"%value)
        self._port = value

    def __str__(self):
        return "%s"%self.port

    def isSame(self, obj):
        return self.portType == obj.portType and \
            self.port == obj.port

    def match(self, port):
        try:
            port = int(port)
            return validate(port) and self.port == port
        except:
            return False

    def getRandom(self, seed=None):
        return self.port

class PortRange(_PortBase):
    def __init__(self, string):
        try:
            portSlice = string.split('-')

            if len(portSlice) != 2:
                raise ValueError("Invalid Port range "
                                ": %s"%(string))

            self.low = int(portSlice[0])
            self.high = int(portSlice[1])

            if self._low > self._high:
                raise ValueError("Invalid Port range "
                                ": %s"%(string))
        except:
            raise ValueError("Invalid Port range : %s"%(string))

        _PortBase.__init__(self, PORT_TYPE.PORT_RANGE)

    @property
    def low(self):
        return self._low

    @low.setter
    def low(self, value):
        if not validate(value):
            raise ValueError("Invalid low port %s"%value)
        self._low = value

    @property
    def high(self):
        return self._high

    @high.setter
    def high(self, value):
        if not validate(value):
            raise ValueError("Invalid high port %s"%value)
        self._high = value

    def __str__(self):
        return "%s - %s"%(self.low, self.high)

    def isSame(self, obj):
        return self.portType == obj.portType and \
            self.low == obj.low and \
            self.high == self.high

    def match(self, port):
        try:
            port = int(port)
            return validate(port) and self.low <= port <= self.high
        except:
            return False

    def getRandom(self, seed=None):
        random.seed(seed)
        return random.randrange(self.low, self.high+1)
