#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs

igmp_acl = False

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return

def TestCaseSetup(tc):
    global igmp_acl
    iterelem = tc.module.iterator.Get()

    tc.pvtdata.priotag = False
    if 'priotag' in iterelem.__dict__:
        tc.pvtdata.priotag = iterelem.priotag

    if iterelem.id == 'IGMP-TO-CPU' and not igmp_acl:
        acl = tc.infra_data.ConfigStore.objects.Get('ACL_IPV4_IGMP_ACTION_SUP_REDIRECT')
        tc.pvtdata.acl = acl

        # Update the ACL parameters based on the testcase
        acl.UpdateFromTCConfig(tc.config.flow, tc.config.src.endpoint, tc.config.dst.endpoint,\
                               tc.config.src.segment, tc.config.src.tenant)

        acl.Configure()
        igmp_acl = True

    return

