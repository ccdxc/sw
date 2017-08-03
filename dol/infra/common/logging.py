#! /usr/bin/python3

import datetime
import pdb
import sys
import inspect

import infra.common.defs as defs

from infra.common.glopts import GlobalOptions

__levels = {
    0: 'NONE',
    1: 'CRITICAL',
    2: 'ERROR',
    3: 'WARNING',
    4: 'INFO',
    5: 'DEBUG',
    6: 'VERBOSE',
    7: 'MAX'
}

levels = defs.Dict2Enum(__levels)

gl_file_log = None
#gl_file_log = open('.framework.log', 'w')
class Logger:
    def __init__(self, stdout=True, level=levels.DEBUG, name=None):
        self.sinks = []
        self.indent_enable = False
        self.level = level
        self.levels = levels
        self.name = name
        if stdout:
            self.sinks.append(sys.stdout)
        global gl_file_log
        if gl_file_log:
            self.sinks.append(gl_file_log)
        return

    def add_stdout(self):
        if sys.stdout not in self.sinks:
            self.sinks.append(sys.stdout)
        
    def __flush(self, text):
        for s in self.sinks:
            s.write(text)
            s.flush()
        return

    def __get_timestamp(self):
        #return "[%s] " % str(datetime.datetime.now())
        return ''

    def __format(self, *args, **kwargs):
        text = ""
        indent = 0
        if self.indent_enable:
            indent = len(inspect.stack())
            if indent >= defs.LOGGING_DEFAULT_REV_OFFSET:
                indent = indent - defs.LOGGING_DEFAULT_REV_OFFSET
        level = kwargs['level']
        prefix = levels.str(level)
        if self.level < level:
            return None
        text = text + self.__get_timestamp()
        if self.name:
            text = text + "[%s]" % self.name
        text = text + "[%s] " % prefix
        if indent:
            text = text + "  " * indent
        for a in args:
            text = text + str(a) + " "

        text = text + "\n"
        return text

    def __log(self, *args, **kwargs):
        text = self.__format(*args, **kwargs)
        if text != None:
            self.__flush(text)
        return

    def info(self, *args, **kwargs):
        return self.__log(*args, **kwargs, level=levels.INFO)

    def debug(self, *args, **kwargs):
        return self.__log(*args, **kwargs, level=levels.DEBUG)

    def verbose(self, *args, **kwargs):
        return self.__log(*args, **kwargs, level=levels.VERBOSE)

    def warn(self, *args, **kwargs):
        return self.__log(*args, **kwargs, level=levels.WARNING)

    def error(self, *args, **kwargs):
        return self.__log(*args, **kwargs, level=levels.ERROR)

    def critical(self, *args, **kwargs):
        return self.__log(*args, **kwargs, level=levels.CRITICAL)

    def log(self, level, *args, **kwargs):
        return self.__log(*args, **kwargs, level=level)
    
    def set_level(self, level):
        self.level = level

    def GetLogPrefix(self):
        return "[%s][INF]" % self.name

start_level = levels.INFO
if GlobalOptions.debug:
    start_level = levels.DEBUG

if GlobalOptions.verbose:
    start_level = levels.VERBOSE

logger      = Logger(level = start_level, name = "Default")
ylogger     = Logger(level = start_level, name = " YAML ")
cfglogger   = Logger(level = start_level, name = "Config")
memlogger   = Logger(level = start_level, name = "MemFactory")
pktlogger   = Logger(level = start_level, name = "PktFactory")
