#! /usr/bin/python3

import pdb
def Setup(infra, module):
    asp = infra.ConfigStore.objects.Get('SEC_PROF_ACTIVE')
    iterelem = module.iterator.Get()
    profile = infra.ConfigStore.objects.Get(iterelem.profile)
    module.logger.info("Updating Active Security Profile --> %s" % profile)
    asp.CloneFields(profile)
    asp.Update()

    flowsel = getattr(iterelem, 'flow', None)
    if flowsel is not None:
        module.testspec.selectors.flow.Extend(flowsel)
    return

def Teardown(infra, module):
    if module.iterator.End():
        asp = infra.ConfigStore.objects.Get('SEC_PROF_ACTIVE')
        profile = infra.ConfigStore.objects.Get('SEC_PROF_DEFAULT')
        module.logger.info("Restoring Active Security Profile --> SEC_PROF_DEFAULT")
        asp.CloneFields(profile)
        asp.Update()
    return

def TestCaseVerify(tc):
    return True
