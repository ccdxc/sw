#! /usr/bin/python3
import pdb
import copy

import infra.common.parser as parser
import infra.common.objects as objects
from infra.common.logging import logger as logger
from test.firewall.tracker.store import TrackerStore as TrackerStore

class StepObject(objects.FrameworkTemplateObject):
    def __init__(self, spec = None):
        super().__init__()
        if spec is None:
            return
        self.Clone(spec)
        self.GID(spec.id)
        self.__resolve()
        return

    def __resolve(self):
        base = getattr(self, 'base', None)
        if base is None:
            return
        base = base.Get(TrackerStore)
        self.fields = objects.MergeObjects(self.fields, base.fields)
        self.payloadsize = getattr(self, 'payloadsize', base.payloadsize)
        self.direction = getattr(self, 'direction', base.direction)
        self.advance = getattr(self, 'advance', base.advance)
        self.permit = getattr(self, 'permit', base.permit)
        self.delay = 1
        return

    def __copy__(self):
        obj = type(self)()
        obj.fields = copy.copy(self.fields)
        obj.payloadsize = self.payloadsize
        obj.direction = self.direction
        obj.advance = self.advance
        obj.permit = self.permit
        obj.delay = self.delay
        return obj
        
    def IsIflow(self):
        return self.direction == 'iflow'

    def IsSyn(self):
        return 'syn' in self.fields.flags

    def IsFin(self):
        return 'fin' in self.fields.flags

    def IsDrop(self):
        return self.permit == False

    def CopyFlowState(self, fs):
        self.fields.seq    = getattr(self.fields, 'seq', fs.seq)
        self.fields.ack    = getattr(self.fields, 'ack', fs.ack)
        self.fields.flags  = getattr(self.fields, 'flags', fs.flags)
        self.fields.win    = getattr(self.fields, 'win', fs.win)
        self.fields.scale  = getattr(self.fields, 'scale', fs.scale)
        self.fields.mss    = getattr(self.fields, 'mss', fs.mss)
        return

    def Show(self, lg):
        lg.info("Tracker Step: %s" % self.GID())
        lg.info("- Seq  : ", self.fields.seq)
        lg.info("- Ack  : ", self.fields.ack)
        lg.info("- Flags: ", self.fields.flags)
        lg.info("- Win  : ", self.fields.win)
        lg.info("- Scale: ", self.fields.scale)
        lg.info("- MSS  : ", self.fields.mss)
        return

def __parse_step_specs():
    path = 'test/firewall/tracker/specs/'
    filename = 'steps.spec'
    return parser.ParseDirectory(path, filename)

def LoadStepSpecs():
    specs = __parse_step_specs()
    for spec in specs:
        for stspec in spec.steps:
            obj = StepObject(stspec.step)
            logger.info("- Loading Tracker Step Spec: %s" % obj.GID())
            TrackerStore.steps.Set(obj.GID(), obj)
    return
