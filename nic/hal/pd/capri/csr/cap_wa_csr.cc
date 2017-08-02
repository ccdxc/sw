
#include "cap_wa_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_wa_csr_dhs_lif_qstate_map_entry_t::cap_wa_csr_dhs_lif_qstate_map_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_lif_qstate_map_entry_t::~cap_wa_csr_dhs_lif_qstate_map_entry_t() { }

cap_wa_csr_dhs_lif_qstate_map_t::cap_wa_csr_dhs_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_lif_qstate_map_t::~cap_wa_csr_dhs_lif_qstate_map_t() { }

cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::cap_wa_csr_dhs_doorbell_err_activity_log_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::~cap_wa_csr_dhs_doorbell_err_activity_log_entry_t() { }

cap_wa_csr_dhs_doorbell_err_activity_log_t::cap_wa_csr_dhs_doorbell_err_activity_log_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_doorbell_err_activity_log_t::~cap_wa_csr_dhs_doorbell_err_activity_log_t() { }

cap_wa_csr_dhs_page4k_remap_tbl_entry_t::cap_wa_csr_dhs_page4k_remap_tbl_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_page4k_remap_tbl_entry_t::~cap_wa_csr_dhs_page4k_remap_tbl_entry_t() { }

cap_wa_csr_dhs_page4k_remap_tbl_t::cap_wa_csr_dhs_page4k_remap_tbl_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_page4k_remap_tbl_t::~cap_wa_csr_dhs_page4k_remap_tbl_t() { }

cap_wa_csr_dhs_page4k_remap_db_entry_t::cap_wa_csr_dhs_page4k_remap_db_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_page4k_remap_db_entry_t::~cap_wa_csr_dhs_page4k_remap_db_entry_t() { }

cap_wa_csr_dhs_page4k_remap_db_t::cap_wa_csr_dhs_page4k_remap_db_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_page4k_remap_db_t::~cap_wa_csr_dhs_page4k_remap_db_t() { }

cap_wa_csr_dhs_32b_doorbell_entry_t::cap_wa_csr_dhs_32b_doorbell_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_32b_doorbell_entry_t::~cap_wa_csr_dhs_32b_doorbell_entry_t() { }

cap_wa_csr_dhs_32b_doorbell_t::cap_wa_csr_dhs_32b_doorbell_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_32b_doorbell_t::~cap_wa_csr_dhs_32b_doorbell_t() { }

cap_wa_csr_dhs_local_doorbell_entry_t::cap_wa_csr_dhs_local_doorbell_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_local_doorbell_entry_t::~cap_wa_csr_dhs_local_doorbell_entry_t() { }

cap_wa_csr_dhs_local_doorbell_t::cap_wa_csr_dhs_local_doorbell_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_local_doorbell_t::~cap_wa_csr_dhs_local_doorbell_t() { }

cap_wa_csr_dhs_host_doorbell_entry_t::cap_wa_csr_dhs_host_doorbell_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_host_doorbell_entry_t::~cap_wa_csr_dhs_host_doorbell_entry_t() { }

cap_wa_csr_dhs_host_doorbell_t::cap_wa_csr_dhs_host_doorbell_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_wa_csr_dhs_host_doorbell_t::~cap_wa_csr_dhs_host_doorbell_t() { }

cap_wa_csr_sta_ecc_lif_qstate_map_t::cap_wa_csr_sta_ecc_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sta_ecc_lif_qstate_map_t::~cap_wa_csr_sta_ecc_lif_qstate_map_t() { }

cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::~cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t() { }

cap_wa_csr_sat_wa_pid_chkfail_t::cap_wa_csr_sat_wa_pid_chkfail_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sat_wa_pid_chkfail_t::~cap_wa_csr_sat_wa_pid_chkfail_t() { }

cap_wa_csr_sat_wa_qid_overflow_t::cap_wa_csr_sat_wa_qid_overflow_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sat_wa_qid_overflow_t::~cap_wa_csr_sat_wa_qid_overflow_t() { }

cap_wa_csr_sat_wa_qaddr_cam_conflict_t::cap_wa_csr_sat_wa_qaddr_cam_conflict_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sat_wa_qaddr_cam_conflict_t::~cap_wa_csr_sat_wa_qaddr_cam_conflict_t() { }

cap_wa_csr_sat_wa_ring_access_err_t::cap_wa_csr_sat_wa_ring_access_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sat_wa_ring_access_err_t::~cap_wa_csr_sat_wa_ring_access_err_t() { }

cap_wa_csr_sat_wa_host_access_err_t::cap_wa_csr_sat_wa_host_access_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sat_wa_host_access_err_t::~cap_wa_csr_sat_wa_host_access_err_t() { }

cap_wa_csr_sta_wa_axi_t::cap_wa_csr_sta_wa_axi_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_sta_wa_axi_t::~cap_wa_csr_sta_wa_axi_t() { }

cap_wa_csr_cfg_wa_axi_t::cap_wa_csr_cfg_wa_axi_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_cfg_wa_axi_t::~cap_wa_csr_cfg_wa_axi_t() { }

cap_wa_csr_base_t::cap_wa_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_wa_csr_base_t::~cap_wa_csr_base_t() { }

cap_wa_csr_t::cap_wa_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(16777216);
        set_attributes(0,get_name(), 0);
        }
cap_wa_csr_t::~cap_wa_csr_t() { }

void cap_wa_csr_dhs_lif_qstate_map_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".vld: 0x" << int_var__vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qstate_base: 0x" << int_var__qstate_base << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length0: 0x" << int_var__length0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size0: 0x" << int_var__size0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length1: 0x" << int_var__length1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size1: 0x" << int_var__size1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length2: 0x" << int_var__length2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size2: 0x" << int_var__size2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length3: 0x" << int_var__length3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size3: 0x" << int_var__size3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length4: 0x" << int_var__length4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size4: 0x" << int_var__size4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length5: 0x" << int_var__length5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size5: 0x" << int_var__size5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length6: 0x" << int_var__length6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size6: 0x" << int_var__size6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".length7: 0x" << int_var__length7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".size7: 0x" << int_var__size7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl)
}

void cap_wa_csr_dhs_lif_qstate_map_t::show() {

    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].show();
    }
}

void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qstateaddr: 0x" << int_var__qstateaddr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pid: 0x" << int_var__pid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_conflict: 0x" << int_var__addr_conflict << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".tot_ring_err: 0x" << int_var__tot_ring_err << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".host_ring_err: 0x" << int_var__host_ring_err << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pid_fail: 0x" << int_var__pid_fail << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid_ovflow: 0x" << int_var__qid_ovflow << dec << endl)
}

void cap_wa_csr_dhs_doorbell_err_activity_log_t::show() {

    for(int ii = 0; ii < 8; ii++) {
        entry[ii].show();
    }
}

void cap_wa_csr_dhs_page4k_remap_tbl_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".upd_vec: 0x" << int_var__upd_vec << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lif: 0x" << int_var__lif << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pid: 0x" << int_var__pid << dec << endl)
}

void cap_wa_csr_dhs_page4k_remap_tbl_t::show() {

    for(int ii = 0; ii < 32; ii++) {
        entry[ii].show();
    }
}

void cap_wa_csr_dhs_page4k_remap_db_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".index: 0x" << int_var__index << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ring: 0x" << int_var__ring << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd4pid: 0x" << int_var__rsvd4pid << dec << endl)
}

void cap_wa_csr_dhs_page4k_remap_db_t::show() {

    for(int ii = 0; ii < 16384; ii++) {
        entry[ii].show();
    }
}

void cap_wa_csr_dhs_32b_doorbell_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".index: 0x" << int_var__index << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl)
}

void cap_wa_csr_dhs_32b_doorbell_t::show() {

    entry.show(); // large_array
    
}

