#! /usr/bin/python3

def Setup(infra, module):
    iterelem = module.iterator.Get()

    if iterelem:
        if 'root' in iterelem.__dict__:
            module.testspec.selectors.root = iterelem.root

        if 'tenant' in iterelem.__dict__:
            module.testspec.selectors.tenant.Extend(iterelem.tenant)

        if 'network' in iterelem.__dict__:
            module.testspec.selectors.network.Extend(iterelem.network)

        if 'segment' in iterelem.__dict__:
            module.testspec.selectors.segment.Extend(iterelem.segment)

        if 'endpoint' in iterelem.__dict__:
            module.testspec.selectors.endpoint.Extend(iterelem.endpoint)
            
        if 'enic' in iterelem.__dict__:
            module.testspec.selectors.enic.Extend(iterelem.enic)

        if 'lif' in iterelem.__dict__:
            module.testspec.selectors.lif.Extend(iterelem.lif)

        if 'uplink' in iterelem.__dict__:
            module.testspec.selectors.uplink.Extend(iterelem.uplink)

        if 'uplinkpc' in iterelem.__dict__:
            module.testspec.selectors.uplinkpc.Extend(iterelem.uplinkpc)

        if 'security_profile' in iterelem.__dict__:
            module.testspec.selectors.security_profile.Extend(iterelem.security_profile)

        if 'session' in iterelem.__dict__:
            module.testspec.selectors.session.base.Extend(iterelem.session.base)
            module.testspec.selectors.session.rflow.Extend(iterelem.session.rflow)
            module.testspec.selectors.session.iflow.Extend(iterelem.session.iflow)

    if module.args == None:
        return

    return

def Teardown(infra, module):
    return

def TestCaseSetup(tc):
    return

def TestCaseVerify(tc):
    return True

def TestCaseTeardown(tc):
    return

