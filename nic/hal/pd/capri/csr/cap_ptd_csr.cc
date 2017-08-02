
#include "cap_ptd_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_ptd_csr_cfg_spare_dbg_t::cap_ptd_csr_cfg_spare_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_cfg_spare_dbg_t::~cap_ptd_csr_cfg_spare_dbg_t() { }

cap_ptd_csr_cfg_xoff_t::cap_ptd_csr_cfg_xoff_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_cfg_xoff_t::~cap_ptd_csr_cfg_xoff_t() { }

cap_ptd_csr_axi_attr_t::cap_ptd_csr_axi_attr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_axi_attr_t::~cap_ptd_csr_axi_attr_t() { }

cap_ptd_csr_sta_id_t::cap_ptd_csr_sta_id_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_sta_id_t::~cap_ptd_csr_sta_id_t() { }

cap_ptd_csr_cfg_ctrl_t::cap_ptd_csr_cfg_ctrl_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_cfg_ctrl_t::~cap_ptd_csr_cfg_ctrl_t() { }

cap_ptd_csr_cfg_debug_port_t::cap_ptd_csr_cfg_debug_port_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_cfg_debug_port_t::~cap_ptd_csr_cfg_debug_port_t() { }

cap_ptd_csr_base_t::cap_ptd_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ptd_csr_base_t::~cap_ptd_csr_base_t() { }

cap_ptd_csr_t::cap_ptd_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(128);
        set_attributes(0,get_name(), 0);
        }
cap_ptd_csr_t::~cap_ptd_csr_t() { }

void cap_ptd_csr_cfg_spare_dbg_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
}

void cap_ptd_csr_cfg_xoff_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".numphv_thresh: 0x" << int_var__numphv_thresh << dec << endl)
}

void cap_ptd_csr_axi_attr_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".arcache_0: 0x" << int_var__arcache_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".awcache_0: 0x" << int_var__awcache_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".arcache_1: 0x" << int_var__arcache_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".awcache_1: 0x" << int_var__awcache_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".arqos: 0x" << int_var__arqos << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".awqos_0: 0x" << int_var__awqos_0 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".awqos_1: 0x" << int_var__awqos_1 << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl)
}

void cap_ptd_csr_sta_id_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".wr_pend_cnt: 0x" << int_var__wr_pend_cnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rd_pend_cnt: 0x" << int_var__rd_pend_cnt << dec << endl)
}

void cap_ptd_csr_cfg_ctrl_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".max_rd_req_cnt: 0x" << int_var__max_rd_req_cnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".max_wr_req_cnt: 0x" << int_var__max_wr_req_cnt << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".wdata_ff_thresh: 0x" << int_var__wdata_ff_thresh << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_ff_thresh: 0x" << int_var__pkt_ff_thresh << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".mem_ff_thresh: 0x" << int_var__mem_ff_thresh << dec << endl)
}

void cap_ptd_csr_cfg_debug_port_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".enable: 0x" << int_var__enable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".select: 0x" << int_var__select << dec << endl)
}

void cap_ptd_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_ptd_csr_t::show() {

    base.show();
    cfg_debug_port.show();
    cfg_ctrl.show();
    sta_id.show();
    axi_attr.show();
    cfg_xoff.show();
    cfg_spare_dbg.show();
}

int cap_ptd_csr_cfg_spare_dbg_t::get_width() const {
    return cap_ptd_csr_cfg_spare_dbg_t::s_get_width();

}

int cap_ptd_csr_cfg_xoff_t::get_width() const {
    return cap_ptd_csr_cfg_xoff_t::s_get_width();

}

int cap_ptd_csr_axi_attr_t::get_width() const {
    return cap_ptd_csr_axi_attr_t::s_get_width();

}

int cap_ptd_csr_sta_id_t::get_width() const {
    return cap_ptd_csr_sta_id_t::s_get_width();

}

