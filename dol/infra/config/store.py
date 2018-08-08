#! /usr/bin/python3
from infra.common.objects import ObjectDatabase as ObjectDatabase
from infra.common.logging import logger as logger

class ConfigStore_:
    def __init__(self):
        self.objects    = ObjectDatabase()
        self.templates  = ObjectDatabase()
        self.specs      = ObjectDatabase()
        return

ConfigStore = ConfigStore_()
