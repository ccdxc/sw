
#include "cap_pic_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pic_opcode_decoder_t::cap_pic_opcode_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pic_opcode_decoder_t::~cap_pic_opcode_decoder_t() { }

cap_pic_policer_decoder_t::cap_pic_policer_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pic_policer_decoder_t::~cap_pic_policer_decoder_t() { }

cap_pic_bg_sm_decoder_t::cap_pic_bg_sm_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pic_bg_sm_decoder_t::~cap_pic_bg_sm_decoder_t() { }

cap_pic_rl_decoder_t::cap_pic_rl_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pic_rl_decoder_t::~cap_pic_rl_decoder_t() { }

cap_pic_rl_vld_decoder_t::cap_pic_rl_vld_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_pic_rl_vld_decoder_t::~cap_pic_rl_vld_decoder_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_pic_opcode_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".operation: 0x" << int_var__operation << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd1_sel: 0x" << int_var__oprd1_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".oprd2_sel: 0x" << int_var__oprd2_sel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".saturate: 0x" << int_var__saturate << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".policer: 0x" << int_var__policer << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pic_policer_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tbkt: 0x" << int_var__tbkt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rate: 0x" << int_var__rate << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_wr_pend: 0x" << int_var__axi_wr_pend << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".color_aware: 0x" << int_var__color_aware << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rlimit_prof: 0x" << int_var__rlimit_prof << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rlimit_en: 0x" << int_var__rlimit_en << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pkt_rate: 0x" << int_var__pkt_rate << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pic_bg_sm_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".active: 0x" << int_var__active << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".timer: 0x" << int_var__timer << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cycle: 0x" << int_var__cycle << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pic_rl_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".spare: 0x" << int_var__spare << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".stop: 0x" << int_var__stop << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rlid: 0x" << int_var__rlid << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_pic_rl_vld_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    data.show();
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

int cap_pic_opcode_decoder_t::get_width() const {
    return cap_pic_opcode_decoder_t::s_get_width();

}

int cap_pic_policer_decoder_t::get_width() const {
    return cap_pic_policer_decoder_t::s_get_width();

}

int cap_pic_bg_sm_decoder_t::get_width() const {
    return cap_pic_bg_sm_decoder_t::s_get_width();

}

int cap_pic_rl_decoder_t::get_width() const {
    return cap_pic_rl_decoder_t::s_get_width();

}

int cap_pic_rl_vld_decoder_t::get_width() const {
    return cap_pic_rl_vld_decoder_t::s_get_width();

}

int cap_pic_opcode_decoder_t::s_get_width() {
    int _count = 0;

    _count += 4; // operation
    _count += 2; // oprd1_sel
    _count += 2; // oprd2_sel
    _count += 2; // saturate
    _count += 1; // policer
    return _count;
}

int cap_pic_policer_decoder_t::s_get_width() {
    int _count = 0;

    _count += 40; // tbkt
    _count += 40; // rate
    _count += 40; // burst
    _count += 1; // axi_wr_pend
    _count += 1; // rsvd
    _count += 1; // color_aware
    _count += 2; // rlimit_prof
    _count += 1; // rlimit_en
    _count += 1; // pkt_rate
    _count += 1; // valid
    return _count;
}

int cap_pic_bg_sm_decoder_t::s_get_width() {
    int _count = 0;

    _count += 1; // active
    _count += 16; // timer
    _count += 20; // addr
    _count += 8; // cycle
    return _count;
}

int cap_pic_rl_decoder_t::s_get_width() {
    int _count = 0;

    _count += 4; // spare
    _count += 1; // stop
    _count += 11; // rlid
    return _count;
}

int cap_pic_rl_vld_decoder_t::s_get_width() {
    int _count = 0;

    _count += cap_pic_rl_decoder_t::s_get_width(); // data
    _count += 1; // valid
    return _count;
}

