
#include "cap_dprstats_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dprstats_csr_CNT_ecc_err_t::cap_dprstats_csr_CNT_ecc_err_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprstats_csr_CNT_ecc_err_t::~cap_dprstats_csr_CNT_ecc_err_t() { }

cap_dprstats_csr_CNT_drop_t::cap_dprstats_csr_CNT_drop_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprstats_csr_CNT_drop_t::~cap_dprstats_csr_CNT_drop_t() { }

cap_dprstats_csr_CNT_interface_t::cap_dprstats_csr_CNT_interface_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprstats_csr_CNT_interface_t::~cap_dprstats_csr_CNT_interface_t() { }

cap_dprstats_csr_cfg_spare_t::cap_dprstats_csr_cfg_spare_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprstats_csr_cfg_spare_t::~cap_dprstats_csr_cfg_spare_t() { }

cap_dprstats_csr_cfg_capture_t::cap_dprstats_csr_cfg_capture_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprstats_csr_cfg_capture_t::~cap_dprstats_csr_cfg_capture_t() { }

cap_dprstats_csr_t::cap_dprstats_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(16384);
        set_attributes(0,get_name(), 0);
        }
cap_dprstats_csr_t::~cap_dprstats_csr_t() { }

void cap_dprstats_csr_CNT_ecc_err_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_drop: 0x" << int_var__pkt_drop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".byte_drop: 0x" << int_var__byte_drop << dec << endl)
}

void cap_dprstats_csr_CNT_drop_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_drop: 0x" << int_var__pkt_drop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".byte_drop: 0x" << int_var__byte_drop << dec << endl)
}

void cap_dprstats_csr_CNT_interface_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".dpp_phv_sop: 0x" << int_var__dpp_phv_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dpp_phv_eop: 0x" << int_var__dpp_phv_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dpp_ohi_sop: 0x" << int_var__dpp_ohi_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dpp_ohi_eop: 0x" << int_var__dpp_ohi_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dpp_csum_crc_vld: 0x" << int_var__dpp_csum_crc_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".dpp_frame_vld: 0x" << int_var__dpp_frame_vld << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pa_pkt_sop: 0x" << int_var__pa_pkt_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pa_pkt_eop: 0x" << int_var__pa_pkt_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pb_pkt_sop: 0x" << int_var__pb_pkt_sop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pb_pkt_eop: 0x" << int_var__pb_pkt_eop << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_drop: 0x" << int_var__pkt_drop << dec << endl)
}

void cap_dprstats_csr_cfg_spare_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
}

void cap_dprstats_csr_cfg_capture_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".capture_en: 0x" << int_var__capture_en << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".capture_arm: 0x" << int_var__capture_arm << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl)
}

void cap_dprstats_csr_t::show() {

    cfg_capture.show();
    for(int ii = 0; ii < 1024; ii++) {
        cfg_spare[ii].show();
    }
    CNT_interface.show();
    CNT_drop.show();
    CNT_ecc_err.show();
}

int cap_dprstats_csr_CNT_ecc_err_t::get_width() const {
    return cap_dprstats_csr_CNT_ecc_err_t::s_get_width();

}

int cap_dprstats_csr_CNT_drop_t::get_width() const {
    return cap_dprstats_csr_CNT_drop_t::s_get_width();

}

int cap_dprstats_csr_CNT_interface_t::get_width() const {
    return cap_dprstats_csr_CNT_interface_t::s_get_width();

}

int cap_dprstats_csr_cfg_spare_t::get_width() const {
    return cap_dprstats_csr_cfg_spare_t::s_get_width();

}

int cap_dprstats_csr_cfg_capture_t::get_width() const {
    return cap_dprstats_csr_cfg_capture_t::s_get_width();

}

int cap_dprstats_csr_t::get_width() const {
    return cap_dprstats_csr_t::s_get_width();

}

int cap_dprstats_csr_CNT_ecc_err_t::s_get_width() {
    int _count = 0;

    _count += 32; // pkt_drop
    _count += 40; // byte_drop
    return _count;
}

int cap_dprstats_csr_CNT_drop_t::s_get_width() {
    int _count = 0;

    _count += 32; // pkt_drop
    _count += 40; // byte_drop
    return _count;
}

