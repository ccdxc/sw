#! /usr/bin/python3

import infra.common.objects as objects

from infra.common.logging import logger as logger

class TrackerStoreObject:
    def __init__(self):
        self.connections    = objects.ObjectDatabase()
        self.steps          = objects.ObjectDatabase()
        self.testcases      = objects.ObjectDatabase()
        self.trackers       = objects.ObjectDatabase()
        return

TrackerStore = TrackerStoreObject()
