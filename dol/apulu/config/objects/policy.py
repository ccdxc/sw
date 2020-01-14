#! /usr/bin/python3

import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.policy as policy
import apollo.config.utils as utils
from apollo.config.store import EzAccessStore

class PolicyObjectHelper:
    def __init__(self):
        return

    def __get_policyID_from_subnet(self, subnet, af, direction):
        if af == 'IPV6':
            return subnet.IngV6SecurityPolicyIds[0] if direction == 'ingress' else subnet.EgV6SecurityPolicyIds[0]
        else:
            return subnet.IngV4SecurityPolicyIds[0] if direction == 'ingress' else subnet.EgV4SecurityPolicyIds[0]

    def __is_lmapping_match(self, policyobj, lobj):
        if lobj.VNIC.SUBNET.VPC.VPCId != policyobj.VPCId:
            return False
        if lobj.AddrFamily == policyobj.AddrFamily:
            return (policyobj.PolicyId == self.__get_policyID_from_subnet(lobj.VNIC.SUBNET, policyobj.AddrFamily, policyobj.Direction))
        return False

    def GetMatchingConfigObjects(self, selectors):
        objs = []
        dutNode = EzAccessStore.GetDUTNode()
        policyobjs = filter(lambda x: x.IsFilterMatch(selectors), policy.client.Objects(dutNode))
        for policyObj in policyobjs:
            for lobj in lmapping.client.Objects(dutNode):
                if self.__is_lmapping_match(policyObj, lobj):
                    policyObj.l_obj = lobj
                    objs.append(policyObj)
                    break
        return utils.GetFilteredObjects(objs, selectors.maxlimits, False)

PolicyHelper = PolicyObjectHelper()

def GetMatchingObjects(selectors):
    return PolicyHelper.GetMatchingConfigObjects(selectors)
