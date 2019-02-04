
#include "cap_em_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_em_csr_axi_int_enable_clear_t::cap_em_csr_axi_int_enable_clear_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_axi_int_enable_clear_t::~cap_em_csr_axi_int_enable_clear_t() { }

cap_em_csr_intreg_t::cap_em_csr_intreg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_intreg_t::~cap_em_csr_intreg_t() { }

cap_em_csr_intgrp_t::cap_em_csr_intgrp_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_intgrp_t::~cap_em_csr_intgrp_t() { }

cap_em_csr_intreg_status_t::cap_em_csr_intreg_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_intreg_status_t::~cap_em_csr_intreg_status_t() { }

cap_em_csr_int_groups_int_enable_rw_reg_t::cap_em_csr_int_groups_int_enable_rw_reg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_int_groups_int_enable_rw_reg_t::~cap_em_csr_int_groups_int_enable_rw_reg_t() { }

cap_em_csr_intgrp_status_t::cap_em_csr_intgrp_status_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_intgrp_status_t::~cap_em_csr_intgrp_status_t() { }

cap_em_csr_csr_intr_t::cap_em_csr_csr_intr_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_csr_intr_t::~cap_em_csr_csr_intr_t() { }

cap_em_csr_em_cfg_core_t::cap_em_csr_em_cfg_core_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_em_cfg_core_t::~cap_em_csr_em_cfg_core_t() { }

cap_em_csr_em_cfg_t::cap_em_csr_em_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_em_cfg_t::~cap_em_csr_em_cfg_t() { }

cap_em_csr_cfg_ecc_t::cap_em_csr_cfg_ecc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_cfg_ecc_t::~cap_em_csr_cfg_ecc_t() { }

cap_em_csr_sta_ecc_t::cap_em_csr_sta_ecc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_sta_ecc_t::~cap_em_csr_sta_ecc_t() { }

cap_em_csr_sta_bist_t::cap_em_csr_sta_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_sta_bist_t::~cap_em_csr_sta_bist_t() { }

cap_em_csr_cfg_bist_t::cap_em_csr_cfg_bist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_cfg_bist_t::~cap_em_csr_cfg_bist_t() { }

cap_em_csr_base_t::cap_em_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_em_csr_base_t::~cap_em_csr_base_t() { }

cap_em_csr_t::cap_em_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(256);
        set_attributes(0,get_name(), 0);
        }
cap_em_csr_t::~cap_em_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_axi_int_enable_clear_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_intr_enable: 0x" << int_var__axi_intr_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_wkup_enable: 0x" << int_var__axi_wkup_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram1_ecc_correctable_enable: 0x" << int_var__sram1_ecc_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram1_ecc_uncorrectable_enable: 0x" << int_var__sram1_ecc_uncorrectable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram2_ecc_correctable_enable: 0x" << int_var__sram2_ecc_correctable_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram2_ecc_uncorrectable_enable: 0x" << int_var__sram2_ecc_uncorrectable_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_intreg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_intr_interrupt: 0x" << int_var__axi_intr_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".axi_wkup_interrupt: 0x" << int_var__axi_wkup_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram1_ecc_correctable_interrupt: 0x" << int_var__sram1_ecc_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram1_ecc_uncorrectable_interrupt: 0x" << int_var__sram1_ecc_uncorrectable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram2_ecc_correctable_interrupt: 0x" << int_var__sram2_ecc_correctable_interrupt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sram2_ecc_uncorrectable_interrupt: 0x" << int_var__sram2_ecc_uncorrectable_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_intgrp_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_test_set.show();
    int_enable_set.show();
    int_enable_clear.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_intreg_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_interrupt: 0x" << int_var__axi_interrupt << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_int_groups_int_enable_rw_reg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".axi_enable: 0x" << int_var__axi_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_intgrp_status_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    intreg.show();
    int_enable_rw_reg.show();
    int_rw_reg.show();
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_csr_intr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream: 0x" << int_var__dowstream << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".dowstream_enable: 0x" << int_var__dowstream_enable << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_em_cfg_core_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_baseclkfreq: 0x" << int_var__corecfg_baseclkfreq << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_timeoutclkfreq: 0x" << int_var__corecfg_timeoutclkfreq << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_maxblklength: 0x" << int_var__corecfg_maxblklength << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_8bitsupport: 0x" << int_var__corecfg_8bitsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_adma2support: 0x" << int_var__corecfg_adma2support << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_highspeedsupport: 0x" << int_var__corecfg_highspeedsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdmasupport: 0x" << int_var__corecfg_sdmasupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_suspressupport: 0x" << int_var__corecfg_suspressupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_3p3voltsupport: 0x" << int_var__corecfg_3p3voltsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_3p0voltsupport: 0x" << int_var__corecfg_3p0voltsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_1p8voltsupport: 0x" << int_var__corecfg_1p8voltsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_64bitsupport: 0x" << int_var__corecfg_64bitsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_asynchintrsupport: 0x" << int_var__corecfg_asynchintrsupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_slottype: 0x" << int_var__corecfg_slottype << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdr50support: 0x" << int_var__corecfg_sdr50support << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdr104support: 0x" << int_var__corecfg_sdr104support << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_ddr50support: 0x" << int_var__corecfg_ddr50support << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_hs400support: 0x" << int_var__corecfg_hs400support << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_adriversupport: 0x" << int_var__corecfg_adriversupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_cdriversupport: 0x" << int_var__corecfg_cdriversupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_ddriversupport: 0x" << int_var__corecfg_ddriversupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_type4support: 0x" << int_var__corecfg_type4support << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_retuningtimercnt: 0x" << int_var__corecfg_retuningtimercnt << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_tuningforsdr50: 0x" << int_var__corecfg_tuningforsdr50 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_retuningmodes: 0x" << int_var__corecfg_retuningmodes << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_clockmultiplier: 0x" << int_var__corecfg_clockmultiplier << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_spisupport: 0x" << int_var__corecfg_spisupport << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_spiblkmode: 0x" << int_var__corecfg_spiblkmode << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_initpresetval: 0x" << int_var__corecfg_initpresetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_dspdpresetval: 0x" << int_var__corecfg_dspdpresetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_hspdpresetval: 0x" << int_var__corecfg_hspdpresetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdr12presetval: 0x" << int_var__corecfg_sdr12presetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdr25presetval: 0x" << int_var__corecfg_sdr25presetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdr50presetval: 0x" << int_var__corecfg_sdr50presetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_sdr104presetval: 0x" << int_var__corecfg_sdr104presetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_ddr50presetval: 0x" << int_var__corecfg_ddr50presetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_hs400presetval: 0x" << int_var__corecfg_hs400presetval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_maxcurrent1p8v: 0x" << int_var__corecfg_maxcurrent1p8v << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_maxcurrent3p0v: 0x" << int_var__corecfg_maxcurrent3p0v << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_maxcurrent3p3v: 0x" << int_var__corecfg_maxcurrent3p3v << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_cqfval: 0x" << int_var__corecfg_cqfval << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_cqfmul: 0x" << int_var__corecfg_cqfmul << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".corecfg_timeoutclkunit: 0x" << int_var__corecfg_timeoutclkunit << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_em_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".debug_port_enable: 0x" << int_var__debug_port_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_port_select: 0x" << int_var__debug_port_select << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mstid: 0x" << int_var__mstid << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_cfg_ecc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".disable_det_SRAM1: 0x" << int_var__disable_det_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".disable_cor_SRAM2: 0x" << int_var__disable_cor_SRAM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".disable_det_SRAM2: 0x" << int_var__disable_det_SRAM2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_sta_ecc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".correctable_SRAM1: 0x" << int_var__correctable_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable_SRAM1: 0x" << int_var__uncorrectable_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".correctable_SRAM2: 0x" << int_var__correctable_SRAM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".uncorrectable_SRAM2: 0x" << int_var__uncorrectable_SRAM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_SRAM1: 0x" << int_var__addr_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_SRAM2: 0x" << int_var__addr_SRAM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome_SRAM1: 0x" << int_var__syndrome_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".syndrome_SRAM2: 0x" << int_var__syndrome_SRAM2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_sta_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".done_fail_SRAM1: 0x" << int_var__done_fail_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_fail_SRAM2: 0x" << int_var__done_fail_SRAM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_pass_SRAM1: 0x" << int_var__done_pass_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".done_pass_SRAM2: 0x" << int_var__done_pass_SRAM2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_cfg_bist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".run_SRAM1: 0x" << int_var__run_SRAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".run_SRAM2: 0x" << int_var__run_SRAM2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_em_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    cfg_bist.show();
    sta_bist.show();
    sta_ecc.show();
    cfg_ecc.show();
    em_cfg.show();
    em_cfg_core.show();
    csr_intr.show();
    emm.show();
    int_groups.show();
    axi.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_em_csr_axi_int_enable_clear_t::get_width() const {
    return cap_em_csr_axi_int_enable_clear_t::s_get_width();

}

int cap_em_csr_intreg_t::get_width() const {
    return cap_em_csr_intreg_t::s_get_width();

}

int cap_em_csr_intgrp_t::get_width() const {
    return cap_em_csr_intgrp_t::s_get_width();

}

int cap_em_csr_intreg_status_t::get_width() const {
    return cap_em_csr_intreg_status_t::s_get_width();

}

int cap_em_csr_int_groups_int_enable_rw_reg_t::get_width() const {
    return cap_em_csr_int_groups_int_enable_rw_reg_t::s_get_width();

}

int cap_em_csr_intgrp_status_t::get_width() const {
    return cap_em_csr_intgrp_status_t::s_get_width();

}

int cap_em_csr_csr_intr_t::get_width() const {
    return cap_em_csr_csr_intr_t::s_get_width();

}

int cap_em_csr_em_cfg_core_t::get_width() const {
    return cap_em_csr_em_cfg_core_t::s_get_width();

}

int cap_em_csr_em_cfg_t::get_width() const {
    return cap_em_csr_em_cfg_t::s_get_width();

}

int cap_em_csr_cfg_ecc_t::get_width() const {
    return cap_em_csr_cfg_ecc_t::s_get_width();

}

