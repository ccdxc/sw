#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_s4_t0_tcp_rx_k.h"

struct phv_                     p;
struct s4_t0_tcp_rx_k_          k;
struct s4_t0_tcp_rx_tcp_rtt_d   d;

/* 0x64 << 13, i.e. 100 x 10us, 1000us */
r4 = 0xc8000;

p = {
    rx2tx_extra_rto = 0;
    rx2tx_extra_rcv_tsval = 0;
};

k = {
    common_phv_tsopt_enabled = 1;
    common_phv_tsopt_available = 1;
    common_phv_write_arq = 0;
    common_phv_process_ack_flag = 1;
    common_phv_qstate_addr = 0xdeadbeef;
    to_s4_rcv_tsecr = 0x0;
    to_s4_rcv_tsval = 0x0;
    to_s4_snd_nxt = 1000;
};


d = {
    rto = 0;
    backoff = 0;
    curr_ts = 0;
    rtt_seq = 500;
    srtt_us = 0;
    seq_rtt_us = 0;
    ca_rtt_us = 0;
    mdev_us = 0;
    mdev_max_us = 0;
    rttvar_us = 0;
    rtt_min = 0xffffffff;
    ts_shift = 13;
    ts_ganularity_us = 10;
};

params = {
    tcp_rx_fc_stage_start = 0x987654321;
};
