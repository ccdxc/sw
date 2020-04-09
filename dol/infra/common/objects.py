#! /usr/bin/python3
import sys
import os
import pdb
import random
import ipaddress
import copy
from ctypes import *

import infra.common.utils as utils
import infra.common.loader as loader
import infra.common.defs as defs
import infra.common.timeprofiler as timeprofiler
import infra.penscapy.penscapy as penscapy

from collections import OrderedDict
from infra.common.glopts import GlobalOptions
from infra.common.logging import logger

asic = os.environ.get('ASIC', 'capri')

class FrameworkObject(object):
    __readonly = False
    __locked = False

    def __init__(self):
        self.__gid = None
        return

    def ID(self, gid=None):
        return self.GID(gid)

    def GID(self, gid=None):
        if gid is not None:
            self.__gid = gid
        return self.__gid

    def __setattr__(self, key, val):
        if self.__readonly:
            raise TypeError("Cannot modify a readonly object.")
        if self.__locked and key not in self.__dict__:
            raise TypeError("Cannot add new attribute to class %s" % self)
        self.__dict__[key] = val

    def LockAttributes(self):
        self.__locked = True
        return

    def SetReadOnly(self):
        self.__readonly = True
        return

    def SetReadWrite(self):
        self.__readonly = False
        return

    def IsReadOnly(self):
        return self.__readonly

    def Clone(self, template):
        #assert(isinstance(template, FrameworkTemplateObject))
        old_gid = self.__gid
        self.__dict__.update(copy.deepcopy(template.__dict__))

        # Restore the original GID.
        self.__gid == old_gid
        self.template = template
        # self.LockAttributes()
        return

    def show(self, level=logger.levels.VERBOSE):
        logger.log(level, "OBJECT: ", self)
        for k in self.__dict__:
            v = self.__dict__[k]
            if isinstance(v, FrameworkObject):
                logger.log(level, "%s:" % k)
                v.show(level)
            elif isinstance(v, list):
                logger.log(level, "%s:" % k)
                for e in v:
                    if isinstance(e, FrameworkObject):
                        e.show(level)
                    else:
                        logger.log(level, "list-item = ", e)
            elif isinstance(v, dict):
                logger.log(level, "%s:" % k)
                for key, value in v.items():
                    if isinstance(value, FrameworkObject):
                        logger.log(level, "Begin KEY = ", key)
                        value.show(level)
                        logger.log(level, "End KEY = ", key)
                    else:
                        logger.log(level, "dict-item = ", key, value)
            else:
                logger.log(level, "%s = %s" % (k, str(v)))
        return

    def keys(self):
        return [k for k in self.__dict__ if not k.startswith('_')]


class FrameworkTemplateObject(FrameworkObject):
    def __init__(self):
        super().__init__()
        return

class FrameworkFieldObject(FrameworkObject):
    def __init__(self):
        super().__init__()
        return