int cap_em_csr_sta_ecc_t::get_width() const {
    return cap_em_csr_sta_ecc_t::s_get_width();

}

int cap_em_csr_sta_bist_t::get_width() const {
    return cap_em_csr_sta_bist_t::s_get_width();

}

int cap_em_csr_cfg_bist_t::get_width() const {
    return cap_em_csr_cfg_bist_t::s_get_width();

}

int cap_em_csr_base_t::get_width() const {
    return cap_em_csr_base_t::s_get_width();

}

int cap_em_csr_t::get_width() const {
    return cap_em_csr_t::s_get_width();

}

int cap_em_csr_axi_int_enable_clear_t::s_get_width() {
    int _count = 0;

    _count += 1; // axi_intr_enable
    _count += 1; // axi_wkup_enable
    _count += 1; // sram1_ecc_correctable_enable
    _count += 1; // sram1_ecc_uncorrectable_enable
    _count += 1; // sram2_ecc_correctable_enable
    _count += 1; // sram2_ecc_uncorrectable_enable
    return _count;
}

int cap_em_csr_intreg_t::s_get_width() {
    int _count = 0;

    _count += 1; // axi_intr_interrupt
    _count += 1; // axi_wkup_interrupt
    _count += 1; // sram1_ecc_correctable_interrupt
    _count += 1; // sram1_ecc_uncorrectable_interrupt
    _count += 1; // sram2_ecc_correctable_interrupt
    _count += 1; // sram2_ecc_uncorrectable_interrupt
    return _count;
}

int cap_em_csr_intgrp_t::s_get_width() {
    int _count = 0;

    _count += cap_em_csr_intreg_t::s_get_width(); // intreg
    _count += cap_em_csr_intreg_t::s_get_width(); // int_test_set
    _count += cap_em_csr_axi_int_enable_clear_t::s_get_width(); // int_enable_set
    _count += cap_em_csr_axi_int_enable_clear_t::s_get_width(); // int_enable_clear
    return _count;
}

int cap_em_csr_intreg_status_t::s_get_width() {
    int _count = 0;

    _count += 1; // axi_interrupt
    return _count;
}

int cap_em_csr_int_groups_int_enable_rw_reg_t::s_get_width() {
    int _count = 0;

    _count += 1; // axi_enable
    return _count;
}

int cap_em_csr_intgrp_status_t::s_get_width() {
    int _count = 0;

    _count += cap_em_csr_intreg_status_t::s_get_width(); // intreg
    _count += cap_em_csr_int_groups_int_enable_rw_reg_t::s_get_width(); // int_enable_rw_reg
    _count += cap_em_csr_intreg_status_t::s_get_width(); // int_rw_reg
    return _count;
}

int cap_em_csr_csr_intr_t::s_get_width() {
    int _count = 0;

    _count += 1; // dowstream
    _count += 1; // dowstream_enable
    return _count;
}

int cap_em_csr_em_cfg_core_t::s_get_width() {
    int _count = 0;

    _count += 8; // corecfg_baseclkfreq
    _count += 6; // corecfg_timeoutclkfreq
    _count += 21; // corecfg_maxblklength
    _count += 1; // corecfg_8bitsupport
    _count += 1; // corecfg_adma2support
    _count += 1; // corecfg_highspeedsupport
    _count += 1; // corecfg_sdmasupport
    _count += 1; // corecfg_suspressupport
    _count += 1; // corecfg_3p3voltsupport
    _count += 1; // corecfg_3p0voltsupport
    _count += 1; // corecfg_1p8voltsupport
    _count += 1; // corecfg_64bitsupport
    _count += 1; // corecfg_asynchintrsupport
    _count += 2; // corecfg_slottype
    _count += 1; // corecfg_sdr50support
    _count += 1; // corecfg_sdr104support
    _count += 1; // corecfg_ddr50support
    _count += 1; // corecfg_hs400support
    _count += 1; // corecfg_adriversupport
    _count += 1; // corecfg_cdriversupport
    _count += 1; // corecfg_ddriversupport
    _count += 1; // corecfg_type4support
    _count += 4; // corecfg_retuningtimercnt
    _count += 1; // corecfg_tuningforsdr50
    _count += 2; // corecfg_retuningmodes
    _count += 8; // corecfg_clockmultiplier
    _count += 1; // corecfg_spisupport
    _count += 1; // corecfg_spiblkmode
    _count += 13; // corecfg_initpresetval
    _count += 13; // corecfg_dspdpresetval
    _count += 13; // corecfg_hspdpresetval
    _count += 13; // corecfg_sdr12presetval
    _count += 13; // corecfg_sdr25presetval
    _count += 13; // corecfg_sdr50presetval
    _count += 13; // corecfg_sdr104presetval
    _count += 13; // corecfg_ddr50presetval
    _count += 13; // corecfg_hs400presetval
    _count += 8; // corecfg_maxcurrent1p8v
    _count += 8; // corecfg_maxcurrent3p0v
    _count += 8; // corecfg_maxcurrent3p3v
    _count += 10; // corecfg_cqfval
    _count += 4; // corecfg_cqfmul
    _count += 1; // corecfg_timeoutclkunit
    return _count;
}

int cap_em_csr_em_cfg_t::s_get_width() {
    int _count = 0;

    _count += 1; // debug_port_enable
    _count += 1; // debug_port_select
    _count += 6; // mstid
    return _count;
}

int cap_em_csr_cfg_ecc_t::s_get_width() {
    int _count = 0;

    _count += 1; // disable_det_SRAM1
    _count += 1; // disable_cor_SRAM2
    _count += 1; // disable_det_SRAM2
    return _count;
}

int cap_em_csr_sta_ecc_t::s_get_width() {
    int _count = 0;

    _count += 1; // correctable_SRAM1
    _count += 1; // uncorrectable_SRAM1
    _count += 1; // correctable_SRAM2
    _count += 1; // uncorrectable_SRAM2
    _count += 8; // addr_SRAM1
    _count += 8; // addr_SRAM2
    _count += 8; // syndrome_SRAM1
    _count += 8; // syndrome_SRAM2
    return _count;
}

int cap_em_csr_sta_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // done_fail_SRAM1
    _count += 1; // done_fail_SRAM2
    _count += 1; // done_pass_SRAM1
    _count += 1; // done_pass_SRAM2
    return _count;
}

int cap_em_csr_cfg_bist_t::s_get_width() {
    int _count = 0;

    _count += 1; // run_SRAM1
    _count += 1; // run_SRAM2
    return _count;
}

int cap_em_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_em_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_em_csr_base_t::s_get_width(); // base
    _count += cap_em_csr_cfg_bist_t::s_get_width(); // cfg_bist
    _count += cap_em_csr_sta_bist_t::s_get_width(); // sta_bist
    _count += cap_em_csr_sta_ecc_t::s_get_width(); // sta_ecc
    _count += cap_em_csr_cfg_ecc_t::s_get_width(); // cfg_ecc
    _count += cap_em_csr_em_cfg_t::s_get_width(); // em_cfg
    _count += cap_em_csr_em_cfg_core_t::s_get_width(); // em_cfg_core
    _count += cap_em_csr_csr_intr_t::s_get_width(); // csr_intr
    _count += cap_emm_csr_t::s_get_width(); // emm
    _count += cap_em_csr_intgrp_status_t::s_get_width(); // int_groups
    _count += cap_em_csr_intgrp_t::s_get_width(); // axi
    return _count;
}

void cap_em_csr_axi_int_enable_clear_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi_intr_enable = _val.convert_to< axi_intr_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_wkup_enable = _val.convert_to< axi_wkup_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram1_ecc_correctable_enable = _val.convert_to< sram1_ecc_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram1_ecc_uncorrectable_enable = _val.convert_to< sram1_ecc_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram2_ecc_correctable_enable = _val.convert_to< sram2_ecc_correctable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram2_ecc_uncorrectable_enable = _val.convert_to< sram2_ecc_uncorrectable_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_intreg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi_intr_interrupt = _val.convert_to< axi_intr_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__axi_wkup_interrupt = _val.convert_to< axi_wkup_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram1_ecc_correctable_interrupt = _val.convert_to< sram1_ecc_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram1_ecc_uncorrectable_interrupt = _val.convert_to< sram1_ecc_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram2_ecc_correctable_interrupt = _val.convert_to< sram2_ecc_correctable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__sram2_ecc_uncorrectable_interrupt = _val.convert_to< sram2_ecc_uncorrectable_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_intgrp_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_test_set.all( _val);
    _val = _val >> int_test_set.get_width(); 
    int_enable_set.all( _val);
    _val = _val >> int_enable_set.get_width(); 
    int_enable_clear.all( _val);
    _val = _val >> int_enable_clear.get_width(); 
}

void cap_em_csr_intreg_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi_interrupt = _val.convert_to< axi_interrupt_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_int_groups_int_enable_rw_reg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__axi_enable = _val.convert_to< axi_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_intgrp_status_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    intreg.all( _val);
    _val = _val >> intreg.get_width(); 
    int_enable_rw_reg.all( _val);
    _val = _val >> int_enable_rw_reg.get_width(); 
    int_rw_reg.all( _val);
    _val = _val >> int_rw_reg.get_width(); 
}

