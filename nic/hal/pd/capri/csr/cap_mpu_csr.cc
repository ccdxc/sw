
#include "cap_mpu_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_mpu_csr_CNT_sdp_t::cap_mpu_csr_CNT_sdp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_sdp_t::~cap_mpu_csr_CNT_sdp_t() { }

cap_mpu_csr_sta_error_t::cap_mpu_csr_sta_error_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_error_t::~cap_mpu_csr_sta_error_t() { }

cap_mpu_csr_sta_mpu_t::cap_mpu_csr_sta_mpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_sta_mpu_t::~cap_mpu_csr_sta_mpu_t() { }

cap_mpu_csr_CNT_stall_t::cap_mpu_csr_CNT_stall_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_CNT_stall_t::~cap_mpu_csr_CNT_stall_t() { }

cap_mpu_csr_mpu_run_t::cap_mpu_csr_mpu_run_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_mpu_run_t::~cap_mpu_csr_mpu_run_t() { }

cap_mpu_csr_mpu_ctl_t::cap_mpu_csr_mpu_ctl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_mpu_ctl_t::~cap_mpu_csr_mpu_ctl_t() { }

cap_mpu_csr_count_stage_t::cap_mpu_csr_count_stage_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_count_stage_t::~cap_mpu_csr_count_stage_t() { }

cap_mpu_csr_axi_attr_t::cap_mpu_csr_axi_attr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_axi_attr_t::~cap_mpu_csr_axi_attr_t() { }

cap_mpu_csr_icache_t::cap_mpu_csr_icache_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_icache_t::~cap_mpu_csr_icache_t() { }

cap_mpu_csr_trace_t::cap_mpu_csr_trace_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_mpu_csr_trace_t::~cap_mpu_csr_trace_t() { }

cap_mpu_csr_t::cap_mpu_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(4096);
        set_attributes(0,get_name(), 0);
        }
cap_mpu_csr_t::~cap_mpu_csr_t() { }

void cap_mpu_csr_CNT_sdp_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".sop_in: 0x" << int_var__sop_in << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".eop_in: 0x" << int_var__eop_in << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".sop_out: 0x" << int_var__sop_out << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".eop_out: 0x" << int_var__eop_out << dec << endl)
}

void cap_mpu_csr_sta_error_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".icache0: 0x" << int_var__icache0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".icache1: 0x" << int_var__icache1 << dec << endl)
}

void cap_mpu_csr_sta_mpu_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".stopped: 0x" << int_var__stopped << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing: 0x" << int_var__mpu_processing << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_pkt_id: 0x" << int_var__mpu_processing_pkt_id << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_table_id: 0x" << int_var__mpu_processing_table_id << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mpu_processing_table_addr: 0x" << int_var__mpu_processing_table_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".td: 0x" << int_var__td << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".kd: 0x" << int_var__kd << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ex_pc: 0x" << int_var__ex_pc << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr1: 0x" << int_var__gpr1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr2: 0x" << int_var__gpr2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr3: 0x" << int_var__gpr3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr4: 0x" << int_var__gpr4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr5: 0x" << int_var__gpr5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr6: 0x" << int_var__gpr6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gpr7: 0x" << int_var__gpr7 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c1: 0x" << int_var__c1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c2: 0x" << int_var__c2 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c3: 0x" << int_var__c3 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c4: 0x" << int_var__c4 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c5: 0x" << int_var__c5 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c6: 0x" << int_var__c6 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".c7: 0x" << int_var__c7 << dec << endl)
}

void cap_mpu_csr_CNT_stall_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".inst_executed: 0x" << int_var__inst_executed << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".gensrc_stall: 0x" << int_var__gensrc_stall << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phvwr_stall: 0x" << int_var__phvwr_stall << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".icache_fill_stall: 0x" << int_var__icache_fill_stall << dec << endl)
}

void cap_mpu_csr_mpu_run_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".start_pulse: 0x" << int_var__start_pulse << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".stop_pulse: 0x" << int_var__stop_pulse << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".step_pulse: 0x" << int_var__step_pulse << dec << endl)
}

void cap_mpu_csr_mpu_ctl_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable_stop: 0x" << int_var__enable_stop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".enable_illegal_stop: 0x" << int_var__enable_illegal_stop << dec << endl)
}

void cap_mpu_csr_count_stage_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".debug: 0x" << int_var__debug << dec << endl)
}

void cap_mpu_csr_axi_attr_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".arcache: 0x" << int_var__arcache << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".awcache: 0x" << int_var__awcache << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl)
}

void cap_mpu_csr_icache_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".invalidate: 0x" << int_var__invalidate << dec << endl)
}

void cap_mpu_csr_trace_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".phv_debug: 0x" << int_var__phv_debug << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".wrap: 0x" << int_var__wrap << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rst: 0x" << int_var__rst << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".base_addr: 0x" << int_var__base_addr << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".buf_size: 0x" << int_var__buf_size << dec << endl)
}

void cap_mpu_csr_t::show() {

    for(int ii = 0; ii < 4; ii++) {
        trace[ii].show();
    }
    icache.show();
    axi_attr.show();
    count_stage.show();
    mpu_ctl.show();
    for(int ii = 0; ii < 4; ii++) {
        mpu_run[ii].show();
    }
    for(int ii = 0; ii < 4; ii++) {
        CNT_stall[ii].show();
    }
    for(int ii = 0; ii < 4; ii++) {
        sta_mpu[ii].show();
    }
    for(int ii = 0; ii < 4; ii++) {
        sta_error[ii].show();
    }
    CNT_sdp.show();
}

int cap_mpu_csr_CNT_sdp_t::get_width() const {
    return cap_mpu_csr_CNT_sdp_t::s_get_width();

}