int cap_ptd_csr_cfg_ctrl_t::get_width() const {
    return cap_ptd_csr_cfg_ctrl_t::s_get_width();

}

int cap_ptd_csr_cfg_debug_port_t::get_width() const {
    return cap_ptd_csr_cfg_debug_port_t::s_get_width();

}

int cap_ptd_csr_base_t::get_width() const {
    return cap_ptd_csr_base_t::s_get_width();

}

int cap_ptd_csr_t::get_width() const {
    return cap_ptd_csr_t::s_get_width();

}

int cap_ptd_csr_cfg_spare_dbg_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_ptd_csr_cfg_xoff_t::s_get_width() {
    int _count = 0;

    _count += 11; // numphv_thresh
    return _count;
}

int cap_ptd_csr_axi_attr_t::s_get_width() {
    int _count = 0;

    _count += 4; // arcache_0
    _count += 4; // awcache_0
    _count += 4; // arcache_1
    _count += 4; // awcache_1
    _count += 3; // prot
    _count += 4; // arqos
    _count += 4; // awqos_0
    _count += 4; // awqos_1
    _count += 1; // lock
    return _count;
}

int cap_ptd_csr_sta_id_t::s_get_width() {
    int _count = 0;

    _count += 8; // wr_pend_cnt
    _count += 8; // rd_pend_cnt
    return _count;
}

int cap_ptd_csr_cfg_ctrl_t::s_get_width() {
    int _count = 0;

    _count += 8; // max_rd_req_cnt
    _count += 8; // max_wr_req_cnt
    _count += 9; // wdata_ff_thresh
    _count += 4; // pkt_ff_thresh
    _count += 4; // mem_ff_thresh
    return _count;
}

int cap_ptd_csr_cfg_debug_port_t::s_get_width() {
    int _count = 0;

    _count += 1; // enable
    _count += 1; // select
    return _count;
}

int cap_ptd_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_ptd_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_ptd_csr_base_t::s_get_width(); // base
    _count += cap_ptd_csr_cfg_debug_port_t::s_get_width(); // cfg_debug_port
    _count += cap_ptd_csr_cfg_ctrl_t::s_get_width(); // cfg_ctrl
    _count += cap_ptd_csr_sta_id_t::s_get_width(); // sta_id
    _count += cap_ptd_csr_axi_attr_t::s_get_width(); // axi_attr
    _count += cap_ptd_csr_cfg_xoff_t::s_get_width(); // cfg_xoff
    _count += cap_ptd_csr_cfg_spare_dbg_t::s_get_width(); // cfg_spare_dbg
    return _count;
}

void cap_ptd_csr_cfg_spare_dbg_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< data_cpp_int_t >()  ;
    _count += 32;
}

void cap_ptd_csr_cfg_xoff_t::all(const cpp_int & _val) {
    int _count = 0;

    // numphv_thresh
    int_var__numphv_thresh = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< numphv_thresh_cpp_int_t >()  ;
    _count += 11;
}

void cap_ptd_csr_axi_attr_t::all(const cpp_int & _val) {
    int _count = 0;

    // arcache_0
    int_var__arcache_0 = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< arcache_0_cpp_int_t >()  ;
    _count += 4;
    // awcache_0
    int_var__awcache_0 = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< awcache_0_cpp_int_t >()  ;
    _count += 4;
    // arcache_1
    int_var__arcache_1 = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< arcache_1_cpp_int_t >()  ;
    _count += 4;
    // awcache_1
    int_var__awcache_1 = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< awcache_1_cpp_int_t >()  ;
    _count += 4;
    // prot
    int_var__prot = hlp.get_slc(_val, _count, _count -1 + 3 ).convert_to< prot_cpp_int_t >()  ;
    _count += 3;
    // arqos
    int_var__arqos = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< arqos_cpp_int_t >()  ;
    _count += 4;
    // awqos_0
    int_var__awqos_0 = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< awqos_0_cpp_int_t >()  ;
    _count += 4;
    // awqos_1
    int_var__awqos_1 = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< awqos_1_cpp_int_t >()  ;
    _count += 4;
    // lock
    int_var__lock = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< lock_cpp_int_t >()  ;
    _count += 1;
}

