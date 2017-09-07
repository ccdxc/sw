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
from infra.common.logging import logger
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

# def PrintPacket(pkt):
    # penscapy.hexdump(pkt)
    #i = 0
    # while i < len(pkt):
    #    if i % 16
    #    print("%04x " % i, end='')
    #    i++


DECDIGITS = '0123456789'
HEXDIGITS = '0123456789abcdefABCDEF'


def ParseInteger(string):
    def __is_in_digitset(string, digitset):
        for s in string:
            if s not in digitset:
                return False
        return True
    if string.startswith('0x'):
        return int(string, 16)
    elif __is_in_digitset(string, DECDIGITS):
        return int(string)
    elif __is_in_digitset(string, HEXDIGITS):
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