int cap_mpu_csr_sta_error_t::get_width() const {
    return cap_mpu_csr_sta_error_t::s_get_width();

}

int cap_mpu_csr_sta_mpu_t::get_width() const {
    return cap_mpu_csr_sta_mpu_t::s_get_width();

}

int cap_mpu_csr_CNT_stall_t::get_width() const {
    return cap_mpu_csr_CNT_stall_t::s_get_width();

}

int cap_mpu_csr_mpu_run_t::get_width() const {
    return cap_mpu_csr_mpu_run_t::s_get_width();

}

int cap_mpu_csr_mpu_ctl_t::get_width() const {
    return cap_mpu_csr_mpu_ctl_t::s_get_width();

}

int cap_mpu_csr_count_stage_t::get_width() const {
    return cap_mpu_csr_count_stage_t::s_get_width();

}

int cap_mpu_csr_axi_attr_t::get_width() const {
    return cap_mpu_csr_axi_attr_t::s_get_width();

}

int cap_mpu_csr_icache_t::get_width() const {
    return cap_mpu_csr_icache_t::s_get_width();

}

int cap_mpu_csr_trace_t::get_width() const {
    return cap_mpu_csr_trace_t::s_get_width();

}

int cap_mpu_csr_t::get_width() const {
    return cap_mpu_csr_t::s_get_width();

}

int cap_mpu_csr_CNT_sdp_t::s_get_width() {
    int _count = 0;

    _count += 16; // sop_in
    _count += 16; // eop_in
    _count += 16; // sop_out
    _count += 16; // eop_out
    return _count;
}

int cap_mpu_csr_sta_error_t::s_get_width() {
    int _count = 0;

    _count += 1; // icache0
    _count += 1; // icache1
    return _count;
}

int cap_mpu_csr_sta_mpu_t::s_get_width() {
    int _count = 0;

    _count += 1; // stopped
    _count += 1; // mpu_processing
    _count += 8; // mpu_processing_pkt_id
    _count += 4; // mpu_processing_table_id
    _count += 64; // mpu_processing_table_addr
    _count += 512; // td
    _count += 512; // kd
    _count += 30; // ex_pc
    _count += 64; // gpr1
    _count += 64; // gpr2
    _count += 64; // gpr3
    _count += 64; // gpr4
    _count += 64; // gpr5
    _count += 64; // gpr6
    _count += 64; // gpr7
    _count += 1; // c1
    _count += 1; // c2
    _count += 1; // c3
    _count += 1; // c4
    _count += 1; // c5
    _count += 1; // c6
    _count += 1; // c7
    return _count;
}

int cap_mpu_csr_CNT_stall_t::s_get_width() {
    int _count = 0;

    _count += 16; // inst_executed
    _count += 8; // gensrc_stall
    _count += 8; // phvwr_stall
    _count += 16; // icache_fill_stall
    return _count;
}

int cap_mpu_csr_mpu_run_t::s_get_width() {
    int _count = 0;

    _count += 1; // start_pulse
    _count += 1; // stop_pulse
    _count += 1; // step_pulse
    return _count;
}

int cap_mpu_csr_mpu_ctl_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable_stop
    _count += 1; // enable_illegal_stop
    return _count;
}

int cap_mpu_csr_count_stage_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // debug
    return _count;
}

int cap_mpu_csr_axi_attr_t::s_get_width() {
    int _count = 0;

    _count += 4; // arcache
    _count += 4; // awcache
    _count += 3; // prot
    _count += 4; // qos
    _count += 1; // lock
    return _count;
}

int cap_mpu_csr_icache_t::s_get_width() {
    int _count = 0;

    _count += 1; // invalidate
    return _count;
}

int cap_mpu_csr_trace_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // phv_debug
    _count += 1; // wrap
    _count += 1; // rst
    _count += 27; // base_addr
    _count += 5; // buf_size
    return _count;
}

int cap_mpu_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_mpu_csr_trace_t::s_get_width() * 4); // trace
    _count += cap_mpu_csr_icache_t::s_get_width(); // icache
    _count += cap_mpu_csr_axi_attr_t::s_get_width(); // axi_attr
    _count += cap_mpu_csr_count_stage_t::s_get_width(); // count_stage
    _count += cap_mpu_csr_mpu_ctl_t::s_get_width(); // mpu_ctl
    _count += (cap_mpu_csr_mpu_run_t::s_get_width() * 4); // mpu_run
    _count += (cap_mpu_csr_CNT_stall_t::s_get_width() * 4); // CNT_stall
    _count += (cap_mpu_csr_sta_mpu_t::s_get_width() * 4); // sta_mpu
    _count += (cap_mpu_csr_sta_error_t::s_get_width() * 4); // sta_error
    _count += cap_mpu_csr_CNT_sdp_t::s_get_width(); // CNT_sdp
    return _count;
}

void cap_mpu_csr_CNT_sdp_t::all(const cpp_int & _val) {
    int _count = 0;

    // sop_in
    int_var__sop_in = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< sop_in_cpp_int_t >()  ;
    _count += 16;
    // eop_in
    int_var__eop_in = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< eop_in_cpp_int_t >()  ;
    _count += 16;
    // sop_out
    int_var__sop_out = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< sop_out_cpp_int_t >()  ;
    _count += 16;
    // eop_out
    int_var__eop_out = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< eop_out_cpp_int_t >()  ;
    _count += 16;
}

