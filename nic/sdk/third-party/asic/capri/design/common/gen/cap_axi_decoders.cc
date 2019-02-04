
#include "cap_axi_decoders.h"
#include "LogMsg.h"        
using namespace std;
        
cap_axi_ar_bundle_t::cap_axi_ar_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_ar_bundle_t::~cap_axi_ar_bundle_t() { }

cap_axi_r_bundle_t::cap_axi_r_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_r_bundle_t::~cap_axi_r_bundle_t() { }

cap_axi_aw_bundle_t::cap_axi_aw_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_aw_bundle_t::~cap_axi_aw_bundle_t() { }

cap_axi_w_bundle_t::cap_axi_w_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_w_bundle_t::~cap_axi_w_bundle_t() { }

cap_axi_b_bundle_t::cap_axi_b_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_b_bundle_t::~cap_axi_b_bundle_t() { }

cap_axi_read_bundle_t::cap_axi_read_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_read_bundle_t::~cap_axi_read_bundle_t() { }

cap_axi_write_bundle_t::cap_axi_write_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_write_bundle_t::~cap_axi_write_bundle_t() { }

cap_axi_bundle_t::cap_axi_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_axi_bundle_t::~cap_axi_bundle_t() { }

cap_te_axi_ar_bundle_t::cap_te_axi_ar_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_ar_bundle_t::~cap_te_axi_ar_bundle_t() { }

cap_te_axi_r_bundle_t::cap_te_axi_r_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_r_bundle_t::~cap_te_axi_r_bundle_t() { }

cap_te_axi_aw_bundle_t::cap_te_axi_aw_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_aw_bundle_t::~cap_te_axi_aw_bundle_t() { }

cap_te_axi_w_bundle_t::cap_te_axi_w_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_w_bundle_t::~cap_te_axi_w_bundle_t() { }

cap_te_axi_b_bundle_t::cap_te_axi_b_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_b_bundle_t::~cap_te_axi_b_bundle_t() { }

cap_te_axi_read_bundle_t::cap_te_axi_read_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_read_bundle_t::~cap_te_axi_read_bundle_t() { }

cap_te_axi_write_bundle_t::cap_te_axi_write_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_write_bundle_t::~cap_te_axi_write_bundle_t() { }

cap_te_axi_bundle_t::cap_te_axi_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_bundle_t::~cap_te_axi_bundle_t() { }

cap_he_axi_ar_bundle_t::cap_he_axi_ar_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_ar_bundle_t::~cap_he_axi_ar_bundle_t() { }

cap_he_axi_r_bundle_t::cap_he_axi_r_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_r_bundle_t::~cap_he_axi_r_bundle_t() { }

cap_he_axi_aw_bundle_t::cap_he_axi_aw_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_aw_bundle_t::~cap_he_axi_aw_bundle_t() { }

cap_he_axi_w_bundle_t::cap_he_axi_w_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_w_bundle_t::~cap_he_axi_w_bundle_t() { }

cap_he_axi_b_bundle_t::cap_he_axi_b_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_b_bundle_t::~cap_he_axi_b_bundle_t() { }

cap_he_axi_read_bundle_t::cap_he_axi_read_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_read_bundle_t::~cap_he_axi_read_bundle_t() { }

cap_he_axi_write_bundle_t::cap_he_axi_write_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_write_bundle_t::~cap_he_axi_write_bundle_t() { }

cap_he_axi_bundle_t::cap_he_axi_bundle_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_he_axi_bundle_t::~cap_he_axi_bundle_t() { }

cap_te_axi_addr_decoder_t::cap_te_axi_addr_decoder_t(string name, cap_csr_base * parent): 
    cap_decoder_base(name, parent)  { 
        //init();
        }
cap_te_axi_addr_decoder_t::~cap_te_axi_addr_decoder_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_ar_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_r_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last: 0x" << int_var__last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_aw_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_w_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last: 0x" << int_var__last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".strb: 0x" << int_var__strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_b_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_read_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    ar.show();
    r.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_write_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    aw.show();
    w.show();
    b.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_axi_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    ar.show();
    r.show();
    aw.show();
    w.show();
    b.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_ar_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_r_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last: 0x" << int_var__last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_aw_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_w_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last: 0x" << int_var__last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".strb: 0x" << int_var__strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_b_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_read_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    ar.show();
    r.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_write_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    aw.show();
    w.show();
    b.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    ar.show();
    r.show();
    aw.show();
    w.show();
    b.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_ar_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_r_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last: 0x" << int_var__last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_aw_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".len: 0x" << int_var__len << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sz: 0x" << int_var__sz << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".burst: 0x" << int_var__burst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".prot: 0x" << int_var__prot << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".lock: 0x" << int_var__lock << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cache: 0x" << int_var__cache << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".qos: 0x" << int_var__qos << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_w_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".last: 0x" << int_var__last << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".strb: 0x" << int_var__strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_b_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".id: 0x" << int_var__id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".resp: 0x" << int_var__resp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".valid: 0x" << int_var__valid << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ready: 0x" << int_var__ready << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_read_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    ar.show();
    r.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_write_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    aw.show();
    w.show();
    b.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_he_axi_bundle_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    ar.show();
    r.show();
    aw.show();
    w.show();
    b.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_te_axi_addr_decoder_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".tbl_addr: 0x" << int_var__tbl_addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".tbl_id: 0x" << int_var__tbl_id << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".pktsize: 0x" << int_var__pktsize << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".color_bits: 0x" << int_var__color_bits << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".rsvd: 0x" << int_var__rsvd << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

int cap_axi_ar_bundle_t::get_width() const {
    return cap_axi_ar_bundle_t::s_get_width();

}

int cap_axi_r_bundle_t::get_width() const {
    return cap_axi_r_bundle_t::s_get_width();

}

int cap_axi_aw_bundle_t::get_width() const {
    return cap_axi_aw_bundle_t::s_get_width();

}

int cap_axi_w_bundle_t::get_width() const {
    return cap_axi_w_bundle_t::s_get_width();

}

int cap_axi_b_bundle_t::get_width() const {
    return cap_axi_b_bundle_t::s_get_width();

}

int cap_axi_read_bundle_t::get_width() const {
    return cap_axi_read_bundle_t::s_get_width();

}

int cap_axi_write_bundle_t::get_width() const {
    return cap_axi_write_bundle_t::s_get_width();

}

int cap_axi_bundle_t::get_width() const {
    return cap_axi_bundle_t::s_get_width();

}