void cap_ptd_csr_sta_id_t::all(const cpp_int & _val) {
    int _count = 0;

    // wr_pend_cnt
    int_var__wr_pend_cnt = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< wr_pend_cnt_cpp_int_t >()  ;
    _count += 8;
    // rd_pend_cnt
    int_var__rd_pend_cnt = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< rd_pend_cnt_cpp_int_t >()  ;
    _count += 8;
}

void cap_ptd_csr_cfg_ctrl_t::all(const cpp_int & _val) {
    int _count = 0;

    // max_rd_req_cnt
    int_var__max_rd_req_cnt = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< max_rd_req_cnt_cpp_int_t >()  ;
    _count += 8;
    // max_wr_req_cnt
    int_var__max_wr_req_cnt = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< max_wr_req_cnt_cpp_int_t >()  ;
    _count += 8;
    // wdata_ff_thresh
    int_var__wdata_ff_thresh = hlp.get_slc(_val, _count, _count -1 + 9 ).convert_to< wdata_ff_thresh_cpp_int_t >()  ;
    _count += 9;
    // pkt_ff_thresh
    int_var__pkt_ff_thresh = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< pkt_ff_thresh_cpp_int_t >()  ;
    _count += 4;
    // mem_ff_thresh
    int_var__mem_ff_thresh = hlp.get_slc(_val, _count, _count -1 + 4 ).convert_to< mem_ff_thresh_cpp_int_t >()  ;
    _count += 4;
}

void cap_ptd_csr_cfg_debug_port_t::all(const cpp_int & _val) {
    int _count = 0;

    // enable
    int_var__enable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< enable_cpp_int_t >()  ;
    _count += 1;
    // select
    int_var__select = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< select_cpp_int_t >()  ;
    _count += 1;
}

void cap_ptd_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_ptd_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    cfg_debug_port.all( hlp.get_slc(_val, _count, _count -1 + cfg_debug_port.get_width() )); // cfg_debug_port
    _count += cfg_debug_port.get_width();
    cfg_ctrl.all( hlp.get_slc(_val, _count, _count -1 + cfg_ctrl.get_width() )); // cfg_ctrl
    _count += cfg_ctrl.get_width();
    sta_id.all( hlp.get_slc(_val, _count, _count -1 + sta_id.get_width() )); // sta_id
    _count += sta_id.get_width();
    axi_attr.all( hlp.get_slc(_val, _count, _count -1 + axi_attr.get_width() )); // axi_attr
    _count += axi_attr.get_width();
    cfg_xoff.all( hlp.get_slc(_val, _count, _count -1 + cfg_xoff.get_width() )); // cfg_xoff
    _count += cfg_xoff.get_width();
    cfg_spare_dbg.all( hlp.get_slc(_val, _count, _count -1 + cfg_spare_dbg.get_width() )); // cfg_spare_dbg
    _count += cfg_spare_dbg.get_width();
}

cpp_int cap_ptd_csr_cfg_spare_dbg_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_ptd_csr_cfg_xoff_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // numphv_thresh
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__numphv_thresh) , _count, _count -1 + 11 );
    _count += 11;
    return ret_val;
}

