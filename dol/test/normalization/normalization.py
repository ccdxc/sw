#! /usr/bin/python3
import pdb
def Setup(infra, module):
    module.testspec.config_filter.flow.Extend(module.args.flow)
    asp = infra.ConfigStore.objects.Get('SEC_PROF_ACTIVE')
    profile = infra.ConfigStore.objects.Get(module.iterator.Get())
    module.logger.info("Updating Active Security Profile --> %s" % module.iterator.Get())
    asp.CloneFields(profile)
    asp.Update()
    return

def Teardown(infra, module):
    asp = infra.ConfigStore.objects.Get('SEC_PROF_ACTIVE')
    profile = infra.ConfigStore.objects.Get('SEC_PROF_DEFAULT')
    module.logger.info("Restoring Active Security Profile --> SEC_PROF_DEFAULT")
    asp.CloneFields(profile)
    asp.Update()
    return