int cap_te_axi_ar_bundle_t::get_width() const {
    return cap_te_axi_ar_bundle_t::s_get_width();

}

int cap_te_axi_r_bundle_t::get_width() const {
    return cap_te_axi_r_bundle_t::s_get_width();

}

int cap_te_axi_aw_bundle_t::get_width() const {
    return cap_te_axi_aw_bundle_t::s_get_width();

}

int cap_te_axi_w_bundle_t::get_width() const {
    return cap_te_axi_w_bundle_t::s_get_width();

}

int cap_te_axi_b_bundle_t::get_width() const {
    return cap_te_axi_b_bundle_t::s_get_width();

}

int cap_te_axi_read_bundle_t::get_width() const {
    return cap_te_axi_read_bundle_t::s_get_width();

}

int cap_te_axi_write_bundle_t::get_width() const {
    return cap_te_axi_write_bundle_t::s_get_width();

}

int cap_te_axi_bundle_t::get_width() const {
    return cap_te_axi_bundle_t::s_get_width();

}

int cap_he_axi_ar_bundle_t::get_width() const {
    return cap_he_axi_ar_bundle_t::s_get_width();

}

int cap_he_axi_r_bundle_t::get_width() const {
    return cap_he_axi_r_bundle_t::s_get_width();

}

int cap_he_axi_aw_bundle_t::get_width() const {
    return cap_he_axi_aw_bundle_t::s_get_width();

}

int cap_he_axi_w_bundle_t::get_width() const {
    return cap_he_axi_w_bundle_t::s_get_width();

}

int cap_he_axi_b_bundle_t::get_width() const {
    return cap_he_axi_b_bundle_t::s_get_width();

}

int cap_he_axi_read_bundle_t::get_width() const {
    return cap_he_axi_read_bundle_t::s_get_width();

}

int cap_he_axi_write_bundle_t::get_width() const {
    return cap_he_axi_write_bundle_t::s_get_width();

}

int cap_he_axi_bundle_t::get_width() const {
    return cap_he_axi_bundle_t::s_get_width();

}

int cap_te_axi_addr_decoder_t::get_width() const {
    return cap_te_axi_addr_decoder_t::s_get_width();

}

int cap_axi_ar_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 64; // addr
    _count += 4; // len
    _count += 3; // sz
    _count += 2; // burst
    _count += 1; // valid
    _count += 1; // ready
    _count += 3; // prot
    _count += 1; // lock
    _count += 4; // cache
    _count += 4; // qos
    return _count;
}

int cap_axi_r_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 2; // resp
    _count += 1; // valid
    _count += 1; // ready
    _count += 1; // last
    _count += 512; // data
    return _count;
}

int cap_axi_aw_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 64; // addr
    _count += 4; // len
    _count += 3; // sz
    _count += 2; // burst
    _count += 1; // valid
    _count += 1; // ready
    _count += 3; // prot
    _count += 1; // lock
    _count += 4; // cache
    _count += 4; // qos
    return _count;
}

int cap_axi_w_bundle_t::s_get_width() {
    int _count = 0;

    _count += 512; // data
    _count += 1; // last
    _count += 64; // strb
    _count += 1; // valid
    _count += 1; // ready
    return _count;
}

int cap_axi_b_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 2; // resp
    _count += 1; // valid
    _count += 1; // ready
    return _count;
}

int cap_axi_read_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_axi_ar_bundle_t::s_get_width(); // ar
    _count += cap_axi_r_bundle_t::s_get_width(); // r
    return _count;
}

int cap_axi_write_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_axi_aw_bundle_t::s_get_width(); // aw
    _count += cap_axi_w_bundle_t::s_get_width(); // w
    _count += cap_axi_b_bundle_t::s_get_width(); // b
    return _count;
}

int cap_axi_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_axi_ar_bundle_t::s_get_width(); // ar
    _count += cap_axi_r_bundle_t::s_get_width(); // r
    _count += cap_axi_aw_bundle_t::s_get_width(); // aw
    _count += cap_axi_w_bundle_t::s_get_width(); // w
    _count += cap_axi_b_bundle_t::s_get_width(); // b
    return _count;
}

int cap_te_axi_ar_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 65; // addr
    _count += 4; // len
    _count += 3; // sz
    _count += 2; // burst
    _count += 1; // valid
    _count += 1; // ready
    _count += 3; // prot
    _count += 1; // lock
    _count += 4; // cache
    _count += 4; // qos
    return _count;
}

int cap_te_axi_r_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 2; // resp
    _count += 1; // valid
    _count += 1; // ready
    _count += 1; // last
    _count += 512; // data
    return _count;
}

int cap_te_axi_aw_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 65; // addr
    _count += 4; // len
    _count += 3; // sz
    _count += 2; // burst
    _count += 1; // valid
    _count += 1; // ready
    _count += 3; // prot
    _count += 1; // lock
    _count += 4; // cache
    _count += 4; // qos
    return _count;
}

int cap_te_axi_w_bundle_t::s_get_width() {
    int _count = 0;

    _count += 512; // data
    _count += 1; // last
    _count += 64; // strb
    _count += 1; // valid
    _count += 1; // ready
    return _count;
}

int cap_te_axi_b_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 2; // resp
    _count += 1; // valid
    _count += 1; // ready
    return _count;
}

int cap_te_axi_read_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_te_axi_ar_bundle_t::s_get_width(); // ar
    _count += cap_te_axi_r_bundle_t::s_get_width(); // r
    return _count;
}

int cap_te_axi_write_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_te_axi_aw_bundle_t::s_get_width(); // aw
    _count += cap_te_axi_w_bundle_t::s_get_width(); // w
    _count += cap_te_axi_b_bundle_t::s_get_width(); // b
    return _count;
}

int cap_te_axi_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_te_axi_ar_bundle_t::s_get_width(); // ar
    _count += cap_te_axi_r_bundle_t::s_get_width(); // r
    _count += cap_te_axi_aw_bundle_t::s_get_width(); // aw
    _count += cap_te_axi_w_bundle_t::s_get_width(); // w
    _count += cap_te_axi_b_bundle_t::s_get_width(); // b
    return _count;
}

int cap_he_axi_ar_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 64; // addr
    _count += 32; // len
    _count += 3; // sz
    _count += 2; // burst
    _count += 1; // valid
    _count += 1; // ready
    _count += 3; // prot
    _count += 1; // lock
    _count += 4; // cache
    _count += 4; // qos
    return _count;
}