void cap_mpu_csr_sta_error_t::all(const cpp_int & _val) {
    int _count = 0;

    // icache0
    int_var__icache0 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< icache0_cpp_int_t >()  ;
    _count += 1;
    // icache1
    int_var__icache1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< icache1_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_sta_mpu_t::all(const cpp_int & _val) {
    int _count = 0;

    // stopped
    int_var__stopped = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< stopped_cpp_int_t >()  ;
    _count += 1;
    // mpu_processing
    int_var__mpu_processing = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< mpu_processing_cpp_int_t >()  ;
    _count += 1;
    // mpu_processing_pkt_id
    int_var__mpu_processing_pkt_id = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< mpu_processing_pkt_id_cpp_int_t >()  ;
    _count += 8;
    // mpu_processing_table_id
    int_var__mpu_processing_table_id = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< mpu_processing_table_id_cpp_int_t >()  ;
    _count += 4;
    // mpu_processing_table_addr
    int_var__mpu_processing_table_addr = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< mpu_processing_table_addr_cpp_int_t >()  ;
    _count += 64;
    // td
    int_var__td = hlp.get_slc(_val, _count, _count -1 + 512 ).convert_to< td_cpp_int_t >()  ;
    _count += 512;
    // kd
    int_var__kd = hlp.get_slc(_val, _count, _count -1 + 512 ).convert_to< kd_cpp_int_t >()  ;
    _count += 512;
    // ex_pc
    int_var__ex_pc = hlp.get_slc(_val, _count, _count -1 + 30 ).convert_to< ex_pc_cpp_int_t >()  ;
    _count += 30;
    // gpr1
    int_var__gpr1 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr1_cpp_int_t >()  ;
    _count += 64;
    // gpr2
    int_var__gpr2 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr2_cpp_int_t >()  ;
    _count += 64;
    // gpr3
    int_var__gpr3 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr3_cpp_int_t >()  ;
    _count += 64;
    // gpr4
    int_var__gpr4 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr4_cpp_int_t >()  ;
    _count += 64;
    // gpr5
    int_var__gpr5 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr5_cpp_int_t >()  ;
    _count += 64;
    // gpr6
    int_var__gpr6 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr6_cpp_int_t >()  ;
    _count += 64;
    // gpr7
    int_var__gpr7 = hlp.get_slc(_val, _count, _count -1 + 64 ).convert_to< gpr7_cpp_int_t >()  ;
    _count += 64;
    // c1
    int_var__c1 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c1_cpp_int_t >()  ;
    _count += 1;
    // c2
    int_var__c2 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c2_cpp_int_t >()  ;
    _count += 1;
    // c3
    int_var__c3 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c3_cpp_int_t >()  ;
    _count += 1;
    // c4
    int_var__c4 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c4_cpp_int_t >()  ;
    _count += 1;
    // c5
    int_var__c5 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c5_cpp_int_t >()  ;
    _count += 1;
    // c6
    int_var__c6 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c6_cpp_int_t >()  ;
    _count += 1;
    // c7
    int_var__c7 = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< c7_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_CNT_stall_t::all(const cpp_int & _val) {
    int _count = 0;

    // inst_executed
    int_var__inst_executed = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< inst_executed_cpp_int_t >()  ;
    _count += 16;
    // gensrc_stall
    int_var__gensrc_stall = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< gensrc_stall_cpp_int_t >()  ;
    _count += 8;
    // phvwr_stall
    int_var__phvwr_stall = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< phvwr_stall_cpp_int_t >()  ;
    _count += 8;
    // icache_fill_stall
    int_var__icache_fill_stall = hlp.get_slc(_val, _count, _count -1 + 16 ).convert_to< icache_fill_stall_cpp_int_t >()  ;
    _count += 16;
}

void cap_mpu_csr_mpu_run_t::all(const cpp_int & _val) {
    int _count = 0;

    // start_pulse
    int_var__start_pulse = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< start_pulse_cpp_int_t >()  ;
    _count += 1;
    // stop_pulse
    int_var__stop_pulse = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< stop_pulse_cpp_int_t >()  ;
    _count += 1;
    // step_pulse
    int_var__step_pulse = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< step_pulse_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_mpu_ctl_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable_stop
    int_var__enable_stop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_stop_cpp_int_t >()  ;
    _count += 1;
    // enable_illegal_stop
    int_var__enable_illegal_stop = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_illegal_stop_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_count_stage_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // debug
    int_var__debug = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< debug_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_axi_attr_t::all(const cpp_int & _val) {
    int _count = 0;

    // arcache
    int_var__arcache = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< arcache_cpp_int_t >()  ;
    _count += 4;
    // awcache
    int_var__awcache = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< awcache_cpp_int_t >()  ;
    _count += 4;
    // prot
    int_var__prot = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< prot_cpp_int_t >()  ;
    _count += 3;
    // qos
    int_var__qos = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< qos_cpp_int_t >()  ;
    _count += 4;
    // lock
    int_var__lock = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< lock_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_icache_t::all(const cpp_int & _val) {
    int _count = 0;

    // invalidate
    int_var__invalidate = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< invalidate_cpp_int_t >()  ;
    _count += 1;
}

void cap_mpu_csr_trace_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // phv_debug
    int_var__phv_debug = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< phv_debug_cpp_int_t >()  ;
    _count += 1;
    // wrap
    int_var__wrap = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< wrap_cpp_int_t >()  ;
    _count += 1;
    // rst
    int_var__rst = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< rst_cpp_int_t >()  ;
    _count += 1;
    // base_addr
    int_var__base_addr = hlp.get_slc(_val, _count, _count -1 + 27 ).convert_to< base_addr_cpp_int_t >()  ;
    _count += 27;
    // buf_size
    int_var__buf_size = hlp.get_slc(_val, _count, _count -1 + 5 ).convert_to< buf_size_cpp_int_t >()  ;
    _count += 5;
}

void cap_mpu_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    // trace
    for(int ii = 0; ii < 4; ii++) {
        trace[ii].all( hlp.get_slc(_val, _count, _count -1 + trace[ii].get_width()));
        _count += trace[ii].get_width();
    }
    icache.all( hlp.get_slc(_val, _count, _count -1 + icache.get_width() )); // icache
    _count += icache.get_width();
    axi_attr.all( hlp.get_slc(_val, _count, _count -1 + axi_attr.get_width() )); // axi_attr
    _count += axi_attr.get_width();
    count_stage.all( hlp.get_slc(_val, _count, _count -1 + count_stage.get_width() )); // count_stage
    _count += count_stage.get_width();
    mpu_ctl.all( hlp.get_slc(_val, _count, _count -1 + mpu_ctl.get_width() )); // mpu_ctl
    _count += mpu_ctl.get_width();
    // mpu_run
    for(int ii = 0; ii < 4; ii++) {
        mpu_run[ii].all( hlp.get_slc(_val, _count, _count -1 + mpu_run[ii].get_width()));
        _count += mpu_run[ii].get_width();
    }
    // CNT_stall
    for(int ii = 0; ii < 4; ii++) {
        CNT_stall[ii].all( hlp.get_slc(_val, _count, _count -1 + CNT_stall[ii].get_width()));
        _count += CNT_stall[ii].get_width();
    }
    // sta_mpu
    for(int ii = 0; ii < 4; ii++) {
        sta_mpu[ii].all( hlp.get_slc(_val, _count, _count -1 + sta_mpu[ii].get_width()));
        _count += sta_mpu[ii].get_width();
    }
    // sta_error
    for(int ii = 0; ii < 4; ii++) {
        sta_error[ii].all( hlp.get_slc(_val, _count, _count -1 + sta_error[ii].get_width()));
        _count += sta_error[ii].get_width();
    }
    CNT_sdp.all( hlp.get_slc(_val, _count, _count -1 + CNT_sdp.get_width() )); // CNT_sdp
    _count += CNT_sdp.get_width();
}

cpp_int cap_mpu_csr_CNT_sdp_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // sop_in
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sop_in) , _count, _count -1 + 16 );
    _count += 16;
    // eop_in
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__eop_in) , _count, _count -1 + 16 );
    _count += 16;
    // sop_out
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__sop_out) , _count, _count -1 + 16 );
    _count += 16;
    // eop_out
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__eop_out) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_mpu_csr_sta_error_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // icache0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__icache0) , _count, _count -1 + 1 );
    _count += 1;
    // icache1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__icache1) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_sta_mpu_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // stopped
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__stopped) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_processing
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_processing) , _count, _count -1 + 1 );
    _count += 1;
    // mpu_processing_pkt_id
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_processing_pkt_id) , _count, _count -1 + 8 );
    _count += 8;
    // mpu_processing_table_id
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_processing_table_id) , _count, _count -1 + 4 );
    _count += 4;
    // mpu_processing_table_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mpu_processing_table_addr) , _count, _count -1 + 64 );
    _count += 64;
    // td
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__td) , _count, _count -1 + 512 );
    _count += 512;
    // kd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__kd) , _count, _count -1 + 512 );
    _count += 512;
    // ex_pc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ex_pc) , _count, _count -1 + 30 );
    _count += 30;
    // gpr1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr1) , _count, _count -1 + 64 );
    _count += 64;
    // gpr2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr2) , _count, _count -1 + 64 );
    _count += 64;
    // gpr3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr3) , _count, _count -1 + 64 );
    _count += 64;
    // gpr4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr4) , _count, _count -1 + 64 );
    _count += 64;
    // gpr5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr5) , _count, _count -1 + 64 );
    _count += 64;
    // gpr6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr6) , _count, _count -1 + 64 );
    _count += 64;
    // gpr7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gpr7) , _count, _count -1 + 64 );
    _count += 64;
    // c1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c1) , _count, _count -1 + 1 );
    _count += 1;
    // c2
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c2) , _count, _count -1 + 1 );
    _count += 1;
    // c3
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c3) , _count, _count -1 + 1 );
    _count += 1;
    // c4
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c4) , _count, _count -1 + 1 );
    _count += 1;
    // c5
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c5) , _count, _count -1 + 1 );
    _count += 1;
    // c6
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c6) , _count, _count -1 + 1 );
    _count += 1;
    // c7
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__c7) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_CNT_stall_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // inst_executed
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__inst_executed) , _count, _count -1 + 16 );
    _count += 16;
    // gensrc_stall
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__gensrc_stall) , _count, _count -1 + 8 );
    _count += 8;
    // phvwr_stall
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phvwr_stall) , _count, _count -1 + 8 );
    _count += 8;
    // icache_fill_stall
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__icache_fill_stall) , _count, _count -1 + 16 );
    _count += 16;
    return ret_val;
}

