
#include "cap_ap_csr.h"
#include "LogMsg.h"        
using namespace std;
        
cap_ap_csr_ap_cfg_t::cap_ap_csr_ap_cfg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_ap_cfg_t::~cap_ap_csr_ap_cfg_t() { }

cap_ap_csr_cfg_ACP_t::cap_ap_csr_cfg_ACP_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_ACP_t::~cap_ap_csr_cfg_ACP_t() { }

cap_ap_csr_cfg_nts_t::cap_ap_csr_cfg_nts_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_nts_t::~cap_ap_csr_cfg_nts_t() { }

cap_ap_csr_sta_nts_t::cap_ap_csr_sta_nts_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_nts_t::~cap_ap_csr_sta_nts_t() { }

cap_ap_csr_sta_dstream_t::cap_ap_csr_sta_dstream_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_dstream_t::~cap_ap_csr_sta_dstream_t() { }

cap_ap_csr_cfg_dstream_t::cap_ap_csr_cfg_dstream_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_dstream_t::~cap_ap_csr_cfg_dstream_t() { }

cap_ap_csr_cfg_misc_dbg_t::cap_ap_csr_cfg_misc_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_misc_dbg_t::~cap_ap_csr_cfg_misc_dbg_t() { }

cap_ap_csr_sta_misc_dbg_t::cap_ap_csr_sta_misc_dbg_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_misc_dbg_t::~cap_ap_csr_sta_misc_dbg_t() { }

cap_ap_csr_sta_VSIG_t::cap_ap_csr_sta_VSIG_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_VSIG_t::~cap_ap_csr_sta_VSIG_t() { }

cap_ap_csr_cfg_VSIG_t::cap_ap_csr_cfg_VSIG_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_VSIG_t::~cap_ap_csr_cfg_VSIG_t() { }

cap_ap_csr_cfg_AUTH_t::cap_ap_csr_cfg_AUTH_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_AUTH_t::~cap_ap_csr_cfg_AUTH_t() { }

cap_ap_csr_sta_CTI_t::cap_ap_csr_sta_CTI_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_CTI_t::~cap_ap_csr_sta_CTI_t() { }

cap_ap_csr_cfg_CTI_t::cap_ap_csr_cfg_CTI_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_CTI_t::~cap_ap_csr_cfg_CTI_t() { }

cap_ap_csr_sta_misc_t::cap_ap_csr_sta_misc_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_misc_t::~cap_ap_csr_sta_misc_t() { }

cap_ap_csr_sta_IRQ_t::cap_ap_csr_sta_IRQ_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_IRQ_t::~cap_ap_csr_sta_IRQ_t() { }

cap_ap_csr_sta_L2_t::cap_ap_csr_sta_L2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_L2_t::~cap_ap_csr_sta_L2_t() { }

cap_ap_csr_cfg_L2_t::cap_ap_csr_cfg_L2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_L2_t::~cap_ap_csr_cfg_L2_t() { }

cap_ap_csr_sta_EV_t::cap_ap_csr_sta_EV_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_EV_t::~cap_ap_csr_sta_EV_t() { }

cap_ap_csr_cfg_EV_t::cap_ap_csr_cfg_EV_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_EV_t::~cap_ap_csr_cfg_EV_t() { }

cap_ap_csr_cfg_pmu_t::cap_ap_csr_cfg_pmu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_pmu_t::~cap_ap_csr_cfg_pmu_t() { }

cap_ap_csr_sta_pmu_t::cap_ap_csr_sta_pmu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_pmu_t::~cap_ap_csr_sta_pmu_t() { }

cap_ap_csr_sta_timer_t::cap_ap_csr_sta_timer_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_timer_t::~cap_ap_csr_sta_timer_t() { }

cap_ap_csr_cfg_AT_t::cap_ap_csr_cfg_AT_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_AT_t::~cap_ap_csr_cfg_AT_t() { }

cap_ap_csr_sta_AT_t::cap_ap_csr_sta_AT_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_AT_t::~cap_ap_csr_sta_AT_t() { }

cap_ap_csr_sta_CORE_BIST_t::cap_ap_csr_sta_CORE_BIST_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_CORE_BIST_t::~cap_ap_csr_sta_CORE_BIST_t() { }

cap_ap_csr_cfg_CORE_BIST_t::cap_ap_csr_cfg_CORE_BIST_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_CORE_BIST_t::~cap_ap_csr_cfg_CORE_BIST_t() { }

cap_ap_csr_sta_apms_t::cap_ap_csr_sta_apms_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_apms_t::~cap_ap_csr_sta_apms_t() { }

cap_ap_csr_sta_gic_mbist_t::cap_ap_csr_sta_gic_mbist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_sta_gic_mbist_t::~cap_ap_csr_sta_gic_mbist_t() { }

cap_ap_csr_cfg_gic_t::cap_ap_csr_cfg_gic_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_gic_t::~cap_ap_csr_cfg_gic_t() { }

cap_ap_csr_cfg_gic_mbist_t::cap_ap_csr_cfg_gic_mbist_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_gic_mbist_t::~cap_ap_csr_cfg_gic_mbist_t() { }

cap_ap_csr_cfg_irq_t::cap_ap_csr_cfg_irq_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_irq_t::~cap_ap_csr_cfg_irq_t() { }

cap_ap_csr_cfg_apar_t::cap_ap_csr_cfg_apar_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_apar_t::~cap_ap_csr_cfg_apar_t() { }

cap_ap_csr_cfg_cpu_t::cap_ap_csr_cfg_cpu_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_cpu_t::~cap_ap_csr_cfg_cpu_t() { }

cap_ap_csr_cfg_cpu3_t::cap_ap_csr_cfg_cpu3_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_cpu3_t::~cap_ap_csr_cfg_cpu3_t() { }

cap_ap_csr_cfg_cpu2_t::cap_ap_csr_cfg_cpu2_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_cpu2_t::~cap_ap_csr_cfg_cpu2_t() { }

cap_ap_csr_cfg_cpu1_t::cap_ap_csr_cfg_cpu1_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_cpu1_t::~cap_ap_csr_cfg_cpu1_t() { }

cap_ap_csr_cfg_cpu0_t::cap_ap_csr_cfg_cpu0_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_cpu0_t::~cap_ap_csr_cfg_cpu0_t() { }

cap_ap_csr_cfg_cpu0_flash_t::cap_ap_csr_cfg_cpu0_flash_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_cpu0_flash_t::~cap_ap_csr_cfg_cpu0_flash_t() { }

cap_ap_csr_cfg_static_t::cap_ap_csr_cfg_static_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_cfg_static_t::~cap_ap_csr_cfg_static_t() { }

cap_ap_csr_dummy_t::cap_ap_csr_dummy_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_dummy_t::~cap_ap_csr_dummy_t() { }

cap_ap_csr_base_t::cap_ap_csr_base_t(string name, cap_csr_base * parent): 
    cap_register_base(name, parent)  { 
        //init();
        }
cap_ap_csr_base_t::~cap_ap_csr_base_t() { }

cap_ap_csr_t::cap_ap_csr_t(string name, cap_csr_base * parent): 
    cap_block_base(name, parent)  { 
        set_byte_size(4194304);
        set_attributes(0,get_name(), 0);
        }
cap_ap_csr_t::~cap_ap_csr_t() { }

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_ap_cfg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".debug_port_enable: 0x" << int_var__debug_port_enable << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".debug_port_select: 0x" << int_var__debug_port_select << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".cfg_stall_on_same_id: 0x" << int_var__cfg_stall_on_same_id << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_ACP_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".AINACTS: 0x" << int_var__AINACTS << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ACINACTM: 0x" << int_var__ACINACTM << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_nts_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".TSBITS_TS: 0x" << int_var__TSBITS_TS << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".TSSYNCS_TS: 0x" << int_var__TSSYNCS_TS << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_nts_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".TSSYNCREADYS_TS: 0x" << int_var__TSSYNCREADYS_TS << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_dstream_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".DBGACK: 0x" << int_var__DBGACK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".DBGRSTREQ: 0x" << int_var__DBGRSTREQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_dstream_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".EDBGRQ: 0x" << int_var__EDBGRQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_misc_dbg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".DBGROMADDRV: 0x" << int_var__DBGROMADDRV << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".DBGROMADDR: 0x" << int_var__DBGROMADDR << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_misc_dbg_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".COMMRX: 0x" << int_var__COMMRX << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".COMMTX: 0x" << int_var__COMMTX << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nCOMMIRQ: 0x" << int_var__nCOMMIRQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_VSIG_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".CLREXMONACK: 0x" << int_var__CLREXMONACK << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_VSIG_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".CLREXMONREQ: 0x" << int_var__CLREXMONREQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_AUTH_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".DBGEN: 0x" << int_var__DBGEN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SPNIDEN: 0x" << int_var__SPNIDEN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SPIDEN: 0x" << int_var__SPIDEN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".NIDEN: 0x" << int_var__NIDEN << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_CTI_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".CTICHOUT: 0x" << int_var__CTICHOUT << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CTICHINACK: 0x" << int_var__CTICHINACK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CTIIRQ: 0x" << int_var__CTIIRQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_CTI_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".CTICHIN: 0x" << int_var__CTICHIN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CTICHOUTACK: 0x" << int_var__CTICHOUTACK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CTIIRQACK: 0x" << int_var__CTIIRQACK << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_misc_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".WARMRSTREQ: 0x" << int_var__WARMRSTREQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".WRMEMATTRM: 0x" << int_var__WRMEMATTRM << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".RDMEMATTRM: 0x" << int_var__RDMEMATTRM << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SMPEN: 0x" << int_var__SMPEN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".STANDBYWFE: 0x" << int_var__STANDBYWFE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".STANDBYWFI: 0x" << int_var__STANDBYWFI << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".STANDBYWFIL2: 0x" << int_var__STANDBYWFIL2 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_IRQ_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".nEXTERRIRQ: 0x" << int_var__nEXTERRIRQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nINTERRIRQ: 0x" << int_var__nINTERRIRQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nVCPUMNTIRQ: 0x" << int_var__nVCPUMNTIRQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_L2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".L2FLUSHDONE: 0x" << int_var__L2FLUSHDONE << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_L2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".L2FLUSHREQ: 0x" << int_var__L2FLUSHREQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_EV_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".EVENTO: 0x" << int_var__EVENTO << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_EV_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".EVENTI: 0x" << int_var__EVENTI << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_pmu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".PMUSNAPSHOTREQ: 0x" << int_var__PMUSNAPSHOTREQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_pmu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".PMUEVENT0: 0x" << int_var__PMUEVENT0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PMUEVENT1: 0x" << int_var__PMUEVENT1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PMUEVENT2: 0x" << int_var__PMUEVENT2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PMUEVENT3: 0x" << int_var__PMUEVENT3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nPMUIRQ: 0x" << int_var__nPMUIRQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PMUSNAPSHOTACK: 0x" << int_var__PMUSNAPSHOTACK << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_timer_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".nCNTHPIRQ: 0x" << int_var__nCNTHPIRQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nCNTPNSIRQ: 0x" << int_var__nCNTPNSIRQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nCNTPSIRQ: 0x" << int_var__nCNTPSIRQ << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nCNTVIRQ: 0x" << int_var__nCNTVIRQ << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_AT_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".AFVALIDM0: 0x" << int_var__AFVALIDM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AFVALIDM1: 0x" << int_var__AFVALIDM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AFVALIDM2: 0x" << int_var__AFVALIDM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AFVALIDM3: 0x" << int_var__AFVALIDM3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATREADYM0: 0x" << int_var__ATREADYM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATREADYM1: 0x" << int_var__ATREADYM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATREADYM2: 0x" << int_var__ATREADYM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATREADYM3: 0x" << int_var__ATREADYM3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SYNCREQM0: 0x" << int_var__SYNCREQM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SYNCREQM1: 0x" << int_var__SYNCREQM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SYNCREQM2: 0x" << int_var__SYNCREQM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SYNCREQM3: 0x" << int_var__SYNCREQM3 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_AT_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".AFREADYM0: 0x" << int_var__AFREADYM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AFREADYM1: 0x" << int_var__AFREADYM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AFREADYM2: 0x" << int_var__AFREADYM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AFREADYM3: 0x" << int_var__AFREADYM3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATBYTESM0: 0x" << int_var__ATBYTESM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATBYTESM1: 0x" << int_var__ATBYTESM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATBYTESM2: 0x" << int_var__ATBYTESM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATBYTESM3: 0x" << int_var__ATBYTESM3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATDATAM0: 0x" << int_var__ATDATAM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATDATAM1: 0x" << int_var__ATDATAM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATDATAM2: 0x" << int_var__ATDATAM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATDATAM3: 0x" << int_var__ATDATAM3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATIDM0: 0x" << int_var__ATIDM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATIDM1: 0x" << int_var__ATIDM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATIDM2: 0x" << int_var__ATIDM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATIDM3: 0x" << int_var__ATIDM3 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATVALIDM0: 0x" << int_var__ATVALIDM0 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATVALIDM1: 0x" << int_var__ATVALIDM1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATVALIDM2: 0x" << int_var__ATVALIDM2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".ATVALIDM3: 0x" << int_var__ATVALIDM3 << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_CORE_BIST_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".DONE_FAIL_OUT: 0x" << int_var__DONE_FAIL_OUT << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".DONE_PASS_OUT: 0x" << int_var__DONE_PASS_OUT << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_CORE_BIST_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".PTRN_FILL: 0x" << int_var__PTRN_FILL << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".RPR_MODE: 0x" << int_var__RPR_MODE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".RUN: 0x" << int_var__RUN << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SEL_CLK: 0x" << int_var__SEL_CLK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SEL_L2DCLK: 0x" << int_var__SEL_L2DCLK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SEL_L2TCLK: 0x" << int_var__SEL_L2TCLK << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".MEM_RST: 0x" << int_var__MEM_RST << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".RESET_OVERRIDE: 0x" << int_var__RESET_OVERRIDE << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_apms_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".fatal: 0x" << int_var__fatal << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_sta_gic_mbist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mbistack: 0x" << int_var__mbistack << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistoutdata: 0x" << int_var__mbistoutdata << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_gic_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".base_awuser: 0x" << int_var__base_awuser << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".base_aruser: 0x" << int_var__base_aruser << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".awuser: 0x" << int_var__awuser << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_gic_mbist_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".mbistaddr: 0x" << int_var__mbistaddr << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistarray: 0x" << int_var__mbistarray << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistcfg: 0x" << int_var__mbistcfg << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistreaden: 0x" << int_var__mbistreaden << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistreq: 0x" << int_var__mbistreq << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistresetn: 0x" << int_var__mbistresetn << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistwriteen: 0x" << int_var__mbistwriteen << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".mbistindata: 0x" << int_var__mbistindata << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_irq_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".nFIQ_mask: 0x" << int_var__nFIQ_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nIRQ_mask: 0x" << int_var__nIRQ_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nREI_mask: 0x" << int_var__nREI_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nSEI_mask: 0x" << int_var__nSEI_mask << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".nVSEI_mask: 0x" << int_var__nVSEI_mask << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_apar_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".addr_top24: 0x" << int_var__addr_top24 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".awusers: 0x" << int_var__awusers << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".arusers: 0x" << int_var__arusers << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".addr_sel: 0x" << int_var__addr_sel << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_cpu_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".active: 0x" << int_var__active << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".sta_wake_request: 0x" << int_var__sta_wake_request << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_cpu3_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".RVBARADDR: 0x" << int_var__RVBARADDR << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_cpu2_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".RVBARADDR: 0x" << int_var__RVBARADDR << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_cpu1_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".RVBARADDR: 0x" << int_var__RVBARADDR << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_cpu0_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".RVBARADDR: 0x" << int_var__RVBARADDR << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_cpu0_flash_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".RVBARADDR: 0x" << int_var__RVBARADDR << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_cfg_static_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".DBGL1RSTDISABLE: 0x" << int_var__DBGL1RSTDISABLE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".L2RSTDISABLE: 0x" << int_var__L2RSTDISABLE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CP15SDISABLE: 0x" << int_var__CP15SDISABLE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CLUSTERIDAFF1: 0x" << int_var__CLUSTERIDAFF1 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CLUSTERIDAFF2: 0x" << int_var__CLUSTERIDAFF2 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CRYPTODISABLE: 0x" << int_var__CRYPTODISABLE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".AA64nAA32: 0x" << int_var__AA64nAA32 << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".SYSBARDISABLE: 0x" << int_var__SYSBARDISABLE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".BROADCASTCACHEMAINT: 0x" << int_var__BROADCASTCACHEMAINT << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".BROADCASTINNER: 0x" << int_var__BROADCASTINNER << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".BROADCASTOUTER: 0x" << int_var__BROADCASTOUTER << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CFGEND: 0x" << int_var__CFGEND << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".CFGTE: 0x" << int_var__CFGTE << dec << endl);
    PLOG_MSG(hex << string(get_hier_path()) << ".PERIPHBASE: 0x" << int_var__PERIPHBASE << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_dummy_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".cycle_counter: 0x" << int_var__cycle_counter << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_base_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    PLOG_MSG(hex << string(get_hier_path()) << ".scratch_reg: 0x" << int_var__scratch_reg << dec << endl);
}
#endif // CSR_NO_SHOW_IMPL

#ifndef CSR_NO_SHOW_IMPL 
void cap_ap_csr_t::show() {
    PLOG_MSG(get_hier_path() << "(all) : 0x" << hex << all() << dec << endl);
    if(all() == 0) return;

    base.show();
    dummy.show();
    cfg_static.show();
    cfg_cpu0_flash.show();
    cfg_cpu0.show();
    cfg_cpu1.show();
    cfg_cpu2.show();
    cfg_cpu3.show();
    cfg_cpu.show();
    cfg_apar.show();
    cfg_irq.show();
    cfg_gic_mbist.show();
    cfg_gic.show();
    sta_gic_mbist.show();
    sta_apms.show();
    cfg_CORE_BIST.show();
    sta_CORE_BIST.show();
    sta_AT.show();
    cfg_AT.show();
    sta_timer.show();
    sta_pmu.show();
    cfg_pmu.show();
    cfg_EV.show();
    sta_EV.show();
    cfg_L2.show();
    sta_L2.show();
    sta_IRQ.show();
    sta_misc.show();
    cfg_CTI.show();
    sta_CTI.show();
    cfg_AUTH.show();
    cfg_VSIG.show();
    sta_VSIG.show();
    sta_misc_dbg.show();
    cfg_misc_dbg.show();
    cfg_dstream.show();
    sta_dstream.show();
    sta_nts.show();
    cfg_nts.show();
    cfg_ACP.show();
    ap_cfg.show();
    apb.show();
}
#endif // CSR_NO_SHOW_IMPL

int cap_ap_csr_ap_cfg_t::get_width() const {
    return cap_ap_csr_ap_cfg_t::s_get_width();

}

int cap_ap_csr_cfg_ACP_t::get_width() const {
    return cap_ap_csr_cfg_ACP_t::s_get_width();

}

int cap_ap_csr_cfg_nts_t::get_width() const {
    return cap_ap_csr_cfg_nts_t::s_get_width();

}

int cap_ap_csr_sta_nts_t::get_width() const {
    return cap_ap_csr_sta_nts_t::s_get_width();

}

int cap_ap_csr_sta_dstream_t::get_width() const {
    return cap_ap_csr_sta_dstream_t::s_get_width();

}

int cap_ap_csr_cfg_dstream_t::get_width() const {
    return cap_ap_csr_cfg_dstream_t::s_get_width();

}

int cap_ap_csr_cfg_misc_dbg_t::get_width() const {
    return cap_ap_csr_cfg_misc_dbg_t::s_get_width();

}

