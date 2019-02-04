
#include "cap_msr_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_msr_csr_cfg_flash_t::cap_msr_csr_cfg_flash_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_msr_csr_cfg_flash_t::~cap_msr_csr_cfg_flash_t() { }

cap_msr_csr_cfg_arm_t::cap_msr_csr_cfg_arm_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_msr_csr_cfg_arm_t::~cap_msr_csr_cfg_arm_t() { }

cap_msr_csr_c2p_clk_t::cap_msr_csr_c2p_clk_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_msr_csr_c2p_clk_t::~cap_msr_csr_c2p_clk_t() { }

cap_msr_csr_sta_cause_t::cap_msr_csr_sta_cause_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_msr_csr_sta_cause_t::~cap_msr_csr_sta_cause_t() { }

cap_msr_csr_cfg_boot_t::cap_msr_csr_cfg_boot_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_msr_csr_cfg_boot_t::~cap_msr_csr_cfg_boot_t() { }

cap_msr_csr_cfg_nonresettable_t::cap_msr_csr_cfg_nonresettable_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_msr_csr_cfg_nonresettable_t::~cap_msr_csr_cfg_nonresettable_t() { }

cap_msr_csr_t::cap_msr_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(128);
        set_attributes(0,get_name(), 0);
        }
cap_msr_csr_t::~cap_msr_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_cfg_flash_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".reset_timer: 0x" << int_var__reset_timer << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_cfg_arm_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ov_resRESETPOR: 0x" << int_var__ov_resRESETPOR << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ov_resCPUPORESET: 0x" << int_var__ov_resCPUPORESET << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_c2p_clk_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".obs_sel: 0x" << int_var__obs_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".obs_div: 0x" << int_var__obs_div << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_sta_cause_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".reason_type: 0x" << int_var__reason_type << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_cfg_boot_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mode_reg: 0x" << int_var__mode_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_cfg_nonresettable_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".nr_reg: 0x" << int_var__nr_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_msr_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_nonresettable.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        cfg_nonresettable[ii].show();
    }
    #endif
    
    cfg_boot.show();
    sta_cause.show();
    c2p_clk.show();
    cfg_arm.show();
    cfg_flash.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_msr_csr_cfg_flash_t::get_width() const {
    return cap_msr_csr_cfg_flash_t::s_get_width();

}

int cap_msr_csr_cfg_arm_t::get_width() const {
    return cap_msr_csr_cfg_arm_t::s_get_width();

}

int cap_msr_csr_c2p_clk_t::get_width() const {
    return cap_msr_csr_c2p_clk_t::s_get_width();

}

int cap_msr_csr_sta_cause_t::get_width() const {
    return cap_msr_csr_sta_cause_t::s_get_width();

}

int cap_msr_csr_cfg_boot_t::get_width() const {
    return cap_msr_csr_cfg_boot_t::s_get_width();

}

int cap_msr_csr_cfg_nonresettable_t::get_width() const {
    return cap_msr_csr_cfg_nonresettable_t::s_get_width();

}

int cap_msr_csr_t::get_width() const {
    return cap_msr_csr_t::s_get_width();

}

int cap_msr_csr_cfg_flash_t::s_get_width() {
    int _count = 0;

    _count += 35; // reset_timer
    return _count;
}

int cap_msr_csr_cfg_arm_t::s_get_width() {
    int _count = 0;

    _count += 1; // ov_resRESETPOR
    _count += 1; // ov_resCPUPORESET
    return _count;
}

int cap_msr_csr_c2p_clk_t::s_get_width() {
    int _count = 0;

    _count += 4; // obs_sel
    _count += 2; // obs_div
    return _count;
}

int cap_msr_csr_sta_cause_t::s_get_width() {
    int _count = 0;

    _count += 3; // reason_type
    return _count;
}

int cap_msr_csr_cfg_boot_t::s_get_width() {
    int _count = 0;

    _count += 32; // mode_reg
    return _count;
}

int cap_msr_csr_cfg_nonresettable_t::s_get_width() {
    int _count = 0;

    _count += 32; // nr_reg
    return _count;
}

int cap_msr_csr_t::s_get_width() {
    int _count = 0;

    _count += (cap_msr_csr_cfg_nonresettable_t::s_get_width() * 16); // cfg_nonresettable
    _count += cap_msr_csr_cfg_boot_t::s_get_width(); // cfg_boot
    _count += cap_msr_csr_sta_cause_t::s_get_width(); // sta_cause
    _count += cap_msr_csr_c2p_clk_t::s_get_width(); // c2p_clk
    _count += cap_msr_csr_cfg_arm_t::s_get_width(); // cfg_arm
    _count += cap_msr_csr_cfg_flash_t::s_get_width(); // cfg_flash
    return _count;
}

void cap_msr_csr_cfg_flash_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__reset_timer = _val.convert_to< reset_timer_cpp_int_t >()  ;
    _val = _val >> 35;
    
}

