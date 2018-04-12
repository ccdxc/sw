#! /usr/bin/python3
import test.firewall.tracker.tracker as trackermod
import test.callbacks.networking.modcbs as modcbs

from test.firewall.tracker.store import TrackerStore
def InitTracker(infra, module):
    tracker = None
    connspec = getattr(module.args, 'connspec', None)
    if connspec is not None:
        tracker = trackermod.TrackerObject()
        tracker.Init(module.name, module.args.connspec)
        TrackerStore.trackers.Add(tracker)

    tracker_ref = getattr(module.args, 'tracker', None)
    if tracker_ref is not None:
        tracker = tracker_ref.Get(TrackerStore)

    assert(tracker is not None)
    return tracker

def Setup(infra, module):
   return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    stepspec = tc.module.iterator.Get()
    tc.tracker.SetStep(stepspec, tc)
    tc.tracker.SetupTestcaseConfig(tc)
    return

def TestCasePreTrigger(tc):
    return modcbs.TestCasePreTrigger(tc)

def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)

def TestCaseTeardown(tc):
    tc.tracker.Advance()
    return

