



struct k_struct {
    ipsec_tx_stats_cb_index : 12;
    ipsec_int_buf_size      : 14;
}

struct d_struct {
    tx_pkts : 64;
    tx_bytes : 64;
    last_seen : 48;
};

struct k_strcut k;
struct d_struct d;
struct p_struct p;

ipsec_esp_v4_tun_h2n_upd_stats:
    add r1, r0, 1
    tbladd d.tx_pkts, r1
    add r1, r0, k.ipsec_int_buf_size
    tbladd d.tx_bytes, r1
    // r4 has the current timestamp ??
    // else we can take it from intrinsic_timestamp
    tblwr d.last_seen, r4 
    