void cap_msr_csr_cfg_arm_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ov_resRESETPOR = _val.convert_to< ov_resRESETPOR_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ov_resCPUPORESET = _val.convert_to< ov_resCPUPORESET_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_msr_csr_c2p_clk_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__obs_sel = _val.convert_to< obs_sel_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__obs_div = _val.convert_to< obs_div_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_msr_csr_sta_cause_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__reason_type = _val.convert_to< reason_type_cpp_int_t >()  ;
    _val = _val >> 3;
    
}

void cap_msr_csr_cfg_boot_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mode_reg = _val.convert_to< mode_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_msr_csr_cfg_nonresettable_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__nr_reg = _val.convert_to< nr_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_msr_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_nonresettable
    for(int ii = 0; ii < 16; ii++) {
        cfg_nonresettable[ii].all(_val);
        _val = _val >> cfg_nonresettable[ii].get_width();
    }
    #endif
    
    cfg_boot.all( _val);
    _val = _val >> cfg_boot.get_width(); 
    sta_cause.all( _val);
    _val = _val >> sta_cause.get_width(); 
    c2p_clk.all( _val);
    _val = _val >> c2p_clk.get_width(); 
    cfg_arm.all( _val);
    _val = _val >> cfg_arm.get_width(); 
    cfg_flash.all( _val);
    _val = _val >> cfg_flash.get_width(); 
}

cpp_int cap_msr_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << cfg_flash.get_width(); ret_val = ret_val  | cfg_flash.all(); 
    ret_val = ret_val << cfg_arm.get_width(); ret_val = ret_val  | cfg_arm.all(); 
    ret_val = ret_val << c2p_clk.get_width(); ret_val = ret_val  | c2p_clk.all(); 
    ret_val = ret_val << sta_cause.get_width(); ret_val = ret_val  | sta_cause.all(); 
    ret_val = ret_val << cfg_boot.get_width(); ret_val = ret_val  | cfg_boot.all(); 
    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // cfg_nonresettable
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << cfg_nonresettable[ii].get_width(); ret_val = ret_val  | cfg_nonresettable[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_msr_csr_cfg_nonresettable_t::all() const {
    cpp_int ret_val;

    // nr_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__nr_reg; 
    
    return ret_val;
}

cpp_int cap_msr_csr_cfg_boot_t::all() const {
    cpp_int ret_val;

    // mode_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__mode_reg; 
    
    return ret_val;
}

cpp_int cap_msr_csr_sta_cause_t::all() const {
    cpp_int ret_val;

    // reason_type
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__reason_type; 
    
    return ret_val;
}

cpp_int cap_msr_csr_c2p_clk_t::all() const {
    cpp_int ret_val;

    // obs_div
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__obs_div; 
    
    // obs_sel
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__obs_sel; 
    
    return ret_val;
}

cpp_int cap_msr_csr_cfg_arm_t::all() const {
    cpp_int ret_val;

    // ov_resCPUPORESET
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ov_resCPUPORESET; 
    
    // ov_resRESETPOR
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ov_resRESETPOR; 
    
    return ret_val;
}

cpp_int cap_msr_csr_cfg_flash_t::all() const {
    cpp_int ret_val;

    // reset_timer
    ret_val = ret_val << 35; ret_val = ret_val  | int_var__reset_timer; 
    
    return ret_val;
}

void cap_msr_csr_cfg_flash_t::clear() {

    int_var__reset_timer = 0; 
    
}

void cap_msr_csr_cfg_arm_t::clear() {

    int_var__ov_resRESETPOR = 0; 
    
    int_var__ov_resCPUPORESET = 0; 
    
}

void cap_msr_csr_c2p_clk_t::clear() {

    int_var__obs_sel = 0; 
    
    int_var__obs_div = 0; 
    
}

void cap_msr_csr_sta_cause_t::clear() {

    int_var__reason_type = 0; 
    
}

void cap_msr_csr_cfg_boot_t::clear() {

    int_var__mode_reg = 0; 
    
}

void cap_msr_csr_cfg_nonresettable_t::clear() {

    int_var__nr_reg = 0; 
    
}

void cap_msr_csr_t::clear() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // cfg_nonresettable
    for(int ii = 0; ii < 16; ii++) {
        cfg_nonresettable[ii].clear();
    }
    #endif
    
    cfg_boot.clear();
    sta_cause.clear();
    c2p_clk.clear();
    cfg_arm.clear();
    cfg_flash.clear();
}

void cap_msr_csr_cfg_flash_t::init() {

}

void cap_msr_csr_cfg_arm_t::init() {

}

void cap_msr_csr_c2p_clk_t::init() {

}

void cap_msr_csr_sta_cause_t::init() {

}

void cap_msr_csr_cfg_boot_t::init() {

    set_reset_val(cpp_int("0x2"));
    all(get_reset_val());
}

void cap_msr_csr_cfg_nonresettable_t::init() {

}

void cap_msr_csr_t::init() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    cfg_nonresettable.set_attributes(this, "cfg_nonresettable", 0x0);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) cfg_nonresettable[ii].set_field_init_done(true, true);
        cfg_nonresettable[ii].set_attributes(this,"cfg_nonresettable["+to_string(ii)+"]",  0x0 + (cfg_nonresettable[ii].get_byte_size()*ii));
    }
    #endif
    
    cfg_boot.set_attributes(this,"cfg_boot", 0x40 );
    sta_cause.set_attributes(this,"sta_cause", 0x44 );
    c2p_clk.set_attributes(this,"c2p_clk", 0x48 );
    cfg_arm.set_attributes(this,"cfg_arm", 0x4c );
    cfg_flash.set_attributes(this,"cfg_flash", 0x50 );
}

