#! /usr/bin/python3
import pdb
import copy
import itertools
import infra.config.base           as base
import config.hal.api              as halapi
import config.hal.defs             as haldefs
import config.objects.rules        as rules
import config.hal.defs             as haldefs

from    config.store               import Store
from    infra.common.logging       import cfglogger
from    config.objects.security_group import SecurityGroupObject

class SGPairObject(base.ConfigObjectBase):
    def __init__(self, sg_id, peer_sg_id, obj):
        super().__init__()
        self.sg_id = sg_id
        self.peer_sg_id = peer_sg_id
        self.GID("SGPAIRSG%04dSG%04d" % (sg_id,peer_sg_id))
        self.obj = obj
        return

    def PrepareHALRequestSpec(self, req_spec):
        pl_id = haldefs.nwsec.SecurityGroupPolicyId()
        pl_id.security_group_id = self.sg_id
        pl_id.peer_security_group_id = self.peer_sg_id
        req_spec.key_or_handle.security_group_policy_id.CopyFrom(pl_id)
        for rule_obj in self.obj.in_rules:
            rule_req_spec = req_spec.policy_rules.in_fw_rules.add()
            rule_obj.PrepareHALRequestSpec(rule_req_spec)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.status.policy_handle
        cfglogger.info(" - SecurityPolicy %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        pl_id = haldefs.nwsec.SecurityGroupPolicyId()
        pl_id.security_group_id = self.sg_id
        pl_id.peer_security_group_id = self.peer_sg_id

        get_req_spec.key_or_handle.security_group_policy_id = pl_id
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        return

    def Get(self):
        halapi.GetSecurityGroupPolicies([self])

class SGPairObjectHelper:
    def __init__(self):
        self.sgpair_objlist = []

    def __get_sg_pair(self):
        sglist = Store.objects.GetAllByClass(SecurityGroupObject)
        for sg1,sg2 in itertools.combinations_with_replacement(sglist, 2):
                yield (sg1, sg2)

    def Generate(self, sp_obj):
        sp_len = len(sp_obj.sps)
        index = 0
        for sgpair in self.__get_sg_pair():
            if (sgpair[0].id != sgpair[1].id):
                sgpair_obj = SGPairObject(sgpair[0].id, sgpair[1].id,
                                          sp_obj.sps[index])
                sgpair_obj_rev = SGPairObject(sgpair[1].id, sgpair[0].id,
                                              sp_obj.sps[index])

                self.sgpair_objlist.extend([sgpair_obj, sgpair_obj_rev])
                index = index + 1
            else:
                sgpair_obj = SGPairObject(sgpair[0].id, sgpair[1].id,
                                          sp_obj.default_sps[0])
                self.sgpair_objlist.extend([sgpair_obj])

            if (index == sp_len): index = 0
        Store.objects.SetAll(self.sgpair_objlist)

SGPairHelper = SGPairObjectHelper()

class SecurityGroupPolicyObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SECURITY_POLICY'))
        self.in_rules=[]
        self.eg_rules=[]
        return

    def Init(self, spec):
        if spec.in_rules:
            for rule_spec in spec.in_rules:
                rule_obj = rules.RuleObject()
                rule_obj.Init(rule_spec.rule)
                self.in_rules.append(rule_obj)
        self.GID(spec.id)
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Security Group Objects
class SecurityGroupPolicyObjectHelper:
    def __init__(self):
        self.sps = []
        self.default_sps = []
        return

    def Configure(self):
        sgpairlist  = Store.objects.GetAllByClass(SGPairObject)
        cfglogger.info("Confguring %d SecurityGroupPolicies." %len(sgpairlist))

        halapi.ConfigureSecurityGroupPolicies(sgpairlist)
        return

    def GetSGType(self, flow_obj, sep, dep):
        src_sg_list = sep.sgs
        dst_sg_list = dep.sgs
        for src_sg, dst_sg in itertools.product(src_sg_list, dst_sg_list):
            if src_sg == dst_sg: return 'INTRA'
        return 'INTER'

    def __eval_policy(self, sec_policy, flow_obj):
        for rule_obj in sec_policy.in_rules:
            for svc_obj in rule_obj.svc_objs:
                if (svc_obj.proto == 0): return rule_obj.action
                if (svc_obj.proto == flow_obj.proto):
                    if (flow_obj.icmpcode and
                        flow_obj.icmpcode == svc_obj.icmp_msg_type or
                        (flow_obj.dport == svc_obj.dst_port)):
                            return rule_obj.action
        return None

    def GetAction(self, flow_obj, sep, dep):
        src_sg_list  = sep.sgs
        dst_sg_list  = dep.sgs
        for src_sg, dst_sg in itertools.product(src_sg_list, dst_sg_list):
            if src_sg == dst_sg: continue
            try:
                sgpair_obj = Store.objects.Get("SGPAIR" + str(src_sg) + str(dst_sg))
            except KeyError:
                continue

            if (sgpair_obj):
                #Get Security Policy
                sp = sgpair_obj.obj
                return self.__eval_policy(sp, flow_obj)
        return None

    def Generate(self, topospec):
        sps = getattr(topospec, 'security_policy', None)
        if sps is None:
            return
        spec = topospec.security_policy.Get(Store)
        cfglogger.info("Creating %d SecurityPolicies." % len(spec.policies))
        for p in spec.policies:
            policy = SecurityGroupPolicyObject()
            policy.Init(p.policy)
            if p.policy.default == True:
                self.default_sps.append(policy)
            else:
                self.sps.append(policy)
        Store.objects.SetAll(self.sps)
        #Update the SGPair Object
        SGPairHelper.Generate(self)

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        return

SecurityGroupPolicyHelper = SecurityGroupPolicyObjectHelper()
