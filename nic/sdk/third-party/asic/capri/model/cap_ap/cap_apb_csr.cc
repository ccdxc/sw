
#include "cap_apb_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_apb_csr_dhs_bypass_entry_t::cap_apb_csr_dhs_bypass_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_apb_csr_dhs_bypass_entry_t::~cap_apb_csr_dhs_bypass_entry_t() { }

cap_apb_csr_dhs_bypass_t::cap_apb_csr_dhs_bypass_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_apb_csr_dhs_bypass_t::~cap_apb_csr_dhs_bypass_t() { }

cap_apb_csr_dhs_psys_entry_t::cap_apb_csr_dhs_psys_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_apb_csr_dhs_psys_entry_t::~cap_apb_csr_dhs_psys_entry_t() { }

cap_apb_csr_dhs_psys_t::cap_apb_csr_dhs_psys_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_apb_csr_dhs_psys_t::~cap_apb_csr_dhs_psys_t() { }

cap_apb_csr_cfg_dap_t::cap_apb_csr_cfg_dap_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_apb_csr_cfg_dap_t::~cap_apb_csr_cfg_dap_t() { }

cap_apb_csr_sta_dap_t::cap_apb_csr_sta_dap_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_apb_csr_sta_dap_t::~cap_apb_csr_sta_dap_t() { }

cap_apb_csr_t::cap_apb_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(1048576);
        set_attributes(0,get_name(), 0);
        }
cap_apb_csr_t::~cap_apb_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_dhs_bypass_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_dhs_bypass_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_dhs_psys_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_dhs_psys_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_cfg_dap_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".PCLKENSYS: 0x" << int_var__PCLKENSYS << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PRESETSYSn: 0x" << int_var__PRESETSYSn << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PRESETDBGn: 0x" << int_var__PRESETDBGn << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CDBGPWRUPACK: 0x" << int_var__CDBGPWRUPACK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CSYSPWRUPACK: 0x" << int_var__CSYSPWRUPACK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CDBGRSTACK: 0x" << int_var__CDBGRSTACK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nCDBGPWRDN: 0x" << int_var__nCDBGPWRDN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nCSOCPWRDN: 0x" << int_var__nCSOCPWRDN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".DEVICEEN: 0x" << int_var__DEVICEEN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".psys_paddr_indir: 0x" << int_var__psys_paddr_indir << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PCLKENDBG: 0x" << int_var__PCLKENDBG << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nPOTRST: 0x" << int_var__nPOTRST << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PADDRDBG_mask: 0x" << int_var__PADDRDBG_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PADDRDBG_or: 0x" << int_var__PADDRDBG_or << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bypass_debug: 0x" << int_var__bypass_debug << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_sta_dap_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".CDBGPWRUPREQ: 0x" << int_var__CDBGPWRUPREQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CDBGRSTREQ: 0x" << int_var__CDBGRSTREQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CSYSPWRUPREQ: 0x" << int_var__CSYSPWRUPREQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".DBGSWENABLE: 0x" << int_var__DBGSWENABLE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".JTAGNSW: 0x" << int_var__JTAGNSW << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".JTAGTOP: 0x" << int_var__JTAGTOP << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PSLVERRSYS: 0x" << int_var__PSLVERRSYS << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PSLVERRDBG: 0x" << int_var__PSLVERRDBG << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_apb_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    sta_dap.show();
    cfg_dap.show();
    dhs_psys.show();
    dhs_bypass.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_apb_csr_dhs_bypass_entry_t::get_width() const {
    return cap_apb_csr_dhs_bypass_entry_t::s_get_width();

}

int cap_apb_csr_dhs_bypass_t::get_width() const {
    return cap_apb_csr_dhs_bypass_t::s_get_width();

}

int cap_apb_csr_dhs_psys_entry_t::get_width() const {
    return cap_apb_csr_dhs_psys_entry_t::s_get_width();

}

int cap_apb_csr_dhs_psys_t::get_width() const {
    return cap_apb_csr_dhs_psys_t::s_get_width();

}

int cap_apb_csr_cfg_dap_t::get_width() const {
    return cap_apb_csr_cfg_dap_t::s_get_width();

}

int cap_apb_csr_sta_dap_t::get_width() const {
    return cap_apb_csr_sta_dap_t::s_get_width();

}

int cap_apb_csr_t::get_width() const {
    return cap_apb_csr_t::s_get_width();

}

