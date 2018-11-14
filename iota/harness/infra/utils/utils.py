#! /usr/bin/python3
import pdb
import pprint
import traceback
import re
import sys

from google.protobuf import text_format
class Dict2Enum(object):
    def __init__(self, entries):
        self.str_enums = entries
        entries = dict((v.replace(" ", "_"), k) for k, v in entries.items())
        self.__dict__.update(entries)

    def valid(self, name):
        return name in self.__dict__

    def id(self, name):
        return self.__dict__[name]

    def str(self, v):
        return self.str_enums[v]

def List2Enum(entries):
    db = {}
    value = 0
    for entry in entries:
        db[value] = entry
        value += 1
    return Dict2Enum(db)

def LogMessageContents(hdrstr, msg, logfn):
    logfn("%s %s:" % (msg.__class__.__name__, hdrstr))
    msgstr = text_format.MessageToString(msg).split('\n')
    for s in msgstr:
        if s != '': logfn("- %s: " % s)
    return

def LogException(lg):
    exc_type, exc_value, exc_traceback = sys.exc_info()
    lg.info("Exception: %s-- %s" % (exc_type, exc_value))
    lg.info('-' * 60)
    backtrace = pprint.pformat(traceback.format_tb(exc_traceback))
    for line in backtrace.split('\n'):
        lg.info("%s" % line)
    lg.info('-' * 60)

