#! /usr/bin/python3
import copy
import pdb

import test.firewall.tracker.step as step
import test.firewall.tracker.connection as connection
import test.firewall.tracker.flowstate as flowstate

from infra.common.logging import logger as logger
from test.firewall.tracker.store import TrackerStore
class TrackerObject:
    def __init__(self):
        self.lg         = None
        self.conn       = None
        self.flowstate  = flowstate.FlowStateTracker()
        self.config     = None
        return

    def Init(self, connspec, lg):
        self.lg = lg
        self.conn = connspec.Get(TrackerStore)
        lg.info("- Loading Connection Params from Spec: %s" % self.conn.GID())
        self.flowstate.Init(self.conn, lg)
        self.cpu_copy_valid = True
        self.iport = self.flowstate.GetInitiatorPort()
        self.rport = self.flowstate.GetResponderPort()
        return

    def Advance(self):
        self.cpu_copy_valid = False
        if self.step.IsDrop():
            self.lg.info("- Step Action is NOT PERMIT. Not advancing the step...")
            return
        if self.step.NeedsAdvance() is False:
            self.lg.info("- Step Advance is False. Not advancing the step...")
            return
        self.flowstate.Advance(self.step)
        return

    def IsCpuCopyValid(self):
        return self.cpu_copy_valid

    def SetStep(self, stepspec, lg):
        self.lg = lg
        step = stepspec.step.Get(TrackerStore)
        self.step = copy.copy(step)
        state = self.flowstate.GetState(self.step.IsIflow())
        self.step.CopyFlowState(state)
        self.step.SetPorts(self.iport, self.rport)
        self.step.Show(self.lg)
        return

    def SetupTestcaseConfig(self, tc):
        if self.step.IsIflow():
            self.config = tc.config.session.iconfig
        else:
            self.config = tc.config.session.rconfig
        return

logger.info("Loading Tracker Connection Specs:")
connection.LoadConnectionSpecs()
logger.info("Loading Tracker Step Specs:")
step.LoadStepSpecs()
