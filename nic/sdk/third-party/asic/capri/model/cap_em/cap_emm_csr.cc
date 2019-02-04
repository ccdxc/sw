
#include "cap_emm_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_emm_csr_cfg_ecc_t::cap_emm_csr_cfg_ecc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_emm_csr_cfg_ecc_t::~cap_emm_csr_cfg_ecc_t() { }

cap_emm_csr_em_cfg_core_t::cap_emm_csr_em_cfg_core_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_emm_csr_em_cfg_core_t::~cap_emm_csr_em_cfg_core_t() { }

cap_emm_csr_em_sta_phy_t::cap_emm_csr_em_sta_phy_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_emm_csr_em_sta_phy_t::~cap_emm_csr_em_sta_phy_t() { }

cap_emm_csr_em_cfg_phy_t::cap_emm_csr_em_cfg_phy_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_emm_csr_em_cfg_phy_t::~cap_emm_csr_em_cfg_phy_t() { }

cap_emm_csr_t::cap_emm_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(32);
        set_attributes(0,get_name(), 0);
        }
cap_emm_csr_t::~cap_emm_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_emm_csr_cfg_ecc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".disable_cor_SRAM1: 0x" << int_var__disable_cor_SRAM1 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_emm_csr_em_cfg_core_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_asyncwkupena: 0x" << int_var__corecfg_asyncwkupena << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_tuningcount: 0x" << int_var__corecfg_tuningcount << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_emm_csr_em_sta_phy_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_biststatus: 0x" << int_var__phyctrl_biststatus << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_bistdone: 0x" << int_var__phyctrl_bistdone << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_exr_ninst: 0x" << int_var__phyctrl_exr_ninst << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_dllrdy: 0x" << int_var__phyctrl_dllrdy << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_rtrim: 0x" << int_var__phyctrl_rtrim << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_caldone: 0x" << int_var__phyctrl_caldone << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_emm_csr_em_cfg_phy_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_pdb: 0x" << int_var__phyctrl_pdb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_dr_ty: 0x" << int_var__phyctrl_dr_ty << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_retrim: 0x" << int_var__phyctrl_retrim << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_en_rtrim: 0x" << int_var__phyctrl_en_rtrim << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_dll_trm_icp: 0x" << int_var__phyctrl_dll_trm_icp << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_endll: 0x" << int_var__phyctrl_endll << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_dlliff: 0x" << int_var__phyctrl_dlliff << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_reten: 0x" << int_var__phyctrl_reten << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_retenb: 0x" << int_var__phyctrl_retenb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_oden_strb: 0x" << int_var__phyctrl_oden_strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_oden_cmd: 0x" << int_var__phyctrl_oden_cmd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_oden_dat: 0x" << int_var__phyctrl_oden_dat << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_ren_strb: 0x" << int_var__phyctrl_ren_strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_ren_cmd: 0x" << int_var__phyctrl_ren_cmd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_ren_dat: 0x" << int_var__phyctrl_ren_dat << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_pu_strb: 0x" << int_var__phyctrl_pu_strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_pu_cmd: 0x" << int_var__phyctrl_pu_cmd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_pu_dat: 0x" << int_var__phyctrl_pu_dat << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_OD_release_strb: 0x" << int_var__phyctrl_OD_release_strb << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_OD_release_cmd: 0x" << int_var__phyctrl_OD_release_cmd << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_OD_release_dat: 0x" << int_var__phyctrl_OD_release_dat << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_itapdlysel: 0x" << int_var__phyctrl_itapdlysel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_itapdlyena: 0x" << int_var__phyctrl_itapdlyena << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_itapchgwin: 0x" << int_var__phyctrl_itapchgwin << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_otapdlysel: 0x" << int_var__phyctrl_otapdlysel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_otapdlyena: 0x" << int_var__phyctrl_otapdlyena << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_frqsel: 0x" << int_var__phyctrl_frqsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_strbsel: 0x" << int_var__phyctrl_strbsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_seldlyrxclk: 0x" << int_var__phyctrl_seldlyrxclk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_seldlytxclk: 0x" << int_var__phyctrl_seldlytxclk << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_testctrl: 0x" << int_var__phyctrl_testctrl << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_clkbufsel: 0x" << int_var__phyctrl_clkbufsel << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_bistenable: 0x" << int_var__phyctrl_bistenable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_bistmode: 0x" << int_var__phyctrl_bistmode << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".phyctrl_biststart: 0x" << int_var__phyctrl_biststart << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_emm_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    em_cfg_phy.show();
    em_sta_phy.show();
    em_cfg_core.show();
    cfg_ecc.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_emm_csr_cfg_ecc_t::get_width() const {
    return cap_emm_csr_cfg_ecc_t::s_get_width();

}

int cap_emm_csr_em_cfg_core_t::get_width() const {
    return cap_emm_csr_em_cfg_core_t::s_get_width();

}

int cap_emm_csr_em_sta_phy_t::get_width() const {
    return cap_emm_csr_em_sta_phy_t::s_get_width();

}

int cap_emm_csr_em_cfg_phy_t::get_width() const {
    return cap_emm_csr_em_cfg_phy_t::s_get_width();

}

