
#include "cap_dprmem_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::~cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t() { }

cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::~cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t() { }

cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::~cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t() { }

cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::~cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t() { }

cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::~cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t() { }

cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::~cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t() { }

cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::~cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t() { }

cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::~cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t() { }

cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::~cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t() { }

cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::~cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t() { }

cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::~cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t() { }

cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t(string name, cap_csr_base * parent): 
    cap_memory_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::~cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t() { }

cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::~cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t() { }

cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::~cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t() { }

cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::~cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t() { }

cap_dprmem_csr_cfg_dpr_pktout_fifo_t::cap_dprmem_csr_cfg_dpr_pktout_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfg_dpr_pktout_fifo_t::~cap_dprmem_csr_cfg_dpr_pktout_fifo_t() { }

cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::~cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t() { }

cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::~cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t() { }

cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::~cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t() { }

cap_dprmem_csr_cfg_dpr_ptr_fifo_t::cap_dprmem_csr_cfg_dpr_ptr_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfg_dpr_ptr_fifo_t::~cap_dprmem_csr_cfg_dpr_ptr_fifo_t() { }

cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::~cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t() { }

cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::~cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t() { }

cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::~cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t() { }

cap_dprmem_csr_cfg_dpr_ohi_fifo_t::cap_dprmem_csr_cfg_dpr_ohi_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfg_dpr_ohi_fifo_t::~cap_dprmem_csr_cfg_dpr_ohi_fifo_t() { }

cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::~cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t() { }

cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::~cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t() { }

cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::~cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t() { }

cap_dprmem_csr_cfg_dpr_phv_fifo_t::cap_dprmem_csr_cfg_dpr_phv_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfg_dpr_phv_fifo_t::~cap_dprmem_csr_cfg_dpr_phv_fifo_t() { }

cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::~cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t() { }

cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::~cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t() { }

cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::~cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t() { }

cap_dprmem_csr_cfg_dpr_csum_fifo_t::cap_dprmem_csr_cfg_dpr_csum_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfg_dpr_csum_fifo_t::~cap_dprmem_csr_cfg_dpr_csum_fifo_t() { }

cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::~cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t() { }

cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::~cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t() { }

cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::~cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t() { }

cap_dprmem_csr_cfg_dpr_pktin_fifo_t::cap_dprmem_csr_cfg_dpr_pktin_fifo_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_dprmem_csr_cfg_dpr_pktin_fifo_t::~cap_dprmem_csr_cfg_dpr_pktin_fifo_t() { }

cap_dprmem_csr_t::cap_dprmem_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(524288);
        set_attributes(0,get_name(), 0);
        }
cap_dprmem_csr_t::~cap_dprmem_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 16; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".data: 0x" << int_var__data << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc: 0x" << int_var__ecc << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    #if 832 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.show(); // large_array
    #else
    for(int ii = 0; ii < 832; ii++) {
        entry[ii].show();
    }
    #endif
    
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_written: 0x" << int_var__fifo_ptr_written << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_wptr: 0x" << int_var__fifo_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_rptr: 0x" << int_var__fifo_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_offline: 0x" << int_var__fifo_offline << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_flush: 0x" << int_var__fifo_flush << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_wptr: 0x" << int_var__sta_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_rptr: 0x" << int_var__sta_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_empty: 0x" << int_var__sta_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_full: 0x" << int_var__sta_full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_detect: 0x" << int_var__ecc_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_correct: 0x" << int_var__ecc_correct << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_bypass: 0x" << int_var__ecc_bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_written: 0x" << int_var__fifo_ptr_written << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_wptr: 0x" << int_var__fifo_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_rptr: 0x" << int_var__fifo_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_offline: 0x" << int_var__fifo_offline << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_flush: 0x" << int_var__fifo_flush << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_wptr: 0x" << int_var__sta_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_rptr: 0x" << int_var__sta_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_empty: 0x" << int_var__sta_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_full: 0x" << int_var__sta_full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_detect: 0x" << int_var__ecc_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_correct: 0x" << int_var__ecc_correct << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_bypass: 0x" << int_var__ecc_bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_written: 0x" << int_var__fifo_ptr_written << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_wptr: 0x" << int_var__fifo_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_rptr: 0x" << int_var__fifo_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_offline: 0x" << int_var__fifo_offline << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_flush: 0x" << int_var__fifo_flush << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_wptr: 0x" << int_var__sta_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_rptr: 0x" << int_var__sta_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_empty: 0x" << int_var__sta_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_full: 0x" << int_var__sta_full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_detect: 0x" << int_var__ecc_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_correct: 0x" << int_var__ecc_correct << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_bypass: 0x" << int_var__ecc_bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_written: 0x" << int_var__fifo_ptr_written << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_wptr: 0x" << int_var__fifo_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_rptr: 0x" << int_var__fifo_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_offline: 0x" << int_var__fifo_offline << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_flush: 0x" << int_var__fifo_flush << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_wptr: 0x" << int_var__sta_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_rptr: 0x" << int_var__sta_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_empty: 0x" << int_var__sta_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_full: 0x" << int_var__sta_full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfg_dpr_phv_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_detect: 0x" << int_var__ecc_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_correct: 0x" << int_var__ecc_correct << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_bypass: 0x" << int_var__ecc_bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_written: 0x" << int_var__fifo_ptr_written << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_wptr: 0x" << int_var__fifo_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_rptr: 0x" << int_var__fifo_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_offline: 0x" << int_var__fifo_offline << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_flush: 0x" << int_var__fifo_flush << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_wptr: 0x" << int_var__sta_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_rptr: 0x" << int_var__sta_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_empty: 0x" << int_var__sta_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_full: 0x" << int_var__sta_full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfg_dpr_csum_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_detect: 0x" << int_var__ecc_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_correct: 0x" << int_var__ecc_correct << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_bypass: 0x" << int_var__ecc_bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_written: 0x" << int_var__fifo_ptr_written << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_wptr: 0x" << int_var__fifo_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_ptr_rptr: 0x" << int_var__fifo_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_offline: 0x" << int_var__fifo_offline << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".fifo_flush: 0x" << int_var__fifo_flush << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_wptr: 0x" << int_var__sta_ptr_wptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_ptr_rptr: 0x" << int_var__sta_ptr_rptr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_empty: 0x" << int_var__sta_empty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_full: 0x" << int_var__sta_full << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable: 0x" << int_var__uncorrectable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable: 0x" << int_var__correctable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome: 0x" << int_var__syndrome << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr: 0x" << int_var__addr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_fail: 0x" << int_var__bist_done_fail << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_done_pass: 0x" << int_var__bist_done_pass << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_detect: 0x" << int_var__ecc_detect << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_correct: 0x" << int_var__ecc_correct << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ecc_bypass: 0x" << int_var__ecc_bypass << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".bist_run: 0x" << int_var__bist_run << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_dprmem_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    cfg_dpr_pktin_fifo.show();
    sta_srams_ecc_dpr_pktin_fifo.show();
    sta_ff_ptr_dpr_pktin_fifo.show();
    cfw_ff_dpr_pktin_fifo.show();
    cfg_dpr_csum_fifo.show();
    sta_srams_ecc_dpr_csum_fifo.show();
    sta_ff_ptr_dpr_csum_fifo.show();
    cfw_ff_dpr_csum_fifo.show();
    cfg_dpr_phv_fifo.show();
    sta_srams_ecc_dpr_phv_fifo.show();
    sta_ff_ptr_dpr_phv_fifo.show();
    cfw_ff_dpr_phv_fifo.show();
    cfg_dpr_ohi_fifo.show();
    sta_srams_ecc_dpr_ohi_fifo.show();
    sta_ff_ptr_dpr_ohi_fifo.show();
    cfw_ff_dpr_ohi_fifo.show();
    cfg_dpr_ptr_fifo.show();
    sta_srams_ecc_dpr_ptr_fifo.show();
    sta_ff_ptr_dpr_ptr_fifo.show();
    cfw_ff_dpr_ptr_fifo.show();
    cfg_dpr_pktout_fifo.show();
    sta_srams_ecc_dpr_pktout_fifo.show();
    sta_ff_ptr_dpr_pktout_fifo.show();
    cfw_ff_dpr_pktout_fifo.show();
    dhs_dpr_pktin_fifo_sram.show();
    dhs_dpr_csum_fifo_sram.show();
    dhs_dpr_phv_fifo_sram.show();
    dhs_dpr_ohi_fifo_sram.show();
    dhs_dpr_ptr_fifo_sram.show();
    dhs_dpr_pktout_fifo_sram.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::get_width() const {
    return cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::s_get_width();

}