int cap_ap_csr_sta_misc_dbg_t::get_width() const {
    return cap_ap_csr_sta_misc_dbg_t::s_get_width();

}

int cap_ap_csr_sta_VSIG_t::get_width() const {
    return cap_ap_csr_sta_VSIG_t::s_get_width();

}

int cap_ap_csr_cfg_VSIG_t::get_width() const {
    return cap_ap_csr_cfg_VSIG_t::s_get_width();

}

int cap_ap_csr_cfg_AUTH_t::get_width() const {
    return cap_ap_csr_cfg_AUTH_t::s_get_width();

}

int cap_ap_csr_sta_CTI_t::get_width() const {
    return cap_ap_csr_sta_CTI_t::s_get_width();

}

int cap_ap_csr_cfg_CTI_t::get_width() const {
    return cap_ap_csr_cfg_CTI_t::s_get_width();

}

int cap_ap_csr_sta_misc_t::get_width() const {
    return cap_ap_csr_sta_misc_t::s_get_width();

}

int cap_ap_csr_sta_IRQ_t::get_width() const {
    return cap_ap_csr_sta_IRQ_t::s_get_width();

}

int cap_ap_csr_sta_L2_t::get_width() const {
    return cap_ap_csr_sta_L2_t::s_get_width();

}

int cap_ap_csr_cfg_L2_t::get_width() const {
    return cap_ap_csr_cfg_L2_t::s_get_width();

}

int cap_ap_csr_sta_EV_t::get_width() const {
    return cap_ap_csr_sta_EV_t::s_get_width();

}

int cap_ap_csr_cfg_EV_t::get_width() const {
    return cap_ap_csr_cfg_EV_t::s_get_width();

}

int cap_ap_csr_cfg_pmu_t::get_width() const {
    return cap_ap_csr_cfg_pmu_t::s_get_width();

}

int cap_ap_csr_sta_pmu_t::get_width() const {
    return cap_ap_csr_sta_pmu_t::s_get_width();

}

int cap_ap_csr_sta_timer_t::get_width() const {
    return cap_ap_csr_sta_timer_t::s_get_width();

}

int cap_ap_csr_cfg_AT_t::get_width() const {
    return cap_ap_csr_cfg_AT_t::s_get_width();

}

int cap_ap_csr_sta_AT_t::get_width() const {
    return cap_ap_csr_sta_AT_t::s_get_width();

}

int cap_ap_csr_sta_CORE_BIST_t::get_width() const {
    return cap_ap_csr_sta_CORE_BIST_t::s_get_width();

}

int cap_ap_csr_cfg_CORE_BIST_t::get_width() const {
    return cap_ap_csr_cfg_CORE_BIST_t::s_get_width();

}

int cap_ap_csr_sta_apms_t::get_width() const {
    return cap_ap_csr_sta_apms_t::s_get_width();

}

int cap_ap_csr_sta_gic_mbist_t::get_width() const {
    return cap_ap_csr_sta_gic_mbist_t::s_get_width();

}

int cap_ap_csr_cfg_gic_t::get_width() const {
    return cap_ap_csr_cfg_gic_t::s_get_width();

}

int cap_ap_csr_cfg_gic_mbist_t::get_width() const {
    return cap_ap_csr_cfg_gic_mbist_t::s_get_width();

}

int cap_ap_csr_cfg_irq_t::get_width() const {
    return cap_ap_csr_cfg_irq_t::s_get_width();

}

int cap_ap_csr_cfg_apar_t::get_width() const {
    return cap_ap_csr_cfg_apar_t::s_get_width();

}

int cap_ap_csr_cfg_cpu_t::get_width() const {
    return cap_ap_csr_cfg_cpu_t::s_get_width();

}

int cap_ap_csr_cfg_cpu3_t::get_width() const {
    return cap_ap_csr_cfg_cpu3_t::s_get_width();

}

int cap_ap_csr_cfg_cpu2_t::get_width() const {
    return cap_ap_csr_cfg_cpu2_t::s_get_width();

}

int cap_ap_csr_cfg_cpu1_t::get_width() const {
    return cap_ap_csr_cfg_cpu1_t::s_get_width();

}

int cap_ap_csr_cfg_cpu0_t::get_width() const {
    return cap_ap_csr_cfg_cpu0_t::s_get_width();

}

int cap_ap_csr_cfg_cpu0_flash_t::get_width() const {
    return cap_ap_csr_cfg_cpu0_flash_t::s_get_width();

}

int cap_ap_csr_cfg_static_t::get_width() const {
    return cap_ap_csr_cfg_static_t::s_get_width();

}

int cap_ap_csr_dummy_t::get_width() const {
    return cap_ap_csr_dummy_t::s_get_width();

}

int cap_ap_csr_base_t::get_width() const {
    return cap_ap_csr_base_t::s_get_width();

}

int cap_ap_csr_t::get_width() const {
    return cap_ap_csr_t::s_get_width();

}

int cap_ap_csr_ap_cfg_t::s_get_width() {
    int _count = 0;

    _count += 1; // debug_port_enable
    _count += 1; // debug_port_select
    _count += 1; // cfg_stall_on_same_id
    return _count;
}

int cap_ap_csr_cfg_ACP_t::s_get_width() {
    int _count = 0;

    _count += 1; // AINACTS
    _count += 1; // ACINACTM
    return _count;
}

int cap_ap_csr_cfg_nts_t::s_get_width() {
    int _count = 0;

    _count += 7; // TSBITS_TS
    _count += 2; // TSSYNCS_TS
    return _count;
}

int cap_ap_csr_sta_nts_t::s_get_width() {
    int _count = 0;

    _count += 1; // TSSYNCREADYS_TS
    return _count;
}

int cap_ap_csr_sta_dstream_t::s_get_width() {
    int _count = 0;

    _count += 4; // DBGACK
    _count += 4; // DBGRSTREQ
    return _count;
}

int cap_ap_csr_cfg_dstream_t::s_get_width() {
    int _count = 0;

    _count += 4; // EDBGRQ
    return _count;
}

int cap_ap_csr_cfg_misc_dbg_t::s_get_width() {
    int _count = 0;

    _count += 1; // DBGROMADDRV
    _count += 32; // DBGROMADDR
    return _count;
}

int cap_ap_csr_sta_misc_dbg_t::s_get_width() {
    int _count = 0;

    _count += 4; // COMMRX
    _count += 4; // COMMTX
    _count += 4; // nCOMMIRQ
    return _count;
}

int cap_ap_csr_sta_VSIG_t::s_get_width() {
    int _count = 0;

    _count += 1; // CLREXMONACK
    return _count;
}

int cap_ap_csr_cfg_VSIG_t::s_get_width() {
    int _count = 0;

    _count += 1; // CLREXMONREQ
    return _count;
}

int cap_ap_csr_cfg_AUTH_t::s_get_width() {
    int _count = 0;

    _count += 4; // DBGEN
    _count += 4; // SPNIDEN
    _count += 4; // SPIDEN
    _count += 4; // NIDEN
    return _count;
}

int cap_ap_csr_sta_CTI_t::s_get_width() {
    int _count = 0;

    _count += 4; // CTICHOUT
    _count += 4; // CTICHINACK
    _count += 4; // CTIIRQ
    return _count;
}

int cap_ap_csr_cfg_CTI_t::s_get_width() {
    int _count = 0;

    _count += 4; // CTICHIN
    _count += 4; // CTICHOUTACK
    _count += 4; // CTIIRQACK
    return _count;
}

int cap_ap_csr_sta_misc_t::s_get_width() {
    int _count = 0;

    _count += 4; // WARMRSTREQ
    _count += 8; // WRMEMATTRM
    _count += 8; // RDMEMATTRM
    _count += 4; // SMPEN
    _count += 4; // STANDBYWFE
    _count += 4; // STANDBYWFI
    _count += 1; // STANDBYWFIL2
    return _count;
}

int cap_ap_csr_sta_IRQ_t::s_get_width() {
    int _count = 0;

    _count += 1; // nEXTERRIRQ
    _count += 1; // nINTERRIRQ
    _count += 4; // nVCPUMNTIRQ
    return _count;
}

int cap_ap_csr_sta_L2_t::s_get_width() {
    int _count = 0;

    _count += 1; // L2FLUSHDONE
    return _count;
}

int cap_ap_csr_cfg_L2_t::s_get_width() {
    int _count = 0;

    _count += 1; // L2FLUSHREQ
    return _count;
}

int cap_ap_csr_sta_EV_t::s_get_width() {
    int _count = 0;

    _count += 1; // EVENTO
    return _count;
}

int cap_ap_csr_cfg_EV_t::s_get_width() {
    int _count = 0;

    _count += 1; // EVENTI
    return _count;
}

int cap_ap_csr_cfg_pmu_t::s_get_width() {
    int _count = 0;

    _count += 4; // PMUSNAPSHOTREQ
    return _count;
}

int cap_ap_csr_sta_pmu_t::s_get_width() {
    int _count = 0;

    _count += 25; // PMUEVENT0
    _count += 25; // PMUEVENT1
    _count += 25; // PMUEVENT2
    _count += 25; // PMUEVENT3
    _count += 4; // nPMUIRQ
    _count += 4; // PMUSNAPSHOTACK
    return _count;
}

int cap_ap_csr_sta_timer_t::s_get_width() {
    int _count = 0;

    _count += 4; // nCNTHPIRQ
    _count += 4; // nCNTPNSIRQ
    _count += 4; // nCNTPSIRQ
    _count += 4; // nCNTVIRQ
    return _count;
}

int cap_ap_csr_cfg_AT_t::s_get_width() {
    int _count = 0;

    _count += 1; // AFVALIDM0
    _count += 1; // AFVALIDM1
    _count += 1; // AFVALIDM2
    _count += 1; // AFVALIDM3
    _count += 1; // ATREADYM0
    _count += 1; // ATREADYM1
    _count += 1; // ATREADYM2
    _count += 1; // ATREADYM3
    _count += 1; // SYNCREQM0
    _count += 1; // SYNCREQM1
    _count += 1; // SYNCREQM2
    _count += 1; // SYNCREQM3
    return _count;
}

int cap_ap_csr_sta_AT_t::s_get_width() {
    int _count = 0;

    _count += 1; // AFREADYM0
    _count += 1; // AFREADYM1
    _count += 1; // AFREADYM2
    _count += 1; // AFREADYM3
    _count += 2; // ATBYTESM0
    _count += 2; // ATBYTESM1
    _count += 2; // ATBYTESM2
    _count += 2; // ATBYTESM3
    _count += 32; // ATDATAM0
    _count += 32; // ATDATAM1
    _count += 32; // ATDATAM2
    _count += 32; // ATDATAM3
    _count += 7; // ATIDM0
    _count += 7; // ATIDM1
    _count += 7; // ATIDM2
    _count += 7; // ATIDM3
    _count += 1; // ATVALIDM0
    _count += 1; // ATVALIDM1
    _count += 1; // ATVALIDM2
    _count += 1; // ATVALIDM3
    return _count;
}

int cap_ap_csr_sta_CORE_BIST_t::s_get_width() {
    int _count = 0;

    _count += 444; // DONE_FAIL_OUT
    _count += 444; // DONE_PASS_OUT
    return _count;
}

int cap_ap_csr_cfg_CORE_BIST_t::s_get_width() {
    int _count = 0;

    _count += 1; // PTRN_FILL
    _count += 1; // RPR_MODE
    _count += 1; // RUN
    _count += 1; // SEL_CLK
    _count += 1; // SEL_L2DCLK
    _count += 1; // SEL_L2TCLK
    _count += 1; // MEM_RST
    _count += 1; // RESET_OVERRIDE
    return _count;
}

int cap_ap_csr_sta_apms_t::s_get_width() {
    int _count = 0;

    _count += 2; // fatal
    return _count;
}

int cap_ap_csr_sta_gic_mbist_t::s_get_width() {
    int _count = 0;

    _count += 1; // mbistack
    _count += 64; // mbistoutdata
    return _count;
}

int cap_ap_csr_cfg_gic_t::s_get_width() {
    int _count = 0;

    _count += 3; // base_awuser
    _count += 3; // base_aruser
    _count += 17; // awuser
    return _count;
}

int cap_ap_csr_cfg_gic_mbist_t::s_get_width() {
    int _count = 0;

    _count += 12; // mbistaddr
    _count += 2; // mbistarray
    _count += 1; // mbistcfg
    _count += 1; // mbistreaden
    _count += 1; // mbistreq
    _count += 1; // mbistresetn
    _count += 1; // mbistwriteen
    _count += 64; // mbistindata
    return _count;
}

int cap_ap_csr_cfg_irq_t::s_get_width() {
    int _count = 0;

    _count += 4; // nFIQ_mask
    _count += 4; // nIRQ_mask
    _count += 4; // nREI_mask
    _count += 4; // nSEI_mask
    _count += 4; // nVSEI_mask
    return _count;
}

int cap_ap_csr_cfg_apar_t::s_get_width() {
    int _count = 0;

    _count += 24; // addr_top24
    _count += 2; // awusers
    _count += 2; // arusers
    _count += 1; // addr_sel
    return _count;
}

int cap_ap_csr_cfg_cpu_t::s_get_width() {
    int _count = 0;

    _count += 4; // active
    _count += 4; // sta_wake_request
    return _count;
}

int cap_ap_csr_cfg_cpu3_t::s_get_width() {
    int _count = 0;

    _count += 44; // RVBARADDR
    return _count;
}

int cap_ap_csr_cfg_cpu2_t::s_get_width() {
    int _count = 0;

    _count += 44; // RVBARADDR
    return _count;
}

int cap_ap_csr_cfg_cpu1_t::s_get_width() {
    int _count = 0;

    _count += 44; // RVBARADDR
    return _count;
}

int cap_ap_csr_cfg_cpu0_t::s_get_width() {
    int _count = 0;

    _count += 44; // RVBARADDR
    return _count;
}

int cap_ap_csr_cfg_cpu0_flash_t::s_get_width() {
    int _count = 0;

    _count += 44; // RVBARADDR
    return _count;
}

int cap_ap_csr_cfg_static_t::s_get_width() {
    int _count = 0;

    _count += 1; // DBGL1RSTDISABLE
    _count += 1; // L2RSTDISABLE
    _count += 4; // CP15SDISABLE
    _count += 8; // CLUSTERIDAFF1
    _count += 8; // CLUSTERIDAFF2
    _count += 4; // CRYPTODISABLE
    _count += 4; // AA64nAA32
    _count += 1; // SYSBARDISABLE
    _count += 1; // BROADCASTCACHEMAINT
    _count += 1; // BROADCASTINNER
    _count += 1; // BROADCASTOUTER
    _count += 4; // CFGEND
    _count += 4; // CFGTE
    _count += 26; // PERIPHBASE
    return _count;
}

int cap_ap_csr_dummy_t::s_get_width() {
    int _count = 0;

    _count += 64; // cycle_counter
    return _count;
}

int cap_ap_csr_base_t::s_get_width() {
    int _count = 0;

    _count += 32; // scratch_reg
    return _count;
}

int cap_ap_csr_t::s_get_width() {
    int _count = 0;

    _count += cap_ap_csr_base_t::s_get_width(); // base
    _count += cap_ap_csr_dummy_t::s_get_width(); // dummy
    _count += cap_ap_csr_cfg_static_t::s_get_width(); // cfg_static
    _count += cap_ap_csr_cfg_cpu0_flash_t::s_get_width(); // cfg_cpu0_flash
    _count += cap_ap_csr_cfg_cpu0_t::s_get_width(); // cfg_cpu0
    _count += cap_ap_csr_cfg_cpu1_t::s_get_width(); // cfg_cpu1
    _count += cap_ap_csr_cfg_cpu2_t::s_get_width(); // cfg_cpu2
    _count += cap_ap_csr_cfg_cpu3_t::s_get_width(); // cfg_cpu3
    _count += cap_ap_csr_cfg_cpu_t::s_get_width(); // cfg_cpu
    _count += cap_ap_csr_cfg_apar_t::s_get_width(); // cfg_apar
    _count += cap_ap_csr_cfg_irq_t::s_get_width(); // cfg_irq
    _count += cap_ap_csr_cfg_gic_mbist_t::s_get_width(); // cfg_gic_mbist
    _count += cap_ap_csr_cfg_gic_t::s_get_width(); // cfg_gic
    _count += cap_ap_csr_sta_gic_mbist_t::s_get_width(); // sta_gic_mbist
    _count += cap_ap_csr_sta_apms_t::s_get_width(); // sta_apms
    _count += cap_ap_csr_cfg_CORE_BIST_t::s_get_width(); // cfg_CORE_BIST
    _count += cap_ap_csr_sta_CORE_BIST_t::s_get_width(); // sta_CORE_BIST
    _count += cap_ap_csr_sta_AT_t::s_get_width(); // sta_AT
    _count += cap_ap_csr_cfg_AT_t::s_get_width(); // cfg_AT
    _count += cap_ap_csr_sta_timer_t::s_get_width(); // sta_timer
    _count += cap_ap_csr_sta_pmu_t::s_get_width(); // sta_pmu
    _count += cap_ap_csr_cfg_pmu_t::s_get_width(); // cfg_pmu
    _count += cap_ap_csr_cfg_EV_t::s_get_width(); // cfg_EV
    _count += cap_ap_csr_sta_EV_t::s_get_width(); // sta_EV
    _count += cap_ap_csr_cfg_L2_t::s_get_width(); // cfg_L2
    _count += cap_ap_csr_sta_L2_t::s_get_width(); // sta_L2
    _count += cap_ap_csr_sta_IRQ_t::s_get_width(); // sta_IRQ
    _count += cap_ap_csr_sta_misc_t::s_get_width(); // sta_misc
    _count += cap_ap_csr_cfg_CTI_t::s_get_width(); // cfg_CTI
    _count += cap_ap_csr_sta_CTI_t::s_get_width(); // sta_CTI
    _count += cap_ap_csr_cfg_AUTH_t::s_get_width(); // cfg_AUTH
    _count += cap_ap_csr_cfg_VSIG_t::s_get_width(); // cfg_VSIG
    _count += cap_ap_csr_sta_VSIG_t::s_get_width(); // sta_VSIG
    _count += cap_ap_csr_sta_misc_dbg_t::s_get_width(); // sta_misc_dbg
    _count += cap_ap_csr_cfg_misc_dbg_t::s_get_width(); // cfg_misc_dbg
    _count += cap_ap_csr_cfg_dstream_t::s_get_width(); // cfg_dstream
    _count += cap_ap_csr_sta_dstream_t::s_get_width(); // sta_dstream
    _count += cap_ap_csr_sta_nts_t::s_get_width(); // sta_nts
    _count += cap_ap_csr_cfg_nts_t::s_get_width(); // cfg_nts
    _count += cap_ap_csr_cfg_ACP_t::s_get_width(); // cfg_ACP
    _count += cap_ap_csr_ap_cfg_t::s_get_width(); // ap_cfg
    _count += cap_apb_csr_t::s_get_width(); // apb
    return _count;
}