int cap_emm_csr_t::get_width() const {
    return cap_emm_csr_t::s_get_width();

}

int cap_emm_csr_cfg_ecc_t::s_get_width() {
    int _count = 0;

    _count += 1; // disable_cor_SRAM1
    return _count;
}

int cap_emm_csr_em_cfg_core_t::s_get_width() {
    int _count = 0;

    _count += 1; // corecfg_asyncwkupena
    _count += 6; // corecfg_tuningcount
    return _count;
}

int cap_emm_csr_em_sta_phy_t::s_get_width() {
    int _count = 0;

    _count += 32; // phyctrl_biststatus
    _count += 1; // phyctrl_bistdone
    _count += 1; // phyctrl_exr_ninst
    _count += 1; // phyctrl_dllrdy
    _count += 4; // phyctrl_rtrim
    _count += 1; // phyctrl_caldone
    return _count;
}

int cap_emm_csr_em_cfg_phy_t::s_get_width() {
    int _count = 0;

    _count += 1; // phyctrl_pdb
    _count += 3; // phyctrl_dr_ty
    _count += 1; // phyctrl_retrim
    _count += 1; // phyctrl_en_rtrim
    _count += 4; // phyctrl_dll_trm_icp
    _count += 1; // phyctrl_endll
    _count += 3; // phyctrl_dlliff
    _count += 1; // phyctrl_reten
    _count += 1; // phyctrl_retenb
    _count += 1; // phyctrl_oden_strb
    _count += 1; // phyctrl_oden_cmd
    _count += 8; // phyctrl_oden_dat
    _count += 1; // phyctrl_ren_strb
    _count += 1; // phyctrl_ren_cmd
    _count += 8; // phyctrl_ren_dat
    _count += 1; // phyctrl_pu_strb
    _count += 1; // phyctrl_pu_cmd
    _count += 8; // phyctrl_pu_dat
    _count += 1; // phyctrl_OD_release_strb
    _count += 1; // phyctrl_OD_release_cmd
    _count += 8; // phyctrl_OD_release_dat
    _count += 5; // phyctrl_itapdlysel
    _count += 1; // phyctrl_itapdlyena
    _count += 1; // phyctrl_itapchgwin
    _count += 4; // phyctrl_otapdlysel
    _count += 1; // phyctrl_otapdlyena
    _count += 3; // phyctrl_frqsel
    _count += 4; // phyctrl_strbsel
    _count += 1; // phyctrl_seldlyrxclk
    _count += 1; // phyctrl_seldlytxclk
    _count += 8; // phyctrl_testctrl
    _count += 3; // phyctrl_clkbufsel
    _count += 1; // phyctrl_bistenable
    _count += 4; // phyctrl_bistmode
    _count += 1; // phyctrl_biststart
    return _count;
}

int cap_emm_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_emm_csr_em_cfg_phy_t::s_get_width(); // em_cfg_phy
    _count += cap_emm_csr_em_sta_phy_t::s_get_width(); // em_sta_phy
    _count += cap_emm_csr_em_cfg_core_t::s_get_width(); // em_cfg_core
    _count += cap_emm_csr_cfg_ecc_t::s_get_width(); // cfg_ecc
    return _count;
}

void cap_emm_csr_cfg_ecc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__disable_cor_SRAM1 = _val.convert_to< disable_cor_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_emm_csr_em_cfg_core_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__corecfg_asyncwkupena = _val.convert_to< corecfg_asyncwkupena_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_tuningcount = _val.convert_to< corecfg_tuningcount_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_emm_csr_em_sta_phy_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__phyctrl_biststatus = _val.convert_to< phyctrl_biststatus_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__phyctrl_bistdone = _val.convert_to< phyctrl_bistdone_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_exr_ninst = _val.convert_to< phyctrl_exr_ninst_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_dllrdy = _val.convert_to< phyctrl_dllrdy_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_rtrim = _val.convert_to< phyctrl_rtrim_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phyctrl_caldone = _val.convert_to< phyctrl_caldone_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_emm_csr_em_cfg_phy_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__phyctrl_pdb = _val.convert_to< phyctrl_pdb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_dr_ty = _val.convert_to< phyctrl_dr_ty_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phyctrl_retrim = _val.convert_to< phyctrl_retrim_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_en_rtrim = _val.convert_to< phyctrl_en_rtrim_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_dll_trm_icp = _val.convert_to< phyctrl_dll_trm_icp_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phyctrl_endll = _val.convert_to< phyctrl_endll_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_dlliff = _val.convert_to< phyctrl_dlliff_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phyctrl_reten = _val.convert_to< phyctrl_reten_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_retenb = _val.convert_to< phyctrl_retenb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_oden_strb = _val.convert_to< phyctrl_oden_strb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_oden_cmd = _val.convert_to< phyctrl_oden_cmd_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_oden_dat = _val.convert_to< phyctrl_oden_dat_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__phyctrl_ren_strb = _val.convert_to< phyctrl_ren_strb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_ren_cmd = _val.convert_to< phyctrl_ren_cmd_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_ren_dat = _val.convert_to< phyctrl_ren_dat_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__phyctrl_pu_strb = _val.convert_to< phyctrl_pu_strb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_pu_cmd = _val.convert_to< phyctrl_pu_cmd_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_pu_dat = _val.convert_to< phyctrl_pu_dat_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__phyctrl_OD_release_strb = _val.convert_to< phyctrl_OD_release_strb_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_OD_release_cmd = _val.convert_to< phyctrl_OD_release_cmd_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_OD_release_dat = _val.convert_to< phyctrl_OD_release_dat_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__phyctrl_itapdlysel = _val.convert_to< phyctrl_itapdlysel_cpp_int_t >()  ;
    _val = _val >> 5;
    
    int_var__phyctrl_itapdlyena = _val.convert_to< phyctrl_itapdlyena_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_itapchgwin = _val.convert_to< phyctrl_itapchgwin_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_otapdlysel = _val.convert_to< phyctrl_otapdlysel_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phyctrl_otapdlyena = _val.convert_to< phyctrl_otapdlyena_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_frqsel = _val.convert_to< phyctrl_frqsel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phyctrl_strbsel = _val.convert_to< phyctrl_strbsel_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phyctrl_seldlyrxclk = _val.convert_to< phyctrl_seldlyrxclk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_seldlytxclk = _val.convert_to< phyctrl_seldlytxclk_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_testctrl = _val.convert_to< phyctrl_testctrl_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__phyctrl_clkbufsel = _val.convert_to< phyctrl_clkbufsel_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__phyctrl_bistenable = _val.convert_to< phyctrl_bistenable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__phyctrl_bistmode = _val.convert_to< phyctrl_bistmode_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__phyctrl_biststart = _val.convert_to< phyctrl_biststart_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_emm_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    em_cfg_phy.all( _val);
    _val = _val >> em_cfg_phy.get_width(); 
    em_sta_phy.all( _val);
    _val = _val >> em_sta_phy.get_width(); 
    em_cfg_core.all( _val);
    _val = _val >> em_cfg_core.get_width(); 
    cfg_ecc.all( _val);
    _val = _val >> cfg_ecc.get_width(); 
}