int cap_he_axi_r_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 2; // resp
    _count += 1; // valid
    _count += 1; // ready
    _count += 1; // last
    _count += 512; // data
    return _count;
}

int cap_he_axi_aw_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 64; // addr
    _count += 32; // len
    _count += 3; // sz
    _count += 2; // burst
    _count += 1; // valid
    _count += 1; // ready
    _count += 3; // prot
    _count += 1; // lock
    _count += 4; // cache
    _count += 4; // qos
    return _count;
}

int cap_he_axi_w_bundle_t::s_get_width() {
    int _count = 0;

    _count += 512; // data
    _count += 1; // last
    _count += 64; // strb
    _count += 1; // valid
    _count += 1; // ready
    return _count;
}

int cap_he_axi_b_bundle_t::s_get_width() {
    int _count = 0;

    _count += 7; // id
    _count += 2; // resp
    _count += 1; // valid
    _count += 1; // ready
    return _count;
}

int cap_he_axi_read_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_he_axi_ar_bundle_t::s_get_width(); // ar
    _count += cap_he_axi_r_bundle_t::s_get_width(); // r
    return _count;
}

int cap_he_axi_write_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_he_axi_aw_bundle_t::s_get_width(); // aw
    _count += cap_he_axi_w_bundle_t::s_get_width(); // w
    _count += cap_he_axi_b_bundle_t::s_get_width(); // b
    return _count;
}

int cap_he_axi_bundle_t::s_get_width() {
    int _count = 0;

    _count += cap_he_axi_ar_bundle_t::s_get_width(); // ar
    _count += cap_he_axi_r_bundle_t::s_get_width(); // r
    _count += cap_he_axi_aw_bundle_t::s_get_width(); // aw
    _count += cap_he_axi_w_bundle_t::s_get_width(); // w
    _count += cap_he_axi_b_bundle_t::s_get_width(); // b
    return _count;
}

int cap_te_axi_addr_decoder_t::s_get_width() {
    int _count = 0;

    _count += 21; // tbl_addr
    _count += 4; // tbl_id
    _count += 14; // pktsize
    _count += 2; // color_bits
    _count += 23; // rsvd
    return _count;
}

void cap_axi_ar_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_axi_r_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__last = _val.convert_to< last_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
}

void cap_axi_aw_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_axi_w_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
    int_var__last = _val.convert_to< last_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__strb = _val.convert_to< strb_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_axi_b_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_axi_read_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    ar.all( _val);
    _val = _val >> ar.get_width(); 
    r.all( _val);
    _val = _val >> r.get_width(); 
}

void cap_axi_write_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    aw.all( _val);
    _val = _val >> aw.get_width(); 
    w.all( _val);
    _val = _val >> w.get_width(); 
    b.all( _val);
    _val = _val >> b.get_width(); 
}

void cap_axi_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    ar.all( _val);
    _val = _val >> ar.get_width(); 
    r.all( _val);
    _val = _val >> r.get_width(); 
    aw.all( _val);
    _val = _val >> aw.get_width(); 
    w.all( _val);
    _val = _val >> w.get_width(); 
    b.all( _val);
    _val = _val >> b.get_width(); 
}

void cap_te_axi_ar_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 65;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_te_axi_r_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__last = _val.convert_to< last_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
}

void cap_te_axi_aw_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 65;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_te_axi_w_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
    int_var__last = _val.convert_to< last_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__strb = _val.convert_to< strb_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_axi_b_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_te_axi_read_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    ar.all( _val);
    _val = _val >> ar.get_width(); 
    r.all( _val);
    _val = _val >> r.get_width(); 
}

void cap_te_axi_write_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    aw.all( _val);
    _val = _val >> aw.get_width(); 
    w.all( _val);
    _val = _val >> w.get_width(); 
    b.all( _val);
    _val = _val >> b.get_width(); 
}

void cap_te_axi_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    ar.all( _val);
    _val = _val >> ar.get_width(); 
    r.all( _val);
    _val = _val >> r.get_width(); 
    aw.all( _val);
    _val = _val >> aw.get_width(); 
    w.all( _val);
    _val = _val >> w.get_width(); 
    b.all( _val);
    _val = _val >> b.get_width(); 
}

void cap_he_axi_ar_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_he_axi_r_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__last = _val.convert_to< last_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
}

void cap_he_axi_aw_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__len = _val.convert_to< len_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__sz = _val.convert_to< sz_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__burst = _val.convert_to< burst_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__prot = _val.convert_to< prot_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__lock = _val.convert_to< lock_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cache = _val.convert_to< cache_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__qos = _val.convert_to< qos_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_he_axi_w_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 512;
    
    int_var__last = _val.convert_to< last_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__strb = _val.convert_to< strb_cpp_int_t >()  ;
    _val = _val >> 64;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_he_axi_b_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__id = _val.convert_to< id_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__resp = _val.convert_to< resp_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__valid = _val.convert_to< valid_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ready = _val.convert_to< ready_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_he_axi_read_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    ar.all( _val);
    _val = _val >> ar.get_width(); 
    r.all( _val);
    _val = _val >> r.get_width(); 
}

void cap_he_axi_write_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    aw.all( _val);
    _val = _val >> aw.get_width(); 
    w.all( _val);
    _val = _val >> w.get_width(); 
    b.all( _val);
    _val = _val >> b.get_width(); 
}

void cap_he_axi_bundle_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    ar.all( _val);
    _val = _val >> ar.get_width(); 
    r.all( _val);
    _val = _val >> r.get_width(); 
    aw.all( _val);
    _val = _val >> aw.get_width(); 
    w.all( _val);
    _val = _val >> w.get_width(); 
    b.all( _val);
    _val = _val >> b.get_width(); 
}

void cap_te_axi_addr_decoder_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__tbl_addr = _val.convert_to< tbl_addr_cpp_int_t >()  ;
    _val = _val >> 21;
    
    int_var__tbl_id = _val.convert_to< tbl_id_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__pktsize = _val.convert_to< pktsize_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__color_bits = _val.convert_to< color_bits_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >()  ;
    _val = _val >> 23;
    
}

cpp_int cap_te_axi_addr_decoder_t::all() const {
    cpp_int ret_val;

    // rsvd
    ret_val = ret_val << 23; ret_val = ret_val  | int_var__rsvd; 
    
    // color_bits
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__color_bits; 
    
    // pktsize
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__pktsize; 
    
    // tbl_id
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__tbl_id; 
    
    // tbl_addr
    ret_val = ret_val << 21; ret_val = ret_val  | int_var__tbl_addr; 
    
    return ret_val;
}

