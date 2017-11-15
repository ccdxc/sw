#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct tcp_stateless_normalization_k k;
struct phv_ p;

k = {
    l4_metadata_tcp_data_len = 0x100;
    l4_metadata_tcp_unexpected_mss_action_sbit1_ebit1 = 0x1;
    l4_metadata_tcp_unexpected_win_scale_action = 0x1;
    l4_metadata_tcp_urg_ptr_not_set_action = 0x1;
    l4_metadata_tcp_urg_payload_missing_action = 0x1;
    l4_metadata_tcp_rst_with_data_action = 0x1;
    l4_metadata_tcp_invalid_flags_drop = 0x0;
    l4_metadata_tcp_flags_nonsyn_noack_drop = 0x0;
    l4_metadata_tcp_unexpected_echo_ts_action = 0x1;
    tcp_flags = 0x18;
    tcp_res = 0x0;
    tcp_urgentPtr = 0x0;
    tcp_option_mss_valid = 0;
    tcp_option_ws_valid = 0;
    tcp_option_timestamp_valid = 1;
    tcp_option_timestamp_prev_echo_ts = 0x12345678;
};

p = {
    tcp_flags = 0x4;
};