cpp_int cap_emm_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << cfg_ecc.get_width(); ret_val = ret_val  | cfg_ecc.all(); 
    ret_val = ret_val << em_cfg_core.get_width(); ret_val = ret_val  | em_cfg_core.all(); 
    ret_val = ret_val << em_sta_phy.get_width(); ret_val = ret_val  | em_sta_phy.all(); 
    ret_val = ret_val << em_cfg_phy.get_width(); ret_val = ret_val  | em_cfg_phy.all(); 
    return ret_val;
}

cpp_int cap_emm_csr_em_cfg_phy_t::all() const {
    cpp_int ret_val;

    // phyctrl_biststart
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_biststart; 
    
    // phyctrl_bistmode
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phyctrl_bistmode; 
    
    // phyctrl_bistenable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_bistenable; 
    
    // phyctrl_clkbufsel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phyctrl_clkbufsel; 
    
    // phyctrl_testctrl
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phyctrl_testctrl; 
    
    // phyctrl_seldlytxclk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_seldlytxclk; 
    
    // phyctrl_seldlyrxclk
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_seldlyrxclk; 
    
    // phyctrl_strbsel
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phyctrl_strbsel; 
    
    // phyctrl_frqsel
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phyctrl_frqsel; 
    
    // phyctrl_otapdlyena
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_otapdlyena; 
    
    // phyctrl_otapdlysel
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phyctrl_otapdlysel; 
    
    // phyctrl_itapchgwin
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_itapchgwin; 
    
    // phyctrl_itapdlyena
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_itapdlyena; 
    
    // phyctrl_itapdlysel
    ret_val = ret_val << 5; ret_val = ret_val  | int_var__phyctrl_itapdlysel; 
    
    // phyctrl_OD_release_dat
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phyctrl_OD_release_dat; 
    
    // phyctrl_OD_release_cmd
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_OD_release_cmd; 
    
    // phyctrl_OD_release_strb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_OD_release_strb; 
    
    // phyctrl_pu_dat
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phyctrl_pu_dat; 
    
    // phyctrl_pu_cmd
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_pu_cmd; 
    
    // phyctrl_pu_strb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_pu_strb; 
    
    // phyctrl_ren_dat
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phyctrl_ren_dat; 
    
    // phyctrl_ren_cmd
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_ren_cmd; 
    
    // phyctrl_ren_strb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_ren_strb; 
    
    // phyctrl_oden_dat
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__phyctrl_oden_dat; 
    
    // phyctrl_oden_cmd
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_oden_cmd; 
    
    // phyctrl_oden_strb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_oden_strb; 
    
    // phyctrl_retenb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_retenb; 
    
    // phyctrl_reten
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_reten; 
    
    // phyctrl_dlliff
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phyctrl_dlliff; 
    
    // phyctrl_endll
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_endll; 
    
    // phyctrl_dll_trm_icp
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phyctrl_dll_trm_icp; 
    
    // phyctrl_en_rtrim
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_en_rtrim; 
    
    // phyctrl_retrim
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_retrim; 
    
    // phyctrl_dr_ty
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__phyctrl_dr_ty; 
    
    // phyctrl_pdb
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_pdb; 
    
    return ret_val;
}

