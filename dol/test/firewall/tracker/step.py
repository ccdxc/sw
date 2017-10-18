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
        self.__init_step()
        return

    def __init_step(self):
        base = getattr(self, 'base', None)
        if base is None:
            return
        base = base.Get(TrackerStore)
        self.fields = objects.MergeObjects(self.fields, base.fields)
        self.state = getattr(self, 'state', None)
        self.payloadsize = getattr(self, 'payloadsize', base.payloadsize)
        self.direction = getattr(self, 'direction', base.direction)
        self.advance = getattr(self, 'advance', base.advance)
        self.permit = getattr(self, 'permit', base.permit)
        self.delay = 1
        return

    def __copy__(self):
        obj = type(self)()
        obj.GID(self.GID())
        obj.fields = copy.copy(self.fields)
        obj.state = copy.deepcopy(self.state)
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
    def NeedsAdvance(self):
        return self.advance
    def IsCpuCopyValid(self):
        return self.cpu_copy_valid
    def SetCpuCopyValid(self, cpu_copy_valid):
        self.cpu_copy_valid = cpu_copy_valid

    def __resolve(self, attr, orig):
        curr = getattr(self.fields, attr, None)
        value = curr
        if curr is None:
            value = orig
        elif objects.IsCallback(curr):
            value = curr.call(self, orig)
        assert(value is not None)
        setattr(self.fields, attr, value)
        return

    def SetFlowStates(self, fs, nbrfs):
        self.flowstate = fs
        self.neighbor_flowstate = nbrfs
        self.__resolve('seq', fs.seq)
        self.__resolve('ack', fs.ack)
        self.__resolve('flags', fs.flags)
        self.__resolve('window', fs.window)
        self.__resolve('scale', fs.scale)
        self.__resolve('mss', fs.mss)
        return

    def SetPorts(self, iport, rport):
        if self.IsIflow():
            self.fields.sport = iport
            self.fields.dport = rport
        else:
            self.fields.sport = rport
            self.fields.dport = iport
        return

    def Show(self, lg):
        lg.info("Tracker Step: %s" % self.GID())
        lg.info("- Seq      : ", self.fields.seq)
        lg.info("- Ack      : ", self.fields.ack)
        lg.info("- Flags    : ", self.fields.flags)
        lg.info("- Window   : ", self.fields.window)
        lg.info("- Scale    : ", self.fields.scale)
        lg.info("- MSS      : ", self.fields.mss)
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
