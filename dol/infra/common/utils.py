#! /usr/bin/python3

import os
import pdb
import binascii
import inspect
import traceback
import pprint
import re
import sys
import infra.common.defs as defs
from collections import OrderedDict
from scapy.packet import Packet
from io import StringIO


def IsTemplateField(string):
    # TODO: Add a def to separator instead of hardcoding.
    if "/" in str(string):
        return True
    return False

# Returns 'default' in case of an exception
def SafeFnCall(default, lgh, fn, *args):
    try:
        ret = fn(*args)
    except:
        log_exception(lgh)
        ret = default
    return ret

def GetFullPath(name, path=None, logger=None):
    ws_top = os.environ["WS_TOP"]
    dtop = ws_top + defs.DOL_PATH
    if path == None:
        filename = "%s/%s" % (dtop, name)
    else:
        filename = "%s/%s/%s" % (dtop, path, name)
    if logger:
        logger.verbose("Created Full Path = %s" % filename)
    return filename


def GetFullIglobPath(name, path=None, logger=None):
    ws_top = os.environ["WS_TOP"]
    dtop = ws_top + defs.DOL_PATH
    if path == None:
        filename = "%s/**/%s" % (dtop, name)
    else:
        filename = "%s/%s/**/%s" % (dtop, path, name)
    if logger:
        logger.verbose("Created Full Path = %s" % filename)
    return filename


def ReturnCheckNAssert(ret, logger, *args, **kwargs):
    if ret != defs.status.SUCCESS:
        logger.error(*args, **kwargs)
        logger.error("Failed. Return Code = %d" % ret)
        assert(0)
    return


def ReturnCheckNReturn(ret, logger, *args, **kwargs):
    if ret != defs.status.SUCCESS:
        logger.error(*args, **kwargs)
        logger.error("Failed. Return Code = %d" % ret)
    return ret


def FakeModelSockAddr():
    sockaddr = ('127.0.0.1', defs.FAKE_MODEL_PORT)
    return sockaddr

DECDIGITS = '0123456789'
HEXDIGITS = '0123456789abcdefABCDEF'

def IsInDigitSet(string, digitset):
    for s in string:
        if s not in digitset:
            return False
    return True

def IsDecInteger(string):
    return IsInDigitSet(string, DECDIGITS)

def IsHexInteger(string):
    if string.startswith('0x'): return True
    return IsInDigitSet(string, HEXDIGITS)

def IsInteger(string):
    return IsDecInteger(string) or IsHexInteger(string)

def ParseInteger(string):
    if IsDecInteger(string):
        return int(string)
    elif IsHexInteger(string):
        return int(string, 16)
    assert(0)

def ParseIntegerList(string):
    string = str(string)
    str_nums = [string]
    if " " in string:
        str_nums = string.split(' ')
    nums = []
    for str_num in str_nums:
        num = ParseInteger(str_num)
        nums.append(num)
    return nums

def GetFunctionName():
    return inspect.stack()[1][3]

def LogFunctionBegin(lg):
    lg.debug("BEG: %s()" % inspect.stack()[1][3])
    return

def LogFunctionEnd(lg, status=0):
    lg.debug("END: %s()  Status:%d" % (inspect.stack()[1][3], status))
    return

def log_exception(lg):
    exc_type, exc_value, exc_traceback = sys.exc_info()
    lg.info("Exception: %s-- %s" % (exc_type, exc_value))
    lg.info('-' * 60)
    lg.info("%s" % pprint.pformat(traceback.format_tb(exc_traceback)))
    lg.info('-' * 60)


def CompareObjectFields(self, other, fields, lgh):
    if other == None and self == None:
        return True

    if (other == None and self != None) or \
        (self == None and other != None):
        return False
    
    ret = True
    for field in fields:
        exp_value = getattr(self, field) 
        actual_val =  getattr(other, field)
        if isinstance(exp_value, list):
            if set(exp_value) != set(actual_val):
                lgh.error("Field mismatch : Field : %s Expected : %s, actual : %s"
                          %(field, set(exp_value), set(actual_val)))
                ret = False
        else:
            if exp_value != actual_val:
                lgh.error("Field mismatch : Field : %s, Expected : %s, Actual : %s" %(
                    field,  exp_value, actual_val))
                ret = False
    return ret

def convert_scapy_out_to_dict(spkt):
    capture = StringIO()
    save_stdout = sys.stdout
    sys.stdout = capture
    spkt.show2()
    sys.stdout = save_stdout
    output = capture.getvalue().split("\n")
    ret_dict = OrderedDict()
    index = 0
    while index < len(output):
        line = output[index]
        header = re.search('###\[ (.+?) \]###', line)
        if header:
            header = header.group(1)
            index += 1
            field_dict = OrderedDict()
            while index < len(output):
                line = output[index]
                if not line.strip() or line.startswith("###"):
                    break
                try:
                    list_out = output[index].strip().split("=")
                except:
                    break
                if len(list_out) == 2:
                    key, value = list_out
                    field_dict[key.strip(" \\")] = value.strip()
                elif len(list_out) == 1:
                    field_dict[list_out[0].strip(" \\")] = None
                index += 1
            if field_dict:
                ret_dict[header] = field_dict
        else:
            index += 1

    return ret_dict


def convert_object_to_dict(obj, maintain_odict_order=True, ignore_keys=None,
                           ignore_private_members=True, ignore_empty=True):

    def __convert_object_to_dict(obj):
        if isinstance(obj, list):
            element = []
            for item in obj:
                element.append(__convert_object_to_dict(item))
            return element
        elif isinstance(obj, Packet):
            return convert_scapy_out_to_dict(obj)
        elif isinstance(obj, bytearray) or isinstance(obj, bytes):
            return str(binascii.hexlify(obj), "ascii")
        elif hasattr(obj, "__dict__") or isinstance(obj, dict):
            result = OrderedDict() if maintain_odict_order and isinstance(
                obj, OrderedDict) else dict()
            for key, val in (obj.items() if isinstance(obj, dict) else obj.__dict__.items()):
                if (not val and ignore_empty and val != 0 or
                    not isinstance(obj, dict) and key.startswith("_") and ignore_private_members or
                        key in (ignore_keys or [])):
                    continue
                result[key] = __convert_object_to_dict(val)
            return result
        else:
            return obj

    return __convert_object_to_dict(obj)