void cap_wa_csr_dhs_local_doorbell_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".index: 0x" << int_var__index << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ring: 0x" << int_var__ring << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pid: 0x" << int_var__pid << dec << endl)
}

void cap_wa_csr_dhs_local_doorbell_t::show() {

    entry.show(); // large_array
    
}

void cap_wa_csr_dhs_host_doorbell_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".index: 0x" << int_var__index << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ring: 0x" << int_var__ring << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qid: 0x" << int_var__qid << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pid: 0x" << int_var__pid << dec << endl)
}

void cap_wa_csr_dhs_host_doorbell_t::show() {

    entry.show(); // large_array
    
}

void cap_wa_csr_sta_ecc_lif_qstate_map_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl)
}

void cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cor: 0x" << int_var__cor << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".det: 0x" << int_var__det << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dhs: 0x" << int_var__dhs << dec << endl)
}

void cap_wa_csr_sat_wa_pid_chkfail_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl)
}

void cap_wa_csr_sat_wa_qid_overflow_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl)
}

void cap_wa_csr_sat_wa_qaddr_cam_conflict_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl)
}

void cap_wa_csr_sat_wa_ring_access_err_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl)
}

void cap_wa_csr_sat_wa_host_access_err_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".cnt: 0x" << int_var__cnt << dec << endl)
}

void cap_wa_csr_sta_wa_axi_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".num_ids: 0x" << int_var__num_ids << dec << endl)
}

void cap_wa_csr_cfg_wa_axi_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".num_ids: 0x" << int_var__num_ids << dec << endl)
}

void cap_wa_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_wa_csr_t::show() {

    base.show();
    cfg_wa_axi.show();
    sta_wa_axi.show();
    sat_wa_host_access_err.show();
    sat_wa_ring_access_err.show();
    sat_wa_qaddr_cam_conflict.show();
    sat_wa_qid_overflow.show();
    sat_wa_pid_chkfail.show();
    cfg_ecc_disable_lif_qstate_map.show();
    sta_ecc_lif_qstate_map.show();
    dhs_host_doorbell.show();
    dhs_local_doorbell.show();
    dhs_32b_doorbell.show();
    dhs_page4k_remap_db.show();
    dhs_page4k_remap_tbl.show();
    dhs_doorbell_err_activity_log.show();
    dhs_lif_qstate_map.show();
}

int cap_wa_csr_dhs_lif_qstate_map_entry_t::get_width() const {
    return cap_wa_csr_dhs_lif_qstate_map_entry_t::s_get_width();

}

int cap_wa_csr_dhs_lif_qstate_map_t::get_width() const {
    return cap_wa_csr_dhs_lif_qstate_map_t::s_get_width();

}

int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::get_width() const {
    return cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::s_get_width();

}

int cap_wa_csr_dhs_doorbell_err_activity_log_t::get_width() const {
    return cap_wa_csr_dhs_doorbell_err_activity_log_t::s_get_width();

}

int cap_wa_csr_dhs_page4k_remap_tbl_entry_t::get_width() const {
    return cap_wa_csr_dhs_page4k_remap_tbl_entry_t::s_get_width();

}

int cap_wa_csr_dhs_page4k_remap_tbl_t::get_width() const {
    return cap_wa_csr_dhs_page4k_remap_tbl_t::s_get_width();

}

int cap_wa_csr_dhs_page4k_remap_db_entry_t::get_width() const {
    return cap_wa_csr_dhs_page4k_remap_db_entry_t::s_get_width();

}

int cap_wa_csr_dhs_page4k_remap_db_t::get_width() const {
    return cap_wa_csr_dhs_page4k_remap_db_t::s_get_width();

}

int cap_wa_csr_dhs_32b_doorbell_entry_t::get_width() const {
    return cap_wa_csr_dhs_32b_doorbell_entry_t::s_get_width();

}

int cap_wa_csr_dhs_32b_doorbell_t::get_width() const {
    return cap_wa_csr_dhs_32b_doorbell_t::s_get_width();

}

int cap_wa_csr_dhs_local_doorbell_entry_t::get_width() const {
    return cap_wa_csr_dhs_local_doorbell_entry_t::s_get_width();

}

int cap_wa_csr_dhs_local_doorbell_t::get_width() const {
    return cap_wa_csr_dhs_local_doorbell_t::s_get_width();

}

int cap_wa_csr_dhs_host_doorbell_entry_t::get_width() const {
    return cap_wa_csr_dhs_host_doorbell_entry_t::s_get_width();

}

int cap_wa_csr_dhs_host_doorbell_t::get_width() const {
    return cap_wa_csr_dhs_host_doorbell_t::s_get_width();

}

int cap_wa_csr_sta_ecc_lif_qstate_map_t::get_width() const {
    return cap_wa_csr_sta_ecc_lif_qstate_map_t::s_get_width();

}

int cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::get_width() const {
    return cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width();

}

int cap_wa_csr_sat_wa_pid_chkfail_t::get_width() const {
    return cap_wa_csr_sat_wa_pid_chkfail_t::s_get_width();

}

int cap_wa_csr_sat_wa_qid_overflow_t::get_width() const {
    return cap_wa_csr_sat_wa_qid_overflow_t::s_get_width();

}

int cap_wa_csr_sat_wa_qaddr_cam_conflict_t::get_width() const {
    return cap_wa_csr_sat_wa_qaddr_cam_conflict_t::s_get_width();

}

int cap_wa_csr_sat_wa_ring_access_err_t::get_width() const {
    return cap_wa_csr_sat_wa_ring_access_err_t::s_get_width();

}

int cap_wa_csr_sat_wa_host_access_err_t::get_width() const {
    return cap_wa_csr_sat_wa_host_access_err_t::s_get_width();

}

int cap_wa_csr_sta_wa_axi_t::get_width() const {
    return cap_wa_csr_sta_wa_axi_t::s_get_width();

}

int cap_wa_csr_cfg_wa_axi_t::get_width() const {
    return cap_wa_csr_cfg_wa_axi_t::s_get_width();

}

int cap_wa_csr_base_t::get_width() const {
    return cap_wa_csr_base_t::s_get_width();

}

int cap_wa_csr_t::get_width() const {
    return cap_wa_csr_t::s_get_width();

}

int cap_wa_csr_dhs_lif_qstate_map_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // vld
    _count += 22; // qstate_base
    _count += 5; // length0
    _count += 3; // size0
    _count += 5; // length1
    _count += 3; // size1
    _count += 5; // length2
    _count += 3; // size2
    _count += 5; // length3
    _count += 3; // size3
    _count += 5; // length4
    _count += 3; // size4
    _count += 5; // length5
    _count += 3; // size5
    _count += 5; // length6
    _count += 3; // size6
    _count += 5; // length7
    _count += 3; // size7
    _count += 9; // spare
    _count += 8; // ecc
    return _count;
}

int cap_wa_csr_dhs_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_lif_qstate_map_entry_t::s_get_width() * 2048); // entry
    return _count;
}

int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::s_get_width() {
    int _count = 0;

    _count += 1; // valid
    _count += 29; // qstateaddr
    _count += 16; // pid
    _count += 13; // cnt
    _count += 1; // addr_conflict
    _count += 1; // tot_ring_err
    _count += 1; // host_ring_err
    _count += 1; // pid_fail
    _count += 1; // qid_ovflow
    return _count;
}

int cap_wa_csr_dhs_doorbell_err_activity_log_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::s_get_width() * 8); // entry
    return _count;
}

int cap_wa_csr_dhs_page4k_remap_tbl_entry_t::s_get_width() {
    int _count = 0;

    _count += 40; // upd_vec
    _count += 11; // lif
    _count += 16; // pid
    return _count;
}

int cap_wa_csr_dhs_page4k_remap_tbl_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_page4k_remap_tbl_entry_t::s_get_width() * 32); // entry
    return _count;
}

