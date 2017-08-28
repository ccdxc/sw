#! /usr/bin/python3
import sys
import pdb
import random
import ipaddress
import copy

import infra.common.utils as utils
import infra.common.loader as loader
import infra.common.defs as defs
import infra.penscapy.penscapy as penscapy

from collections import OrderedDict
from infra.common.logging import logger


class FrameworkObject(object):
    __readonly = False
    __locked = False

    def __init__(self):
        self.__gid = None
        return

    def ID(self, gid=None):
        return self.GID(gid)

    def GID(self, gid=None):
        if gid:
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

    def wrap_enable(self):
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
    def __init__(self, string):
        super().__init__()
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
        return

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


class MacAddress(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.value = MacAddressBase(valobj.params[0])
        return

    def get(self):
        return self.value.get()

    def getnum(self):
        return self.value.getnum()


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
    def __init__(self, valobj=None, string=None):
        super().__init__()
        if valobj:
            self.value = ipaddress.IPv4Address(valobj.params[0])
        elif string:
            self.value = ipaddress.IPv4Address(string)
        else:
            assert(0)
        return

    def get(self):
        return str(self.value)

    def getnum(self):
        return int(self.value)


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


class Ipv6Address(FrameworkFieldObject):
    def __init__(self, valobj=None, string=None):
        super().__init__()
        if valobj:
            self.value = ipaddress.IPv6Address(valobj.params[0])
        elif string:
            self.value = ipaddress.IPv6Address(string)
        else:
            assert(0)
        return

    def get(self):
        return str(self.value)

    def getnum(self):
        return int(self.value)


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
        # TODO
        return [self.start] * size


class PatternDecrement(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = int(valobj.params[0])
        self.end = int(valobj.params[1])
        self.step = int(valobj.params[2])
        return

    def get(self, size):
        # TODO
        return [str(self.start)] * size


class PatternRandom(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.start = int(valobj.params[0])
        self.end = int(valobj.params[1])
        self.step = int(valobj.params[2])
        return

    def get(self, size):
        # TODO
        return [self.start] * size


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


class CallbackField(FrameworkFieldObject):
    def __init__(self, valobj):
        super().__init__()
        self.callback = valobj.params[-1]
        self.module = valobj.params[-2]
        pkg_str_list = valobj.params[:-2]
        self.pkg = 'test.callbacks'
        for s in pkg_str_list:
            self.pkg += ".%s" % s
        return

    def call(self, *args):
        cb = loader.GetModuleAttr(self.pkg, self.module, self.callback)
        return cb(*args)


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
    def Extend(self, filter_str):
        obj = TemplateFieldObject(filter_str)
        self.filters.extend(obj.filters)
        return

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
        self.attr = attr_string
        self.id = None
        if '=' in attr_string:
            self.is_object = True
            self.__get_obj_id()
        return

    def __get_obj_id(self):
        attr, value = self.string.split("=")
        assert(attr == 'id')
        self.attr = attr
        self.id = value
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

    def __get_attr(self, node, refattr):
        try:
            val = node.__dict__[refattr.attr]
            if isinstance(val, FrameworkFieldObject):
                return val.get()
            else:
                return val
        except:
            logger.error("Failed to resolve Ref: %s" % self.string)
            logger.error(" - Attr: %s" % refattr.attr)
            assert(0)
        return None

    def Get(self, root):
        value = root
        for r in self.refattrs:
            if r.is_object:
                value = self.__get_object(value, r)
            else:
                value = self.__get_attr(value, r)
        return value

    def GetRootID(self):
        return self.rootid

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
#   PATTERN_FIXED       :   pattfixed/<value>
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

    'pattfixed': {
        'object': PatternFixed,
        'pcount': 1,
    },

    'pattincr': {
        'object': PatternIncrement,
        'pcount': 3,
    },

    'pattdecr': {
        'object': PatternDecrement,
        'pcount': 3,
    },

    'pattrandom': {
        'object': PatternRandom,
        'pcount': 3,
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


def IsReference(field):
    return isinstance(field, ReferenceField)


def IsCallback(field):
    return isinstance(field, CallbackField)


class ObjectDatabase(FrameworkObject):
    def __init__(self, logger):
        super().__init__()
        self.db = {}
        self.logger = logger
        return

    def __len__(self):
        return len(self.db)

    def Get(self, key):
        return self.db[key]

    def GetAll(self):
        values = []
        for val in self.db.values():
            values.append(val)
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
        if self.logger: self.logger.info("- Adding %s to store." % key)
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