cpp_int cap_mpu_csr_mpu_run_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // start_pulse
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__start_pulse) , _count, _count -1 + 1 );
    _count += 1;
    // stop_pulse
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__stop_pulse) , _count, _count -1 + 1 );
    _count += 1;
    // step_pulse
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__step_pulse) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_mpu_ctl_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable_stop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable_stop) , _count, _count -1 + 1 );
    _count += 1;
    // enable_illegal_stop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable_illegal_stop) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_count_stage_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // debug
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__debug) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_axi_attr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // arcache
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__arcache) , _count, _count -1 + 4 );
    _count += 4;
    // awcache
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__awcache) , _count, _count -1 + 4 );
    _count += 4;
    // prot
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__prot) , _count, _count -1 + 3 );
    _count += 3;
    // qos
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__qos) , _count, _count -1 + 4 );
    _count += 4;
    // lock
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lock) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_icache_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // invalidate
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__invalidate) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_mpu_csr_trace_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // phv_debug
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__phv_debug) , _count, _count -1 + 1 );
    _count += 1;
    // wrap
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__wrap) , _count, _count -1 + 1 );
    _count += 1;
    // rst
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rst) , _count, _count -1 + 1 );
    _count += 1;
    // base_addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__base_addr) , _count, _count -1 + 27 );
    _count += 27;
    // buf_size
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__buf_size) , _count, _count -1 + 5 );
    _count += 5;
    return ret_val;
}