int cap_wa_csr_dhs_page4k_remap_db_entry_t::s_get_width() {
    int _count = 0;

    _count += 16; // index
    _count += 3; // ring
    _count += 5; // rsvd
    _count += 24; // qid
    _count += 16; // rsvd4pid
    return _count;
}

int cap_wa_csr_dhs_page4k_remap_db_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_page4k_remap_db_entry_t::s_get_width() * 16384); // entry
    return _count;
}

int cap_wa_csr_dhs_32b_doorbell_entry_t::s_get_width() {
    int _count = 0;

    _count += 16; // index
    _count += 16; // qid
    return _count;
}

int cap_wa_csr_dhs_32b_doorbell_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_32b_doorbell_entry_t::s_get_width() * 524288); // entry
    return _count;
}

int cap_wa_csr_dhs_local_doorbell_entry_t::s_get_width() {
    int _count = 0;

    _count += 16; // index
    _count += 3; // ring
    _count += 5; // rsvd
    _count += 24; // qid
    _count += 16; // pid
    return _count;
}

int cap_wa_csr_dhs_local_doorbell_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_local_doorbell_entry_t::s_get_width() * 524288); // entry
    return _count;
}

int cap_wa_csr_dhs_host_doorbell_entry_t::s_get_width() {
    int _count = 0;

    _count += 16; // index
    _count += 3; // ring
    _count += 5; // rsvd
    _count += 24; // qid
    _count += 16; // pid
    return _count;
}

int cap_wa_csr_dhs_host_doorbell_t::s_get_width() {
    int _count = 0;

    _count += (cap_wa_csr_dhs_host_doorbell_entry_t::s_get_width() * 524288); // entry
    return _count;
}

int cap_wa_csr_sta_ecc_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 8; // syndrome
    _count += 11; // addr
    return _count;
}

int cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width() {
    int _count = 0;

    _count += 1; // cor
    _count += 1; // det
    _count += 1; // dhs
    return _count;
}

int cap_wa_csr_sat_wa_pid_chkfail_t::s_get_width() {
    int _count = 0;

    _count += 16; // cnt
    return _count;
}

int cap_wa_csr_sat_wa_qid_overflow_t::s_get_width() {
    int _count = 0;

    _count += 16; // cnt
    return _count;
}

int cap_wa_csr_sat_wa_qaddr_cam_conflict_t::s_get_width() {
    int _count = 0;

    _count += 32; // cnt
    return _count;
}

int cap_wa_csr_sat_wa_ring_access_err_t::s_get_width() {
    int _count = 0;

    _count += 16; // cnt
    return _count;
}

int cap_wa_csr_sat_wa_host_access_err_t::s_get_width() {
    int _count = 0;

    _count += 16; // cnt
    return _count;
}

int cap_wa_csr_sta_wa_axi_t::s_get_width() {
    int _count = 0;

    _count += 8; // num_ids
    return _count;
}

int cap_wa_csr_cfg_wa_axi_t::s_get_width() {
    int _count = 0;

    _count += 8; // num_ids
    return _count;
}

int cap_wa_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_wa_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_wa_csr_base_t::s_get_width(); // base
    _count += cap_wa_csr_cfg_wa_axi_t::s_get_width(); // cfg_wa_axi
    _count += cap_wa_csr_sta_wa_axi_t::s_get_width(); // sta_wa_axi
    _count += cap_wa_csr_sat_wa_host_access_err_t::s_get_width(); // sat_wa_host_access_err
    _count += cap_wa_csr_sat_wa_ring_access_err_t::s_get_width(); // sat_wa_ring_access_err
    _count += cap_wa_csr_sat_wa_qaddr_cam_conflict_t::s_get_width(); // sat_wa_qaddr_cam_conflict
    _count += cap_wa_csr_sat_wa_qid_overflow_t::s_get_width(); // sat_wa_qid_overflow
    _count += cap_wa_csr_sat_wa_pid_chkfail_t::s_get_width(); // sat_wa_pid_chkfail
    _count += cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::s_get_width(); // cfg_ecc_disable_lif_qstate_map
    _count += cap_wa_csr_sta_ecc_lif_qstate_map_t::s_get_width(); // sta_ecc_lif_qstate_map
    _count += cap_wa_csr_dhs_host_doorbell_t::s_get_width(); // dhs_host_doorbell
    _count += cap_wa_csr_dhs_local_doorbell_t::s_get_width(); // dhs_local_doorbell
    _count += cap_wa_csr_dhs_32b_doorbell_t::s_get_width(); // dhs_32b_doorbell
    _count += cap_wa_csr_dhs_page4k_remap_db_t::s_get_width(); // dhs_page4k_remap_db
    _count += cap_wa_csr_dhs_page4k_remap_tbl_t::s_get_width(); // dhs_page4k_remap_tbl
    _count += cap_wa_csr_dhs_doorbell_err_activity_log_t::s_get_width(); // dhs_doorbell_err_activity_log
    _count += cap_wa_csr_dhs_lif_qstate_map_t::s_get_width(); // dhs_lif_qstate_map
    return _count;
}

void cap_wa_csr_dhs_lif_qstate_map_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // vld
    int_var__vld = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< vld_cpp_int_t >()  ;
    _count += 1;
    // qstate_base
    int_var__qstate_base = hlp.get_slc(_val, _count, _count -1 + 22 ).convert_to< qstate_base_cpp_int_t >()  ;
    _count += 22;
    // length0
    int_var__length0 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length0_cpp_int_t >()  ;
    _count += 5;
    // size0
    int_var__size0 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size0_cpp_int_t >()  ;
    _count += 3;
    // length1
    int_var__length1 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length1_cpp_int_t >()  ;
    _count += 5;
    // size1
    int_var__size1 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size1_cpp_int_t >()  ;
    _count += 3;
    // length2
    int_var__length2 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length2_cpp_int_t >()  ;
    _count += 5;
    // size2
    int_var__size2 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size2_cpp_int_t >()  ;
    _count += 3;
    // length3
    int_var__length3 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length3_cpp_int_t >()  ;
    _count += 5;
    // size3
    int_var__size3 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size3_cpp_int_t >()  ;
    _count += 3;
    // length4
    int_var__length4 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length4_cpp_int_t >()  ;
    _count += 5;
    // size4
    int_var__size4 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size4_cpp_int_t >()  ;
    _count += 3;
    // length5
    int_var__length5 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length5_cpp_int_t >()  ;
    _count += 5;
    // size5
    int_var__size5 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size5_cpp_int_t >()  ;
    _count += 3;
    // length6
    int_var__length6 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length6_cpp_int_t >()  ;
    _count += 5;
    // size6
    int_var__size6 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size6_cpp_int_t >()  ;
    _count += 3;
    // length7
    int_var__length7 = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< length7_cpp_int_t >()  ;
    _count += 5;
    // size7
    int_var__size7 = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< size7_cpp_int_t >()  ;
    _count += 3;
    // spare
    int_var__spare = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< spare_cpp_int_t >()  ;
    _count += 9;
    // ecc
    int_var__ecc = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< ecc_cpp_int_t >()  ;
    _count += 8;
}

void cap_wa_csr_dhs_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 2048; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // valid
    int_var__valid = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< valid_cpp_int_t >()  ;
    _count += 1;
    // qstateaddr
    int_var__qstateaddr = hlp.get_slc(_val, _count, _count -1 + 29 ).convert_to< qstateaddr_cpp_int_t >()  ;
    _count += 29;
    // pid
    int_var__pid = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< pid_cpp_int_t >()  ;
    _count += 16;
    // cnt
    int_var__cnt = hlp.get_slc(_val, _count, _count -1 + 13 ).convert_to< cnt_cpp_int_t >()  ;
    _count += 13;
    // addr_conflict
    int_var__addr_conflict = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< addr_conflict_cpp_int_t >()  ;
    _count += 1;
    // tot_ring_err
    int_var__tot_ring_err = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< tot_ring_err_cpp_int_t >()  ;
    _count += 1;
    // host_ring_err
    int_var__host_ring_err = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< host_ring_err_cpp_int_t >()  ;
    _count += 1;
    // pid_fail
    int_var__pid_fail = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< pid_fail_cpp_int_t >()  ;
    _count += 1;
    // qid_ovflow
    int_var__qid_ovflow = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< qid_ovflow_cpp_int_t >()  ;
    _count += 1;
}

