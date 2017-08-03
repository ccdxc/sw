#! /usr/bin/python3
import pdb
import copy

import infra.common.defs    as defs
import infra.common.parser  as parser
import infra.common.objects as objects
import infra.common.utils   as utils

from infra.factory.store import FactoryStore as FactoryStore

class TestSpecSessionStepEntry(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.step = None
        return

class TestSpecSessionStep(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

class TestSpec(objects.FrameworkObject):
    def __init__(self, path, filename):
        super().__init__()
        tspec = parser.ParseFile(path, filename)
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
        return

    def __merge_section(self, sec_name):
        spec_section = None
        template_section = self.template.__dict__[sec_name]
        if sec_name in self.__dict__:
            spec_section = self.__dict__[sec_name]

        self.__dict__[sec_name] = objects.MergeObjects(spec_section,
                                                       template_section)
        return

    def __merge(self):
        self.__merge_section('config_filter')
        self.__merge_section('packets')
        self.__merge_section('descriptors')
        self.__merge_section('buffers')
        self.__merge_section('state')
        self.__merge_section('session')
        #self.__merge_section('trigger')
        #self.__merge_section('expect')
        return