int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::get_width() const {
    return cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::get_width() const {
    return cap_dprmem_csr_cfg_dpr_pktout_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::get_width() const {
    return cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::get_width() const {
    return cap_dprmem_csr_cfg_dpr_ptr_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::get_width() const {
    return cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::get_width() const {
    return cap_dprmem_csr_cfg_dpr_ohi_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::get_width() const {
    return cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfg_dpr_phv_fifo_t::get_width() const {
    return cap_dprmem_csr_cfg_dpr_phv_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::get_width() const {
    return cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfg_dpr_csum_fifo_t::get_width() const {
    return cap_dprmem_csr_cfg_dpr_csum_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::get_width() const {
    return cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::s_get_width();

}

int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::get_width() const {
    return cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::s_get_width();

}

int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::get_width() const {
    return cap_dprmem_csr_cfg_dpr_pktin_fifo_t::s_get_width();

}

int cap_dprmem_csr_t::get_width() const {
    return cap_dprmem_csr_t::s_get_width();

}

int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 521; // data
    _count += 36; // ecc
    return _count;
}

int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::s_get_width() * 16); // entry
    return _count;
}

int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 1218; // data
    _count += 72; // ecc
    return _count;
}

int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::s_get_width() * 32); // entry
    return _count;
}

int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 515; // data
    _count += 20; // ecc
    return _count;
}

int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::s_get_width() * 16); // entry
    return _count;
}

int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 515; // data
    _count += 36; // ecc
    return _count;
}

int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::s_get_width() * 64); // entry
    return _count;
}

int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 521; // data
    _count += 36; // ecc
    return _count;
}

int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::s_get_width() * 512); // entry
    return _count;
}

int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::s_get_width() {
    int _count = 0;

    _count += 521; // data
    _count += 36; // ecc
    return _count;
}

int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::s_get_width() {
    int _count = 0;

    _count += (cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::s_get_width() * 832); // entry
    return _count;
}

int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // fifo_ptr_written
    _count += 4; // fifo_ptr_wptr
    _count += 4; // fifo_ptr_rptr
    _count += 1; // fifo_offline
    _count += 1; // fifo_flush
    return _count;
}

int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::s_get_width() {
    int _count = 0;

    _count += 4; // sta_ptr_wptr
    _count += 4; // sta_ptr_rptr
    _count += 1; // sta_empty
    _count += 1; // sta_full
    return _count;
}

int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 36; // syndrome
    _count += 4; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_detect
    _count += 1; // ecc_correct
    _count += 1; // ecc_bypass
    _count += 1; // bist_run
    return _count;
}

int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // fifo_ptr_written
    _count += 5; // fifo_ptr_wptr
    _count += 5; // fifo_ptr_rptr
    _count += 1; // fifo_offline
    _count += 1; // fifo_flush
    return _count;
}

int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::s_get_width() {
    int _count = 0;

    _count += 5; // sta_ptr_wptr
    _count += 5; // sta_ptr_rptr
    _count += 1; // sta_empty
    _count += 1; // sta_full
    return _count;
}

int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 72; // syndrome
    _count += 5; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_detect
    _count += 1; // ecc_correct
    _count += 1; // ecc_bypass
    _count += 1; // bist_run
    return _count;
}

int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // fifo_ptr_written
    _count += 4; // fifo_ptr_wptr
    _count += 4; // fifo_ptr_rptr
    _count += 1; // fifo_offline
    _count += 1; // fifo_flush
    return _count;
}

int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::s_get_width() {
    int _count = 0;

    _count += 4; // sta_ptr_wptr
    _count += 4; // sta_ptr_rptr
    _count += 1; // sta_empty
    _count += 1; // sta_full
    return _count;
}

int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 20; // syndrome
    _count += 4; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_detect
    _count += 1; // ecc_correct
    _count += 1; // ecc_bypass
    _count += 1; // bist_run
    return _count;
}

int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // fifo_ptr_written
    _count += 6; // fifo_ptr_wptr
    _count += 6; // fifo_ptr_rptr
    _count += 1; // fifo_offline
    _count += 1; // fifo_flush
    return _count;
}

int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::s_get_width() {
    int _count = 0;

    _count += 6; // sta_ptr_wptr
    _count += 6; // sta_ptr_rptr
    _count += 1; // sta_empty
    _count += 1; // sta_full
    return _count;
}

int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 36; // syndrome
    _count += 6; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_dprmem_csr_cfg_dpr_phv_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_detect
    _count += 1; // ecc_correct
    _count += 1; // ecc_bypass
    _count += 1; // bist_run
    return _count;
}

int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // fifo_ptr_written
    _count += 9; // fifo_ptr_wptr
    _count += 9; // fifo_ptr_rptr
    _count += 1; // fifo_offline
    _count += 1; // fifo_flush
    return _count;
}

int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::s_get_width() {
    int _count = 0;

    _count += 9; // sta_ptr_wptr
    _count += 9; // sta_ptr_rptr
    _count += 1; // sta_empty
    _count += 1; // sta_full
    return _count;
}

int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 36; // syndrome
    _count += 9; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_dprmem_csr_cfg_dpr_csum_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_detect
    _count += 1; // ecc_correct
    _count += 1; // ecc_bypass
    _count += 1; // bist_run
    return _count;
}

int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // fifo_ptr_written
    _count += 10; // fifo_ptr_wptr
    _count += 10; // fifo_ptr_rptr
    _count += 1; // fifo_offline
    _count += 1; // fifo_flush
    return _count;
}

int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::s_get_width() {
    int _count = 0;

    _count += 10; // sta_ptr_wptr
    _count += 10; // sta_ptr_rptr
    _count += 1; // sta_empty
    _count += 1; // sta_full
    return _count;
}

int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // uncorrectable
    _count += 1; // correctable
    _count += 36; // syndrome
    _count += 10; // addr
    _count += 1; // bist_done_fail
    _count += 1; // bist_done_pass
    return _count;
}

int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::s_get_width() {
    int _count = 0;

    _count += 1; // ecc_detect
    _count += 1; // ecc_correct
    _count += 1; // ecc_bypass
    _count += 1; // bist_run
    return _count;
}

int cap_dprmem_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_dprmem_csr_cfg_dpr_pktin_fifo_t::s_get_width(); // cfg_dpr_pktin_fifo
    _count += cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::s_get_width(); // sta_srams_ecc_dpr_pktin_fifo
    _count += cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::s_get_width(); // sta_ff_ptr_dpr_pktin_fifo
    _count += cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::s_get_width(); // cfw_ff_dpr_pktin_fifo
    _count += cap_dprmem_csr_cfg_dpr_csum_fifo_t::s_get_width(); // cfg_dpr_csum_fifo
    _count += cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::s_get_width(); // sta_srams_ecc_dpr_csum_fifo
    _count += cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::s_get_width(); // sta_ff_ptr_dpr_csum_fifo
    _count += cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::s_get_width(); // cfw_ff_dpr_csum_fifo
    _count += cap_dprmem_csr_cfg_dpr_phv_fifo_t::s_get_width(); // cfg_dpr_phv_fifo
    _count += cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::s_get_width(); // sta_srams_ecc_dpr_phv_fifo
    _count += cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::s_get_width(); // sta_ff_ptr_dpr_phv_fifo
    _count += cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::s_get_width(); // cfw_ff_dpr_phv_fifo
    _count += cap_dprmem_csr_cfg_dpr_ohi_fifo_t::s_get_width(); // cfg_dpr_ohi_fifo
    _count += cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::s_get_width(); // sta_srams_ecc_dpr_ohi_fifo
    _count += cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::s_get_width(); // sta_ff_ptr_dpr_ohi_fifo
    _count += cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::s_get_width(); // cfw_ff_dpr_ohi_fifo
    _count += cap_dprmem_csr_cfg_dpr_ptr_fifo_t::s_get_width(); // cfg_dpr_ptr_fifo
    _count += cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::s_get_width(); // sta_srams_ecc_dpr_ptr_fifo
    _count += cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::s_get_width(); // sta_ff_ptr_dpr_ptr_fifo
    _count += cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::s_get_width(); // cfw_ff_dpr_ptr_fifo
    _count += cap_dprmem_csr_cfg_dpr_pktout_fifo_t::s_get_width(); // cfg_dpr_pktout_fifo
    _count += cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::s_get_width(); // sta_srams_ecc_dpr_pktout_fifo
    _count += cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::s_get_width(); // sta_ff_ptr_dpr_pktout_fifo
    _count += cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::s_get_width(); // cfw_ff_dpr_pktout_fifo
    _count += cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::s_get_width(); // dhs_dpr_pktin_fifo_sram
    _count += cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::s_get_width(); // dhs_dpr_csum_fifo_sram
    _count += cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::s_get_width(); // dhs_dpr_phv_fifo_sram
    _count += cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::s_get_width(); // dhs_dpr_ohi_fifo_sram
    _count += cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::s_get_width(); // dhs_dpr_ptr_fifo_sram
    _count += cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::s_get_width(); // dhs_dpr_pktout_fifo_sram
    return _count;
}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 521;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 36;
    
}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 16; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 1218;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 72;
    
}

void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 515;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 20;
    
}

void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 16; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 515;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 36;
    
}

void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 521;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 36;
    
}

void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__data = _val.convert_to< data_cpp_int_t >()  ;
    _val = _val >> 521;
    
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >()  ;
    _val = _val >> 36;
    
}

void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    #if 832 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 832; ii++) {
        entry[ii].all(_val);
        _val = _val >> entry[ii].get_width();
    }
    #endif
    
}

