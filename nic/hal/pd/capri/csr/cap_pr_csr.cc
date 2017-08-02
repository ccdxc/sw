
#include "cap_pr_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_pr_csr_base_t::cap_pr_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_pr_csr_base_t::~cap_pr_csr_base_t() { }

cap_pr_csr_t::cap_pr_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(524288);
        set_attributes(0,get_name(), 0);
        }
cap_pr_csr_t::~cap_pr_csr_t() { }

void cap_pr_csr_base_t::show() {

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl)
}

void cap_pr_csr_t::show() {

    base.show();
    prd.show();
    psp.show();
}

int cap_pr_csr_base_t::get_width() const {
    return cap_pr_csr_base_t::s_get_width();

}

int cap_pr_csr_t::get_width() const {
    return cap_pr_csr_t::s_get_width();

}

int cap_pr_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_pr_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pr_csr_base_t::s_get_width(); // base
    _count += cap_prd_csr_t::s_get_width(); // prd
    _count += cap_psp_csr_t::s_get_width(); // psp
    return _count;
}

void cap_pr_csr_base_t::all(const cpp_int & _val) {
    int _count = 0;

    // scratch_reg
    int_var__scratch_reg = hlp.get_slc(_val, _count, _count -1 + 32 ).convert_to< scratch_reg_cpp_int_t >()  ;
    _count += 32;
}

void cap_pr_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    base.all( hlp.get_slc(_val, _count, _count -1 + base.get_width() )); // base
    _count += base.get_width();
    prd.all( hlp.get_slc(_val, _count, _count -1 + prd.get_width() )); // prd
    _count += prd.get_width();
    psp.all( hlp.get_slc(_val, _count, _count -1 + psp.get_width() )); // psp
    _count += psp.get_width();
}

cpp_int cap_pr_csr_base_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    // scratch_reg
    ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__scratch_reg) , _count, _count -1 + 32 );
    _count += 32;
    return ret_val;
}

cpp_int cap_pr_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, base.all() , _count, _count -1 + base.get_width() ); // base
    _count += base.get_width();
    ret_val = hlp.set_slc(ret_val, prd.all() , _count, _count -1 + prd.get_width() ); // prd
    _count += prd.get_width();
    ret_val = hlp.set_slc(ret_val, psp.all() , _count, _count -1 + psp.get_width() ); // psp
    _count += psp.get_width();
    return ret_val;
}

void cap_pr_csr_base_t::init() {

        #ifndef EXCLUDE_PER_FIELD_CNTRL
        if(!get_field_init_done()) {
            register_set_func("scratch_reg", (cap_csr_base::set_function_type_t)&cap_pr_csr_base_t::scratch_reg);
            register_get_func("scratch_reg", (cap_csr_base::get_function_type_t)&cap_pr_csr_base_t::scratch_reg);
        }
        #endif
    
    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_pr_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    prd.set_attributes(this,"prd", 0x100 );
    psp.set_attributes(this,"psp", 0x20000 );
}

void cap_pr_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_pr_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg.convert_to< cpp_int >();
}
    