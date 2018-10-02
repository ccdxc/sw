#! /usr/bin/python3

import iota.harness.infra.types as types
import iota.harness.infra.svc as svc

from iota.harness.infra.logger import Logger as Logger

class HealthMonitor:
    def __init__(self):
        return

    def CheckHealth(self):
        resp = svc.CheckClusterHealth()
        Logger.info("Validation")

        