void cap_em_csr_csr_intr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_em_cfg_core_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__corecfg_baseclkfreq = _val.convert_to< corecfg_baseclkfreq_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__corecfg_timeoutclkfreq = _val.convert_to< corecfg_timeoutclkfreq_cpp_int_t >()  ;
    _val = _val >> 6;
    
    int_var__corecfg_maxblklength = _val.convert_to< corecfg_maxblklength_cpp_int_t >()  ;
    _val = _val >> 21;
    
    int_var__corecfg_8bitsupport = _val.convert_to< corecfg_8bitsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_adma2support = _val.convert_to< corecfg_adma2support_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_highspeedsupport = _val.convert_to< corecfg_highspeedsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_sdmasupport = _val.convert_to< corecfg_sdmasupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_suspressupport = _val.convert_to< corecfg_suspressupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_3p3voltsupport = _val.convert_to< corecfg_3p3voltsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_3p0voltsupport = _val.convert_to< corecfg_3p0voltsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_1p8voltsupport = _val.convert_to< corecfg_1p8voltsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_64bitsupport = _val.convert_to< corecfg_64bitsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_asynchintrsupport = _val.convert_to< corecfg_asynchintrsupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_slottype = _val.convert_to< corecfg_slottype_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__corecfg_sdr50support = _val.convert_to< corecfg_sdr50support_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_sdr104support = _val.convert_to< corecfg_sdr104support_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_ddr50support = _val.convert_to< corecfg_ddr50support_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_hs400support = _val.convert_to< corecfg_hs400support_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_adriversupport = _val.convert_to< corecfg_adriversupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_cdriversupport = _val.convert_to< corecfg_cdriversupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_ddriversupport = _val.convert_to< corecfg_ddriversupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_type4support = _val.convert_to< corecfg_type4support_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_retuningtimercnt = _val.convert_to< corecfg_retuningtimercnt_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__corecfg_tuningforsdr50 = _val.convert_to< corecfg_tuningforsdr50_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_retuningmodes = _val.convert_to< corecfg_retuningmodes_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__corecfg_clockmultiplier = _val.convert_to< corecfg_clockmultiplier_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__corecfg_spisupport = _val.convert_to< corecfg_spisupport_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_spiblkmode = _val.convert_to< corecfg_spiblkmode_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__corecfg_initpresetval = _val.convert_to< corecfg_initpresetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_dspdpresetval = _val.convert_to< corecfg_dspdpresetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_hspdpresetval = _val.convert_to< corecfg_hspdpresetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_sdr12presetval = _val.convert_to< corecfg_sdr12presetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_sdr25presetval = _val.convert_to< corecfg_sdr25presetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_sdr50presetval = _val.convert_to< corecfg_sdr50presetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_sdr104presetval = _val.convert_to< corecfg_sdr104presetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_ddr50presetval = _val.convert_to< corecfg_ddr50presetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_hs400presetval = _val.convert_to< corecfg_hs400presetval_cpp_int_t >()  ;
    _val = _val >> 13;
    
    int_var__corecfg_maxcurrent1p8v = _val.convert_to< corecfg_maxcurrent1p8v_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__corecfg_maxcurrent3p0v = _val.convert_to< corecfg_maxcurrent3p0v_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__corecfg_maxcurrent3p3v = _val.convert_to< corecfg_maxcurrent3p3v_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__corecfg_cqfval = _val.convert_to< corecfg_cqfval_cpp_int_t >()  ;
    _val = _val >> 10;
    
    int_var__corecfg_cqfmul = _val.convert_to< corecfg_cqfmul_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__corecfg_timeoutclkunit = _val.convert_to< corecfg_timeoutclkunit_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_em_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__debug_port_enable = _val.convert_to< debug_port_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__debug_port_select = _val.convert_to< debug_port_select_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mstid = _val.convert_to< mstid_cpp_int_t >()  ;
    _val = _val >> 6;
    
}

void cap_em_csr_cfg_ecc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__disable_det_SRAM1 = _val.convert_to< disable_det_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__disable_cor_SRAM2 = _val.convert_to< disable_cor_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__disable_det_SRAM2 = _val.convert_to< disable_det_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_sta_ecc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__correctable_SRAM1 = _val.convert_to< correctable_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__uncorrectable_SRAM1 = _val.convert_to< uncorrectable_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__correctable_SRAM2 = _val.convert_to< correctable_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__uncorrectable_SRAM2 = _val.convert_to< uncorrectable_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__addr_SRAM1 = _val.convert_to< addr_SRAM1_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__addr_SRAM2 = _val.convert_to< addr_SRAM2_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__syndrome_SRAM1 = _val.convert_to< syndrome_SRAM1_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__syndrome_SRAM2 = _val.convert_to< syndrome_SRAM2_cpp_int_t >()  ;
    _val = _val >> 8;
    
}

void cap_em_csr_sta_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__done_fail_SRAM1 = _val.convert_to< done_fail_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_fail_SRAM2 = _val.convert_to< done_fail_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_pass_SRAM1 = _val.convert_to< done_pass_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__done_pass_SRAM2 = _val.convert_to< done_pass_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_cfg_bist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__run_SRAM1 = _val.convert_to< run_SRAM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__run_SRAM2 = _val.convert_to< run_SRAM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_em_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_em_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    cfg_bist.all( _val);
    _val = _val >> cfg_bist.get_width(); 
    sta_bist.all( _val);
    _val = _val >> sta_bist.get_width(); 
    sta_ecc.all( _val);
    _val = _val >> sta_ecc.get_width(); 
    cfg_ecc.all( _val);
    _val = _val >> cfg_ecc.get_width(); 
    em_cfg.all( _val);
    _val = _val >> em_cfg.get_width(); 
    em_cfg_core.all( _val);
    _val = _val >> em_cfg_core.get_width(); 
    csr_intr.all( _val);
    _val = _val >> csr_intr.get_width(); 
    emm.all( _val);
    _val = _val >> emm.get_width(); 
    int_groups.all( _val);
    _val = _val >> int_groups.get_width(); 
    axi.all( _val);
    _val = _val >> axi.get_width(); 
}

cpp_int cap_em_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << axi.get_width(); ret_val = ret_val  | axi.all(); 
    ret_val = ret_val << int_groups.get_width(); ret_val = ret_val  | int_groups.all(); 
    ret_val = ret_val << emm.get_width(); ret_val = ret_val  | emm.all(); 
    ret_val = ret_val << csr_intr.get_width(); ret_val = ret_val  | csr_intr.all(); 
    ret_val = ret_val << em_cfg_core.get_width(); ret_val = ret_val  | em_cfg_core.all(); 
    ret_val = ret_val << em_cfg.get_width(); ret_val = ret_val  | em_cfg.all(); 
    ret_val = ret_val << cfg_ecc.get_width(); ret_val = ret_val  | cfg_ecc.all(); 
    ret_val = ret_val << sta_ecc.get_width(); ret_val = ret_val  | sta_ecc.all(); 
    ret_val = ret_val << sta_bist.get_width(); ret_val = ret_val  | sta_bist.all(); 
    ret_val = ret_val << cfg_bist.get_width(); ret_val = ret_val  | cfg_bist.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_em_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_em_csr_cfg_bist_t::all() const {
    cpp_int ret_val;

    // run_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__run_SRAM2; 
    
    // run_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__run_SRAM1; 
    
    return ret_val;
}

cpp_int cap_em_csr_sta_bist_t::all() const {
    cpp_int ret_val;

    // done_pass_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_pass_SRAM2; 
    
    // done_pass_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_pass_SRAM1; 
    
    // done_fail_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_fail_SRAM2; 
    
    // done_fail_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__done_fail_SRAM1; 
    
    return ret_val;
}

cpp_int cap_em_csr_sta_ecc_t::all() const {
    cpp_int ret_val;

    // syndrome_SRAM2
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome_SRAM2; 
    
    // syndrome_SRAM1
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__syndrome_SRAM1; 
    
    // addr_SRAM2
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr_SRAM2; 
    
    // addr_SRAM1
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__addr_SRAM1; 
    
    // uncorrectable_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable_SRAM2; 
    
    // correctable_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable_SRAM2; 
    
    // uncorrectable_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__uncorrectable_SRAM1; 
    
    // correctable_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__correctable_SRAM1; 
    
    return ret_val;
}

cpp_int cap_em_csr_cfg_ecc_t::all() const {
    cpp_int ret_val;

    // disable_det_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__disable_det_SRAM2; 
    
    // disable_cor_SRAM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__disable_cor_SRAM2; 
    
    // disable_det_SRAM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__disable_det_SRAM1; 
    
    return ret_val;
}

cpp_int cap_em_csr_em_cfg_t::all() const {
    cpp_int ret_val;

    // mstid
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__mstid; 
    
    // debug_port_select
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_port_select; 
    
    // debug_port_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_port_enable; 
    
    return ret_val;
}

cpp_int cap_em_csr_em_cfg_core_t::all() const {
    cpp_int ret_val;

    // corecfg_timeoutclkunit
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_timeoutclkunit; 
    
    // corecfg_cqfmul
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__corecfg_cqfmul; 
    
    // corecfg_cqfval
    ret_val = ret_val << 10; ret_val = ret_val  | int_var__corecfg_cqfval; 
    
    // corecfg_maxcurrent3p3v
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__corecfg_maxcurrent3p3v; 
    
    // corecfg_maxcurrent3p0v
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__corecfg_maxcurrent3p0v; 
    
    // corecfg_maxcurrent1p8v
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__corecfg_maxcurrent1p8v; 
    
    // corecfg_hs400presetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_hs400presetval; 
    
    // corecfg_ddr50presetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_ddr50presetval; 
    
    // corecfg_sdr104presetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_sdr104presetval; 
    
    // corecfg_sdr50presetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_sdr50presetval; 
    
    // corecfg_sdr25presetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_sdr25presetval; 
    
    // corecfg_sdr12presetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_sdr12presetval; 
    
    // corecfg_hspdpresetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_hspdpresetval; 
    
    // corecfg_dspdpresetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_dspdpresetval; 
    
    // corecfg_initpresetval
    ret_val = ret_val << 13; ret_val = ret_val  | int_var__corecfg_initpresetval; 
    
    // corecfg_spiblkmode
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_spiblkmode; 
    
    // corecfg_spisupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_spisupport; 
    
    // corecfg_clockmultiplier
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__corecfg_clockmultiplier; 
    
    // corecfg_retuningmodes
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__corecfg_retuningmodes; 
    
    // corecfg_tuningforsdr50
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_tuningforsdr50; 
    
    // corecfg_retuningtimercnt
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__corecfg_retuningtimercnt; 
    
    // corecfg_type4support
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_type4support; 
    
    // corecfg_ddriversupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_ddriversupport; 
    
    // corecfg_cdriversupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_cdriversupport; 
    
    // corecfg_adriversupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_adriversupport; 
    
    // corecfg_hs400support
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_hs400support; 
    
    // corecfg_ddr50support
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_ddr50support; 
    
    // corecfg_sdr104support
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_sdr104support; 
    
    // corecfg_sdr50support
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_sdr50support; 
    
    // corecfg_slottype
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__corecfg_slottype; 
    
    // corecfg_asynchintrsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_asynchintrsupport; 
    
    // corecfg_64bitsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_64bitsupport; 
    
    // corecfg_1p8voltsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_1p8voltsupport; 
    
    // corecfg_3p0voltsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_3p0voltsupport; 
    
    // corecfg_3p3voltsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_3p3voltsupport; 
    
    // corecfg_suspressupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_suspressupport; 
    
    // corecfg_sdmasupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_sdmasupport; 
    
    // corecfg_highspeedsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_highspeedsupport; 
    
    // corecfg_adma2support
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_adma2support; 
    
    // corecfg_8bitsupport
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__corecfg_8bitsupport; 
    
    // corecfg_maxblklength
    ret_val = ret_val << 21; ret_val = ret_val  | int_var__corecfg_maxblklength; 
    
    // corecfg_timeoutclkfreq
    ret_val = ret_val << 6; ret_val = ret_val  | int_var__corecfg_timeoutclkfreq; 
    
    // corecfg_baseclkfreq
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__corecfg_baseclkfreq; 
    
    return ret_val;
}