int cap_apb_csr_dhs_bypass_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_apb_csr_dhs_bypass_t::s_get_width() {
    int _count = 0;

    _count += (cap_apb_csr_dhs_bypass_entry_t::s_get_width() * 65536); // entry
    return _count;
}

int cap_apb_csr_dhs_psys_entry_t::s_get_width() {
    int _count = 0;

    _count += 32; // data
    return _count;
}

int cap_apb_csr_dhs_psys_t::s_get_width() {
    int _count = 0;

    _count += (cap_apb_csr_dhs_psys_entry_t::s_get_width() * 65536); // entry
    return _count;
}

int cap_apb_csr_cfg_dap_t::s_get_width() {
    int _count = 0;

    _count += 1; // PCLKENSYS
    _count += 1; // PRESETSYSn
    _count += 1; // PRESETDBGn
    _count += 1; // CDBGPWRUPACK
    _count += 1; // CSYSPWRUPACK
    _count += 1; // CDBGRSTACK
    _count += 1; // nCDBGPWRDN
    _count += 1; // nCSOCPWRDN
    _count += 1; // DEVICEEN
    _count += 14; // psys_paddr_indir
    _count += 1; // PCLKENDBG
    _count += 1; // nPOTRST
    _count += 32; // PADDRDBG_mask
    _count += 32; // PADDRDBG_or
    _count += 1; // bypass_debug
    return _count;
}

int cap_apb_csr_sta_dap_t::s_get_width() {
    int _count = 0;

    _count += 1; // CDBGPWRUPREQ
    _count += 1; // CDBGRSTREQ
    _count += 1; // CSYSPWRUPREQ
    _count += 1; // DBGSWENABLE
    _count += 1; // JTAGNSW
    _count += 1; // JTAGTOP
    _count += 1; // PSLVERRSYS
    _count += 1; // PSLVERRDBG
    return _count;
}

int cap_apb_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_apb_csr_sta_dap_t::s_get_width(); // sta_dap
    _count += cap_apb_csr_cfg_dap_t::s_get_width(); // cfg_dap
    _count += cap_apb_csr_dhs_psys_t::s_get_width(); // dhs_psys
    _count += cap_apb_csr_dhs_bypass_t::s_get_width(); // dhs_bypass
    return _count;
}

void cap_apb_csr_dhs_bypass_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_apb_csr_dhs_bypass_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_apb_csr_dhs_psys_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_apb_csr_dhs_psys_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_apb_csr_cfg_dap_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__PCLKENSYS = _val.convert_to< PCLKENSYS_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__PRESETSYSn = _val.convert_to< PRESETSYSn_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__PRESETDBGn = _val.convert_to< PRESETDBGn_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CDBGPWRUPACK = _val.convert_to< CDBGPWRUPACK_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CSYSPWRUPACK = _val.convert_to< CSYSPWRUPACK_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CDBGRSTACK = _val.convert_to< CDBGRSTACK_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__nCDBGPWRDN = _val.convert_to< nCDBGPWRDN_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__nCSOCPWRDN = _val.convert_to< nCSOCPWRDN_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__DEVICEEN = _val.convert_to< DEVICEEN_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__psys_paddr_indir = _val.convert_to< psys_paddr_indir_cpp_int_t >()  ;
    _val = _val >> 14;
    
    int_var__PCLKENDBG = _val.convert_to< PCLKENDBG_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__nPOTRST = _val.convert_to< nPOTRST_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__PADDRDBG_mask = _val.convert_to< PADDRDBG_mask_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__PADDRDBG_or = _val.convert_to< PADDRDBG_or_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__bypass_debug = _val.convert_to< bypass_debug_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_apb_csr_sta_dap_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__CDBGPWRUPREQ = _val.convert_to< CDBGPWRUPREQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CDBGRSTREQ = _val.convert_to< CDBGRSTREQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CSYSPWRUPREQ = _val.convert_to< CSYSPWRUPREQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__DBGSWENABLE = _val.convert_to< DBGSWENABLE_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__JTAGNSW = _val.convert_to< JTAGNSW_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__JTAGTOP = _val.convert_to< JTAGTOP_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__PSLVERRSYS = _val.convert_to< PSLVERRSYS_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__PSLVERRDBG = _val.convert_to< PSLVERRDBG_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_apb_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    sta_dap.all( _val);
    _val = _val >> sta_dap.get_width(); 
    cfg_dap.all( _val);
    _val = _val >> cfg_dap.get_width(); 
    dhs_psys.all( _val);
    _val = _val >> dhs_psys.get_width(); 
    dhs_bypass.all( _val);
    _val = _val >> dhs_bypass.get_width(); 
}