cpp_int cap_he_axi_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << b.get_width(); ret_val = ret_val  | b.all(); 
    ret_val = ret_val << w.get_width(); ret_val = ret_val  | w.all(); 
    ret_val = ret_val << aw.get_width(); ret_val = ret_val  | aw.all(); 
    ret_val = ret_val << r.get_width(); ret_val = ret_val  | r.all(); 
    ret_val = ret_val << ar.get_width(); ret_val = ret_val  | ar.all(); 
    return ret_val;
}

cpp_int cap_he_axi_write_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << b.get_width(); ret_val = ret_val  | b.all(); 
    ret_val = ret_val << w.get_width(); ret_val = ret_val  | w.all(); 
    ret_val = ret_val << aw.get_width(); ret_val = ret_val  | aw.all(); 
    return ret_val;
}

cpp_int cap_he_axi_read_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << r.get_width(); ret_val = ret_val  | r.all(); 
    ret_val = ret_val << ar.get_width(); ret_val = ret_val  | ar.all(); 
    return ret_val;
}

cpp_int cap_he_axi_b_bundle_t::all() const {
    cpp_int ret_val;

    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_he_axi_w_bundle_t::all() const {
    cpp_int ret_val;

    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // strb
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__strb; 
    
    // last
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last; 
    
    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_he_axi_aw_bundle_t::all() const {
    cpp_int ret_val;

    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // cache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cache; 
    
    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // burst
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__burst; 
    
    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__len; 
    
    // addr
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__addr; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_he_axi_r_bundle_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    // last
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_he_axi_ar_bundle_t::all() const {
    cpp_int ret_val;

    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // cache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cache; 
    
    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // burst
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__burst; 
    
    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__len; 
    
    // addr
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__addr; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_te_axi_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << b.get_width(); ret_val = ret_val  | b.all(); 
    ret_val = ret_val << w.get_width(); ret_val = ret_val  | w.all(); 
    ret_val = ret_val << aw.get_width(); ret_val = ret_val  | aw.all(); 
    ret_val = ret_val << r.get_width(); ret_val = ret_val  | r.all(); 
    ret_val = ret_val << ar.get_width(); ret_val = ret_val  | ar.all(); 
    return ret_val;
}

cpp_int cap_te_axi_write_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << b.get_width(); ret_val = ret_val  | b.all(); 
    ret_val = ret_val << w.get_width(); ret_val = ret_val  | w.all(); 
    ret_val = ret_val << aw.get_width(); ret_val = ret_val  | aw.all(); 
    return ret_val;
}

cpp_int cap_te_axi_read_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << r.get_width(); ret_val = ret_val  | r.all(); 
    ret_val = ret_val << ar.get_width(); ret_val = ret_val  | ar.all(); 
    return ret_val;
}

cpp_int cap_te_axi_b_bundle_t::all() const {
    cpp_int ret_val;

    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_te_axi_w_bundle_t::all() const {
    cpp_int ret_val;

    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // strb
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__strb; 
    
    // last
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last; 
    
    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_te_axi_aw_bundle_t::all() const {
    cpp_int ret_val;

    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // cache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cache; 
    
    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // burst
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__burst; 
    
    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len; 
    
    // addr
    ret_val = ret_val << 65; ret_val = ret_val  | int_var__addr; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_te_axi_r_bundle_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    // last
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_te_axi_ar_bundle_t::all() const {
    cpp_int ret_val;

    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // cache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cache; 
    
    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // burst
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__burst; 
    
    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len; 
    
    // addr
    ret_val = ret_val << 65; ret_val = ret_val  | int_var__addr; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_axi_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << b.get_width(); ret_val = ret_val  | b.all(); 
    ret_val = ret_val << w.get_width(); ret_val = ret_val  | w.all(); 
    ret_val = ret_val << aw.get_width(); ret_val = ret_val  | aw.all(); 
    ret_val = ret_val << r.get_width(); ret_val = ret_val  | r.all(); 
    ret_val = ret_val << ar.get_width(); ret_val = ret_val  | ar.all(); 
    return ret_val;
}

cpp_int cap_axi_write_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << b.get_width(); ret_val = ret_val  | b.all(); 
    ret_val = ret_val << w.get_width(); ret_val = ret_val  | w.all(); 
    ret_val = ret_val << aw.get_width(); ret_val = ret_val  | aw.all(); 
    return ret_val;
}

cpp_int cap_axi_read_bundle_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << r.get_width(); ret_val = ret_val  | r.all(); 
    ret_val = ret_val << ar.get_width(); ret_val = ret_val  | ar.all(); 
    return ret_val;
}

cpp_int cap_axi_b_bundle_t::all() const {
    cpp_int ret_val;

    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_axi_w_bundle_t::all() const {
    cpp_int ret_val;

    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // strb
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__strb; 
    
    // last
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last; 
    
    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_axi_aw_bundle_t::all() const {
    cpp_int ret_val;

    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // cache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cache; 
    
    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // burst
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__burst; 
    
    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len; 
    
    // addr
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__addr; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_axi_r_bundle_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 512; ret_val = ret_val  | int_var__data; 
    
    // last
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__last; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // resp
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__resp; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

cpp_int cap_axi_ar_bundle_t::all() const {
    cpp_int ret_val;

    // qos
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__qos; 
    
    // cache
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__cache; 
    
    // lock
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__lock; 
    
    // prot
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__prot; 
    
    // ready
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ready; 
    
    // valid
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__valid; 
    
    // burst
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__burst; 
    
    // sz
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__sz; 
    
    // len
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__len; 
    
    // addr
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__addr; 
    
    // id
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__id; 
    
    return ret_val;
}

void cap_axi_ar_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__addr = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
    int_var__burst = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__prot = 0; 
    
    int_var__lock = 0; 
    
    int_var__cache = 0; 
    
    int_var__qos = 0; 
    
}

void cap_axi_r_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__resp = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__last = 0; 
    
    int_var__data = 0; 
    
}

void cap_axi_aw_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__addr = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
    int_var__burst = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__prot = 0; 
    
    int_var__lock = 0; 
    
    int_var__cache = 0; 
    
    int_var__qos = 0; 
    
}

void cap_axi_w_bundle_t::clear() {

    int_var__data = 0; 
    
    int_var__last = 0; 
    
    int_var__strb = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
}

void cap_axi_b_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__resp = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
}

void cap_axi_read_bundle_t::clear() {

    ar.clear();
    r.clear();
}

void cap_axi_write_bundle_t::clear() {

    aw.clear();
    w.clear();
    b.clear();
}

void cap_axi_bundle_t::clear() {

    ar.clear();
    r.clear();
    aw.clear();
    w.clear();
    b.clear();
}

void cap_te_axi_ar_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__addr = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
    int_var__burst = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__prot = 0; 
    
    int_var__lock = 0; 
    
    int_var__cache = 0; 
    
    int_var__qos = 0; 
    
}

