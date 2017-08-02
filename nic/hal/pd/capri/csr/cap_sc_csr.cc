
#include "cap_sc_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_sc_csr_t::cap_sc_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(16777216);
        set_attributes(0,get_name(), 0);
        }
cap_sc_csr_t::~cap_sc_csr_t() { }

void cap_sc_csr_t::show() {

    pics.show();
    pict.show();
}

int cap_sc_csr_t::get_width() const {
    return cap_sc_csr_t::s_get_width();

}

int cap_sc_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_pics_csr_t::s_get_width(); // pics
    _count += cap_pict_csr_t::s_get_width(); // pict
    return _count;
}

void cap_sc_csr_t::all(const cpp_int & _val) {
    int _count = 0;

    pics.all( hlp.get_slc(_val, _count, _count -1 + pics.get_width() )); // pics
    _count += pics.get_width();
    pict.all( hlp.get_slc(_val, _count, _count -1 + pict.get_width() )); // pict
    _count += pict.get_width();
}

cpp_int cap_sc_csr_t::all() const {
    int _count = 0;
    cpp_int ret_val;

    ret_val = hlp.set_slc(ret_val, pics.all() , _count, _count -1 + pics.get_width() ); // pics
    _count += pics.get_width();
    ret_val = hlp.set_slc(ret_val, pict.all() , _count, _count -1 + pict.get_width() ); // pict
    _count += pict.get_width();
    return ret_val;
}

void cap_sc_csr_t::init() {

    pics.set_attributes(this,"pics", 0x0 );
    pict.set_attributes(this,"pict", 0x800000 );
}
