#! /usr/bin/python3

from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger as logger
class FactoryStoreObject:
    def __init__(self):
        self.headers        = ObjectDatabase()
        self.packets        = ObjectDatabase()
        self.payloads       = ObjectDatabase()
        
        self.templates      = ObjectDatabase()
        self.specs          = ObjectDatabase()

        self.testobjects    = ObjectDatabase()
        return

    def GetHeaderByScapyId(self, sid):
        for hdr in self.headers.GetAll():
            if hdr.meta.scapy == sid:
                return hdr
        return None

FactoryStore = FactoryStoreObject()