void cap_wa_csr_dhs_doorbell_err_activity_log_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 8; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_wa_csr_dhs_page4k_remap_tbl_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // upd_vec
    int_var__upd_vec = hlp.get_slc(_val, _count, _count -1 + 40 ).convert_to< upd_vec_cpp_int_t >()  ;
    _count += 40;
    // lif
    int_var__lif = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< lif_cpp_int_t >()  ;
    _count += 11;
    // pid
    int_var__pid = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< pid_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_dhs_page4k_remap_tbl_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_wa_csr_dhs_page4k_remap_db_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // index
    int_var__index = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< index_cpp_int_t >()  ;
    _count += 16;
    // ring
    int_var__ring = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< ring_cpp_int_t >()  ;
    _count += 3;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 5;
    // qid
    int_var__qid = hlp.get_slc(_val, _count, _count -1 + 24 ).convert_to< qid_cpp_int_t >()  ;
    _count += 24;
    // rsvd4pid
    int_var__rsvd4pid = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< rsvd4pid_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_dhs_page4k_remap_db_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 16384; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_wa_csr_dhs_32b_doorbell_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // index
    int_var__index = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< index_cpp_int_t >()  ;
    _count += 16;
    // qid
    int_var__qid = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< qid_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_dhs_32b_doorbell_t::all(const cpp_int & _val) {
    int _count = 0;

    PLOG_ERR("all function for large_array not implemented" << endl);
    
}

void cap_wa_csr_dhs_local_doorbell_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // index
    int_var__index = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< index_cpp_int_t >()  ;
    _count += 16;
    // ring
    int_var__ring = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< ring_cpp_int_t >()  ;
    _count += 3;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 5;
    // qid
    int_var__qid = hlp.get_slc(_val, _count, _count -1 + 24 ).convert_to< qid_cpp_int_t >()  ;
    _count += 24;
    // pid
    int_var__pid = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< pid_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_dhs_local_doorbell_t::all(const cpp_int & _val) {
    int _count = 0;

    PLOG_ERR("all function for large_array not implemented" << endl);
    
}

void cap_wa_csr_dhs_host_doorbell_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // index
    int_var__index = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< index_cpp_int_t >()  ;
    _count += 16;
    // ring
    int_var__ring = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< ring_cpp_int_t >()  ;
    _count += 3;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 5;
    // qid
    int_var__qid = hlp.get_slc(_val, _count, _count -1 + 24 ).convert_to< qid_cpp_int_t >()  ;
    _count += 24;
    // pid
    int_var__pid = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< pid_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_dhs_host_doorbell_t::all(const cpp_int & _val) {
    int _count = 0;

    PLOG_ERR("all function for large_array not implemented" << endl);
    
}

void cap_wa_csr_sta_ecc_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // uncorrectable
    int_var__uncorrectable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< uncorrectable_cpp_int_t >()  ;
    _count += 1;
    // correctable
    int_var__correctable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< correctable_cpp_int_t >()  ;
    _count += 1;
    // syndrome
    int_var__syndrome = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< syndrome_cpp_int_t >()  ;
    _count += 8;
    // addr
    int_var__addr = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< addr_cpp_int_t >()  ;
    _count += 11;
}

void cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::all(const cpp_int & _val) {
    int _count = 0;

    // cor
    int_var__cor = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< cor_cpp_int_t >()  ;
    _count += 1;
    // det
    int_var__det = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< det_cpp_int_t >()  ;
    _count += 1;
    // dhs
    int_var__dhs = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< dhs_cpp_int_t >()  ;
    _count += 1;
}

