#! /usr/bin/python3
import pdb
import os
import json
from collections import defaultdict

import infra.common.defs            as defs
import infra.common.utils           as utils
import infra.common.parser          as parser
import infra.common.timeprofiler    as timeprofiler

import iris.config.hal.api               as halapi
import iris.config.resmgr                as resmgr

from infra.common.glopts                     import GlobalOptions
from iris.config.objects.uplink              import UplinkHelper
from iris.config.objects.nic                 import NicHelper
from iris.config.objects.uplinkpc            import UplinkPcHelper
from iris.config.objects.tenant              import TenantHelper
from iris.config.objects.session             import SessionHelper
from iris.config.objects.rdma.session        import RdmaSessionHelper
from iris.config.objects.nvme.gbl            import NvmeGlobalObject
from iris.config.objects.nvme.session        import NvmeSessionHelper
from iris.config.objects.security_profile    import SecurityProfileHelper
from iris.config.objects.acl                 import AclHelper
from iris.config.objects.qos_class           import QosClassHelper
from iris.config.objects.proxy_service       import ProxyServiceHelper
from iris.config.objects.ipsec_proxy_cb      import IpsecCbHelper
from iris.config.objects.cpu                 import CpuHelper
from iris.config.objects.app_redir_if        import AppRedirIfHelper
from iris.config.objects.system              import SystemHelper

if GlobalOptions.gft:
    from iris.config.objects.gft_hdr_group                   import GftHeaderGroupHelper
    from iris.config.objects.gft_exm_profile                 import GftExmProfileHelper
    from iris.config.objects.gft_flow                        import GftFlowHelper
    from iris.config.objects.gft_transposition_profile       import GftTranspositionProfileHelper
    from iris.config.objects.gft_transposition_hdr_group     import GftTranspositionHeaderGroupHelper

from infra.config.parser                    import ConfigParser as ConfigParser
from iris.config.objects.swdr               import SwDscrRingHelper
from iris.config.objects.brq                import BRQHelper
from iris.config.objects.timer              import TimerHelper
from iris.config.objects.security_policy    import SecurityGroupPolicyHelper
from infra.common.logging                   import logger as logger
from infra.asic.model                       import ModelConnector
from iris.config.store                      import Store

def process(topospec):
    # Device Mode
    NicHelper.main()

    # Security Profiles
    SecurityProfileHelper.main(topospec)
    QosClassHelper.main(topospec)

    #initialize nvme proxy related global resources before
    #any LIFs are configured
    nvme_proxy = getattr(topospec, 'nvme_proxy', False)
    if nvme_proxy:
        nvme_gbl = NvmeGlobalObject()
        nvme_gbl.main(topospec.nvme_proxy)
        Store.objects.SetAll([nvme_gbl])

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
        SwDscrRingHelper.main("NMDPR")
        SwDscrRingHelper.main("IPSEC_NMDR")
        SwDscrRingHelper.main("IPSEC_NMPR")
        BRQHelper.main()
        for i in range(3):
            SwDscrRingHelper.main("ARQ", ('CPU%04d' % i), i)
            SwDscrRingHelper.main("ARQ-TX", ('CPU%04d' % i), i)
            SwDscrRingHelper.main("TCP_ACTL", ('CPU%04d' % i), i)

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

    # Generate all Nvme sessions
    if nvme_proxy:
        NvmeSessionHelper.main()

    ipsec = getattr(topospec, 'ipsec', False)
    #if ipsec:
    #    IpsecCbHelper.main()

    CpuHelper.main(topospec)
    # Generate ACLs
    AclHelper.main(topospec)
    TimerHelper.main(topospec)

    if GlobalOptions.gft:
        GftHeaderGroupHelper.main(topospec)
        GftExmProfileHelper.main(topospec)
        GftTranspositionHeaderGroupHelper.main(topospec)
        GftTranspositionProfileHelper.main(topospec)
        GftFlowHelper.main()

    return

def main(topofile):
    timeprofiler.ConfigTimeProfiler.Start()
    halapi.init()
    if not GlobalOptions.naples_container:
        logger.info("Initializing Resmgr")
        resmgr.init()
    logger.info("Generating Config Objects for Topology = %s" % topofile)
    topospec = parser.ParseFile('iris/config/topology/', topofile)
    if topospec:
        process(topospec)
    timeprofiler.ConfigTimeProfiler.Stop()
    if not GlobalOptions.naples_container:
        ModelConnector.ConfigDone()
    return

def SaveStoreToJson(conf_file):
    config_dict = defaultdict(lambda:{})
    for cfg_object in Store.objects.GetAll():
        config_dict[cfg_object.__class__.__name__][cfg_object.GID()] = cfg_object.ToJson()
    with open(conf_file, 'w') as fp:
        json.dump(config_dict, fp, indent=4)
    logger.info("Dumped configuration to file %s" % conf_file)