void cap_te_axi_r_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__resp = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__last = 0; 
    
    int_var__data = 0; 
    
}

void cap_te_axi_aw_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__addr = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
    int_var__burst = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__prot = 0; 
    
    int_var__lock = 0; 
    
    int_var__cache = 0; 
    
    int_var__qos = 0; 
    
}

void cap_te_axi_w_bundle_t::clear() {

    int_var__data = 0; 
    
    int_var__last = 0; 
    
    int_var__strb = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
}

void cap_te_axi_b_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__resp = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
}

void cap_te_axi_read_bundle_t::clear() {

    ar.clear();
    r.clear();
}

void cap_te_axi_write_bundle_t::clear() {

    aw.clear();
    w.clear();
    b.clear();
}

void cap_te_axi_bundle_t::clear() {

    ar.clear();
    r.clear();
    aw.clear();
    w.clear();
    b.clear();
}

void cap_he_axi_ar_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__addr = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
    int_var__burst = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__prot = 0; 
    
    int_var__lock = 0; 
    
    int_var__cache = 0; 
    
    int_var__qos = 0; 
    
}

void cap_he_axi_r_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__resp = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__last = 0; 
    
    int_var__data = 0; 
    
}

void cap_he_axi_aw_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__addr = 0; 
    
    int_var__len = 0; 
    
    int_var__sz = 0; 
    
    int_var__burst = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
    int_var__prot = 0; 
    
    int_var__lock = 0; 
    
    int_var__cache = 0; 
    
    int_var__qos = 0; 
    
}

void cap_he_axi_w_bundle_t::clear() {

    int_var__data = 0; 
    
    int_var__last = 0; 
    
    int_var__strb = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
}

void cap_he_axi_b_bundle_t::clear() {

    int_var__id = 0; 
    
    int_var__resp = 0; 
    
    int_var__valid = 0; 
    
    int_var__ready = 0; 
    
}

void cap_he_axi_read_bundle_t::clear() {

    ar.clear();
    r.clear();
}

void cap_he_axi_write_bundle_t::clear() {

    aw.clear();
    w.clear();
    b.clear();
}

void cap_he_axi_bundle_t::clear() {

    ar.clear();
    r.clear();
    aw.clear();
    w.clear();
    b.clear();
}

void cap_te_axi_addr_decoder_t::clear() {

    int_var__tbl_addr = 0; 
    
    int_var__tbl_id = 0; 
    
    int_var__pktsize = 0; 
    
    int_var__color_bits = 0; 
    
    int_var__rsvd = 0; 
    
}

void cap_axi_ar_bundle_t::init() {

}

void cap_axi_r_bundle_t::init() {

}

void cap_axi_aw_bundle_t::init() {

}

void cap_axi_w_bundle_t::init() {

}

void cap_axi_b_bundle_t::init() {

}

void cap_axi_read_bundle_t::init() {

    ar.set_attributes(this,"ar", 0x0 );
    r.set_attributes(this,"r", 0x0 );
}

void cap_axi_write_bundle_t::init() {

    aw.set_attributes(this,"aw", 0x0 );
    w.set_attributes(this,"w", 0x0 );
    b.set_attributes(this,"b", 0x0 );
}

void cap_axi_bundle_t::init() {

    ar.set_attributes(this,"ar", 0x0 );
    r.set_attributes(this,"r", 0x0 );
    aw.set_attributes(this,"aw", 0x0 );
    w.set_attributes(this,"w", 0x0 );
    b.set_attributes(this,"b", 0x0 );
}

void cap_te_axi_ar_bundle_t::init() {

}

void cap_te_axi_r_bundle_t::init() {

}

void cap_te_axi_aw_bundle_t::init() {

}

void cap_te_axi_w_bundle_t::init() {

}

void cap_te_axi_b_bundle_t::init() {

}

void cap_te_axi_read_bundle_t::init() {

    ar.set_attributes(this,"ar", 0x0 );
    r.set_attributes(this,"r", 0x0 );
}

void cap_te_axi_write_bundle_t::init() {

    aw.set_attributes(this,"aw", 0x0 );
    w.set_attributes(this,"w", 0x0 );
    b.set_attributes(this,"b", 0x0 );
}

void cap_te_axi_bundle_t::init() {

    ar.set_attributes(this,"ar", 0x0 );
    r.set_attributes(this,"r", 0x0 );
    aw.set_attributes(this,"aw", 0x0 );
    w.set_attributes(this,"w", 0x0 );
    b.set_attributes(this,"b", 0x0 );
}

void cap_he_axi_ar_bundle_t::init() {

}

void cap_he_axi_r_bundle_t::init() {

}

void cap_he_axi_aw_bundle_t::init() {

}

void cap_he_axi_w_bundle_t::init() {

}

void cap_he_axi_b_bundle_t::init() {

}

void cap_he_axi_read_bundle_t::init() {

    ar.set_attributes(this,"ar", 0x0 );
    r.set_attributes(this,"r", 0x0 );
}

void cap_he_axi_write_bundle_t::init() {

    aw.set_attributes(this,"aw", 0x0 );
    w.set_attributes(this,"w", 0x0 );
    b.set_attributes(this,"b", 0x0 );
}

void cap_he_axi_bundle_t::init() {

    ar.set_attributes(this,"ar", 0x0 );
    r.set_attributes(this,"r", 0x0 );
    aw.set_attributes(this,"aw", 0x0 );
    w.set_attributes(this,"w", 0x0 );
    b.set_attributes(this,"b", 0x0 );
}

void cap_te_axi_addr_decoder_t::init() {

}

void cap_axi_ar_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::id() const {
    return int_var__id;
}
    
void cap_axi_ar_bundle_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::addr() const {
    return int_var__addr;
}
    
void cap_axi_ar_bundle_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::len() const {
    return int_var__len;
}
    
void cap_axi_ar_bundle_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::sz() const {
    return int_var__sz;
}
    
void cap_axi_ar_bundle_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::burst() const {
    return int_var__burst;
}
    
void cap_axi_ar_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_axi_ar_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_axi_ar_bundle_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::prot() const {
    return int_var__prot;
}
    
