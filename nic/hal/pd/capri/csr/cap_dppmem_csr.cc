
#include "cap_dppmem_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::~cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t() { }

cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::~cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t() { }

cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::~cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t() { }

cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::~cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t() { }

cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::~cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t() { }

cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::~cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t() { }

cap_dppmem_csr_t::cap_dppmem_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(262144);
        set_attributes(0,get_name(), 0);
        }
cap_dppmem_csr_t::~cap_dppmem_csr_t() { }

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl)
}

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::show() {

    for(int ii = 0; ii < 210; ii++) {
        entry[ii].show();
    }
}

void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl)
}

void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::show() {

    for(int ii = 0; ii < 640; ii++) {
        entry[ii].show();
    }
}

void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl)
}

void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl)
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl)
}

void cap_dppmem_csr_t::show() {

    sta_srams_ecc_dpp_phv_fifo.show();
    sta_srams_ecc_dpp_ohi_fifo.show();
    dhs_dpp_phv_fifo_sram.show();
    dhs_dpp_ohi_fifo_sram.show();
}

int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::get_width() const {
    return cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::s_get_width();

}

int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::get_width() const {
    return cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::s_get_width();

}

int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::get_width() const {
    return cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::s_get_width();

}

int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::get_width() const {
    return cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::s_get_width();

}

int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::get_width() const {
    return cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::s_get_width();

}

int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::get_width() const {
    return cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::s_get_width();

}

int cap_dppmem_csr_t::get_width() const {
    return cap_dppmem_csr_t::s_get_width();

}

int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 515; // data
    _count += 36; // ecc
    return _count;
}

int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::s_get_width() * 210); // entry
    return _count;
}

int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 515; // data
    _count += 20; // ecc
    return _count;
}

int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::s_get_width() * 640); // entry
    return _count;
}

int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 11; // syndrome
    _count += 8; // addr
    return _count;
}

int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 11; // syndrome
    _count += 10; // addr
    return _count;
}

int cap_dppmem_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::s_get_width(); // sta_srams_ecc_dpp_phv_fifo
    _count += cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::s_get_width(); // sta_srams_ecc_dpp_ohi_fifo
    _count += cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::s_get_width(); // dhs_dpp_phv_fifo_sram
    _count += cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::s_get_width(); // dhs_dpp_ohi_fifo_sram
    return _count;
}

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 515 ).convert_to< data_cpp_int_t >()  ;
    _count += 515;
    // ecc
    int_var__ecc = hlp.get_slc(_val, _count, _count -1 + 36 ).convert_to< ecc_cpp_int_t >()  ;
    _count += 36;
}

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 210; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::all(const cpp_int & _val) {
    int _count = 0;

    // data
    int_var__data = hlp.get_slc(_val, _count, _count -1 + 515 ).convert_to< data_cpp_int_t >()  ;
    _count += 515;
    // ecc
    int_var__ecc = hlp.get_slc(_val, _count, _count -1 + 20 ).convert_to< ecc_cpp_int_t >()  ;
    _count += 20;
}

void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::all(const cpp_int & _val) {
    int _count = 0;

    // entry
    for(int ii = 0; ii < 640; ii++) {
        entry[ii].all( hlp.get_slc(_val, _count, _count -1 + entry[ii].get_width()));
        _count += entry[ii].get_width();
    }
}

void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::all(const cpp_int & _val) {
    int _count = 0;

    // uncorrectable
    int_var__uncorrectable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< uncorrectable_cpp_int_t >()  ;
    _count += 1;
    // correctable
    int_var__correctable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< correctable_cpp_int_t >()  ;
    _count += 1;
    // syndrome
    int_var__syndrome = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< syndrome_cpp_int_t >()  ;
    _count += 11;
    // addr
    int_var__addr = hlp.get_slc(_val, _count, _count -1 + 8 ).convert_to< addr_cpp_int_t >()  ;
    _count += 8;
}

void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::all(const cpp_int & _val) {
    int _count = 0;

    // uncorrectable
    int_var__uncorrectable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< uncorrectable_cpp_int_t >()  ;
    _count += 1;
    // correctable
    int_var__correctable = hlp.get_slc(_val, _count, _count -1 + 1 ).convert_to< correctable_cpp_int_t >()  ;
    _count += 1;
    // syndrome
    int_var__syndrome = hlp.get_slc(_val, _count, _count -1 + 11 ).convert_to< syndrome_cpp_int_t >()  ;
    _count += 11;
    // addr
    int_var__addr = hlp.get_slc(_val, _count, _count -1 + 10 ).convert_to< addr_cpp_int_t >()  ;
    _count += 10;
}

void cap_dppmem_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    sta_srams_ecc_dpp_phv_fifo.all( hlp.get_slc(_val, _count, _count -1 + sta_srams_ecc_dpp_phv_fifo.get_width() )); // sta_srams_ecc_dpp_phv_fifo
    _count += sta_srams_ecc_dpp_phv_fifo.get_width();
    sta_srams_ecc_dpp_ohi_fifo.all( hlp.get_slc(_val, _count, _count -1 + sta_srams_ecc_dpp_ohi_fifo.get_width() )); // sta_srams_ecc_dpp_ohi_fifo
    _count += sta_srams_ecc_dpp_ohi_fifo.get_width();
    dhs_dpp_phv_fifo_sram.all( hlp.get_slc(_val, _count, _count -1 + dhs_dpp_phv_fifo_sram.get_width() )); // dhs_dpp_phv_fifo_sram
    _count += dhs_dpp_phv_fifo_sram.get_width();
    dhs_dpp_ohi_fifo_sram.all( hlp.get_slc(_val, _count, _count -1 + dhs_dpp_ohi_fifo_sram.get_width() )); // dhs_dpp_ohi_fifo_sram
    _count += dhs_dpp_ohi_fifo_sram.get_width();
}

