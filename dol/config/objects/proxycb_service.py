#! /usr/bin

import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.hal.defs          as haldefs
import config.hal.api           as halapi

from config.store               import Store
from infra.common.logging       import cfglogger
from config.objects.tcp_proxy_cb        import TcpCbHelper

class ProxyCbServiceObject(base.ConfigObjectBase):
    def __init__(self, session):
        super().__init__()
        self.Clone(Store.templates.Get("PROXYCB"))
        self.session = session
        return

    def Show(self):
        cfglogger.info("Service List for %s" % self.GID())
        return

    def PrepareHALRequestSpec(self, req_spec):
        print("Configuring proxy for the flow with label: " + self.session.iflow.label)
        if self.session.iflow.label == 'TCP-PROXY' or self.session.iflow.label == 'TCP-PROXY-E2E' or \
            self.session.iflow.label == 'PROXY-REDIR':
            req_spec.meta.vrf_id = self.session.initiator.ep.tenant.id
            req_spec.spec.key_or_handle.proxy_id = 0
            req_spec.spec.proxy_type = 1
            if req_spec.__class__.__name__ == 'ProxyFlowConfigRequest':
                req_spec.proxy_en = True
                req_spec.alloc_qid = True
            self.session.iflow.PrepareHALRequestSpec(req_spec)
        elif self.session.iflow.label == 'RAW-REDIR' or self.session.iflow.label == 'RAW-REDIR-FLOW-MISS' or \
             self.session.iflow.label == 'RAW-REDIR-KNOWN-APPID':
            req_spec.meta.vrf_id = self.session.initiator.ep.tenant.id
            req_spec.spec.key_or_handle.proxy_id = 4
            req_spec.spec.proxy_type = 7
            if req_spec.__class__.__name__ == 'ProxyFlowConfigRequest':
                req_spec.proxy_en = True
                req_spec.alloc_qid = True
            self.session.iflow.PrepareHALRequestSpec(req_spec)
        elif self.session.iflow.label == 'PROXY-REDIR-E2E':
            req_spec.meta.vrf_id = self.session.initiator.ep.tenant.id
            req_spec.spec.key_or_handle.proxy_id = 5
            req_spec.spec.proxy_type = 9
            if req_spec.__class__.__name__ == 'ProxyFlowConfigRequest':
                req_spec.proxy_en = True
                req_spec.alloc_qid = True
            self.session.iflow.PrepareHALRequestSpec(req_spec)
        elif self.session.iflow.label == 'ESP-PROXY':
            print("Configuring esp ipsec proxy for the flow with label: " + self.session.iflow.label)
            req_spec.meta.vrf_id = self.session.initiator.ep.tenant.id
            req_spec.spec.key_or_handle.proxy_id = 0
            req_spec.spec.proxy_type = 3
            if req_spec.__class__.__name__ == 'ProxyFlowConfigRequest':
                req_spec.proxy_en = True
                req_spec.alloc_qid = True
                req_spec.ipsec_config.encrypt = False
            self.session.iflow.PrepareHALRequestSpec(req_spec)
        elif self.session.iflow.label == 'IPSEC-PROXY':
            print("Configuring host ipsec proxy for the flow with label: " + self.session.iflow.label)
            req_spec.meta.vrf_id = self.session.initiator.ep.tenant.id
            req_spec.spec.key_or_handle.proxy_id = 0
            req_spec.spec.proxy_type = 3
            if req_spec.__class__.__name__ == 'ProxyFlowConfigRequest':
                req_spec.proxy_en = True
                req_spec.alloc_qid = False
                req_spec.ipsec_config.encrypt = True
            self.session.iflow.PrepareHALRequestSpec(req_spec)
 
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if resp_spec.__class__.__name__ == 'ProxyGetFlowInfoResponse':
            cfglogger.info("Received response %s qid1 %d qid2 %d qtype %d lif_id %d" % (haldefs.common.ApiStatus.Name(resp_spec.api_status), resp_spec.qid1, resp_spec.qid2, resp_spec.qtype, resp_spec.lif_id))
            self.qid = resp_spec.qid1
            self.other_qid = resp_spec.qid2
        else:
            cfglogger.info("Received response %s" % (haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

class ProxyCbServiceObjectHelper:
    def __init__(self):
        self.proxy_service_list = []
        return

    def Configure(self):
        for proxycb in self.proxy_service_list:
            if proxycb.session.iflow.label == 'TCP-PROXY' or proxycb.session.iflow.label == 'ESP-PROXY' or proxycb.session.iflow.label == 'IPSEC-PROXY' or \
                proxycb.session.iflow.label == 'RAW-REDIR' or proxycb.session.iflow.label == 'RAW-REDIR-FLOW-MISS' or \
                proxycb.session.iflow.label == 'RAW-REDIR-KNOWN-APPID' or \
                proxycb.session.iflow.label == 'PROXY-REDIR' or proxycb.session.iflow.label == 'PROXY-REDIR-E2E' or \
                proxycb.session.iflow.label == 'TCP-PROXY-E2E':
                lst = []
                lst.append(proxycb)
                halapi.ConfigureProxyCbService(lst)
                halapi.GetQidProxycbGetFlowInfo(lst)
                #TcpCbHelper.main(proxycb.qid)
        return

    def GetSessionQids(self, session):
        lst = []
        proxyFlowObj = ProxyCbServiceObject(session)
        lst.append(proxyFlowObj)
        halapi.GetQidProxycbGetFlowInfo(lst)
        return proxyFlowObj.qid, proxyFlowObj.other_qid

    def GetFlowInfo(self, session):
        lst = []
        proxyFlowObj = ProxyCbServiceObject(session)
        lst.append(proxyFlowObj)
        halapi.GetQidProxycbGetFlowInfo(lst)
        return proxyFlowObj.qid 

    def GetOtherFlowInfo(self, session):
        lst = []
        proxyFlowObj = ProxyCbServiceObject(session)
        lst.append(proxyFlowObj)
        halapi.GetQidProxycbGetFlowInfo(lst)
        return proxyFlowObj.other_qid 
   
    def main(self, sessions):
        for session in sessions:
            proxyFlowObj = ProxyCbServiceObject(session)
            self.proxy_service_list.append(proxyFlowObj)
        self.Configure()

ProxyCbServiceHelper = ProxyCbServiceObjectHelper()

