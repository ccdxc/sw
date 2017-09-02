#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.parser      as parser
import config.hal.api           as halapi

from config.objects.uplink              import UplinkHelper
from config.objects.uplinkpc            import UplinkPcHelper
from config.objects.tenant              import TenantHelper
from config.objects.session             import SessionHelper
from config.objects.security_profile    import SecurityProfileHelper
from config.objects.tcp_proxy_cb        import TcpCbHelper
from config.objects.acl                 import AclHelper
from config.objects.proxy_service       import ProxyServiceHelper
from config.objects.ipsec_proxy_cb      import IpsecCbHelper

from infra.common.logging import cfglogger as cfglogger
from config.objects.swdr                import SwDscrRingHelper

def process(topospec):
    # Security Profiles
    SecurityProfileHelper.main()
    # Uplinks
    UplinkHelper.main(topospec)
    # UplinkPC
    UplinkPcHelper.main(topospec)
    # Generate and Configure Tenants
    TenantHelper.main(topospec)
    # Generate ACLs
    AclHelper.main(topospec)

    # Enable all segments on Uplinks and UplinkPcs
    UplinkHelper.ConfigureAllSegments()
    UplinkPcHelper.ConfigureAllSegments()
    # Phase2 Tenant config - EPs and ENICs
    TenantHelper.ConfigurePhase2()

    # Generate all sessions
    SessionHelper.main()
    
    ProxyServiceHelper.main()

    # Global descriptors and page rings
    SwDscrRingHelper.main("NMDR")
    SwDscrRingHelper.main("NMPR")
    SwDscrRingHelper.main("BRQ")

    TcpCbHelper.main()    
    IpsecCbHelper.main()    
    return

def main(topofile):
    halapi.init()
    cfglogger.info("Generating Config Objects for Topology = %s" % topofile)
    topospec = parser.ParseFile('config/topology/', topofile)
    process(topospec)
    return