cpp_int cap_mpu_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // trace
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, trace[ii].all() , _count, _count -1 + trace[ii].get_width() );
        _count += trace[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, icache.all() , _count, _count -1 + icache.get_width() ); // icache
    _count += icache.get_width();
    ret_val = hlp.set_slc(ret_val, axi_attr.all() , _count, _count -1 + axi_attr.get_width() ); // axi_attr
    _count += axi_attr.get_width();
    ret_val = hlp.set_slc(ret_val, count_stage.all() , _count, _count -1 + count_stage.get_width() ); // count_stage
    _count += count_stage.get_width();
    ret_val = hlp.set_slc(ret_val, mpu_ctl.all() , _count, _count -1 + mpu_ctl.get_width() ); // mpu_ctl
    _count += mpu_ctl.get_width();
    // mpu_run
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, mpu_run[ii].all() , _count, _count -1 + mpu_run[ii].get_width() );
        _count += mpu_run[ii].get_width();
    }
    // CNT_stall
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, CNT_stall[ii].all() , _count, _count -1 + CNT_stall[ii].get_width() );
        _count += CNT_stall[ii].get_width();
    }
    // sta_mpu
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, sta_mpu[ii].all() , _count, _count -1 + sta_mpu[ii].get_width() );
        _count += sta_mpu[ii].get_width();
    }
    // sta_error
    for(int ii = 0; ii < 4; ii++) {
         ret_val = hlp.set_slc(ret_val, sta_error[ii].all() , _count, _count -1 + sta_error[ii].get_width() );
        _count += sta_error[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, CNT_sdp.all() , _count, _count -1 + CNT_sdp.get_width() ); // CNT_sdp
    _count += CNT_sdp.get_width();
    return ret_val;
}

void cap_mpu_csr_CNT_sdp_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("sop_in", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_sdp_t::sop_in);
        register_get_func("sop_in", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_sdp_t::sop_in);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("eop_in", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_sdp_t::eop_in);
        register_get_func("eop_in", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_sdp_t::eop_in);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("sop_out", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_sdp_t::sop_out);
        register_get_func("sop_out", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_sdp_t::sop_out);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("eop_out", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_sdp_t::eop_out);
        register_get_func("eop_out", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_sdp_t::eop_out);
    #endif
    
}

void cap_mpu_csr_sta_error_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("icache0", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_error_t::icache0);
        register_get_func("icache0", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_error_t::icache0);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("icache1", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_error_t::icache1);
        register_get_func("icache1", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_error_t::icache1);
    #endif
    
}

void cap_mpu_csr_sta_mpu_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("stopped", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::stopped);
        register_get_func("stopped", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::stopped);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("mpu_processing", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing);
        register_get_func("mpu_processing", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("mpu_processing_pkt_id", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing_pkt_id);
        register_get_func("mpu_processing_pkt_id", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing_pkt_id);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("mpu_processing_table_id", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing_table_id);
        register_get_func("mpu_processing_table_id", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing_table_id);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("mpu_processing_table_addr", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing_table_addr);
        register_get_func("mpu_processing_table_addr", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::mpu_processing_table_addr);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("td", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::td);
        register_get_func("td", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::td);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("kd", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::kd);
        register_get_func("kd", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::kd);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("ex_pc", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::ex_pc);
        register_get_func("ex_pc", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::ex_pc);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr1", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr1);
        register_get_func("gpr1", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr1);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr2", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr2);
        register_get_func("gpr2", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr2);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr3", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr3);
        register_get_func("gpr3", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr3);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr4", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr4);
        register_get_func("gpr4", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr4);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr5", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr5);
        register_get_func("gpr5", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr5);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr6", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr6);
        register_get_func("gpr6", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr6);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gpr7", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr7);
        register_get_func("gpr7", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::gpr7);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c1", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c1);
        register_get_func("c1", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c1);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c2", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c2);
        register_get_func("c2", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c2);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c3", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c3);
        register_get_func("c3", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c3);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c4", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c4);
        register_get_func("c4", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c4);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c5", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c5);
        register_get_func("c5", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c5);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c6", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c6);
        register_get_func("c6", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c6);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("c7", (cap_csr_base::set_function_type_t)&cap_mpu_csr_sta_mpu_t::c7);
        register_get_func("c7", (cap_csr_base::get_function_type_t)&cap_mpu_csr_sta_mpu_t::c7);
    #endif
    
}

void cap_mpu_csr_CNT_stall_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("inst_executed", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_stall_t::inst_executed);
        register_get_func("inst_executed", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_stall_t::inst_executed);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("gensrc_stall", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_stall_t::gensrc_stall);
        register_get_func("gensrc_stall", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_stall_t::gensrc_stall);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("phvwr_stall", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_stall_t::phvwr_stall);
        register_get_func("phvwr_stall", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_stall_t::phvwr_stall);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("icache_fill_stall", (cap_csr_base::set_function_type_t)&cap_mpu_csr_CNT_stall_t::icache_fill_stall);
        register_get_func("icache_fill_stall", (cap_csr_base::get_function_type_t)&cap_mpu_csr_CNT_stall_t::icache_fill_stall);
    #endif
    
}