cpp_int cap_ptd_csr_axi_attr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // arcache_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__arcache_0) , _count, _count -1 + 4 );
    _count += 4;
    // awcache_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__awcache_0) , _count, _count -1 + 4 );
    _count += 4;
    // arcache_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__arcache_1) , _count, _count -1 + 4 );
    _count += 4;
    // awcache_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__awcache_1) , _count, _count -1 + 4 );
    _count += 4;
    // prot
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__prot) , _count, _count -1 + 3 );
    _count += 3;
    // arqos
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__arqos) , _count, _count -1 + 4 );
    _count += 4;
    // awqos_0
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__awqos_0) , _count, _count -1 + 4 );
    _count += 4;
    // awqos_1
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__awqos_1) , _count, _count -1 + 4 );
    _count += 4;
    // lock
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__lock) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_ptd_csr_sta_id_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // wr_pend_cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__wr_pend_cnt) , _count, _count -1 + 8 );
    _count += 8;
    // rd_pend_cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rd_pend_cnt) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_ptd_csr_cfg_ctrl_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // max_rd_req_cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__max_rd_req_cnt) , _count, _count -1 + 8 );
    _count += 8;
    // max_wr_req_cnt
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__max_wr_req_cnt) , _count, _count -1 + 8 );
    _count += 8;
    // wdata_ff_thresh
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__wdata_ff_thresh) , _count, _count -1 + 9 );
    _count += 9;
    // pkt_ff_thresh
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pkt_ff_thresh) , _count, _count -1 + 4 );
    _count += 4;
    // mem_ff_thresh
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__mem_ff_thresh) , _count, _count -1 + 4 );
    _count += 4;
    return ret_val;
}

cpp_int cap_ptd_csr_cfg_debug_port_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // enable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__enable) , _count, _count -1 + 1 );
    _count += 1;
    // select
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__select) , _count, _count -1 + 1 );
    _count += 1;
    return ret_val;
}

cpp_int cap_ptd_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_ptd_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_debug_port.all() , _count, _count -1 + cfg_debug_port.get_width() ); // cfg_debug_port
    _count += cfg_debug_port.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_ctrl.all() , _count, _count -1 + cfg_ctrl.get_width() ); // cfg_ctrl
    _count += cfg_ctrl.get_width();
    ret_val = hlp.set_slc(ret_val, sta_id.all() , _count, _count -1 + sta_id.get_width() ); // sta_id
    _count += sta_id.get_width();
    ret_val = hlp.set_slc(ret_val, axi_attr.all() , _count, _count -1 + axi_attr.get_width() ); // axi_attr
    _count += axi_attr.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_xoff.all() , _count, _count -1 + cfg_xoff.get_width() ); // cfg_xoff
    _count += cfg_xoff.get_width();
    ret_val = hlp.set_slc(ret_val, cfg_spare_dbg.all() , _count, _count -1 + cfg_spare_dbg.get_width() ); // cfg_spare_dbg
    _count += cfg_spare_dbg.get_width();
    return ret_val;
}

void cap_ptd_csr_cfg_spare_dbg_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_spare_dbg_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_spare_dbg_t::data);
        }
        #endif
    
    set_reset_val(cpp_int("0xdeadbeef"));
    all(get_reset_val());
}

void cap_ptd_csr_cfg_xoff_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("numphv_thresh", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_xoff_t::numphv_thresh);
            register_get_func("numphv_thresh", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_xoff_t::numphv_thresh);
        }
        #endif
    
    set_reset_val(cpp_int("0x400"));
    all(get_reset_val());
}