void cap_axi_ar_bundle_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::lock() const {
    return int_var__lock;
}
    
void cap_axi_ar_bundle_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::cache() const {
    return int_var__cache;
}
    
void cap_axi_ar_bundle_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_axi_ar_bundle_t::qos() const {
    return int_var__qos;
}
    
void cap_axi_r_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_axi_r_bundle_t::id() const {
    return int_var__id;
}
    
void cap_axi_r_bundle_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_axi_r_bundle_t::resp() const {
    return int_var__resp;
}
    
void cap_axi_r_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_axi_r_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_axi_r_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_axi_r_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_axi_r_bundle_t::last(const cpp_int & _val) { 
    // last
    int_var__last = _val.convert_to< last_cpp_int_t >();
}

cpp_int cap_axi_r_bundle_t::last() const {
    return int_var__last;
}
    
void cap_axi_r_bundle_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_axi_r_bundle_t::data() const {
    return int_var__data;
}
    
void cap_axi_aw_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::id() const {
    return int_var__id;
}
    
void cap_axi_aw_bundle_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::addr() const {
    return int_var__addr;
}
    
void cap_axi_aw_bundle_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::len() const {
    return int_var__len;
}
    
void cap_axi_aw_bundle_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::sz() const {
    return int_var__sz;
}
    
void cap_axi_aw_bundle_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::burst() const {
    return int_var__burst;
}
    
void cap_axi_aw_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_axi_aw_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_axi_aw_bundle_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::prot() const {
    return int_var__prot;
}
    
void cap_axi_aw_bundle_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::lock() const {
    return int_var__lock;
}
    
void cap_axi_aw_bundle_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::cache() const {
    return int_var__cache;
}
    
void cap_axi_aw_bundle_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_axi_aw_bundle_t::qos() const {
    return int_var__qos;
}
    
void cap_axi_w_bundle_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_axi_w_bundle_t::data() const {
    return int_var__data;
}
    
void cap_axi_w_bundle_t::last(const cpp_int & _val) { 
    // last
    int_var__last = _val.convert_to< last_cpp_int_t >();
}

cpp_int cap_axi_w_bundle_t::last() const {
    return int_var__last;
}
    
void cap_axi_w_bundle_t::strb(const cpp_int & _val) { 
    // strb
    int_var__strb = _val.convert_to< strb_cpp_int_t >();
}

cpp_int cap_axi_w_bundle_t::strb() const {
    return int_var__strb;
}
    
void cap_axi_w_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_axi_w_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_axi_w_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_axi_w_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_axi_b_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_axi_b_bundle_t::id() const {
    return int_var__id;
}
    
void cap_axi_b_bundle_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_axi_b_bundle_t::resp() const {
    return int_var__resp;
}
    
void cap_axi_b_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_axi_b_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_axi_b_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_axi_b_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_te_axi_ar_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::id() const {
    return int_var__id;
}
    
void cap_te_axi_ar_bundle_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::addr() const {
    return int_var__addr;
}
    
void cap_te_axi_ar_bundle_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::len() const {
    return int_var__len;
}
    
void cap_te_axi_ar_bundle_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::sz() const {
    return int_var__sz;
}
    
void cap_te_axi_ar_bundle_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::burst() const {
    return int_var__burst;
}
    
void cap_te_axi_ar_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_te_axi_ar_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_te_axi_ar_bundle_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::prot() const {
    return int_var__prot;
}
    
void cap_te_axi_ar_bundle_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::lock() const {
    return int_var__lock;
}
    
void cap_te_axi_ar_bundle_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::cache() const {
    return int_var__cache;
}
    
void cap_te_axi_ar_bundle_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_te_axi_ar_bundle_t::qos() const {
    return int_var__qos;
}
    
void cap_te_axi_r_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_te_axi_r_bundle_t::id() const {
    return int_var__id;
}
    
void cap_te_axi_r_bundle_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_te_axi_r_bundle_t::resp() const {
    return int_var__resp;
}
    
void cap_te_axi_r_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_axi_r_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_te_axi_r_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_te_axi_r_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_te_axi_r_bundle_t::last(const cpp_int & _val) { 
    // last
    int_var__last = _val.convert_to< last_cpp_int_t >();
}

cpp_int cap_te_axi_r_bundle_t::last() const {
    return int_var__last;
}
    
void cap_te_axi_r_bundle_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_te_axi_r_bundle_t::data() const {
    return int_var__data;
}
    
void cap_te_axi_aw_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::id() const {
    return int_var__id;
}
    
void cap_te_axi_aw_bundle_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::addr() const {
    return int_var__addr;
}
    
void cap_te_axi_aw_bundle_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::len() const {
    return int_var__len;
}
    
void cap_te_axi_aw_bundle_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::sz() const {
    return int_var__sz;
}
    
void cap_te_axi_aw_bundle_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::burst() const {
    return int_var__burst;
}
    
void cap_te_axi_aw_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_te_axi_aw_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_te_axi_aw_bundle_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::prot() const {
    return int_var__prot;
}
    
void cap_te_axi_aw_bundle_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::lock() const {
    return int_var__lock;
}
    
void cap_te_axi_aw_bundle_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::cache() const {
    return int_var__cache;
}
    
void cap_te_axi_aw_bundle_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_te_axi_aw_bundle_t::qos() const {
    return int_var__qos;
}
    
void cap_te_axi_w_bundle_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_te_axi_w_bundle_t::data() const {
    return int_var__data;
}
    
void cap_te_axi_w_bundle_t::last(const cpp_int & _val) { 
    // last
    int_var__last = _val.convert_to< last_cpp_int_t >();
}

cpp_int cap_te_axi_w_bundle_t::last() const {
    return int_var__last;
}
    
void cap_te_axi_w_bundle_t::strb(const cpp_int & _val) { 
    // strb
    int_var__strb = _val.convert_to< strb_cpp_int_t >();
}

cpp_int cap_te_axi_w_bundle_t::strb() const {
    return int_var__strb;
}
    
void cap_te_axi_w_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_axi_w_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_te_axi_w_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_te_axi_w_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_te_axi_b_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_te_axi_b_bundle_t::id() const {
    return int_var__id;
}
    
void cap_te_axi_b_bundle_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_te_axi_b_bundle_t::resp() const {
    return int_var__resp;
}
    
void cap_te_axi_b_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_te_axi_b_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_te_axi_b_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_te_axi_b_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_he_axi_ar_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::id() const {
    return int_var__id;
}
    
void cap_he_axi_ar_bundle_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::addr() const {
    return int_var__addr;
}
    