void cap_msr_csr_cfg_flash_t::reset_timer(const cpp_int & _val) { 
    // reset_timer
    int_var__reset_timer = _val.convert_to< reset_timer_cpp_int_t >();
}

cpp_int cap_msr_csr_cfg_flash_t::reset_timer() const {
    return int_var__reset_timer;
}
    
void cap_msr_csr_cfg_arm_t::ov_resRESETPOR(const cpp_int & _val) { 
    // ov_resRESETPOR
    int_var__ov_resRESETPOR = _val.convert_to< ov_resRESETPOR_cpp_int_t >();
}

cpp_int cap_msr_csr_cfg_arm_t::ov_resRESETPOR() const {
    return int_var__ov_resRESETPOR;
}
    
void cap_msr_csr_cfg_arm_t::ov_resCPUPORESET(const cpp_int & _val) { 
    // ov_resCPUPORESET
    int_var__ov_resCPUPORESET = _val.convert_to< ov_resCPUPORESET_cpp_int_t >();
}

cpp_int cap_msr_csr_cfg_arm_t::ov_resCPUPORESET() const {
    return int_var__ov_resCPUPORESET;
}
    
void cap_msr_csr_c2p_clk_t::obs_sel(const cpp_int & _val) { 
    // obs_sel
    int_var__obs_sel = _val.convert_to< obs_sel_cpp_int_t >();
}

cpp_int cap_msr_csr_c2p_clk_t::obs_sel() const {
    return int_var__obs_sel;
}
    
void cap_msr_csr_c2p_clk_t::obs_div(const cpp_int & _val) { 
    // obs_div
    int_var__obs_div = _val.convert_to< obs_div_cpp_int_t >();
}

cpp_int cap_msr_csr_c2p_clk_t::obs_div() const {
    return int_var__obs_div;
}
    
void cap_msr_csr_sta_cause_t::reason_type(const cpp_int & _val) { 
    // reason_type
    int_var__reason_type = _val.convert_to< reason_type_cpp_int_t >();
}

cpp_int cap_msr_csr_sta_cause_t::reason_type() const {
    return int_var__reason_type;
}
    
void cap_msr_csr_cfg_boot_t::mode_reg(const cpp_int & _val) { 
    // mode_reg
    int_var__mode_reg = _val.convert_to< mode_reg_cpp_int_t >();
}

cpp_int cap_msr_csr_cfg_boot_t::mode_reg() const {
    return int_var__mode_reg;
}
    
void cap_msr_csr_cfg_nonresettable_t::nr_reg(const cpp_int & _val) { 
    // nr_reg
    int_var__nr_reg = _val.convert_to< nr_reg_cpp_int_t >();
}

cpp_int cap_msr_csr_cfg_nonresettable_t::nr_reg() const {
    return int_var__nr_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_flash_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "reset_timer")) { field_val = reset_timer(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_arm_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ov_resRESETPOR")) { field_val = ov_resRESETPOR(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ov_resCPUPORESET")) { field_val = ov_resCPUPORESET(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_c2p_clk_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "obs_sel")) { field_val = obs_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "obs_div")) { field_val = obs_div(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_sta_cause_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "reason_type")) { field_val = reason_type(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_boot_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mode_reg")) { field_val = mode_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_nonresettable_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nr_reg")) { field_val = nr_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = cfg_boot.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_cause.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = c2p_clk.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_arm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_flash.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_flash_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "reset_timer")) { reset_timer(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_arm_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ov_resRESETPOR")) { ov_resRESETPOR(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ov_resCPUPORESET")) { ov_resCPUPORESET(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_c2p_clk_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "obs_sel")) { obs_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "obs_div")) { obs_div(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_sta_cause_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "reason_type")) { reason_type(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_boot_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mode_reg")) { mode_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_cfg_nonresettable_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nr_reg")) { nr_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_msr_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = cfg_boot.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_cause.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = c2p_clk.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_arm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_flash.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_cfg_flash_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("reset_timer");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_cfg_arm_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ov_resRESETPOR");
    ret_vec.push_back("ov_resCPUPORESET");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_c2p_clk_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("obs_sel");
    ret_vec.push_back("obs_div");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_sta_cause_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("reason_type");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_cfg_boot_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mode_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_cfg_nonresettable_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("nr_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_msr_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : cfg_boot.get_fields(level-1)) {
            ret_vec.push_back("cfg_boot." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_cause.get_fields(level-1)) {
            ret_vec.push_back("sta_cause." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : c2p_clk.get_fields(level-1)) {
            ret_vec.push_back("c2p_clk." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_arm.get_fields(level-1)) {
            ret_vec.push_back("cfg_arm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_flash.get_fields(level-1)) {
            ret_vec.push_back("cfg_flash." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
