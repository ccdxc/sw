#! /usr/bin/python3
import pdb
import copy

import test.firewall.tracker.step as base
import infra.common.parser as parser
import infra.common.objects as objects
from infra.common.logging import logger as logger
from test.fte.alg.tracker.store import TrackerStore

class StepALGObject(base.StepObject):
    def __init__(self, spec = None):
        super().__init__(spec)
        self.pkt_template = None
        self.fte_done = None

    def __init_step(self):
        super().__init_step()
        self.pkt_template = None
        self.fte_done = None
        return

    def IsSyn(self):
        if self.fields.flags is not None:
            return 'syn' in self.fields.flags
        else:
            return False
    def IsFin(self):
        if self.fields.flags is not None:
            return 'fin' in self.fields.flags
        else:
            return False

    def PktTemplate(self):
        if 'pkt_template' in dir(self): 
            return self.pkt_template
    def SetPktTemplate(self, pkt_template):
        self.pkt_template = pkt_template

    def IsFteDone(self):
        return self.fte_done
    def SetFTEDone(self, fte_done):
        self.fte_done = fte_done

    def SetPorts(self, iport, rport):
        if self.IsIflow():
            if 'sport' not in dir(self.fields):
                self.fields.sport = iport
            if 'dport' not in dir(self.fields):
                self.fields.dport = rport
        else:
            if 'sport' not in dir(self.fields):
                self.fields.sport = rport
            if 'dport' not in dir(self.fields):
                self.fields.dport = iport
        return
 
    def SetIPs(self, tc):
        if self.IsIflow():
            if 'sip' not in dir(self.fields):
                self.fields.sip = tc.config.session.iconfig.flow.sip
            if 'dip' not in dir(self.fields):
                self.fields.dip = tc.config.session.iconfig.flow.dip
        else:
            if 'sip' not in dir(self.fields):
                self.fields.sip = tc.config.session.rconfig.flow.sip 
            if 'dip' not in dir(self.fields):
                self.fields.dip = tc.config.session.rconfig.flow.dip
        return

    def Show(self):
        logger.info("Tracker Step: %s" % self.GID())
        logger.info("- Sport      : ", self.fields.sport)
        logger.info("- Dport      : ", self.fields.dport)
        if 'datapgm' in dir(self.fields):
            logger.info("- datapgm    : ", self.fields.datapgm)
        if 'dataproto' in dir(self.fields):
            logger.info("- dataproto  : ", self.fields.dataproto)
        return

def __parse_step_specs():
    path = 'test/fte/alg/tracker/specs/'
    filename = 'steps.spec'
    return parser.ParseDirectory(path, filename)

def LoadStepSpecs():
    specs = __parse_step_specs()
    for spec in specs:
        for stspec in spec.steps:
            obj = StepALGObject(stspec.step)
            logger.info("- Loading Tracker Step Spec: %s" % obj.GID())
            TrackerStore.steps.Set(obj.GID(), obj)
    return
