#! /usr/bin/python3
import pdb

import infra.common.defs            as defs
import infra.common.parser          as parser
import infra.common.timeprofiler    as timeprofiler
import config.hal.api               as halapi
import config.resmgr                as resmgr

from config.objects.uplink              import UplinkHelper
from config.objects.uplinkpc            import UplinkPcHelper
from config.objects.tenant              import TenantHelper
from config.objects.session             import SessionHelper
from config.objects.rdma.session        import RdmaSessionHelper
from config.objects.security_profile    import SecurityProfileHelper
from config.objects.acl                 import AclHelper
from config.objects.qos_class           import QosClassHelper
from config.objects.proxy_service       import ProxyServiceHelper
from config.objects.ipsec_proxy_cb      import IpsecCbHelper
from config.objects.cpu                 import CpuHelper

from config.objects.swdr                import SwDscrRingHelper
from config.objects.brq                 import BRQHelper
from config.objects.timer               import TimerHelper
from config.objects.security_policy     import SecurityGroupPolicyHelper
from infra.common.logging               import cfglogger as cfglogger

def process(topospec):
    QosClassHelper.main(topospec)
    # Security Profiles
    SecurityProfileHelper.main(topospec)

    # Uplinks
    UplinkHelper.main(topospec)
    # UplinkPC
    UplinkPcHelper.main(topospec)
    # Generate and Configure Tenants
    TenantHelper.main(topospec)

    # Security Groups
    SecurityGroupPolicyHelper.main(topospec)

    # Enable all segments on Uplinks and UplinkPcs
    UplinkHelper.ConfigureAllSegments()
    UplinkPcHelper.ConfigureAllSegments()
    # Phase2 Tenant config - EPs and ENICs
    TenantHelper.ConfigurePhase2()

    proxy = getattr(topospec, 'proxy', False)
    if proxy == True:
        # Global descriptors and page rings
        ProxyServiceHelper.main()
        SwDscrRingHelper.main("NMDR")
        SwDscrRingHelper.main("NMPR")
        BRQHelper.main()
        for i in range(3):
            SwDscrRingHelper.main("ARQ", ('CPU%04d' % i), i)
            SwDscrRingHelper.main("ARQ-TX", ('CPU%04d' % i), i)
        SwDscrRingHelper.main("RAWCCBQ")
        SwDscrRingHelper.main("PROXYRCBQ")
        SwDscrRingHelper.main("PROXYCCBQ")

    # Generate all sessions
    SessionHelper.main()

    # Generate all sessions
    RdmaSessionHelper.main()

    ipsec = getattr(topospec, 'ipsec', False)
    if ipsec:
        IpsecCbHelper.main()

    CpuHelper.main(topospec)
    # Generate ACLs
    AclHelper.main(topospec)
    TimerHelper.main(topospec)
    return

def main(topofile):
    timeprofiler.ConfigTimeProfiler.Start()
    halapi.init()
    cfglogger.info("Initializing Resmgr")
    resmgr.init()
    cfglogger.info("Generating Config Objects for Topology = %s" % topofile)
    topospec = parser.ParseFile('config/topology/', topofile)
    process(topospec)
    timeprofiler.ConfigTimeProfiler.Stop()
    return