void cap_he_axi_ar_bundle_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::len() const {
    return int_var__len;
}
    
void cap_he_axi_ar_bundle_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::sz() const {
    return int_var__sz;
}
    
void cap_he_axi_ar_bundle_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::burst() const {
    return int_var__burst;
}
    
void cap_he_axi_ar_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_he_axi_ar_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_he_axi_ar_bundle_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::prot() const {
    return int_var__prot;
}
    
void cap_he_axi_ar_bundle_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::lock() const {
    return int_var__lock;
}
    
void cap_he_axi_ar_bundle_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::cache() const {
    return int_var__cache;
}
    
void cap_he_axi_ar_bundle_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_he_axi_ar_bundle_t::qos() const {
    return int_var__qos;
}
    
void cap_he_axi_r_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_he_axi_r_bundle_t::id() const {
    return int_var__id;
}
    
void cap_he_axi_r_bundle_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_he_axi_r_bundle_t::resp() const {
    return int_var__resp;
}
    
void cap_he_axi_r_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_he_axi_r_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_he_axi_r_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_he_axi_r_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_he_axi_r_bundle_t::last(const cpp_int & _val) { 
    // last
    int_var__last = _val.convert_to< last_cpp_int_t >();
}

cpp_int cap_he_axi_r_bundle_t::last() const {
    return int_var__last;
}
    
void cap_he_axi_r_bundle_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_he_axi_r_bundle_t::data() const {
    return int_var__data;
}
    
void cap_he_axi_aw_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::id() const {
    return int_var__id;
}
    
void cap_he_axi_aw_bundle_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::addr() const {
    return int_var__addr;
}
    