cpp_int cap_em_csr_csr_intr_t::all() const {
    cpp_int ret_val;

    // dowstream_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream_enable; 
    
    // dowstream
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__dowstream; 
    
    return ret_val;
}

cpp_int cap_em_csr_intgrp_status_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_rw_reg.get_width(); ret_val = ret_val  | int_rw_reg.all(); 
    ret_val = ret_val << int_enable_rw_reg.get_width(); ret_val = ret_val  | int_enable_rw_reg.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_em_csr_intreg_status_t::all() const {
    cpp_int ret_val;

    // axi_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_interrupt; 
    
    return ret_val;
}

cpp_int cap_em_csr_int_groups_int_enable_rw_reg_t::all() const {
    cpp_int ret_val;

    // axi_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_enable; 
    
    return ret_val;
}

cpp_int cap_em_csr_intgrp_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << int_enable_clear.get_width(); ret_val = ret_val  | int_enable_clear.all(); 
    ret_val = ret_val << int_enable_set.get_width(); ret_val = ret_val  | int_enable_set.all(); 
    ret_val = ret_val << int_test_set.get_width(); ret_val = ret_val  | int_test_set.all(); 
    ret_val = ret_val << intreg.get_width(); ret_val = ret_val  | intreg.all(); 
    return ret_val;
}

cpp_int cap_em_csr_intreg_t::all() const {
    cpp_int ret_val;

    // sram2_ecc_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram2_ecc_uncorrectable_interrupt; 
    
    // sram2_ecc_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram2_ecc_correctable_interrupt; 
    
    // sram1_ecc_uncorrectable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram1_ecc_uncorrectable_interrupt; 
    
    // sram1_ecc_correctable_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram1_ecc_correctable_interrupt; 
    
    // axi_wkup_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_wkup_interrupt; 
    
    // axi_intr_interrupt
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_intr_interrupt; 
    
    return ret_val;
}

cpp_int cap_em_csr_axi_int_enable_clear_t::all() const {
    cpp_int ret_val;

    // sram2_ecc_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram2_ecc_uncorrectable_enable; 
    
    // sram2_ecc_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram2_ecc_correctable_enable; 
    
    // sram1_ecc_uncorrectable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram1_ecc_uncorrectable_enable; 
    
    // sram1_ecc_correctable_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__sram1_ecc_correctable_enable; 
    
    // axi_wkup_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_wkup_enable; 
    
    // axi_intr_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__axi_intr_enable; 
    
    return ret_val;
}

void cap_em_csr_axi_int_enable_clear_t::clear() {

    int_var__axi_intr_enable = 0; 
    
    int_var__axi_wkup_enable = 0; 
    
    int_var__sram1_ecc_correctable_enable = 0; 
    
    int_var__sram1_ecc_uncorrectable_enable = 0; 
    
    int_var__sram2_ecc_correctable_enable = 0; 
    
    int_var__sram2_ecc_uncorrectable_enable = 0; 
    
}

void cap_em_csr_intreg_t::clear() {

    int_var__axi_intr_interrupt = 0; 
    
    int_var__axi_wkup_interrupt = 0; 
    
    int_var__sram1_ecc_correctable_interrupt = 0; 
    
    int_var__sram1_ecc_uncorrectable_interrupt = 0; 
    
    int_var__sram2_ecc_correctable_interrupt = 0; 
    
    int_var__sram2_ecc_uncorrectable_interrupt = 0; 
    
}

void cap_em_csr_intgrp_t::clear() {

    intreg.clear();
    int_test_set.clear();
    int_enable_set.clear();
    int_enable_clear.clear();
}

void cap_em_csr_intreg_status_t::clear() {

    int_var__axi_interrupt = 0; 
    
}

void cap_em_csr_int_groups_int_enable_rw_reg_t::clear() {

    int_var__axi_enable = 0; 
    
}

void cap_em_csr_intgrp_status_t::clear() {

    intreg.clear();
    int_enable_rw_reg.clear();
    int_rw_reg.clear();
}

void cap_em_csr_csr_intr_t::clear() {

    int_var__dowstream = 0; 
    
    int_var__dowstream_enable = 0; 
    
}

void cap_em_csr_em_cfg_core_t::clear() {

    int_var__corecfg_baseclkfreq = 0; 
    
    int_var__corecfg_timeoutclkfreq = 0; 
    
    int_var__corecfg_maxblklength = 0; 
    
    int_var__corecfg_8bitsupport = 0; 
    
    int_var__corecfg_adma2support = 0; 
    
    int_var__corecfg_highspeedsupport = 0; 
    
    int_var__corecfg_sdmasupport = 0; 
    
    int_var__corecfg_suspressupport = 0; 
    
    int_var__corecfg_3p3voltsupport = 0; 
    
    int_var__corecfg_3p0voltsupport = 0; 
    
    int_var__corecfg_1p8voltsupport = 0; 
    
    int_var__corecfg_64bitsupport = 0; 
    
    int_var__corecfg_asynchintrsupport = 0; 
    
    int_var__corecfg_slottype = 0; 
    
    int_var__corecfg_sdr50support = 0; 
    
    int_var__corecfg_sdr104support = 0; 
    
    int_var__corecfg_ddr50support = 0; 
    
    int_var__corecfg_hs400support = 0; 
    
    int_var__corecfg_adriversupport = 0; 
    
    int_var__corecfg_cdriversupport = 0; 
    
    int_var__corecfg_ddriversupport = 0; 
    
    int_var__corecfg_type4support = 0; 
    
    int_var__corecfg_retuningtimercnt = 0; 
    
    int_var__corecfg_tuningforsdr50 = 0; 
    
    int_var__corecfg_retuningmodes = 0; 
    
    int_var__corecfg_clockmultiplier = 0; 
    
    int_var__corecfg_spisupport = 0; 
    
    int_var__corecfg_spiblkmode = 0; 
    
    int_var__corecfg_initpresetval = 0; 
    
    int_var__corecfg_dspdpresetval = 0; 
    
    int_var__corecfg_hspdpresetval = 0; 
    
    int_var__corecfg_sdr12presetval = 0; 
    
    int_var__corecfg_sdr25presetval = 0; 
    
    int_var__corecfg_sdr50presetval = 0; 
    
    int_var__corecfg_sdr104presetval = 0; 
    
    int_var__corecfg_ddr50presetval = 0; 
    
    int_var__corecfg_hs400presetval = 0; 
    
    int_var__corecfg_maxcurrent1p8v = 0; 
    
    int_var__corecfg_maxcurrent3p0v = 0; 
    
    int_var__corecfg_maxcurrent3p3v = 0; 
    
    int_var__corecfg_cqfval = 0; 
    
    int_var__corecfg_cqfmul = 0; 
    
    int_var__corecfg_timeoutclkunit = 0; 
    
}

void cap_em_csr_em_cfg_t::clear() {

    int_var__debug_port_enable = 0; 
    
    int_var__debug_port_select = 0; 
    
    int_var__mstid = 0; 
    
}

void cap_em_csr_cfg_ecc_t::clear() {

    int_var__disable_det_SRAM1 = 0; 
    
    int_var__disable_cor_SRAM2 = 0; 
    
    int_var__disable_det_SRAM2 = 0; 
    
}

void cap_em_csr_sta_ecc_t::clear() {

    int_var__correctable_SRAM1 = 0; 
    
    int_var__uncorrectable_SRAM1 = 0; 
    
    int_var__correctable_SRAM2 = 0; 
    
    int_var__uncorrectable_SRAM2 = 0; 
    
    int_var__addr_SRAM1 = 0; 
    
    int_var__addr_SRAM2 = 0; 
    
    int_var__syndrome_SRAM1 = 0; 
    
    int_var__syndrome_SRAM2 = 0; 
    
}

void cap_em_csr_sta_bist_t::clear() {

    int_var__done_fail_SRAM1 = 0; 
    
    int_var__done_fail_SRAM2 = 0; 
    
    int_var__done_pass_SRAM1 = 0; 
    
    int_var__done_pass_SRAM2 = 0; 
    
}

void cap_em_csr_cfg_bist_t::clear() {

    int_var__run_SRAM1 = 0; 
    
    int_var__run_SRAM2 = 0; 
    
}

void cap_em_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_em_csr_t::clear() {

    base.clear();
    cfg_bist.clear();
    sta_bist.clear();
    sta_ecc.clear();
    cfg_ecc.clear();
    em_cfg.clear();
    em_cfg_core.clear();
    csr_intr.clear();
    emm.clear();
    int_groups.clear();
    axi.clear();
}

void cap_em_csr_axi_int_enable_clear_t::init() {

}

void cap_em_csr_intreg_t::init() {

}

void cap_em_csr_intgrp_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_test_set.set_attributes(this,"int_test_set", 0x4 );
    int_enable_set.set_attributes(this,"int_enable_set", 0x8 );
    int_enable_clear.set_attributes(this,"int_enable_clear", 0xc );
}

void cap_em_csr_intreg_status_t::init() {

}

void cap_em_csr_int_groups_int_enable_rw_reg_t::init() {

}

void cap_em_csr_intgrp_status_t::init() {

    intreg.set_attributes(this,"intreg", 0x0 );
    int_enable_rw_reg.set_attributes(this,"int_enable_rw_reg", 0x4 );
    int_rw_reg.set_attributes(this,"int_rw_reg", 0x8 );
}