int cap_dprstats_csr_CNT_interface_t::s_get_width() {
    int _count = 0;

    _count += 32; // dpp_phv_sop
    _count += 32; // dpp_phv_eop
    _count += 32; // dpp_ohi_sop
    _count += 32; // dpp_ohi_eop
    _count += 32; // dpp_csum_crc_vld
    _count += 32; // dpp_frame_vld
    _count += 32; // pa_pkt_sop
    _count += 32; // pa_pkt_eop
    _count += 32; // pb_pkt_sop
    _count += 32; // pb_pkt_eop
    _count += 32; // pkt_drop
    return _count;
}

int cap_dprstats_csr_cfg_spare_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_dprstats_csr_cfg_capture_t::s_get_width() {
    int _count = 0;

    _count += 1; // capture_en
    _count += 1; // capture_arm
    _count += 30; // rsvd
    return _count;
}

int cap_dprstats_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_dprstats_csr_cfg_capture_t::s_get_width(); // cfg_capture
    _count += (cap_dprstats_csr_cfg_spare_t::s_get_width() * 1024); // cfg_spare
    _count += cap_dprstats_csr_CNT_interface_t::s_get_width(); // CNT_interface
    _count += cap_dprstats_csr_CNT_drop_t::s_get_width(); // CNT_drop
    _count += cap_dprstats_csr_CNT_ecc_err_t::s_get_width(); // CNT_ecc_err
    return _count;
}

void cap_dprstats_csr_CNT_ecc_err_t::all(const cpp_int & _val) {
    int _count = 0;

    // pkt_drop
    int_var__pkt_drop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pkt_drop_cpp_int_t >()  ;
    _count += 32;
    // byte_drop
    int_var__byte_drop = hlp.get_slc(_val, _count, _count -1 + 40 ).convert_to< byte_drop_cpp_int_t >()  ;
    _count += 40;
}

void cap_dprstats_csr_CNT_drop_t::all(const cpp_int & _val) {
    int _count = 0;

    // pkt_drop
    int_var__pkt_drop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pkt_drop_cpp_int_t >()  ;
    _count += 32;
    // byte_drop
    int_var__byte_drop = hlp.get_slc(_val, _count, _count -1 + 40 ).convert_to< byte_drop_cpp_int_t >()  ;
    _count += 40;
}

void cap_dprstats_csr_CNT_interface_t::all(const cpp_int & _val) {
    int _count = 0;

    // dpp_phv_sop
    int_var__dpp_phv_sop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< dpp_phv_sop_cpp_int_t >()  ;
    _count += 32;
    // dpp_phv_eop
    int_var__dpp_phv_eop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< dpp_phv_eop_cpp_int_t >()  ;
    _count += 32;
    // dpp_ohi_sop
    int_var__dpp_ohi_sop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< dpp_ohi_sop_cpp_int_t >()  ;
    _count += 32;
    // dpp_ohi_eop
    int_var__dpp_ohi_eop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< dpp_ohi_eop_cpp_int_t >()  ;
    _count += 32;
    // dpp_csum_crc_vld
    int_var__dpp_csum_crc_vld = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< dpp_csum_crc_vld_cpp_int_t >()  ;
    _count += 32;
    // dpp_frame_vld
    int_var__dpp_frame_vld = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< dpp_frame_vld_cpp_int_t >()  ;
    _count += 32;
    // pa_pkt_sop
    int_var__pa_pkt_sop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pa_pkt_sop_cpp_int_t >()  ;
    _count += 32;
    // pa_pkt_eop
    int_var__pa_pkt_eop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pa_pkt_eop_cpp_int_t >()  ;
    _count += 32;
    // pb_pkt_sop
    int_var__pb_pkt_sop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pb_pkt_sop_cpp_int_t >()  ;
    _count += 32;
    // pb_pkt_eop
    int_var__pb_pkt_eop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pb_pkt_eop_cpp_int_t >()  ;
    _count += 32;
    // pkt_drop
    int_var__pkt_drop = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< pkt_drop_cpp_int_t >()  ;
    _count += 32;
}

void cap_dprstats_csr_cfg_spare_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< data_cpp_int_t >()  ;
    _count += 32;
}