void cap_wa_csr_sat_wa_pid_chkfail_t::all(const cpp_int & _val) {
    int _count = 0;

    // cnt
    int_var__cnt = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< cnt_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_sat_wa_qid_overflow_t::all(const cpp_int & _val) {
    int _count = 0;

    // cnt
    int_var__cnt = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< cnt_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_sat_wa_qaddr_cam_conflict_t::all(const cpp_int & _val) {
    int _count = 0;

    // cnt
    int_var__cnt = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< cnt_cpp_int_t >()  ;
    _count += 32;
}

void cap_wa_csr_sat_wa_ring_access_err_t::all(const cpp_int & _val) {
    int _count = 0;

    // cnt
    int_var__cnt = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< cnt_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_sat_wa_host_access_err_t::all(const cpp_int & _val) {
    int _count = 0;

    // cnt
    int_var__cnt = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< cnt_cpp_int_t >()  ;
    _count += 16;
}

void cap_wa_csr_sta_wa_axi_t::all(const cpp_int & _val) {
    int _count = 0;

    // num_ids
    int_var__num_ids = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< num_ids_cpp_int_t >()  ;
    _count += 8;
}

void cap_wa_csr_cfg_wa_axi_t::all(const cpp_int & _val) {
    int _count = 0;

    // num_ids
    int_var__num_ids = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< num_ids_cpp_int_t >()  ;
    _count += 8;
}

void cap_wa_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_wa_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_wa_axi.all( hlp.get_slc(_val, _count, _count -1 + cfg_wa_axi.get_width() )); // cfg_wa_axi
    _count += cfg_wa_axi.get_width();
    sta_wa_axi.all( hlp.get_slc(_val, _count, _count -1 + sta_wa_axi.get_width() )); // sta_wa_axi
    _count += sta_wa_axi.get_width();
    sat_wa_host_access_err.all( hlp.get_slc(_val, _count, _count -1 + sat_wa_host_access_err.get_width() )); // sat_wa_host_access_err
    _count += sat_wa_host_access_err.get_width();
    sat_wa_ring_access_err.all( hlp.get_slc(_val, _count, _count -1 + sat_wa_ring_access_err.get_width() )); // sat_wa_ring_access_err
    _count += sat_wa_ring_access_err.get_width();
    sat_wa_qaddr_cam_conflict.all( hlp.get_slc(_val, _count, _count -1 + sat_wa_qaddr_cam_conflict.get_width() )); // sat_wa_qaddr_cam_conflict
    _count += sat_wa_qaddr_cam_conflict.get_width();
    sat_wa_qid_overflow.all( hlp.get_slc(_val, _count, _count -1 + sat_wa_qid_overflow.get_width() )); // sat_wa_qid_overflow
    _count += sat_wa_qid_overflow.get_width();
    sat_wa_pid_chkfail.all( hlp.get_slc(_val, _count, _count -1 + sat_wa_pid_chkfail.get_width() )); // sat_wa_pid_chkfail
    _count += sat_wa_pid_chkfail.get_width();
    cfg_ecc_disable_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + cfg_ecc_disable_lif_qstate_map.get_width() )); // cfg_ecc_disable_lif_qstate_map
    _count += cfg_ecc_disable_lif_qstate_map.get_width();
    sta_ecc_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + sta_ecc_lif_qstate_map.get_width() )); // sta_ecc_lif_qstate_map
    _count += sta_ecc_lif_qstate_map.get_width();
    dhs_host_doorbell.all( hlp.get_slc(_val, _count, _count -1 + dhs_host_doorbell.get_width() )); // dhs_host_doorbell
    _count += dhs_host_doorbell.get_width();
    dhs_local_doorbell.all( hlp.get_slc(_val, _count, _count -1 + dhs_local_doorbell.get_width() )); // dhs_local_doorbell
    _count += dhs_local_doorbell.get_width();
    dhs_32b_doorbell.all( hlp.get_slc(_val, _count, _count -1 + dhs_32b_doorbell.get_width() )); // dhs_32b_doorbell
    _count += dhs_32b_doorbell.get_width();
    dhs_page4k_remap_db.all( hlp.get_slc(_val, _count, _count -1 + dhs_page4k_remap_db.get_width() )); // dhs_page4k_remap_db
    _count += dhs_page4k_remap_db.get_width();
    dhs_page4k_remap_tbl.all( hlp.get_slc(_val, _count, _count -1 + dhs_page4k_remap_tbl.get_width() )); // dhs_page4k_remap_tbl
    _count += dhs_page4k_remap_tbl.get_width();
    dhs_doorbell_err_activity_log.all( hlp.get_slc(_val, _count, _count -1 + dhs_doorbell_err_activity_log.get_width() )); // dhs_doorbell_err_activity_log
    _count += dhs_doorbell_err_activity_log.get_width();
    dhs_lif_qstate_map.all( hlp.get_slc(_val, _count, _count -1 + dhs_lif_qstate_map.get_width() )); // dhs_lif_qstate_map
    _count += dhs_lif_qstate_map.get_width();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__vld) , _count, _count -1 + 1 );
    _count += 1;
    // qstate_base
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qstate_base) , _count, _count -1 + 22 );
    _count += 22;
    // length0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length0) , _count, _count -1 + 5 );
    _count += 5;
    // size0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size0) , _count, _count -1 + 3 );
    _count += 3;
    // length1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length1) , _count, _count -1 + 5 );
    _count += 5;
    // size1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size1) , _count, _count -1 + 3 );
    _count += 3;
    // length2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length2) , _count, _count -1 + 5 );
    _count += 5;
    // size2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size2) , _count, _count -1 + 3 );
    _count += 3;
    // length3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length3) , _count, _count -1 + 5 );
    _count += 5;
    // size3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size3) , _count, _count -1 + 3 );
    _count += 3;
    // length4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length4) , _count, _count -1 + 5 );
    _count += 5;
    // size4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size4) , _count, _count -1 + 3 );
    _count += 3;
    // length5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length5) , _count, _count -1 + 5 );
    _count += 5;
    // size5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size5) , _count, _count -1 + 3 );
    _count += 3;
    // length6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length6) , _count, _count -1 + 5 );
    _count += 5;
    // size6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size6) , _count, _count -1 + 3 );
    _count += 3;
    // length7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__length7) , _count, _count -1 + 5 );
    _count += 5;
    // size7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__size7) , _count, _count -1 + 3 );
    _count += 3;
    // spare
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__spare) , _count, _count -1 + 9 );
    _count += 9;
    // ecc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 2048; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // valid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__valid) , _count, _count -1 + 1 );
    _count += 1;
    // qstateaddr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qstateaddr) , _count, _count -1 + 29 );
    _count += 29;
    // pid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pid) , _count, _count -1 + 16 );
    _count += 16;
    // cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cnt) , _count, _count -1 + 13 );
    _count += 13;
    // addr_conflict
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr_conflict) , _count, _count -1 + 1 );
    _count += 1;
    // tot_ring_err
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__tot_ring_err) , _count, _count -1 + 1 );
    _count += 1;
    // host_ring_err
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__host_ring_err) , _count, _count -1 + 1 );
    _count += 1;
    // pid_fail
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pid_fail) , _count, _count -1 + 1 );
    _count += 1;
    // qid_ovflow
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid_ovflow) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 8; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_wa_csr_dhs_page4k_remap_tbl_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // upd_vec
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__upd_vec) , _count, _count -1 + 40 );
    _count += 40;
    // lif
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lif) , _count, _count -1 + 11 );
    _count += 11;
    // pid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pid) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_page4k_remap_tbl_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 32; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // index
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__index) , _count, _count -1 + 16 );
    _count += 16;
    // ring
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ring) , _count, _count -1 + 3 );
    _count += 3;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 5 );
    _count += 5;
    // qid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid) , _count, _count -1 + 24 );
    _count += 24;
    // rsvd4pid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd4pid) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 16384; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_wa_csr_dhs_32b_doorbell_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // index
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__index) , _count, _count -1 + 16 );
    _count += 16;
    // qid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_32b_doorbell_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    PLOG_ERR("all function for large_array not implemented" << endl);
    
    return ret_val;
}

cpp_int cap_wa_csr_dhs_local_doorbell_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // index
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__index) , _count, _count -1 + 16 );
    _count += 16;
    // ring
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ring) , _count, _count -1 + 3 );
    _count += 3;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 5 );
    _count += 5;
    // qid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid) , _count, _count -1 + 24 );
    _count += 24;
    // pid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pid) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_local_doorbell_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    PLOG_ERR("all function for large_array not implemented" << endl);
    
    return ret_val;
}

cpp_int cap_wa_csr_dhs_host_doorbell_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // index
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__index) , _count, _count -1 + 16 );
    _count += 16;
    // ring
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ring) , _count, _count -1 + 3 );
    _count += 3;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 5 );
    _count += 5;
    // qid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qid) , _count, _count -1 + 24 );
    _count += 24;
    // pid
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pid) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_dhs_host_doorbell_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    PLOG_ERR("all function for large_array not implemented" << endl);
    
    return ret_val;
}

cpp_int cap_wa_csr_sta_ecc_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // uncorrectable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__uncorrectable) , _count, _count -1 + 1 );
    _count += 1;
    // correctable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__correctable) , _count, _count -1 + 1 );
    _count += 1;
    // syndrome
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__syndrome) , _count, _count -1 + 8 );
    _count += 8;
    // addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr) , _count, _count -1 + 11 );
    _count += 11;
    return ret_val;
}

cpp_int cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cor
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cor) , _count, _count -1 + 1 );
    _count += 1;
    // det
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__det) , _count, _count -1 + 1 );
    _count += 1;
    // dhs
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dhs) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_wa_csr_sat_wa_pid_chkfail_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cnt) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_sat_wa_qid_overflow_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cnt) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_sat_wa_qaddr_cam_conflict_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cnt) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_wa_csr_sat_wa_ring_access_err_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cnt) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_sat_wa_host_access_err_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__cnt) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_wa_csr_sta_wa_axi_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // num_ids
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__num_ids) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_wa_csr_cfg_wa_axi_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // num_ids
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__num_ids) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_wa_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_wa_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_wa_axi.all() , _count, _count -1 + cfg_wa_axi.get_width() ); // cfg_wa_axi
    _count += cfg_wa_axi.get_width();
    ret_val = hlp.set_slc(ret_val, sta_wa_axi.all() , _count, _count -1 + sta_wa_axi.get_width() ); // sta_wa_axi
    _count += sta_wa_axi.get_width();
    ret_val = hlp.set_slc(ret_val, sat_wa_host_access_err.all() , _count, _count -1 + sat_wa_host_access_err.get_width() ); // sat_wa_host_access_err
    _count += sat_wa_host_access_err.get_width();
    ret_val = hlp.set_slc(ret_val, sat_wa_ring_access_err.all() , _count, _count -1 + sat_wa_ring_access_err.get_width() ); // sat_wa_ring_access_err
    _count += sat_wa_ring_access_err.get_width();
    ret_val = hlp.set_slc(ret_val, sat_wa_qaddr_cam_conflict.all() , _count, _count -1 + sat_wa_qaddr_cam_conflict.get_width() ); // sat_wa_qaddr_cam_conflict
    _count += sat_wa_qaddr_cam_conflict.get_width();
    ret_val = hlp.set_slc(ret_val, sat_wa_qid_overflow.all() , _count, _count -1 + sat_wa_qid_overflow.get_width() ); // sat_wa_qid_overflow
    _count += sat_wa_qid_overflow.get_width();
    ret_val = hlp.set_slc(ret_val, sat_wa_pid_chkfail.all() , _count, _count -1 + sat_wa_pid_chkfail.get_width() ); // sat_wa_pid_chkfail
    _count += sat_wa_pid_chkfail.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_ecc_disable_lif_qstate_map.all() , _count, _count -1 + cfg_ecc_disable_lif_qstate_map.get_width() ); // cfg_ecc_disable_lif_qstate_map
    _count += cfg_ecc_disable_lif_qstate_map.get_width();
    ret_val = hlp.set_slc(ret_val, sta_ecc_lif_qstate_map.all() , _count, _count -1 + sta_ecc_lif_qstate_map.get_width() ); // sta_ecc_lif_qstate_map
    _count += sta_ecc_lif_qstate_map.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_host_doorbell.all() , _count, _count -1 + dhs_host_doorbell.get_width() ); // dhs_host_doorbell
    _count += dhs_host_doorbell.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_local_doorbell.all() , _count, _count -1 + dhs_local_doorbell.get_width() ); // dhs_local_doorbell
    _count += dhs_local_doorbell.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_32b_doorbell.all() , _count, _count -1 + dhs_32b_doorbell.get_width() ); // dhs_32b_doorbell
    _count += dhs_32b_doorbell.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_page4k_remap_db.all() , _count, _count -1 + dhs_page4k_remap_db.get_width() ); // dhs_page4k_remap_db
    _count += dhs_page4k_remap_db.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_page4k_remap_tbl.all() , _count, _count -1 + dhs_page4k_remap_tbl.get_width() ); // dhs_page4k_remap_tbl
    _count += dhs_page4k_remap_tbl.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_doorbell_err_activity_log.all() , _count, _count -1 + dhs_doorbell_err_activity_log.get_width() ); // dhs_doorbell_err_activity_log
    _count += dhs_doorbell_err_activity_log.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_lif_qstate_map.all() , _count, _count -1 + dhs_lif_qstate_map.get_width() ); // dhs_lif_qstate_map
    _count += dhs_lif_qstate_map.get_width();
    return ret_val;
}