cpp_int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 515 );
    _count += 515;
    // ecc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc) , _count, _count -1 + 36 );
    _count += 36;
    return ret_val;
}

cpp_int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 210; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // data
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__data) , _count, _count -1 + 515 );
    _count += 515;
    // ecc
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__ecc) , _count, _count -1 + 20 );
    _count += 20;
    return ret_val;
}

cpp_int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // entry
    for(int ii = 0; ii < 640; ii++) {
         ret_val = hlp.set_slc(ret_val, entry[ii].all() , _count, _count -1 + entry[ii].get_width() );
        _count += entry[ii].get_width();
    }
    return ret_val;
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // uncorrectable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__uncorrectable) , _count, _count -1 + 1 );
    _count += 1;
    // correctable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__correctable) , _count, _count -1 + 1 );
    _count += 1;
    // syndrome
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__syndrome) , _count, _count -1 + 11 );
    _count += 11;
    // addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr) , _count, _count -1 + 8 );
    _count += 8;
    return ret_val;
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // uncorrectable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__uncorrectable) , _count, _count -1 + 1 );
    _count += 1;
    // correctable
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__correctable) , _count, _count -1 + 1 );
    _count += 1;
    // syndrome
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__syndrome) , _count, _count -1 + 11 );
    _count += 11;
    // addr
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__addr) , _count, _count -1 + 10 );
    _count += 10;
    return ret_val;
}

cpp_int cap_dppmem_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, sta_srams_ecc_dpp_phv_fifo.all() , _count, _count -1 + sta_srams_ecc_dpp_phv_fifo.get_width() ); // sta_srams_ecc_dpp_phv_fifo
    _count += sta_srams_ecc_dpp_phv_fifo.get_width();
    ret_val = hlp.set_slc(ret_val, sta_srams_ecc_dpp_ohi_fifo.all() , _count, _count -1 + sta_srams_ecc_dpp_ohi_fifo.get_width() ); // sta_srams_ecc_dpp_ohi_fifo
    _count += sta_srams_ecc_dpp_ohi_fifo.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_dpp_phv_fifo_sram.all() , _count, _count -1 + dhs_dpp_phv_fifo_sram.get_width() ); // dhs_dpp_phv_fifo_sram
    _count += dhs_dpp_phv_fifo_sram.get_width();
    ret_val = hlp.set_slc(ret_val, dhs_dpp_ohi_fifo_sram.all() , _count, _count -1 + dhs_dpp_ohi_fifo_sram.get_width() ); // dhs_dpp_ohi_fifo_sram
    _count += dhs_dpp_ohi_fifo_sram.get_width();
    return ret_val;
}

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::ecc);
            register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::ecc);
        }
        #endif
    
}

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t::init() {

    for(int ii = 0; ii < 210; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("data", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::data);
            register_get_func("data", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::data);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("ecc", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::ecc);
            register_get_func("ecc", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::ecc);
        }
        #endif
    
}

void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t::init() {

    for(int ii = 0; ii < 640; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
}

void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("uncorrectable", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::uncorrectable);
            register_get_func("uncorrectable", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::uncorrectable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("correctable", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::correctable);
            register_get_func("correctable", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::correctable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("syndrome", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::syndrome);
            register_get_func("syndrome", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::syndrome);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::addr);
            register_get_func("addr", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::addr);
        }
        #endif
    
}

void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("uncorrectable", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::uncorrectable);
            register_get_func("uncorrectable", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::uncorrectable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("correctable", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::correctable);
            register_get_func("correctable", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::correctable);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("syndrome", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::syndrome);
            register_get_func("syndrome", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::syndrome);
        }
        #endif
    
        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("addr", (cap_csr_base::set_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::addr);
            register_get_func("addr", (cap_csr_base::get_function_type_t)&cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::addr);
        }
        #endif
    
}

void cap_dppmem_csr_t::init() {

    sta_srams_ecc_dpp_phv_fifo.set_attributes(this,"sta_srams_ecc_dpp_phv_fifo", 0x20000 );
    sta_srams_ecc_dpp_ohi_fifo.set_attributes(this,"sta_srams_ecc_dpp_ohi_fifo", 0x30000 );
    dhs_dpp_phv_fifo_sram.set_attributes(this,"dhs_dpp_phv_fifo_sram", 0x0 );
    dhs_dpp_ohi_fifo_sram.set_attributes(this,"dhs_dpp_ohi_fifo_sram", 0x28000 );
}

void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::data() const {
    return int_var__data.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t::ecc() const {
    return int_var__ecc.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::uncorrectable() const {
    return int_var__uncorrectable.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::correctable() const {
    return int_var__correctable.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::syndrome() const {
    return int_var__syndrome.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t::addr() const {
    return int_var__addr.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::uncorrectable() const {
    return int_var__uncorrectable.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::correctable() const {
    return int_var__correctable.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::syndrome() const {
    return int_var__syndrome.convert_to< cpp_int >();
}
    
void cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t::addr() const {
    return int_var__addr.convert_to< cpp_int >();
}
    