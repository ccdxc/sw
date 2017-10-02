#! /usr/bin/python3
import test.firewall.tracker.tracker as trackermod

def InitTracker(infra, module):
    tracker = trackermod.TrackerObject()
    tracker.Init(module.args.connspec, module.logger)
    return tracker

def Setup(infra, module):
   return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    stepspec = tc.module.iterator.Get()
    tc.tracker.SetStep(stepspec, tc)
    tc.tracker.SetupTestcaseConfig(tc)
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    tc.tracker.Advance()
    return