void cap_wa_csr_dhs_lif_qstate_map_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("vld", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::vld);
            register_get_func("vld", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qstate_base", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::qstate_base);
            register_get_func("qstate_base", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::qstate_base);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length0", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length0);
            register_get_func("length0", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size0", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size0);
            register_get_func("size0", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length1", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length1);
            register_get_func("length1", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size1", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size1);
            register_get_func("size1", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length2", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length2);
            register_get_func("length2", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size2", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size2);
            register_get_func("size2", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size2);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length3", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length3);
            register_get_func("length3", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size3", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size3);
            register_get_func("size3", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size3);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length4", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length4);
            register_get_func("length4", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size4", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size4);
            register_get_func("size4", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size4);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length5", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length5);
            register_get_func("length5", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size5", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size5);
            register_get_func("size5", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size5);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length6", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length6);
            register_get_func("length6", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size6", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size6);
            register_get_func("size6", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size6);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("length7", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length7);
            register_get_func("length7", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::length7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("size7", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size7);
            register_get_func("size7", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::size7);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("spare", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::spare);
            register_get_func("spare", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::spare);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::ecc);
            register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_lif_qstate_map_entry_t::ecc);
        }
        #endif
    
}

void cap_wa_csr_dhs_lif_qstate_map_t::init() {

    for(int ii = 0; ii < 2048; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("valid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::valid);
            register_get_func("valid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::valid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qstateaddr", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qstateaddr);
            register_get_func("qstateaddr", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qstateaddr);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid);
            register_get_func("pid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cnt", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::cnt);
            register_get_func("cnt", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::cnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr_conflict", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::addr_conflict);
            register_get_func("addr_conflict", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::addr_conflict);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("tot_ring_err", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::tot_ring_err);
            register_get_func("tot_ring_err", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::tot_ring_err);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("host_ring_err", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::host_ring_err);
            register_get_func("host_ring_err", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::host_ring_err);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pid_fail", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid_fail);
            register_get_func("pid_fail", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid_fail);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid_ovflow", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qid_ovflow);
            register_get_func("qid_ovflow", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qid_ovflow);
        }
        #endif
    
}

void cap_wa_csr_dhs_doorbell_err_activity_log_t::init() {

    for(int ii = 0; ii < 8; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_wa_csr_dhs_page4k_remap_tbl_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("upd_vec", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_tbl_entry_t::upd_vec);
            register_get_func("upd_vec", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_tbl_entry_t::upd_vec);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lif", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_tbl_entry_t::lif);
            register_get_func("lif", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_tbl_entry_t::lif);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_tbl_entry_t::pid);
            register_get_func("pid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_tbl_entry_t::pid);
        }
        #endif
    
}

void cap_wa_csr_dhs_page4k_remap_tbl_t::init() {

    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_wa_csr_dhs_page4k_remap_db_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("index", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::index);
            register_get_func("index", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::index);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ring", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::ring);
            register_get_func("ring", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::ring);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::qid);
            register_get_func("qid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::qid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd4pid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd4pid);
            register_get_func("rsvd4pid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd4pid);
        }
        #endif
    
}

void cap_wa_csr_dhs_page4k_remap_db_t::init() {

    for(int ii = 0; ii < 16384; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_wa_csr_dhs_32b_doorbell_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("index", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_32b_doorbell_entry_t::index);
            register_get_func("index", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_32b_doorbell_entry_t::index);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_32b_doorbell_entry_t::qid);
            register_get_func("qid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_32b_doorbell_entry_t::qid);
        }
        #endif
    
}

void cap_wa_csr_dhs_32b_doorbell_t::init() {

    entry.set_attributes(this, "entry", 0x0);
    
}

void cap_wa_csr_dhs_local_doorbell_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("index", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::index);
            register_get_func("index", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::index);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ring", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::ring);
            register_get_func("ring", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::ring);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::rsvd);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::qid);
            register_get_func("qid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::qid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::pid);
            register_get_func("pid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_local_doorbell_entry_t::pid);
        }
        #endif
    
}

void cap_wa_csr_dhs_local_doorbell_t::init() {

    entry.set_attributes(this, "entry", 0x0);
    
}

void cap_wa_csr_dhs_host_doorbell_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("index", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::index);
            register_get_func("index", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::index);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ring", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::ring);
            register_get_func("ring", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::ring);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::rsvd);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("qid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::qid);
            register_get_func("qid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::qid);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pid", (cap_csr_base::set_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::pid);
            register_get_func("pid", (cap_csr_base::get_function_type_t)&cap_wa_csr_dhs_host_doorbell_entry_t::pid);
        }
        #endif
    
}

void cap_wa_csr_dhs_host_doorbell_t::init() {

    entry.set_attributes(this, "entry", 0x0);
    
}

void cap_wa_csr_sta_ecc_lif_qstate_map_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("uncorrectable", (cap_csr_base::set_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::uncorrectable);
            register_get_func("uncorrectable", (cap_csr_base::get_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::uncorrectable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("correctable", (cap_csr_base::set_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::correctable);
            register_get_func("correctable", (cap_csr_base::get_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::correctable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("syndrome", (cap_csr_base::set_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::syndrome);
            register_get_func("syndrome", (cap_csr_base::get_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::syndrome);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr", (cap_csr_base::set_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::addr);
            register_get_func("addr", (cap_csr_base::get_function_type_t)&cap_wa_csr_sta_ecc_lif_qstate_map_t::addr);
        }
        #endif
    
}

void cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cor", (cap_csr_base::set_function_type_t)&cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::cor);
            register_get_func("cor", (cap_csr_base::get_function_type_t)&cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::cor);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("det", (cap_csr_base::set_function_type_t)&cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::det);
            register_get_func("det", (cap_csr_base::get_function_type_t)&cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::det);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dhs", (cap_csr_base::set_function_type_t)&cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::dhs);
            register_get_func("dhs", (cap_csr_base::get_function_type_t)&cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::dhs);
        }
        #endif
    
}

void cap_wa_csr_sat_wa_pid_chkfail_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cnt", (cap_csr_base::set_function_type_t)&cap_wa_csr_sat_wa_pid_chkfail_t::cnt);
            register_get_func("cnt", (cap_csr_base::get_function_type_t)&cap_wa_csr_sat_wa_pid_chkfail_t::cnt);
        }
        #endif
    
}

void cap_wa_csr_sat_wa_qid_overflow_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cnt", (cap_csr_base::set_function_type_t)&cap_wa_csr_sat_wa_qid_overflow_t::cnt);
            register_get_func("cnt", (cap_csr_base::get_function_type_t)&cap_wa_csr_sat_wa_qid_overflow_t::cnt);
        }
        #endif
    
}

void cap_wa_csr_sat_wa_qaddr_cam_conflict_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cnt", (cap_csr_base::set_function_type_t)&cap_wa_csr_sat_wa_qaddr_cam_conflict_t::cnt);
            register_get_func("cnt", (cap_csr_base::get_function_type_t)&cap_wa_csr_sat_wa_qaddr_cam_conflict_t::cnt);
        }
        #endif
    
}

void cap_wa_csr_sat_wa_ring_access_err_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cnt", (cap_csr_base::set_function_type_t)&cap_wa_csr_sat_wa_ring_access_err_t::cnt);
            register_get_func("cnt", (cap_csr_base::get_function_type_t)&cap_wa_csr_sat_wa_ring_access_err_t::cnt);
        }
        #endif
    
}

void cap_wa_csr_sat_wa_host_access_err_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("cnt", (cap_csr_base::set_function_type_t)&cap_wa_csr_sat_wa_host_access_err_t::cnt);
            register_get_func("cnt", (cap_csr_base::get_function_type_t)&cap_wa_csr_sat_wa_host_access_err_t::cnt);
        }
        #endif
    
}