void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 36;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 72;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 20;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 36;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfg_dpr_phv_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 36;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 9;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfg_dpr_csum_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >()  ;
    _val = _val >> 36;
    
    int_var__addr = _val.convert_to< addr_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_dprmem_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    cfg_dpr_pktin_fifo.all( _val);
    _val = _val >> cfg_dpr_pktin_fifo.get_width(); 
    sta_srams_ecc_dpr_pktin_fifo.all( _val);
    _val = _val >> sta_srams_ecc_dpr_pktin_fifo.get_width(); 
    sta_ff_ptr_dpr_pktin_fifo.all( _val);
    _val = _val >> sta_ff_ptr_dpr_pktin_fifo.get_width(); 
    cfw_ff_dpr_pktin_fifo.all( _val);
    _val = _val >> cfw_ff_dpr_pktin_fifo.get_width(); 
    cfg_dpr_csum_fifo.all( _val);
    _val = _val >> cfg_dpr_csum_fifo.get_width(); 
    sta_srams_ecc_dpr_csum_fifo.all( _val);
    _val = _val >> sta_srams_ecc_dpr_csum_fifo.get_width(); 
    sta_ff_ptr_dpr_csum_fifo.all( _val);
    _val = _val >> sta_ff_ptr_dpr_csum_fifo.get_width(); 
    cfw_ff_dpr_csum_fifo.all( _val);
    _val = _val >> cfw_ff_dpr_csum_fifo.get_width(); 
    cfg_dpr_phv_fifo.all( _val);
    _val = _val >> cfg_dpr_phv_fifo.get_width(); 
    sta_srams_ecc_dpr_phv_fifo.all( _val);
    _val = _val >> sta_srams_ecc_dpr_phv_fifo.get_width(); 
    sta_ff_ptr_dpr_phv_fifo.all( _val);
    _val = _val >> sta_ff_ptr_dpr_phv_fifo.get_width(); 
    cfw_ff_dpr_phv_fifo.all( _val);
    _val = _val >> cfw_ff_dpr_phv_fifo.get_width(); 
    cfg_dpr_ohi_fifo.all( _val);
    _val = _val >> cfg_dpr_ohi_fifo.get_width(); 
    sta_srams_ecc_dpr_ohi_fifo.all( _val);
    _val = _val >> sta_srams_ecc_dpr_ohi_fifo.get_width(); 
    sta_ff_ptr_dpr_ohi_fifo.all( _val);
    _val = _val >> sta_ff_ptr_dpr_ohi_fifo.get_width(); 
    cfw_ff_dpr_ohi_fifo.all( _val);
    _val = _val >> cfw_ff_dpr_ohi_fifo.get_width(); 
    cfg_dpr_ptr_fifo.all( _val);
    _val = _val >> cfg_dpr_ptr_fifo.get_width(); 
    sta_srams_ecc_dpr_ptr_fifo.all( _val);
    _val = _val >> sta_srams_ecc_dpr_ptr_fifo.get_width(); 
    sta_ff_ptr_dpr_ptr_fifo.all( _val);
    _val = _val >> sta_ff_ptr_dpr_ptr_fifo.get_width(); 
    cfw_ff_dpr_ptr_fifo.all( _val);
    _val = _val >> cfw_ff_dpr_ptr_fifo.get_width(); 
    cfg_dpr_pktout_fifo.all( _val);
    _val = _val >> cfg_dpr_pktout_fifo.get_width(); 
    sta_srams_ecc_dpr_pktout_fifo.all( _val);
    _val = _val >> sta_srams_ecc_dpr_pktout_fifo.get_width(); 
    sta_ff_ptr_dpr_pktout_fifo.all( _val);
    _val = _val >> sta_ff_ptr_dpr_pktout_fifo.get_width(); 
    cfw_ff_dpr_pktout_fifo.all( _val);
    _val = _val >> cfw_ff_dpr_pktout_fifo.get_width(); 
    dhs_dpr_pktin_fifo_sram.all( _val);
    _val = _val >> dhs_dpr_pktin_fifo_sram.get_width(); 
    dhs_dpr_csum_fifo_sram.all( _val);
    _val = _val >> dhs_dpr_csum_fifo_sram.get_width(); 
    dhs_dpr_phv_fifo_sram.all( _val);
    _val = _val >> dhs_dpr_phv_fifo_sram.get_width(); 
    dhs_dpr_ohi_fifo_sram.all( _val);
    _val = _val >> dhs_dpr_ohi_fifo_sram.get_width(); 
    dhs_dpr_ptr_fifo_sram.all( _val);
    _val = _val >> dhs_dpr_ptr_fifo_sram.get_width(); 
    dhs_dpr_pktout_fifo_sram.all( _val);
    _val = _val >> dhs_dpr_pktout_fifo_sram.get_width(); 
}

cpp_int cap_dprmem_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << dhs_dpr_pktout_fifo_sram.get_width(); ret_val = ret_val  | dhs_dpr_pktout_fifo_sram.all(); 
    ret_val = ret_val << dhs_dpr_ptr_fifo_sram.get_width(); ret_val = ret_val  | dhs_dpr_ptr_fifo_sram.all(); 
    ret_val = ret_val << dhs_dpr_ohi_fifo_sram.get_width(); ret_val = ret_val  | dhs_dpr_ohi_fifo_sram.all(); 
    ret_val = ret_val << dhs_dpr_phv_fifo_sram.get_width(); ret_val = ret_val  | dhs_dpr_phv_fifo_sram.all(); 
    ret_val = ret_val << dhs_dpr_csum_fifo_sram.get_width(); ret_val = ret_val  | dhs_dpr_csum_fifo_sram.all(); 
    ret_val = ret_val << dhs_dpr_pktin_fifo_sram.get_width(); ret_val = ret_val  | dhs_dpr_pktin_fifo_sram.all(); 
    ret_val = ret_val << cfw_ff_dpr_pktout_fifo.get_width(); ret_val = ret_val  | cfw_ff_dpr_pktout_fifo.all(); 
    ret_val = ret_val << sta_ff_ptr_dpr_pktout_fifo.get_width(); ret_val = ret_val  | sta_ff_ptr_dpr_pktout_fifo.all(); 
    ret_val = ret_val << sta_srams_ecc_dpr_pktout_fifo.get_width(); ret_val = ret_val  | sta_srams_ecc_dpr_pktout_fifo.all(); 
    ret_val = ret_val << cfg_dpr_pktout_fifo.get_width(); ret_val = ret_val  | cfg_dpr_pktout_fifo.all(); 
    ret_val = ret_val << cfw_ff_dpr_ptr_fifo.get_width(); ret_val = ret_val  | cfw_ff_dpr_ptr_fifo.all(); 
    ret_val = ret_val << sta_ff_ptr_dpr_ptr_fifo.get_width(); ret_val = ret_val  | sta_ff_ptr_dpr_ptr_fifo.all(); 
    ret_val = ret_val << sta_srams_ecc_dpr_ptr_fifo.get_width(); ret_val = ret_val  | sta_srams_ecc_dpr_ptr_fifo.all(); 
    ret_val = ret_val << cfg_dpr_ptr_fifo.get_width(); ret_val = ret_val  | cfg_dpr_ptr_fifo.all(); 
    ret_val = ret_val << cfw_ff_dpr_ohi_fifo.get_width(); ret_val = ret_val  | cfw_ff_dpr_ohi_fifo.all(); 
    ret_val = ret_val << sta_ff_ptr_dpr_ohi_fifo.get_width(); ret_val = ret_val  | sta_ff_ptr_dpr_ohi_fifo.all(); 
    ret_val = ret_val << sta_srams_ecc_dpr_ohi_fifo.get_width(); ret_val = ret_val  | sta_srams_ecc_dpr_ohi_fifo.all(); 
    ret_val = ret_val << cfg_dpr_ohi_fifo.get_width(); ret_val = ret_val  | cfg_dpr_ohi_fifo.all(); 
    ret_val = ret_val << cfw_ff_dpr_phv_fifo.get_width(); ret_val = ret_val  | cfw_ff_dpr_phv_fifo.all(); 
    ret_val = ret_val << sta_ff_ptr_dpr_phv_fifo.get_width(); ret_val = ret_val  | sta_ff_ptr_dpr_phv_fifo.all(); 
    ret_val = ret_val << sta_srams_ecc_dpr_phv_fifo.get_width(); ret_val = ret_val  | sta_srams_ecc_dpr_phv_fifo.all(); 
    ret_val = ret_val << cfg_dpr_phv_fifo.get_width(); ret_val = ret_val  | cfg_dpr_phv_fifo.all(); 
    ret_val = ret_val << cfw_ff_dpr_csum_fifo.get_width(); ret_val = ret_val  | cfw_ff_dpr_csum_fifo.all(); 
    ret_val = ret_val << sta_ff_ptr_dpr_csum_fifo.get_width(); ret_val = ret_val  | sta_ff_ptr_dpr_csum_fifo.all(); 
    ret_val = ret_val << sta_srams_ecc_dpr_csum_fifo.get_width(); ret_val = ret_val  | sta_srams_ecc_dpr_csum_fifo.all(); 
    ret_val = ret_val << cfg_dpr_csum_fifo.get_width(); ret_val = ret_val  | cfg_dpr_csum_fifo.all(); 
    ret_val = ret_val << cfw_ff_dpr_pktin_fifo.get_width(); ret_val = ret_val  | cfw_ff_dpr_pktin_fifo.all(); 
    ret_val = ret_val << sta_ff_ptr_dpr_pktin_fifo.get_width(); ret_val = ret_val  | sta_ff_ptr_dpr_pktin_fifo.all(); 
    ret_val = ret_val << sta_srams_ecc_dpr_pktin_fifo.get_width(); ret_val = ret_val  | sta_srams_ecc_dpr_pktin_fifo.all(); 
    ret_val = ret_val << cfg_dpr_pktin_fifo.get_width(); ret_val = ret_val  | cfg_dpr_pktin_fifo.all(); 
    return ret_val;
}

