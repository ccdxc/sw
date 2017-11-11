#! /usr/bin/python3
import copy
import pdb

import infra.common.objects as objects
import test.firewall.tracker.step as step
import test.firewall.tracker.connection as connection
import test.firewall.tracker.flowstate as flowstate

from infra.common.logging import logger as logger
from test.firewall.tracker.store import TrackerStore
class TrackerObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.lg         = None
        self.conn       = None
        self.flowstate  = flowstate.FlowStateTracker()
        self.config     = None
        return

    def Init(self, gid, connspec, lg):
        self.GID(gid)
        self.lg = lg
        self.conn = connspec.Get(TrackerStore)
        lg.info("- Loading Connection Params from Spec: %s" % self.conn.GID())
        self.flowstate.Init(self.conn, lg)
        self.iport = self.flowstate.GetInitiatorPort()
        self.rport = self.flowstate.GetResponderPort()
        return

    def Advance(self):
        if self.step.IsDrop():
            self.lg.info("- Step Action is NOT PERMIT. Not advancing the step...")
            return
        if self.step.NeedsAdvance() is False:
            self.lg.info("- Step Advance is False. Not advancing the step...")
            return
        self.flowstate.Advance(self.step)
        return

    def IsCpuCopyValid(self):
        return self.step.IsCpuCopyValid()

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
        self.__set_flow_states()
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