void cap_wa_csr_sta_wa_axi_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("num_ids", (cap_csr_base::set_function_type_t)&cap_wa_csr_sta_wa_axi_t::num_ids);
            register_get_func("num_ids", (cap_csr_base::get_function_type_t)&cap_wa_csr_sta_wa_axi_t::num_ids);
        }
        #endif
    
}

void cap_wa_csr_cfg_wa_axi_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("num_ids", (cap_csr_base::set_function_type_t)&cap_wa_csr_cfg_wa_axi_t::num_ids);
            register_get_func("num_ids", (cap_csr_base::get_function_type_t)&cap_wa_csr_cfg_wa_axi_t::num_ids);
        }
        #endif
    
}

void cap_wa_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_wa_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_wa_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_wa_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_wa_axi.set_attributes(this,"cfg_wa_axi", 0xe20200 );
    sta_wa_axi.set_attributes(this,"sta_wa_axi", 0xe20204 );
    sat_wa_host_access_err.set_attributes(this,"sat_wa_host_access_err", 0xe20208 );
    sat_wa_ring_access_err.set_attributes(this,"sat_wa_ring_access_err", 0xe2020c );
    sat_wa_qaddr_cam_conflict.set_attributes(this,"sat_wa_qaddr_cam_conflict", 0xe20210 );
    sat_wa_qid_overflow.set_attributes(this,"sat_wa_qid_overflow", 0xe20214 );
    sat_wa_pid_chkfail.set_attributes(this,"sat_wa_pid_chkfail", 0xe20218 );
    cfg_ecc_disable_lif_qstate_map.set_attributes(this,"cfg_ecc_disable_lif_qstate_map", 0xe30000 );
    sta_ecc_lif_qstate_map.set_attributes(this,"sta_ecc_lif_qstate_map", 0xe30004 );
    dhs_host_doorbell.set_attributes(this,"dhs_host_doorbell", 0x400000 );
    dhs_local_doorbell.set_attributes(this,"dhs_local_doorbell", 0x800000 );
    dhs_32b_doorbell.set_attributes(this,"dhs_32b_doorbell", 0xc00000 );
    dhs_page4k_remap_db.set_attributes(this,"dhs_page4k_remap_db", 0xe00000 );
    dhs_page4k_remap_tbl.set_attributes(this,"dhs_page4k_remap_tbl", 0xe20000 );
    dhs_doorbell_err_activity_log.set_attributes(this,"dhs_doorbell_err_activity_log", 0xe20240 );
    dhs_lif_qstate_map.set_attributes(this,"dhs_lif_qstate_map", 0xe28000 );
}