cpp_int cap_apb_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << dhs_bypass.get_width(); ret_val = ret_val  | dhs_bypass.all(); 
    ret_val = ret_val << dhs_psys.get_width(); ret_val = ret_val  | dhs_psys.all(); 
    ret_val = ret_val << cfg_dap.get_width(); ret_val = ret_val  | cfg_dap.all(); 
    ret_val = ret_val << sta_dap.get_width(); ret_val = ret_val  | sta_dap.all(); 
    return ret_val;
}

cpp_int cap_apb_csr_sta_dap_t::all() const {
    cpp_int ret_val;

    // PSLVERRDBG
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PSLVERRDBG; 
    
    // PSLVERRSYS
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PSLVERRSYS; 
    
    // JTAGTOP
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__JTAGTOP; 
    
    // JTAGNSW
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__JTAGNSW; 
    
    // DBGSWENABLE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__DBGSWENABLE; 
    
    // CSYSPWRUPREQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CSYSPWRUPREQ; 
    
    // CDBGRSTREQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CDBGRSTREQ; 
    
    // CDBGPWRUPREQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CDBGPWRUPREQ; 
    
    return ret_val;
}

cpp_int cap_apb_csr_cfg_dap_t::all() const {
    cpp_int ret_val;

    // bypass_debug
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bypass_debug; 
    
    // PADDRDBG_or
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__PADDRDBG_or; 
    
    // PADDRDBG_mask
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__PADDRDBG_mask; 
    
    // nPOTRST
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__nPOTRST; 
    
    // PCLKENDBG
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PCLKENDBG; 
    
    // psys_paddr_indir
    ret_val = ret_val << 14; ret_val = ret_val  | int_var__psys_paddr_indir; 
    
    // DEVICEEN
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__DEVICEEN; 
    
    // nCSOCPWRDN
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__nCSOCPWRDN; 
    
    // nCDBGPWRDN
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__nCDBGPWRDN; 
    
    // CDBGRSTACK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CDBGRSTACK; 
    
    // CSYSPWRUPACK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CSYSPWRUPACK; 
    
    // CDBGPWRUPACK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CDBGPWRUPACK; 
    
    // PRESETDBGn
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PRESETDBGn; 
    
    // PRESETSYSn
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PRESETSYSn; 
    
    // PCLKENSYS
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PCLKENSYS; 
    
    return ret_val;
}

