#! /usr/bin/python3
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import cfglogger as cfglogger

class ConfigStore:
    def __init__(self):
        self.objects    = ObjectDatabase(cfglogger)
        self.templates  = ObjectDatabase(cfglogger)
        self.specs      = ObjectDatabase(cfglogger)
        
        # Custom Database for easy access.
        self.trunk_uplinks = ObjectDatabase(cfglogger)
        return

Store = ConfigStore()