void cap_ap_csr_ap_cfg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__debug_port_enable = _val.convert_to< debug_port_enable_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__debug_port_select = _val.convert_to< debug_port_select_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__cfg_stall_on_same_id = _val.convert_to< cfg_stall_on_same_id_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_ACP_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__AINACTS = _val.convert_to< AINACTS_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ACINACTM = _val.convert_to< ACINACTM_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_nts_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__TSBITS_TS = _val.convert_to< TSBITS_TS_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__TSSYNCS_TS = _val.convert_to< TSSYNCS_TS_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_ap_csr_sta_nts_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__TSSYNCREADYS_TS = _val.convert_to< TSSYNCREADYS_TS_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_sta_dstream_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__DBGACK = _val.convert_to< DBGACK_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__DBGRSTREQ = _val.convert_to< DBGRSTREQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_cfg_dstream_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__EDBGRQ = _val.convert_to< EDBGRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_cfg_misc_dbg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__DBGROMADDRV = _val.convert_to< DBGROMADDRV_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__DBGROMADDR = _val.convert_to< DBGROMADDR_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_ap_csr_sta_misc_dbg_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__COMMRX = _val.convert_to< COMMRX_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__COMMTX = _val.convert_to< COMMTX_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nCOMMIRQ = _val.convert_to< nCOMMIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_sta_VSIG_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__CLREXMONACK = _val.convert_to< CLREXMONACK_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_VSIG_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__CLREXMONREQ = _val.convert_to< CLREXMONREQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_AUTH_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__DBGEN = _val.convert_to< DBGEN_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__SPNIDEN = _val.convert_to< SPNIDEN_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__SPIDEN = _val.convert_to< SPIDEN_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__NIDEN = _val.convert_to< NIDEN_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_sta_CTI_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__CTICHOUT = _val.convert_to< CTICHOUT_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__CTICHINACK = _val.convert_to< CTICHINACK_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__CTIIRQ = _val.convert_to< CTIIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_cfg_CTI_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__CTICHIN = _val.convert_to< CTICHIN_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__CTICHOUTACK = _val.convert_to< CTICHOUTACK_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__CTIIRQACK = _val.convert_to< CTIIRQACK_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_sta_misc_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__WARMRSTREQ = _val.convert_to< WARMRSTREQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__WRMEMATTRM = _val.convert_to< WRMEMATTRM_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__RDMEMATTRM = _val.convert_to< RDMEMATTRM_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__SMPEN = _val.convert_to< SMPEN_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__STANDBYWFE = _val.convert_to< STANDBYWFE_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__STANDBYWFI = _val.convert_to< STANDBYWFI_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__STANDBYWFIL2 = _val.convert_to< STANDBYWFIL2_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_sta_IRQ_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__nEXTERRIRQ = _val.convert_to< nEXTERRIRQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__nINTERRIRQ = _val.convert_to< nINTERRIRQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__nVCPUMNTIRQ = _val.convert_to< nVCPUMNTIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_sta_L2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__L2FLUSHDONE = _val.convert_to< L2FLUSHDONE_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_L2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__L2FLUSHREQ = _val.convert_to< L2FLUSHREQ_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_sta_EV_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__EVENTO = _val.convert_to< EVENTO_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_EV_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__EVENTI = _val.convert_to< EVENTI_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_pmu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__PMUSNAPSHOTREQ = _val.convert_to< PMUSNAPSHOTREQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_sta_pmu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__PMUEVENT0 = _val.convert_to< PMUEVENT0_cpp_int_t >()  ;
    _val = _val >> 25;
    
    int_var__PMUEVENT1 = _val.convert_to< PMUEVENT1_cpp_int_t >()  ;
    _val = _val >> 25;
    
    int_var__PMUEVENT2 = _val.convert_to< PMUEVENT2_cpp_int_t >()  ;
    _val = _val >> 25;
    
    int_var__PMUEVENT3 = _val.convert_to< PMUEVENT3_cpp_int_t >()  ;
    _val = _val >> 25;
    
    int_var__nPMUIRQ = _val.convert_to< nPMUIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__PMUSNAPSHOTACK = _val.convert_to< PMUSNAPSHOTACK_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_sta_timer_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__nCNTHPIRQ = _val.convert_to< nCNTHPIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nCNTPNSIRQ = _val.convert_to< nCNTPNSIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nCNTPSIRQ = _val.convert_to< nCNTPSIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nCNTVIRQ = _val.convert_to< nCNTVIRQ_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_cfg_AT_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__AFVALIDM0 = _val.convert_to< AFVALIDM0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__AFVALIDM1 = _val.convert_to< AFVALIDM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__AFVALIDM2 = _val.convert_to< AFVALIDM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__AFVALIDM3 = _val.convert_to< AFVALIDM3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATREADYM0 = _val.convert_to< ATREADYM0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATREADYM1 = _val.convert_to< ATREADYM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATREADYM2 = _val.convert_to< ATREADYM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATREADYM3 = _val.convert_to< ATREADYM3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SYNCREQM0 = _val.convert_to< SYNCREQM0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SYNCREQM1 = _val.convert_to< SYNCREQM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SYNCREQM2 = _val.convert_to< SYNCREQM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SYNCREQM3 = _val.convert_to< SYNCREQM3_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_sta_AT_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__AFREADYM0 = _val.convert_to< AFREADYM0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__AFREADYM1 = _val.convert_to< AFREADYM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__AFREADYM2 = _val.convert_to< AFREADYM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__AFREADYM3 = _val.convert_to< AFREADYM3_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATBYTESM0 = _val.convert_to< ATBYTESM0_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__ATBYTESM1 = _val.convert_to< ATBYTESM1_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__ATBYTESM2 = _val.convert_to< ATBYTESM2_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__ATBYTESM3 = _val.convert_to< ATBYTESM3_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__ATDATAM0 = _val.convert_to< ATDATAM0_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ATDATAM1 = _val.convert_to< ATDATAM1_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ATDATAM2 = _val.convert_to< ATDATAM2_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ATDATAM3 = _val.convert_to< ATDATAM3_cpp_int_t >()  ;
    _val = _val >> 32;
    
    int_var__ATIDM0 = _val.convert_to< ATIDM0_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__ATIDM1 = _val.convert_to< ATIDM1_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__ATIDM2 = _val.convert_to< ATIDM2_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__ATIDM3 = _val.convert_to< ATIDM3_cpp_int_t >()  ;
    _val = _val >> 7;
    
    int_var__ATVALIDM0 = _val.convert_to< ATVALIDM0_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATVALIDM1 = _val.convert_to< ATVALIDM1_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATVALIDM2 = _val.convert_to< ATVALIDM2_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__ATVALIDM3 = _val.convert_to< ATVALIDM3_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_sta_CORE_BIST_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__DONE_FAIL_OUT = _val.convert_to< DONE_FAIL_OUT_cpp_int_t >()  ;
    _val = _val >> 444;
    
    int_var__DONE_PASS_OUT = _val.convert_to< DONE_PASS_OUT_cpp_int_t >()  ;
    _val = _val >> 444;
    
}

void cap_ap_csr_cfg_CORE_BIST_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__PTRN_FILL = _val.convert_to< PTRN_FILL_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__RPR_MODE = _val.convert_to< RPR_MODE_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__RUN = _val.convert_to< RUN_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SEL_CLK = _val.convert_to< SEL_CLK_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SEL_L2DCLK = _val.convert_to< SEL_L2DCLK_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__SEL_L2TCLK = _val.convert_to< SEL_L2TCLK_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__MEM_RST = _val.convert_to< MEM_RST_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__RESET_OVERRIDE = _val.convert_to< RESET_OVERRIDE_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_sta_apms_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__fatal = _val.convert_to< fatal_cpp_int_t >()  ;
    _val = _val >> 2;
    
}

void cap_ap_csr_sta_gic_mbist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mbistack = _val.convert_to< mbistack_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mbistoutdata = _val.convert_to< mbistoutdata_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_ap_csr_cfg_gic_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__base_awuser = _val.convert_to< base_awuser_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__base_aruser = _val.convert_to< base_aruser_cpp_int_t >()  ;
    _val = _val >> 3;
    
    int_var__awuser = _val.convert_to< awuser_cpp_int_t >()  ;
    _val = _val >> 17;
    
}

void cap_ap_csr_cfg_gic_mbist_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__mbistaddr = _val.convert_to< mbistaddr_cpp_int_t >()  ;
    _val = _val >> 12;
    
    int_var__mbistarray = _val.convert_to< mbistarray_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__mbistcfg = _val.convert_to< mbistcfg_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mbistreaden = _val.convert_to< mbistreaden_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mbistreq = _val.convert_to< mbistreq_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mbistresetn = _val.convert_to< mbistresetn_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mbistwriteen = _val.convert_to< mbistwriteen_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__mbistindata = _val.convert_to< mbistindata_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_ap_csr_cfg_irq_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__nFIQ_mask = _val.convert_to< nFIQ_mask_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nIRQ_mask = _val.convert_to< nIRQ_mask_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nREI_mask = _val.convert_to< nREI_mask_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nSEI_mask = _val.convert_to< nSEI_mask_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__nVSEI_mask = _val.convert_to< nVSEI_mask_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_cfg_apar_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__addr_top24 = _val.convert_to< addr_top24_cpp_int_t >()  ;
    _val = _val >> 24;
    
    int_var__awusers = _val.convert_to< awusers_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__arusers = _val.convert_to< arusers_cpp_int_t >()  ;
    _val = _val >> 2;
    
    int_var__addr_sel = _val.convert_to< addr_sel_cpp_int_t >()  ;
    _val = _val >> 1;
    
}

void cap_ap_csr_cfg_cpu_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__active = _val.convert_to< active_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__sta_wake_request = _val.convert_to< sta_wake_request_cpp_int_t >()  ;
    _val = _val >> 4;
    
}

void cap_ap_csr_cfg_cpu3_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >()  ;
    _val = _val >> 44;
    
}

void cap_ap_csr_cfg_cpu2_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >()  ;
    _val = _val >> 44;
    
}

void cap_ap_csr_cfg_cpu1_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >()  ;
    _val = _val >> 44;
    
}

void cap_ap_csr_cfg_cpu0_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >()  ;
    _val = _val >> 44;
    
}

void cap_ap_csr_cfg_cpu0_flash_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >()  ;
    _val = _val >> 44;
    
}

void cap_ap_csr_cfg_static_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__DBGL1RSTDISABLE = _val.convert_to< DBGL1RSTDISABLE_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__L2RSTDISABLE = _val.convert_to< L2RSTDISABLE_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CP15SDISABLE = _val.convert_to< CP15SDISABLE_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__CLUSTERIDAFF1 = _val.convert_to< CLUSTERIDAFF1_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__CLUSTERIDAFF2 = _val.convert_to< CLUSTERIDAFF2_cpp_int_t >()  ;
    _val = _val >> 8;
    
    int_var__CRYPTODISABLE = _val.convert_to< CRYPTODISABLE_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__AA64nAA32 = _val.convert_to< AA64nAA32_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__SYSBARDISABLE = _val.convert_to< SYSBARDISABLE_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__BROADCASTCACHEMAINT = _val.convert_to< BROADCASTCACHEMAINT_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__BROADCASTINNER = _val.convert_to< BROADCASTINNER_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__BROADCASTOUTER = _val.convert_to< BROADCASTOUTER_cpp_int_t >()  ;
    _val = _val >> 1;
    
    int_var__CFGEND = _val.convert_to< CFGEND_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__CFGTE = _val.convert_to< CFGTE_cpp_int_t >()  ;
    _val = _val >> 4;
    
    int_var__PERIPHBASE = _val.convert_to< PERIPHBASE_cpp_int_t >()  ;
    _val = _val >> 26;
    
}

void cap_ap_csr_dummy_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__cycle_counter = _val.convert_to< cycle_counter_cpp_int_t >()  ;
    _val = _val >> 64;
    
}

void cap_ap_csr_base_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >()  ;
    _val = _val >> 32;
    
}

void cap_ap_csr_t::all(const cpp_int & in_val) {
    cpp_int _val = in_val;

    base.all( _val);
    _val = _val >> base.get_width(); 
    dummy.all( _val);
    _val = _val >> dummy.get_width(); 
    cfg_static.all( _val);
    _val = _val >> cfg_static.get_width(); 
    cfg_cpu0_flash.all( _val);
    _val = _val >> cfg_cpu0_flash.get_width(); 
    cfg_cpu0.all( _val);
    _val = _val >> cfg_cpu0.get_width(); 
    cfg_cpu1.all( _val);
    _val = _val >> cfg_cpu1.get_width(); 
    cfg_cpu2.all( _val);
    _val = _val >> cfg_cpu2.get_width(); 
    cfg_cpu3.all( _val);
    _val = _val >> cfg_cpu3.get_width(); 
    cfg_cpu.all( _val);
    _val = _val >> cfg_cpu.get_width(); 
    cfg_apar.all( _val);
    _val = _val >> cfg_apar.get_width(); 
    cfg_irq.all( _val);
    _val = _val >> cfg_irq.get_width(); 
    cfg_gic_mbist.all( _val);
    _val = _val >> cfg_gic_mbist.get_width(); 
    cfg_gic.all( _val);
    _val = _val >> cfg_gic.get_width(); 
    sta_gic_mbist.all( _val);
    _val = _val >> sta_gic_mbist.get_width(); 
    sta_apms.all( _val);
    _val = _val >> sta_apms.get_width(); 
    cfg_CORE_BIST.all( _val);
    _val = _val >> cfg_CORE_BIST.get_width(); 
    sta_CORE_BIST.all( _val);
    _val = _val >> sta_CORE_BIST.get_width(); 
    sta_AT.all( _val);
    _val = _val >> sta_AT.get_width(); 
    cfg_AT.all( _val);
    _val = _val >> cfg_AT.get_width(); 
    sta_timer.all( _val);
    _val = _val >> sta_timer.get_width(); 
    sta_pmu.all( _val);
    _val = _val >> sta_pmu.get_width(); 
    cfg_pmu.all( _val);
    _val = _val >> cfg_pmu.get_width(); 
    cfg_EV.all( _val);
    _val = _val >> cfg_EV.get_width(); 
    sta_EV.all( _val);
    _val = _val >> sta_EV.get_width(); 
    cfg_L2.all( _val);
    _val = _val >> cfg_L2.get_width(); 
    sta_L2.all( _val);
    _val = _val >> sta_L2.get_width(); 
    sta_IRQ.all( _val);
    _val = _val >> sta_IRQ.get_width(); 
    sta_misc.all( _val);
    _val = _val >> sta_misc.get_width(); 
    cfg_CTI.all( _val);
    _val = _val >> cfg_CTI.get_width(); 
    sta_CTI.all( _val);
    _val = _val >> sta_CTI.get_width(); 
    cfg_AUTH.all( _val);
    _val = _val >> cfg_AUTH.get_width(); 
    cfg_VSIG.all( _val);
    _val = _val >> cfg_VSIG.get_width(); 
    sta_VSIG.all( _val);
    _val = _val >> sta_VSIG.get_width(); 
    sta_misc_dbg.all( _val);
    _val = _val >> sta_misc_dbg.get_width(); 
    cfg_misc_dbg.all( _val);
    _val = _val >> cfg_misc_dbg.get_width(); 
    cfg_dstream.all( _val);
    _val = _val >> cfg_dstream.get_width(); 
    sta_dstream.all( _val);
    _val = _val >> sta_dstream.get_width(); 
    sta_nts.all( _val);
    _val = _val >> sta_nts.get_width(); 
    cfg_nts.all( _val);
    _val = _val >> cfg_nts.get_width(); 
    cfg_ACP.all( _val);
    _val = _val >> cfg_ACP.get_width(); 
    ap_cfg.all( _val);
    _val = _val >> ap_cfg.get_width(); 
    apb.all( _val);
    _val = _val >> apb.get_width(); 
}

cpp_int cap_ap_csr_t::all() const {
    cpp_int ret_val;

    ret_val = ret_val << apb.get_width(); ret_val = ret_val  | apb.all(); 
    ret_val = ret_val << ap_cfg.get_width(); ret_val = ret_val  | ap_cfg.all(); 
    ret_val = ret_val << cfg_ACP.get_width(); ret_val = ret_val  | cfg_ACP.all(); 
    ret_val = ret_val << cfg_nts.get_width(); ret_val = ret_val  | cfg_nts.all(); 
    ret_val = ret_val << sta_nts.get_width(); ret_val = ret_val  | sta_nts.all(); 
    ret_val = ret_val << sta_dstream.get_width(); ret_val = ret_val  | sta_dstream.all(); 
    ret_val = ret_val << cfg_dstream.get_width(); ret_val = ret_val  | cfg_dstream.all(); 
    ret_val = ret_val << cfg_misc_dbg.get_width(); ret_val = ret_val  | cfg_misc_dbg.all(); 
    ret_val = ret_val << sta_misc_dbg.get_width(); ret_val = ret_val  | sta_misc_dbg.all(); 
    ret_val = ret_val << sta_VSIG.get_width(); ret_val = ret_val  | sta_VSIG.all(); 
    ret_val = ret_val << cfg_VSIG.get_width(); ret_val = ret_val  | cfg_VSIG.all(); 
    ret_val = ret_val << cfg_AUTH.get_width(); ret_val = ret_val  | cfg_AUTH.all(); 
    ret_val = ret_val << sta_CTI.get_width(); ret_val = ret_val  | sta_CTI.all(); 
    ret_val = ret_val << cfg_CTI.get_width(); ret_val = ret_val  | cfg_CTI.all(); 
    ret_val = ret_val << sta_misc.get_width(); ret_val = ret_val  | sta_misc.all(); 
    ret_val = ret_val << sta_IRQ.get_width(); ret_val = ret_val  | sta_IRQ.all(); 
    ret_val = ret_val << sta_L2.get_width(); ret_val = ret_val  | sta_L2.all(); 
    ret_val = ret_val << cfg_L2.get_width(); ret_val = ret_val  | cfg_L2.all(); 
    ret_val = ret_val << sta_EV.get_width(); ret_val = ret_val  | sta_EV.all(); 
    ret_val = ret_val << cfg_EV.get_width(); ret_val = ret_val  | cfg_EV.all(); 
    ret_val = ret_val << cfg_pmu.get_width(); ret_val = ret_val  | cfg_pmu.all(); 
    ret_val = ret_val << sta_pmu.get_width(); ret_val = ret_val  | sta_pmu.all(); 
    ret_val = ret_val << sta_timer.get_width(); ret_val = ret_val  | sta_timer.all(); 
    ret_val = ret_val << cfg_AT.get_width(); ret_val = ret_val  | cfg_AT.all(); 
    ret_val = ret_val << sta_AT.get_width(); ret_val = ret_val  | sta_AT.all(); 
    ret_val = ret_val << sta_CORE_BIST.get_width(); ret_val = ret_val  | sta_CORE_BIST.all(); 
    ret_val = ret_val << cfg_CORE_BIST.get_width(); ret_val = ret_val  | cfg_CORE_BIST.all(); 
    ret_val = ret_val << sta_apms.get_width(); ret_val = ret_val  | sta_apms.all(); 
    ret_val = ret_val << sta_gic_mbist.get_width(); ret_val = ret_val  | sta_gic_mbist.all(); 
    ret_val = ret_val << cfg_gic.get_width(); ret_val = ret_val  | cfg_gic.all(); 
    ret_val = ret_val << cfg_gic_mbist.get_width(); ret_val = ret_val  | cfg_gic_mbist.all(); 
    ret_val = ret_val << cfg_irq.get_width(); ret_val = ret_val  | cfg_irq.all(); 
    ret_val = ret_val << cfg_apar.get_width(); ret_val = ret_val  | cfg_apar.all(); 
    ret_val = ret_val << cfg_cpu.get_width(); ret_val = ret_val  | cfg_cpu.all(); 
    ret_val = ret_val << cfg_cpu3.get_width(); ret_val = ret_val  | cfg_cpu3.all(); 
    ret_val = ret_val << cfg_cpu2.get_width(); ret_val = ret_val  | cfg_cpu2.all(); 
    ret_val = ret_val << cfg_cpu1.get_width(); ret_val = ret_val  | cfg_cpu1.all(); 
    ret_val = ret_val << cfg_cpu0.get_width(); ret_val = ret_val  | cfg_cpu0.all(); 
    ret_val = ret_val << cfg_cpu0_flash.get_width(); ret_val = ret_val  | cfg_cpu0_flash.all(); 
    ret_val = ret_val << cfg_static.get_width(); ret_val = ret_val  | cfg_static.all(); 
    ret_val = ret_val << dummy.get_width(); ret_val = ret_val  | dummy.all(); 
    ret_val = ret_val << base.get_width(); ret_val = ret_val  | base.all(); 
    return ret_val;
}

