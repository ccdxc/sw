#! /usr/bin/python3

def Setup(infra, module):
    iterelem = module.iterator.Get()

    module.logger.info("Iterator Selectors")

    if iterelem:
        if 'flow' in iterelem.__dict__:
            module.logger.info("- flow: %s" % iterelem.flow)
            module.testspec.selectors.flow.Extend(iterelem.flow)

        if 'srcif' in iterelem.__dict__:
            module.logger.info("- srcif: %s" % iterelem.srcif)
            module.testspec.selectors.src.interface.Extend(iterelem.srcif)
        else:
            module.testspec.selectors.src.interface.Extend('filter://any')
        
        if 'dstif' in iterelem.__dict__:
            module.logger.info("- srcif: %s" % iterelem.dstif)
            module.testspec.selectors.dst.interface.Extend(iterelem.dstif)
        else:
            module.testspec.selectors.dst.interface.Extend('filter://any')

        if 'segment' in iterelem.__dict__:
            module.logger.info("- segment: %s" % iterelem.segment)
            module.testspec.selectors.src.segment.Extend(iterelem.segment)
            module.testspec.selectors.dst.segment.Extend(iterelem.segment)

        if 'srcseg' in iterelem.__dict__:
            module.logger.info("- srcseg: %s" % iterelem.srcseg)
            module.testspec.selectors.src.segment.Extend(iterelem.srcseg)

        if 'dstseg' in iterelem.__dict__:
            module.logger.info("- dstseg: %s" % iterelem.dstseg)
            module.testspec.selectors.dst.segment.Extend(iterelem.dstseg)

        if 'priotag' in iterelem.__dict__:
            module.logger.info("- priotag: %s" % iterelem.priotag)

    if module.args == None:
        return

    if 'maxflows' in module.args.__dict__:
        module.logger.info("- maxflows: %s" % module.args.maxflows)
        module.testspec.selectors.maxflows = module.args.maxflows

    return

def Teardown(infra, module):
    return

def Verify(infra, module):
    return True

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return
