#! /usr/bin/python3
from subprocess import Popen, PIPE, call
from iota.harness.infra.utils.logger import Logger as Logger

class IotaProcess:
    def __init__(self, cmd, logfile):
        self.__cmd = cmd
        self.__logfile = logfile
        return

    def Start(self):
        Logger.info("Starting %s (Logfile = %s)" % (self.__cmd, self.__logfile))
        log = open(self.__logfile, "w")
        self.__p = Popen(self.__cmd, stdout=log, stderr=log, shell=True)
        return

    def Stop(self):
        self.__p.kill()
        return
