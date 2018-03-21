#! /usr/bin/python3
import pdb
from infra.common.logging import logger as logger

def Setup(infra, module):
    iterelem = module.iterator.Get()
    module.pvtdata.profile = None
    
    flowsel = getattr(iterelem, 'flow', None)
    if flowsel is not None:
        module.testspec.selectors.flow.Extend(flowsel)
    return

def Teardown(infra, module):
    if module.iterator.End():
        asp = infra.ConfigStore.objects.Get('SEC_PROF_ACTIVE')
        profile = infra.ConfigStore.objects.Get('SEC_PROF_DEFAULT')
        logger.info("Restoring Active Security Profile --> SEC_PROF_DEFAULT")
        asp.CloneFields(profile)
        asp.Update()
    return

def TestCaseSetup(tc):
    asp = tc.module.infra_data.ConfigStore.objects.Get('SEC_PROF_ACTIVE')

    iterelem = tc.module.iterator.Get()
    if iterelem is None:
        return

    pfname = getattr(iterelem, 'profile', None)
    if pfname is not None and tc.module.pvtdata.profile != pfname:
        profile = tc.module.infra_data.ConfigStore.objects.Get(pfname)
        logger.info("Updating Active Security Profile --> %s" % profile)
        asp.CloneFields(profile)
        asp.Update()
        tc.module.pvtdata.profile = pfname

    return


def TestCaseVerify(tc):
    return True