void cap_ptd_csr_axi_attr_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("arcache_0", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::arcache_0);
            register_get_func("arcache_0", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::arcache_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("awcache_0", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::awcache_0);
            register_get_func("awcache_0", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::awcache_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("arcache_1", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::arcache_1);
            register_get_func("arcache_1", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::arcache_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("awcache_1", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::awcache_1);
            register_get_func("awcache_1", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::awcache_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("prot", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::prot);
            register_get_func("prot", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::prot);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("arqos", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::arqos);
            register_get_func("arqos", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::arqos);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("awqos_0", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::awqos_0);
            register_get_func("awqos_0", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::awqos_0);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("awqos_1", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::awqos_1);
            register_get_func("awqos_1", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::awqos_1);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("lock", (cap_csr_base::set_function_type_t)&cap_ptd_csr_axi_attr_t::lock);
            register_get_func("lock", (cap_csr_base::get_function_type_t)&cap_ptd_csr_axi_attr_t::lock);
        }
        #endif
    
    set_reset_val(cpp_int("0xff00"));
    all(get_reset_val());
}

void cap_ptd_csr_sta_id_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("wr_pend_cnt", (cap_csr_base::set_function_type_t)&cap_ptd_csr_sta_id_t::wr_pend_cnt);
            register_get_func("wr_pend_cnt", (cap_csr_base::get_function_type_t)&cap_ptd_csr_sta_id_t::wr_pend_cnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rd_pend_cnt", (cap_csr_base::set_function_type_t)&cap_ptd_csr_sta_id_t::rd_pend_cnt);
            register_get_func("rd_pend_cnt", (cap_csr_base::get_function_type_t)&cap_ptd_csr_sta_id_t::rd_pend_cnt);
        }
        #endif
    
}

void cap_ptd_csr_cfg_ctrl_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_rd_req_cnt", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_ctrl_t::max_rd_req_cnt);
            register_get_func("max_rd_req_cnt", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_ctrl_t::max_rd_req_cnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("max_wr_req_cnt", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_ctrl_t::max_wr_req_cnt);
            register_get_func("max_wr_req_cnt", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_ctrl_t::max_wr_req_cnt);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("wdata_ff_thresh", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_ctrl_t::wdata_ff_thresh);
            register_get_func("wdata_ff_thresh", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_ctrl_t::wdata_ff_thresh);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_ff_thresh", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_ctrl_t::pkt_ff_thresh);
            register_get_func("pkt_ff_thresh", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_ctrl_t::pkt_ff_thresh);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("mem_ff_thresh", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_ctrl_t::mem_ff_thresh);
            register_get_func("mem_ff_thresh", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_ctrl_t::mem_ff_thresh);
        }
        #endif
    
    set_reset_val(cpp_int("0x88fc8080"));
    all(get_reset_val());
}

void cap_ptd_csr_cfg_debug_port_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("enable", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_debug_port_t::enable);
            register_get_func("enable", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_debug_port_t::enable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("select", (cap_csr_base::set_function_type_t)&cap_ptd_csr_cfg_debug_port_t::select);
            register_get_func("select", (cap_csr_base::get_function_type_t)&cap_ptd_csr_cfg_debug_port_t::select);
        }
        #endif
    
}

void cap_ptd_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_ptd_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_ptd_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_ptd_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_debug_port.set_attributes(this,"cfg_debug_port", 0x40 );
    cfg_ctrl.set_attributes(this,"cfg_ctrl", 0x48 );
    sta_id.set_attributes(this,"sta_id", 0x50 );
    axi_attr.set_attributes(this,"axi_attr", 0x54 );
    cfg_xoff.set_attributes(this,"cfg_xoff", 0x58 );
    cfg_spare_dbg.set_attributes(this,"cfg_spare_dbg", 0x5c );
}

