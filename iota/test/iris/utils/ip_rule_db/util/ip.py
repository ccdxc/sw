#!/usr/bin/python3

import sys
import random

from ipaddress import IPv4Address
from ipaddress import ip_address
from ipaddress import ip_network
from ipaddress import summarize_address_range as summarize

from enum import Enum, auto

class IPType(Enum):
    IP_RANGE = auto()
    IP_LIST = auto()
    IP_SUBNET = auto()
    IP_ANY = auto()

def IP(string):
    try:
        return IPSubnet(string)
    except:
        pass

    try:
        return IPList(string)
    except:
        pass

    try:
        return IPRange(string)
    except:
        pass

    try:
        return IPAny(string)
    except:
        pass

    raise ValueError("Can not parse IP from '%s'"%string)

class _IPBase:

    def __init__(self, ipType = None, version=None):
        self.ipType = ipType
        self.version = version

    @property
    def ipType(self):
        return self._ipType

    @ipType.setter
    def ipType(self, value):
        if not value or not isinstance(value, IPType):
            raise ValueError("invalid ipType %s"%value)
        self._ipType = value

    @property
    def version(self):
        return self._version

    @version.setter
    def version(self, value):
        self._version = value

    def __str__(self):
        raise NotImplementedError("Base method")

    def isSame(self, obj):
        raise NotImplementedError("Base method")

    def match(self, addr):
        raise NotImplementedError("Base method")

    def getRandom(self, seed=None):
        raise NotImplementedError("Base method")

    def __contains__(self, addr):
        try:
            return self.match(addr)
        except:
            return NotImplemented

    def __eq__(self, obj):
        try:
            return self.ipType == obj.ipType and \
                self.version == obj.version and \
                self.isSame(obj)
        except:
            return NotImplemented

class IPAny(_IPBase):

    def __init__(self, string=None):
        if not string or string.lower() not in ['any', '*'] :
            raise ValueError("Invalid IP any %s"%string)
        self.disp = string
        _IPBase.__init__(self, IPType.IP_ANY)

    def isSame(self, obj):
        return self.ipType == obj.ipType

    def match(self, addr):
        try:
            ip_address(addr)
            return True
        except:
            return False

    def getRandom(self, seed=None):
        net = ip_network("0.0.0.0/0")
        random.seed(seed)
        return str(net[random.randrange(net.num_addresses)])

    def __str__(self):
        return self.disp

class IPRange(_IPBase):

    def __init__(self, string=None):
        if not string or string.count("-") != 1 or string.count(".") != 6:
            raise ValueError("Invalid IP range %s"%(string))

        try:
            first,last = string.split('-')
            self.first = ip_address(first.strip())
            self.last = ip_address(last.strip())
        except:
            raise ValueError("Invalid IP range %s"%(string))

        if self.first > self.last:
            raise ValueError("Invalid IP range, start %s can not "
                             "be greater than end %s"
                             %(self.first, self.last))

        _IPBase.__init__(self, IPType.IP_RANGE, self.first.version)

    def isSame(self, obj):
        return self.ipType == obj.ipType and \
            self.first == obj.first and \
            self.last == obj.last

    def getRandom(self, seed=None):
        random.seed(seed)
        net = [n for n in summarize(self.first, self.last)][0]
        return str(net[random.randrange(net.num_addresses)])

    def match(self, addr):
        try:
            addr = ip_address(addr)
            return self.first <= addr <= self.last
        except:
            return False

    def __str__(self):
        return "%s - %s"%(self.first, self.last)

class IPList(_IPBase):

    def __init__(self, pIpList):
        if not len(pIpList):
            raise valueError("Invalid IPList input: %s", in_iplist)

        self.iplist = set([])
        iplist = pIpList.split(',')

        for item in iplist:
            self.iplist.add(ip_address(item.strip()))

        ver = set([addr.version for addr in self.iplist])
        if len(ver) != 1:
            raise ValueError("All addresses in iplist are not "
                             "of same version %s"%(pIpList))

        _IPBase.__init__(self, IPType.IP_LIST, list(ver)[0])

    def isSame(self, obj):
        return self.ipType == obj.ipType and self.iplist == obj.iplist

    def match(self, addr):
        try:
            addr = ip_address(addr)
            return addr in self.iplist
        except:
            return False

    def getRandom(self, seed=None):
        random.seed(seed)
        return str(random.choice(self.iplist))

    def __str__(self):
        out = ""

        if len(self.iplist) == 0:
            return out

        for a in self.iplist:
            out += "%s, "%str(a)

        return out[:-2]

class IPSubnet(_IPBase):

    def __init__(self, subnet):
        self.subnet = ip_network(subnet)
        _IPBase.__init__(self, IPType.IP_SUBNET, self.subnet.version)

    def isSame(self, obj):
        return self.ipType == obj.ipType and self.subnet == obj.subnet

    def match(self, addr):
        try:
            addr = ip_address(addr)
            return addr in self.subnet
        except:
            return False

    def getRandom(self, seed=None):
        random.seed(seed)
        return str(self.subnet[random.randrange(self.subnet.num_addresses)])

    def __str__(self):
        return str(self.subnet)
