#! /usr/bin/python3

from infra.factory.store import FactoryStore

def GetPacketTemplate(pktid):
    return FactoryStore.packets.Get(pktid)