cpp_int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_bypass; 
    
    // ecc_correct
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_correct; 
    
    // ecc_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_detect; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::all() const {
    cpp_int ret_val;

    // sta_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_full; 
    
    // sta_empty
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_empty; 
    
    // sta_ptr_rptr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__sta_ptr_rptr; 
    
    // sta_ptr_wptr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__sta_ptr_wptr; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::all() const {
    cpp_int ret_val;

    // fifo_flush
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_flush; 
    
    // fifo_offline
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_offline; 
    
    // fifo_ptr_rptr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__fifo_ptr_rptr; 
    
    // fifo_ptr_wptr
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__fifo_ptr_wptr; 
    
    // fifo_ptr_written
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_ptr_written; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfg_dpr_csum_fifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_bypass; 
    
    // ecc_correct
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_correct; 
    
    // ecc_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_detect; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::all() const {
    cpp_int ret_val;

    // sta_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_full; 
    
    // sta_empty
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_empty; 
    
    // sta_ptr_rptr
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sta_ptr_rptr; 
    
    // sta_ptr_wptr
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__sta_ptr_wptr; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::all() const {
    cpp_int ret_val;

    // fifo_flush
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_flush; 
    
    // fifo_offline
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_offline; 
    
    // fifo_ptr_rptr
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__fifo_ptr_rptr; 
    
    // fifo_ptr_wptr
    ret_val = ret_val << 9; ret_val = ret_val  | int_var__fifo_ptr_wptr; 
    
    // fifo_ptr_written
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_ptr_written; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfg_dpr_phv_fifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_bypass; 
    
    // ecc_correct
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_correct; 
    
    // ecc_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_detect; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::all() const {
    cpp_int ret_val;

    // sta_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_full; 
    
    // sta_empty
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_empty; 
    
    // sta_ptr_rptr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__sta_ptr_rptr; 
    
    // sta_ptr_wptr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__sta_ptr_wptr; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::all() const {
    cpp_int ret_val;

    // fifo_flush
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_flush; 
    
    // fifo_offline
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_offline; 
    
    // fifo_ptr_rptr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__fifo_ptr_rptr; 
    
    // fifo_ptr_wptr
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__fifo_ptr_wptr; 
    
    // fifo_ptr_written
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_ptr_written; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_bypass; 
    
    // ecc_correct
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_correct; 
    
    // ecc_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_detect; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::all() const {
    cpp_int ret_val;

    // sta_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_full; 
    
    // sta_empty
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_empty; 
    
    // sta_ptr_rptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sta_ptr_rptr; 
    
    // sta_ptr_wptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sta_ptr_wptr; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::all() const {
    cpp_int ret_val;

    // fifo_flush
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_flush; 
    
    // fifo_offline
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_offline; 
    
    // fifo_ptr_rptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__fifo_ptr_rptr; 
    
    // fifo_ptr_wptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__fifo_ptr_wptr; 
    
    // fifo_ptr_written
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_ptr_written; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_bypass; 
    
    // ecc_correct
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_correct; 
    
    // ecc_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_detect; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 72; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::all() const {
    cpp_int ret_val;

    // sta_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_full; 
    
    // sta_empty
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_empty; 
    
    // sta_ptr_rptr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__sta_ptr_rptr; 
    
    // sta_ptr_wptr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__sta_ptr_wptr; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::all() const {
    cpp_int ret_val;

    // fifo_flush
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_flush; 
    
    // fifo_offline
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_offline; 
    
    // fifo_ptr_rptr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__fifo_ptr_rptr; 
    
    // fifo_ptr_wptr
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__fifo_ptr_wptr; 
    
    // fifo_ptr_written
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_ptr_written; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::all() const {
    cpp_int ret_val;

    // bist_run
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_run; 
    
    // ecc_bypass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_bypass; 
    
    // ecc_correct
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_correct; 
    
    // ecc_detect
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ecc_detect; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::all() const {
    cpp_int ret_val;

    // bist_done_pass
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_pass; 
    
    // bist_done_fail
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__bist_done_fail; 
    
    // addr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__addr; 
    
    // syndrome
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__syndrome; 
    
    // correctable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable; 
    
    // uncorrectable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::all() const {
    cpp_int ret_val;

    // sta_full
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_full; 
    
    // sta_empty
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sta_empty; 
    
    // sta_ptr_rptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sta_ptr_rptr; 
    
    // sta_ptr_wptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sta_ptr_wptr; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::all() const {
    cpp_int ret_val;

    // fifo_flush
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_flush; 
    
    // fifo_offline
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_offline; 
    
    // fifo_ptr_rptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__fifo_ptr_rptr; 
    
    // fifo_ptr_wptr
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__fifo_ptr_wptr; 
    
    // fifo_ptr_written
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__fifo_ptr_written; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::all() const {
    cpp_int ret_val;

    #if 832 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 832-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__ecc; 
    
    // data
    ret_val = ret_val << 521; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::all() const {
    cpp_int ret_val;

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 512-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__ecc; 
    
    // data
    ret_val = ret_val << 521; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::all() const {
    cpp_int ret_val;

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 64-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__ecc; 
    
    // data
    ret_val = ret_val << 515; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::all() const {
    cpp_int ret_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 20; ret_val = ret_val  | int_var__ecc; 
    
    // data
    ret_val = ret_val << 515; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::all() const {
    cpp_int ret_val;

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 32-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 72; ret_val = ret_val  | int_var__ecc; 
    
    // data
    ret_val = ret_val << 1218; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::all() const {
    cpp_int ret_val;

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("all function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 16-1; ii >= 0; ii--) {
         ret_val = ret_val << entry[ii].get_width(); ret_val = ret_val  | entry[ii].all(); 
    }
    #endif
    
    return ret_val;
}

cpp_int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::all() const {
    cpp_int ret_val;

    // ecc
    ret_val = ret_val << 36; ret_val = ret_val  | int_var__ecc; 
    
    // data
    ret_val = ret_val << 521; ret_val = ret_val  | int_var__data; 
    
    return ret_val;
}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::clear() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 16; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::clear() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 32; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::clear() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 16; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::clear() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 64; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::clear() {

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 512; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::clear() {

    int_var__data = 0; 
    
    int_var__ecc = 0; 
    
}

void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::clear() {

    #if 832 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    PLOG_ERR("clear function for large_array not implemented" << endl);
    #else
    // entry
    for(int ii = 0; ii < 832; ii++) {
        entry[ii].clear();
    }
    #endif
    
}

void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::clear() {

    int_var__fifo_ptr_written = 0; 
    
    int_var__fifo_ptr_wptr = 0; 
    
    int_var__fifo_ptr_rptr = 0; 
    
    int_var__fifo_offline = 0; 
    
    int_var__fifo_flush = 0; 
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::clear() {

    int_var__sta_ptr_wptr = 0; 
    
    int_var__sta_ptr_rptr = 0; 
    
    int_var__sta_empty = 0; 
    
    int_var__sta_full = 0; 
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::clear() {

    int_var__ecc_detect = 0; 
    
    int_var__ecc_correct = 0; 
    
    int_var__ecc_bypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::clear() {

    int_var__fifo_ptr_written = 0; 
    
    int_var__fifo_ptr_wptr = 0; 
    
    int_var__fifo_ptr_rptr = 0; 
    
    int_var__fifo_offline = 0; 
    
    int_var__fifo_flush = 0; 
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::clear() {

    int_var__sta_ptr_wptr = 0; 
    
    int_var__sta_ptr_rptr = 0; 
    
    int_var__sta_empty = 0; 
    
    int_var__sta_full = 0; 
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::clear() {

    int_var__ecc_detect = 0; 
    
    int_var__ecc_correct = 0; 
    
    int_var__ecc_bypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::clear() {

    int_var__fifo_ptr_written = 0; 
    
    int_var__fifo_ptr_wptr = 0; 
    
    int_var__fifo_ptr_rptr = 0; 
    
    int_var__fifo_offline = 0; 
    
    int_var__fifo_flush = 0; 
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::clear() {

    int_var__sta_ptr_wptr = 0; 
    
    int_var__sta_ptr_rptr = 0; 
    
    int_var__sta_empty = 0; 
    
    int_var__sta_full = 0; 
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::clear() {

    int_var__ecc_detect = 0; 
    
    int_var__ecc_correct = 0; 
    
    int_var__ecc_bypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::clear() {

    int_var__fifo_ptr_written = 0; 
    
    int_var__fifo_ptr_wptr = 0; 
    
    int_var__fifo_ptr_rptr = 0; 
    
    int_var__fifo_offline = 0; 
    
    int_var__fifo_flush = 0; 
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::clear() {

    int_var__sta_ptr_wptr = 0; 
    
    int_var__sta_ptr_rptr = 0; 
    
    int_var__sta_empty = 0; 
    
    int_var__sta_full = 0; 
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_dprmem_csr_cfg_dpr_phv_fifo_t::clear() {

    int_var__ecc_detect = 0; 
    
    int_var__ecc_correct = 0; 
    
    int_var__ecc_bypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::clear() {

    int_var__fifo_ptr_written = 0; 
    
    int_var__fifo_ptr_wptr = 0; 
    
    int_var__fifo_ptr_rptr = 0; 
    
    int_var__fifo_offline = 0; 
    
    int_var__fifo_flush = 0; 
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::clear() {

    int_var__sta_ptr_wptr = 0; 
    
    int_var__sta_ptr_rptr = 0; 
    
    int_var__sta_empty = 0; 
    
    int_var__sta_full = 0; 
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_dprmem_csr_cfg_dpr_csum_fifo_t::clear() {

    int_var__ecc_detect = 0; 
    
    int_var__ecc_correct = 0; 
    
    int_var__ecc_bypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::clear() {

    int_var__fifo_ptr_written = 0; 
    
    int_var__fifo_ptr_wptr = 0; 
    
    int_var__fifo_ptr_rptr = 0; 
    
    int_var__fifo_offline = 0; 
    
    int_var__fifo_flush = 0; 
    
}

void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::clear() {

    int_var__sta_ptr_wptr = 0; 
    
    int_var__sta_ptr_rptr = 0; 
    
    int_var__sta_empty = 0; 
    
    int_var__sta_full = 0; 
    
}

void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::clear() {

    int_var__uncorrectable = 0; 
    
    int_var__correctable = 0; 
    
    int_var__syndrome = 0; 
    
    int_var__addr = 0; 
    
    int_var__bist_done_fail = 0; 
    
    int_var__bist_done_pass = 0; 
    
}

void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::clear() {

    int_var__ecc_detect = 0; 
    
    int_var__ecc_correct = 0; 
    
    int_var__ecc_bypass = 0; 
    
    int_var__bist_run = 0; 
    
}

void cap_dprmem_csr_t::clear() {

    cfg_dpr_pktin_fifo.clear();
    sta_srams_ecc_dpr_pktin_fifo.clear();
    sta_ff_ptr_dpr_pktin_fifo.clear();
    cfw_ff_dpr_pktin_fifo.clear();
    cfg_dpr_csum_fifo.clear();
    sta_srams_ecc_dpr_csum_fifo.clear();
    sta_ff_ptr_dpr_csum_fifo.clear();
    cfw_ff_dpr_csum_fifo.clear();
    cfg_dpr_phv_fifo.clear();
    sta_srams_ecc_dpr_phv_fifo.clear();
    sta_ff_ptr_dpr_phv_fifo.clear();
    cfw_ff_dpr_phv_fifo.clear();
    cfg_dpr_ohi_fifo.clear();
    sta_srams_ecc_dpr_ohi_fifo.clear();
    sta_ff_ptr_dpr_ohi_fifo.clear();
    cfw_ff_dpr_ohi_fifo.clear();
    cfg_dpr_ptr_fifo.clear();
    sta_srams_ecc_dpr_ptr_fifo.clear();
    sta_ff_ptr_dpr_ptr_fifo.clear();
    cfw_ff_dpr_ptr_fifo.clear();
    cfg_dpr_pktout_fifo.clear();
    sta_srams_ecc_dpr_pktout_fifo.clear();
    sta_ff_ptr_dpr_pktout_fifo.clear();
    cfw_ff_dpr_pktout_fifo.clear();
    dhs_dpr_pktin_fifo_sram.clear();
    dhs_dpr_csum_fifo_sram.clear();
    dhs_dpr_phv_fifo_sram.clear();
    dhs_dpr_ohi_fifo_sram.clear();
    dhs_dpr_ptr_fifo_sram.clear();
    dhs_dpr_pktout_fifo_sram.clear();
}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::init() {

}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::init() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::init() {

}

void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::init() {

    #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 32; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::init() {

}

void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::init() {

    #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 16; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::init() {

}

void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::init() {

    #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 64; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::init() {

}

void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::init() {

    #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 512; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::init() {

}

void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::init() {

    #if 832 > CAP_CSR_LARGE_ARRAY_THRESHOLD
    entry.set_attributes(this, "entry", 0x0);
    #else
    for(int ii = 0; ii < 832; ii++) {
        if(ii != 0) entry[ii].set_field_init_done(true, true);
        entry[ii].set_attributes(this,"entry["+to_string(ii)+"]",  0x0 + (entry[ii].get_byte_size()*ii));
    }
    #endif
    
}

void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::init() {

}

void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::init() {

}

void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::init() {

}

void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::init() {

}

void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::init() {

}

void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::init() {

}

void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::init() {

}

void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::init() {

}

void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::init() {

}

void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::init() {

}

void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::init() {

}

void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::init() {

}

void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::init() {

}

void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::init() {

}

void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::init() {

}

void cap_dprmem_csr_cfg_dpr_phv_fifo_t::init() {

}

void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::init() {

}

void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::init() {

}

void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::init() {

}

void cap_dprmem_csr_cfg_dpr_csum_fifo_t::init() {

}

void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::init() {

}

void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::init() {

}

void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::init() {

}

void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::init() {

}

void cap_dprmem_csr_t::init() {

    cfg_dpr_pktin_fifo.set_attributes(this,"cfg_dpr_pktin_fifo", 0x20000 );
    sta_srams_ecc_dpr_pktin_fifo.set_attributes(this,"sta_srams_ecc_dpr_pktin_fifo", 0x20008 );
    sta_ff_ptr_dpr_pktin_fifo.set_attributes(this,"sta_ff_ptr_dpr_pktin_fifo", 0x20010 );
    cfw_ff_dpr_pktin_fifo.set_attributes(this,"cfw_ff_dpr_pktin_fifo", 0x20014 );
    cfg_dpr_csum_fifo.set_attributes(this,"cfg_dpr_csum_fifo", 0x40000 );
    sta_srams_ecc_dpr_csum_fifo.set_attributes(this,"sta_srams_ecc_dpr_csum_fifo", 0x40008 );
    sta_ff_ptr_dpr_csum_fifo.set_attributes(this,"sta_ff_ptr_dpr_csum_fifo", 0x40010 );
    cfw_ff_dpr_csum_fifo.set_attributes(this,"cfw_ff_dpr_csum_fifo", 0x40014 );
    cfg_dpr_phv_fifo.set_attributes(this,"cfg_dpr_phv_fifo", 0x44000 );
    sta_srams_ecc_dpr_phv_fifo.set_attributes(this,"sta_srams_ecc_dpr_phv_fifo", 0x44008 );
    sta_ff_ptr_dpr_phv_fifo.set_attributes(this,"sta_ff_ptr_dpr_phv_fifo", 0x44010 );
    cfw_ff_dpr_phv_fifo.set_attributes(this,"cfw_ff_dpr_phv_fifo", 0x44014 );
    cfg_dpr_ohi_fifo.set_attributes(this,"cfg_dpr_ohi_fifo", 0x45000 );
    sta_srams_ecc_dpr_ohi_fifo.set_attributes(this,"sta_srams_ecc_dpr_ohi_fifo", 0x45004 );
    sta_ff_ptr_dpr_ohi_fifo.set_attributes(this,"sta_ff_ptr_dpr_ohi_fifo", 0x45008 );
    cfw_ff_dpr_ohi_fifo.set_attributes(this,"cfw_ff_dpr_ohi_fifo", 0x4500c );
    cfg_dpr_ptr_fifo.set_attributes(this,"cfg_dpr_ptr_fifo", 0x48000 );
    sta_srams_ecc_dpr_ptr_fifo.set_attributes(this,"sta_srams_ecc_dpr_ptr_fifo", 0x48010 );
    sta_ff_ptr_dpr_ptr_fifo.set_attributes(this,"sta_ff_ptr_dpr_ptr_fifo", 0x48020 );
    cfw_ff_dpr_ptr_fifo.set_attributes(this,"cfw_ff_dpr_ptr_fifo", 0x48024 );
    cfg_dpr_pktout_fifo.set_attributes(this,"cfg_dpr_pktout_fifo", 0x49000 );
    sta_srams_ecc_dpr_pktout_fifo.set_attributes(this,"sta_srams_ecc_dpr_pktout_fifo", 0x49008 );
    sta_ff_ptr_dpr_pktout_fifo.set_attributes(this,"sta_ff_ptr_dpr_pktout_fifo", 0x49010 );
    cfw_ff_dpr_pktout_fifo.set_attributes(this,"cfw_ff_dpr_pktout_fifo", 0x49014 );
    dhs_dpr_pktin_fifo_sram.set_attributes(this,"dhs_dpr_pktin_fifo_sram", 0x0 );
    dhs_dpr_csum_fifo_sram.set_attributes(this,"dhs_dpr_csum_fifo_sram", 0x30000 );
    dhs_dpr_phv_fifo_sram.set_attributes(this,"dhs_dpr_phv_fifo_sram", 0x42000 );
    dhs_dpr_ohi_fifo_sram.set_attributes(this,"dhs_dpr_ohi_fifo_sram", 0x44800 );
    dhs_dpr_ptr_fifo_sram.set_attributes(this,"dhs_dpr_ptr_fifo_sram", 0x46000 );
    dhs_dpr_pktout_fifo_sram.set_attributes(this,"dhs_dpr_pktout_fifo_sram", 0x48800 );
}

void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::data() const {
    return int_var__data;
}
    
void cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::data() const {
    return int_var__data;
}
    
void cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::data() const {
    return int_var__data;
}
    
void cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::data() const {
    return int_var__data;
}
    
void cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::data() const {
    return int_var__data;
}
    
void cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::data(const cpp_int & _val) { 
    // data
    int_var__data = _val.convert_to< data_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::data() const {
    return int_var__data;
}
    
void cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::ecc(const cpp_int & _val) { 
    // ecc
    int_var__ecc = _val.convert_to< ecc_cpp_int_t >();
}

cpp_int cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::ecc() const {
    return int_var__ecc;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_ptr_written(const cpp_int & _val) { 
    // fifo_ptr_written
    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_ptr_written() const {
    return int_var__fifo_ptr_written;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_ptr_wptr(const cpp_int & _val) { 
    // fifo_ptr_wptr
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_ptr_wptr() const {
    return int_var__fifo_ptr_wptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_ptr_rptr(const cpp_int & _val) { 
    // fifo_ptr_rptr
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_ptr_rptr() const {
    return int_var__fifo_ptr_rptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_offline(const cpp_int & _val) { 
    // fifo_offline
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_offline() const {
    return int_var__fifo_offline;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_flush(const cpp_int & _val) { 
    // fifo_flush
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::fifo_flush() const {
    return int_var__fifo_flush;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_ptr_wptr(const cpp_int & _val) { 
    // sta_ptr_wptr
    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_ptr_wptr() const {
    return int_var__sta_ptr_wptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_ptr_rptr(const cpp_int & _val) { 
    // sta_ptr_rptr
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_ptr_rptr() const {
    return int_var__sta_ptr_rptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_empty(const cpp_int & _val) { 
    // sta_empty
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_empty() const {
    return int_var__sta_empty;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_full(const cpp_int & _val) { 
    // sta_full
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::sta_full() const {
    return int_var__sta_full;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::correctable() const {
    return int_var__correctable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::addr() const {
    return int_var__addr;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::ecc_detect(const cpp_int & _val) { 
    // ecc_detect
    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::ecc_detect() const {
    return int_var__ecc_detect;
}
    
void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::ecc_correct(const cpp_int & _val) { 
    // ecc_correct
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::ecc_correct() const {
    return int_var__ecc_correct;
}
    
void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::ecc_bypass(const cpp_int & _val) { 
    // ecc_bypass
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::ecc_bypass() const {
    return int_var__ecc_bypass;
}
    
void cap_dprmem_csr_cfg_dpr_pktout_fifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktout_fifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_ptr_written(const cpp_int & _val) { 
    // fifo_ptr_written
    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_ptr_written() const {
    return int_var__fifo_ptr_written;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_ptr_wptr(const cpp_int & _val) { 
    // fifo_ptr_wptr
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_ptr_wptr() const {
    return int_var__fifo_ptr_wptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_ptr_rptr(const cpp_int & _val) { 
    // fifo_ptr_rptr
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_ptr_rptr() const {
    return int_var__fifo_ptr_rptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_offline(const cpp_int & _val) { 
    // fifo_offline
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_offline() const {
    return int_var__fifo_offline;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_flush(const cpp_int & _val) { 
    // fifo_flush
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::fifo_flush() const {
    return int_var__fifo_flush;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_ptr_wptr(const cpp_int & _val) { 
    // sta_ptr_wptr
    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_ptr_wptr() const {
    return int_var__sta_ptr_wptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_ptr_rptr(const cpp_int & _val) { 
    // sta_ptr_rptr
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_ptr_rptr() const {
    return int_var__sta_ptr_rptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_empty(const cpp_int & _val) { 
    // sta_empty
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_empty() const {
    return int_var__sta_empty;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_full(const cpp_int & _val) { 
    // sta_full
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::sta_full() const {
    return int_var__sta_full;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::correctable() const {
    return int_var__correctable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::addr() const {
    return int_var__addr;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::ecc_detect(const cpp_int & _val) { 
    // ecc_detect
    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::ecc_detect() const {
    return int_var__ecc_detect;
}
    
void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::ecc_correct(const cpp_int & _val) { 
    // ecc_correct
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::ecc_correct() const {
    return int_var__ecc_correct;
}
    
void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::ecc_bypass(const cpp_int & _val) { 
    // ecc_bypass
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::ecc_bypass() const {
    return int_var__ecc_bypass;
}
    
void cap_dprmem_csr_cfg_dpr_ptr_fifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ptr_fifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_ptr_written(const cpp_int & _val) { 
    // fifo_ptr_written
    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_ptr_written() const {
    return int_var__fifo_ptr_written;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_ptr_wptr(const cpp_int & _val) { 
    // fifo_ptr_wptr
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_ptr_wptr() const {
    return int_var__fifo_ptr_wptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_ptr_rptr(const cpp_int & _val) { 
    // fifo_ptr_rptr
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_ptr_rptr() const {
    return int_var__fifo_ptr_rptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_offline(const cpp_int & _val) { 
    // fifo_offline
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_offline() const {
    return int_var__fifo_offline;
}
    
void cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_flush(const cpp_int & _val) { 
    // fifo_flush
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::fifo_flush() const {
    return int_var__fifo_flush;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_ptr_wptr(const cpp_int & _val) { 
    // sta_ptr_wptr
    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_ptr_wptr() const {
    return int_var__sta_ptr_wptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_ptr_rptr(const cpp_int & _val) { 
    // sta_ptr_rptr
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_ptr_rptr() const {
    return int_var__sta_ptr_rptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_empty(const cpp_int & _val) { 
    // sta_empty
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_empty() const {
    return int_var__sta_empty;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_full(const cpp_int & _val) { 
    // sta_full
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::sta_full() const {
    return int_var__sta_full;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::correctable() const {
    return int_var__correctable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::addr() const {
    return int_var__addr;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::ecc_detect(const cpp_int & _val) { 
    // ecc_detect
    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::ecc_detect() const {
    return int_var__ecc_detect;
}
    
void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::ecc_correct(const cpp_int & _val) { 
    // ecc_correct
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::ecc_correct() const {
    return int_var__ecc_correct;
}
    
void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::ecc_bypass(const cpp_int & _val) { 
    // ecc_bypass
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::ecc_bypass() const {
    return int_var__ecc_bypass;
}
    
void cap_dprmem_csr_cfg_dpr_ohi_fifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_ohi_fifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_ptr_written(const cpp_int & _val) { 
    // fifo_ptr_written
    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_ptr_written() const {
    return int_var__fifo_ptr_written;
}
    
void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_ptr_wptr(const cpp_int & _val) { 
    // fifo_ptr_wptr
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_ptr_wptr() const {
    return int_var__fifo_ptr_wptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_ptr_rptr(const cpp_int & _val) { 
    // fifo_ptr_rptr
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_ptr_rptr() const {
    return int_var__fifo_ptr_rptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_offline(const cpp_int & _val) { 
    // fifo_offline
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_offline() const {
    return int_var__fifo_offline;
}
    
void cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_flush(const cpp_int & _val) { 
    // fifo_flush
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::fifo_flush() const {
    return int_var__fifo_flush;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_ptr_wptr(const cpp_int & _val) { 
    // sta_ptr_wptr
    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_ptr_wptr() const {
    return int_var__sta_ptr_wptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_ptr_rptr(const cpp_int & _val) { 
    // sta_ptr_rptr
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_ptr_rptr() const {
    return int_var__sta_ptr_rptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_empty(const cpp_int & _val) { 
    // sta_empty
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_empty() const {
    return int_var__sta_empty;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_full(const cpp_int & _val) { 
    // sta_full
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::sta_full() const {
    return int_var__sta_full;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::correctable() const {
    return int_var__correctable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::addr() const {
    return int_var__addr;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_dprmem_csr_cfg_dpr_phv_fifo_t::ecc_detect(const cpp_int & _val) { 
    // ecc_detect
    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_phv_fifo_t::ecc_detect() const {
    return int_var__ecc_detect;
}
    
void cap_dprmem_csr_cfg_dpr_phv_fifo_t::ecc_correct(const cpp_int & _val) { 
    // ecc_correct
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_phv_fifo_t::ecc_correct() const {
    return int_var__ecc_correct;
}
    
void cap_dprmem_csr_cfg_dpr_phv_fifo_t::ecc_bypass(const cpp_int & _val) { 
    // ecc_bypass
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_phv_fifo_t::ecc_bypass() const {
    return int_var__ecc_bypass;
}
    
void cap_dprmem_csr_cfg_dpr_phv_fifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_phv_fifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_ptr_written(const cpp_int & _val) { 
    // fifo_ptr_written
    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_ptr_written() const {
    return int_var__fifo_ptr_written;
}
    
void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_ptr_wptr(const cpp_int & _val) { 
    // fifo_ptr_wptr
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_ptr_wptr() const {
    return int_var__fifo_ptr_wptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_ptr_rptr(const cpp_int & _val) { 
    // fifo_ptr_rptr
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_ptr_rptr() const {
    return int_var__fifo_ptr_rptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_offline(const cpp_int & _val) { 
    // fifo_offline
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_offline() const {
    return int_var__fifo_offline;
}
    
void cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_flush(const cpp_int & _val) { 
    // fifo_flush
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::fifo_flush() const {
    return int_var__fifo_flush;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_ptr_wptr(const cpp_int & _val) { 
    // sta_ptr_wptr
    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_ptr_wptr() const {
    return int_var__sta_ptr_wptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_ptr_rptr(const cpp_int & _val) { 
    // sta_ptr_rptr
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_ptr_rptr() const {
    return int_var__sta_ptr_rptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_empty(const cpp_int & _val) { 
    // sta_empty
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_empty() const {
    return int_var__sta_empty;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_full(const cpp_int & _val) { 
    // sta_full
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::sta_full() const {
    return int_var__sta_full;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::correctable() const {
    return int_var__correctable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::addr() const {
    return int_var__addr;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_dprmem_csr_cfg_dpr_csum_fifo_t::ecc_detect(const cpp_int & _val) { 
    // ecc_detect
    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_csum_fifo_t::ecc_detect() const {
    return int_var__ecc_detect;
}
    
void cap_dprmem_csr_cfg_dpr_csum_fifo_t::ecc_correct(const cpp_int & _val) { 
    // ecc_correct
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_csum_fifo_t::ecc_correct() const {
    return int_var__ecc_correct;
}
    
void cap_dprmem_csr_cfg_dpr_csum_fifo_t::ecc_bypass(const cpp_int & _val) { 
    // ecc_bypass
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_csum_fifo_t::ecc_bypass() const {
    return int_var__ecc_bypass;
}
    
void cap_dprmem_csr_cfg_dpr_csum_fifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_csum_fifo_t::bist_run() const {
    return int_var__bist_run;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_ptr_written(const cpp_int & _val) { 
    // fifo_ptr_written
    int_var__fifo_ptr_written = _val.convert_to< fifo_ptr_written_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_ptr_written() const {
    return int_var__fifo_ptr_written;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_ptr_wptr(const cpp_int & _val) { 
    // fifo_ptr_wptr
    int_var__fifo_ptr_wptr = _val.convert_to< fifo_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_ptr_wptr() const {
    return int_var__fifo_ptr_wptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_ptr_rptr(const cpp_int & _val) { 
    // fifo_ptr_rptr
    int_var__fifo_ptr_rptr = _val.convert_to< fifo_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_ptr_rptr() const {
    return int_var__fifo_ptr_rptr;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_offline(const cpp_int & _val) { 
    // fifo_offline
    int_var__fifo_offline = _val.convert_to< fifo_offline_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_offline() const {
    return int_var__fifo_offline;
}
    
void cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_flush(const cpp_int & _val) { 
    // fifo_flush
    int_var__fifo_flush = _val.convert_to< fifo_flush_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::fifo_flush() const {
    return int_var__fifo_flush;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_ptr_wptr(const cpp_int & _val) { 
    // sta_ptr_wptr
    int_var__sta_ptr_wptr = _val.convert_to< sta_ptr_wptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_ptr_wptr() const {
    return int_var__sta_ptr_wptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_ptr_rptr(const cpp_int & _val) { 
    // sta_ptr_rptr
    int_var__sta_ptr_rptr = _val.convert_to< sta_ptr_rptr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_ptr_rptr() const {
    return int_var__sta_ptr_rptr;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_empty(const cpp_int & _val) { 
    // sta_empty
    int_var__sta_empty = _val.convert_to< sta_empty_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_empty() const {
    return int_var__sta_empty;
}
    
void cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_full(const cpp_int & _val) { 
    // sta_full
    int_var__sta_full = _val.convert_to< sta_full_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::sta_full() const {
    return int_var__sta_full;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::uncorrectable(const cpp_int & _val) { 
    // uncorrectable
    int_var__uncorrectable = _val.convert_to< uncorrectable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::uncorrectable() const {
    return int_var__uncorrectable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::correctable(const cpp_int & _val) { 
    // correctable
    int_var__correctable = _val.convert_to< correctable_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::correctable() const {
    return int_var__correctable;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::syndrome(const cpp_int & _val) { 
    // syndrome
    int_var__syndrome = _val.convert_to< syndrome_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::syndrome() const {
    return int_var__syndrome;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::addr(const cpp_int & _val) { 
    // addr
    int_var__addr = _val.convert_to< addr_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::addr() const {
    return int_var__addr;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::bist_done_fail(const cpp_int & _val) { 
    // bist_done_fail
    int_var__bist_done_fail = _val.convert_to< bist_done_fail_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::bist_done_fail() const {
    return int_var__bist_done_fail;
}
    
void cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::bist_done_pass(const cpp_int & _val) { 
    // bist_done_pass
    int_var__bist_done_pass = _val.convert_to< bist_done_pass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::bist_done_pass() const {
    return int_var__bist_done_pass;
}
    
void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::ecc_detect(const cpp_int & _val) { 
    // ecc_detect
    int_var__ecc_detect = _val.convert_to< ecc_detect_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::ecc_detect() const {
    return int_var__ecc_detect;
}
    
void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::ecc_correct(const cpp_int & _val) { 
    // ecc_correct
    int_var__ecc_correct = _val.convert_to< ecc_correct_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::ecc_correct() const {
    return int_var__ecc_correct;
}
    
void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::ecc_bypass(const cpp_int & _val) { 
    // ecc_bypass
    int_var__ecc_bypass = _val.convert_to< ecc_bypass_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::ecc_bypass() const {
    return int_var__ecc_bypass;
}
    
void cap_dprmem_csr_cfg_dpr_pktin_fifo_t::bist_run(const cpp_int & _val) { 
    // bist_run
    int_var__bist_run = _val.convert_to< bist_run_cpp_int_t >();
}

cpp_int cap_dprmem_csr_cfg_dpr_pktin_fifo_t::bist_run() const {
    return int_var__bist_run;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { field_val = data(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { field_val = ecc(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { field_val = fifo_ptr_written(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { field_val = fifo_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { field_val = fifo_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { field_val = fifo_offline(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { field_val = fifo_flush(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { field_val = sta_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { field_val = sta_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { field_val = sta_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { field_val = sta_full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_pktout_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { field_val = ecc_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { field_val = ecc_correct(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { field_val = ecc_bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { field_val = fifo_ptr_written(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { field_val = fifo_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { field_val = fifo_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { field_val = fifo_offline(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { field_val = fifo_flush(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { field_val = sta_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { field_val = sta_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { field_val = sta_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { field_val = sta_full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_ptr_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { field_val = ecc_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { field_val = ecc_correct(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { field_val = ecc_bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { field_val = fifo_ptr_written(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { field_val = fifo_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { field_val = fifo_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { field_val = fifo_offline(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { field_val = fifo_flush(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { field_val = sta_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { field_val = sta_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { field_val = sta_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { field_val = sta_full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_ohi_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { field_val = ecc_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { field_val = ecc_correct(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { field_val = ecc_bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { field_val = fifo_ptr_written(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { field_val = fifo_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { field_val = fifo_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { field_val = fifo_offline(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { field_val = fifo_flush(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { field_val = sta_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { field_val = sta_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { field_val = sta_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { field_val = sta_full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_phv_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { field_val = ecc_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { field_val = ecc_correct(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { field_val = ecc_bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { field_val = fifo_ptr_written(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { field_val = fifo_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { field_val = fifo_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { field_val = fifo_offline(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { field_val = fifo_flush(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { field_val = sta_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { field_val = sta_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { field_val = sta_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { field_val = sta_full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_csum_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { field_val = ecc_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { field_val = ecc_correct(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { field_val = ecc_bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { field_val = fifo_ptr_written(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { field_val = fifo_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { field_val = fifo_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { field_val = fifo_offline(); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { field_val = fifo_flush(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { field_val = sta_ptr_wptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { field_val = sta_ptr_rptr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { field_val = sta_empty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { field_val = sta_full(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { field_val = uncorrectable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { field_val = correctable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { field_val = syndrome(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { field_val = addr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { field_val = bist_done_fail(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { field_val = bist_done_pass(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_pktin_fifo_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { field_val = ecc_detect(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { field_val = ecc_correct(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { field_val = ecc_bypass(); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { field_val = bist_run(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = cfg_dpr_pktin_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_pktin_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_pktin_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_pktin_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_csum_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_csum_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_csum_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_csum_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_phv_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_phv_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_phv_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_phv_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_ohi_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_ohi_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_ohi_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_ohi_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_ptr_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_ptr_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_ptr_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_ptr_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_pktout_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_pktout_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_pktout_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_pktout_fifo.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_pktin_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_csum_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_phv_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_ohi_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_ptr_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_pktout_fifo_sram.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "data")) { data(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc")) { ecc(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { fifo_ptr_written(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { fifo_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { fifo_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { fifo_offline(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { fifo_flush(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { sta_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { sta_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { sta_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { sta_full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_pktout_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { ecc_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { ecc_correct(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { ecc_bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { fifo_ptr_written(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { fifo_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { fifo_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { fifo_offline(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { fifo_flush(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { sta_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { sta_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { sta_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { sta_full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_ptr_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { ecc_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { ecc_correct(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { ecc_bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { fifo_ptr_written(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { fifo_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { fifo_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { fifo_offline(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { fifo_flush(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { sta_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { sta_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { sta_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { sta_full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_ohi_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { ecc_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { ecc_correct(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { ecc_bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { fifo_ptr_written(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { fifo_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { fifo_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { fifo_offline(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { fifo_flush(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { sta_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { sta_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { sta_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { sta_full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_phv_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { ecc_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { ecc_correct(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { ecc_bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { fifo_ptr_written(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { fifo_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { fifo_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { fifo_offline(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { fifo_flush(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { sta_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { sta_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { sta_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { sta_full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_csum_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { ecc_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { ecc_correct(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { ecc_bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fifo_ptr_written")) { fifo_ptr_written(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_wptr")) { fifo_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_ptr_rptr")) { fifo_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_offline")) { fifo_offline(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "fifo_flush")) { fifo_flush(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "sta_ptr_wptr")) { sta_ptr_wptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_ptr_rptr")) { sta_ptr_rptr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_empty")) { sta_empty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_full")) { sta_full(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "uncorrectable")) { uncorrectable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable")) { correctable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome")) { syndrome(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr")) { addr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_fail")) { bist_done_fail(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_done_pass")) { bist_done_pass(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_cfg_dpr_pktin_fifo_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "ecc_detect")) { ecc_detect(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_correct")) { ecc_correct(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ecc_bypass")) { ecc_bypass(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "bist_run")) { bist_run(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_dprmem_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = cfg_dpr_pktin_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_pktin_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_pktin_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_pktin_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_csum_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_csum_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_csum_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_csum_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_phv_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_phv_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_phv_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_phv_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_ohi_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_ohi_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_ohi_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_ohi_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_ptr_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_ptr_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_ptr_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_ptr_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dpr_pktout_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_srams_ecc_dpr_pktout_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ff_ptr_dpr_pktout_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfw_ff_dpr_pktout_fifo.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_pktin_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_csum_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_phv_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_ohi_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_ptr_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dhs_dpr_pktout_fifo_sram.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("data");
    ret_vec.push_back("ecc");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fifo_ptr_written");
    ret_vec.push_back("fifo_ptr_wptr");
    ret_vec.push_back("fifo_ptr_rptr");
    ret_vec.push_back("fifo_offline");
    ret_vec.push_back("fifo_flush");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sta_ptr_wptr");
    ret_vec.push_back("sta_ptr_rptr");
    ret_vec.push_back("sta_empty");
    ret_vec.push_back("sta_full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfg_dpr_pktout_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_detect");
    ret_vec.push_back("ecc_correct");
    ret_vec.push_back("ecc_bypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fifo_ptr_written");
    ret_vec.push_back("fifo_ptr_wptr");
    ret_vec.push_back("fifo_ptr_rptr");
    ret_vec.push_back("fifo_offline");
    ret_vec.push_back("fifo_flush");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sta_ptr_wptr");
    ret_vec.push_back("sta_ptr_rptr");
    ret_vec.push_back("sta_empty");
    ret_vec.push_back("sta_full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfg_dpr_ptr_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_detect");
    ret_vec.push_back("ecc_correct");
    ret_vec.push_back("ecc_bypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fifo_ptr_written");
    ret_vec.push_back("fifo_ptr_wptr");
    ret_vec.push_back("fifo_ptr_rptr");
    ret_vec.push_back("fifo_offline");
    ret_vec.push_back("fifo_flush");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sta_ptr_wptr");
    ret_vec.push_back("sta_ptr_rptr");
    ret_vec.push_back("sta_empty");
    ret_vec.push_back("sta_full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfg_dpr_ohi_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_detect");
    ret_vec.push_back("ecc_correct");
    ret_vec.push_back("ecc_bypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fifo_ptr_written");
    ret_vec.push_back("fifo_ptr_wptr");
    ret_vec.push_back("fifo_ptr_rptr");
    ret_vec.push_back("fifo_offline");
    ret_vec.push_back("fifo_flush");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sta_ptr_wptr");
    ret_vec.push_back("sta_ptr_rptr");
    ret_vec.push_back("sta_empty");
    ret_vec.push_back("sta_full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfg_dpr_phv_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_detect");
    ret_vec.push_back("ecc_correct");
    ret_vec.push_back("ecc_bypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fifo_ptr_written");
    ret_vec.push_back("fifo_ptr_wptr");
    ret_vec.push_back("fifo_ptr_rptr");
    ret_vec.push_back("fifo_offline");
    ret_vec.push_back("fifo_flush");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sta_ptr_wptr");
    ret_vec.push_back("sta_ptr_rptr");
    ret_vec.push_back("sta_empty");
    ret_vec.push_back("sta_full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfg_dpr_csum_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_detect");
    ret_vec.push_back("ecc_correct");
    ret_vec.push_back("ecc_bypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fifo_ptr_written");
    ret_vec.push_back("fifo_ptr_wptr");
    ret_vec.push_back("fifo_ptr_rptr");
    ret_vec.push_back("fifo_offline");
    ret_vec.push_back("fifo_flush");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("sta_ptr_wptr");
    ret_vec.push_back("sta_ptr_rptr");
    ret_vec.push_back("sta_empty");
    ret_vec.push_back("sta_full");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("uncorrectable");
    ret_vec.push_back("correctable");
    ret_vec.push_back("syndrome");
    ret_vec.push_back("addr");
    ret_vec.push_back("bist_done_fail");
    ret_vec.push_back("bist_done_pass");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_cfg_dpr_pktin_fifo_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("ecc_detect");
    ret_vec.push_back("ecc_correct");
    ret_vec.push_back("ecc_bypass");
    ret_vec.push_back("bist_run");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_dprmem_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : cfg_dpr_pktin_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_dpr_pktin_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_dpr_pktin_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_dpr_pktin_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_ptr_dpr_pktin_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_ptr_dpr_pktin_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_ff_dpr_pktin_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfw_ff_dpr_pktin_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dpr_csum_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_dpr_csum_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_dpr_csum_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_dpr_csum_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_ptr_dpr_csum_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_ptr_dpr_csum_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_ff_dpr_csum_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfw_ff_dpr_csum_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dpr_phv_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_dpr_phv_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_dpr_phv_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_dpr_phv_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_ptr_dpr_phv_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_ptr_dpr_phv_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_ff_dpr_phv_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfw_ff_dpr_phv_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dpr_ohi_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_dpr_ohi_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_dpr_ohi_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_dpr_ohi_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_ptr_dpr_ohi_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_ptr_dpr_ohi_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_ff_dpr_ohi_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfw_ff_dpr_ohi_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dpr_ptr_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_dpr_ptr_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_dpr_ptr_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_dpr_ptr_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_ptr_dpr_ptr_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_ptr_dpr_ptr_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_ff_dpr_ptr_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfw_ff_dpr_ptr_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dpr_pktout_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfg_dpr_pktout_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_srams_ecc_dpr_pktout_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_srams_ecc_dpr_pktout_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ff_ptr_dpr_pktout_fifo.get_fields(level-1)) {
            ret_vec.push_back("sta_ff_ptr_dpr_pktout_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfw_ff_dpr_pktout_fifo.get_fields(level-1)) {
            ret_vec.push_back("cfw_ff_dpr_pktout_fifo." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dpr_pktin_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_dpr_pktin_fifo_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dpr_csum_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_dpr_csum_fifo_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dpr_phv_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_dpr_phv_fifo_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dpr_ohi_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_dpr_ohi_fifo_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dpr_ptr_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_dpr_ptr_fifo_sram." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dhs_dpr_pktout_fifo_sram.get_fields(level-1)) {
            ret_vec.push_back("dhs_dpr_pktout_fifo_sram." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