void cap_dprstats_csr_cfg_capture_t::all(const cpp_int & _val) {
    int _count = 0;

    // capture_en
    int_var__capture_en = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< capture_en_cpp_int_t >()  ;
    _count += 1;
    // capture_arm
    int_var__capture_arm = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< capture_arm_cpp_int_t >()  ;
    _count += 1;
    // rsvd
    int_var__rsvd = hlp.get_slc(_val, _count, _count -1 + 30 ).convert_to< rsvd_cpp_int_t >()  ;
    _count += 30;
}

void cap_dprstats_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    cfg_capture.all( hlp.get_slc(_val, _count, _count -1 + cfg_capture.get_width() )); // cfg_capture
    _count += cfg_capture.get_width();
    // cfg_spare
    for(int ii = 0; ii < 1024; ii++) {
        cfg_spare[ii].all( hlp.get_slc(_val, _count, _count -1 + cfg_spare[ii].get_width()));
        _count += cfg_spare[ii].get_width();
    }
    CNT_interface.all( hlp.get_slc(_val, _count, _count -1 + CNT_interface.get_width() )); // CNT_interface
    _count += CNT_interface.get_width();
    CNT_drop.all( hlp.get_slc(_val, _count, _count -1 + CNT_drop.get_width() )); // CNT_drop
    _count += CNT_drop.get_width();
    CNT_ecc_err.all( hlp.get_slc(_val, _count, _count -1 + CNT_ecc_err.get_width() )); // CNT_ecc_err
    _count += CNT_ecc_err.get_width();
}

cpp_int cap_dprstats_csr_CNT_ecc_err_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // pkt_drop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pkt_drop) , _count, _count -1 + 32 );
    _count += 32;
    // byte_drop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__byte_drop) , _count, _count -1 + 40 );
    _count += 40;
    return ret_val;
}

cpp_int cap_dprstats_csr_CNT_drop_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // pkt_drop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pkt_drop) , _count, _count -1 + 32 );
    _count += 32;
    // byte_drop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__byte_drop) , _count, _count -1 + 40 );
    _count += 40;
    return ret_val;
}

cpp_int cap_dprstats_csr_CNT_interface_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // dpp_phv_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dpp_phv_sop) , _count, _count -1 + 32 );
    _count += 32;
    // dpp_phv_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dpp_phv_eop) , _count, _count -1 + 32 );
    _count += 32;
    // dpp_ohi_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dpp_ohi_sop) , _count, _count -1 + 32 );
    _count += 32;
    // dpp_ohi_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dpp_ohi_eop) , _count, _count -1 + 32 );
    _count += 32;
    // dpp_csum_crc_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dpp_csum_crc_vld) , _count, _count -1 + 32 );
    _count += 32;
    // dpp_frame_vld
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__dpp_frame_vld) , _count, _count -1 + 32 );
    _count += 32;
    // pa_pkt_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pa_pkt_sop) , _count, _count -1 + 32 );
    _count += 32;
    // pa_pkt_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pa_pkt_eop) , _count, _count -1 + 32 );
    _count += 32;
    // pb_pkt_sop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pb_pkt_sop) , _count, _count -1 + 32 );
    _count += 32;
    // pb_pkt_eop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pb_pkt_eop) , _count, _count -1 + 32 );
    _count += 32;
    // pkt_drop
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__pkt_drop) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dprstats_csr_cfg_spare_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_dprstats_csr_cfg_capture_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // capture_en
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__capture_en) , _count, _count -1 + 1 );
    _count += 1;
    // capture_arm
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__capture_arm) , _count, _count -1 + 1 );
    _count += 1;
    // rsvd
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__rsvd) , _count, _count -1 + 30 );
    _count += 30;
    return ret_val;
}

cpp_int cap_dprstats_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, cfg_capture.all() , _count, _count -1 + cfg_capture.get_width() ); // cfg_capture
    _count += cfg_capture.get_width();
    // cfg_spare
    for(int ii = 0; ii < 1024; ii++) {
         ret_val = hlp.set_slc(ret_val, cfg_spare[ii].all() , _count, _count -1 + cfg_spare[ii].get_width() );
        _count += cfg_spare[ii].get_width();
    }
    ret_val = hlp.set_slc(ret_val, CNT_interface.all() , _count, _count -1 + CNT_interface.get_width() ); // CNT_interface
    _count += CNT_interface.get_width();
    ret_val = hlp.set_slc(ret_val, CNT_drop.all() , _count, _count -1 + CNT_drop.get_width() ); // CNT_drop
    _count += CNT_drop.get_width();
    ret_val = hlp.set_slc(ret_val, CNT_ecc_err.all() , _count, _count -1 + CNT_ecc_err.get_width() ); // CNT_ecc_err
    _count += CNT_ecc_err.get_width();
    return ret_val;
}