void cap_wa_csr_dhs_lif_qstate_map_entry_t::vld(const cpp_int & _val) { 
    // vld
    int_var__vld = _val.convert_to< vld_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::vld() const {
    return int_var__vld.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::qstate_base(const cpp_int & _val) { 
    // qstate_base
    int_var__qstate_base = _val.convert_to< qstate_base_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::qstate_base() const {
    return int_var__qstate_base.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length0(const cpp_int & _val) { 
    // length0
    int_var__length0 = _val.convert_to< length0_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length0() const {
    return int_var__length0.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size0(const cpp_int & _val) { 
    // size0
    int_var__size0 = _val.convert_to< size0_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size0() const {
    return int_var__size0.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length1(const cpp_int & _val) { 
    // length1
    int_var__length1 = _val.convert_to< length1_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length1() const {
    return int_var__length1.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size1(const cpp_int & _val) { 
    // size1
    int_var__size1 = _val.convert_to< size1_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size1() const {
    return int_var__size1.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length2(const cpp_int & _val) { 
    // length2
    int_var__length2 = _val.convert_to< length2_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length2() const {
    return int_var__length2.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size2(const cpp_int & _val) { 
    // size2
    int_var__size2 = _val.convert_to< size2_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size2() const {
    return int_var__size2.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length3(const cpp_int & _val) { 
    // length3
    int_var__length3 = _val.convert_to< length3_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length3() const {
    return int_var__length3.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size3(const cpp_int & _val) { 
    // size3
    int_var__size3 = _val.convert_to< size3_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size3() const {
    return int_var__size3.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length4(const cpp_int & _val) { 
    // length4
    int_var__length4 = _val.convert_to< length4_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length4() const {
    return int_var__length4.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size4(const cpp_int & _val) { 
    // size4
    int_var__size4 = _val.convert_to< size4_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size4() const {
    return int_var__size4.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length5(const cpp_int & _val) { 
    // length5
    int_var__length5 = _val.convert_to< length5_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length5() const {
    return int_var__length5.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size5(const cpp_int & _val) { 
    // size5
    int_var__size5 = _val.convert_to< size5_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size5() const {
    return int_var__size5.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length6(const cpp_int & _val) { 
    // length6
    int_var__length6 = _val.convert_to< length6_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length6() const {
    return int_var__length6.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size6(const cpp_int & _val) { 
    // size6
    int_var__size6 = _val.convert_to< size6_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size6() const {
    return int_var__size6.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::length7(const cpp_int & _val) { 
    // length7
    int_var__length7 = _val.convert_to< length7_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::length7() const {
    return int_var__length7.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::size7(const cpp_int & _val) { 
    // size7
    int_var__size7 = _val.convert_to< size7_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::size7() const {
    return int_var__size7.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::spare() const {
    return int_var__spare.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_lif_qstate_map_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_lif_qstate_map_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::valid() const {
    return int_var__valid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qstateaddr(const cpp_int & _val) { 
    // qstateaddr
    int_var__qstateaddr = _val.convert_to< qstateaddr_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qstateaddr() const {
    return int_var__qstateaddr.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid(const cpp_int & _val) { 
    // pid
    int_var__pid = _val.convert_to< pid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid() const {
    return int_var__pid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::cnt() const {
    return int_var__cnt.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::addr_conflict(const cpp_int & _val) { 
    // addr_conflict
    int_var__addr_conflict = _val.convert_to< addr_conflict_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::addr_conflict() const {
    return int_var__addr_conflict.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::tot_ring_err(const cpp_int & _val) { 
    // tot_ring_err
    int_var__tot_ring_err = _val.convert_to< tot_ring_err_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::tot_ring_err() const {
    return int_var__tot_ring_err.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::host_ring_err(const cpp_int & _val) { 
    // host_ring_err
    int_var__host_ring_err = _val.convert_to< host_ring_err_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::host_ring_err() const {
    return int_var__host_ring_err.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid_fail(const cpp_int & _val) { 
    // pid_fail
    int_var__pid_fail = _val.convert_to< pid_fail_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::pid_fail() const {
    return int_var__pid_fail.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qid_ovflow(const cpp_int & _val) { 
    // qid_ovflow
    int_var__qid_ovflow = _val.convert_to< qid_ovflow_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_doorbell_err_activity_log_entry_t::qid_ovflow() const {
    return int_var__qid_ovflow.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_tbl_entry_t::upd_vec(const cpp_int & _val) { 
    // upd_vec
    int_var__upd_vec = _val.convert_to< upd_vec_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_tbl_entry_t::upd_vec() const {
    return int_var__upd_vec.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_tbl_entry_t::lif(const cpp_int & _val) { 
    // lif
    int_var__lif = _val.convert_to< lif_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_tbl_entry_t::lif() const {
    return int_var__lif.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_tbl_entry_t::pid(const cpp_int & _val) { 
    // pid
    int_var__pid = _val.convert_to< pid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_tbl_entry_t::pid() const {
    return int_var__pid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_db_entry_t::index(const cpp_int & _val) { 
    // index
    int_var__index = _val.convert_to< index_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_entry_t::index() const {
    return int_var__index.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_db_entry_t::ring(const cpp_int & _val) { 
    // ring
    int_var__ring = _val.convert_to< ring_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_entry_t::ring() const {
    return int_var__ring.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_db_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_entry_t::qid() const {
    return int_var__qid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd4pid(const cpp_int & _val) { 
    // rsvd4pid
    int_var__rsvd4pid = _val.convert_to< rsvd4pid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_page4k_remap_db_entry_t::rsvd4pid() const {
    return int_var__rsvd4pid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_32b_doorbell_entry_t::index(const cpp_int & _val) { 
    // index
    int_var__index = _val.convert_to< index_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_32b_doorbell_entry_t::index() const {
    return int_var__index.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_32b_doorbell_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_32b_doorbell_entry_t::qid() const {
    return int_var__qid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_local_doorbell_entry_t::index(const cpp_int & _val) { 
    // index
    int_var__index = _val.convert_to< index_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_local_doorbell_entry_t::index() const {
    return int_var__index.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_local_doorbell_entry_t::ring(const cpp_int & _val) { 
    // ring
    int_var__ring = _val.convert_to< ring_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_local_doorbell_entry_t::ring() const {
    return int_var__ring.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_local_doorbell_entry_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_local_doorbell_entry_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_local_doorbell_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_local_doorbell_entry_t::qid() const {
    return int_var__qid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_local_doorbell_entry_t::pid(const cpp_int & _val) { 
    // pid
    int_var__pid = _val.convert_to< pid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_local_doorbell_entry_t::pid() const {
    return int_var__pid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_host_doorbell_entry_t::index(const cpp_int & _val) { 
    // index
    int_var__index = _val.convert_to< index_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_host_doorbell_entry_t::index() const {
    return int_var__index.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_host_doorbell_entry_t::ring(const cpp_int & _val) { 
    // ring
    int_var__ring = _val.convert_to< ring_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_host_doorbell_entry_t::ring() const {
    return int_var__ring.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_host_doorbell_entry_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_host_doorbell_entry_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_host_doorbell_entry_t::qid(const cpp_int & _val) { 
    // qid
    int_var__qid = _val.convert_to< qid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_host_doorbell_entry_t::qid() const {
    return int_var__qid.convert_to< cpp_int >();
}
    
void cap_wa_csr_dhs_host_doorbell_entry_t::pid(const cpp_int & _val) { 
    // pid
    int_var__pid = _val.convert_to< pid_cpp_int_t >();
}

cpp_int cap_wa_csr_dhs_host_doorbell_entry_t::pid() const {
    return int_var__pid.convert_to< cpp_int >();
}
    
void cap_wa_csr_sta_ecc_lif_qstate_map_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_wa_csr_sta_ecc_lif_qstate_map_t::uncorrectable() const {
    return int_var__uncorrectable.convert_to< cpp_int >();
}
    
void cap_wa_csr_sta_ecc_lif_qstate_map_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_wa_csr_sta_ecc_lif_qstate_map_t::correctable() const {
    return int_var__correctable.convert_to< cpp_int >();
}
    
void cap_wa_csr_sta_ecc_lif_qstate_map_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_wa_csr_sta_ecc_lif_qstate_map_t::syndrome() const {
    return int_var__syndrome.convert_to< cpp_int >();
}
    
void cap_wa_csr_sta_ecc_lif_qstate_map_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_wa_csr_sta_ecc_lif_qstate_map_t::addr() const {
    return int_var__addr.convert_to< cpp_int >();
}
    
void cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::cor(const cpp_int & _val) { 
    // cor
    int_var__cor = _val.convert_to< cor_cpp_int_t >();
}

cpp_int cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::cor() const {
    return int_var__cor.convert_to< cpp_int >();
}
    
void cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::det(const cpp_int & _val) { 
    // det
    int_var__det = _val.convert_to< det_cpp_int_t >();
}

cpp_int cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::det() const {
    return int_var__det.convert_to< cpp_int >();
}
    
void cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::dhs(const cpp_int & _val) { 
    // dhs
    int_var__dhs = _val.convert_to< dhs_cpp_int_t >();
}

cpp_int cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t::dhs() const {
    return int_var__dhs.convert_to< cpp_int >();
}
    
void cap_wa_csr_sat_wa_pid_chkfail_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_wa_csr_sat_wa_pid_chkfail_t::cnt() const {
    return int_var__cnt.convert_to< cpp_int >();
}
    
void cap_wa_csr_sat_wa_qid_overflow_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_wa_csr_sat_wa_qid_overflow_t::cnt() const {
    return int_var__cnt.convert_to< cpp_int >();
}
    
void cap_wa_csr_sat_wa_qaddr_cam_conflict_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_wa_csr_sat_wa_qaddr_cam_conflict_t::cnt() const {
    return int_var__cnt.convert_to< cpp_int >();
}
    
void cap_wa_csr_sat_wa_ring_access_err_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_wa_csr_sat_wa_ring_access_err_t::cnt() const {
    return int_var__cnt.convert_to< cpp_int >();
}
    
void cap_wa_csr_sat_wa_host_access_err_t::cnt(const cpp_int & _val) { 
    // cnt
    int_var__cnt = _val.convert_to< cnt_cpp_int_t >();
}

cpp_int cap_wa_csr_sat_wa_host_access_err_t::cnt() const {
    return int_var__cnt.convert_to< cpp_int >();
}
    
void cap_wa_csr_sta_wa_axi_t::num_ids(const cpp_int & _val) { 
    // num_ids
    int_var__num_ids = _val.convert_to< num_ids_cpp_int_t >();
}

cpp_int cap_wa_csr_sta_wa_axi_t::num_ids() const {
    return int_var__num_ids.convert_to< cpp_int >();
}
    
void cap_wa_csr_cfg_wa_axi_t::num_ids(const cpp_int & _val) { 
    // num_ids
    int_var__num_ids = _val.convert_to< num_ids_cpp_int_t >();
}

cpp_int cap_wa_csr_cfg_wa_axi_t::num_ids() const {
    return int_var__num_ids.convert_to< cpp_int >();
}
    
void cap_wa_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_wa_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    