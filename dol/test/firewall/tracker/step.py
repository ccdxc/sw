#! /usr/bin/python3
import pdb
import copy

import infra.common.parser as parser
import infra.common.objects as objects
from infra.common.logging import logger as logger
from test.firewall.tracker.store import TrackerStore as TrackerStore

class StepTcpOptions:
    def __init__(self):
        self.nop_v          = None
        self.nop            = None
        self.spec_nop       = None
        self.scale_v        = None
        self.scale          = None
        self.spec_scale     = None
        self.sack_v         = None
        self.sack           = None
        self.spec_sack      = None
        self.sackok_v       = None
        self.sackok         = None
        self.spec_sackok    = None
        self.timestamp_v    = None
        self.timestamp      = None
        self.spec_timestamp = None
        self.mss_v          = None
        self.mss            = None
        self.spec_mss       = None
        return

    def __init_sackok(self, spec):
        self.sackok_v = 'SAckOK'
        self.sackok = str(spec.sackok)
        self.spec_sackok = spec.sackok
        return

    def __init_sack(self, spec):
        self.sack_v = 'SAck'
        self.sack = spec.sack
        self.spec_sack = spec.sack
        return

    def __init_timestamp(self, spec):
        self.timestamp_v = 'Timestamp'
        self.spec_timestamp = spec.timestamp
        low = spec.timestamp & 0xFFFFFFFF
        high = (spec.timestamp >> 32) & 0xFFFFFFFF
        self.timestamp = '%d %d' % (high, low)
        return

    def __init_scale(self, spec):
        self.scale_v = 'WScale'
        self.scale = spec.scale
        self.spec_scale = spec.scale
        return

    def __init_mss(self, spec):
        self.mss_v = 'MSS'
        self.mss = spec.mss
        self.spec_mss = spec.mss
        return

    def __init_nop(self, spec):
        self.nop_v = 'NOP'
        self.nop = None
        self.spec_nop = spec.nop
        return

    def Init(self, spec):
        if getattr(spec, 'timestamp', None) is not None:
            self.__init_timestamp(spec)

        if getattr(spec, 'scale', None) is not None:
            self.__init_scale(spec)

        if getattr(spec, 'mss', None) is not None:
            self.__init_mss(spec)

        if getattr(spec, 'nop', None) is not None:
            self.__init_nop(spec)

        if getattr(spec, 'sackok', None) is not None:
            self.__init_sackok(spec)
        
        if getattr(spec, 'sack', None) is not None:
            self.__init_sack(spec)
        return

    def Show(self, lg):
        lg.info("- Options:")
        if self.nop_v is not None:
            lg.info("  - NOP        :", self.spec_nop)
        if self.scale_v is not None:
            lg.info("  - Scale      :", self.spec_scale)
        if self.sack_v is not None:
            lg.info("  - Sack       :", self.spec_sack)
        if self.sackok_v is not None:
            lg.info("  - SackOK     :", self.spec_sackok)
        if self.timestamp_v is not None:
            lg.info("  - Timestamp  :", self.spec_timestamp)
        if self.mss_v is not None:
            lg.info("  - MSS        :", self.spec_mss)
        return

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

    def __resolve_by_callback(self, cb, default):
        value = cb.call(self, default)
        return value

    def __resolve(self, obj, attr, default):
        curr = getattr(obj, attr, None)
        value = curr
        if curr is None:
            value = default
        elif objects.IsReference(curr):
            value = curr.Get(self)
        elif objects.IsCallback(curr):
            value = self.__resolve_by_callback(curr, default)
            value = value & 0xffffffff
        setattr(obj, attr, value)
        return

    def __resolve_all(self, obj, fstate):
        self.__resolve(obj, 'seq', fstate.seq)
        self.__resolve(obj, 'ack', fstate.ack)
        self.__resolve(obj, 'flags', fstate.flags)
        self.__resolve(obj, 'window', fstate.window)
        self.__resolve(obj, 'scale', fstate.scale)
        self.__resolve(obj, 'mss', fstate.mss)
        return

    def __resolve_options(self):
        if getattr(self.fields, 'options', None) is None:
            return

        self.__resolve(self.fields.options, 'timestamp', None)
        self.__resolve(self.fields.options, 'mss', None)
        self.__resolve(self.fields.options, 'scale', None)
        return 

    def __process_options(self):
        options = StepTcpOptions()
        spec = getattr(self.fields, 'options', None)
        if spec is not None:
            options.Init(spec)
        self.fields.options = options
        return

    def __resolve_fields(self):
        self.__resolve_all(self.fields, self.myfstate)
        self.__resolve_options()
        self.__process_options()
        return

    def __resolve_state_iflow(self):
        if getattr(self.state, 'iflow', None) is None:
            return
        self.__resolve_all(self.state.iflow, self.ifstate)
        return

    def __resolve_state_rflow(self):
        if getattr(self.state, 'rflow', None) is None:
            return
        self.__resolve_all(self.state.rflow, self.rfstate)
        return

    def __resolve_state(self):
        if getattr(self, 'state', None) is None:
            return
        self.__resolve_state_iflow()
        self.__resolve_state_rflow()
        return

    def SetFlowStates(self, ifstate, rfstate):
        self.ifstate = ifstate
        self.rfstate = rfstate
        if self.IsIflow():
            self.myfstate = ifstate
            self.prfstate = rfstate
        else:
            self.myfstate = rfstate
            self.prfstate = ifstate
        self.__resolve_fields()
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
        self.fields.options.Show(lg)
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