void cap_em_csr_csr_intr_t::init() {

}

void cap_em_csr_em_cfg_core_t::init() {

}

void cap_em_csr_em_cfg_t::init() {

}

void cap_em_csr_cfg_ecc_t::init() {

}

void cap_em_csr_sta_ecc_t::init() {

}

void cap_em_csr_sta_bist_t::init() {

}

void cap_em_csr_cfg_bist_t::init() {

}

void cap_em_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_em_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    cfg_bist.set_attributes(this,"cfg_bist", 0x40 );
    sta_bist.set_attributes(this,"sta_bist", 0x44 );
    sta_ecc.set_attributes(this,"sta_ecc", 0x48 );
    cfg_ecc.set_attributes(this,"cfg_ecc", 0x50 );
    em_cfg.set_attributes(this,"em_cfg", 0x54 );
    em_cfg_core.set_attributes(this,"em_cfg_core", 0x60 );
    csr_intr.set_attributes(this,"csr_intr", 0x80 );
    emm.set_attributes(this,"emm", 0x20 );
    int_groups.set_attributes(this,"int_groups", 0x90 );
    axi.set_attributes(this,"axi", 0xa0 );
}

void cap_em_csr_axi_int_enable_clear_t::axi_intr_enable(const cpp_int & _val) { 
    // axi_intr_enable
    int_var__axi_intr_enable = _val.convert_to< axi_intr_enable_cpp_int_t >();
}

cpp_int cap_em_csr_axi_int_enable_clear_t::axi_intr_enable() const {
    return int_var__axi_intr_enable;
}
    
void cap_em_csr_axi_int_enable_clear_t::axi_wkup_enable(const cpp_int & _val) { 
    // axi_wkup_enable
    int_var__axi_wkup_enable = _val.convert_to< axi_wkup_enable_cpp_int_t >();
}

cpp_int cap_em_csr_axi_int_enable_clear_t::axi_wkup_enable() const {
    return int_var__axi_wkup_enable;
}
    
void cap_em_csr_axi_int_enable_clear_t::sram1_ecc_correctable_enable(const cpp_int & _val) { 
    // sram1_ecc_correctable_enable
    int_var__sram1_ecc_correctable_enable = _val.convert_to< sram1_ecc_correctable_enable_cpp_int_t >();
}

cpp_int cap_em_csr_axi_int_enable_clear_t::sram1_ecc_correctable_enable() const {
    return int_var__sram1_ecc_correctable_enable;
}
    