void cap_pic_opcode_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__operation = _val.convert_to< operation_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__oprd1_sel = _val.convert_to< oprd1_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__oprd2_sel = _val.convert_to< oprd2_sel_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__saturate = _val.convert_to< saturate_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__policer = _val.convert_to< policer_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pic_policer_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tbkt = _val.convert_to< tbkt_cpp_int_t >()  ;
    _val = _val >> 40;
    
    int_var__rate = _val.convert_to< rate_cpp_int_t >()  ;
    _val = _val >> 40;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 40;
    
    int_var__axi_wr_pend = _val.convert_to< axi_wr_pend_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__color_aware = _val.convert_to< color_aware_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rlimit_prof = _val.convert_to< rlimit_prof_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__rlimit_en = _val.convert_to< rlimit_en_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__pkt_rate = _val.convert_to< pkt_rate_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_pic_bg_sm_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__active = _val.convert_to< active_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__timer = _val.convert_to< timer_cpp_int_t >()  ;
    _val = _val >> 16;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 20;
    
    int_var__cycle = _val.convert_to< cycle_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_pic_rl_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__spare = _val.convert_to< spare_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__stop = _val.convert_to< stop_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__rlid = _val.convert_to< rlid_cpp_int_t >()  ;
    _val = _val >> 11;
    
}

void cap_pic_rl_vld_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    data.all( _val);
    _val = _val >> data.get_width(); 
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

cpp_int cap_pic_rl_vld_decoder_t::all() const {
    cpp_int ret_val;

    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    ret_val = ret_val << data.get_width(); ret_val = ret_val  | data.all(); 
    return ret_val;
}

cpp_int cap_pic_rl_decoder_t::all() const {
    cpp_int ret_val;

    // rlid
    ret_val = ret_val << 11; ret_val = ret_val  | int_var__rlid; 
    
    // stop
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__stop; 
    
    // spare
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__spare; 
    
    return ret_val;
}

cpp_int cap_pic_bg_sm_decoder_t::all() const {
    cpp_int ret_val;

    // cycle
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__cycle; 
    
    // addr
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__addr; 
    
    // timer
    ret_val = ret_val << 16; ret_val = ret_val  | int_var__timer; 
    
    // active
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__active; 
    
    return ret_val;
}

cpp_int cap_pic_policer_decoder_t::all() const {
    cpp_int ret_val;

    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // pkt_rate
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__pkt_rate; 
    
    // rlimit_en
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rlimit_en; 
    
    // rlimit_prof
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__rlimit_prof; 
    
    // color_aware
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__color_aware; 
    
    // rsvd
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__rsvd; 
    
    // axi_wr_pend
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_wr_pend; 
    
    // burst
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__burst; 
    
    // rate
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__rate; 
    
    // tbkt
    ret_val = ret_val << 40; ret_val = ret_val  | int_var__tbkt; 
    
    return ret_val;
}

cpp_int cap_pic_opcode_decoder_t::all() const {
    cpp_int ret_val;

    // policer
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__policer; 
    
    // saturate
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__saturate; 
    
    // oprd2_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__oprd2_sel; 
    
    // oprd1_sel
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__oprd1_sel; 
    
    // operation
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__operation; 
    
    return ret_val;
}

void cap_pic_opcode_decoder_t::clear() {

    int_var__operation = 0; 
    
    int_var__oprd1_sel = 0; 
    
    int_var__oprd2_sel = 0; 
    
    int_var__saturate = 0; 
    
    int_var__policer = 0; 
    
}

void cap_pic_policer_decoder_t::clear() {

    int_var__tbkt = 0; 
    
    int_var__rate = 0; 
    
    int_var__burst = 0; 
    
    int_var__axi_wr_pend = 0; 
    
    int_var__rsvd = 0; 
    
    int_var__color_aware = 0; 
    
    int_var__rlimit_prof = 0; 
    
    int_var__rlimit_en = 0; 
    
    int_var__pkt_rate = 0; 
    
    int_var__valid = 0; 
    
}

void cap_pic_bg_sm_decoder_t::clear() {

    int_var__active = 0; 
    
    int_var__timer = 0; 
    
    int_var__addr = 0; 
    
    int_var__cycle = 0; 
    
}

void cap_pic_rl_decoder_t::clear() {

    int_var__spare = 0; 
    
    int_var__stop = 0; 
    
    int_var__rlid = 0; 
    
}

void cap_pic_rl_vld_decoder_t::clear() {

    data.clear();
    int_var__valid = 0; 
    
}

void cap_pic_opcode_decoder_t::init() {

}

void cap_pic_policer_decoder_t::init() {

}

void cap_pic_bg_sm_decoder_t::init() {

}

