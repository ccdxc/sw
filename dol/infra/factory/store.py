#! /usr/bin/python3

from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import pktlogger as pktlogger
class FactoryStoreObject:
    def __init__(self):
        self.headers        = ObjectDatabase(pktlogger)
        self.packets        = ObjectDatabase(pktlogger)
        self.payloads       = ObjectDatabase(pktlogger)
        
        self.templates      = ObjectDatabase(pktlogger)
        self.specs          = ObjectDatabase(pktlogger)

        self.testobjects    = ObjectDatabase(pktlogger)
        return

    def GetHeaderByScapyId(self, sid):
        for hdr in self.headers.GetAll():
            if hdr.meta.scapy == sid:
                return hdr
        return None

FactoryStore = FactoryStoreObject()