cpp_int cap_emm_csr_em_sta_phy_t::all() const {
    cpp_int ret_val;

    // phyctrl_caldone
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_caldone; 
    
    // phyctrl_rtrim
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__phyctrl_rtrim; 
    
    // phyctrl_dllrdy
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_dllrdy; 
    
    // phyctrl_exr_ninst
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_exr_ninst; 
    
    // phyctrl_bistdone
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__phyctrl_bistdone; 
    
    // phyctrl_biststatus
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__phyctrl_biststatus; 
    
    return ret_val;
}

cpp_int cap_emm_csr_em_cfg_core_t::all() const {
    cpp_int ret_val;

    // corecfg_tuningcount
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__corecfg_tuningcount; 
    
    // corecfg_asyncwkupena
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_asyncwkupena; 
    
    return ret_val;
}

cpp_int cap_emm_csr_cfg_ecc_t::all() const {
    cpp_int ret_val;

    // disable_cor_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__disable_cor_SRAM1; 
    
    return ret_val;
}

void cap_emm_csr_cfg_ecc_t::clear() {

    int_var__disable_cor_SRAM1 = 0; 
    
}

void cap_emm_csr_em_cfg_core_t::clear() {

    int_var__corecfg_asyncwkupena = 0; 
    
    int_var__corecfg_tuningcount = 0; 
    
}

void cap_emm_csr_em_sta_phy_t::clear() {

    int_var__phyctrl_biststatus = 0; 
    
    int_var__phyctrl_bistdone = 0; 
    
    int_var__phyctrl_exr_ninst = 0; 
    
    int_var__phyctrl_dllrdy = 0; 
    
    int_var__phyctrl_rtrim = 0; 
    
    int_var__phyctrl_caldone = 0; 
    
}

void cap_emm_csr_em_cfg_phy_t::clear() {

    int_var__phyctrl_pdb = 0; 
    
    int_var__phyctrl_dr_ty = 0; 
    
    int_var__phyctrl_retrim = 0; 
    
    int_var__phyctrl_en_rtrim = 0; 
    
    int_var__phyctrl_dll_trm_icp = 0; 
    
    int_var__phyctrl_endll = 0; 
    
    int_var__phyctrl_dlliff = 0; 
    
    int_var__phyctrl_reten = 0; 
    
    int_var__phyctrl_retenb = 0; 
    
    int_var__phyctrl_oden_strb = 0; 
    
    int_var__phyctrl_oden_cmd = 0; 
    
    int_var__phyctrl_oden_dat = 0; 
    
    int_var__phyctrl_ren_strb = 0; 
    
    int_var__phyctrl_ren_cmd = 0; 
    
    int_var__phyctrl_ren_dat = 0; 
    
    int_var__phyctrl_pu_strb = 0; 
    
    int_var__phyctrl_pu_cmd = 0; 
    
    int_var__phyctrl_pu_dat = 0; 
    
    int_var__phyctrl_OD_release_strb = 0; 
    
    int_var__phyctrl_OD_release_cmd = 0; 
    
    int_var__phyctrl_OD_release_dat = 0; 
    
    int_var__phyctrl_itapdlysel = 0; 
    
    int_var__phyctrl_itapdlyena = 0; 
    
    int_var__phyctrl_itapchgwin = 0; 
    
    int_var__phyctrl_otapdlysel = 0; 
    
    int_var__phyctrl_otapdlyena = 0; 
    
    int_var__phyctrl_frqsel = 0; 
    
    int_var__phyctrl_strbsel = 0; 
    
    int_var__phyctrl_seldlyrxclk = 0; 
    
    int_var__phyctrl_seldlytxclk = 0; 
    
    int_var__phyctrl_testctrl = 0; 
    
    int_var__phyctrl_clkbufsel = 0; 
    
    int_var__phyctrl_bistenable = 0; 
    
    int_var__phyctrl_bistmode = 0; 
    
    int_var__phyctrl_biststart = 0; 
    
}

void cap_emm_csr_t::clear() {

    em_cfg_phy.clear();
    em_sta_phy.clear();
    em_cfg_core.clear();
    cfg_ecc.clear();
}

void cap_emm_csr_cfg_ecc_t::init() {

}

void cap_emm_csr_em_cfg_core_t::init() {

}

void cap_emm_csr_em_sta_phy_t::init() {

}

void cap_emm_csr_em_cfg_phy_t::init() {

}

void cap_emm_csr_t::init() {

    em_cfg_phy.set_attributes(this,"em_cfg_phy", 0x0 );
    em_sta_phy.set_attributes(this,"em_sta_phy", 0x10 );
    em_cfg_core.set_attributes(this,"em_cfg_core", 0x18 );
    cfg_ecc.set_attributes(this,"cfg_ecc", 0x1c );
}

void cap_emm_csr_cfg_ecc_t::disable_cor_SRAM1(const cpp_int & _val) { 
    // disable_cor_SRAM1
    int_var__disable_cor_SRAM1 = _val.convert_to< disable_cor_SRAM1_cpp_int_t >();
}

cpp_int cap_emm_csr_cfg_ecc_t::disable_cor_SRAM1() const {
    return int_var__disable_cor_SRAM1;
}
    
