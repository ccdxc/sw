#define DOORBELL_UPD_PID_CHECK              1
#
#define DOORBELL_IDX_CTL_NONE               0
#define DOORBELL_IDX_CTL_UPD_CIDX           1
#define DOORBELL_IDX_CTL_UPD_PIDX           2
#define DOORBELL_IDX_CTL_INC_PIDX           2
#
#define DOORBELL_SCHED_CTL_NONE             0
#define DOORBELL_SCHED_CTL_EVAL             1
#define DOORBELL_SCHED_CTL_CLEAR            2
#define DOORBELL_SCHED_CTL_SET              3

header_type doorbell_addr_t {
    fields {
        offset : 3;
        qtype : 3;
        lif : 11;
        upd_pid_chk : 1;
        upd_index_ctl : 2;
        upd_sched_ctl : 2;
    }
}

#define DOORBELL_ADDR_FILL(_doorbell, _offset, _qtype, _lif, _upd_pid_chk, \
                           _upd_index_ctl, _upd_sched_ctl) \
    modify_field(_doorbell.offset, _offset); \
    modify_field(_doorbell.qtype, _qtype); \
    modify_field(_doorbell.lif, _lif); \
    modify_field(_doorbell.upd_pid_chk, _upd_pid_chk); \
    modify_field(_doorbell.upd_index_ctl, _upd_index_ctl); \
    modify_field(_doorbell.upd_sched_ctl, _upd_sched_ctl);

header_type doorbell_data_t {
    fields {
        pid : 16;
        qid : 24;
        pad : 5;
        ring : 3;
        index : 16;
    }
}

header_type doorbell_data_raw_t {
    fields {
        data : 64;
    }
}

#define DOORBELL_DATA_FILL(_doorbell, _index, _ring, _qid, _pid) \
    modify_field(_doorbell.index, _index); \
    modify_field(_doorbell.ring, _ring); \
    modify_field(_doorbell.pad, 0); \
    modify_field(_doorbell.qid, _qid); \
    modify_field(_doorbell.pid, _pid);
