#! /usr/bin/python3
import pdb

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
    msgstr = str(msg).split('\n')
    for s in msgstr:
        if s != '': logfn("- %s: " % s)
    return
