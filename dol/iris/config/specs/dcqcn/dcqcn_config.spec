# RDMA DCQCN Config Spec
meta:
    id: DCQCN_CONFIG_RDMA

default_profile:
    id                              : 0
    np_incp_802p_prio               : 6
    np_cnp_dscp                     : 48
    rp_token_bucket_size            : 150000
    rp_initial_alpha_value          : 1023
    rp_dce_tcp_g                    : 65535
    rp_dce_tcp_rtt                  : 55
    rp_rate_reduce_monitor_period   : 4
    rp_rate_to_set_on_first_cnp     : 0
    rp_min_rate                     : 1
    rp_gd                           : 11
    rp_min_dec_fac                  : 50
    rp_clamp_flags                  : 2
    rp_threshold                    : 5
    rp_time_reset                   : 5
    rp_qp_rate                      : 1
    rp_byte_reset                   : 31
    rp_ai_rate                      : 5
    rp_hai_rate                     : 15
