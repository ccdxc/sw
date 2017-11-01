#! /usr/bin/python3
# Test Module

import pdb
import infra.api.api as InfraApi
import test.callbacks.networking.modcbs as modcbs
from config.store import Store

acls = False

def Setup(infra, module):
    global acls

    modcbs.Setup(infra, module)

    if not acls:
        Store.objects.Get('ACL_IPV4_IGMP_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_MLD_REQ_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_MLD_RES_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_MLD_DONE_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_RS_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_RA_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_NS_ACTION_SUP_REDIRECT').Configure()
        Store.objects.Get('ACL_ICMPV6_NA_ACTION_SUP_REDIRECT').Configure()
        acls = True

    return

def TestCaseSetup(tc):
    iterelem = tc.module.iterator.Get()

    tc.pvtdata.priotag = False
    if 'priotag' in iterelem.__dict__:
        tc.pvtdata.priotag = iterelem.priotag

    return