class RandomNumGenerator(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = utils.ParseInteger(valobj.params[0])
        self.end = utils.ParseInteger(valobj.params[1])
        return

    def get(self):
        random.seed(0)
        return random.randint(self.start, self.end)


class ConstantValue(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.value = utils.ParseInteger(valobj.params[0])
        return

    def get(self):
        return self.value


class IntegerRange(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = utils.ParseInteger(valobj.params[0])
        self.end = utils.ParseInteger(valobj.params[1])
        self.step = 1
        if len(valobj.params) > 2:
            self.step = utils.ParseInteger(valobj.params[2])
        self.curr = self.start
        self.allow_wrap = False
        return

    def EnableWrapAround(self):
        self.allow_wrap = True
        return

    def reset(self):
        self.curr = self.start

    def get(self):
        if self.curr > self.end:
            if self.allow_wrap == False:
                assert(0)
            else:
                self.curr = self.start
        out = self.curr
        self.curr += self.step
        return out

    def GetStart(self):
        return self.start
    def GetEnd(self):
        return self.end
    def GetCount(self):
        return self.end - self.start + 1

class MacAddressBase(FrameworkFieldObject):
    def __init__(self, string=None, integer=None):
        super().__init__()
        if string:
            if "." in string:
                words16 = string.split(".")
                assert(len(words16) == 3)
                self.num = (int(words16[0], 16) << 32) +\
                           (int(words16[1], 16) << 16) +\
                           (int(words16[2], 16))
            elif ":" in string:
                by8 = string.split(":")
                assert(len(by8) == 6)
                self.num = (int(by8[0], 16) << 40) +\
                           (int(by8[1], 16) << 32) +\
                           (int(by8[2], 16) << 24) +\
                           (int(by8[3], 16) << 16) +\
                           (int(by8[4], 16) << 8) +\
                           (int(by8[5], 16))
            else:
                assert 0
        elif integer is not None:
            #Assume its converted.
            self.num = integer
        return

    def __str__(self):
        return self.get()


    def get(self):
        by = [0] * 6
        by[0] = (self.num >> 40) & 0xFF
        by[1] = (self.num >> 32) & 0xFF
        by[2] = (self.num >> 24) & 0xFF
        by[3] = (self.num >> 16) & 0xFF
        by[4] = (self.num >> 8) & 0xFF
        by[5] = (self.num >> 0) & 0xFF
        string = "%02x:%02x:%02x:%02x:%02x:%02x" %\
                 (by[0], by[1], by[2], by[3], by[4], by[5])
        return string

    def getnum(self):
        return self.num

    def update(self, macincr):
        self.num += macincr
        return

    def __eq__(self, other):
        return other and self.getnum() == other.getnum()


class MacAddress(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.value = MacAddressBase(valobj.params[0])
        return

    def get(self):
        return self.value.get()

    def getnum(self):
        return self.value.getnum()

    def update(self, macincr):
        self.value.update(macincr)

    def __str__(self):
        return self.value.get()


class MacAddressStep(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = MacAddressBase(valobj.params[0])
        self.step = MacAddressBase(valobj.params[1])
        self.curr = self.start
        return

    def __getone(self):
        out = self.curr
        self.curr.num += self.step.num
        return out

    def get(self):
        out = self.__getone()
        return MacAddressBase(out.get())


class IpAddress(FrameworkFieldObject):
    def __init__(self, valobj=None, string=None, integer=None):
        super().__init__()
        if valobj:
            self.value = ipaddress.IPv4Address(valobj.params[0])
        elif string:
            self.value = ipaddress.IPv4Address(string)
        elif integer is not None:
            self.value = ipaddress.IPv4Address(integer)
        else:
            assert(0)
        return

    def __str__(self):
        return self.get()

    def get(self):
        return str(self.value)

    def getnum(self):
        return int(self.value)

    def __eq__(self, other):
        return other and self.value == other.value

    def __hash__(self):
        return hash(self.value)

class IpAddressStep(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = ipaddress.IPv4Address(valobj.params[0])
        self.step = ipaddress.IPv4Address(valobj.params[1])
        self.curr = self.start
        self.count = 0
        if len(valobj.params) == 3:
            self.count = utils.ParseInteger(valobj.params[2])
        return

    def __getone(self):
        out = self.curr
        self.curr = ipaddress.IPv4Address(int(self.curr) + int(self.step))
        return out

    def get(self):
        return IpAddress(string=str(self.__getone()))

    def GetCount(self):
        return self.count

    def GetLast(self):
        #Assuming /24
        return ipaddress.IPv4Address(int(self.start) + 254)

class Ipv6Address(FrameworkFieldObject):
    def __init__(self, valobj=None, string=None, integer=None):
        super().__init__()
        if valobj:
            self.value = ipaddress.IPv6Address(valobj.params[0])
        elif string:
            self.value = ipaddress.IPv6Address(string)
        elif integer is not None:
            self.value = ipaddress.IPv6Address(integer)
        else:
            assert(0)
        return

    def __str__(self):
        return self.get()

    def __hash__(self):
        return hash(self.value)

    def get(self):
        return str(self.value)

    def getnum(self):
        return int(self.value)

    def __eq__(self, other):
        return other and self.value == other.value

class Ipv6AddressStep(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = ipaddress.IPv6Address(valobj.params[0])
        self.step = ipaddress.IPv6Address(valobj.params[1])
        self.curr = self.start
        return

    def __getone(self):
        out = self.curr
        self.curr = ipaddress.IPv6Address(int(self.curr) + int(self.step))
        return out

    def get(self):
        return Ipv6Address(string=str(self.__getone()))


class AutoField(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.valstr = valobj.params[0].upper()
        self.val = defs.autotypes.__dict__[self.valstr]
        return

    def get(self):
        return self.val


class IpProtocol(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        if valobj.params[0].startswith('0x'):
            self.val = int(valobj.params[0], 16)
        else:
            self.val = int(valobj.params[0])
        assert(self.val >= 0 and self.val <= 255)
        return

    def get(self):
        return self.val


class PatternFixed(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.val = int(valobj.params[0])
        return

    def get(self, size):
        return [self.val] * size


class PatternIncrement(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = int(valobj.params[0])
        self.end = int(valobj.params[1])
        self.step = int(valobj.params[2])
        return

    def get(self, size):
        assert(self.start < self.end)
        patt = list(range(self.start, self.end, self.step))
        repeat = int(size / len(patt) + 1)
        patt = patt * repeat
        return patt[:size]

class PatternDecrement(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = int(valobj.params[0])
        self.end = int(valobj.params[1])
        self.step = int(valobj.params[2])
        return

    def get(self, size):
        assert(self.start > self.end)
        patt = list(range(self.start, self.end, -1 * self.step))
        repeat = int(size / len(patt) + 1)
        patt = patt * repeat
        return patt[:size]

class PatternRandom(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = int(valobj.params[0])
        self.end = int(valobj.params[1])
        self.step = int(valobj.params[2])
        return

    def get(self, size):
        patt = []
        for x in range(size):
            patt.append(random.randint(self.start, self.end))
        return patt

class Timestamp(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.zone = valobj.params[0]
        return

    def get(self):
        return 0


class VlanId(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.val = int(valobj.params[0])
        assert(self.val >= 0 and self.val < 4096)
        return

    def get(self):
        return self.val


class L4Port(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.val = int(valobj.params[0])
        assert(self.val >= 0 and self.val < 65535)
        return

    def get(self):
        return self.val


class Ethertype(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.val = int(valobj.params[0], 16)
        assert(self.val >= 0 and self.val <= 0xFFFF)
        return

    def get(self):
        return self.val


class ConfigField(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.typestr = valobj.params[0].upper()
        self.scopestr = valobj.params[1].upper()
        self.fieldstr = valobj.params[1].upper()
        self.keyval = valobj.params[1].upper()
        self.filterstr = None

        if len(valobj.params) > 2:
            self.filterstr = valobj.params[2]

        self.type = defs.cfgtypes.id(self.typestr)
        if defs.config_field_scopes.valid(self.scopestr):
            self.scope = defs.config_field_scopes.id(self.scopestr)
            self.field = None
        elif defs.filters.valid(self.fieldstr):
            self.scope = None
            self.field = defs.filters.id(self.fieldstr)
        else:
            logger.verbose("Config Field Param:%s not a SCOPE or OBJECTFIELD" %
                           self.fieldstr)
        return

    def type(self):
        return self.cfgtype

    def scope(self):
        return self.scope

    def field(self):
        return self.field

    def key(self):
        return self.keyval

class CallbackArgsObject:
    def __init__(self, argstr):
        arglist = argstr.split(',')
        for arg in arglist:
            nv = arg.split('=')
            value = utils.ParseInteger(nv[1]) if utils.IsInteger(nv[1]) else nv[1]
            self.__dict__[nv[0]] = value
        return

class FileField(FrameworkFieldObject):
    def __init__(self, valobj):
        self.valobj = valobj
        self.path = "."
        for param in valobj.params:
            self.path += "/%s" % param
        return

    def Get(self):
        return self.path

class CallbackField(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.pidx       = -1
        self.args       = None
        self.callback   = None
        self.module     = None
        self.pkg        = None
        self.valobj     = valobj

        self.__process_args()
        self.__process_callback()
        self.__process_module()
        self.__process_package()
        return

    __package_path = None
    @staticmethod
    def SetPackagePath(path):
        CallbackField.__package_path = path

    def __process_args(self):
        argstr = self.valobj.params[self.pidx]
        if '=' not in argstr:
            return
        self.pidx -= 1
        self.args = CallbackArgsObject(argstr)
        return

    def __process_callback(self):
        self.callback = self.valobj.params[self.pidx]
        self.pidx -= 1
        return

    def __process_module(self):
        self.module = self.valobj.params[self.pidx]
        self.pidx -= 1
        return

    def __process_package(self):
        pkg_str_list = self.valobj.params[:self.pidx+1]
        assert(self.__package_path)
        self.pkg = self.__package_path
        for s in pkg_str_list:
            self.pkg += ".%s" % s
        self.pidx = None # All params are consumed.
        return

    def call(self, *args):
        cb = loader.GetModuleAttr(self.pkg, self.module, self.callback)
        if self.args is None:
            return cb(*args)
        return cb(*args, self.args)


class PercentField(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.value = int(valobj.params[0])
        return

    def get(self):
        return self.value


class FilterField(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.string = valobj.string
        self.filters = []
        key_value_pairs = valobj.params[0].split(',')
        for key_value_str in key_value_pairs:
            if key_value_str == 'any':
                self.filters.append(('any', None))
                return
            params = key_value_str.split('=')
            assert(len(params) == 2)
            self.filters.append((params[0], params[1]))
        return

    def Extend(self, fobj):
        if isinstance(fobj, FilterField):
            obj = fobj
        else:
            obj = TemplateFieldObject(fobj)
        self.filters.extend(obj.filters)
        return

    def __str__(self):
        return self.string

    def GetValueByKey(self, key):
        for k,v in self.filters:
            if key == k:
                return v
        return None


class TspecReferenceFied(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.type_str = valobj.params[0]
        self.id_str = valobj.params[1]
        if len(valobj.params) > 2:
            self.attr_str = valobj.params[2]
        return


class ReferenceFieldAttr:
    def __init__(self, attr_string):
        self.string = attr_string
        self.is_object = False
        self.is_index = False
        self.attr = attr_string
        self.id = None
        if '=' in attr_string:
            self.is_object = True
            self.__get_obj_id()
        elif attr_string.isdigit():
            self.is_index = True
        return

    def __get_obj_id(self):
        attr, value = self.string.split("=")
        assert(attr == 'id')
        self.attr = attr
        self.id = value
        return

    def ScaleUp(self, s):
        self.id = "%s_%s" % (self.id, s)
        return


class ReferenceField(FrameworkFieldObject):
    def __init__(self, obj):
        super().__init__()
        self.string = obj.string
        self.rootid = defs.ref_roots.id(obj.params[0].upper())
        del obj.params[0]
        self.attr_list = obj.params

        self.refattrs = []
        for a in self.attr_list:
            refattr = ReferenceFieldAttr(a)
            self.refattrs.append(refattr)
        return

    def __get_object(self, db, refattr):
        return db.Get(refattr.id)
    
    def __get_by_index(self, obj, refattr):
        assert(isinstance(obj, list))
        ret = obj[refattr]
        if isinstance(ret, FrameworkFieldObject):
            return ret.get()
        else:
            return ret

    def __get_attr(self, node, refattr):
        val = getattr(node, refattr.attr, 'NOTFOUND')
        if val is 'NOTFOUND':
            logger.error("Failed to resolve Ref: %s" % self.string)
            logger.error(" - Attr: %s" % refattr.attr)
            assert(0)

        if isinstance(val, FrameworkFieldObject):
            return val.get()
        else:
            return val
        return None

    def Get(self, root):
        value = root
        for r in self.refattrs:
            if r.is_index:
                value = self.__get_by_index(value, int(r.string))
            elif r.is_object:
                value = self.__get_object(value, r)
            else:
                value = self.__get_attr(value, r)
        return value

    def GetRootID(self):
        return self.rootid

class ScaledReferenceField(ReferenceField):
    def __init__(self, obj):
        super().__init__(obj)
        return

    def ScaleUp(self, s):
        for r in self.refattrs:
            if r.is_object:
                r.ScaleUp(s)
        return

# A Value object is a generic object. It is used in the following format.
# type/param1[/param2/[param3]......]
#
# Following are the formats of different objects:
#   RANDOM_NUM_GEN      :   random/<start>/<end>
#   CONSTANT_VALUE      :   const/<value>
#   INTEGER_STEP        :   range/<start>/<end>/<step>
#   MAC_ADDRESS         :   macaddr/<hhhh.hhhh.hhhh>
#   MAC_ADDRESS_STEP    :   macstep/<start>/<step as hhhh.hhhh.hhhh>/<count>
#   IP_ADDRESSS         :   ipaddr/<A.B.C.D>
#   IP_ADDRESS_STEP     :   ipstep/<A.B.C.D>/<step as S.S.S.S>/<count>
#   IPV6_ADDRESS        :   ipv6addr/<hhhh::hhhh>
#   IPV6_ADDRESS_STEP   :   ipv6step/<hhhh::hhhh>/<step as ssss::ssss>/<count>
#   AUTO_FIELD          :   auto/<source>
#   IP_PROTOCOL         :   ipproto/<value>
#   PATTERN_FIXED       :   pattern-fixed/<value>
#   PATTERN_INCREMENT   :   pattincr/<start>/<end>/<step>
#   PATTERN_DECREMENT   :   pattdecr/<start>/<end>/<step>
#   PATTERN_RANDOM      :   pattrand/<start>/<end>/<count>
#   TIMESTAMP           :   timestamp/<zone>
#   VLAN_ID             :   vlanid/<value>
#   L4_PORT             :   l4port/<value>
#   ETHERTYPE           :   ethertype/<value>
#   REFERENCE_FIELD_TEMPLATE:   template/<type>/<object-type>/<id>
#   CONFIG_FIELD        :   config/<dbtype>/<chunk>/<field>/[<filter1>,<filter2>,...,<filterN>]
#   CALLBACK_FIELD      :   callback://package/module/<function>
#   PERCENT_FIELD       :   percent/<value>
#   TSPEC_REF_FIELD     :   testspec/<type>/<id>/<attr>


TemplateFieldValueToObject = {
    'random': {
        'object': RandomNumGenerator,
        'pcount': 2,
    },

    'const': {
        'object': ConstantValue,
        'pcount': 1,
        'opcount': 1,
    },

    'range': {
        'object': IntegerRange,
        'pcount': 2,
        'opcount': 1,
    },

    'macaddr': {
        'object': MacAddress,
        'pcount': 1,
    },

    'macstep': {
        'object': MacAddressStep,
        'pcount': 2,
        'opcount': 1,
    },

    'ipaddr': {
        'object': IpAddress,
        'pcount': 1,
    },

    'ipstep': {
        'object': IpAddressStep,
        'pcount': 2,
        'opcount': 1,
    },

    'ipv6addr': {
        'object': Ipv6Address,
        'pcount': 1,
    },

    'ipv6step': {
        'object': Ipv6AddressStep,
        'pcount': 2,
        'opcount': 1,
    },

    'auto': {
        'object': AutoField,
        'pcount': 1,
    },

    'ipproto': {
        'object': IpProtocol,
        'pcount': 1,
    },

    'pattern-fixed://': {
        'object'    : PatternFixed,
        'pcount'    : 1,
        'opcount'   : 128,
    },

    'pattern-increment://': {
        'object'    : PatternIncrement,
        'pcount'    : 1,
        'opcount'   : 128,
    },

    'pattern-decrement://': {
        'object'    : PatternDecrement,
        'pcount'    : 1,
        'opcount'   : 128,
    },

    'pattern-random://': {
        'object'    : PatternRandom,
        'pcount'    : 1,
        'opcount'   : 128,
    },

    'timestamp': {
        'object': Timestamp,
        'pcount': 1,
    },

    'vlanid': {
        'object': VlanId,
        'pcount': 1,
    },

    'l4port': {
        'object': L4Port,
        'pcount': 1,
    },

    'ethertype': {
        'object': Ethertype,
        'pcount': 1,
    },

    'config': {
        'object': ConfigField,
        'pcount': 2,
        'opcount': 2,
    },

    'callback://': {
        'object': CallbackField,
        'pcount': 2,
        'opcount': 128,
    },

    'percent': {
        'object': PercentField,
        'pcount': 1,
    },

    'filter://': {
        'object': FilterField,
        'pcount': 1,
    },

    'ref://': {
        'object': ReferenceField,
        'pcount': 1,
        'opcount': 128,
    },
    'scale-ref://': {
        'object': ScaledReferenceField,
        'pcount': 1,
        'opcount': 128,
    },
    'file://': {
        'object': FileField,
        'pcount': 1,
        'opcount': 128,
    },

}


class TemplateFieldValue:
    def __init__(self, string):
        self.string = string
        self.object = None
        self.pcount = 0
        self.params = []
        self.__parse()
        return

    def __parse(self):
        if '://' in self.string:
            lst = self.string.split("://")
            prefix = lst[0] + '://'
            lst = self.string.split(prefix)
            string = lst[1]
            self.params = string.split("/")
            typ = prefix
        else:
            self.params = self.string.split("/")
            typ = self.params[0]
            del self.params[0]
        self.object = TemplateFieldValueToObject[typ]['object']
        self.pcount = TemplateFieldValueToObject[typ]['pcount']
        self.opcount = 0
        if 'opcount' in TemplateFieldValueToObject[typ]:
            self.opcount = TemplateFieldValueToObject[typ]['opcount']

        self.total_pcount = self.pcount + self.opcount

        if len(self.params) < self.pcount:
            logger.error("Invalid # of params for field: %s [Expected %d params, Got %d params]" %
                         (self.string, self.pcount, len(self.params)))
            assert(0)
        if len(self.params) > self.total_pcount:
            logger.error("Invalid # of total params for field: %s [Expected %d params, Got %d params]" %
                         (self.string, self.total_pcount, len(self.params)))
            assert(0)

        return


def TemplateFieldObject(string):
    vobj = TemplateFieldValue(string)
    result = vobj.object(vobj)
    return result


def MergeObjects(hi, lo):
    def MergeObject(hi, lo):
        if hi == None:
            return copy.deepcopy(lo)

        result = copy.deepcopy(hi)
        for key in lo.__dict__:
            logger.verbose("Merging MEMBER = %s" % key)
            lo_data = lo.__dict__[key]
            # If key not present in HI, then LO is winner.
            if key not in hi.__dict__:
                logger.verbose("    - not present in HI. using LO")
                result.__dict__[key] = lo_data
                continue

            hi_data = hi.__dict__[key]
            # If key present in HI,
            if isinstance(lo_data, FrameworkObject):
                logger.verbose("    - merging recursively.")
                if hi_data:
                    if isinstance(hi_data, FrameworkObject):
                        # and it is an object, merge recursively
                        merged_data = MergeObjects(hi_data, lo_data)
                    else:
                        merged_data = copy.deepcopy(hi_data)
                else:
                    merged_data = copy.deepcopy(lo_data)
                result.__dict__[key] = merged_data
            else:
                # if not an object, then HI is winner.
                result.__dict__[key] = hi_data
        return result

    if isinstance(hi, list):
        assert(isinstance(lo, list))
        assert(len(lo) == 1)
        result_list = []
        for hiobj in hi:
            result = MergeObject(hiobj, lo[0])
            result_list.append(result)
        return result_list
    return MergeObject(hi, lo)


def ValidateSubClass(sub, sup):
    if sub == None:
        return
    assert(sup != None)

    for key in sub.__dict__:
        logger.verbose("Validating KEY = %s" % key)
        data = sub.__dict__[key]
        # First check if key is present in SUPER.
        if key not in sup.__dict__:
            logger.error("Key %s not present in SuperClass" % key)
            assert(0)

        if isinstance(data, FrameworkFieldObject):
            # Fields dont need to be validated.
            continue

        # If data is an object, recursively check in SUPER
        sup_data = sup.__dict__[key]
        if isinstance(data, FrameworkObject):
            logger.verbose("Recursively validating object")
            ValidateSubClass(data, sup_data)
    return


def IsAutoField(field):
    if not isinstance(field, FrameworkFieldObject):
        return False

    if not isinstance(field, AutoField):
        return False

    return True


def IsTemplateFieldString(string):
    if '://' in string:
        return True
    return False

def IsTemplateFieldValueObject(obj):
    return isinstance(obj, TemplateFieldValue)

def IsReference(field):
    return isinstance(field, ReferenceField)

def IsScaledReference(field):
    return isinstance(field, ScaledReferenceField)

def IsCallback(field):
    return isinstance(field, CallbackField)


class ObjectDatabase(FrameworkObject):
    def __init__(self):
        super().__init__()
        self.db = OrderedDict()
        return

    def __len__(self):
        return len(self.db)

    def Get(self, key):
        return self.db[key]

    def GetAllInList(self):
        return list(self.db.values())

    def GetAll(self):
        values =  self.db.values()
        return values

    def GetAllByClass(self, object_class):
        objlist = []
        for obj in self.db.values():
            if isinstance(obj, object_class):
                objlist.append(obj)
        return objlist

    def Set(self, key, data):
        if key in self.db:
            print("ERROR: Duplicate Key : %s" % key)
            assert(0)
        logger.info("- Adding %s to store." % key)
        self.db[key] = data
        return

    def SetAll(self, objlist):
        assert(objlist)
        for obj in objlist:
            self.Set(obj.ID(), obj)
        return

    def Add(self, obj):
        self.Set(obj.ID(), obj)
        return

    def IsKeyIn(self, key):
        if key in self.db:
            return True
        return False

class PacketComparePartial(FrameworkObject):
    def __init__(self):
        super().__init__()
        self.ignore_hdrs = {}


class HeaderComparePartial(FrameworkObject):
    def __init__(self):
        super().__init__()
        self.ignore_fields = []


def IsFrameworkObject(obj):
    return isinstance(obj, FrameworkObject)


def IsFrameworkTemplateObject(obj):
    return isinstance(obj, FrameworkTemplateObject)


def IsFrameworkFieldObject(obj):
    return isinstance(obj, FrameworkFieldObject)


FrameworkInternalAttrs = ['meta', '__locked', '__readonly']


def IsFrameworkObjectInternalAttr(attr):
    return attr in FrameworkInternalAttrs

class MemHandle(object):

    def __init__(self, va, pa):
        self.va = va
        self.pa = pa

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        return self.va == other.va and self.pa == other.pa

    def __str__(self):
        if self.va is None or self.pa is None:
            return '<None>'
        return '<va=0x%x, pa=0x%x>' % (self.va, self.pa)

    def __add__(self, other):
        assert isinstance(other, int)
        return MemHandle(self.va + other, self.pa + other)


NullMemHandle = MemHandle(0, 0)


class HostMemory(object):
    def __init__(self, libpath):
        if not GlobalOptions.dryrun and GlobalOptions.hostmem:
            self.lib = CDLL(libpath, mode=RTLD_GLOBAL)
            self.lib.alloc_host_mem.argtypes = [c_uint64]
            self.lib.alloc_host_mem.restype = c_void_p
            self.lib.alloc_page_aligned_host_mem.argtypes = [c_uint64]
            self.lib.alloc_page_aligned_host_mem.restype = c_void_p
            self.lib.host_mem_v2p.argtypes = [c_void_p]
            self.lib.host_mem_v2p.restype = c_uint64
            self.lib.host_mem_p2v.argtypes = [c_uint64]
            self.lib.host_mem_p2v.restype = c_void_p
            self.lib.free_host_mem.argtypes = [c_void_p]
            self.lib.free_host_mem.restype = None
            assert self.lib.init_host_mem() == 0

    def get(self, size, page_aligned=True):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return NullMemHandle
        assert isinstance(size, int)
        if page_aligned:
            ptr = self.lib.alloc_page_aligned_host_mem(size)
        else:
            ptr = self.lib.alloc_host_mem(size)
        return MemHandle(ptr, self.lib.host_mem_v2p(ptr))

    def p2v(self, pa):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return NullMemHandle.pa
        assert isinstance(pa, int)
        return self.lib.host_mem_p2v(pa)

    def v2p(self, va):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return NullMemHandle.va
        assert isinstance(va, int)
        return self.lib.host_mem_v2p(va)

    def write(self, memhandle, data):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return
        assert isinstance(memhandle, MemHandle)
        assert isinstance(data, bytes)
        va = memhandle.va
        ba = bytearray(data)
        arr = c_char * len(ba)
        arr = arr.from_buffer(ba)
        memmove(va, arr, sizeof(arr))

    def read(self, memhandle, size):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return bytes()
        assert isinstance(memhandle, MemHandle)
        assert isinstance(size, int)
        ba = bytearray([0x0]*size)
        va = memhandle.va
        arr = c_char * size
        arr = arr.from_buffer(ba)
        memmove(arr, va, sizeof(arr))
        return bytes(ba)

    def zero(self, memhandle, size):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return
        assert isinstance(memhandle, MemHandle)
        assert isinstance(size, int)
        va = memhandle.va
        memset(va, 0, c_uint64(size))

    def __del__(self):
        if not GlobalOptions or GlobalOptions.dryrun or not GlobalOptions.hostmem: return
        self.lib.delete_host_mem()


HostMemMgr = None

def GetHostMemMgrObject():
    global HostMemMgr

    if HostMemMgr is not None:
        return HostMemMgr
    # initialize host memory manager
    if GlobalOptions.gft:
        pipeline_name = "gft"
    else:
        pipeline_name = GlobalOptions.pipeline
    libpath = "nic/build/x86_64/%s/%s/lib/libhost_mem.so" % (pipeline_name, asic)
    libpath = os.path.join(os.environ['WS_TOP'], libpath)
    HostMemMgr = HostMemory(libpath)
    return HostMemMgr