void cap_mpu_csr_mpu_run_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("start_pulse", (cap_csr_base::set_function_type_t)&cap_mpu_csr_mpu_run_t::start_pulse);
        register_get_func("start_pulse", (cap_csr_base::get_function_type_t)&cap_mpu_csr_mpu_run_t::start_pulse);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("stop_pulse", (cap_csr_base::set_function_type_t)&cap_mpu_csr_mpu_run_t::stop_pulse);
        register_get_func("stop_pulse", (cap_csr_base::get_function_type_t)&cap_mpu_csr_mpu_run_t::stop_pulse);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("step_pulse", (cap_csr_base::set_function_type_t)&cap_mpu_csr_mpu_run_t::step_pulse);
        register_get_func("step_pulse", (cap_csr_base::get_function_type_t)&cap_mpu_csr_mpu_run_t::step_pulse);
    #endif
    
}

void cap_mpu_csr_mpu_ctl_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("enable_stop", (cap_csr_base::set_function_type_t)&cap_mpu_csr_mpu_ctl_t::enable_stop);
        register_get_func("enable_stop", (cap_csr_base::get_function_type_t)&cap_mpu_csr_mpu_ctl_t::enable_stop);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("enable_illegal_stop", (cap_csr_base::set_function_type_t)&cap_mpu_csr_mpu_ctl_t::enable_illegal_stop);
        register_get_func("enable_illegal_stop", (cap_csr_base::get_function_type_t)&cap_mpu_csr_mpu_ctl_t::enable_illegal_stop);
    #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_mpu_csr_count_stage_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_mpu_csr_count_stage_t::enable);
        register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_mpu_csr_count_stage_t::enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("debug", (cap_csr_base::set_function_type_t)&cap_mpu_csr_count_stage_t::debug);
        register_get_func("debug", (cap_csr_base::get_function_type_t)&cap_mpu_csr_count_stage_t::debug);
    #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_mpu_csr_axi_attr_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("arcache", (cap_csr_base::set_function_type_t)&cap_mpu_csr_axi_attr_t::arcache);
        register_get_func("arcache", (cap_csr_base::get_function_type_t)&cap_mpu_csr_axi_attr_t::arcache);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("awcache", (cap_csr_base::set_function_type_t)&cap_mpu_csr_axi_attr_t::awcache);
        register_get_func("awcache", (cap_csr_base::get_function_type_t)&cap_mpu_csr_axi_attr_t::awcache);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("prot", (cap_csr_base::set_function_type_t)&cap_mpu_csr_axi_attr_t::prot);
        register_get_func("prot", (cap_csr_base::get_function_type_t)&cap_mpu_csr_axi_attr_t::prot);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("qos", (cap_csr_base::set_function_type_t)&cap_mpu_csr_axi_attr_t::qos);
        register_get_func("qos", (cap_csr_base::get_function_type_t)&cap_mpu_csr_axi_attr_t::qos);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("lock", (cap_csr_base::set_function_type_t)&cap_mpu_csr_axi_attr_t::lock);
        register_get_func("lock", (cap_csr_base::get_function_type_t)&cap_mpu_csr_axi_attr_t::lock);
    #endif
    
    set_reset_val(cpp_int("0xff"));
    all(get_reset_val());
}

void cap_mpu_csr_icache_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("invalidate", (cap_csr_base::set_function_type_t)&cap_mpu_csr_icache_t::invalidate);
        register_get_func("invalidate", (cap_csr_base::get_function_type_t)&cap_mpu_csr_icache_t::invalidate);
    #endif
    
}

void cap_mpu_csr_trace_t::init() {

    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_mpu_csr_trace_t::enable);
        register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_mpu_csr_trace_t::enable);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("phv_debug", (cap_csr_base::set_function_type_t)&cap_mpu_csr_trace_t::phv_debug);
        register_get_func("phv_debug", (cap_csr_base::get_function_type_t)&cap_mpu_csr_trace_t::phv_debug);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("wrap", (cap_csr_base::set_function_type_t)&cap_mpu_csr_trace_t::wrap);
        register_get_func("wrap", (cap_csr_base::get_function_type_t)&cap_mpu_csr_trace_t::wrap);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("rst", (cap_csr_base::set_function_type_t)&cap_mpu_csr_trace_t::rst);
        register_get_func("rst", (cap_csr_base::get_function_type_t)&cap_mpu_csr_trace_t::rst);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("base_addr", (cap_csr_base::set_function_type_t)&cap_mpu_csr_trace_t::base_addr);
        register_get_func("base_addr", (cap_csr_base::get_function_type_t)&cap_mpu_csr_trace_t::base_addr);
    #endif
    
    #ifdef CAP_PER_FIELD_CNTRL_EN
        register_set_func("buf_size", (cap_csr_base::set_function_type_t)&cap_mpu_csr_trace_t::buf_size);
        register_get_func("buf_size", (cap_csr_base::get_function_type_t)&cap_mpu_csr_trace_t::buf_size);
    #endif
    
}

