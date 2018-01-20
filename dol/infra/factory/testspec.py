#! /usr/bin/python3
import pdb
import copy
import sys

import infra.common.defs    as defs
import infra.common.parser  as parser
import infra.common.objects as objects
import infra.common.utils   as utils

from infra.common.glopts import GlobalOptions as GlobalOptions

from infra.factory.store import FactoryStore as FactoryStore

class TestSpecConfigSelectors(objects.FrameworkObject):
    def __init__(self, spec):
        super().__init__()
        self.Clone(spec)
        self.spec = spec
        return

    def DeriveLimits(self):
        if GlobalOptions.regression:
            self.maxflows = None
            self.maxsessions = None
            self.maxrdmasessions = None
        elif GlobalOptions.rtl:
            self.maxflows = 1
            self.maxsessions = 1
        return

    def SetMaxFlows(self, maxflows):
        if GlobalOptions.regression:
            return
        self.maxflows = maxflows
        return

    def IsFlowBased(self):
        return self.Valid() and self.flow != None

    def IsSessionBased(self):
        return self.Valid() and self.session != None

    def IsRdmaSessionBased(self):
        return self.Valid() and self.rdmasession != None

    def Valid(self):
        if self.flow and self.session:
            return False
        return True

    def Merge(self, selectors):
        newspec = objects.MergeObjects(selectors, self.spec)
        self.Clone(newspec)
        return

    def SwapSrcDst(self):
        self.src,self.dst = self.dst,self.src
        return

    def SetFlow(self, flow):
        self.flow = flow

    def GetFlowLabel(self):
        return self.flow.GetValueByKey('label')

class TestSpecSessionStepEntry(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.step = None
        return

class TestSpecSessionStep(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

class TestSpecObject(objects.FrameworkObject):
    def __init__(self, path, filename, logger):
        super().__init__()
        tspec = parser.ParseFile(path, filename)
        if tspec is None:
            logger.error("Failed to parse testspec file: %s/%s" %\
                         (path, filename))
            sys.exit(1)
        self.Clone(tspec)
        
        self.template = FactoryStore.testobjects.Get('TESTSPEC')
        #self.template.SetReadOnly()
        #self.template = copy.deepcopy(self.template)

        # Check if the TestSpec is a session or non-session
        if hasattr(self, 'session') == False:
            entry = TestSpecSessionStepEntry()
            self.session = [ entry ]

        if self.session[0].step == None:
            # Non-session: Add a default session step and
            # move the trigger and expectations inside.
            step = TestSpecSessionStep()
            step.trigger = self.trigger
            step.expect = self.expect
            self.session[0].step = step

        self.__merge()
        self.selectors = TestSpecConfigSelectors(self.selectors)
        return

    def MergeSelectors(self, selectors):
        if selectors is not None:
            self.selectors.Merge(selectors)
        return

    def DeriveLimits(self):
        self.selectors.DeriveLimits()
        return

    def __merge_section(self, name):
        spsn = None
        tpsn = self.template.__dict__[name]
        if name in self.__dict__:
            spsn = self.__dict__[name]
        
        self.__dict__[name] = objects.MergeObjects(spsn, tpsn)
        return

    def __merge_obj_list(self, splist, tp):
        objs = []
        for sp in splist:
            obj = objects.MergeObjects(sp, tp)
            objs.append(obj)
        return objs

    def __merge_packets(self, sp, tp):
        if sp == None or tp == None: return
        if 'packets' not in sp.__dict__: return None
        return self.__merge_obj_list(sp.packets, tp.packets[0])
            
    def __merge_descriptors(self, sp, tp):
        if sp == None or tp == None: return
        if 'descriptors' not in sp.__dict__: return None
        return self.__merge_obj_list(sp.descriptors, tp.descriptors[0])

    def __merge_buffers(self, sp, tp):
        if sp == None or tp == None: return
        if 'buffers' not in sp.__dict__: return None
        return self.__merge_obj_list(sp.buffers, tp.buffers[0])

    def __merge_doorbell(self, sp, tp):
        if sp == None or tp == None: return
        if 'doorbell' not in sp.__dict__: return None
        return objects.MergeObjects(sp.doorbell, tp.doorbell)

    def __merge_trig_exp_common(self, sp, tp):
        if sp == None or tp == None: return None
        sp.packets = self.__merge_packets(sp, tp)
        sp.descriptors = self.__merge_descriptors(sp, tp)
        sp.buffers = self.__merge_buffers(sp, tp)
        sp.doorbell = self.__merge_doorbell(sp, tp)
        return sp

    def __merge_step(self, sp, tp):
        sp.trigger = self.__merge_trig_exp_common(sp.trigger, tp.trigger)
        sp.expect = self.__merge_trig_exp_common(sp.expect, tp.expect)
        return sp

    def __merge_step_entry(self, sp, tp):
        sp.step = self.__merge_step(sp.step, tp.step)
        return sp

    def __merge_session(self):
        spsn = self.__dict__['session']
        tpsn = self.template.__dict__['session']
        tp = tpsn[0]
        steps = []
        for sp in spsn:
            step = self.__merge_step_entry(sp, tp)
            steps.append(step)
        self.__dict__['session'] = steps
        return

    def __merge(self):
        self.__merge_section('selectors')
        self.__merge_section('packets')
        self.__merge_section('descriptors')
        self.__merge_section('buffers')
        self.__merge_section('state')
        self.__merge_session()
        return

