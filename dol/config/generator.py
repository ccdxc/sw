#! /usr/bin/python3
import pdb
import os
import json
from collections import defaultdict 

import infra.common.defs            as defs
import infra.common.utils           as utils
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
from config.objects.app_redir_if        import AppRedirIfHelper
from config.objects.system              import SystemHelper
from config.objects.gft_hdr_group       import GftHeaderGroupHelper
from config.objects.gft_exm_profile     import GftExmProfileHelper
from config.objects.gft_flow            import GftFlowHelper
from config.objects.gft_transposition_profile import GftTranspositionProfileHelper
from config.objects.gft_transposition_hdr_group       import GftTranspositionHeaderGroupHelper

from config.objects.swdr                import SwDscrRingHelper
from config.objects.brq                 import BRQHelper
from config.objects.timer               import TimerHelper
from config.objects.security_policy     import SecurityGroupPolicyHelper
from infra.common.logging               import cfglogger as cfglogger
from infra.asic.model                   import ModelConnector
from config.store                       import Store

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

    app_redir = getattr(topospec, 'app_redir', False)
    proxy = getattr(topospec, 'proxy', False)
    if proxy == True or app_redir == True:
        # Global descriptors and page rings
        ProxyServiceHelper.main()
        SwDscrRingHelper.main("NMDR")
        SwDscrRingHelper.main("NMPR")
        BRQHelper.main()
        for i in range(3):
            SwDscrRingHelper.main("ARQ", ('CPU%04d' % i), i)
            SwDscrRingHelper.main("ARQ-TX", ('CPU%04d' % i), i)

    if app_redir == True:
        SwDscrRingHelper.main("RAWCCBQ")
        SwDscrRingHelper.main("PROXYRCBQ")
        SwDscrRingHelper.main("PROXYCCBQ")
        AppRedirIfHelper.main(topospec)

    # Generate all sessions
    SessionHelper.main()

    # setup the system drop actions
    SystemHelper.main(topospec)

    # Generate all sessions
    RdmaSessionHelper.main()

    ipsec = getattr(topospec, 'ipsec', False)
    if ipsec:
        IpsecCbHelper.main()

    CpuHelper.main(topospec)
    # Generate ACLs
    AclHelper.main(topospec)
    TimerHelper.main(topospec)

    GftHeaderGroupHelper.main(topospec)
    GftExmProfileHelper.main(topospec)
    GftTranspositionHeaderGroupHelper.main(topospec)
    GftTranspositionProfileHelper.main(topospec)
    GftFlowHelper.main()
    return

def main(topofile):
    timeprofiler.ConfigTimeProfiler.Start()
    halapi.init()
    cfglogger.info("Initializing Resmgr")
    resmgr.init()
    cfglogger.info("Generating Config Objects for Topology = %s" % topofile)
    topospec = parser.ParseFile('config/topology/', topofile)
    if topospec:
        process(topospec)
    timeprofiler.ConfigTimeProfiler.Stop()
    ModelConnector.ConfigDone()
    return

def dump_configuration(conf_file):
    config_dict = defaultdict(lambda:{})
    for cfg_object in Store.objects.GetAll():
        config_dict[cfg_object.__class__.__name__][cfg_object.GID()] = cfg_object.ToJson()
    with open(conf_file, 'w') as fp:
        json.dump(config_dict, fp, indent=4)
    cfglogger.info("Dumped configuration to file %s" % conf_file)
    
