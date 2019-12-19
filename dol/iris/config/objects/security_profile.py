#! /usr/bin/python3
import pdb
import copy

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.common.utils       as utils
import iris.config.resmgr            as resmgr
import infra.config.base        as base

from iris.config.store               import Store
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

import iris.config.hal.api            as halapi
import iris.config.hal.defs           as haldefs
import iris.config.agent.api          as agentapi

class SecurityProfileObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SECURITY_PROFILE'))
        self.id = resmgr.SecProfIdAllocator.get()
        return

    def Init(self, spec):
        if spec.fields:
            self.fields.Clone(spec.fields)
        self.GID(spec.id)
        if GlobalOptions.hostpin:
            self.fields.ipsg_en = False
            logger.info("%s Disabling IPSG for Hostpin mode." % self.GID())
        self.Show()
        return

    def CloneFields(self, srcobj):
        self.fields.Clone(srcobj.fields)
        return

    def Update(self):
        logger.info("Updating SecurityProfile %s" % self.GID())
        halapi.ConfigureSecurityProfiles([ self ], update = True)
        return

    def __copy__(self):
        sc_profile = SecurityProfileObject()
        sc_profile.id = self.id
        sc_profile.fields = copy.copy(self.fields)
        return sc_profile

    def Equals(self, other, lgh):
        if not isinstance(other, self.__class__):
            return False
        fields = ["id"]
        if not self.CompareObjectFields(other, fields, lgh):
            return False
        fields = self.fields.__dict__
        if not utils.CompareObjectFields(self.fields, other.fields, fields, lgh):
            return False
        return True


    def Show(self):
        logger.info("- Security Profile  : %s (id: %d)" %\
                       (self.GID(), self.id))
        return

    def __getEnumValue(self, val):
        valstr = "NORM_ACTION_" + val
        return haldefs.nwsec.NormalizationAction.Value(valstr)

    def __getStringValue(self, val):
        return haldefs.nwsec.NormalizationAction.Name(val).split("_")[-1]

    def PrepareHALRequestSpec(self, req_spec):
        logger.info("Populating sec prof id: %d" % self.id)
        req_spec.key_or_handle.profile_id = self.id
        req_spec.cnxn_tracking_en = self.fields.cnxn_tracking_en
        req_spec.ipsg_en = self.fields.ipsg_en
        req_spec.tcp_rtt_estimate_en = self.fields.tcp_rtt_estimate_en
        if ('session_idle_timeout' in dir(self.fields)):
            req_spec.session_idle_timeout = self.fields.session_idle_timeout
        else:
            req_spec.session_idle_timeout = 65535
        req_spec.tcp_cnxn_setup_timeout = self.fields.tcp_cnxn_setup_timeout
        req_spec.tcp_close_timeout = self.fields.tcp_close_timeout
        req_spec.tcp_half_closed_timeout = self.fields.tcp_half_closed_timeout
        req_spec.ip_normalization_en = self.fields.ip_normalization_en
        req_spec.tcp_normalization_en = self.fields.tcp_normalization_en
        req_spec.icmp_normalization_en = self.fields.icmp_normalization_en
        req_spec.ip_ttl_change_detect_en = self.fields.ip_ttl_change_detect_en
        req_spec.ip_rsvd_flags_action = self.__getEnumValue(self.fields.ip_rsvd_flags_action)
        req_spec.ip_df_action = self.__getEnumValue(self.fields.ip_df_action)
        req_spec.ip_options_action = self.__getEnumValue(self.fields.ip_options_action)
        req_spec.ip_invalid_len_action = self.__getEnumValue(self.fields.ip_invalid_len_action)
        req_spec.ip_normalize_ttl = self.fields.ip_normalize_ttl
        req_spec.icmp_invalid_code_action = self.__getEnumValue(self.fields.icmp_invalid_code_action)
        req_spec.icmp_deprecated_msgs_drop = self.fields.icmp_deprecated_msgs_drop
        req_spec.icmp_redirect_msg_drop = self.fields.icmp_redirect_msg_drop
        req_spec.tcp_non_syn_first_pkt_drop = self.fields.tcp_non_syn_first_pkt_drop
        req_spec.tcp_split_handshake_drop = self.fields.tcp_split_handshake_drop
        req_spec.tcp_rsvd_flags_action = self.__getEnumValue(self.fields.tcp_rsvd_flags_action)
        req_spec.tcp_unexpected_mss_action = self.__getEnumValue(self.fields.tcp_unexpected_mss_action)
        req_spec.tcp_unexpected_win_scale_action = self.__getEnumValue(self.fields.tcp_unexpected_win_scale_action)
        req_spec.tcp_unexpected_sack_perm_action = self.__getEnumValue(self.fields.tcp_unexpected_sack_perm_action)
        req_spec.tcp_urg_ptr_not_set_action = self.__getEnumValue(self.fields.tcp_urg_ptr_not_set_action)
        req_spec.tcp_urg_flag_not_set_action = self.__getEnumValue(self.fields.tcp_urg_flag_not_set_action)
        req_spec.tcp_urg_payload_missing_action = self.__getEnumValue(self.fields.tcp_urg_payload_missing_action)
        req_spec.tcp_rst_with_data_action = self.__getEnumValue(self.fields.tcp_rst_with_data_action)
        req_spec.tcp_data_len_gt_mss_action = self.__getEnumValue(self.fields.tcp_data_len_gt_mss_action)
        req_spec.tcp_data_len_gt_win_size_action = self.__getEnumValue(self.fields.tcp_data_len_gt_win_size_action)
        req_spec.tcp_unexpected_ts_option_action = self.__getEnumValue(self.fields.tcp_unexpected_ts_option_action)
        req_spec.tcp_unexpected_sack_option_action = self.__getEnumValue(self.fields.tcp_unexpected_sack_option_action)
        req_spec.tcp_unexpected_echo_ts_action = self.__getEnumValue(self.fields.tcp_unexpected_echo_ts_action)
        req_spec.tcp_ts_not_present_drop = self.fields.tcp_ts_not_present_drop
        req_spec.tcp_invalid_flags_drop = self.fields.tcp_invalid_flags_drop
        req_spec.tcp_nonsyn_noack_drop = self.fields.tcp_nonsyn_noack_drop
        req_spec.tcp_normalize_mss = self.fields.tcp_normalize_mss
        req_spec.flow_learn_en = self.fields.flow_learn_en
        req_spec.policy_enforce_en = self.fields.policy_enforce_en
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.profile_status.profile_handle
        logger.info("  - SecurityProfile %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        get_req_spec.key_or_handle.profile_id = self.id
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        if get_resp.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.id = get_resp.spec.key_or_handle.profile_id
            self.fields.cnxn_tracking_en = get_resp.spec.cnxn_tracking_en
            self.fields.ipsg_en = get_resp.spec.ipsg_en
            self.fields.tcp_rtt_estimate_en = get_resp.spec.tcp_rtt_estimate_en
            self.fields.session_idle_timeout = get_resp.spec.session_idle_timeout
            self.fields.tcp_cnxn_setup_timeout = get_resp.spec.tcp_cnxn_setup_timeout
            self.fields.tcp_close_timeout = get_resp.spec.tcp_close_timeout
            self.fields.tcp_half_closed_timeout = get_resp.spec.tcp_half_closed_timeout
            self.fields.ip_normalization_en = get_resp.spec.ip_normalization_en
            self.fields.tcp_normalization_en = get_resp.spec.tcp_normalization_en
            self.fields.icmp_normalization_en = get_resp.spec.icmp_normalization_en
            self.fields.ip_ttl_change_detect_en = get_resp.spec.ip_ttl_change_detect_en
            self.fields.ip_rsvd_flags_action = self.__getStringValue(get_resp.spec.ip_rsvd_flags_action)
            self.fields.ip_df_action = self.__getStringValue(get_resp.spec.ip_df_action)
            self.fields.ip_options_action = self.__getStringValue(get_resp.spec.ip_options_action)
            self.fields.ip_invalid_len_action = self.__getStringValue(get_resp.spec.ip_invalid_len_action)
            self.fields.ip_normalize_ttl = get_resp.spec.ip_normalize_ttl
            self.fields.icmp_invalid_code_action = self.__getStringValue(get_resp.spec.icmp_invalid_code_action)
            self.fields.icmp_deprecated_msgs_drop = get_resp.spec.icmp_deprecated_msgs_drop
            self.fields.icmp_redirect_msg_drop = get_resp.spec.icmp_redirect_msg_drop
            self.fields.tcp_non_syn_first_pkt_drop = get_resp.spec.tcp_non_syn_first_pkt_drop
            self.fields.tcp_split_handshake_drop = get_resp.spec.tcp_split_handshake_drop
            self.fields.tcp_rsvd_flags_action = self.__getStringValue(get_resp.spec.tcp_rsvd_flags_action)
            self.fields.tcp_unexpected_mss_action = self.__getStringValue(get_resp.spec.tcp_unexpected_mss_action)
            self.fields.tcp_unexpected_win_scale_action = self.__getStringValue(get_resp.spec.tcp_unexpected_win_scale_action)
            self.fields.tcp_urg_ptr_not_set_action = self.__getStringValue(get_resp.spec.tcp_urg_ptr_not_set_action)
            self.fields.tcp_urg_flag_not_set_action = self.__getStringValue(get_resp.spec.tcp_urg_flag_not_set_action)
            self.fields.tcp_urg_payload_missing_action = self.__getStringValue(get_resp.spec.tcp_urg_payload_missing_action)
            self.fields.tcp_rst_with_data_action = self.__getStringValue(get_resp.spec.tcp_rst_with_data_action)
            self.fields.tcp_data_len_gt_mss_action = self.__getStringValue(get_resp.spec.tcp_data_len_gt_mss_action)
            self.fields.tcp_data_len_gt_win_size_action = self.__getStringValue(get_resp.spec.tcp_data_len_gt_win_size_action)
            self.fields.tcp_unexpected_ts_option_action = self.__getStringValue(get_resp.spec.tcp_unexpected_ts_option_action)
            self.fields.tcp_unexpected_echo_ts_action = self.__getStringValue(get_resp.spec.tcp_unexpected_echo_ts_action)
            self.fields.tcp_ts_not_present_drop = get_resp.spec.tcp_ts_not_present_drop
            self.fields.tcp_invalid_flags_drop = get_resp.spec.tcp_invalid_flags_drop
            self.fields.tcp_nonsyn_noack_drop = get_resp.spec.tcp_nonsyn_noack_drop
            self.fields.tcp_normalize_mss = get_resp.spec.tcp_normalize_mss
            self.fields.flow_learn_en = get_resp.spec.flow_learn_en
            self.fields.policy_enforce_en = get_resp.spec.policy_enforce_en

        return

    def Get(self):
        halapi.GetSecurityProfiles([self])

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Security Profile Objects
class SecurityProfileObjectHelper:
    def __init__(self):
        self.sps = []
        return

    def Configure(self):
        logger.info("Configuring %d SecurityProfiles." % len(self.sps))
        if GlobalOptions.agent:
            agentapi.ConfigureSecurityProfiles(self.sps)
        else:
            halapi.ConfigureSecurityProfiles(self.sps)
        return

    def Generate(self, topospec):
        sps = getattr(topospec, 'security_profiles', None)
        if sps is None:
            return
        spec = topospec.security_profiles.Get(Store)
        logger.info("Creating %d SecurityProfiles." % len(spec.profiles))
        for p in spec.profiles:
            profile = SecurityProfileObject()
            profile.Init(p)
            self.sps.append(profile)
        Store.objects.SetAll(self.sps)
        return

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        return

SecurityProfileHelper = SecurityProfileObjectHelper()

def GetMatchingObjects(selectors):
    sps =  Store.objects.GetAllByClass(SecurityProfileObject)
    return [sp for sp in sps if sp.IsFilterMatch(selectors.security_profile)]