void cap_emm_csr_em_cfg_core_t::corecfg_asyncwkupena(const cpp_int & _val) { 
    // corecfg_asyncwkupena
    int_var__corecfg_asyncwkupena = _val.convert_to< corecfg_asyncwkupena_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_core_t::corecfg_asyncwkupena() const {
    return int_var__corecfg_asyncwkupena;
}
    
void cap_emm_csr_em_cfg_core_t::corecfg_tuningcount(const cpp_int & _val) { 
    // corecfg_tuningcount
    int_var__corecfg_tuningcount = _val.convert_to< corecfg_tuningcount_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_core_t::corecfg_tuningcount() const {
    return int_var__corecfg_tuningcount;
}
    
void cap_emm_csr_em_sta_phy_t::phyctrl_biststatus(const cpp_int & _val) { 
    // phyctrl_biststatus
    int_var__phyctrl_biststatus = _val.convert_to< phyctrl_biststatus_cpp_int_t >();
}

cpp_int cap_emm_csr_em_sta_phy_t::phyctrl_biststatus() const {
    return int_var__phyctrl_biststatus;
}
    
void cap_emm_csr_em_sta_phy_t::phyctrl_bistdone(const cpp_int & _val) { 
    // phyctrl_bistdone
    int_var__phyctrl_bistdone = _val.convert_to< phyctrl_bistdone_cpp_int_t >();
}

cpp_int cap_emm_csr_em_sta_phy_t::phyctrl_bistdone() const {
    return int_var__phyctrl_bistdone;
}
    
void cap_emm_csr_em_sta_phy_t::phyctrl_exr_ninst(const cpp_int & _val) { 
    // phyctrl_exr_ninst
    int_var__phyctrl_exr_ninst = _val.convert_to< phyctrl_exr_ninst_cpp_int_t >();
}

cpp_int cap_emm_csr_em_sta_phy_t::phyctrl_exr_ninst() const {
    return int_var__phyctrl_exr_ninst;
}
    
void cap_emm_csr_em_sta_phy_t::phyctrl_dllrdy(const cpp_int & _val) { 
    // phyctrl_dllrdy
    int_var__phyctrl_dllrdy = _val.convert_to< phyctrl_dllrdy_cpp_int_t >();
}

cpp_int cap_emm_csr_em_sta_phy_t::phyctrl_dllrdy() const {
    return int_var__phyctrl_dllrdy;
}
    
void cap_emm_csr_em_sta_phy_t::phyctrl_rtrim(const cpp_int & _val) { 
    // phyctrl_rtrim
    int_var__phyctrl_rtrim = _val.convert_to< phyctrl_rtrim_cpp_int_t >();
}

cpp_int cap_emm_csr_em_sta_phy_t::phyctrl_rtrim() const {
    return int_var__phyctrl_rtrim;
}
    
void cap_emm_csr_em_sta_phy_t::phyctrl_caldone(const cpp_int & _val) { 
    // phyctrl_caldone
    int_var__phyctrl_caldone = _val.convert_to< phyctrl_caldone_cpp_int_t >();
}

cpp_int cap_emm_csr_em_sta_phy_t::phyctrl_caldone() const {
    return int_var__phyctrl_caldone;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_pdb(const cpp_int & _val) { 
    // phyctrl_pdb
    int_var__phyctrl_pdb = _val.convert_to< phyctrl_pdb_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_pdb() const {
    return int_var__phyctrl_pdb;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_dr_ty(const cpp_int & _val) { 
    // phyctrl_dr_ty
    int_var__phyctrl_dr_ty = _val.convert_to< phyctrl_dr_ty_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_dr_ty() const {
    return int_var__phyctrl_dr_ty;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_retrim(const cpp_int & _val) { 
    // phyctrl_retrim
    int_var__phyctrl_retrim = _val.convert_to< phyctrl_retrim_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_retrim() const {
    return int_var__phyctrl_retrim;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_en_rtrim(const cpp_int & _val) { 
    // phyctrl_en_rtrim
    int_var__phyctrl_en_rtrim = _val.convert_to< phyctrl_en_rtrim_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_en_rtrim() const {
    return int_var__phyctrl_en_rtrim;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_dll_trm_icp(const cpp_int & _val) { 
    // phyctrl_dll_trm_icp
    int_var__phyctrl_dll_trm_icp = _val.convert_to< phyctrl_dll_trm_icp_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_dll_trm_icp() const {
    return int_var__phyctrl_dll_trm_icp;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_endll(const cpp_int & _val) { 
    // phyctrl_endll
    int_var__phyctrl_endll = _val.convert_to< phyctrl_endll_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_endll() const {
    return int_var__phyctrl_endll;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_dlliff(const cpp_int & _val) { 
    // phyctrl_dlliff
    int_var__phyctrl_dlliff = _val.convert_to< phyctrl_dlliff_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_dlliff() const {
    return int_var__phyctrl_dlliff;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_reten(const cpp_int & _val) { 
    // phyctrl_reten
    int_var__phyctrl_reten = _val.convert_to< phyctrl_reten_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_reten() const {
    return int_var__phyctrl_reten;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_retenb(const cpp_int & _val) { 
    // phyctrl_retenb
    int_var__phyctrl_retenb = _val.convert_to< phyctrl_retenb_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_retenb() const {
    return int_var__phyctrl_retenb;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_oden_strb(const cpp_int & _val) { 
    // phyctrl_oden_strb
    int_var__phyctrl_oden_strb = _val.convert_to< phyctrl_oden_strb_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_oden_strb() const {
    return int_var__phyctrl_oden_strb;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_oden_cmd(const cpp_int & _val) { 
    // phyctrl_oden_cmd
    int_var__phyctrl_oden_cmd = _val.convert_to< phyctrl_oden_cmd_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_oden_cmd() const {
    return int_var__phyctrl_oden_cmd;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_oden_dat(const cpp_int & _val) { 
    // phyctrl_oden_dat
    int_var__phyctrl_oden_dat = _val.convert_to< phyctrl_oden_dat_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_oden_dat() const {
    return int_var__phyctrl_oden_dat;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_ren_strb(const cpp_int & _val) { 
    // phyctrl_ren_strb
    int_var__phyctrl_ren_strb = _val.convert_to< phyctrl_ren_strb_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_ren_strb() const {
    return int_var__phyctrl_ren_strb;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_ren_cmd(const cpp_int & _val) { 
    // phyctrl_ren_cmd
    int_var__phyctrl_ren_cmd = _val.convert_to< phyctrl_ren_cmd_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_ren_cmd() const {
    return int_var__phyctrl_ren_cmd;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_ren_dat(const cpp_int & _val) { 
    // phyctrl_ren_dat
    int_var__phyctrl_ren_dat = _val.convert_to< phyctrl_ren_dat_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_ren_dat() const {
    return int_var__phyctrl_ren_dat;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_pu_strb(const cpp_int & _val) { 
    // phyctrl_pu_strb
    int_var__phyctrl_pu_strb = _val.convert_to< phyctrl_pu_strb_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_pu_strb() const {
    return int_var__phyctrl_pu_strb;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_pu_cmd(const cpp_int & _val) { 
    // phyctrl_pu_cmd
    int_var__phyctrl_pu_cmd = _val.convert_to< phyctrl_pu_cmd_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_pu_cmd() const {
    return int_var__phyctrl_pu_cmd;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_pu_dat(const cpp_int & _val) { 
    // phyctrl_pu_dat
    int_var__phyctrl_pu_dat = _val.convert_to< phyctrl_pu_dat_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_pu_dat() const {
    return int_var__phyctrl_pu_dat;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_OD_release_strb(const cpp_int & _val) { 
    // phyctrl_OD_release_strb
    int_var__phyctrl_OD_release_strb = _val.convert_to< phyctrl_OD_release_strb_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_OD_release_strb() const {
    return int_var__phyctrl_OD_release_strb;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_OD_release_cmd(const cpp_int & _val) { 
    // phyctrl_OD_release_cmd
    int_var__phyctrl_OD_release_cmd = _val.convert_to< phyctrl_OD_release_cmd_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_OD_release_cmd() const {
    return int_var__phyctrl_OD_release_cmd;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_OD_release_dat(const cpp_int & _val) { 
    // phyctrl_OD_release_dat
    int_var__phyctrl_OD_release_dat = _val.convert_to< phyctrl_OD_release_dat_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_OD_release_dat() const {
    return int_var__phyctrl_OD_release_dat;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_itapdlysel(const cpp_int & _val) { 
    // phyctrl_itapdlysel
    int_var__phyctrl_itapdlysel = _val.convert_to< phyctrl_itapdlysel_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_itapdlysel() const {
    return int_var__phyctrl_itapdlysel;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_itapdlyena(const cpp_int & _val) { 
    // phyctrl_itapdlyena
    int_var__phyctrl_itapdlyena = _val.convert_to< phyctrl_itapdlyena_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_itapdlyena() const {
    return int_var__phyctrl_itapdlyena;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_itapchgwin(const cpp_int & _val) { 
    // phyctrl_itapchgwin
    int_var__phyctrl_itapchgwin = _val.convert_to< phyctrl_itapchgwin_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_itapchgwin() const {
    return int_var__phyctrl_itapchgwin;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_otapdlysel(const cpp_int & _val) { 
    // phyctrl_otapdlysel
    int_var__phyctrl_otapdlysel = _val.convert_to< phyctrl_otapdlysel_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_otapdlysel() const {
    return int_var__phyctrl_otapdlysel;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_otapdlyena(const cpp_int & _val) { 
    // phyctrl_otapdlyena
    int_var__phyctrl_otapdlyena = _val.convert_to< phyctrl_otapdlyena_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_otapdlyena() const {
    return int_var__phyctrl_otapdlyena;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_frqsel(const cpp_int & _val) { 
    // phyctrl_frqsel
    int_var__phyctrl_frqsel = _val.convert_to< phyctrl_frqsel_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_frqsel() const {
    return int_var__phyctrl_frqsel;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_strbsel(const cpp_int & _val) { 
    // phyctrl_strbsel
    int_var__phyctrl_strbsel = _val.convert_to< phyctrl_strbsel_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_strbsel() const {
    return int_var__phyctrl_strbsel;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_seldlyrxclk(const cpp_int & _val) { 
    // phyctrl_seldlyrxclk
    int_var__phyctrl_seldlyrxclk = _val.convert_to< phyctrl_seldlyrxclk_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_seldlyrxclk() const {
    return int_var__phyctrl_seldlyrxclk;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_seldlytxclk(const cpp_int & _val) { 
    // phyctrl_seldlytxclk
    int_var__phyctrl_seldlytxclk = _val.convert_to< phyctrl_seldlytxclk_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_seldlytxclk() const {
    return int_var__phyctrl_seldlytxclk;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_testctrl(const cpp_int & _val) { 
    // phyctrl_testctrl
    int_var__phyctrl_testctrl = _val.convert_to< phyctrl_testctrl_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_testctrl() const {
    return int_var__phyctrl_testctrl;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_clkbufsel(const cpp_int & _val) { 
    // phyctrl_clkbufsel
    int_var__phyctrl_clkbufsel = _val.convert_to< phyctrl_clkbufsel_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_clkbufsel() const {
    return int_var__phyctrl_clkbufsel;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_bistenable(const cpp_int & _val) { 
    // phyctrl_bistenable
    int_var__phyctrl_bistenable = _val.convert_to< phyctrl_bistenable_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_bistenable() const {
    return int_var__phyctrl_bistenable;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_bistmode(const cpp_int & _val) { 
    // phyctrl_bistmode
    int_var__phyctrl_bistmode = _val.convert_to< phyctrl_bistmode_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_bistmode() const {
    return int_var__phyctrl_bistmode;
}
    
void cap_emm_csr_em_cfg_phy_t::phyctrl_biststart(const cpp_int & _val) { 
    // phyctrl_biststart
    int_var__phyctrl_biststart = _val.convert_to< phyctrl_biststart_cpp_int_t >();
}

cpp_int cap_emm_csr_em_cfg_phy_t::phyctrl_biststart() const {
    return int_var__phyctrl_biststart;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_cfg_ecc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "disable_cor_SRAM1")) { field_val = disable_cor_SRAM1(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_em_cfg_core_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "corecfg_asyncwkupena")) { field_val = corecfg_asyncwkupena(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_tuningcount")) { field_val = corecfg_tuningcount(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_em_sta_phy_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phyctrl_biststatus")) { field_val = phyctrl_biststatus(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_bistdone")) { field_val = phyctrl_bistdone(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_exr_ninst")) { field_val = phyctrl_exr_ninst(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dllrdy")) { field_val = phyctrl_dllrdy(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_rtrim")) { field_val = phyctrl_rtrim(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_caldone")) { field_val = phyctrl_caldone(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_em_cfg_phy_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phyctrl_pdb")) { field_val = phyctrl_pdb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dr_ty")) { field_val = phyctrl_dr_ty(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_retrim")) { field_val = phyctrl_retrim(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_en_rtrim")) { field_val = phyctrl_en_rtrim(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dll_trm_icp")) { field_val = phyctrl_dll_trm_icp(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_endll")) { field_val = phyctrl_endll(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dlliff")) { field_val = phyctrl_dlliff(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_reten")) { field_val = phyctrl_reten(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_retenb")) { field_val = phyctrl_retenb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_oden_strb")) { field_val = phyctrl_oden_strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_oden_cmd")) { field_val = phyctrl_oden_cmd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_oden_dat")) { field_val = phyctrl_oden_dat(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_ren_strb")) { field_val = phyctrl_ren_strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_ren_cmd")) { field_val = phyctrl_ren_cmd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_ren_dat")) { field_val = phyctrl_ren_dat(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_pu_strb")) { field_val = phyctrl_pu_strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_pu_cmd")) { field_val = phyctrl_pu_cmd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_pu_dat")) { field_val = phyctrl_pu_dat(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_OD_release_strb")) { field_val = phyctrl_OD_release_strb(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_OD_release_cmd")) { field_val = phyctrl_OD_release_cmd(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_OD_release_dat")) { field_val = phyctrl_OD_release_dat(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_itapdlysel")) { field_val = phyctrl_itapdlysel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_itapdlyena")) { field_val = phyctrl_itapdlyena(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_itapchgwin")) { field_val = phyctrl_itapchgwin(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_otapdlysel")) { field_val = phyctrl_otapdlysel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_otapdlyena")) { field_val = phyctrl_otapdlyena(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_frqsel")) { field_val = phyctrl_frqsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_strbsel")) { field_val = phyctrl_strbsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_seldlyrxclk")) { field_val = phyctrl_seldlyrxclk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_seldlytxclk")) { field_val = phyctrl_seldlytxclk(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_testctrl")) { field_val = phyctrl_testctrl(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_clkbufsel")) { field_val = phyctrl_clkbufsel(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_bistenable")) { field_val = phyctrl_bistenable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_bistmode")) { field_val = phyctrl_bistmode(); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_biststart")) { field_val = phyctrl_biststart(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = em_cfg_phy.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_sta_phy.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_cfg_core.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ecc.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_cfg_ecc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "disable_cor_SRAM1")) { disable_cor_SRAM1(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_em_cfg_core_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "corecfg_asyncwkupena")) { corecfg_asyncwkupena(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_tuningcount")) { corecfg_tuningcount(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_em_sta_phy_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phyctrl_biststatus")) { phyctrl_biststatus(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_bistdone")) { phyctrl_bistdone(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_exr_ninst")) { phyctrl_exr_ninst(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dllrdy")) { phyctrl_dllrdy(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_rtrim")) { phyctrl_rtrim(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_caldone")) { phyctrl_caldone(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_em_cfg_phy_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "phyctrl_pdb")) { phyctrl_pdb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dr_ty")) { phyctrl_dr_ty(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_retrim")) { phyctrl_retrim(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_en_rtrim")) { phyctrl_en_rtrim(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dll_trm_icp")) { phyctrl_dll_trm_icp(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_endll")) { phyctrl_endll(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_dlliff")) { phyctrl_dlliff(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_reten")) { phyctrl_reten(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_retenb")) { phyctrl_retenb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_oden_strb")) { phyctrl_oden_strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_oden_cmd")) { phyctrl_oden_cmd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_oden_dat")) { phyctrl_oden_dat(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_ren_strb")) { phyctrl_ren_strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_ren_cmd")) { phyctrl_ren_cmd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_ren_dat")) { phyctrl_ren_dat(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_pu_strb")) { phyctrl_pu_strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_pu_cmd")) { phyctrl_pu_cmd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_pu_dat")) { phyctrl_pu_dat(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_OD_release_strb")) { phyctrl_OD_release_strb(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_OD_release_cmd")) { phyctrl_OD_release_cmd(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_OD_release_dat")) { phyctrl_OD_release_dat(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_itapdlysel")) { phyctrl_itapdlysel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_itapdlyena")) { phyctrl_itapdlyena(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_itapchgwin")) { phyctrl_itapchgwin(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_otapdlysel")) { phyctrl_otapdlysel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_otapdlyena")) { phyctrl_otapdlyena(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_frqsel")) { phyctrl_frqsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_strbsel")) { phyctrl_strbsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_seldlyrxclk")) { phyctrl_seldlyrxclk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_seldlytxclk")) { phyctrl_seldlytxclk(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_testctrl")) { phyctrl_testctrl(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_clkbufsel")) { phyctrl_clkbufsel(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_bistenable")) { phyctrl_bistenable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_bistmode")) { phyctrl_bistmode(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "phyctrl_biststart")) { phyctrl_biststart(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_emm_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = em_cfg_phy.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_sta_phy.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_cfg_core.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ecc.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_emm_csr_cfg_ecc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("disable_cor_SRAM1");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_emm_csr_em_cfg_core_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("corecfg_asyncwkupena");
    ret_vec.push_back("corecfg_tuningcount");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_emm_csr_em_sta_phy_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("phyctrl_biststatus");
    ret_vec.push_back("phyctrl_bistdone");
    ret_vec.push_back("phyctrl_exr_ninst");
    ret_vec.push_back("phyctrl_dllrdy");
    ret_vec.push_back("phyctrl_rtrim");
    ret_vec.push_back("phyctrl_caldone");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_emm_csr_em_cfg_phy_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("phyctrl_pdb");
    ret_vec.push_back("phyctrl_dr_ty");
    ret_vec.push_back("phyctrl_retrim");
    ret_vec.push_back("phyctrl_en_rtrim");
    ret_vec.push_back("phyctrl_dll_trm_icp");
    ret_vec.push_back("phyctrl_endll");
    ret_vec.push_back("phyctrl_dlliff");
    ret_vec.push_back("phyctrl_reten");
    ret_vec.push_back("phyctrl_retenb");
    ret_vec.push_back("phyctrl_oden_strb");
    ret_vec.push_back("phyctrl_oden_cmd");
    ret_vec.push_back("phyctrl_oden_dat");
    ret_vec.push_back("phyctrl_ren_strb");
    ret_vec.push_back("phyctrl_ren_cmd");
    ret_vec.push_back("phyctrl_ren_dat");
    ret_vec.push_back("phyctrl_pu_strb");
    ret_vec.push_back("phyctrl_pu_cmd");
    ret_vec.push_back("phyctrl_pu_dat");
    ret_vec.push_back("phyctrl_OD_release_strb");
    ret_vec.push_back("phyctrl_OD_release_cmd");
    ret_vec.push_back("phyctrl_OD_release_dat");
    ret_vec.push_back("phyctrl_itapdlysel");
    ret_vec.push_back("phyctrl_itapdlyena");
    ret_vec.push_back("phyctrl_itapchgwin");
    ret_vec.push_back("phyctrl_otapdlysel");
    ret_vec.push_back("phyctrl_otapdlyena");
    ret_vec.push_back("phyctrl_frqsel");
    ret_vec.push_back("phyctrl_strbsel");
    ret_vec.push_back("phyctrl_seldlyrxclk");
    ret_vec.push_back("phyctrl_seldlytxclk");
    ret_vec.push_back("phyctrl_testctrl");
    ret_vec.push_back("phyctrl_clkbufsel");
    ret_vec.push_back("phyctrl_bistenable");
    ret_vec.push_back("phyctrl_bistmode");
    ret_vec.push_back("phyctrl_biststart");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_emm_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : em_cfg_phy.get_fields(level-1)) {
            ret_vec.push_back("em_cfg_phy." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : em_sta_phy.get_fields(level-1)) {
            ret_vec.push_back("em_sta_phy." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : em_cfg_core.get_fields(level-1)) {
            ret_vec.push_back("em_cfg_core." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_ecc.get_fields(level-1)) {
            ret_vec.push_back("cfg_ecc." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