void cap_he_axi_aw_bundle_t::len(const cpp_int & _val) { 
    // len
    int_var__len = _val.convert_to< len_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::len() const {
    return int_var__len;
}
    
void cap_he_axi_aw_bundle_t::sz(const cpp_int & _val) { 
    // sz
    int_var__sz = _val.convert_to< sz_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::sz() const {
    return int_var__sz;
}
    
void cap_he_axi_aw_bundle_t::burst(const cpp_int & _val) { 
    // burst
    int_var__burst = _val.convert_to< burst_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::burst() const {
    return int_var__burst;
}
    
void cap_he_axi_aw_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_he_axi_aw_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_he_axi_aw_bundle_t::prot(const cpp_int & _val) { 
    // prot
    int_var__prot = _val.convert_to< prot_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::prot() const {
    return int_var__prot;
}
    
void cap_he_axi_aw_bundle_t::lock(const cpp_int & _val) { 
    // lock
    int_var__lock = _val.convert_to< lock_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::lock() const {
    return int_var__lock;
}
    
void cap_he_axi_aw_bundle_t::cache(const cpp_int & _val) { 
    // cache
    int_var__cache = _val.convert_to< cache_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::cache() const {
    return int_var__cache;
}
    
void cap_he_axi_aw_bundle_t::qos(const cpp_int & _val) { 
    // qos
    int_var__qos = _val.convert_to< qos_cpp_int_t >();
}

cpp_int cap_he_axi_aw_bundle_t::qos() const {
    return int_var__qos;
}
    
void cap_he_axi_w_bundle_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_he_axi_w_bundle_t::data() const {
    return int_var__data;
}
    
void cap_he_axi_w_bundle_t::last(const cpp_int & _val) { 
    // last
    int_var__last = _val.convert_to< last_cpp_int_t >();
}

cpp_int cap_he_axi_w_bundle_t::last() const {
    return int_var__last;
}
    
void cap_he_axi_w_bundle_t::strb(const cpp_int & _val) { 
    // strb
    int_var__strb = _val.convert_to< strb_cpp_int_t >();
}

cpp_int cap_he_axi_w_bundle_t::strb() const {
    return int_var__strb;
}
    
void cap_he_axi_w_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_he_axi_w_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_he_axi_w_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_he_axi_w_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_he_axi_b_bundle_t::id(const cpp_int & _val) { 
    // id
    int_var__id = _val.convert_to< id_cpp_int_t >();
}

cpp_int cap_he_axi_b_bundle_t::id() const {
    return int_var__id;
}
    
void cap_he_axi_b_bundle_t::resp(const cpp_int & _val) { 
    // resp
    int_var__resp = _val.convert_to< resp_cpp_int_t >();
}

cpp_int cap_he_axi_b_bundle_t::resp() const {
    return int_var__resp;
}
    
void cap_he_axi_b_bundle_t::valid(const cpp_int & _val) { 
    // valid
    int_var__valid = _val.convert_to< valid_cpp_int_t >();
}

cpp_int cap_he_axi_b_bundle_t::valid() const {
    return int_var__valid;
}
    
void cap_he_axi_b_bundle_t::ready(const cpp_int & _val) { 
    // ready
    int_var__ready = _val.convert_to< ready_cpp_int_t >();
}

cpp_int cap_he_axi_b_bundle_t::ready() const {
    return int_var__ready;
}
    
void cap_te_axi_addr_decoder_t::tbl_addr(const cpp_int & _val) { 
    // tbl_addr
    int_var__tbl_addr = _val.convert_to< tbl_addr_cpp_int_t >();
}

cpp_int cap_te_axi_addr_decoder_t::tbl_addr() const {
    return int_var__tbl_addr;
}
    
void cap_te_axi_addr_decoder_t::tbl_id(const cpp_int & _val) { 
    // tbl_id
    int_var__tbl_id = _val.convert_to< tbl_id_cpp_int_t >();
}

cpp_int cap_te_axi_addr_decoder_t::tbl_id() const {
    return int_var__tbl_id;
}
    
void cap_te_axi_addr_decoder_t::pktsize(const cpp_int & _val) { 
    // pktsize
    int_var__pktsize = _val.convert_to< pktsize_cpp_int_t >();
}

cpp_int cap_te_axi_addr_decoder_t::pktsize() const {
    return int_var__pktsize;
}
    
void cap_te_axi_addr_decoder_t::color_bits(const cpp_int & _val) { 
    // color_bits
    int_var__color_bits = _val.convert_to< color_bits_cpp_int_t >();
}

cpp_int cap_te_axi_addr_decoder_t::color_bits() const {
    return int_var__color_bits;
}
    
void cap_te_axi_addr_decoder_t::rsvd(const cpp_int & _val) { 
    // rsvd
    int_var__rsvd = _val.convert_to< rsvd_cpp_int_t >();
}

cpp_int cap_te_axi_addr_decoder_t::rsvd() const {
    return int_var__rsvd;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_ar_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_r_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { field_val = last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_aw_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_w_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { field_val = last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "strb")) { field_val = strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_b_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_read_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = ar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_write_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = aw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = ar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = aw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_ar_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_r_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { field_val = last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_aw_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_w_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { field_val = last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "strb")) { field_val = strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_b_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_read_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = ar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_write_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = aw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = ar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = aw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_ar_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_r_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { field_val = last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_aw_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { field_val = len(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { field_val = sz(); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { field_val = burst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { field_val = prot(); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { field_val = lock(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { field_val = cache(); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { field_val = qos(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_w_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { field_val = last(); field_found=1; }
    if(!field_found && !strcmp(field_name, "strb")) { field_val = strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_b_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { field_val = id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { field_val = resp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { field_val = valid(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { field_val = ready(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_read_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = ar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_write_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = aw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_bundle_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = ar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = aw.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_addr_decoder_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tbl_addr")) { field_val = tbl_addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbl_id")) { field_val = tbl_id(); field_found=1; }
    if(!field_found && !strcmp(field_name, "pktsize")) { field_val = pktsize(); field_found=1; }
    if(!field_found && !strcmp(field_name, "color_bits")) { field_val = color_bits(); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { field_val = rsvd(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_ar_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_r_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_aw_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_w_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "strb")) { strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_b_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_read_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = ar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_write_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = aw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_axi_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = ar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = aw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_ar_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_r_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_aw_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_w_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "strb")) { strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_b_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_read_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = ar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_write_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = aw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = ar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = aw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_ar_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_r_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_aw_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "len")) { len(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sz")) { sz(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "burst")) { burst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "prot")) { prot(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "lock")) { lock(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cache")) { cache(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "qos")) { qos(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_w_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "last")) { last(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "strb")) { strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_b_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "id")) { id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "resp")) { resp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "valid")) { valid(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ready")) { ready(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_read_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = ar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_write_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = aw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_he_axi_bundle_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = ar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = r.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = aw.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = w.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = b.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_te_axi_addr_decoder_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "tbl_addr")) { tbl_addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "tbl_id")) { tbl_id(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "pktsize")) { pktsize(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "color_bits")) { color_bits(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "rsvd")) { rsvd(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_ar_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("addr");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    ret_vec.push_back("burst");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("prot");
    ret_vec.push_back("lock");
    ret_vec.push_back("cache");
    ret_vec.push_back("qos");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_r_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("resp");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("last");
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_aw_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("addr");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    ret_vec.push_back("burst");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("prot");
    ret_vec.push_back("lock");
    ret_vec.push_back("cache");
    ret_vec.push_back("qos");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_w_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("last");
    ret_vec.push_back("strb");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_b_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("resp");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_read_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : ar.get_fields(level-1)) {
            ret_vec.push_back("ar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : r.get_fields(level-1)) {
            ret_vec.push_back("r." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_write_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : aw.get_fields(level-1)) {
            ret_vec.push_back("aw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : w.get_fields(level-1)) {
            ret_vec.push_back("w." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : b.get_fields(level-1)) {
            ret_vec.push_back("b." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_axi_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : ar.get_fields(level-1)) {
            ret_vec.push_back("ar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : r.get_fields(level-1)) {
            ret_vec.push_back("r." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : aw.get_fields(level-1)) {
            ret_vec.push_back("aw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : w.get_fields(level-1)) {
            ret_vec.push_back("w." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : b.get_fields(level-1)) {
            ret_vec.push_back("b." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_ar_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("addr");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    ret_vec.push_back("burst");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("prot");
    ret_vec.push_back("lock");
    ret_vec.push_back("cache");
    ret_vec.push_back("qos");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_r_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("resp");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("last");
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_aw_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("addr");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    ret_vec.push_back("burst");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("prot");
    ret_vec.push_back("lock");
    ret_vec.push_back("cache");
    ret_vec.push_back("qos");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_w_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("last");
    ret_vec.push_back("strb");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_b_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("resp");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_read_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : ar.get_fields(level-1)) {
            ret_vec.push_back("ar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : r.get_fields(level-1)) {
            ret_vec.push_back("r." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_write_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : aw.get_fields(level-1)) {
            ret_vec.push_back("aw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : w.get_fields(level-1)) {
            ret_vec.push_back("w." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : b.get_fields(level-1)) {
            ret_vec.push_back("b." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : ar.get_fields(level-1)) {
            ret_vec.push_back("ar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : r.get_fields(level-1)) {
            ret_vec.push_back("r." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : aw.get_fields(level-1)) {
            ret_vec.push_back("aw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : w.get_fields(level-1)) {
            ret_vec.push_back("w." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : b.get_fields(level-1)) {
            ret_vec.push_back("b." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_ar_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("addr");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    ret_vec.push_back("burst");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("prot");
    ret_vec.push_back("lock");
    ret_vec.push_back("cache");
    ret_vec.push_back("qos");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_r_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("resp");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("last");
    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_aw_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("addr");
    ret_vec.push_back("len");
    ret_vec.push_back("sz");
    ret_vec.push_back("burst");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    ret_vec.push_back("prot");
    ret_vec.push_back("lock");
    ret_vec.push_back("cache");
    ret_vec.push_back("qos");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_w_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("last");
    ret_vec.push_back("strb");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_b_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("id");
    ret_vec.push_back("resp");
    ret_vec.push_back("valid");
    ret_vec.push_back("ready");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_read_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : ar.get_fields(level-1)) {
            ret_vec.push_back("ar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : r.get_fields(level-1)) {
            ret_vec.push_back("r." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_write_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : aw.get_fields(level-1)) {
            ret_vec.push_back("aw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : w.get_fields(level-1)) {
            ret_vec.push_back("w." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : b.get_fields(level-1)) {
            ret_vec.push_back("b." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_he_axi_bundle_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : ar.get_fields(level-1)) {
            ret_vec.push_back("ar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : r.get_fields(level-1)) {
            ret_vec.push_back("r." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : aw.get_fields(level-1)) {
            ret_vec.push_back("aw." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : w.get_fields(level-1)) {
            ret_vec.push_back("w." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : b.get_fields(level-1)) {
            ret_vec.push_back("b." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_te_axi_addr_decoder_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("tbl_addr");
    ret_vec.push_back("tbl_id");
    ret_vec.push_back("pktsize");
    ret_vec.push_back("color_bits");
    ret_vec.push_back("rsvd");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