void cap_ptd_csr_cfg_spare_dbg_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_spare_dbg_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_xoff_t::numphv_thresh(const cpp_int & _val) { 
    // numphv_thresh
    int_var__numphv_thresh = _val.convert_to< numphv_thresh_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_xoff_t::numphv_thresh() const {
    return int_var__numphv_thresh.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::arcache_0(const cpp_int & _val) { 
    // arcache_0
    int_var__arcache_0 = _val.convert_to< arcache_0_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::arcache_0() const {
    return int_var__arcache_0.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::awcache_0(const cpp_int & _val) { 
    // awcache_0
    int_var__awcache_0 = _val.convert_to< awcache_0_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::awcache_0() const {
    return int_var__awcache_0.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::arcache_1(const cpp_int & _val) { 
    // arcache_1
    int_var__arcache_1 = _val.convert_to< arcache_1_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::arcache_1() const {
    return int_var__arcache_1.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::awcache_1(const cpp_int & _val) { 
    // awcache_1
    int_var__awcache_1 = _val.convert_to< awcache_1_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::awcache_1() const {
    return int_var__awcache_1.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::prot() const {
    return int_var__prot.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::arqos(const cpp_int & _val) { 
    // arqos
    int_var__arqos = _val.convert_to< arqos_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::arqos() const {
    return int_var__arqos.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::awqos_0(const cpp_int & _val) { 
    // awqos_0
    int_var__awqos_0 = _val.convert_to< awqos_0_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::awqos_0() const {
    return int_var__awqos_0.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::awqos_1(const cpp_int & _val) { 
    // awqos_1
    int_var__awqos_1 = _val.convert_to< awqos_1_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::awqos_1() const {
    return int_var__awqos_1.convert_to< cpp_int >();
}
    
void cap_ptd_csr_axi_attr_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_ptd_csr_axi_attr_t::lock() const {
    return int_var__lock.convert_to< cpp_int >();
}
    
void cap_ptd_csr_sta_id_t::wr_pend_cnt(const cpp_int & _val) { 
    // wr_pend_cnt
    int_var__wr_pend_cnt = _val.convert_to< wr_pend_cnt_cpp_int_t >();
}

cpp_int cap_ptd_csr_sta_id_t::wr_pend_cnt() const {
    return int_var__wr_pend_cnt.convert_to< cpp_int >();
}
    
void cap_ptd_csr_sta_id_t::rd_pend_cnt(const cpp_int & _val) { 
    // rd_pend_cnt
    int_var__rd_pend_cnt = _val.convert_to< rd_pend_cnt_cpp_int_t >();
}

cpp_int cap_ptd_csr_sta_id_t::rd_pend_cnt() const {
    return int_var__rd_pend_cnt.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_ctrl_t::max_rd_req_cnt(const cpp_int & _val) { 
    // max_rd_req_cnt
    int_var__max_rd_req_cnt = _val.convert_to< max_rd_req_cnt_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_ctrl_t::max_rd_req_cnt() const {
    return int_var__max_rd_req_cnt.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_ctrl_t::max_wr_req_cnt(const cpp_int & _val) { 
    // max_wr_req_cnt
    int_var__max_wr_req_cnt = _val.convert_to< max_wr_req_cnt_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_ctrl_t::max_wr_req_cnt() const {
    return int_var__max_wr_req_cnt.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_ctrl_t::wdata_ff_thresh(const cpp_int & _val) { 
    // wdata_ff_thresh
    int_var__wdata_ff_thresh = _val.convert_to< wdata_ff_thresh_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_ctrl_t::wdata_ff_thresh() const {
    return int_var__wdata_ff_thresh.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_ctrl_t::pkt_ff_thresh(const cpp_int & _val) { 
    // pkt_ff_thresh
    int_var__pkt_ff_thresh = _val.convert_to< pkt_ff_thresh_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_ctrl_t::pkt_ff_thresh() const {
    return int_var__pkt_ff_thresh.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_ctrl_t::mem_ff_thresh(const cpp_int & _val) { 
    // mem_ff_thresh
    int_var__mem_ff_thresh = _val.convert_to< mem_ff_thresh_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_ctrl_t::mem_ff_thresh() const {
    return int_var__mem_ff_thresh.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_debug_port_t::enable(const cpp_int & _val) { 
    // enable
    int_var__enable = _val.convert_to< enable_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_debug_port_t::enable() const {
    return int_var__enable.convert_to< cpp_int >();
}
    
void cap_ptd_csr_cfg_debug_port_t::select(const cpp_int & _val) { 
    // select
    int_var__select = _val.convert_to< select_cpp_int_t >();
}

cpp_int cap_ptd_csr_cfg_debug_port_t::select() const {
    return int_var__select.convert_to< cpp_int >();
}
    
void cap_ptd_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_ptd_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    