void cap_dprstats_csr_CNT_ecc_err_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_drop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_ecc_err_t::pkt_drop);
            register_get_func("pkt_drop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_ecc_err_t::pkt_drop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("byte_drop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_ecc_err_t::byte_drop);
            register_get_func("byte_drop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_ecc_err_t::byte_drop);
        }
        #endif
    
}

void cap_dprstats_csr_CNT_drop_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_drop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_drop_t::pkt_drop);
            register_get_func("pkt_drop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_drop_t::pkt_drop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("byte_drop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_drop_t::byte_drop);
            register_get_func("byte_drop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_drop_t::byte_drop);
        }
        #endif
    
}

void cap_dprstats_csr_CNT_interface_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dpp_phv_sop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_phv_sop);
            register_get_func("dpp_phv_sop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_phv_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dpp_phv_eop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_phv_eop);
            register_get_func("dpp_phv_eop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_phv_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dpp_ohi_sop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_ohi_sop);
            register_get_func("dpp_ohi_sop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_ohi_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dpp_ohi_eop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_ohi_eop);
            register_get_func("dpp_ohi_eop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_ohi_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dpp_csum_crc_vld", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_csum_crc_vld);
            register_get_func("dpp_csum_crc_vld", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_csum_crc_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("dpp_frame_vld", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_frame_vld);
            register_get_func("dpp_frame_vld", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::dpp_frame_vld);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pa_pkt_sop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::pa_pkt_sop);
            register_get_func("pa_pkt_sop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::pa_pkt_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pa_pkt_eop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::pa_pkt_eop);
            register_get_func("pa_pkt_eop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::pa_pkt_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pb_pkt_sop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::pb_pkt_sop);
            register_get_func("pb_pkt_sop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::pb_pkt_sop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pb_pkt_eop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::pb_pkt_eop);
            register_get_func("pb_pkt_eop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::pb_pkt_eop);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("pkt_drop", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_CNT_interface_t::pkt_drop);
            register_get_func("pkt_drop", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_CNT_interface_t::pkt_drop);
        }
        #endif
    
}

void cap_dprstats_csr_cfg_spare_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_cfg_spare_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_cfg_spare_t::data);
        }
        #endif
    
}

void cap_dprstats_csr_cfg_capture_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("capture_en", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_cfg_capture_t::capture_en);
            register_get_func("capture_en", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_cfg_capture_t::capture_en);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("capture_arm", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_cfg_capture_t::capture_arm);
            register_get_func("capture_arm", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_cfg_capture_t::capture_arm);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("rsvd", (cap_csr_base::set_function_type_t)&cap_dprstats_csr_cfg_capture_t::rsvd);
            register_get_func("rsvd", (cap_csr_base::get_function_type_t)&cap_dprstats_csr_cfg_capture_t::rsvd);
        }
        #endif
    
}

void cap_dprstats_csr_t::init() {

    cfg_capture.set_attributes(this,"cfg_capture", 0x0 );
    for(int ii = 0; ii < 1024; ii++) {
        if(ii != 0) cfg_spare[ii].set_field_init_done(true);
        cfg_spare[ii].set_attributes(this,"cfg_spare["+to_string(ii)+"]",  0x1000 + (cfg_spare[ii].get_byte_size()*ii));
    }
    CNT_interface.set_attributes(this,"CNT_interface", 0x2000 );
    CNT_drop.set_attributes(this,"CNT_drop", 0x2040 );
    CNT_ecc_err.set_attributes(this,"CNT_ecc_err", 0x2050 );
}

