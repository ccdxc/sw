#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.telemetry.telemetry as telemetry
import test.callbacks.networking.modcbs as modcbs

from infra.common.logging import logger as logger

def Setup(infra, module):

    iterelem = module.iterator.Get()

    logger.info("ACL id - %s" % iterelem.id)
    if 'flow' in iterelem.__dict__:
        module.testspec.selectors.flow.Extend(iterelem.flow)

    if 'srcseg' in iterelem.__dict__:
        module.testspec.selectors.src.segment.Extend(iterelem.srcseg)

    if 'mirror' in iterelem.__dict__:
        telemetry.setup_span(infra, module, iterelem.mirror)
    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    modcbs.TestCaseSetup(tc)
    iterelem = tc.module.iterator.Get()

    acl = tc.infra_data.ConfigStore.objects.Get(iterelem.id)
    tc.pvtdata.acl = acl
    tc.pvtdata.scenario = getattr(iterelem, 'scenario', None)

    if 'mirror' in iterelem.__dict__:
        tc.pvtdata.span_case = iterelem.mirror
        tc.config.ingress_mirror.session1 = acl.GetIngressMirrorSession(idx = 1)
        tc.config.ingress_mirror.session2 = acl.GetIngressMirrorSession(idx = 2)
        tc.config.ingress_mirror.session3 = acl.GetIngressMirrorSession(idx = 3)
        tc.config.egress_mirror.session1 = acl.GetEgressMirrorSession(idx = 1)
        tc.config.egress_mirror.session2 = acl.GetEgressMirrorSession(idx = 2)
        tc.config.egress_mirror.session3 = acl.GetEgressMirrorSession(idx = 3)

    # Update the ACL parameters based on the testcase
    acl.UpdateFromTCConfig(tc.config.flow, tc.config.src.endpoint, tc.config.dst.endpoint,\
                           tc.config.src.segment, tc.config.src.tenant)

    acl.Update()
    return

def TestCasePreTrigger(tc):
    return modcbs.TestCasePreTrigger(tc)

def TestCaseVerify(tc):
    return modcbs.TestCaseVerify(tc)

def TestCaseTeardown(tc):
    return