void cap_em_csr_axi_int_enable_clear_t::sram1_ecc_uncorrectable_enable(const cpp_int & _val) { 
    // sram1_ecc_uncorrectable_enable
    int_var__sram1_ecc_uncorrectable_enable = _val.convert_to< sram1_ecc_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_em_csr_axi_int_enable_clear_t::sram1_ecc_uncorrectable_enable() const {
    return int_var__sram1_ecc_uncorrectable_enable;
}
    
void cap_em_csr_axi_int_enable_clear_t::sram2_ecc_correctable_enable(const cpp_int & _val) { 
    // sram2_ecc_correctable_enable
    int_var__sram2_ecc_correctable_enable = _val.convert_to< sram2_ecc_correctable_enable_cpp_int_t >();
}

cpp_int cap_em_csr_axi_int_enable_clear_t::sram2_ecc_correctable_enable() const {
    return int_var__sram2_ecc_correctable_enable;
}
    
void cap_em_csr_axi_int_enable_clear_t::sram2_ecc_uncorrectable_enable(const cpp_int & _val) { 
    // sram2_ecc_uncorrectable_enable
    int_var__sram2_ecc_uncorrectable_enable = _val.convert_to< sram2_ecc_uncorrectable_enable_cpp_int_t >();
}

cpp_int cap_em_csr_axi_int_enable_clear_t::sram2_ecc_uncorrectable_enable() const {
    return int_var__sram2_ecc_uncorrectable_enable;
}
    
void cap_em_csr_intreg_t::axi_intr_interrupt(const cpp_int & _val) { 
    // axi_intr_interrupt
    int_var__axi_intr_interrupt = _val.convert_to< axi_intr_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_t::axi_intr_interrupt() const {
    return int_var__axi_intr_interrupt;
}
    
void cap_em_csr_intreg_t::axi_wkup_interrupt(const cpp_int & _val) { 
    // axi_wkup_interrupt
    int_var__axi_wkup_interrupt = _val.convert_to< axi_wkup_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_t::axi_wkup_interrupt() const {
    return int_var__axi_wkup_interrupt;
}
    
void cap_em_csr_intreg_t::sram1_ecc_correctable_interrupt(const cpp_int & _val) { 
    // sram1_ecc_correctable_interrupt
    int_var__sram1_ecc_correctable_interrupt = _val.convert_to< sram1_ecc_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_t::sram1_ecc_correctable_interrupt() const {
    return int_var__sram1_ecc_correctable_interrupt;
}
    
void cap_em_csr_intreg_t::sram1_ecc_uncorrectable_interrupt(const cpp_int & _val) { 
    // sram1_ecc_uncorrectable_interrupt
    int_var__sram1_ecc_uncorrectable_interrupt = _val.convert_to< sram1_ecc_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_t::sram1_ecc_uncorrectable_interrupt() const {
    return int_var__sram1_ecc_uncorrectable_interrupt;
}
    
void cap_em_csr_intreg_t::sram2_ecc_correctable_interrupt(const cpp_int & _val) { 
    // sram2_ecc_correctable_interrupt
    int_var__sram2_ecc_correctable_interrupt = _val.convert_to< sram2_ecc_correctable_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_t::sram2_ecc_correctable_interrupt() const {
    return int_var__sram2_ecc_correctable_interrupt;
}
    
void cap_em_csr_intreg_t::sram2_ecc_uncorrectable_interrupt(const cpp_int & _val) { 
    // sram2_ecc_uncorrectable_interrupt
    int_var__sram2_ecc_uncorrectable_interrupt = _val.convert_to< sram2_ecc_uncorrectable_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_t::sram2_ecc_uncorrectable_interrupt() const {
    return int_var__sram2_ecc_uncorrectable_interrupt;
}
    
void cap_em_csr_intreg_status_t::axi_interrupt(const cpp_int & _val) { 
    // axi_interrupt
    int_var__axi_interrupt = _val.convert_to< axi_interrupt_cpp_int_t >();
}

cpp_int cap_em_csr_intreg_status_t::axi_interrupt() const {
    return int_var__axi_interrupt;
}
    
void cap_em_csr_int_groups_int_enable_rw_reg_t::axi_enable(const cpp_int & _val) { 
    // axi_enable
    int_var__axi_enable = _val.convert_to< axi_enable_cpp_int_t >();
}

cpp_int cap_em_csr_int_groups_int_enable_rw_reg_t::axi_enable() const {
    return int_var__axi_enable;
}
    
void cap_em_csr_csr_intr_t::dowstream(const cpp_int & _val) { 
    // dowstream
    int_var__dowstream = _val.convert_to< dowstream_cpp_int_t >();
}

cpp_int cap_em_csr_csr_intr_t::dowstream() const {
    return int_var__dowstream;
}
    
void cap_em_csr_csr_intr_t::dowstream_enable(const cpp_int & _val) { 
    // dowstream_enable
    int_var__dowstream_enable = _val.convert_to< dowstream_enable_cpp_int_t >();
}

cpp_int cap_em_csr_csr_intr_t::dowstream_enable() const {
    return int_var__dowstream_enable;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_baseclkfreq(const cpp_int & _val) { 
    // corecfg_baseclkfreq
    int_var__corecfg_baseclkfreq = _val.convert_to< corecfg_baseclkfreq_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_baseclkfreq() const {
    return int_var__corecfg_baseclkfreq;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_timeoutclkfreq(const cpp_int & _val) { 
    // corecfg_timeoutclkfreq
    int_var__corecfg_timeoutclkfreq = _val.convert_to< corecfg_timeoutclkfreq_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_timeoutclkfreq() const {
    return int_var__corecfg_timeoutclkfreq;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_maxblklength(const cpp_int & _val) { 
    // corecfg_maxblklength
    int_var__corecfg_maxblklength = _val.convert_to< corecfg_maxblklength_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_maxblklength() const {
    return int_var__corecfg_maxblklength;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_8bitsupport(const cpp_int & _val) { 
    // corecfg_8bitsupport
    int_var__corecfg_8bitsupport = _val.convert_to< corecfg_8bitsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_8bitsupport() const {
    return int_var__corecfg_8bitsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_adma2support(const cpp_int & _val) { 
    // corecfg_adma2support
    int_var__corecfg_adma2support = _val.convert_to< corecfg_adma2support_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_adma2support() const {
    return int_var__corecfg_adma2support;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_highspeedsupport(const cpp_int & _val) { 
    // corecfg_highspeedsupport
    int_var__corecfg_highspeedsupport = _val.convert_to< corecfg_highspeedsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_highspeedsupport() const {
    return int_var__corecfg_highspeedsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdmasupport(const cpp_int & _val) { 
    // corecfg_sdmasupport
    int_var__corecfg_sdmasupport = _val.convert_to< corecfg_sdmasupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdmasupport() const {
    return int_var__corecfg_sdmasupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_suspressupport(const cpp_int & _val) { 
    // corecfg_suspressupport
    int_var__corecfg_suspressupport = _val.convert_to< corecfg_suspressupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_suspressupport() const {
    return int_var__corecfg_suspressupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_3p3voltsupport(const cpp_int & _val) { 
    // corecfg_3p3voltsupport
    int_var__corecfg_3p3voltsupport = _val.convert_to< corecfg_3p3voltsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_3p3voltsupport() const {
    return int_var__corecfg_3p3voltsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_3p0voltsupport(const cpp_int & _val) { 
    // corecfg_3p0voltsupport
    int_var__corecfg_3p0voltsupport = _val.convert_to< corecfg_3p0voltsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_3p0voltsupport() const {
    return int_var__corecfg_3p0voltsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_1p8voltsupport(const cpp_int & _val) { 
    // corecfg_1p8voltsupport
    int_var__corecfg_1p8voltsupport = _val.convert_to< corecfg_1p8voltsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_1p8voltsupport() const {
    return int_var__corecfg_1p8voltsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_64bitsupport(const cpp_int & _val) { 
    // corecfg_64bitsupport
    int_var__corecfg_64bitsupport = _val.convert_to< corecfg_64bitsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_64bitsupport() const {
    return int_var__corecfg_64bitsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_asynchintrsupport(const cpp_int & _val) { 
    // corecfg_asynchintrsupport
    int_var__corecfg_asynchintrsupport = _val.convert_to< corecfg_asynchintrsupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_asynchintrsupport() const {
    return int_var__corecfg_asynchintrsupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_slottype(const cpp_int & _val) { 
    // corecfg_slottype
    int_var__corecfg_slottype = _val.convert_to< corecfg_slottype_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_slottype() const {
    return int_var__corecfg_slottype;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdr50support(const cpp_int & _val) { 
    // corecfg_sdr50support
    int_var__corecfg_sdr50support = _val.convert_to< corecfg_sdr50support_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdr50support() const {
    return int_var__corecfg_sdr50support;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdr104support(const cpp_int & _val) { 
    // corecfg_sdr104support
    int_var__corecfg_sdr104support = _val.convert_to< corecfg_sdr104support_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdr104support() const {
    return int_var__corecfg_sdr104support;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_ddr50support(const cpp_int & _val) { 
    // corecfg_ddr50support
    int_var__corecfg_ddr50support = _val.convert_to< corecfg_ddr50support_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_ddr50support() const {
    return int_var__corecfg_ddr50support;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_hs400support(const cpp_int & _val) { 
    // corecfg_hs400support
    int_var__corecfg_hs400support = _val.convert_to< corecfg_hs400support_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_hs400support() const {
    return int_var__corecfg_hs400support;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_adriversupport(const cpp_int & _val) { 
    // corecfg_adriversupport
    int_var__corecfg_adriversupport = _val.convert_to< corecfg_adriversupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_adriversupport() const {
    return int_var__corecfg_adriversupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_cdriversupport(const cpp_int & _val) { 
    // corecfg_cdriversupport
    int_var__corecfg_cdriversupport = _val.convert_to< corecfg_cdriversupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_cdriversupport() const {
    return int_var__corecfg_cdriversupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_ddriversupport(const cpp_int & _val) { 
    // corecfg_ddriversupport
    int_var__corecfg_ddriversupport = _val.convert_to< corecfg_ddriversupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_ddriversupport() const {
    return int_var__corecfg_ddriversupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_type4support(const cpp_int & _val) { 
    // corecfg_type4support
    int_var__corecfg_type4support = _val.convert_to< corecfg_type4support_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_type4support() const {
    return int_var__corecfg_type4support;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_retuningtimercnt(const cpp_int & _val) { 
    // corecfg_retuningtimercnt
    int_var__corecfg_retuningtimercnt = _val.convert_to< corecfg_retuningtimercnt_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_retuningtimercnt() const {
    return int_var__corecfg_retuningtimercnt;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_tuningforsdr50(const cpp_int & _val) { 
    // corecfg_tuningforsdr50
    int_var__corecfg_tuningforsdr50 = _val.convert_to< corecfg_tuningforsdr50_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_tuningforsdr50() const {
    return int_var__corecfg_tuningforsdr50;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_retuningmodes(const cpp_int & _val) { 
    // corecfg_retuningmodes
    int_var__corecfg_retuningmodes = _val.convert_to< corecfg_retuningmodes_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_retuningmodes() const {
    return int_var__corecfg_retuningmodes;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_clockmultiplier(const cpp_int & _val) { 
    // corecfg_clockmultiplier
    int_var__corecfg_clockmultiplier = _val.convert_to< corecfg_clockmultiplier_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_clockmultiplier() const {
    return int_var__corecfg_clockmultiplier;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_spisupport(const cpp_int & _val) { 
    // corecfg_spisupport
    int_var__corecfg_spisupport = _val.convert_to< corecfg_spisupport_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_spisupport() const {
    return int_var__corecfg_spisupport;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_spiblkmode(const cpp_int & _val) { 
    // corecfg_spiblkmode
    int_var__corecfg_spiblkmode = _val.convert_to< corecfg_spiblkmode_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_spiblkmode() const {
    return int_var__corecfg_spiblkmode;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_initpresetval(const cpp_int & _val) { 
    // corecfg_initpresetval
    int_var__corecfg_initpresetval = _val.convert_to< corecfg_initpresetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_initpresetval() const {
    return int_var__corecfg_initpresetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_dspdpresetval(const cpp_int & _val) { 
    // corecfg_dspdpresetval
    int_var__corecfg_dspdpresetval = _val.convert_to< corecfg_dspdpresetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_dspdpresetval() const {
    return int_var__corecfg_dspdpresetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_hspdpresetval(const cpp_int & _val) { 
    // corecfg_hspdpresetval
    int_var__corecfg_hspdpresetval = _val.convert_to< corecfg_hspdpresetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_hspdpresetval() const {
    return int_var__corecfg_hspdpresetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdr12presetval(const cpp_int & _val) { 
    // corecfg_sdr12presetval
    int_var__corecfg_sdr12presetval = _val.convert_to< corecfg_sdr12presetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdr12presetval() const {
    return int_var__corecfg_sdr12presetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdr25presetval(const cpp_int & _val) { 
    // corecfg_sdr25presetval
    int_var__corecfg_sdr25presetval = _val.convert_to< corecfg_sdr25presetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdr25presetval() const {
    return int_var__corecfg_sdr25presetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdr50presetval(const cpp_int & _val) { 
    // corecfg_sdr50presetval
    int_var__corecfg_sdr50presetval = _val.convert_to< corecfg_sdr50presetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdr50presetval() const {
    return int_var__corecfg_sdr50presetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_sdr104presetval(const cpp_int & _val) { 
    // corecfg_sdr104presetval
    int_var__corecfg_sdr104presetval = _val.convert_to< corecfg_sdr104presetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_sdr104presetval() const {
    return int_var__corecfg_sdr104presetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_ddr50presetval(const cpp_int & _val) { 
    // corecfg_ddr50presetval
    int_var__corecfg_ddr50presetval = _val.convert_to< corecfg_ddr50presetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_ddr50presetval() const {
    return int_var__corecfg_ddr50presetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_hs400presetval(const cpp_int & _val) { 
    // corecfg_hs400presetval
    int_var__corecfg_hs400presetval = _val.convert_to< corecfg_hs400presetval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_hs400presetval() const {
    return int_var__corecfg_hs400presetval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_maxcurrent1p8v(const cpp_int & _val) { 
    // corecfg_maxcurrent1p8v
    int_var__corecfg_maxcurrent1p8v = _val.convert_to< corecfg_maxcurrent1p8v_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_maxcurrent1p8v() const {
    return int_var__corecfg_maxcurrent1p8v;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_maxcurrent3p0v(const cpp_int & _val) { 
    // corecfg_maxcurrent3p0v
    int_var__corecfg_maxcurrent3p0v = _val.convert_to< corecfg_maxcurrent3p0v_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_maxcurrent3p0v() const {
    return int_var__corecfg_maxcurrent3p0v;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_maxcurrent3p3v(const cpp_int & _val) { 
    // corecfg_maxcurrent3p3v
    int_var__corecfg_maxcurrent3p3v = _val.convert_to< corecfg_maxcurrent3p3v_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_maxcurrent3p3v() const {
    return int_var__corecfg_maxcurrent3p3v;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_cqfval(const cpp_int & _val) { 
    // corecfg_cqfval
    int_var__corecfg_cqfval = _val.convert_to< corecfg_cqfval_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_cqfval() const {
    return int_var__corecfg_cqfval;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_cqfmul(const cpp_int & _val) { 
    // corecfg_cqfmul
    int_var__corecfg_cqfmul = _val.convert_to< corecfg_cqfmul_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_cqfmul() const {
    return int_var__corecfg_cqfmul;
}
    
void cap_em_csr_em_cfg_core_t::corecfg_timeoutclkunit(const cpp_int & _val) { 
    // corecfg_timeoutclkunit
    int_var__corecfg_timeoutclkunit = _val.convert_to< corecfg_timeoutclkunit_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_core_t::corecfg_timeoutclkunit() const {
    return int_var__corecfg_timeoutclkunit;
}
    
void cap_em_csr_em_cfg_t::debug_port_enable(const cpp_int & _val) { 
    // debug_port_enable
    int_var__debug_port_enable = _val.convert_to< debug_port_enable_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_t::debug_port_enable() const {
    return int_var__debug_port_enable;
}
    
void cap_em_csr_em_cfg_t::debug_port_select(const cpp_int & _val) { 
    // debug_port_select
    int_var__debug_port_select = _val.convert_to< debug_port_select_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_t::debug_port_select() const {
    return int_var__debug_port_select;
}
    
void cap_em_csr_em_cfg_t::mstid(const cpp_int & _val) { 
    // mstid
    int_var__mstid = _val.convert_to< mstid_cpp_int_t >();
}

cpp_int cap_em_csr_em_cfg_t::mstid() const {
    return int_var__mstid;
}
    
void cap_em_csr_cfg_ecc_t::disable_det_SRAM1(const cpp_int & _val) { 
    // disable_det_SRAM1
    int_var__disable_det_SRAM1 = _val.convert_to< disable_det_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_cfg_ecc_t::disable_det_SRAM1() const {
    return int_var__disable_det_SRAM1;
}
    
void cap_em_csr_cfg_ecc_t::disable_cor_SRAM2(const cpp_int & _val) { 
    // disable_cor_SRAM2
    int_var__disable_cor_SRAM2 = _val.convert_to< disable_cor_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_cfg_ecc_t::disable_cor_SRAM2() const {
    return int_var__disable_cor_SRAM2;
}
    
void cap_em_csr_cfg_ecc_t::disable_det_SRAM2(const cpp_int & _val) { 
    // disable_det_SRAM2
    int_var__disable_det_SRAM2 = _val.convert_to< disable_det_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_cfg_ecc_t::disable_det_SRAM2() const {
    return int_var__disable_det_SRAM2;
}
    
void cap_em_csr_sta_ecc_t::correctable_SRAM1(const cpp_int & _val) { 
    // correctable_SRAM1
    int_var__correctable_SRAM1 = _val.convert_to< correctable_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::correctable_SRAM1() const {
    return int_var__correctable_SRAM1;
}
    
void cap_em_csr_sta_ecc_t::uncorrectable_SRAM1(const cpp_int & _val) { 
    // uncorrectable_SRAM1
    int_var__uncorrectable_SRAM1 = _val.convert_to< uncorrectable_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::uncorrectable_SRAM1() const {
    return int_var__uncorrectable_SRAM1;
}
    
void cap_em_csr_sta_ecc_t::correctable_SRAM2(const cpp_int & _val) { 
    // correctable_SRAM2
    int_var__correctable_SRAM2 = _val.convert_to< correctable_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::correctable_SRAM2() const {
    return int_var__correctable_SRAM2;
}
    
void cap_em_csr_sta_ecc_t::uncorrectable_SRAM2(const cpp_int & _val) { 
    // uncorrectable_SRAM2
    int_var__uncorrectable_SRAM2 = _val.convert_to< uncorrectable_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::uncorrectable_SRAM2() const {
    return int_var__uncorrectable_SRAM2;
}
    
void cap_em_csr_sta_ecc_t::addr_SRAM1(const cpp_int & _val) { 
    // addr_SRAM1
    int_var__addr_SRAM1 = _val.convert_to< addr_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::addr_SRAM1() const {
    return int_var__addr_SRAM1;
}
    
void cap_em_csr_sta_ecc_t::addr_SRAM2(const cpp_int & _val) { 
    // addr_SRAM2
    int_var__addr_SRAM2 = _val.convert_to< addr_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::addr_SRAM2() const {
    return int_var__addr_SRAM2;
}
    
void cap_em_csr_sta_ecc_t::syndrome_SRAM1(const cpp_int & _val) { 
    // syndrome_SRAM1
    int_var__syndrome_SRAM1 = _val.convert_to< syndrome_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::syndrome_SRAM1() const {
    return int_var__syndrome_SRAM1;
}
    
void cap_em_csr_sta_ecc_t::syndrome_SRAM2(const cpp_int & _val) { 
    // syndrome_SRAM2
    int_var__syndrome_SRAM2 = _val.convert_to< syndrome_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_sta_ecc_t::syndrome_SRAM2() const {
    return int_var__syndrome_SRAM2;
}
    
void cap_em_csr_sta_bist_t::done_fail_SRAM1(const cpp_int & _val) { 
    // done_fail_SRAM1
    int_var__done_fail_SRAM1 = _val.convert_to< done_fail_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_sta_bist_t::done_fail_SRAM1() const {
    return int_var__done_fail_SRAM1;
}
    
void cap_em_csr_sta_bist_t::done_fail_SRAM2(const cpp_int & _val) { 
    // done_fail_SRAM2
    int_var__done_fail_SRAM2 = _val.convert_to< done_fail_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_sta_bist_t::done_fail_SRAM2() const {
    return int_var__done_fail_SRAM2;
}
    
void cap_em_csr_sta_bist_t::done_pass_SRAM1(const cpp_int & _val) { 
    // done_pass_SRAM1
    int_var__done_pass_SRAM1 = _val.convert_to< done_pass_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_sta_bist_t::done_pass_SRAM1() const {
    return int_var__done_pass_SRAM1;
}
    
void cap_em_csr_sta_bist_t::done_pass_SRAM2(const cpp_int & _val) { 
    // done_pass_SRAM2
    int_var__done_pass_SRAM2 = _val.convert_to< done_pass_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_sta_bist_t::done_pass_SRAM2() const {
    return int_var__done_pass_SRAM2;
}
    
void cap_em_csr_cfg_bist_t::run_SRAM1(const cpp_int & _val) { 
    // run_SRAM1
    int_var__run_SRAM1 = _val.convert_to< run_SRAM1_cpp_int_t >();
}

cpp_int cap_em_csr_cfg_bist_t::run_SRAM1() const {
    return int_var__run_SRAM1;
}
    
void cap_em_csr_cfg_bist_t::run_SRAM2(const cpp_int & _val) { 
    // run_SRAM2
    int_var__run_SRAM2 = _val.convert_to< run_SRAM2_cpp_int_t >();
}

cpp_int cap_em_csr_cfg_bist_t::run_SRAM2() const {
    return int_var__run_SRAM2;
}
    
void cap_em_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_em_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_axi_int_enable_clear_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_intr_enable")) { field_val = axi_intr_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wkup_enable")) { field_val = axi_wkup_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_correctable_enable")) { field_val = sram1_ecc_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_uncorrectable_enable")) { field_val = sram1_ecc_uncorrectable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_correctable_enable")) { field_val = sram2_ecc_correctable_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_uncorrectable_enable")) { field_val = sram2_ecc_uncorrectable_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intreg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_intr_interrupt")) { field_val = axi_intr_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wkup_interrupt")) { field_val = axi_wkup_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_correctable_interrupt")) { field_val = sram1_ecc_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_uncorrectable_interrupt")) { field_val = sram1_ecc_uncorrectable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_correctable_interrupt")) { field_val = sram2_ecc_correctable_interrupt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_uncorrectable_interrupt")) { field_val = sram2_ecc_uncorrectable_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intgrp_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = intreg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_test_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_set.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_clear.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intreg_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_interrupt")) { field_val = axi_interrupt(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_int_groups_int_enable_rw_reg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_enable")) { field_val = axi_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intgrp_status_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = intreg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_rw_reg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_rw_reg.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_csr_intr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { field_val = dowstream(); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { field_val = dowstream_enable(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_em_cfg_core_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "corecfg_baseclkfreq")) { field_val = corecfg_baseclkfreq(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_timeoutclkfreq")) { field_val = corecfg_timeoutclkfreq(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxblklength")) { field_val = corecfg_maxblklength(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_8bitsupport")) { field_val = corecfg_8bitsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_adma2support")) { field_val = corecfg_adma2support(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_highspeedsupport")) { field_val = corecfg_highspeedsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdmasupport")) { field_val = corecfg_sdmasupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_suspressupport")) { field_val = corecfg_suspressupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_3p3voltsupport")) { field_val = corecfg_3p3voltsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_3p0voltsupport")) { field_val = corecfg_3p0voltsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_1p8voltsupport")) { field_val = corecfg_1p8voltsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_64bitsupport")) { field_val = corecfg_64bitsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_asynchintrsupport")) { field_val = corecfg_asynchintrsupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_slottype")) { field_val = corecfg_slottype(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr50support")) { field_val = corecfg_sdr50support(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr104support")) { field_val = corecfg_sdr104support(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_ddr50support")) { field_val = corecfg_ddr50support(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_hs400support")) { field_val = corecfg_hs400support(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_adriversupport")) { field_val = corecfg_adriversupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_cdriversupport")) { field_val = corecfg_cdriversupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_ddriversupport")) { field_val = corecfg_ddriversupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_type4support")) { field_val = corecfg_type4support(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_retuningtimercnt")) { field_val = corecfg_retuningtimercnt(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_tuningforsdr50")) { field_val = corecfg_tuningforsdr50(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_retuningmodes")) { field_val = corecfg_retuningmodes(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_clockmultiplier")) { field_val = corecfg_clockmultiplier(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_spisupport")) { field_val = corecfg_spisupport(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_spiblkmode")) { field_val = corecfg_spiblkmode(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_initpresetval")) { field_val = corecfg_initpresetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_dspdpresetval")) { field_val = corecfg_dspdpresetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_hspdpresetval")) { field_val = corecfg_hspdpresetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr12presetval")) { field_val = corecfg_sdr12presetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr25presetval")) { field_val = corecfg_sdr25presetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr50presetval")) { field_val = corecfg_sdr50presetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr104presetval")) { field_val = corecfg_sdr104presetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_ddr50presetval")) { field_val = corecfg_ddr50presetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_hs400presetval")) { field_val = corecfg_hs400presetval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxcurrent1p8v")) { field_val = corecfg_maxcurrent1p8v(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxcurrent3p0v")) { field_val = corecfg_maxcurrent3p0v(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxcurrent3p3v")) { field_val = corecfg_maxcurrent3p3v(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_cqfval")) { field_val = corecfg_cqfval(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_cqfmul")) { field_val = corecfg_cqfmul(); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_timeoutclkunit")) { field_val = corecfg_timeoutclkunit(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_em_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "debug_port_enable")) { field_val = debug_port_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_port_select")) { field_val = debug_port_select(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mstid")) { field_val = mstid(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_cfg_ecc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "disable_det_SRAM1")) { field_val = disable_det_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "disable_cor_SRAM2")) { field_val = disable_cor_SRAM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "disable_det_SRAM2")) { field_val = disable_det_SRAM2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_sta_ecc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "correctable_SRAM1")) { field_val = correctable_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_SRAM1")) { field_val = uncorrectable_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_SRAM2")) { field_val = correctable_SRAM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_SRAM2")) { field_val = uncorrectable_SRAM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_SRAM1")) { field_val = addr_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_SRAM2")) { field_val = addr_SRAM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome_SRAM1")) { field_val = syndrome_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome_SRAM2")) { field_val = syndrome_SRAM2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_sta_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_fail_SRAM1")) { field_val = done_fail_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_fail_SRAM2")) { field_val = done_fail_SRAM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_pass_SRAM1")) { field_val = done_pass_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_pass_SRAM2")) { field_val = done_pass_SRAM2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_cfg_bist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run_SRAM1")) { field_val = run_SRAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "run_SRAM2")) { field_val = run_SRAM2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ecc.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ecc.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_cfg_core.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = emm.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = axi.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_axi_int_enable_clear_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_intr_enable")) { axi_intr_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wkup_enable")) { axi_wkup_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_correctable_enable")) { sram1_ecc_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_uncorrectable_enable")) { sram1_ecc_uncorrectable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_correctable_enable")) { sram2_ecc_correctable_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_uncorrectable_enable")) { sram2_ecc_uncorrectable_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intreg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_intr_interrupt")) { axi_intr_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "axi_wkup_interrupt")) { axi_wkup_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_correctable_interrupt")) { sram1_ecc_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram1_ecc_uncorrectable_interrupt")) { sram1_ecc_uncorrectable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_correctable_interrupt")) { sram2_ecc_correctable_interrupt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sram2_ecc_uncorrectable_interrupt")) { sram2_ecc_uncorrectable_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intgrp_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = intreg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_test_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_set.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_clear.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intreg_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_interrupt")) { axi_interrupt(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_int_groups_int_enable_rw_reg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "axi_enable")) { axi_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_intgrp_status_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = intreg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_enable_rw_reg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_rw_reg.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_csr_intr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "dowstream")) { dowstream(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "dowstream_enable")) { dowstream_enable(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_em_cfg_core_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "corecfg_baseclkfreq")) { corecfg_baseclkfreq(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_timeoutclkfreq")) { corecfg_timeoutclkfreq(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxblklength")) { corecfg_maxblklength(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_8bitsupport")) { corecfg_8bitsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_adma2support")) { corecfg_adma2support(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_highspeedsupport")) { corecfg_highspeedsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdmasupport")) { corecfg_sdmasupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_suspressupport")) { corecfg_suspressupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_3p3voltsupport")) { corecfg_3p3voltsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_3p0voltsupport")) { corecfg_3p0voltsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_1p8voltsupport")) { corecfg_1p8voltsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_64bitsupport")) { corecfg_64bitsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_asynchintrsupport")) { corecfg_asynchintrsupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_slottype")) { corecfg_slottype(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr50support")) { corecfg_sdr50support(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr104support")) { corecfg_sdr104support(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_ddr50support")) { corecfg_ddr50support(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_hs400support")) { corecfg_hs400support(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_adriversupport")) { corecfg_adriversupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_cdriversupport")) { corecfg_cdriversupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_ddriversupport")) { corecfg_ddriversupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_type4support")) { corecfg_type4support(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_retuningtimercnt")) { corecfg_retuningtimercnt(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_tuningforsdr50")) { corecfg_tuningforsdr50(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_retuningmodes")) { corecfg_retuningmodes(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_clockmultiplier")) { corecfg_clockmultiplier(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_spisupport")) { corecfg_spisupport(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_spiblkmode")) { corecfg_spiblkmode(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_initpresetval")) { corecfg_initpresetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_dspdpresetval")) { corecfg_dspdpresetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_hspdpresetval")) { corecfg_hspdpresetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr12presetval")) { corecfg_sdr12presetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr25presetval")) { corecfg_sdr25presetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr50presetval")) { corecfg_sdr50presetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_sdr104presetval")) { corecfg_sdr104presetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_ddr50presetval")) { corecfg_ddr50presetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_hs400presetval")) { corecfg_hs400presetval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxcurrent1p8v")) { corecfg_maxcurrent1p8v(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxcurrent3p0v")) { corecfg_maxcurrent3p0v(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_maxcurrent3p3v")) { corecfg_maxcurrent3p3v(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_cqfval")) { corecfg_cqfval(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_cqfmul")) { corecfg_cqfmul(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "corecfg_timeoutclkunit")) { corecfg_timeoutclkunit(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_em_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "debug_port_enable")) { debug_port_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_port_select")) { debug_port_select(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mstid")) { mstid(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_cfg_ecc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "disable_det_SRAM1")) { disable_det_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "disable_cor_SRAM2")) { disable_cor_SRAM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "disable_det_SRAM2")) { disable_det_SRAM2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_sta_ecc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "correctable_SRAM1")) { correctable_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_SRAM1")) { uncorrectable_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "correctable_SRAM2")) { correctable_SRAM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "uncorrectable_SRAM2")) { uncorrectable_SRAM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_SRAM1")) { addr_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_SRAM2")) { addr_SRAM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome_SRAM1")) { syndrome_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "syndrome_SRAM2")) { syndrome_SRAM2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_sta_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "done_fail_SRAM1")) { done_fail_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_fail_SRAM2")) { done_fail_SRAM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_pass_SRAM1")) { done_pass_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "done_pass_SRAM2")) { done_pass_SRAM2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_cfg_bist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "run_SRAM1")) { run_SRAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "run_SRAM2")) { run_SRAM2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_em_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_bist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_ecc.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ecc.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = em_cfg_core.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = csr_intr.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = emm.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = int_groups.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = axi.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_axi_int_enable_clear_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi_intr_enable");
    ret_vec.push_back("axi_wkup_enable");
    ret_vec.push_back("sram1_ecc_correctable_enable");
    ret_vec.push_back("sram1_ecc_uncorrectable_enable");
    ret_vec.push_back("sram2_ecc_correctable_enable");
    ret_vec.push_back("sram2_ecc_uncorrectable_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_intreg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi_intr_interrupt");
    ret_vec.push_back("axi_wkup_interrupt");
    ret_vec.push_back("sram1_ecc_correctable_interrupt");
    ret_vec.push_back("sram1_ecc_uncorrectable_interrupt");
    ret_vec.push_back("sram2_ecc_correctable_interrupt");
    ret_vec.push_back("sram2_ecc_uncorrectable_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_intgrp_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : intreg.get_fields(level-1)) {
            ret_vec.push_back("intreg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_test_set.get_fields(level-1)) {
            ret_vec.push_back("int_test_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_set.get_fields(level-1)) {
            ret_vec.push_back("int_enable_set." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_clear.get_fields(level-1)) {
            ret_vec.push_back("int_enable_clear." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_intreg_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi_interrupt");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_int_groups_int_enable_rw_reg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("axi_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_intgrp_status_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : intreg.get_fields(level-1)) {
            ret_vec.push_back("intreg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_enable_rw_reg.get_fields(level-1)) {
            ret_vec.push_back("int_enable_rw_reg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_rw_reg.get_fields(level-1)) {
            ret_vec.push_back("int_rw_reg." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_csr_intr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("dowstream");
    ret_vec.push_back("dowstream_enable");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_em_cfg_core_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("corecfg_baseclkfreq");
    ret_vec.push_back("corecfg_timeoutclkfreq");
    ret_vec.push_back("corecfg_maxblklength");
    ret_vec.push_back("corecfg_8bitsupport");
    ret_vec.push_back("corecfg_adma2support");
    ret_vec.push_back("corecfg_highspeedsupport");
    ret_vec.push_back("corecfg_sdmasupport");
    ret_vec.push_back("corecfg_suspressupport");
    ret_vec.push_back("corecfg_3p3voltsupport");
    ret_vec.push_back("corecfg_3p0voltsupport");
    ret_vec.push_back("corecfg_1p8voltsupport");
    ret_vec.push_back("corecfg_64bitsupport");
    ret_vec.push_back("corecfg_asynchintrsupport");
    ret_vec.push_back("corecfg_slottype");
    ret_vec.push_back("corecfg_sdr50support");
    ret_vec.push_back("corecfg_sdr104support");
    ret_vec.push_back("corecfg_ddr50support");
    ret_vec.push_back("corecfg_hs400support");
    ret_vec.push_back("corecfg_adriversupport");
    ret_vec.push_back("corecfg_cdriversupport");
    ret_vec.push_back("corecfg_ddriversupport");
    ret_vec.push_back("corecfg_type4support");
    ret_vec.push_back("corecfg_retuningtimercnt");
    ret_vec.push_back("corecfg_tuningforsdr50");
    ret_vec.push_back("corecfg_retuningmodes");
    ret_vec.push_back("corecfg_clockmultiplier");
    ret_vec.push_back("corecfg_spisupport");
    ret_vec.push_back("corecfg_spiblkmode");
    ret_vec.push_back("corecfg_initpresetval");
    ret_vec.push_back("corecfg_dspdpresetval");
    ret_vec.push_back("corecfg_hspdpresetval");
    ret_vec.push_back("corecfg_sdr12presetval");
    ret_vec.push_back("corecfg_sdr25presetval");
    ret_vec.push_back("corecfg_sdr50presetval");
    ret_vec.push_back("corecfg_sdr104presetval");
    ret_vec.push_back("corecfg_ddr50presetval");
    ret_vec.push_back("corecfg_hs400presetval");
    ret_vec.push_back("corecfg_maxcurrent1p8v");
    ret_vec.push_back("corecfg_maxcurrent3p0v");
    ret_vec.push_back("corecfg_maxcurrent3p3v");
    ret_vec.push_back("corecfg_cqfval");
    ret_vec.push_back("corecfg_cqfmul");
    ret_vec.push_back("corecfg_timeoutclkunit");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_em_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("debug_port_enable");
    ret_vec.push_back("debug_port_select");
    ret_vec.push_back("mstid");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_cfg_ecc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("disable_det_SRAM1");
    ret_vec.push_back("disable_cor_SRAM2");
    ret_vec.push_back("disable_det_SRAM2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_sta_ecc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("correctable_SRAM1");
    ret_vec.push_back("uncorrectable_SRAM1");
    ret_vec.push_back("correctable_SRAM2");
    ret_vec.push_back("uncorrectable_SRAM2");
    ret_vec.push_back("addr_SRAM1");
    ret_vec.push_back("addr_SRAM2");
    ret_vec.push_back("syndrome_SRAM1");
    ret_vec.push_back("syndrome_SRAM2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_sta_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("done_fail_SRAM1");
    ret_vec.push_back("done_fail_SRAM2");
    ret_vec.push_back("done_pass_SRAM1");
    ret_vec.push_back("done_pass_SRAM2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_cfg_bist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("run_SRAM1");
    ret_vec.push_back("run_SRAM2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_em_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_bist.get_fields(level-1)) {
            ret_vec.push_back("cfg_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_bist.get_fields(level-1)) {
            ret_vec.push_back("sta_bist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_ecc.get_fields(level-1)) {
            ret_vec.push_back("sta_ecc." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_ecc.get_fields(level-1)) {
            ret_vec.push_back("cfg_ecc." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : em_cfg.get_fields(level-1)) {
            ret_vec.push_back("em_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : em_cfg_core.get_fields(level-1)) {
            ret_vec.push_back("em_cfg_core." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : csr_intr.get_fields(level-1)) {
            ret_vec.push_back("csr_intr." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : emm.get_fields(level-1)) {
            ret_vec.push_back("emm." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : int_groups.get_fields(level-1)) {
            ret_vec.push_back("int_groups." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : axi.get_fields(level-1)) {
            ret_vec.push_back("axi." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