void cap_dprstats_csr_CNT_ecc_err_t::pkt_drop(const cpp_int & _val) { 
    // pkt_drop
    int_var__pkt_drop = _val.convert_to< pkt_drop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_ecc_err_t::pkt_drop() const {
    return int_var__pkt_drop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_ecc_err_t::byte_drop(const cpp_int & _val) { 
    // byte_drop
    int_var__byte_drop = _val.convert_to< byte_drop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_ecc_err_t::byte_drop() const {
    return int_var__byte_drop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_drop_t::pkt_drop(const cpp_int & _val) { 
    // pkt_drop
    int_var__pkt_drop = _val.convert_to< pkt_drop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_drop_t::pkt_drop() const {
    return int_var__pkt_drop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_drop_t::byte_drop(const cpp_int & _val) { 
    // byte_drop
    int_var__byte_drop = _val.convert_to< byte_drop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_drop_t::byte_drop() const {
    return int_var__byte_drop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::dpp_phv_sop(const cpp_int & _val) { 
    // dpp_phv_sop
    int_var__dpp_phv_sop = _val.convert_to< dpp_phv_sop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::dpp_phv_sop() const {
    return int_var__dpp_phv_sop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::dpp_phv_eop(const cpp_int & _val) { 
    // dpp_phv_eop
    int_var__dpp_phv_eop = _val.convert_to< dpp_phv_eop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::dpp_phv_eop() const {
    return int_var__dpp_phv_eop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::dpp_ohi_sop(const cpp_int & _val) { 
    // dpp_ohi_sop
    int_var__dpp_ohi_sop = _val.convert_to< dpp_ohi_sop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::dpp_ohi_sop() const {
    return int_var__dpp_ohi_sop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::dpp_ohi_eop(const cpp_int & _val) { 
    // dpp_ohi_eop
    int_var__dpp_ohi_eop = _val.convert_to< dpp_ohi_eop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::dpp_ohi_eop() const {
    return int_var__dpp_ohi_eop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::dpp_csum_crc_vld(const cpp_int & _val) { 
    // dpp_csum_crc_vld
    int_var__dpp_csum_crc_vld = _val.convert_to< dpp_csum_crc_vld_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::dpp_csum_crc_vld() const {
    return int_var__dpp_csum_crc_vld.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::dpp_frame_vld(const cpp_int & _val) { 
    // dpp_frame_vld
    int_var__dpp_frame_vld = _val.convert_to< dpp_frame_vld_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::dpp_frame_vld() const {
    return int_var__dpp_frame_vld.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::pa_pkt_sop(const cpp_int & _val) { 
    // pa_pkt_sop
    int_var__pa_pkt_sop = _val.convert_to< pa_pkt_sop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::pa_pkt_sop() const {
    return int_var__pa_pkt_sop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::pa_pkt_eop(const cpp_int & _val) { 
    // pa_pkt_eop
    int_var__pa_pkt_eop = _val.convert_to< pa_pkt_eop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::pa_pkt_eop() const {
    return int_var__pa_pkt_eop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::pb_pkt_sop(const cpp_int & _val) { 
    // pb_pkt_sop
    int_var__pb_pkt_sop = _val.convert_to< pb_pkt_sop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::pb_pkt_sop() const {
    return int_var__pb_pkt_sop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::pb_pkt_eop(const cpp_int & _val) { 
    // pb_pkt_eop
    int_var__pb_pkt_eop = _val.convert_to< pb_pkt_eop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::pb_pkt_eop() const {
    return int_var__pb_pkt_eop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_CNT_interface_t::pkt_drop(const cpp_int & _val) { 
    // pkt_drop
    int_var__pkt_drop = _val.convert_to< pkt_drop_cpp_int_t >();
}

cpp_int cap_dprstats_csr_CNT_interface_t::pkt_drop() const {
    return int_var__pkt_drop.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_cfg_spare_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprstats_csr_cfg_spare_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_cfg_capture_t::capture_en(const cpp_int & _val) { 
    // capture_en
    int_var__capture_en = _val.convert_to< capture_en_cpp_int_t >();
}

cpp_int cap_dprstats_csr_cfg_capture_t::capture_en() const {
    return int_var__capture_en.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_cfg_capture_t::capture_arm(const cpp_int & _val) { 
    // capture_arm
    int_var__capture_arm = _val.convert_to< capture_arm_cpp_int_t >();
}

cpp_int cap_dprstats_csr_cfg_capture_t::capture_arm() const {
    return int_var__capture_arm.convert_to< cpp_int >();
}
    
void cap_dprstats_csr_cfg_capture_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_dprstats_csr_cfg_capture_t::rsvd() const {
    return int_var__rsvd.convert_to< cpp_int >();
}
    