void cap_mpu_csr_t::init() {

    for(int ii = 0; ii < 4; ii++) {
        trace[ii].set_attributes(this,"trace["+to_string(ii)+"]",  0x0 + (trace[ii].get_byte_size()*ii));
    }
    icache.set_attributes(this,"icache", 0x20 );
    axi_attr.set_attributes(this,"axi_attr", 0x24 );
    count_stage.set_attributes(this,"count_stage", 0x28 );
    mpu_ctl.set_attributes(this,"mpu_ctl", 0x2c );
    for(int ii = 0; ii < 4; ii++) {
        mpu_run[ii].set_attributes(this,"mpu_run["+to_string(ii)+"]",  0x30 + (mpu_run[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 4; ii++) {
        CNT_stall[ii].set_attributes(this,"CNT_stall["+to_string(ii)+"]",  0x40 + (CNT_stall[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 4; ii++) {
        sta_mpu[ii].set_attributes(this,"sta_mpu["+to_string(ii)+"]",  0x400 + (sta_mpu[ii].get_byte_size()*ii));
    }
    for(int ii = 0; ii < 4; ii++) {
        sta_error[ii].set_attributes(this,"sta_error["+to_string(ii)+"]",  0x800 + (sta_error[ii].get_byte_size()*ii));
    }
    CNT_sdp.set_attributes(this,"CNT_sdp", 0x810 );
}

void cap_mpu_csr_CNT_sdp_t::sop_in(const cpp_int & _val) { 
    // sop_in
    int_var__sop_in = _val.convert_to< sop_in_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::sop_in() const {
    return int_var__sop_in.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_sdp_t::eop_in(const cpp_int & _val) { 
    // eop_in
    int_var__eop_in = _val.convert_to< eop_in_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::eop_in() const {
    return int_var__eop_in.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_sdp_t::sop_out(const cpp_int & _val) { 
    // sop_out
    int_var__sop_out = _val.convert_to< sop_out_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::sop_out() const {
    return int_var__sop_out.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_sdp_t::eop_out(const cpp_int & _val) { 
    // eop_out
    int_var__eop_out = _val.convert_to< eop_out_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_sdp_t::eop_out() const {
    return int_var__eop_out.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_error_t::icache0(const cpp_int & _val) { 
    // icache0
    int_var__icache0 = _val.convert_to< icache0_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_error_t::icache0() const {
    return int_var__icache0.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_error_t::icache1(const cpp_int & _val) { 
    // icache1
    int_var__icache1 = _val.convert_to< icache1_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_error_t::icache1() const {
    return int_var__icache1.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::stopped(const cpp_int & _val) { 
    // stopped
    int_var__stopped = _val.convert_to< stopped_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::stopped() const {
    return int_var__stopped.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::mpu_processing(const cpp_int & _val) { 
    // mpu_processing
    int_var__mpu_processing = _val.convert_to< mpu_processing_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::mpu_processing() const {
    return int_var__mpu_processing.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::mpu_processing_pkt_id(const cpp_int & _val) { 
    // mpu_processing_pkt_id
    int_var__mpu_processing_pkt_id = _val.convert_to< mpu_processing_pkt_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::mpu_processing_pkt_id() const {
    return int_var__mpu_processing_pkt_id.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::mpu_processing_table_id(const cpp_int & _val) { 
    // mpu_processing_table_id
    int_var__mpu_processing_table_id = _val.convert_to< mpu_processing_table_id_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::mpu_processing_table_id() const {
    return int_var__mpu_processing_table_id.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::mpu_processing_table_addr(const cpp_int & _val) { 
    // mpu_processing_table_addr
    int_var__mpu_processing_table_addr = _val.convert_to< mpu_processing_table_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::mpu_processing_table_addr() const {
    return int_var__mpu_processing_table_addr.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::td(const cpp_int & _val) { 
    // td
    int_var__td = _val.convert_to< td_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::td() const {
    return int_var__td.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::kd(const cpp_int & _val) { 
    // kd
    int_var__kd = _val.convert_to< kd_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::kd() const {
    return int_var__kd.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::ex_pc(const cpp_int & _val) { 
    // ex_pc
    int_var__ex_pc = _val.convert_to< ex_pc_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::ex_pc() const {
    return int_var__ex_pc.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr1(const cpp_int & _val) { 
    // gpr1
    int_var__gpr1 = _val.convert_to< gpr1_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr1() const {
    return int_var__gpr1.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr2(const cpp_int & _val) { 
    // gpr2
    int_var__gpr2 = _val.convert_to< gpr2_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr2() const {
    return int_var__gpr2.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr3(const cpp_int & _val) { 
    // gpr3
    int_var__gpr3 = _val.convert_to< gpr3_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr3() const {
    return int_var__gpr3.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr4(const cpp_int & _val) { 
    // gpr4
    int_var__gpr4 = _val.convert_to< gpr4_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr4() const {
    return int_var__gpr4.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr5(const cpp_int & _val) { 
    // gpr5
    int_var__gpr5 = _val.convert_to< gpr5_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr5() const {
    return int_var__gpr5.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr6(const cpp_int & _val) { 
    // gpr6
    int_var__gpr6 = _val.convert_to< gpr6_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr6() const {
    return int_var__gpr6.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::gpr7(const cpp_int & _val) { 
    // gpr7
    int_var__gpr7 = _val.convert_to< gpr7_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::gpr7() const {
    return int_var__gpr7.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c1(const cpp_int & _val) { 
    // c1
    int_var__c1 = _val.convert_to< c1_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c1() const {
    return int_var__c1.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c2(const cpp_int & _val) { 
    // c2
    int_var__c2 = _val.convert_to< c2_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c2() const {
    return int_var__c2.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c3(const cpp_int & _val) { 
    // c3
    int_var__c3 = _val.convert_to< c3_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c3() const {
    return int_var__c3.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c4(const cpp_int & _val) { 
    // c4
    int_var__c4 = _val.convert_to< c4_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c4() const {
    return int_var__c4.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c5(const cpp_int & _val) { 
    // c5
    int_var__c5 = _val.convert_to< c5_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c5() const {
    return int_var__c5.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c6(const cpp_int & _val) { 
    // c6
    int_var__c6 = _val.convert_to< c6_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c6() const {
    return int_var__c6.convert_to< cpp_int >();
}
    
void cap_mpu_csr_sta_mpu_t::c7(const cpp_int & _val) { 
    // c7
    int_var__c7 = _val.convert_to< c7_cpp_int_t >();
}

cpp_int cap_mpu_csr_sta_mpu_t::c7() const {
    return int_var__c7.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_stall_t::inst_executed(const cpp_int & _val) { 
    // inst_executed
    int_var__inst_executed = _val.convert_to< inst_executed_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_stall_t::inst_executed() const {
    return int_var__inst_executed.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_stall_t::gensrc_stall(const cpp_int & _val) { 
    // gensrc_stall
    int_var__gensrc_stall = _val.convert_to< gensrc_stall_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_stall_t::gensrc_stall() const {
    return int_var__gensrc_stall.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_stall_t::phvwr_stall(const cpp_int & _val) { 
    // phvwr_stall
    int_var__phvwr_stall = _val.convert_to< phvwr_stall_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_stall_t::phvwr_stall() const {
    return int_var__phvwr_stall.convert_to< cpp_int >();
}
    
void cap_mpu_csr_CNT_stall_t::icache_fill_stall(const cpp_int & _val) { 
    // icache_fill_stall
    int_var__icache_fill_stall = _val.convert_to< icache_fill_stall_cpp_int_t >();
}

cpp_int cap_mpu_csr_CNT_stall_t::icache_fill_stall() const {
    return int_var__icache_fill_stall.convert_to< cpp_int >();
}
    
void cap_mpu_csr_mpu_run_t::start_pulse(const cpp_int & _val) { 
    // start_pulse
    int_var__start_pulse = _val.convert_to< start_pulse_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_run_t::start_pulse() const {
    return int_var__start_pulse.convert_to< cpp_int >();
}
    
void cap_mpu_csr_mpu_run_t::stop_pulse(const cpp_int & _val) { 
    // stop_pulse
    int_var__stop_pulse = _val.convert_to< stop_pulse_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_run_t::stop_pulse() const {
    return int_var__stop_pulse.convert_to< cpp_int >();
}
    
void cap_mpu_csr_mpu_run_t::step_pulse(const cpp_int & _val) { 
    // step_pulse
    int_var__step_pulse = _val.convert_to< step_pulse_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_run_t::step_pulse() const {
    return int_var__step_pulse.convert_to< cpp_int >();
}
    
void cap_mpu_csr_mpu_ctl_t::enable_stop(const cpp_int & _val) { 
    // enable_stop
    int_var__enable_stop = _val.convert_to< enable_stop_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_ctl_t::enable_stop() const {
    return int_var__enable_stop.convert_to< cpp_int >();
}
    
void cap_mpu_csr_mpu_ctl_t::enable_illegal_stop(const cpp_int & _val) { 
    // enable_illegal_stop
    int_var__enable_illegal_stop = _val.convert_to< enable_illegal_stop_cpp_int_t >();
}

cpp_int cap_mpu_csr_mpu_ctl_t::enable_illegal_stop() const {
    return int_var__enable_illegal_stop.convert_to< cpp_int >();
}
    
void cap_mpu_csr_count_stage_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_count_stage_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_mpu_csr_count_stage_t::debug(const cpp_int & _val) { 
    // debug
    int_var__debug = _val.convert_to< debug_cpp_int_t >();
}

cpp_int cap_mpu_csr_count_stage_t::debug() const {
    return int_var__debug.convert_to< cpp_int >();
}
    
void cap_mpu_csr_axi_attr_t::arcache(const cpp_int & _val) { 
    // arcache
    int_var__arcache = _val.convert_to< arcache_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::arcache() const {
    return int_var__arcache.convert_to< cpp_int >();
}
    
void cap_mpu_csr_axi_attr_t::awcache(const cpp_int & _val) { 
    // awcache
    int_var__awcache = _val.convert_to< awcache_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::awcache() const {
    return int_var__awcache.convert_to< cpp_int >();
}
    
void cap_mpu_csr_axi_attr_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::prot() const {
    return int_var__prot.convert_to< cpp_int >();
}
    
void cap_mpu_csr_axi_attr_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::qos() const {
    return int_var__qos.convert_to< cpp_int >();
}
    
void cap_mpu_csr_axi_attr_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_mpu_csr_axi_attr_t::lock() const {
    return int_var__lock.convert_to< cpp_int >();
}
    
void cap_mpu_csr_icache_t::invalidate(const cpp_int & _val) { 
    // invalidate
    int_var__invalidate = _val.convert_to< invalidate_cpp_int_t >();
}

cpp_int cap_mpu_csr_icache_t::invalidate() const {
    return int_var__invalidate.convert_to< cpp_int >();
}
    
void cap_mpu_csr_trace_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_mpu_csr_trace_t::phv_debug(const cpp_int & _val) { 
    // phv_debug
    int_var__phv_debug = _val.convert_to< phv_debug_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::phv_debug() const {
    return int_var__phv_debug.convert_to< cpp_int >();
}
    
void cap_mpu_csr_trace_t::wrap(const cpp_int & _val) { 
    // wrap
    int_var__wrap = _val.convert_to< wrap_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::wrap() const {
    return int_var__wrap.convert_to< cpp_int >();
}
    
void cap_mpu_csr_trace_t::rst(const cpp_int & _val) { 
    // rst
    int_var__rst = _val.convert_to< rst_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::rst() const {
    return int_var__rst.convert_to< cpp_int >();
}
    
void cap_mpu_csr_trace_t::base_addr(const cpp_int & _val) { 
    // base_addr
    int_var__base_addr = _val.convert_to< base_addr_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::base_addr() const {
    return int_var__base_addr.convert_to< cpp_int >();
}
    
void cap_mpu_csr_trace_t::buf_size(const cpp_int & _val) { 
    // buf_size
    int_var__buf_size = _val.convert_to< buf_size_cpp_int_t >();
}

cpp_int cap_mpu_csr_trace_t::buf_size() const {
    return int_var__buf_size.convert_to< cpp_int >();
}
    