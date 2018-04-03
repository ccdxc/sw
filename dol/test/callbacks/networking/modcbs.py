#! /usr/bin/python3
import config.objects.stats_utils as stats_utils
import config.objects.drop_stats as drop_stats

from infra.common.logging import logger as logger

def Setup(infra, module):
    iterelem = module.iterator.Get()

    logger.info("Iterator Selectors")

    if iterelem:
        if 'session' in iterelem.__dict__:
            logger.info("- session: %s" % iterelem.session)
            module.testspec.selectors.session.base.Extend(iterelem.session)

        if 'flow' in iterelem.__dict__:
            logger.info("- flow: %s" % iterelem.flow)
            module.testspec.selectors.flow.Extend(iterelem.flow)

        if 'srcif' in iterelem.__dict__:
            logger.info("- srcif: %s" % iterelem.srcif)
            module.testspec.selectors.src.interface.Extend(iterelem.srcif)
        else:
            module.testspec.selectors.src.interface.Extend('filter://any')
        
        if 'dstif' in iterelem.__dict__:
            logger.info("- srcif: %s" % iterelem.dstif)
            module.testspec.selectors.dst.interface.Extend(iterelem.dstif)
        else:
            module.testspec.selectors.dst.interface.Extend('filter://any')

        if 'segment' in iterelem.__dict__:
            logger.info("- segment: %s" % iterelem.segment)
            module.testspec.selectors.src.segment.Extend(iterelem.segment)
            module.testspec.selectors.dst.segment.Extend(iterelem.segment)

        if 'srcseg' in iterelem.__dict__:
            logger.info("- srcseg: %s" % iterelem.srcseg)
            module.testspec.selectors.src.segment.Extend(iterelem.srcseg)

        if 'dstseg' in iterelem.__dict__:
            logger.info("- dstseg: %s" % iterelem.dstseg)
            module.testspec.selectors.dst.segment.Extend(iterelem.dstseg)

        if 'priotag' in iterelem.__dict__:
            logger.info("- priotag: %s" % iterelem.priotag)

        if 'tenant' in iterelem.__dict__:
            logger.info("- tenant: %s" % iterelem.tenant)
            module.testspec.selectors.src.tenant.Extend(iterelem.tenant)
            module.testspec.selectors.dst.tenant.Extend(iterelem.tenant)

        if 'ep' in iterelem.__dict__:
            logger.info("- ep: %s" % iterelem.tenant)
            module.testspec.selectors.src.endpoint.Extend(iterelem.tenant)
            module.testspec.selectors.dst.endpoint.Extend(iterelem.tenant)

        if 'srcep' in iterelem.__dict__:
            logger.info("- srcep: %s" % iterelem.tenant)
            module.testspec.selectors.src.endpoint.Extend(iterelem.tenant)

        if 'dstep' in iterelem.__dict__:
            logger.info("- dstep: %s" % iterelem.tenant)
            module.testspec.selectors.dst.endpoint.Extend(iterelem.tenant)

    if module.args == None:
        return

    if getattr(module.args, 'maxflows', None):
        module.testspec.selectors.SetMaxFlows(module.args.maxflows)
        logger.info("- maxflows: %s" % module.testspec.selectors.maxflows)

    return

def Teardown(infra, module):
    return

def Verify(infra, module):
    return True

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()
    if iterelem is None:
        return
    tc.pvtdata.priotag = getattr(iterelem, 'priotag', False)
    tc.pvtdata.scenario = getattr(iterelem, 'scenario', None)
    tc.pvtdata.ipopts = getattr(iterelem, 'ipopts', None)
    tc.pvtdata.verify_lif_stats = getattr(iterelem, "lif_stats", False)
    tc.pvtdata.verify_session_stats = getattr(iterelem, "session_stats", False)
    tc.pvtdata.verify_drop_stats = getattr(iterelem, "drop_stats", False)
    tc.pvtdata.drop_reasons = getattr(iterelem, "drop_reasons", [])
    
    if getattr(iterelem, 'drop', False):
        tc.SetDrop()
    if tc.pvtdata.ipopts is not None:
        tc.pvtdata.ipopts = tc.pvtdata.ipopts.split(',')
    return

def TestCasePreTrigger(tc):
    tc.pvtdata.svh = stats_utils.StatsVerifHelper()
    tc.pvtdata.svh.Init(tc)
    if tc.pvtdata.verify_lif_stats:
        tc.pvtdata.svh.InitLifStats(tc)
    if tc.pvtdata.verify_session_stats:
        tc.pvtdata.svh.InitSessionStats(tc)

    if tc.pvtdata.verify_drop_stats:
        tc.pvtdata.dvh = drop_stats.DropStatsVerifHelper()
        tc.pvtdata.dvh.Init(tc)
    return

def TestCaseTeardown(tc):
    root = getattr(tc.config, 'flow', None)
    if root is None:
       root = getattr(tc.config.session.iconfig, 'flow', None)

    if root.IsFteEnabled() is False:
        return

    fte_session_aware = getattr(tc.pvtdata, 'fte_session_aware', True)
    if fte_session_aware is True:
        session = root.GetSession()
        session.SetLabel("FTE_DONE")
    else:
        root.SetLabel("FTE_DONE")
    return

def TestCaseStepSetup(tc, step):
    assert(tc is not None)
    assert(step is not None)
    return

def TestCaseStepTrigger(tc, step):
    assert(tc is not None)
    assert(step is not None)
    return

def TestCaseStepVerify(tc, step):
    assert(tc is not None)
    assert(step is not None)
    return True

def TestCaseStepTeardown(tc, step):
    assert(tc is not None)
    assert(step is not None)
    return

def TestCaseVerify(tc):
    ret = True
    if tc.pvtdata.verify_lif_stats and\
       tc.pvtdata.svh.VerifyLifStats(tc) is False:
        ret = False
    if tc.pvtdata.verify_session_stats and\
       tc.pvtdata.svh.VerifySessionStats(tc) is False:
        ret = False
    if tc.pvtdata.verify_drop_stats and\
       tc.pvtdata.dvh.Verify(tc) is False:
        ret = False
    return ret