void cap_pic_rl_decoder_t::init() {

}

void cap_pic_rl_vld_decoder_t::init() {

    data.set_attributes(this,"data", 0x0 );
}

void cap_pic_opcode_decoder_t::operation(const cpp_int & _val) { 
    // operation
    int_var__operation = _val.convert_to< operation_cpp_int_t >();
}

cpp_int cap_pic_opcode_decoder_t::operation() const {
    return int_var__operation;
}
    
void cap_pic_opcode_decoder_t::oprd1_sel(const cpp_int & _val) { 
    // oprd1_sel
    int_var__oprd1_sel = _val.convert_to< oprd1_sel_cpp_int_t >();
}

cpp_int cap_pic_opcode_decoder_t::oprd1_sel() const {
    return int_var__oprd1_sel;
}
    
void cap_pic_opcode_decoder_t::oprd2_sel(const cpp_int & _val) { 
    // oprd2_sel
    int_var__oprd2_sel = _val.convert_to< oprd2_sel_cpp_int_t >();
}

cpp_int cap_pic_opcode_decoder_t::oprd2_sel() const {
    return int_var__oprd2_sel;
}
    
void cap_pic_opcode_decoder_t::saturate(const cpp_int & _val) { 
    // saturate
    int_var__saturate = _val.convert_to< saturate_cpp_int_t >();
}

cpp_int cap_pic_opcode_decoder_t::saturate() const {
    return int_var__saturate;
}
    
void cap_pic_opcode_decoder_t::policer(const cpp_int & _val) { 
    // policer
    int_var__policer = _val.convert_to< policer_cpp_int_t >();
}

cpp_int cap_pic_opcode_decoder_t::policer() const {
    return int_var__policer;
}
    