cpp_int cap_ap_csr_base_t::all() const {
    cpp_int ret_val;

    // scratch_reg
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__scratch_reg; 
    
    return ret_val;
}

cpp_int cap_ap_csr_dummy_t::all() const {
    cpp_int ret_val;

    // cycle_counter
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__cycle_counter; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_static_t::all() const {
    cpp_int ret_val;

    // PERIPHBASE
    ret_val = ret_val << 26; ret_val = ret_val  | int_var__PERIPHBASE; 
    
    // CFGTE
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CFGTE; 
    
    // CFGEND
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CFGEND; 
    
    // BROADCASTOUTER
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__BROADCASTOUTER; 
    
    // BROADCASTINNER
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__BROADCASTINNER; 
    
    // BROADCASTCACHEMAINT
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__BROADCASTCACHEMAINT; 
    
    // SYSBARDISABLE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SYSBARDISABLE; 
    
    // AA64nAA32
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__AA64nAA32; 
    
    // CRYPTODISABLE
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CRYPTODISABLE; 
    
    // CLUSTERIDAFF2
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__CLUSTERIDAFF2; 
    
    // CLUSTERIDAFF1
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__CLUSTERIDAFF1; 
    
    // CP15SDISABLE
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CP15SDISABLE; 
    
    // L2RSTDISABLE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__L2RSTDISABLE; 
    
    // DBGL1RSTDISABLE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__DBGL1RSTDISABLE; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_cpu0_flash_t::all() const {
    cpp_int ret_val;

    // RVBARADDR
    ret_val = ret_val << 44; ret_val = ret_val  | int_var__RVBARADDR; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_cpu0_t::all() const {
    cpp_int ret_val;

    // RVBARADDR
    ret_val = ret_val << 44; ret_val = ret_val  | int_var__RVBARADDR; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_cpu1_t::all() const {
    cpp_int ret_val;

    // RVBARADDR
    ret_val = ret_val << 44; ret_val = ret_val  | int_var__RVBARADDR; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_cpu2_t::all() const {
    cpp_int ret_val;

    // RVBARADDR
    ret_val = ret_val << 44; ret_val = ret_val  | int_var__RVBARADDR; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_cpu3_t::all() const {
    cpp_int ret_val;

    // RVBARADDR
    ret_val = ret_val << 44; ret_val = ret_val  | int_var__RVBARADDR; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_cpu_t::all() const {
    cpp_int ret_val;

    // sta_wake_request
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__sta_wake_request; 
    
    // active
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__active; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_apar_t::all() const {
    cpp_int ret_val;

    // addr_sel
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__addr_sel; 
    
    // arusers
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__arusers; 
    
    // awusers
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__awusers; 
    
    // addr_top24
    ret_val = ret_val << 24; ret_val = ret_val  | int_var__addr_top24; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_irq_t::all() const {
    cpp_int ret_val;

    // nVSEI_mask
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nVSEI_mask; 
    
    // nSEI_mask
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nSEI_mask; 
    
    // nREI_mask
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nREI_mask; 
    
    // nIRQ_mask
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nIRQ_mask; 
    
    // nFIQ_mask
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nFIQ_mask; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::all() const {
    cpp_int ret_val;

    // mbistindata
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__mbistindata; 
    
    // mbistwriteen
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mbistwriteen; 
    
    // mbistresetn
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mbistresetn; 
    
    // mbistreq
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mbistreq; 
    
    // mbistreaden
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mbistreaden; 
    
    // mbistcfg
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mbistcfg; 
    
    // mbistarray
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__mbistarray; 
    
    // mbistaddr
    ret_val = ret_val << 12; ret_val = ret_val  | int_var__mbistaddr; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_gic_t::all() const {
    cpp_int ret_val;

    // awuser
    ret_val = ret_val << 17; ret_val = ret_val  | int_var__awuser; 
    
    // base_aruser
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__base_aruser; 
    
    // base_awuser
    ret_val = ret_val << 3; ret_val = ret_val  | int_var__base_awuser; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_gic_mbist_t::all() const {
    cpp_int ret_val;

    // mbistoutdata
    ret_val = ret_val << 64; ret_val = ret_val  | int_var__mbistoutdata; 
    
    // mbistack
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__mbistack; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_apms_t::all() const {
    cpp_int ret_val;

    // fatal
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__fatal; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::all() const {
    cpp_int ret_val;

    // RESET_OVERRIDE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__RESET_OVERRIDE; 
    
    // MEM_RST
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__MEM_RST; 
    
    // SEL_L2TCLK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SEL_L2TCLK; 
    
    // SEL_L2DCLK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SEL_L2DCLK; 
    
    // SEL_CLK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SEL_CLK; 
    
    // RUN
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__RUN; 
    
    // RPR_MODE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__RPR_MODE; 
    
    // PTRN_FILL
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__PTRN_FILL; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_CORE_BIST_t::all() const {
    cpp_int ret_val;

    // DONE_PASS_OUT
    ret_val = ret_val << 444; ret_val = ret_val  | int_var__DONE_PASS_OUT; 
    
    // DONE_FAIL_OUT
    ret_val = ret_val << 444; ret_val = ret_val  | int_var__DONE_FAIL_OUT; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_AT_t::all() const {
    cpp_int ret_val;

    // ATVALIDM3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATVALIDM3; 
    
    // ATVALIDM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATVALIDM2; 
    
    // ATVALIDM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATVALIDM1; 
    
    // ATVALIDM0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATVALIDM0; 
    
    // ATIDM3
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__ATIDM3; 
    
    // ATIDM2
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__ATIDM2; 
    
    // ATIDM1
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__ATIDM1; 
    
    // ATIDM0
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__ATIDM0; 
    
    // ATDATAM3
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ATDATAM3; 
    
    // ATDATAM2
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ATDATAM2; 
    
    // ATDATAM1
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ATDATAM1; 
    
    // ATDATAM0
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__ATDATAM0; 
    
    // ATBYTESM3
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ATBYTESM3; 
    
    // ATBYTESM2
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ATBYTESM2; 
    
    // ATBYTESM1
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ATBYTESM1; 
    
    // ATBYTESM0
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__ATBYTESM0; 
    
    // AFREADYM3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFREADYM3; 
    
    // AFREADYM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFREADYM2; 
    
    // AFREADYM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFREADYM1; 
    
    // AFREADYM0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFREADYM0; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_AT_t::all() const {
    cpp_int ret_val;

    // SYNCREQM3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SYNCREQM3; 
    
    // SYNCREQM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SYNCREQM2; 
    
    // SYNCREQM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SYNCREQM1; 
    
    // SYNCREQM0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__SYNCREQM0; 
    
    // ATREADYM3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATREADYM3; 
    
    // ATREADYM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATREADYM2; 
    
    // ATREADYM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATREADYM1; 
    
    // ATREADYM0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ATREADYM0; 
    
    // AFVALIDM3
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFVALIDM3; 
    
    // AFVALIDM2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFVALIDM2; 
    
    // AFVALIDM1
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFVALIDM1; 
    
    // AFVALIDM0
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AFVALIDM0; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_timer_t::all() const {
    cpp_int ret_val;

    // nCNTVIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nCNTVIRQ; 
    
    // nCNTPSIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nCNTPSIRQ; 
    
    // nCNTPNSIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nCNTPNSIRQ; 
    
    // nCNTHPIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nCNTHPIRQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_pmu_t::all() const {
    cpp_int ret_val;

    // PMUSNAPSHOTACK
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__PMUSNAPSHOTACK; 
    
    // nPMUIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nPMUIRQ; 
    
    // PMUEVENT3
    ret_val = ret_val << 25; ret_val = ret_val  | int_var__PMUEVENT3; 
    
    // PMUEVENT2
    ret_val = ret_val << 25; ret_val = ret_val  | int_var__PMUEVENT2; 
    
    // PMUEVENT1
    ret_val = ret_val << 25; ret_val = ret_val  | int_var__PMUEVENT1; 
    
    // PMUEVENT0
    ret_val = ret_val << 25; ret_val = ret_val  | int_var__PMUEVENT0; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_pmu_t::all() const {
    cpp_int ret_val;

    // PMUSNAPSHOTREQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__PMUSNAPSHOTREQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_EV_t::all() const {
    cpp_int ret_val;

    // EVENTI
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__EVENTI; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_EV_t::all() const {
    cpp_int ret_val;

    // EVENTO
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__EVENTO; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_L2_t::all() const {
    cpp_int ret_val;

    // L2FLUSHREQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__L2FLUSHREQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_L2_t::all() const {
    cpp_int ret_val;

    // L2FLUSHDONE
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__L2FLUSHDONE; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_IRQ_t::all() const {
    cpp_int ret_val;

    // nVCPUMNTIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nVCPUMNTIRQ; 
    
    // nINTERRIRQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__nINTERRIRQ; 
    
    // nEXTERRIRQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__nEXTERRIRQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_misc_t::all() const {
    cpp_int ret_val;

    // STANDBYWFIL2
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__STANDBYWFIL2; 
    
    // STANDBYWFI
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__STANDBYWFI; 
    
    // STANDBYWFE
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__STANDBYWFE; 
    
    // SMPEN
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__SMPEN; 
    
    // RDMEMATTRM
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__RDMEMATTRM; 
    
    // WRMEMATTRM
    ret_val = ret_val << 8; ret_val = ret_val  | int_var__WRMEMATTRM; 
    
    // WARMRSTREQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__WARMRSTREQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_CTI_t::all() const {
    cpp_int ret_val;

    // CTIIRQACK
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CTIIRQACK; 
    
    // CTICHOUTACK
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CTICHOUTACK; 
    
    // CTICHIN
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CTICHIN; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_CTI_t::all() const {
    cpp_int ret_val;

    // CTIIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CTIIRQ; 
    
    // CTICHINACK
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CTICHINACK; 
    
    // CTICHOUT
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__CTICHOUT; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_AUTH_t::all() const {
    cpp_int ret_val;

    // NIDEN
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__NIDEN; 
    
    // SPIDEN
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__SPIDEN; 
    
    // SPNIDEN
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__SPNIDEN; 
    
    // DBGEN
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__DBGEN; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_VSIG_t::all() const {
    cpp_int ret_val;

    // CLREXMONREQ
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CLREXMONREQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_VSIG_t::all() const {
    cpp_int ret_val;

    // CLREXMONACK
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__CLREXMONACK; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_misc_dbg_t::all() const {
    cpp_int ret_val;

    // nCOMMIRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__nCOMMIRQ; 
    
    // COMMTX
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__COMMTX; 
    
    // COMMRX
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__COMMRX; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_misc_dbg_t::all() const {
    cpp_int ret_val;

    // DBGROMADDR
    ret_val = ret_val << 32; ret_val = ret_val  | int_var__DBGROMADDR; 
    
    // DBGROMADDRV
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__DBGROMADDRV; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_dstream_t::all() const {
    cpp_int ret_val;

    // EDBGRQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__EDBGRQ; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_dstream_t::all() const {
    cpp_int ret_val;

    // DBGRSTREQ
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__DBGRSTREQ; 
    
    // DBGACK
    ret_val = ret_val << 4; ret_val = ret_val  | int_var__DBGACK; 
    
    return ret_val;
}

cpp_int cap_ap_csr_sta_nts_t::all() const {
    cpp_int ret_val;

    // TSSYNCREADYS_TS
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__TSSYNCREADYS_TS; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_nts_t::all() const {
    cpp_int ret_val;

    // TSSYNCS_TS
    ret_val = ret_val << 2; ret_val = ret_val  | int_var__TSSYNCS_TS; 
    
    // TSBITS_TS
    ret_val = ret_val << 7; ret_val = ret_val  | int_var__TSBITS_TS; 
    
    return ret_val;
}

cpp_int cap_ap_csr_cfg_ACP_t::all() const {
    cpp_int ret_val;

    // ACINACTM
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__ACINACTM; 
    
    // AINACTS
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__AINACTS; 
    
    return ret_val;
}

cpp_int cap_ap_csr_ap_cfg_t::all() const {
    cpp_int ret_val;

    // cfg_stall_on_same_id
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__cfg_stall_on_same_id; 
    
    // debug_port_select
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_port_select; 
    
    // debug_port_enable
    ret_val = ret_val << 1; ret_val = ret_val  | int_var__debug_port_enable; 
    
    return ret_val;
}

void cap_ap_csr_ap_cfg_t::clear() {

    int_var__debug_port_enable = 0; 
    
    int_var__debug_port_select = 0; 
    
    int_var__cfg_stall_on_same_id = 0; 
    
}

void cap_ap_csr_cfg_ACP_t::clear() {

    int_var__AINACTS = 0; 
    
    int_var__ACINACTM = 0; 
    
}

void cap_ap_csr_cfg_nts_t::clear() {

    int_var__TSBITS_TS = 0; 
    
    int_var__TSSYNCS_TS = 0; 
    
}

void cap_ap_csr_sta_nts_t::clear() {

    int_var__TSSYNCREADYS_TS = 0; 
    
}

void cap_ap_csr_sta_dstream_t::clear() {

    int_var__DBGACK = 0; 
    
    int_var__DBGRSTREQ = 0; 
    
}

void cap_ap_csr_cfg_dstream_t::clear() {

    int_var__EDBGRQ = 0; 
    
}

void cap_ap_csr_cfg_misc_dbg_t::clear() {

    int_var__DBGROMADDRV = 0; 
    
    int_var__DBGROMADDR = 0; 
    
}

void cap_ap_csr_sta_misc_dbg_t::clear() {

    int_var__COMMRX = 0; 
    
    int_var__COMMTX = 0; 
    
    int_var__nCOMMIRQ = 0; 
    
}

void cap_ap_csr_sta_VSIG_t::clear() {

    int_var__CLREXMONACK = 0; 
    
}

void cap_ap_csr_cfg_VSIG_t::clear() {

    int_var__CLREXMONREQ = 0; 
    
}

void cap_ap_csr_cfg_AUTH_t::clear() {

    int_var__DBGEN = 0; 
    
    int_var__SPNIDEN = 0; 
    
    int_var__SPIDEN = 0; 
    
    int_var__NIDEN = 0; 
    
}

void cap_ap_csr_sta_CTI_t::clear() {

    int_var__CTICHOUT = 0; 
    
    int_var__CTICHINACK = 0; 
    
    int_var__CTIIRQ = 0; 
    
}

void cap_ap_csr_cfg_CTI_t::clear() {

    int_var__CTICHIN = 0; 
    
    int_var__CTICHOUTACK = 0; 
    
    int_var__CTIIRQACK = 0; 
    
}

void cap_ap_csr_sta_misc_t::clear() {

    int_var__WARMRSTREQ = 0; 
    
    int_var__WRMEMATTRM = 0; 
    
    int_var__RDMEMATTRM = 0; 
    
    int_var__SMPEN = 0; 
    
    int_var__STANDBYWFE = 0; 
    
    int_var__STANDBYWFI = 0; 
    
    int_var__STANDBYWFIL2 = 0; 
    
}

void cap_ap_csr_sta_IRQ_t::clear() {

    int_var__nEXTERRIRQ = 0; 
    
    int_var__nINTERRIRQ = 0; 
    
    int_var__nVCPUMNTIRQ = 0; 
    
}

void cap_ap_csr_sta_L2_t::clear() {

    int_var__L2FLUSHDONE = 0; 
    
}

void cap_ap_csr_cfg_L2_t::clear() {

    int_var__L2FLUSHREQ = 0; 
    
}

void cap_ap_csr_sta_EV_t::clear() {

    int_var__EVENTO = 0; 
    
}

void cap_ap_csr_cfg_EV_t::clear() {

    int_var__EVENTI = 0; 
    
}

void cap_ap_csr_cfg_pmu_t::clear() {

    int_var__PMUSNAPSHOTREQ = 0; 
    
}

void cap_ap_csr_sta_pmu_t::clear() {

    int_var__PMUEVENT0 = 0; 
    
    int_var__PMUEVENT1 = 0; 
    
    int_var__PMUEVENT2 = 0; 
    
    int_var__PMUEVENT3 = 0; 
    
    int_var__nPMUIRQ = 0; 
    
    int_var__PMUSNAPSHOTACK = 0; 
    
}

void cap_ap_csr_sta_timer_t::clear() {

    int_var__nCNTHPIRQ = 0; 
    
    int_var__nCNTPNSIRQ = 0; 
    
    int_var__nCNTPSIRQ = 0; 
    
    int_var__nCNTVIRQ = 0; 
    
}

void cap_ap_csr_cfg_AT_t::clear() {

    int_var__AFVALIDM0 = 0; 
    
    int_var__AFVALIDM1 = 0; 
    
    int_var__AFVALIDM2 = 0; 
    
    int_var__AFVALIDM3 = 0; 
    
    int_var__ATREADYM0 = 0; 
    
    int_var__ATREADYM1 = 0; 
    
    int_var__ATREADYM2 = 0; 
    
    int_var__ATREADYM3 = 0; 
    
    int_var__SYNCREQM0 = 0; 
    
    int_var__SYNCREQM1 = 0; 
    
    int_var__SYNCREQM2 = 0; 
    
    int_var__SYNCREQM3 = 0; 
    
}

void cap_ap_csr_sta_AT_t::clear() {

    int_var__AFREADYM0 = 0; 
    
    int_var__AFREADYM1 = 0; 
    
    int_var__AFREADYM2 = 0; 
    
    int_var__AFREADYM3 = 0; 
    
    int_var__ATBYTESM0 = 0; 
    
    int_var__ATBYTESM1 = 0; 
    
    int_var__ATBYTESM2 = 0; 
    
    int_var__ATBYTESM3 = 0; 
    
    int_var__ATDATAM0 = 0; 
    
    int_var__ATDATAM1 = 0; 
    
    int_var__ATDATAM2 = 0; 
    
    int_var__ATDATAM3 = 0; 
    
    int_var__ATIDM0 = 0; 
    
    int_var__ATIDM1 = 0; 
    
    int_var__ATIDM2 = 0; 
    
    int_var__ATIDM3 = 0; 
    
    int_var__ATVALIDM0 = 0; 
    
    int_var__ATVALIDM1 = 0; 
    
    int_var__ATVALIDM2 = 0; 
    
    int_var__ATVALIDM3 = 0; 
    
}

void cap_ap_csr_sta_CORE_BIST_t::clear() {

    int_var__DONE_FAIL_OUT = 0; 
    
    int_var__DONE_PASS_OUT = 0; 
    
}

void cap_ap_csr_cfg_CORE_BIST_t::clear() {

    int_var__PTRN_FILL = 0; 
    
    int_var__RPR_MODE = 0; 
    
    int_var__RUN = 0; 
    
    int_var__SEL_CLK = 0; 
    
    int_var__SEL_L2DCLK = 0; 
    
    int_var__SEL_L2TCLK = 0; 
    
    int_var__MEM_RST = 0; 
    
    int_var__RESET_OVERRIDE = 0; 
    
}

void cap_ap_csr_sta_apms_t::clear() {

    int_var__fatal = 0; 
    
}

void cap_ap_csr_sta_gic_mbist_t::clear() {

    int_var__mbistack = 0; 
    
    int_var__mbistoutdata = 0; 
    
}

void cap_ap_csr_cfg_gic_t::clear() {

    int_var__base_awuser = 0; 
    
    int_var__base_aruser = 0; 
    
    int_var__awuser = 0; 
    
}

void cap_ap_csr_cfg_gic_mbist_t::clear() {

    int_var__mbistaddr = 0; 
    
    int_var__mbistarray = 0; 
    
    int_var__mbistcfg = 0; 
    
    int_var__mbistreaden = 0; 
    
    int_var__mbistreq = 0; 
    
    int_var__mbistresetn = 0; 
    
    int_var__mbistwriteen = 0; 
    
    int_var__mbistindata = 0; 
    
}

void cap_ap_csr_cfg_irq_t::clear() {

    int_var__nFIQ_mask = 0; 
    
    int_var__nIRQ_mask = 0; 
    
    int_var__nREI_mask = 0; 
    
    int_var__nSEI_mask = 0; 
    
    int_var__nVSEI_mask = 0; 
    
}

void cap_ap_csr_cfg_apar_t::clear() {

    int_var__addr_top24 = 0; 
    
    int_var__awusers = 0; 
    
    int_var__arusers = 0; 
    
    int_var__addr_sel = 0; 
    
}

void cap_ap_csr_cfg_cpu_t::clear() {

    int_var__active = 0; 
    
    int_var__sta_wake_request = 0; 
    
}

void cap_ap_csr_cfg_cpu3_t::clear() {

    int_var__RVBARADDR = 0; 
    
}

void cap_ap_csr_cfg_cpu2_t::clear() {

    int_var__RVBARADDR = 0; 
    
}

void cap_ap_csr_cfg_cpu1_t::clear() {

    int_var__RVBARADDR = 0; 
    
}

void cap_ap_csr_cfg_cpu0_t::clear() {

    int_var__RVBARADDR = 0; 
    
}

void cap_ap_csr_cfg_cpu0_flash_t::clear() {

    int_var__RVBARADDR = 0; 
    
}

void cap_ap_csr_cfg_static_t::clear() {

    int_var__DBGL1RSTDISABLE = 0; 
    
    int_var__L2RSTDISABLE = 0; 
    
    int_var__CP15SDISABLE = 0; 
    
    int_var__CLUSTERIDAFF1 = 0; 
    
    int_var__CLUSTERIDAFF2 = 0; 
    
    int_var__CRYPTODISABLE = 0; 
    
    int_var__AA64nAA32 = 0; 
    
    int_var__SYSBARDISABLE = 0; 
    
    int_var__BROADCASTCACHEMAINT = 0; 
    
    int_var__BROADCASTINNER = 0; 
    
    int_var__BROADCASTOUTER = 0; 
    
    int_var__CFGEND = 0; 
    
    int_var__CFGTE = 0; 
    
    int_var__PERIPHBASE = 0; 
    
}

void cap_ap_csr_dummy_t::clear() {

    int_var__cycle_counter = 0; 
    
}

void cap_ap_csr_base_t::clear() {

    int_var__scratch_reg = 0; 
    
}

void cap_ap_csr_t::clear() {

    base.clear();
    dummy.clear();
    cfg_static.clear();
    cfg_cpu0_flash.clear();
    cfg_cpu0.clear();
    cfg_cpu1.clear();
    cfg_cpu2.clear();
    cfg_cpu3.clear();
    cfg_cpu.clear();
    cfg_apar.clear();
    cfg_irq.clear();
    cfg_gic_mbist.clear();
    cfg_gic.clear();
    sta_gic_mbist.clear();
    sta_apms.clear();
    cfg_CORE_BIST.clear();
    sta_CORE_BIST.clear();
    sta_AT.clear();
    cfg_AT.clear();
    sta_timer.clear();
    sta_pmu.clear();
    cfg_pmu.clear();
    cfg_EV.clear();
    sta_EV.clear();
    cfg_L2.clear();
    sta_L2.clear();
    sta_IRQ.clear();
    sta_misc.clear();
    cfg_CTI.clear();
    sta_CTI.clear();
    cfg_AUTH.clear();
    cfg_VSIG.clear();
    sta_VSIG.clear();
    sta_misc_dbg.clear();
    cfg_misc_dbg.clear();
    cfg_dstream.clear();
    sta_dstream.clear();
    sta_nts.clear();
    cfg_nts.clear();
    cfg_ACP.clear();
    ap_cfg.clear();
    apb.clear();
}

void cap_ap_csr_ap_cfg_t::init() {

}

void cap_ap_csr_cfg_ACP_t::init() {

}

void cap_ap_csr_cfg_nts_t::init() {

}

void cap_ap_csr_sta_nts_t::init() {

}

void cap_ap_csr_sta_dstream_t::init() {

}

void cap_ap_csr_cfg_dstream_t::init() {

}

void cap_ap_csr_cfg_misc_dbg_t::init() {

    set_reset_val(cpp_int("0xc0281"));
    all(get_reset_val());
}

void cap_ap_csr_sta_misc_dbg_t::init() {

}

void cap_ap_csr_sta_VSIG_t::init() {

}

void cap_ap_csr_cfg_VSIG_t::init() {

}

void cap_ap_csr_cfg_AUTH_t::init() {

    set_reset_val(cpp_int("0xffff"));
    all(get_reset_val());
}

void cap_ap_csr_sta_CTI_t::init() {

}

void cap_ap_csr_cfg_CTI_t::init() {

}

void cap_ap_csr_sta_misc_t::init() {

}

void cap_ap_csr_sta_IRQ_t::init() {

}

void cap_ap_csr_sta_L2_t::init() {

}

void cap_ap_csr_cfg_L2_t::init() {

}

void cap_ap_csr_sta_EV_t::init() {

}

void cap_ap_csr_cfg_EV_t::init() {

}

void cap_ap_csr_cfg_pmu_t::init() {

}

void cap_ap_csr_sta_pmu_t::init() {

}

void cap_ap_csr_sta_timer_t::init() {

}

void cap_ap_csr_cfg_AT_t::init() {

}

void cap_ap_csr_sta_AT_t::init() {

}

void cap_ap_csr_sta_CORE_BIST_t::init() {

}

void cap_ap_csr_cfg_CORE_BIST_t::init() {

}

void cap_ap_csr_sta_apms_t::init() {

}

void cap_ap_csr_sta_gic_mbist_t::init() {

}

void cap_ap_csr_cfg_gic_t::init() {

    set_reset_val(cpp_int("0x3f"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_gic_mbist_t::init() {

    set_reset_val(cpp_int("0x20000"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_irq_t::init() {

    set_reset_val(cpp_int("0xfffff"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_apar_t::init() {

}

void cap_ap_csr_cfg_cpu_t::init() {

    set_reset_val(cpp_int("0xf"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_cpu3_t::init() {

    set_reset_val(cpp_int("0x400000"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_cpu2_t::init() {

    set_reset_val(cpp_int("0x400000"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_cpu1_t::init() {

    set_reset_val(cpp_int("0x400000"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_cpu0_t::init() {

    set_reset_val(cpp_int("0x400000"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_cpu0_flash_t::init() {

    set_reset_val(cpp_int("0x70100000"));
    all(get_reset_val());
}

void cap_ap_csr_cfg_static_t::init() {

    set_reset_val(cpp_int("0x600003fc000000"));
    all(get_reset_val());
}

void cap_ap_csr_dummy_t::init() {

}

void cap_ap_csr_base_t::init() {

    set_reset_val(cpp_int("0x1"));
    all(get_reset_val());
}

void cap_ap_csr_t::init() {

    base.set_attributes(this,"base", 0x0 );
    dummy.set_attributes(this,"dummy", 0x200000 );
    cfg_static.set_attributes(this,"cfg_static", 0x200010 );
    cfg_cpu0_flash.set_attributes(this,"cfg_cpu0_flash", 0x200020 );
    cfg_cpu0.set_attributes(this,"cfg_cpu0", 0x200028 );
    cfg_cpu1.set_attributes(this,"cfg_cpu1", 0x200030 );
    cfg_cpu2.set_attributes(this,"cfg_cpu2", 0x200038 );
    cfg_cpu3.set_attributes(this,"cfg_cpu3", 0x200040 );
    cfg_cpu.set_attributes(this,"cfg_cpu", 0x200048 );
    cfg_apar.set_attributes(this,"cfg_apar", 0x20004c );
    cfg_irq.set_attributes(this,"cfg_irq", 0x200050 );
    cfg_gic_mbist.set_attributes(this,"cfg_gic_mbist", 0x200060 );
    cfg_gic.set_attributes(this,"cfg_gic", 0x200070 );
    sta_gic_mbist.set_attributes(this,"sta_gic_mbist", 0x200080 );
    sta_apms.set_attributes(this,"sta_apms", 0x200090 );
    cfg_CORE_BIST.set_attributes(this,"cfg_CORE_BIST", 0x200094 );
    sta_CORE_BIST.set_attributes(this,"sta_CORE_BIST", 0x200100 );
    sta_AT.set_attributes(this,"sta_AT", 0x200180 );
    cfg_AT.set_attributes(this,"cfg_AT", 0x2001a0 );
    sta_timer.set_attributes(this,"sta_timer", 0x2001a4 );
    sta_pmu.set_attributes(this,"sta_pmu", 0x2001b0 );
    cfg_pmu.set_attributes(this,"cfg_pmu", 0x2001c0 );
    cfg_EV.set_attributes(this,"cfg_EV", 0x2001c4 );
    sta_EV.set_attributes(this,"sta_EV", 0x2001c8 );
    cfg_L2.set_attributes(this,"cfg_L2", 0x2001cc );
    sta_L2.set_attributes(this,"sta_L2", 0x2001d0 );
    sta_IRQ.set_attributes(this,"sta_IRQ", 0x2001d4 );
    sta_misc.set_attributes(this,"sta_misc", 0x2001d8 );
    cfg_CTI.set_attributes(this,"cfg_CTI", 0x2001e0 );
    sta_CTI.set_attributes(this,"sta_CTI", 0x2001e4 );
    cfg_AUTH.set_attributes(this,"cfg_AUTH", 0x2001e8 );
    cfg_VSIG.set_attributes(this,"cfg_VSIG", 0x2001ec );
    sta_VSIG.set_attributes(this,"sta_VSIG", 0x2001f0 );
    sta_misc_dbg.set_attributes(this,"sta_misc_dbg", 0x2001f4 );
    cfg_misc_dbg.set_attributes(this,"cfg_misc_dbg", 0x2001f8 );
    cfg_dstream.set_attributes(this,"cfg_dstream", 0x200200 );
    sta_dstream.set_attributes(this,"sta_dstream", 0x200204 );
    sta_nts.set_attributes(this,"sta_nts", 0x200208 );
    cfg_nts.set_attributes(this,"cfg_nts", 0x20020c );
    cfg_ACP.set_attributes(this,"cfg_ACP", 0x200210 );
    ap_cfg.set_attributes(this,"ap_cfg", 0x200214 );
    apb.set_attributes(this,"apb", 0x100000 );
}

void cap_ap_csr_ap_cfg_t::debug_port_enable(const cpp_int & _val) { 
    // debug_port_enable
    int_var__debug_port_enable = _val.convert_to< debug_port_enable_cpp_int_t >();
}

cpp_int cap_ap_csr_ap_cfg_t::debug_port_enable() const {
    return int_var__debug_port_enable;
}
    
void cap_ap_csr_ap_cfg_t::debug_port_select(const cpp_int & _val) { 
    // debug_port_select
    int_var__debug_port_select = _val.convert_to< debug_port_select_cpp_int_t >();
}

cpp_int cap_ap_csr_ap_cfg_t::debug_port_select() const {
    return int_var__debug_port_select;
}
    
void cap_ap_csr_ap_cfg_t::cfg_stall_on_same_id(const cpp_int & _val) { 
    // cfg_stall_on_same_id
    int_var__cfg_stall_on_same_id = _val.convert_to< cfg_stall_on_same_id_cpp_int_t >();
}

cpp_int cap_ap_csr_ap_cfg_t::cfg_stall_on_same_id() const {
    return int_var__cfg_stall_on_same_id;
}
    
void cap_ap_csr_cfg_ACP_t::AINACTS(const cpp_int & _val) { 
    // AINACTS
    int_var__AINACTS = _val.convert_to< AINACTS_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_ACP_t::AINACTS() const {
    return int_var__AINACTS;
}
    
void cap_ap_csr_cfg_ACP_t::ACINACTM(const cpp_int & _val) { 
    // ACINACTM
    int_var__ACINACTM = _val.convert_to< ACINACTM_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_ACP_t::ACINACTM() const {
    return int_var__ACINACTM;
}
    
void cap_ap_csr_cfg_nts_t::TSBITS_TS(const cpp_int & _val) { 
    // TSBITS_TS
    int_var__TSBITS_TS = _val.convert_to< TSBITS_TS_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_nts_t::TSBITS_TS() const {
    return int_var__TSBITS_TS;
}
    
void cap_ap_csr_cfg_nts_t::TSSYNCS_TS(const cpp_int & _val) { 
    // TSSYNCS_TS
    int_var__TSSYNCS_TS = _val.convert_to< TSSYNCS_TS_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_nts_t::TSSYNCS_TS() const {
    return int_var__TSSYNCS_TS;
}
    
void cap_ap_csr_sta_nts_t::TSSYNCREADYS_TS(const cpp_int & _val) { 
    // TSSYNCREADYS_TS
    int_var__TSSYNCREADYS_TS = _val.convert_to< TSSYNCREADYS_TS_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_nts_t::TSSYNCREADYS_TS() const {
    return int_var__TSSYNCREADYS_TS;
}
    
void cap_ap_csr_sta_dstream_t::DBGACK(const cpp_int & _val) { 
    // DBGACK
    int_var__DBGACK = _val.convert_to< DBGACK_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_dstream_t::DBGACK() const {
    return int_var__DBGACK;
}
    
void cap_ap_csr_sta_dstream_t::DBGRSTREQ(const cpp_int & _val) { 
    // DBGRSTREQ
    int_var__DBGRSTREQ = _val.convert_to< DBGRSTREQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_dstream_t::DBGRSTREQ() const {
    return int_var__DBGRSTREQ;
}
    
void cap_ap_csr_cfg_dstream_t::EDBGRQ(const cpp_int & _val) { 
    // EDBGRQ
    int_var__EDBGRQ = _val.convert_to< EDBGRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_dstream_t::EDBGRQ() const {
    return int_var__EDBGRQ;
}
    
void cap_ap_csr_cfg_misc_dbg_t::DBGROMADDRV(const cpp_int & _val) { 
    // DBGROMADDRV
    int_var__DBGROMADDRV = _val.convert_to< DBGROMADDRV_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_misc_dbg_t::DBGROMADDRV() const {
    return int_var__DBGROMADDRV;
}
    
void cap_ap_csr_cfg_misc_dbg_t::DBGROMADDR(const cpp_int & _val) { 
    // DBGROMADDR
    int_var__DBGROMADDR = _val.convert_to< DBGROMADDR_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_misc_dbg_t::DBGROMADDR() const {
    return int_var__DBGROMADDR;
}
    
void cap_ap_csr_sta_misc_dbg_t::COMMRX(const cpp_int & _val) { 
    // COMMRX
    int_var__COMMRX = _val.convert_to< COMMRX_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_dbg_t::COMMRX() const {
    return int_var__COMMRX;
}
    
void cap_ap_csr_sta_misc_dbg_t::COMMTX(const cpp_int & _val) { 
    // COMMTX
    int_var__COMMTX = _val.convert_to< COMMTX_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_dbg_t::COMMTX() const {
    return int_var__COMMTX;
}
    
void cap_ap_csr_sta_misc_dbg_t::nCOMMIRQ(const cpp_int & _val) { 
    // nCOMMIRQ
    int_var__nCOMMIRQ = _val.convert_to< nCOMMIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_dbg_t::nCOMMIRQ() const {
    return int_var__nCOMMIRQ;
}
    
void cap_ap_csr_sta_VSIG_t::CLREXMONACK(const cpp_int & _val) { 
    // CLREXMONACK
    int_var__CLREXMONACK = _val.convert_to< CLREXMONACK_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_VSIG_t::CLREXMONACK() const {
    return int_var__CLREXMONACK;
}
    
void cap_ap_csr_cfg_VSIG_t::CLREXMONREQ(const cpp_int & _val) { 
    // CLREXMONREQ
    int_var__CLREXMONREQ = _val.convert_to< CLREXMONREQ_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_VSIG_t::CLREXMONREQ() const {
    return int_var__CLREXMONREQ;
}
    
void cap_ap_csr_cfg_AUTH_t::DBGEN(const cpp_int & _val) { 
    // DBGEN
    int_var__DBGEN = _val.convert_to< DBGEN_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AUTH_t::DBGEN() const {
    return int_var__DBGEN;
}
    
void cap_ap_csr_cfg_AUTH_t::SPNIDEN(const cpp_int & _val) { 
    // SPNIDEN
    int_var__SPNIDEN = _val.convert_to< SPNIDEN_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AUTH_t::SPNIDEN() const {
    return int_var__SPNIDEN;
}
    
void cap_ap_csr_cfg_AUTH_t::SPIDEN(const cpp_int & _val) { 
    // SPIDEN
    int_var__SPIDEN = _val.convert_to< SPIDEN_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AUTH_t::SPIDEN() const {
    return int_var__SPIDEN;
}
    
void cap_ap_csr_cfg_AUTH_t::NIDEN(const cpp_int & _val) { 
    // NIDEN
    int_var__NIDEN = _val.convert_to< NIDEN_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AUTH_t::NIDEN() const {
    return int_var__NIDEN;
}
    
void cap_ap_csr_sta_CTI_t::CTICHOUT(const cpp_int & _val) { 
    // CTICHOUT
    int_var__CTICHOUT = _val.convert_to< CTICHOUT_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_CTI_t::CTICHOUT() const {
    return int_var__CTICHOUT;
}
    
void cap_ap_csr_sta_CTI_t::CTICHINACK(const cpp_int & _val) { 
    // CTICHINACK
    int_var__CTICHINACK = _val.convert_to< CTICHINACK_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_CTI_t::CTICHINACK() const {
    return int_var__CTICHINACK;
}
    
void cap_ap_csr_sta_CTI_t::CTIIRQ(const cpp_int & _val) { 
    // CTIIRQ
    int_var__CTIIRQ = _val.convert_to< CTIIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_CTI_t::CTIIRQ() const {
    return int_var__CTIIRQ;
}
    
void cap_ap_csr_cfg_CTI_t::CTICHIN(const cpp_int & _val) { 
    // CTICHIN
    int_var__CTICHIN = _val.convert_to< CTICHIN_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CTI_t::CTICHIN() const {
    return int_var__CTICHIN;
}
    
void cap_ap_csr_cfg_CTI_t::CTICHOUTACK(const cpp_int & _val) { 
    // CTICHOUTACK
    int_var__CTICHOUTACK = _val.convert_to< CTICHOUTACK_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CTI_t::CTICHOUTACK() const {
    return int_var__CTICHOUTACK;
}
    
void cap_ap_csr_cfg_CTI_t::CTIIRQACK(const cpp_int & _val) { 
    // CTIIRQACK
    int_var__CTIIRQACK = _val.convert_to< CTIIRQACK_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CTI_t::CTIIRQACK() const {
    return int_var__CTIIRQACK;
}
    
void cap_ap_csr_sta_misc_t::WARMRSTREQ(const cpp_int & _val) { 
    // WARMRSTREQ
    int_var__WARMRSTREQ = _val.convert_to< WARMRSTREQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::WARMRSTREQ() const {
    return int_var__WARMRSTREQ;
}
    
void cap_ap_csr_sta_misc_t::WRMEMATTRM(const cpp_int & _val) { 
    // WRMEMATTRM
    int_var__WRMEMATTRM = _val.convert_to< WRMEMATTRM_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::WRMEMATTRM() const {
    return int_var__WRMEMATTRM;
}
    
void cap_ap_csr_sta_misc_t::RDMEMATTRM(const cpp_int & _val) { 
    // RDMEMATTRM
    int_var__RDMEMATTRM = _val.convert_to< RDMEMATTRM_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::RDMEMATTRM() const {
    return int_var__RDMEMATTRM;
}
    
void cap_ap_csr_sta_misc_t::SMPEN(const cpp_int & _val) { 
    // SMPEN
    int_var__SMPEN = _val.convert_to< SMPEN_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::SMPEN() const {
    return int_var__SMPEN;
}
    
void cap_ap_csr_sta_misc_t::STANDBYWFE(const cpp_int & _val) { 
    // STANDBYWFE
    int_var__STANDBYWFE = _val.convert_to< STANDBYWFE_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::STANDBYWFE() const {
    return int_var__STANDBYWFE;
}
    
void cap_ap_csr_sta_misc_t::STANDBYWFI(const cpp_int & _val) { 
    // STANDBYWFI
    int_var__STANDBYWFI = _val.convert_to< STANDBYWFI_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::STANDBYWFI() const {
    return int_var__STANDBYWFI;
}
    
void cap_ap_csr_sta_misc_t::STANDBYWFIL2(const cpp_int & _val) { 
    // STANDBYWFIL2
    int_var__STANDBYWFIL2 = _val.convert_to< STANDBYWFIL2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_misc_t::STANDBYWFIL2() const {
    return int_var__STANDBYWFIL2;
}
    
void cap_ap_csr_sta_IRQ_t::nEXTERRIRQ(const cpp_int & _val) { 
    // nEXTERRIRQ
    int_var__nEXTERRIRQ = _val.convert_to< nEXTERRIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_IRQ_t::nEXTERRIRQ() const {
    return int_var__nEXTERRIRQ;
}
    
void cap_ap_csr_sta_IRQ_t::nINTERRIRQ(const cpp_int & _val) { 
    // nINTERRIRQ
    int_var__nINTERRIRQ = _val.convert_to< nINTERRIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_IRQ_t::nINTERRIRQ() const {
    return int_var__nINTERRIRQ;
}
    
void cap_ap_csr_sta_IRQ_t::nVCPUMNTIRQ(const cpp_int & _val) { 
    // nVCPUMNTIRQ
    int_var__nVCPUMNTIRQ = _val.convert_to< nVCPUMNTIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_IRQ_t::nVCPUMNTIRQ() const {
    return int_var__nVCPUMNTIRQ;
}
    
void cap_ap_csr_sta_L2_t::L2FLUSHDONE(const cpp_int & _val) { 
    // L2FLUSHDONE
    int_var__L2FLUSHDONE = _val.convert_to< L2FLUSHDONE_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_L2_t::L2FLUSHDONE() const {
    return int_var__L2FLUSHDONE;
}
    
void cap_ap_csr_cfg_L2_t::L2FLUSHREQ(const cpp_int & _val) { 
    // L2FLUSHREQ
    int_var__L2FLUSHREQ = _val.convert_to< L2FLUSHREQ_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_L2_t::L2FLUSHREQ() const {
    return int_var__L2FLUSHREQ;
}
    
void cap_ap_csr_sta_EV_t::EVENTO(const cpp_int & _val) { 
    // EVENTO
    int_var__EVENTO = _val.convert_to< EVENTO_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_EV_t::EVENTO() const {
    return int_var__EVENTO;
}
    
void cap_ap_csr_cfg_EV_t::EVENTI(const cpp_int & _val) { 
    // EVENTI
    int_var__EVENTI = _val.convert_to< EVENTI_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_EV_t::EVENTI() const {
    return int_var__EVENTI;
}
    
void cap_ap_csr_cfg_pmu_t::PMUSNAPSHOTREQ(const cpp_int & _val) { 
    // PMUSNAPSHOTREQ
    int_var__PMUSNAPSHOTREQ = _val.convert_to< PMUSNAPSHOTREQ_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_pmu_t::PMUSNAPSHOTREQ() const {
    return int_var__PMUSNAPSHOTREQ;
}
    
void cap_ap_csr_sta_pmu_t::PMUEVENT0(const cpp_int & _val) { 
    // PMUEVENT0
    int_var__PMUEVENT0 = _val.convert_to< PMUEVENT0_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_pmu_t::PMUEVENT0() const {
    return int_var__PMUEVENT0;
}
    
void cap_ap_csr_sta_pmu_t::PMUEVENT1(const cpp_int & _val) { 
    // PMUEVENT1
    int_var__PMUEVENT1 = _val.convert_to< PMUEVENT1_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_pmu_t::PMUEVENT1() const {
    return int_var__PMUEVENT1;
}
    
void cap_ap_csr_sta_pmu_t::PMUEVENT2(const cpp_int & _val) { 
    // PMUEVENT2
    int_var__PMUEVENT2 = _val.convert_to< PMUEVENT2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_pmu_t::PMUEVENT2() const {
    return int_var__PMUEVENT2;
}
    
void cap_ap_csr_sta_pmu_t::PMUEVENT3(const cpp_int & _val) { 
    // PMUEVENT3
    int_var__PMUEVENT3 = _val.convert_to< PMUEVENT3_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_pmu_t::PMUEVENT3() const {
    return int_var__PMUEVENT3;
}
    
void cap_ap_csr_sta_pmu_t::nPMUIRQ(const cpp_int & _val) { 
    // nPMUIRQ
    int_var__nPMUIRQ = _val.convert_to< nPMUIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_pmu_t::nPMUIRQ() const {
    return int_var__nPMUIRQ;
}
    
void cap_ap_csr_sta_pmu_t::PMUSNAPSHOTACK(const cpp_int & _val) { 
    // PMUSNAPSHOTACK
    int_var__PMUSNAPSHOTACK = _val.convert_to< PMUSNAPSHOTACK_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_pmu_t::PMUSNAPSHOTACK() const {
    return int_var__PMUSNAPSHOTACK;
}
    
void cap_ap_csr_sta_timer_t::nCNTHPIRQ(const cpp_int & _val) { 
    // nCNTHPIRQ
    int_var__nCNTHPIRQ = _val.convert_to< nCNTHPIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_timer_t::nCNTHPIRQ() const {
    return int_var__nCNTHPIRQ;
}
    
void cap_ap_csr_sta_timer_t::nCNTPNSIRQ(const cpp_int & _val) { 
    // nCNTPNSIRQ
    int_var__nCNTPNSIRQ = _val.convert_to< nCNTPNSIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_timer_t::nCNTPNSIRQ() const {
    return int_var__nCNTPNSIRQ;
}
    
void cap_ap_csr_sta_timer_t::nCNTPSIRQ(const cpp_int & _val) { 
    // nCNTPSIRQ
    int_var__nCNTPSIRQ = _val.convert_to< nCNTPSIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_timer_t::nCNTPSIRQ() const {
    return int_var__nCNTPSIRQ;
}
    
void cap_ap_csr_sta_timer_t::nCNTVIRQ(const cpp_int & _val) { 
    // nCNTVIRQ
    int_var__nCNTVIRQ = _val.convert_to< nCNTVIRQ_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_timer_t::nCNTVIRQ() const {
    return int_var__nCNTVIRQ;
}
    
void cap_ap_csr_cfg_AT_t::AFVALIDM0(const cpp_int & _val) { 
    // AFVALIDM0
    int_var__AFVALIDM0 = _val.convert_to< AFVALIDM0_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::AFVALIDM0() const {
    return int_var__AFVALIDM0;
}
    
void cap_ap_csr_cfg_AT_t::AFVALIDM1(const cpp_int & _val) { 
    // AFVALIDM1
    int_var__AFVALIDM1 = _val.convert_to< AFVALIDM1_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::AFVALIDM1() const {
    return int_var__AFVALIDM1;
}
    
void cap_ap_csr_cfg_AT_t::AFVALIDM2(const cpp_int & _val) { 
    // AFVALIDM2
    int_var__AFVALIDM2 = _val.convert_to< AFVALIDM2_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::AFVALIDM2() const {
    return int_var__AFVALIDM2;
}
    
void cap_ap_csr_cfg_AT_t::AFVALIDM3(const cpp_int & _val) { 
    // AFVALIDM3
    int_var__AFVALIDM3 = _val.convert_to< AFVALIDM3_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::AFVALIDM3() const {
    return int_var__AFVALIDM3;
}
    
void cap_ap_csr_cfg_AT_t::ATREADYM0(const cpp_int & _val) { 
    // ATREADYM0
    int_var__ATREADYM0 = _val.convert_to< ATREADYM0_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::ATREADYM0() const {
    return int_var__ATREADYM0;
}
    
void cap_ap_csr_cfg_AT_t::ATREADYM1(const cpp_int & _val) { 
    // ATREADYM1
    int_var__ATREADYM1 = _val.convert_to< ATREADYM1_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::ATREADYM1() const {
    return int_var__ATREADYM1;
}
    
void cap_ap_csr_cfg_AT_t::ATREADYM2(const cpp_int & _val) { 
    // ATREADYM2
    int_var__ATREADYM2 = _val.convert_to< ATREADYM2_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::ATREADYM2() const {
    return int_var__ATREADYM2;
}
    
void cap_ap_csr_cfg_AT_t::ATREADYM3(const cpp_int & _val) { 
    // ATREADYM3
    int_var__ATREADYM3 = _val.convert_to< ATREADYM3_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::ATREADYM3() const {
    return int_var__ATREADYM3;
}
    
void cap_ap_csr_cfg_AT_t::SYNCREQM0(const cpp_int & _val) { 
    // SYNCREQM0
    int_var__SYNCREQM0 = _val.convert_to< SYNCREQM0_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::SYNCREQM0() const {
    return int_var__SYNCREQM0;
}
    
void cap_ap_csr_cfg_AT_t::SYNCREQM1(const cpp_int & _val) { 
    // SYNCREQM1
    int_var__SYNCREQM1 = _val.convert_to< SYNCREQM1_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::SYNCREQM1() const {
    return int_var__SYNCREQM1;
}
    
void cap_ap_csr_cfg_AT_t::SYNCREQM2(const cpp_int & _val) { 
    // SYNCREQM2
    int_var__SYNCREQM2 = _val.convert_to< SYNCREQM2_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::SYNCREQM2() const {
    return int_var__SYNCREQM2;
}
    
void cap_ap_csr_cfg_AT_t::SYNCREQM3(const cpp_int & _val) { 
    // SYNCREQM3
    int_var__SYNCREQM3 = _val.convert_to< SYNCREQM3_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_AT_t::SYNCREQM3() const {
    return int_var__SYNCREQM3;
}
    
void cap_ap_csr_sta_AT_t::AFREADYM0(const cpp_int & _val) { 
    // AFREADYM0
    int_var__AFREADYM0 = _val.convert_to< AFREADYM0_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::AFREADYM0() const {
    return int_var__AFREADYM0;
}
    
void cap_ap_csr_sta_AT_t::AFREADYM1(const cpp_int & _val) { 
    // AFREADYM1
    int_var__AFREADYM1 = _val.convert_to< AFREADYM1_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::AFREADYM1() const {
    return int_var__AFREADYM1;
}
    
void cap_ap_csr_sta_AT_t::AFREADYM2(const cpp_int & _val) { 
    // AFREADYM2
    int_var__AFREADYM2 = _val.convert_to< AFREADYM2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::AFREADYM2() const {
    return int_var__AFREADYM2;
}
    
void cap_ap_csr_sta_AT_t::AFREADYM3(const cpp_int & _val) { 
    // AFREADYM3
    int_var__AFREADYM3 = _val.convert_to< AFREADYM3_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::AFREADYM3() const {
    return int_var__AFREADYM3;
}
    
void cap_ap_csr_sta_AT_t::ATBYTESM0(const cpp_int & _val) { 
    // ATBYTESM0
    int_var__ATBYTESM0 = _val.convert_to< ATBYTESM0_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATBYTESM0() const {
    return int_var__ATBYTESM0;
}
    
void cap_ap_csr_sta_AT_t::ATBYTESM1(const cpp_int & _val) { 
    // ATBYTESM1
    int_var__ATBYTESM1 = _val.convert_to< ATBYTESM1_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATBYTESM1() const {
    return int_var__ATBYTESM1;
}
    
void cap_ap_csr_sta_AT_t::ATBYTESM2(const cpp_int & _val) { 
    // ATBYTESM2
    int_var__ATBYTESM2 = _val.convert_to< ATBYTESM2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATBYTESM2() const {
    return int_var__ATBYTESM2;
}
    
void cap_ap_csr_sta_AT_t::ATBYTESM3(const cpp_int & _val) { 
    // ATBYTESM3
    int_var__ATBYTESM3 = _val.convert_to< ATBYTESM3_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATBYTESM3() const {
    return int_var__ATBYTESM3;
}
    
void cap_ap_csr_sta_AT_t::ATDATAM0(const cpp_int & _val) { 
    // ATDATAM0
    int_var__ATDATAM0 = _val.convert_to< ATDATAM0_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATDATAM0() const {
    return int_var__ATDATAM0;
}
    
void cap_ap_csr_sta_AT_t::ATDATAM1(const cpp_int & _val) { 
    // ATDATAM1
    int_var__ATDATAM1 = _val.convert_to< ATDATAM1_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATDATAM1() const {
    return int_var__ATDATAM1;
}
    
void cap_ap_csr_sta_AT_t::ATDATAM2(const cpp_int & _val) { 
    // ATDATAM2
    int_var__ATDATAM2 = _val.convert_to< ATDATAM2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATDATAM2() const {
    return int_var__ATDATAM2;
}
    
void cap_ap_csr_sta_AT_t::ATDATAM3(const cpp_int & _val) { 
    // ATDATAM3
    int_var__ATDATAM3 = _val.convert_to< ATDATAM3_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATDATAM3() const {
    return int_var__ATDATAM3;
}
    
void cap_ap_csr_sta_AT_t::ATIDM0(const cpp_int & _val) { 
    // ATIDM0
    int_var__ATIDM0 = _val.convert_to< ATIDM0_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATIDM0() const {
    return int_var__ATIDM0;
}
    
void cap_ap_csr_sta_AT_t::ATIDM1(const cpp_int & _val) { 
    // ATIDM1
    int_var__ATIDM1 = _val.convert_to< ATIDM1_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATIDM1() const {
    return int_var__ATIDM1;
}
    
void cap_ap_csr_sta_AT_t::ATIDM2(const cpp_int & _val) { 
    // ATIDM2
    int_var__ATIDM2 = _val.convert_to< ATIDM2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATIDM2() const {
    return int_var__ATIDM2;
}
    
void cap_ap_csr_sta_AT_t::ATIDM3(const cpp_int & _val) { 
    // ATIDM3
    int_var__ATIDM3 = _val.convert_to< ATIDM3_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATIDM3() const {
    return int_var__ATIDM3;
}
    
void cap_ap_csr_sta_AT_t::ATVALIDM0(const cpp_int & _val) { 
    // ATVALIDM0
    int_var__ATVALIDM0 = _val.convert_to< ATVALIDM0_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATVALIDM0() const {
    return int_var__ATVALIDM0;
}
    
void cap_ap_csr_sta_AT_t::ATVALIDM1(const cpp_int & _val) { 
    // ATVALIDM1
    int_var__ATVALIDM1 = _val.convert_to< ATVALIDM1_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATVALIDM1() const {
    return int_var__ATVALIDM1;
}
    
void cap_ap_csr_sta_AT_t::ATVALIDM2(const cpp_int & _val) { 
    // ATVALIDM2
    int_var__ATVALIDM2 = _val.convert_to< ATVALIDM2_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATVALIDM2() const {
    return int_var__ATVALIDM2;
}
    
void cap_ap_csr_sta_AT_t::ATVALIDM3(const cpp_int & _val) { 
    // ATVALIDM3
    int_var__ATVALIDM3 = _val.convert_to< ATVALIDM3_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_AT_t::ATVALIDM3() const {
    return int_var__ATVALIDM3;
}
    
void cap_ap_csr_sta_CORE_BIST_t::DONE_FAIL_OUT(const cpp_int & _val) { 
    // DONE_FAIL_OUT
    int_var__DONE_FAIL_OUT = _val.convert_to< DONE_FAIL_OUT_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_CORE_BIST_t::DONE_FAIL_OUT() const {
    return int_var__DONE_FAIL_OUT;
}
    
void cap_ap_csr_sta_CORE_BIST_t::DONE_PASS_OUT(const cpp_int & _val) { 
    // DONE_PASS_OUT
    int_var__DONE_PASS_OUT = _val.convert_to< DONE_PASS_OUT_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_CORE_BIST_t::DONE_PASS_OUT() const {
    return int_var__DONE_PASS_OUT;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::PTRN_FILL(const cpp_int & _val) { 
    // PTRN_FILL
    int_var__PTRN_FILL = _val.convert_to< PTRN_FILL_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::PTRN_FILL() const {
    return int_var__PTRN_FILL;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::RPR_MODE(const cpp_int & _val) { 
    // RPR_MODE
    int_var__RPR_MODE = _val.convert_to< RPR_MODE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::RPR_MODE() const {
    return int_var__RPR_MODE;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::RUN(const cpp_int & _val) { 
    // RUN
    int_var__RUN = _val.convert_to< RUN_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::RUN() const {
    return int_var__RUN;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::SEL_CLK(const cpp_int & _val) { 
    // SEL_CLK
    int_var__SEL_CLK = _val.convert_to< SEL_CLK_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::SEL_CLK() const {
    return int_var__SEL_CLK;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::SEL_L2DCLK(const cpp_int & _val) { 
    // SEL_L2DCLK
    int_var__SEL_L2DCLK = _val.convert_to< SEL_L2DCLK_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::SEL_L2DCLK() const {
    return int_var__SEL_L2DCLK;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::SEL_L2TCLK(const cpp_int & _val) { 
    // SEL_L2TCLK
    int_var__SEL_L2TCLK = _val.convert_to< SEL_L2TCLK_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::SEL_L2TCLK() const {
    return int_var__SEL_L2TCLK;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::MEM_RST(const cpp_int & _val) { 
    // MEM_RST
    int_var__MEM_RST = _val.convert_to< MEM_RST_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::MEM_RST() const {
    return int_var__MEM_RST;
}
    
void cap_ap_csr_cfg_CORE_BIST_t::RESET_OVERRIDE(const cpp_int & _val) { 
    // RESET_OVERRIDE
    int_var__RESET_OVERRIDE = _val.convert_to< RESET_OVERRIDE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_CORE_BIST_t::RESET_OVERRIDE() const {
    return int_var__RESET_OVERRIDE;
}
    
void cap_ap_csr_sta_apms_t::fatal(const cpp_int & _val) { 
    // fatal
    int_var__fatal = _val.convert_to< fatal_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_apms_t::fatal() const {
    return int_var__fatal;
}
    
void cap_ap_csr_sta_gic_mbist_t::mbistack(const cpp_int & _val) { 
    // mbistack
    int_var__mbistack = _val.convert_to< mbistack_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_gic_mbist_t::mbistack() const {
    return int_var__mbistack;
}
    
void cap_ap_csr_sta_gic_mbist_t::mbistoutdata(const cpp_int & _val) { 
    // mbistoutdata
    int_var__mbistoutdata = _val.convert_to< mbistoutdata_cpp_int_t >();
}

cpp_int cap_ap_csr_sta_gic_mbist_t::mbistoutdata() const {
    return int_var__mbistoutdata;
}
    
void cap_ap_csr_cfg_gic_t::base_awuser(const cpp_int & _val) { 
    // base_awuser
    int_var__base_awuser = _val.convert_to< base_awuser_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_t::base_awuser() const {
    return int_var__base_awuser;
}
    
void cap_ap_csr_cfg_gic_t::base_aruser(const cpp_int & _val) { 
    // base_aruser
    int_var__base_aruser = _val.convert_to< base_aruser_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_t::base_aruser() const {
    return int_var__base_aruser;
}
    
void cap_ap_csr_cfg_gic_t::awuser(const cpp_int & _val) { 
    // awuser
    int_var__awuser = _val.convert_to< awuser_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_t::awuser() const {
    return int_var__awuser;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistaddr(const cpp_int & _val) { 
    // mbistaddr
    int_var__mbistaddr = _val.convert_to< mbistaddr_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistaddr() const {
    return int_var__mbistaddr;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistarray(const cpp_int & _val) { 
    // mbistarray
    int_var__mbistarray = _val.convert_to< mbistarray_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistarray() const {
    return int_var__mbistarray;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistcfg(const cpp_int & _val) { 
    // mbistcfg
    int_var__mbistcfg = _val.convert_to< mbistcfg_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistcfg() const {
    return int_var__mbistcfg;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistreaden(const cpp_int & _val) { 
    // mbistreaden
    int_var__mbistreaden = _val.convert_to< mbistreaden_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistreaden() const {
    return int_var__mbistreaden;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistreq(const cpp_int & _val) { 
    // mbistreq
    int_var__mbistreq = _val.convert_to< mbistreq_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistreq() const {
    return int_var__mbistreq;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistresetn(const cpp_int & _val) { 
    // mbistresetn
    int_var__mbistresetn = _val.convert_to< mbistresetn_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistresetn() const {
    return int_var__mbistresetn;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistwriteen(const cpp_int & _val) { 
    // mbistwriteen
    int_var__mbistwriteen = _val.convert_to< mbistwriteen_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistwriteen() const {
    return int_var__mbistwriteen;
}
    
void cap_ap_csr_cfg_gic_mbist_t::mbistindata(const cpp_int & _val) { 
    // mbistindata
    int_var__mbistindata = _val.convert_to< mbistindata_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_gic_mbist_t::mbistindata() const {
    return int_var__mbistindata;
}
    
void cap_ap_csr_cfg_irq_t::nFIQ_mask(const cpp_int & _val) { 
    // nFIQ_mask
    int_var__nFIQ_mask = _val.convert_to< nFIQ_mask_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_irq_t::nFIQ_mask() const {
    return int_var__nFIQ_mask;
}
    
void cap_ap_csr_cfg_irq_t::nIRQ_mask(const cpp_int & _val) { 
    // nIRQ_mask
    int_var__nIRQ_mask = _val.convert_to< nIRQ_mask_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_irq_t::nIRQ_mask() const {
    return int_var__nIRQ_mask;
}
    
void cap_ap_csr_cfg_irq_t::nREI_mask(const cpp_int & _val) { 
    // nREI_mask
    int_var__nREI_mask = _val.convert_to< nREI_mask_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_irq_t::nREI_mask() const {
    return int_var__nREI_mask;
}
    
void cap_ap_csr_cfg_irq_t::nSEI_mask(const cpp_int & _val) { 
    // nSEI_mask
    int_var__nSEI_mask = _val.convert_to< nSEI_mask_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_irq_t::nSEI_mask() const {
    return int_var__nSEI_mask;
}
    
void cap_ap_csr_cfg_irq_t::nVSEI_mask(const cpp_int & _val) { 
    // nVSEI_mask
    int_var__nVSEI_mask = _val.convert_to< nVSEI_mask_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_irq_t::nVSEI_mask() const {
    return int_var__nVSEI_mask;
}
    
void cap_ap_csr_cfg_apar_t::addr_top24(const cpp_int & _val) { 
    // addr_top24
    int_var__addr_top24 = _val.convert_to< addr_top24_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_apar_t::addr_top24() const {
    return int_var__addr_top24;
}
    
void cap_ap_csr_cfg_apar_t::awusers(const cpp_int & _val) { 
    // awusers
    int_var__awusers = _val.convert_to< awusers_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_apar_t::awusers() const {
    return int_var__awusers;
}
    
void cap_ap_csr_cfg_apar_t::arusers(const cpp_int & _val) { 
    // arusers
    int_var__arusers = _val.convert_to< arusers_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_apar_t::arusers() const {
    return int_var__arusers;
}
    
void cap_ap_csr_cfg_apar_t::addr_sel(const cpp_int & _val) { 
    // addr_sel
    int_var__addr_sel = _val.convert_to< addr_sel_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_apar_t::addr_sel() const {
    return int_var__addr_sel;
}
    
void cap_ap_csr_cfg_cpu_t::active(const cpp_int & _val) { 
    // active
    int_var__active = _val.convert_to< active_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu_t::active() const {
    return int_var__active;
}
    
void cap_ap_csr_cfg_cpu_t::sta_wake_request(const cpp_int & _val) { 
    // sta_wake_request
    int_var__sta_wake_request = _val.convert_to< sta_wake_request_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu_t::sta_wake_request() const {
    return int_var__sta_wake_request;
}
    
void cap_ap_csr_cfg_cpu3_t::RVBARADDR(const cpp_int & _val) { 
    // RVBARADDR
    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu3_t::RVBARADDR() const {
    return int_var__RVBARADDR;
}
    
void cap_ap_csr_cfg_cpu2_t::RVBARADDR(const cpp_int & _val) { 
    // RVBARADDR
    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu2_t::RVBARADDR() const {
    return int_var__RVBARADDR;
}
    
void cap_ap_csr_cfg_cpu1_t::RVBARADDR(const cpp_int & _val) { 
    // RVBARADDR
    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu1_t::RVBARADDR() const {
    return int_var__RVBARADDR;
}
    
void cap_ap_csr_cfg_cpu0_t::RVBARADDR(const cpp_int & _val) { 
    // RVBARADDR
    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu0_t::RVBARADDR() const {
    return int_var__RVBARADDR;
}
    
void cap_ap_csr_cfg_cpu0_flash_t::RVBARADDR(const cpp_int & _val) { 
    // RVBARADDR
    int_var__RVBARADDR = _val.convert_to< RVBARADDR_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_cpu0_flash_t::RVBARADDR() const {
    return int_var__RVBARADDR;
}
    
void cap_ap_csr_cfg_static_t::DBGL1RSTDISABLE(const cpp_int & _val) { 
    // DBGL1RSTDISABLE
    int_var__DBGL1RSTDISABLE = _val.convert_to< DBGL1RSTDISABLE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::DBGL1RSTDISABLE() const {
    return int_var__DBGL1RSTDISABLE;
}
    
void cap_ap_csr_cfg_static_t::L2RSTDISABLE(const cpp_int & _val) { 
    // L2RSTDISABLE
    int_var__L2RSTDISABLE = _val.convert_to< L2RSTDISABLE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::L2RSTDISABLE() const {
    return int_var__L2RSTDISABLE;
}
    
void cap_ap_csr_cfg_static_t::CP15SDISABLE(const cpp_int & _val) { 
    // CP15SDISABLE
    int_var__CP15SDISABLE = _val.convert_to< CP15SDISABLE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::CP15SDISABLE() const {
    return int_var__CP15SDISABLE;
}
    
void cap_ap_csr_cfg_static_t::CLUSTERIDAFF1(const cpp_int & _val) { 
    // CLUSTERIDAFF1
    int_var__CLUSTERIDAFF1 = _val.convert_to< CLUSTERIDAFF1_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::CLUSTERIDAFF1() const {
    return int_var__CLUSTERIDAFF1;
}
    
void cap_ap_csr_cfg_static_t::CLUSTERIDAFF2(const cpp_int & _val) { 
    // CLUSTERIDAFF2
    int_var__CLUSTERIDAFF2 = _val.convert_to< CLUSTERIDAFF2_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::CLUSTERIDAFF2() const {
    return int_var__CLUSTERIDAFF2;
}
    
void cap_ap_csr_cfg_static_t::CRYPTODISABLE(const cpp_int & _val) { 
    // CRYPTODISABLE
    int_var__CRYPTODISABLE = _val.convert_to< CRYPTODISABLE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::CRYPTODISABLE() const {
    return int_var__CRYPTODISABLE;
}
    
void cap_ap_csr_cfg_static_t::AA64nAA32(const cpp_int & _val) { 
    // AA64nAA32
    int_var__AA64nAA32 = _val.convert_to< AA64nAA32_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::AA64nAA32() const {
    return int_var__AA64nAA32;
}
    
void cap_ap_csr_cfg_static_t::SYSBARDISABLE(const cpp_int & _val) { 
    // SYSBARDISABLE
    int_var__SYSBARDISABLE = _val.convert_to< SYSBARDISABLE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::SYSBARDISABLE() const {
    return int_var__SYSBARDISABLE;
}
    
void cap_ap_csr_cfg_static_t::BROADCASTCACHEMAINT(const cpp_int & _val) { 
    // BROADCASTCACHEMAINT
    int_var__BROADCASTCACHEMAINT = _val.convert_to< BROADCASTCACHEMAINT_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::BROADCASTCACHEMAINT() const {
    return int_var__BROADCASTCACHEMAINT;
}
    
void cap_ap_csr_cfg_static_t::BROADCASTINNER(const cpp_int & _val) { 
    // BROADCASTINNER
    int_var__BROADCASTINNER = _val.convert_to< BROADCASTINNER_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::BROADCASTINNER() const {
    return int_var__BROADCASTINNER;
}
    
void cap_ap_csr_cfg_static_t::BROADCASTOUTER(const cpp_int & _val) { 
    // BROADCASTOUTER
    int_var__BROADCASTOUTER = _val.convert_to< BROADCASTOUTER_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::BROADCASTOUTER() const {
    return int_var__BROADCASTOUTER;
}
    
void cap_ap_csr_cfg_static_t::CFGEND(const cpp_int & _val) { 
    // CFGEND
    int_var__CFGEND = _val.convert_to< CFGEND_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::CFGEND() const {
    return int_var__CFGEND;
}
    
void cap_ap_csr_cfg_static_t::CFGTE(const cpp_int & _val) { 
    // CFGTE
    int_var__CFGTE = _val.convert_to< CFGTE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::CFGTE() const {
    return int_var__CFGTE;
}
    
void cap_ap_csr_cfg_static_t::PERIPHBASE(const cpp_int & _val) { 
    // PERIPHBASE
    int_var__PERIPHBASE = _val.convert_to< PERIPHBASE_cpp_int_t >();
}

cpp_int cap_ap_csr_cfg_static_t::PERIPHBASE() const {
    return int_var__PERIPHBASE;
}
    
void cap_ap_csr_dummy_t::cycle_counter(const cpp_int & _val) { 
    // cycle_counter
    int_var__cycle_counter = _val.convert_to< cycle_counter_cpp_int_t >();
}

cpp_int cap_ap_csr_dummy_t::cycle_counter() const {
    return int_var__cycle_counter;
}
    
void cap_ap_csr_base_t::scratch_reg(const cpp_int & _val) { 
    // scratch_reg
    int_var__scratch_reg = _val.convert_to< scratch_reg_cpp_int_t >();
}

cpp_int cap_ap_csr_base_t::scratch_reg() const {
    return int_var__scratch_reg;
}
    
#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_ap_cfg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "debug_port_enable")) { field_val = debug_port_enable(); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_port_select")) { field_val = debug_port_select(); field_found=1; }
    if(!field_found && !strcmp(field_name, "cfg_stall_on_same_id")) { field_val = cfg_stall_on_same_id(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_ACP_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "AINACTS")) { field_val = AINACTS(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ACINACTM")) { field_val = ACINACTM(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_nts_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "TSBITS_TS")) { field_val = TSBITS_TS(); field_found=1; }
    if(!field_found && !strcmp(field_name, "TSSYNCS_TS")) { field_val = TSSYNCS_TS(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_nts_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "TSSYNCREADYS_TS")) { field_val = TSSYNCREADYS_TS(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_dstream_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGACK")) { field_val = DBGACK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "DBGRSTREQ")) { field_val = DBGRSTREQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_dstream_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "EDBGRQ")) { field_val = EDBGRQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_misc_dbg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGROMADDRV")) { field_val = DBGROMADDRV(); field_found=1; }
    if(!field_found && !strcmp(field_name, "DBGROMADDR")) { field_val = DBGROMADDR(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_misc_dbg_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "COMMRX")) { field_val = COMMRX(); field_found=1; }
    if(!field_found && !strcmp(field_name, "COMMTX")) { field_val = COMMTX(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCOMMIRQ")) { field_val = nCOMMIRQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_VSIG_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CLREXMONACK")) { field_val = CLREXMONACK(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_VSIG_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CLREXMONREQ")) { field_val = CLREXMONREQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_AUTH_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGEN")) { field_val = DBGEN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SPNIDEN")) { field_val = SPNIDEN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SPIDEN")) { field_val = SPIDEN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "NIDEN")) { field_val = NIDEN(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_CTI_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CTICHOUT")) { field_val = CTICHOUT(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTICHINACK")) { field_val = CTICHINACK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTIIRQ")) { field_val = CTIIRQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_CTI_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CTICHIN")) { field_val = CTICHIN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTICHOUTACK")) { field_val = CTICHOUTACK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTIIRQACK")) { field_val = CTIIRQACK(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_misc_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "WARMRSTREQ")) { field_val = WARMRSTREQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "WRMEMATTRM")) { field_val = WRMEMATTRM(); field_found=1; }
    if(!field_found && !strcmp(field_name, "RDMEMATTRM")) { field_val = RDMEMATTRM(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SMPEN")) { field_val = SMPEN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "STANDBYWFE")) { field_val = STANDBYWFE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "STANDBYWFI")) { field_val = STANDBYWFI(); field_found=1; }
    if(!field_found && !strcmp(field_name, "STANDBYWFIL2")) { field_val = STANDBYWFIL2(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_IRQ_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nEXTERRIRQ")) { field_val = nEXTERRIRQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nINTERRIRQ")) { field_val = nINTERRIRQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nVCPUMNTIRQ")) { field_val = nVCPUMNTIRQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_L2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "L2FLUSHDONE")) { field_val = L2FLUSHDONE(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_L2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "L2FLUSHREQ")) { field_val = L2FLUSHREQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_EV_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "EVENTO")) { field_val = EVENTO(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_EV_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "EVENTI")) { field_val = EVENTI(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_pmu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PMUSNAPSHOTREQ")) { field_val = PMUSNAPSHOTREQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_pmu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PMUEVENT0")) { field_val = PMUEVENT0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUEVENT1")) { field_val = PMUEVENT1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUEVENT2")) { field_val = PMUEVENT2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUEVENT3")) { field_val = PMUEVENT3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nPMUIRQ")) { field_val = nPMUIRQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUSNAPSHOTACK")) { field_val = PMUSNAPSHOTACK(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_timer_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nCNTHPIRQ")) { field_val = nCNTHPIRQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCNTPNSIRQ")) { field_val = nCNTPNSIRQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCNTPSIRQ")) { field_val = nCNTPSIRQ(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCNTVIRQ")) { field_val = nCNTVIRQ(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_AT_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "AFVALIDM0")) { field_val = AFVALIDM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFVALIDM1")) { field_val = AFVALIDM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFVALIDM2")) { field_val = AFVALIDM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFVALIDM3")) { field_val = AFVALIDM3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM0")) { field_val = ATREADYM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM1")) { field_val = ATREADYM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM2")) { field_val = ATREADYM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM3")) { field_val = ATREADYM3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM0")) { field_val = SYNCREQM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM1")) { field_val = SYNCREQM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM2")) { field_val = SYNCREQM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM3")) { field_val = SYNCREQM3(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_AT_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "AFREADYM0")) { field_val = AFREADYM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFREADYM1")) { field_val = AFREADYM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFREADYM2")) { field_val = AFREADYM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFREADYM3")) { field_val = AFREADYM3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM0")) { field_val = ATBYTESM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM1")) { field_val = ATBYTESM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM2")) { field_val = ATBYTESM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM3")) { field_val = ATBYTESM3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM0")) { field_val = ATDATAM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM1")) { field_val = ATDATAM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM2")) { field_val = ATDATAM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM3")) { field_val = ATDATAM3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM0")) { field_val = ATIDM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM1")) { field_val = ATIDM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM2")) { field_val = ATIDM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM3")) { field_val = ATIDM3(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM0")) { field_val = ATVALIDM0(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM1")) { field_val = ATVALIDM1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM2")) { field_val = ATVALIDM2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM3")) { field_val = ATVALIDM3(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_CORE_BIST_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DONE_FAIL_OUT")) { field_val = DONE_FAIL_OUT(); field_found=1; }
    if(!field_found && !strcmp(field_name, "DONE_PASS_OUT")) { field_val = DONE_PASS_OUT(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_CORE_BIST_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PTRN_FILL")) { field_val = PTRN_FILL(); field_found=1; }
    if(!field_found && !strcmp(field_name, "RPR_MODE")) { field_val = RPR_MODE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "RUN")) { field_val = RUN(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SEL_CLK")) { field_val = SEL_CLK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SEL_L2DCLK")) { field_val = SEL_L2DCLK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SEL_L2TCLK")) { field_val = SEL_L2TCLK(); field_found=1; }
    if(!field_found && !strcmp(field_name, "MEM_RST")) { field_val = MEM_RST(); field_found=1; }
    if(!field_found && !strcmp(field_name, "RESET_OVERRIDE")) { field_val = RESET_OVERRIDE(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_apms_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fatal")) { field_val = fatal(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_gic_mbist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mbistack")) { field_val = mbistack(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistoutdata")) { field_val = mbistoutdata(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_gic_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "base_awuser")) { field_val = base_awuser(); field_found=1; }
    if(!field_found && !strcmp(field_name, "base_aruser")) { field_val = base_aruser(); field_found=1; }
    if(!field_found && !strcmp(field_name, "awuser")) { field_val = awuser(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_gic_mbist_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mbistaddr")) { field_val = mbistaddr(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistarray")) { field_val = mbistarray(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistcfg")) { field_val = mbistcfg(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistreaden")) { field_val = mbistreaden(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistreq")) { field_val = mbistreq(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistresetn")) { field_val = mbistresetn(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistwriteen")) { field_val = mbistwriteen(); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistindata")) { field_val = mbistindata(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_irq_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nFIQ_mask")) { field_val = nFIQ_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nIRQ_mask")) { field_val = nIRQ_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nREI_mask")) { field_val = nREI_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nSEI_mask")) { field_val = nSEI_mask(); field_found=1; }
    if(!field_found && !strcmp(field_name, "nVSEI_mask")) { field_val = nVSEI_mask(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_apar_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr_top24")) { field_val = addr_top24(); field_found=1; }
    if(!field_found && !strcmp(field_name, "awusers")) { field_val = awusers(); field_found=1; }
    if(!field_found && !strcmp(field_name, "arusers")) { field_val = arusers(); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_sel")) { field_val = addr_sel(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "active")) { field_val = active(); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_wake_request")) { field_val = sta_wake_request(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu3_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { field_val = RVBARADDR(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu2_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { field_val = RVBARADDR(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu1_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { field_val = RVBARADDR(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu0_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { field_val = RVBARADDR(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu0_flash_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { field_val = RVBARADDR(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_static_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGL1RSTDISABLE")) { field_val = DBGL1RSTDISABLE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "L2RSTDISABLE")) { field_val = L2RSTDISABLE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CP15SDISABLE")) { field_val = CP15SDISABLE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CLUSTERIDAFF1")) { field_val = CLUSTERIDAFF1(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CLUSTERIDAFF2")) { field_val = CLUSTERIDAFF2(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CRYPTODISABLE")) { field_val = CRYPTODISABLE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "AA64nAA32")) { field_val = AA64nAA32(); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYSBARDISABLE")) { field_val = SYSBARDISABLE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "BROADCASTCACHEMAINT")) { field_val = BROADCASTCACHEMAINT(); field_found=1; }
    if(!field_found && !strcmp(field_name, "BROADCASTINNER")) { field_val = BROADCASTINNER(); field_found=1; }
    if(!field_found && !strcmp(field_name, "BROADCASTOUTER")) { field_val = BROADCASTOUTER(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CFGEND")) { field_val = CFGEND(); field_found=1; }
    if(!field_found && !strcmp(field_name, "CFGTE")) { field_val = CFGTE(); field_found=1; }
    if(!field_found && !strcmp(field_name, "PERIPHBASE")) { field_val = PERIPHBASE(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_dummy_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cycle_counter")) { field_val = cycle_counter(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_base_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { field_val = scratch_reg(); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_t::get_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { field_val = all(); field_found = 1; }

    if(!field_found) { field_found = base.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dummy.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_static.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu0_flash.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu0.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu1.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu3.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_apar.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_irq.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_gic_mbist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_gic.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_gic_mbist.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_apms.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_CORE_BIST.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_CORE_BIST.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_AT.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_AT.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_pmu.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_pmu.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_EV.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_EV.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_L2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_L2.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_IRQ.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_misc.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_CTI.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_CTI.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_AUTH.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_VSIG.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_VSIG.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_misc_dbg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_misc_dbg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dstream.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_dstream.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_nts.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_nts.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ACP.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = ap_cfg.get_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = apb.get_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_ap_cfg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "debug_port_enable")) { debug_port_enable(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "debug_port_select")) { debug_port_select(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "cfg_stall_on_same_id")) { cfg_stall_on_same_id(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_ACP_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "AINACTS")) { AINACTS(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ACINACTM")) { ACINACTM(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_nts_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "TSBITS_TS")) { TSBITS_TS(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "TSSYNCS_TS")) { TSSYNCS_TS(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_nts_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "TSSYNCREADYS_TS")) { TSSYNCREADYS_TS(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_dstream_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGACK")) { DBGACK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "DBGRSTREQ")) { DBGRSTREQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_dstream_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "EDBGRQ")) { EDBGRQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_misc_dbg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGROMADDRV")) { DBGROMADDRV(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "DBGROMADDR")) { DBGROMADDR(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_misc_dbg_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "COMMRX")) { COMMRX(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "COMMTX")) { COMMTX(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCOMMIRQ")) { nCOMMIRQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_VSIG_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CLREXMONACK")) { CLREXMONACK(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_VSIG_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CLREXMONREQ")) { CLREXMONREQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_AUTH_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGEN")) { DBGEN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SPNIDEN")) { SPNIDEN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SPIDEN")) { SPIDEN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "NIDEN")) { NIDEN(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_CTI_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CTICHOUT")) { CTICHOUT(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTICHINACK")) { CTICHINACK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTIIRQ")) { CTIIRQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_CTI_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "CTICHIN")) { CTICHIN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTICHOUTACK")) { CTICHOUTACK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CTIIRQACK")) { CTIIRQACK(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_misc_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "WARMRSTREQ")) { WARMRSTREQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "WRMEMATTRM")) { WRMEMATTRM(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "RDMEMATTRM")) { RDMEMATTRM(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SMPEN")) { SMPEN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "STANDBYWFE")) { STANDBYWFE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "STANDBYWFI")) { STANDBYWFI(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "STANDBYWFIL2")) { STANDBYWFIL2(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_IRQ_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nEXTERRIRQ")) { nEXTERRIRQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nINTERRIRQ")) { nINTERRIRQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nVCPUMNTIRQ")) { nVCPUMNTIRQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_L2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "L2FLUSHDONE")) { L2FLUSHDONE(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_L2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "L2FLUSHREQ")) { L2FLUSHREQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_EV_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "EVENTO")) { EVENTO(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_EV_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "EVENTI")) { EVENTI(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_pmu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PMUSNAPSHOTREQ")) { PMUSNAPSHOTREQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_pmu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PMUEVENT0")) { PMUEVENT0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUEVENT1")) { PMUEVENT1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUEVENT2")) { PMUEVENT2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUEVENT3")) { PMUEVENT3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nPMUIRQ")) { nPMUIRQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PMUSNAPSHOTACK")) { PMUSNAPSHOTACK(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_timer_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nCNTHPIRQ")) { nCNTHPIRQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCNTPNSIRQ")) { nCNTPNSIRQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCNTPSIRQ")) { nCNTPSIRQ(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nCNTVIRQ")) { nCNTVIRQ(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_AT_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "AFVALIDM0")) { AFVALIDM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFVALIDM1")) { AFVALIDM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFVALIDM2")) { AFVALIDM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFVALIDM3")) { AFVALIDM3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM0")) { ATREADYM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM1")) { ATREADYM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM2")) { ATREADYM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATREADYM3")) { ATREADYM3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM0")) { SYNCREQM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM1")) { SYNCREQM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM2")) { SYNCREQM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYNCREQM3")) { SYNCREQM3(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_AT_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "AFREADYM0")) { AFREADYM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFREADYM1")) { AFREADYM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFREADYM2")) { AFREADYM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AFREADYM3")) { AFREADYM3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM0")) { ATBYTESM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM1")) { ATBYTESM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM2")) { ATBYTESM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATBYTESM3")) { ATBYTESM3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM0")) { ATDATAM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM1")) { ATDATAM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM2")) { ATDATAM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATDATAM3")) { ATDATAM3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM0")) { ATIDM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM1")) { ATIDM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM2")) { ATIDM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATIDM3")) { ATIDM3(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM0")) { ATVALIDM0(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM1")) { ATVALIDM1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM2")) { ATVALIDM2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "ATVALIDM3")) { ATVALIDM3(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_CORE_BIST_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DONE_FAIL_OUT")) { DONE_FAIL_OUT(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "DONE_PASS_OUT")) { DONE_PASS_OUT(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_CORE_BIST_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "PTRN_FILL")) { PTRN_FILL(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "RPR_MODE")) { RPR_MODE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "RUN")) { RUN(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SEL_CLK")) { SEL_CLK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SEL_L2DCLK")) { SEL_L2DCLK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SEL_L2TCLK")) { SEL_L2TCLK(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "MEM_RST")) { MEM_RST(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "RESET_OVERRIDE")) { RESET_OVERRIDE(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_apms_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "fatal")) { fatal(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_sta_gic_mbist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mbistack")) { mbistack(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistoutdata")) { mbistoutdata(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_gic_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "base_awuser")) { base_awuser(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "base_aruser")) { base_aruser(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "awuser")) { awuser(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_gic_mbist_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "mbistaddr")) { mbistaddr(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistarray")) { mbistarray(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistcfg")) { mbistcfg(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistreaden")) { mbistreaden(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistreq")) { mbistreq(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistresetn")) { mbistresetn(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistwriteen")) { mbistwriteen(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "mbistindata")) { mbistindata(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_irq_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "nFIQ_mask")) { nFIQ_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nIRQ_mask")) { nIRQ_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nREI_mask")) { nREI_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nSEI_mask")) { nSEI_mask(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "nVSEI_mask")) { nVSEI_mask(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_apar_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "addr_top24")) { addr_top24(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "awusers")) { awusers(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "arusers")) { arusers(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "addr_sel")) { addr_sel(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "active")) { active(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "sta_wake_request")) { sta_wake_request(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu3_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { RVBARADDR(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu2_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { RVBARADDR(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu1_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { RVBARADDR(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu0_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { RVBARADDR(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_cpu0_flash_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "RVBARADDR")) { RVBARADDR(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_cfg_static_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "DBGL1RSTDISABLE")) { DBGL1RSTDISABLE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "L2RSTDISABLE")) { L2RSTDISABLE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CP15SDISABLE")) { CP15SDISABLE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CLUSTERIDAFF1")) { CLUSTERIDAFF1(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CLUSTERIDAFF2")) { CLUSTERIDAFF2(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CRYPTODISABLE")) { CRYPTODISABLE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "AA64nAA32")) { AA64nAA32(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "SYSBARDISABLE")) { SYSBARDISABLE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "BROADCASTCACHEMAINT")) { BROADCASTCACHEMAINT(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "BROADCASTINNER")) { BROADCASTINNER(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "BROADCASTOUTER")) { BROADCASTOUTER(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CFGEND")) { CFGEND(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "CFGTE")) { CFGTE(field_val); field_found=1; }
    if(!field_found && !strcmp(field_name, "PERIPHBASE")) { PERIPHBASE(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_dummy_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "cycle_counter")) { cycle_counter(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_base_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found && !strcmp(field_name, "scratch_reg")) { scratch_reg(field_val); field_found=1; }
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
bool cap_ap_csr_t::set_field_val(const char * field_name, cpp_int & field_val, int level) { 
    bool field_found = false;
    if(level==0) return field_found;
    if(!strcmp(field_name, "all")) { all(field_val); field_found = 1; }

    if(!field_found) { field_found = base.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = dummy.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_static.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu0_flash.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu0.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu1.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu3.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_cpu.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_apar.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_irq.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_gic_mbist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_gic.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_gic_mbist.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_apms.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_CORE_BIST.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_CORE_BIST.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_AT.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_AT.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_timer.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_pmu.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_pmu.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_EV.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_EV.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_L2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_L2.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_IRQ.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_misc.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_CTI.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_CTI.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_AUTH.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_VSIG.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_VSIG.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_misc_dbg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_misc_dbg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_dstream.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_dstream.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = sta_nts.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_nts.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = cfg_ACP.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = ap_cfg.set_field_val(field_name, field_val, level-1); }
    
    if(!field_found) { field_found = apb.set_field_val(field_name, field_val, level-1); }
    
    return field_found;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_ap_cfg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("debug_port_enable");
    ret_vec.push_back("debug_port_select");
    ret_vec.push_back("cfg_stall_on_same_id");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_ACP_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("AINACTS");
    ret_vec.push_back("ACINACTM");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_nts_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("TSBITS_TS");
    ret_vec.push_back("TSSYNCS_TS");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_nts_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("TSSYNCREADYS_TS");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_dstream_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("DBGACK");
    ret_vec.push_back("DBGRSTREQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_dstream_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("EDBGRQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_misc_dbg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("DBGROMADDRV");
    ret_vec.push_back("DBGROMADDR");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_misc_dbg_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("COMMRX");
    ret_vec.push_back("COMMTX");
    ret_vec.push_back("nCOMMIRQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_VSIG_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("CLREXMONACK");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_VSIG_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("CLREXMONREQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_AUTH_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("DBGEN");
    ret_vec.push_back("SPNIDEN");
    ret_vec.push_back("SPIDEN");
    ret_vec.push_back("NIDEN");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_CTI_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("CTICHOUT");
    ret_vec.push_back("CTICHINACK");
    ret_vec.push_back("CTIIRQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_CTI_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("CTICHIN");
    ret_vec.push_back("CTICHOUTACK");
    ret_vec.push_back("CTIIRQACK");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_misc_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("WARMRSTREQ");
    ret_vec.push_back("WRMEMATTRM");
    ret_vec.push_back("RDMEMATTRM");
    ret_vec.push_back("SMPEN");
    ret_vec.push_back("STANDBYWFE");
    ret_vec.push_back("STANDBYWFI");
    ret_vec.push_back("STANDBYWFIL2");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_IRQ_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("nEXTERRIRQ");
    ret_vec.push_back("nINTERRIRQ");
    ret_vec.push_back("nVCPUMNTIRQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_L2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("L2FLUSHDONE");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_L2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("L2FLUSHREQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_EV_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("EVENTO");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_EV_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("EVENTI");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_pmu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("PMUSNAPSHOTREQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_pmu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("PMUEVENT0");
    ret_vec.push_back("PMUEVENT1");
    ret_vec.push_back("PMUEVENT2");
    ret_vec.push_back("PMUEVENT3");
    ret_vec.push_back("nPMUIRQ");
    ret_vec.push_back("PMUSNAPSHOTACK");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_timer_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("nCNTHPIRQ");
    ret_vec.push_back("nCNTPNSIRQ");
    ret_vec.push_back("nCNTPSIRQ");
    ret_vec.push_back("nCNTVIRQ");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_AT_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("AFVALIDM0");
    ret_vec.push_back("AFVALIDM1");
    ret_vec.push_back("AFVALIDM2");
    ret_vec.push_back("AFVALIDM3");
    ret_vec.push_back("ATREADYM0");
    ret_vec.push_back("ATREADYM1");
    ret_vec.push_back("ATREADYM2");
    ret_vec.push_back("ATREADYM3");
    ret_vec.push_back("SYNCREQM0");
    ret_vec.push_back("SYNCREQM1");
    ret_vec.push_back("SYNCREQM2");
    ret_vec.push_back("SYNCREQM3");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_AT_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("AFREADYM0");
    ret_vec.push_back("AFREADYM1");
    ret_vec.push_back("AFREADYM2");
    ret_vec.push_back("AFREADYM3");
    ret_vec.push_back("ATBYTESM0");
    ret_vec.push_back("ATBYTESM1");
    ret_vec.push_back("ATBYTESM2");
    ret_vec.push_back("ATBYTESM3");
    ret_vec.push_back("ATDATAM0");
    ret_vec.push_back("ATDATAM1");
    ret_vec.push_back("ATDATAM2");
    ret_vec.push_back("ATDATAM3");
    ret_vec.push_back("ATIDM0");
    ret_vec.push_back("ATIDM1");
    ret_vec.push_back("ATIDM2");
    ret_vec.push_back("ATIDM3");
    ret_vec.push_back("ATVALIDM0");
    ret_vec.push_back("ATVALIDM1");
    ret_vec.push_back("ATVALIDM2");
    ret_vec.push_back("ATVALIDM3");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_CORE_BIST_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("DONE_FAIL_OUT");
    ret_vec.push_back("DONE_PASS_OUT");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_CORE_BIST_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("PTRN_FILL");
    ret_vec.push_back("RPR_MODE");
    ret_vec.push_back("RUN");
    ret_vec.push_back("SEL_CLK");
    ret_vec.push_back("SEL_L2DCLK");
    ret_vec.push_back("SEL_L2TCLK");
    ret_vec.push_back("MEM_RST");
    ret_vec.push_back("RESET_OVERRIDE");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_apms_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("fatal");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_sta_gic_mbist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mbistack");
    ret_vec.push_back("mbistoutdata");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_gic_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("base_awuser");
    ret_vec.push_back("base_aruser");
    ret_vec.push_back("awuser");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_gic_mbist_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("mbistaddr");
    ret_vec.push_back("mbistarray");
    ret_vec.push_back("mbistcfg");
    ret_vec.push_back("mbistreaden");
    ret_vec.push_back("mbistreq");
    ret_vec.push_back("mbistresetn");
    ret_vec.push_back("mbistwriteen");
    ret_vec.push_back("mbistindata");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_irq_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("nFIQ_mask");
    ret_vec.push_back("nIRQ_mask");
    ret_vec.push_back("nREI_mask");
    ret_vec.push_back("nSEI_mask");
    ret_vec.push_back("nVSEI_mask");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_apar_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("addr_top24");
    ret_vec.push_back("awusers");
    ret_vec.push_back("arusers");
    ret_vec.push_back("addr_sel");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_cpu_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("active");
    ret_vec.push_back("sta_wake_request");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_cpu3_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("RVBARADDR");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_cpu2_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("RVBARADDR");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_cpu1_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("RVBARADDR");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_cpu0_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("RVBARADDR");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_cpu0_flash_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("RVBARADDR");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_cfg_static_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("DBGL1RSTDISABLE");
    ret_vec.push_back("L2RSTDISABLE");
    ret_vec.push_back("CP15SDISABLE");
    ret_vec.push_back("CLUSTERIDAFF1");
    ret_vec.push_back("CLUSTERIDAFF2");
    ret_vec.push_back("CRYPTODISABLE");
    ret_vec.push_back("AA64nAA32");
    ret_vec.push_back("SYSBARDISABLE");
    ret_vec.push_back("BROADCASTCACHEMAINT");
    ret_vec.push_back("BROADCASTINNER");
    ret_vec.push_back("BROADCASTOUTER");
    ret_vec.push_back("CFGEND");
    ret_vec.push_back("CFGTE");
    ret_vec.push_back("PERIPHBASE");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_dummy_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("cycle_counter");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_base_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    ret_vec.push_back("scratch_reg");
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL

#ifndef EXCLUDE_PER_FIELD_CNTRL
std::vector<string> cap_ap_csr_t::get_fields(int level) const { 
    std::vector<string> ret_vec;
    if(level == 0) return ret_vec;
    ret_vec.push_back("all");

    {
        for(auto tmp_vec : base.get_fields(level-1)) {
            ret_vec.push_back("base." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : dummy.get_fields(level-1)) {
            ret_vec.push_back("dummy." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_static.get_fields(level-1)) {
            ret_vec.push_back("cfg_static." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_cpu0_flash.get_fields(level-1)) {
            ret_vec.push_back("cfg_cpu0_flash." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_cpu0.get_fields(level-1)) {
            ret_vec.push_back("cfg_cpu0." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_cpu1.get_fields(level-1)) {
            ret_vec.push_back("cfg_cpu1." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_cpu2.get_fields(level-1)) {
            ret_vec.push_back("cfg_cpu2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_cpu3.get_fields(level-1)) {
            ret_vec.push_back("cfg_cpu3." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_cpu.get_fields(level-1)) {
            ret_vec.push_back("cfg_cpu." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_apar.get_fields(level-1)) {
            ret_vec.push_back("cfg_apar." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_irq.get_fields(level-1)) {
            ret_vec.push_back("cfg_irq." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_gic_mbist.get_fields(level-1)) {
            ret_vec.push_back("cfg_gic_mbist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_gic.get_fields(level-1)) {
            ret_vec.push_back("cfg_gic." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_gic_mbist.get_fields(level-1)) {
            ret_vec.push_back("sta_gic_mbist." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_apms.get_fields(level-1)) {
            ret_vec.push_back("sta_apms." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_CORE_BIST.get_fields(level-1)) {
            ret_vec.push_back("cfg_CORE_BIST." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_CORE_BIST.get_fields(level-1)) {
            ret_vec.push_back("sta_CORE_BIST." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_AT.get_fields(level-1)) {
            ret_vec.push_back("sta_AT." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_AT.get_fields(level-1)) {
            ret_vec.push_back("cfg_AT." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_timer.get_fields(level-1)) {
            ret_vec.push_back("sta_timer." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_pmu.get_fields(level-1)) {
            ret_vec.push_back("sta_pmu." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_pmu.get_fields(level-1)) {
            ret_vec.push_back("cfg_pmu." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_EV.get_fields(level-1)) {
            ret_vec.push_back("cfg_EV." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_EV.get_fields(level-1)) {
            ret_vec.push_back("sta_EV." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_L2.get_fields(level-1)) {
            ret_vec.push_back("cfg_L2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_L2.get_fields(level-1)) {
            ret_vec.push_back("sta_L2." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_IRQ.get_fields(level-1)) {
            ret_vec.push_back("sta_IRQ." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_misc.get_fields(level-1)) {
            ret_vec.push_back("sta_misc." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_CTI.get_fields(level-1)) {
            ret_vec.push_back("cfg_CTI." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_CTI.get_fields(level-1)) {
            ret_vec.push_back("sta_CTI." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_AUTH.get_fields(level-1)) {
            ret_vec.push_back("cfg_AUTH." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_VSIG.get_fields(level-1)) {
            ret_vec.push_back("cfg_VSIG." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_VSIG.get_fields(level-1)) {
            ret_vec.push_back("sta_VSIG." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_misc_dbg.get_fields(level-1)) {
            ret_vec.push_back("sta_misc_dbg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_misc_dbg.get_fields(level-1)) {
            ret_vec.push_back("cfg_misc_dbg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_dstream.get_fields(level-1)) {
            ret_vec.push_back("cfg_dstream." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_dstream.get_fields(level-1)) {
            ret_vec.push_back("sta_dstream." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : sta_nts.get_fields(level-1)) {
            ret_vec.push_back("sta_nts." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_nts.get_fields(level-1)) {
            ret_vec.push_back("cfg_nts." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : cfg_ACP.get_fields(level-1)) {
            ret_vec.push_back("cfg_ACP." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : ap_cfg.get_fields(level-1)) {
            ret_vec.push_back("ap_cfg." + tmp_vec);
        }
    }
    {
        for(auto tmp_vec : apb.get_fields(level-1)) {
            ret_vec.push_back("apb." + tmp_vec);
        }
    }
    return ret_vec;
}
#endif //EXCLUDE_PER_FIELD_CNTRL
