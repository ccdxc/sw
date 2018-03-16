#! /usr/bin/python3

import infra.common.objects as objects
import test.firewall.tracker.store as base

from infra.common.logging import logger as logger

class ALGTrackerStoreObject(base.TrackerStoreObject):
    def __init__(self):
        base.TrackerStoreObject.__init__(self)

TrackerStore = ALGTrackerStoreObject()