void cap_pic_policer_decoder_t::tbkt(const cpp_int & _val) { 
    // tbkt
    int_var__tbkt = _val.convert_to< tbkt_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::tbkt() const {
    return int_var__tbkt;
}
    
void cap_pic_policer_decoder_t::rate(const cpp_int & _val) { 
    // rate
    int_var__rate = _val.convert_to< rate_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::rate() const {
    return int_var__rate;
}
    
void cap_pic_policer_decoder_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::burst() const {
    return int_var__burst;
}
    
void cap_pic_policer_decoder_t::axi_wr_pend(const cpp_int & _val) { 
    // axi_wr_pend
    int_var__axi_wr_pend = _val.convert_to< axi_wr_pend_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::axi_wr_pend() const {
    return int_var__axi_wr_pend;
}
    
void cap_pic_policer_decoder_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::rsvd() const {
    return int_var__rsvd;
}
    
void cap_pic_policer_decoder_t::color_aware(const cpp_int & _val) { 
    // color_aware
    int_var__color_aware = _val.convert_to< color_aware_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::color_aware() const {
    return int_var__color_aware;
}
    
void cap_pic_policer_decoder_t::rlimit_prof(const cpp_int & _val) { 
    // rlimit_prof
    int_var__rlimit_prof = _val.convert_to< rlimit_prof_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::rlimit_prof() const {
    return int_var__rlimit_prof;
}
    
void cap_pic_policer_decoder_t::rlimit_en(const cpp_int & _val) { 
    // rlimit_en
    int_var__rlimit_en = _val.convert_to< rlimit_en_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::rlimit_en() const {
    return int_var__rlimit_en;
}
    
void cap_pic_policer_decoder_t::pkt_rate(const cpp_int & _val) { 
    // pkt_rate
    int_var__pkt_rate = _val.convert_to< pkt_rate_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::pkt_rate() const {
    return int_var__pkt_rate;
}
    
void cap_pic_policer_decoder_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_pic_policer_decoder_t::valid() const {
    return int_var__valid;
}
    
void cap_pic_bg_sm_decoder_t::active(const cpp_int & _val) { 
    // active
    int_var__active = _val.convert_to< active_cpp_int_t >();
}

cpp_int cap_pic_bg_sm_decoder_t::active() const {
    return int_var__active;
}
    
void cap_pic_bg_sm_decoder_t::timer(const cpp_int & _val) { 
    // timer
    int_var__timer = _val.convert_to< timer_cpp_int_t >();
}

cpp_int cap_pic_bg_sm_decoder_t::timer() const {
    return int_var__timer;
}
    
void cap_pic_bg_sm_decoder_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_pic_bg_sm_decoder_t::addr() const {
    return int_var__addr;
}
    
void cap_pic_bg_sm_decoder_t::cycle(const cpp_int & _val) { 
    // cycle
    int_var__cycle = _val.convert_to< cycle_cpp_int_t >();
}

cpp_int cap_pic_bg_sm_decoder_t::cycle() const {
    return int_var__cycle;
}
    
void cap_pic_rl_decoder_t::spare(const cpp_int & _val) { 
    // spare
    int_var__spare = _val.convert_to< spare_cpp_int_t >();
}

cpp_int cap_pic_rl_decoder_t::spare() const {
    return int_var__spare;
}
    
void cap_pic_rl_decoder_t::stop(const cpp_int & _val) { 
    // stop
    int_var__stop = _val.convert_to< stop_cpp_int_t >();
}

cpp_int cap_pic_rl_decoder_t::stop() const {
    return int_var__stop;
}
    
void cap_pic_rl_decoder_t::rlid(const cpp_int & _val) { 
    // rlid
    int_var__rlid = _val.convert_to< rlid_cpp_int_t >();
}

cpp_int cap_pic_rl_decoder_t::rlid() const {
    return int_var__rlid;
}
    
void cap_pic_rl_vld_decoder_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_pic_rl_vld_decoder_t::valid() const {
    return int_var__valid;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_opcode_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "operation")) { field_val = operation(); field_found=1; }
    if(!field_found && !strcmp(field_name, "oprd1_sel")) { field_val = oprd1_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "oprd2_sel")) { field_val = oprd2_sel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "saturate")) { field_val = saturate(); field_found=1; }
    if(!field_found && !strcmp(field_name, "policer")) { field_val = policer(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_policer_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tbkt")) { field_val = tbkt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rate")) { field_val = rate(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wr_pend")) { field_val = axi_wr_pend(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "color_aware")) { field_val = color_aware(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rlimit_prof")) { field_val = rlimit_prof(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rlimit_en")) { field_val = rlimit_en(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_rate")) { field_val = pkt_rate(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_bg_sm_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "active")) { field_val = active(); field_found=1; }
    if(!field_found && !strcmp(field_name, "timer")) { field_val = timer(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cycle")) { field_val = cycle(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_rl_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { field_val = spare(); field_found=1; }
    if(!field_found && !strcmp(field_name, "stop")) { field_val = stop(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rlid")) { field_val = rlid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_rl_vld_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = data.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_opcode_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "operation")) { operation(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "oprd1_sel")) { oprd1_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "oprd2_sel")) { oprd2_sel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "saturate")) { saturate(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "policer")) { policer(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_policer_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tbkt")) { tbkt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rate")) { rate(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wr_pend")) { axi_wr_pend(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "color_aware")) { color_aware(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rlimit_prof")) { rlimit_prof(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rlimit_en")) { rlimit_en(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pkt_rate")) { pkt_rate(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_bg_sm_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "active")) { active(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "timer")) { timer(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cycle")) { cycle(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_rl_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "spare")) { spare(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "stop")) { stop(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rlid")) { rlid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_pic_rl_vld_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = data.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pic_opcode_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("operation");
    ret_vec.push_back("oprd1_sel");
    ret_vec.push_back("oprd2_sel");
    ret_vec.push_back("saturate");
    ret_vec.push_back("policer");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pic_policer_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tbkt");
    ret_vec.push_back("rate");
    ret_vec.push_back("burst");
    ret_vec.push_back("axi_wr_pend");
    ret_vec.push_back("rsvd");
    ret_vec.push_back("color_aware");
    ret_vec.push_back("rlimit_prof");
    ret_vec.push_back("rlimit_en");
    ret_vec.push_back("pkt_rate");
    ret_vec.push_back("valid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pic_bg_sm_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("active");
    ret_vec.push_back("timer");
    ret_vec.push_back("addr");
    ret_vec.push_back("cycle");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pic_rl_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("spare");
    ret_vec.push_back("stop");
    ret_vec.push_back("rlid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_pic_rl_vld_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : data.get_fields(level-1)) {
            ret_vec.push_back("data." + tmp_vec);
        }
    }
    ret_vec.push_back("valid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
