#! /usr/bin/python3

def Setup(infra, module):
    iterelem = module.iterator.Get()

    if iterelem:
        if 'flow' in iterelem.__dict__:
            module.testspec.config_filter.flow.Extend(iterelem.flow)

        if 'srcif' in iterelem.__dict__:
            module.testspec.config_filter.src.interface.Extend(iterelem.srcif)
        else:
            module.testspec.config_filter.src.interface.Extend('filter://any')
        
        if 'dstif' in iterelem.__dict__:
            module.testspec.config_filter.dst.interface.Extend(iterelem.dstif)
        else:
            module.testspec.config_filter.dst.interface.Extend('filter://any')

        if 'segment' in iterelem.__dict__:
            module.testspec.config_filter.src.segment.Extend(iterelem.segment)
            module.testspec.config_filter.dst.segment.Extend(iterelem.segment)

    if module.args == None:
        return

    if 'maxflows' in module.args.__dict__:
        module.testspec.config_filter.maxflows = module.args.maxflows

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
