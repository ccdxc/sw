#! /usr/bin/python3
import signal
from subprocess import Popen, PIPE, call
from iota.harness.infra.utils.logger import Logger as Logger

def preexec_function():
    signal.signal(signal.SIGINT, signal.SIG_IGN)

class IotaProcess:
    def __init__(self, cmd, logfile):
        self.__cmd = cmd
        self.__logfile = logfile
        return

    def Start(self):
        Logger.info("Starting IOTA Server: %s (Logfile = %s)" % (self.__cmd, self.__logfile))
        log = open(self.__logfile, "w")
        self.__p = Popen(self.__cmd, stdout=log, stderr=log, shell=True, preexec_fn=preexec_function)
        return

    def Stop(self):
        self.__p.kill()
        return

    def IsRunning(self):
        return self.__p.poll() is None