cpp_int cap_apb_csr_dhs_psys_t::all() const {
    cpp_int ret_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 65536-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_apb_csr_dhs_psys_entry_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_apb_csr_dhs_bypass_t::all() const {
    cpp_int ret_val;

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 65536-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_apb_csr_dhs_bypass_entry_t::all() const {
    cpp_int ret_val;

    // data
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

void cap_apb_csr_dhs_bypass_entry_t::clear() {

    int_var__data = 0; 
    
}

void cap_apb_csr_dhs_bypass_t::clear() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_apb_csr_dhs_psys_entry_t::clear() {

    int_var__data = 0; 
    
}

void cap_apb_csr_dhs_psys_t::clear() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 65536; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_apb_csr_cfg_dap_t::clear() {

    int_var__PCLKENSYS = 0; 
    
    int_var__PRESETSYSn = 0; 
    
    int_var__PRESETDBGn = 0; 
    
    int_var__CDBGPWRUPACK = 0; 
    
    int_var__CSYSPWRUPACK = 0; 
    
    int_var__CDBGRSTACK = 0; 
    
    int_var__nCDBGPWRDN = 0; 
    
    int_var__nCSOCPWRDN = 0; 
    
    int_var__DEVICEEN = 0; 
    
    int_var__psys_paddr_indir = 0; 
    
    int_var__PCLKENDBG = 0; 
    
    int_var__nPOTRST = 0; 
    
    int_var__PADDRDBG_mask = 0; 
    
    int_var__PADDRDBG_or = 0; 
    
    int_var__bypass_debug = 0; 
    
}

void cap_apb_csr_sta_dap_t::clear() {

    int_var__CDBGPWRUPREQ = 0; 
    
    int_var__CDBGRSTREQ = 0; 
    
    int_var__CSYSPWRUPREQ = 0; 
    
    int_var__DBGSWENABLE = 0; 
    
    int_var__JTAGNSW = 0; 
    
    int_var__JTAGTOP = 0; 
    
    int_var__PSLVERRSYS = 0; 
    
    int_var__PSLVERRDBG = 0; 
    
}

void cap_apb_csr_t::clear() {

    sta_dap.clear();
    cfg_dap.clear();
    dhs_psys.clear();
    dhs_bypass.clear();
}

void cap_apb_csr_dhs_bypass_entry_t::init() {

}

void cap_apb_csr_dhs_bypass_t::init() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 65536; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_apb_csr_dhs_psys_entry_t::init() {

}

void cap_apb_csr_dhs_psys_t::init() {

    #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 65536; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_apb_csr_cfg_dap_t::init() {

    set_reset_val(cpp_int("0x1fffffffe8001f9"));
    all(get_reset_val());
}

void cap_apb_csr_sta_dap_t::init() {

}

void cap_apb_csr_t::init() {

    sta_dap.set_attributes(this,"sta_dap", 0x0 );
    cfg_dap.set_attributes(this,"cfg_dap", 0x10 );
    dhs_psys.set_attributes(this,"dhs_psys", 0x40000 );
    dhs_bypass.set_attributes(this,"dhs_bypass", 0x80000 );
}

void cap_apb_csr_dhs_bypass_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_apb_csr_dhs_bypass_entry_t::data() const {
    return int_var__data;
}
    
void cap_apb_csr_dhs_psys_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_apb_csr_dhs_psys_entry_t::data() const {
    return int_var__data;
}
    
void cap_apb_csr_cfg_dap_t::PCLKENSYS(const cpp_int & _val) { 
    // PCLKENSYS
    int_var__PCLKENSYS = _val.convert_to< PCLKENSYS_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::PCLKENSYS() const {
    return int_var__PCLKENSYS;
}
    
void cap_apb_csr_cfg_dap_t::PRESETSYSn(const cpp_int & _val) { 
    // PRESETSYSn
    int_var__PRESETSYSn = _val.convert_to< PRESETSYSn_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::PRESETSYSn() const {
    return int_var__PRESETSYSn;
}
    
void cap_apb_csr_cfg_dap_t::PRESETDBGn(const cpp_int & _val) { 
    // PRESETDBGn
    int_var__PRESETDBGn = _val.convert_to< PRESETDBGn_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::PRESETDBGn() const {
    return int_var__PRESETDBGn;
}
    
void cap_apb_csr_cfg_dap_t::CDBGPWRUPACK(const cpp_int & _val) { 
    // CDBGPWRUPACK
    int_var__CDBGPWRUPACK = _val.convert_to< CDBGPWRUPACK_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::CDBGPWRUPACK() const {
    return int_var__CDBGPWRUPACK;
}
    
void cap_apb_csr_cfg_dap_t::CSYSPWRUPACK(const cpp_int & _val) { 
    // CSYSPWRUPACK
    int_var__CSYSPWRUPACK = _val.convert_to< CSYSPWRUPACK_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::CSYSPWRUPACK() const {
    return int_var__CSYSPWRUPACK;
}
    
void cap_apb_csr_cfg_dap_t::CDBGRSTACK(const cpp_int & _val) { 
    // CDBGRSTACK
    int_var__CDBGRSTACK = _val.convert_to< CDBGRSTACK_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::CDBGRSTACK() const {
    return int_var__CDBGRSTACK;
}
    
void cap_apb_csr_cfg_dap_t::nCDBGPWRDN(const cpp_int & _val) { 
    // nCDBGPWRDN
    int_var__nCDBGPWRDN = _val.convert_to< nCDBGPWRDN_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::nCDBGPWRDN() const {
    return int_var__nCDBGPWRDN;
}
    
void cap_apb_csr_cfg_dap_t::nCSOCPWRDN(const cpp_int & _val) { 
    // nCSOCPWRDN
    int_var__nCSOCPWRDN = _val.convert_to< nCSOCPWRDN_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::nCSOCPWRDN() const {
    return int_var__nCSOCPWRDN;
}
    
void cap_apb_csr_cfg_dap_t::DEVICEEN(const cpp_int & _val) { 
    // DEVICEEN
    int_var__DEVICEEN = _val.convert_to< DEVICEEN_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::DEVICEEN() const {
    return int_var__DEVICEEN;
}
    
void cap_apb_csr_cfg_dap_t::psys_paddr_indir(const cpp_int & _val) { 
    // psys_paddr_indir
    int_var__psys_paddr_indir = _val.convert_to< psys_paddr_indir_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::psys_paddr_indir() const {
    return int_var__psys_paddr_indir;
}
    
void cap_apb_csr_cfg_dap_t::PCLKENDBG(const cpp_int & _val) { 
    // PCLKENDBG
    int_var__PCLKENDBG = _val.convert_to< PCLKENDBG_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::PCLKENDBG() const {
    return int_var__PCLKENDBG;
}
    
void cap_apb_csr_cfg_dap_t::nPOTRST(const cpp_int & _val) { 
    // nPOTRST
    int_var__nPOTRST = _val.convert_to< nPOTRST_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::nPOTRST() const {
    return int_var__nPOTRST;
}
    
void cap_apb_csr_cfg_dap_t::PADDRDBG_mask(const cpp_int & _val) { 
    // PADDRDBG_mask
    int_var__PADDRDBG_mask = _val.convert_to< PADDRDBG_mask_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::PADDRDBG_mask() const {
    return int_var__PADDRDBG_mask;
}
    
void cap_apb_csr_cfg_dap_t::PADDRDBG_or(const cpp_int & _val) { 
    // PADDRDBG_or
    int_var__PADDRDBG_or = _val.convert_to< PADDRDBG_or_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::PADDRDBG_or() const {
    return int_var__PADDRDBG_or;
}
    
void cap_apb_csr_cfg_dap_t::bypass_debug(const cpp_int & _val) { 
    // bypass_debug
    int_var__bypass_debug = _val.convert_to< bypass_debug_cpp_int_t >();
}

cpp_int cap_apb_csr_cfg_dap_t::bypass_debug() const {
    return int_var__bypass_debug;
}
    
void cap_apb_csr_sta_dap_t::CDBGPWRUPREQ(const cpp_int & _val) { 
    // CDBGPWRUPREQ
    int_var__CDBGPWRUPREQ = _val.convert_to< CDBGPWRUPREQ_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::CDBGPWRUPREQ() const {
    return int_var__CDBGPWRUPREQ;
}
    
void cap_apb_csr_sta_dap_t::CDBGRSTREQ(const cpp_int & _val) { 
    // CDBGRSTREQ
    int_var__CDBGRSTREQ = _val.convert_to< CDBGRSTREQ_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::CDBGRSTREQ() const {
    return int_var__CDBGRSTREQ;
}
    
void cap_apb_csr_sta_dap_t::CSYSPWRUPREQ(const cpp_int & _val) { 
    // CSYSPWRUPREQ
    int_var__CSYSPWRUPREQ = _val.convert_to< CSYSPWRUPREQ_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::CSYSPWRUPREQ() const {
    return int_var__CSYSPWRUPREQ;
}
    
void cap_apb_csr_sta_dap_t::DBGSWENABLE(const cpp_int & _val) { 
    // DBGSWENABLE
    int_var__DBGSWENABLE = _val.convert_to< DBGSWENABLE_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::DBGSWENABLE() const {
    return int_var__DBGSWENABLE;
}
    
void cap_apb_csr_sta_dap_t::JTAGNSW(const cpp_int & _val) { 
    // JTAGNSW
    int_var__JTAGNSW = _val.convert_to< JTAGNSW_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::JTAGNSW() const {
    return int_var__JTAGNSW;
}
    
void cap_apb_csr_sta_dap_t::JTAGTOP(const cpp_int & _val) { 
    // JTAGTOP
    int_var__JTAGTOP = _val.convert_to< JTAGTOP_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::JTAGTOP() const {
    return int_var__JTAGTOP;
}
    
void cap_apb_csr_sta_dap_t::PSLVERRSYS(const cpp_int & _val) { 
    // PSLVERRSYS
    int_var__PSLVERRSYS = _val.convert_to< PSLVERRSYS_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::PSLVERRSYS() const {
    return int_var__PSLVERRSYS;
}
    
void cap_apb_csr_sta_dap_t::PSLVERRDBG(const cpp_int & _val) { 
    // PSLVERRDBG
    int_var__PSLVERRDBG = _val.convert_to< PSLVERRDBG_cpp_int_t >();
}

cpp_int cap_apb_csr_sta_dap_t::PSLVERRDBG() const {
    return int_var__PSLVERRDBG;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_bypass_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_bypass_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_psys_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_psys_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_cfg_dap_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PCLKENSYS")) { field_val = PCLKENSYS(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PRESETSYSn")) { field_val = PRESETSYSn(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PRESETDBGn")) { field_val = PRESETDBGn(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CDBGPWRUPACK")) { field_val = CDBGPWRUPACK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CSYSPWRUPACK")) { field_val = CSYSPWRUPACK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CDBGRSTACK")) { field_val = CDBGRSTACK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCDBGPWRDN")) { field_val = nCDBGPWRDN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCSOCPWRDN")) { field_val = nCSOCPWRDN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "DEVICEEN")) { field_val = DEVICEEN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "psys_paddr_indir")) { field_val = psys_paddr_indir(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PCLKENDBG")) { field_val = PCLKENDBG(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nPOTRST")) { field_val = nPOTRST(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PADDRDBG_mask")) { field_val = PADDRDBG_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PADDRDBG_or")) { field_val = PADDRDBG_or(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bypass_debug")) { field_val = bypass_debug(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_sta_dap_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CDBGPWRUPREQ")) { field_val = CDBGPWRUPREQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CDBGRSTREQ")) { field_val = CDBGRSTREQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CSYSPWRUPREQ")) { field_val = CSYSPWRUPREQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "DBGSWENABLE")) { field_val = DBGSWENABLE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "JTAGNSW")) { field_val = JTAGNSW(); field_found=1; }
    if(!field_found && !strcmp(field_name, "JTAGTOP")) { field_val = JTAGTOP(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PSLVERRSYS")) { field_val = PSLVERRSYS(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PSLVERRDBG")) { field_val = PSLVERRDBG(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = sta_dap.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dap.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_psys.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_bypass.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_bypass_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_bypass_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_psys_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_dhs_psys_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_cfg_dap_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PCLKENSYS")) { PCLKENSYS(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PRESETSYSn")) { PRESETSYSn(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PRESETDBGn")) { PRESETDBGn(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CDBGPWRUPACK")) { CDBGPWRUPACK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CSYSPWRUPACK")) { CSYSPWRUPACK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CDBGRSTACK")) { CDBGRSTACK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCDBGPWRDN")) { nCDBGPWRDN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCSOCPWRDN")) { nCSOCPWRDN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "DEVICEEN")) { DEVICEEN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "psys_paddr_indir")) { psys_paddr_indir(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PCLKENDBG")) { PCLKENDBG(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nPOTRST")) { nPOTRST(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PADDRDBG_mask")) { PADDRDBG_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PADDRDBG_or")) { PADDRDBG_or(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bypass_debug")) { bypass_debug(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_sta_dap_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CDBGPWRUPREQ")) { CDBGPWRUPREQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CDBGRSTREQ")) { CDBGRSTREQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CSYSPWRUPREQ")) { CSYSPWRUPREQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "DBGSWENABLE")) { DBGSWENABLE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "JTAGNSW")) { JTAGNSW(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "JTAGTOP")) { JTAGTOP(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PSLVERRSYS")) { PSLVERRSYS(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PSLVERRDBG")) { PSLVERRDBG(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_apb_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = sta_dap.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dap.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_psys.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_bypass.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_dhs_bypass_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_dhs_bypass_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_dhs_psys_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_dhs_psys_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_cfg_dap_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("PCLKENSYS");
    ret_vec.push_back("PRESETSYSn");
    ret_vec.push_back("PRESETDBGn");
    ret_vec.push_back("CDBGPWRUPACK");
    ret_vec.push_back("CSYSPWRUPACK");
    ret_vec.push_back("CDBGRSTACK");
    ret_vec.push_back("nCDBGPWRDN");
    ret_vec.push_back("nCSOCPWRDN");
    ret_vec.push_back("DEVICEEN");
    ret_vec.push_back("psys_paddr_indir");
    ret_vec.push_back("PCLKENDBG");
    ret_vec.push_back("nPOTRST");
    ret_vec.push_back("PADDRDBG_mask");
    ret_vec.push_back("PADDRDBG_or");
    ret_vec.push_back("bypass_debug");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_sta_dap_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("CDBGPWRUPREQ");
    ret_vec.push_back("CDBGRSTREQ");
    ret_vec.push_back("CSYSPWRUPREQ");
    ret_vec.push_back("DBGSWENABLE");
    ret_vec.push_back("JTAGNSW");
    ret_vec.push_back("JTAGTOP");
    ret_vec.push_back("PSLVERRSYS");
    ret_vec.push_back("PSLVERRDBG");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_apb_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : sta_dap.get_fields(level-1)) {
            ret_vec.push_back("sta_dap." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dap.get_fields(level-1)) {
            ret_vec.push_back("cfg_dap." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_psys.get_fields(level-1)) {
            ret_vec.push_back("dhs_psys." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_bypass.get_fields(level-1)) {
            ret_vec.push_back("dhs_bypass." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
