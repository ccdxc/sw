#! /usr/bin/python3

import infra.common.objects as objects

from infra.common.logging import logger as logger

class TrackerStoreObject:
    def __init__(self):
        self.connections    = objects.ObjectDatabase(logger)
        self.steps          = objects.ObjectDatabase(logger)
        self.testcases      = objects.ObjectDatabase(logger)
        return

TrackerStore = TrackerStoreObject()
