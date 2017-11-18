#! /usr/bin/python3
import copy
import pdb

import test.firewall.tracker.tracker as base 
import test.fte.tracker.step as step
import test.fte.tracker.connection as connection
import test.firewall.tracker.flowstate as flowstate

from infra.common.logging import logger as logger
from test.fte.tracker.store import TrackerStore

class ALGTrackerObject(base.TrackerObject):
    def __init__(self):
        super().__init__()

    def Init(self, gid, connspec, lg):
        super().Init(gid, connspec, lg)

    def PktTemplate(self):
        return self.step.PktTemplate()

    def IsFteDone(self):
        return self.step.IsFteDone()

    def __set_flow_states(self):
        ifstate = self.flowstate.GetState(True)
        rfstate = self.flowstate.GetState(False)
        self.step.SetFlowStates(ifstate, rfstate)
        return

    def SetStep(self, stepspec, lg):
        self.lg = lg
        step = stepspec.step.Get(TrackerStore)
        self.step = copy.copy(step)
        self.step.SetCpuCopyValid(getattr(stepspec, 'cpu', False))
        self.step.SetFTEDone(getattr(stepspec, 'fte_done', False))
        self.__set_flow_states()
        self.step.SetPorts(self.iport, self.rport)
        self.step.Show(self.lg)
        if 'pkttemplate' in dir(stepspec):
            self.step.SetPktTemplate(getattr(stepspec, 'pkttemplate'))

logger.info("Loading Tracker Connection Specs:")
connection.LoadConnectionSpecs()
logger.info("Loading Tracker Step Specs:")
step.LoadStepSpecs()
