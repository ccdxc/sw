
#ifndef CAP_EM_CSR_H
#define CAP_EM_CSR_H

#include "cap_csr_base.h" 
#include "cap_emm_csr.h" 

using namespace std;
class cap_em_csr_axi_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_axi_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_axi_int_enable_clear_t(string name = "cap_em_csr_axi_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_axi_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > axi_intr_enable_cpp_int_t;
        cpp_int int_var__axi_intr_enable;
        void axi_intr_enable (const cpp_int  & l__val);
        cpp_int axi_intr_enable() const;
    
        typedef pu_cpp_int< 1 > axi_wkup_enable_cpp_int_t;
        cpp_int int_var__axi_wkup_enable;
        void axi_wkup_enable (const cpp_int  & l__val);
        cpp_int axi_wkup_enable() const;
    
        typedef pu_cpp_int< 1 > sram1_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__sram1_ecc_correctable_enable;
        void sram1_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int sram1_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > sram1_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__sram1_ecc_uncorrectable_enable;
        void sram1_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int sram1_ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > sram2_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__sram2_ecc_correctable_enable;
        void sram2_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int sram2_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > sram2_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__sram2_ecc_uncorrectable_enable;
        void sram2_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int sram2_ecc_uncorrectable_enable() const;
    
}; // cap_em_csr_axi_int_enable_clear_t
    
class cap_em_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_intreg_t(string name = "cap_em_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > axi_intr_interrupt_cpp_int_t;
        cpp_int int_var__axi_intr_interrupt;
        void axi_intr_interrupt (const cpp_int  & l__val);
        cpp_int axi_intr_interrupt() const;
    
        typedef pu_cpp_int< 1 > axi_wkup_interrupt_cpp_int_t;
        cpp_int int_var__axi_wkup_interrupt;
        void axi_wkup_interrupt (const cpp_int  & l__val);
        cpp_int axi_wkup_interrupt() const;
    
        typedef pu_cpp_int< 1 > sram1_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__sram1_ecc_correctable_interrupt;
        void sram1_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int sram1_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sram1_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__sram1_ecc_uncorrectable_interrupt;
        void sram1_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int sram1_ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sram2_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__sram2_ecc_correctable_interrupt;
        void sram2_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int sram2_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sram2_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__sram2_ecc_uncorrectable_interrupt;
        void sram2_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int sram2_ecc_uncorrectable_interrupt() const;
    
}; // cap_em_csr_intreg_t
    
class cap_em_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_intgrp_t(string name = "cap_em_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_intgrp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_em_csr_intreg_t intreg;
    
        cap_em_csr_intreg_t int_test_set;
    
        cap_em_csr_axi_int_enable_clear_t int_enable_set;
    
        cap_em_csr_axi_int_enable_clear_t int_enable_clear;
    
}; // cap_em_csr_intgrp_t
    
class cap_em_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_intreg_status_t(string name = "cap_em_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > axi_interrupt_cpp_int_t;
        cpp_int int_var__axi_interrupt;
        void axi_interrupt (const cpp_int  & l__val);
        cpp_int axi_interrupt() const;
    
}; // cap_em_csr_intreg_status_t
    
class cap_em_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_int_groups_int_enable_rw_reg_t(string name = "cap_em_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > axi_enable_cpp_int_t;
        cpp_int int_var__axi_enable;
        void axi_enable (const cpp_int  & l__val);
        cpp_int axi_enable() const;
    
}; // cap_em_csr_int_groups_int_enable_rw_reg_t
    
class cap_em_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_intgrp_status_t(string name = "cap_em_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_em_csr_intreg_status_t intreg;
    
        cap_em_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_em_csr_intreg_status_t int_rw_reg;
    
}; // cap_em_csr_intgrp_status_t
    
class cap_em_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_csr_intr_t(string name = "cap_em_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dowstream_cpp_int_t;
        cpp_int int_var__dowstream;
        void dowstream (const cpp_int  & l__val);
        cpp_int dowstream() const;
    
        typedef pu_cpp_int< 1 > dowstream_enable_cpp_int_t;
        cpp_int int_var__dowstream_enable;
        void dowstream_enable (const cpp_int  & l__val);
        cpp_int dowstream_enable() const;
    
}; // cap_em_csr_csr_intr_t
    
class cap_em_csr_em_cfg_core_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_em_cfg_core_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_em_cfg_core_t(string name = "cap_em_csr_em_cfg_core_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_em_cfg_core_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > corecfg_baseclkfreq_cpp_int_t;
        cpp_int int_var__corecfg_baseclkfreq;
        void corecfg_baseclkfreq (const cpp_int  & l__val);
        cpp_int corecfg_baseclkfreq() const;
    
        typedef pu_cpp_int< 6 > corecfg_timeoutclkfreq_cpp_int_t;
        cpp_int int_var__corecfg_timeoutclkfreq;
        void corecfg_timeoutclkfreq (const cpp_int  & l__val);
        cpp_int corecfg_timeoutclkfreq() const;
    
        typedef pu_cpp_int< 21 > corecfg_maxblklength_cpp_int_t;
        cpp_int int_var__corecfg_maxblklength;
        void corecfg_maxblklength (const cpp_int  & l__val);
        cpp_int corecfg_maxblklength() const;
    
        typedef pu_cpp_int< 1 > corecfg_8bitsupport_cpp_int_t;
        cpp_int int_var__corecfg_8bitsupport;
        void corecfg_8bitsupport (const cpp_int  & l__val);
        cpp_int corecfg_8bitsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_adma2support_cpp_int_t;
        cpp_int int_var__corecfg_adma2support;
        void corecfg_adma2support (const cpp_int  & l__val);
        cpp_int corecfg_adma2support() const;
    
        typedef pu_cpp_int< 1 > corecfg_highspeedsupport_cpp_int_t;
        cpp_int int_var__corecfg_highspeedsupport;
        void corecfg_highspeedsupport (const cpp_int  & l__val);
        cpp_int corecfg_highspeedsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_sdmasupport_cpp_int_t;
        cpp_int int_var__corecfg_sdmasupport;
        void corecfg_sdmasupport (const cpp_int  & l__val);
        cpp_int corecfg_sdmasupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_suspressupport_cpp_int_t;
        cpp_int int_var__corecfg_suspressupport;
        void corecfg_suspressupport (const cpp_int  & l__val);
        cpp_int corecfg_suspressupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_3p3voltsupport_cpp_int_t;
        cpp_int int_var__corecfg_3p3voltsupport;
        void corecfg_3p3voltsupport (const cpp_int  & l__val);
        cpp_int corecfg_3p3voltsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_3p0voltsupport_cpp_int_t;
        cpp_int int_var__corecfg_3p0voltsupport;
        void corecfg_3p0voltsupport (const cpp_int  & l__val);
        cpp_int corecfg_3p0voltsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_1p8voltsupport_cpp_int_t;
        cpp_int int_var__corecfg_1p8voltsupport;
        void corecfg_1p8voltsupport (const cpp_int  & l__val);
        cpp_int corecfg_1p8voltsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_64bitsupport_cpp_int_t;
        cpp_int int_var__corecfg_64bitsupport;
        void corecfg_64bitsupport (const cpp_int  & l__val);
        cpp_int corecfg_64bitsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_asynchintrsupport_cpp_int_t;
        cpp_int int_var__corecfg_asynchintrsupport;
        void corecfg_asynchintrsupport (const cpp_int  & l__val);
        cpp_int corecfg_asynchintrsupport() const;
    
        typedef pu_cpp_int< 2 > corecfg_slottype_cpp_int_t;
        cpp_int int_var__corecfg_slottype;
        void corecfg_slottype (const cpp_int  & l__val);
        cpp_int corecfg_slottype() const;
    
        typedef pu_cpp_int< 1 > corecfg_sdr50support_cpp_int_t;
        cpp_int int_var__corecfg_sdr50support;
        void corecfg_sdr50support (const cpp_int  & l__val);
        cpp_int corecfg_sdr50support() const;
    
        typedef pu_cpp_int< 1 > corecfg_sdr104support_cpp_int_t;
        cpp_int int_var__corecfg_sdr104support;
        void corecfg_sdr104support (const cpp_int  & l__val);
        cpp_int corecfg_sdr104support() const;
    
        typedef pu_cpp_int< 1 > corecfg_ddr50support_cpp_int_t;
        cpp_int int_var__corecfg_ddr50support;
        void corecfg_ddr50support (const cpp_int  & l__val);
        cpp_int corecfg_ddr50support() const;
    
        typedef pu_cpp_int< 1 > corecfg_hs400support_cpp_int_t;
        cpp_int int_var__corecfg_hs400support;
        void corecfg_hs400support (const cpp_int  & l__val);
        cpp_int corecfg_hs400support() const;
    
        typedef pu_cpp_int< 1 > corecfg_adriversupport_cpp_int_t;
        cpp_int int_var__corecfg_adriversupport;
        void corecfg_adriversupport (const cpp_int  & l__val);
        cpp_int corecfg_adriversupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_cdriversupport_cpp_int_t;
        cpp_int int_var__corecfg_cdriversupport;
        void corecfg_cdriversupport (const cpp_int  & l__val);
        cpp_int corecfg_cdriversupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_ddriversupport_cpp_int_t;
        cpp_int int_var__corecfg_ddriversupport;
        void corecfg_ddriversupport (const cpp_int  & l__val);
        cpp_int corecfg_ddriversupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_type4support_cpp_int_t;
        cpp_int int_var__corecfg_type4support;
        void corecfg_type4support (const cpp_int  & l__val);
        cpp_int corecfg_type4support() const;
    
        typedef pu_cpp_int< 4 > corecfg_retuningtimercnt_cpp_int_t;
        cpp_int int_var__corecfg_retuningtimercnt;
        void corecfg_retuningtimercnt (const cpp_int  & l__val);
        cpp_int corecfg_retuningtimercnt() const;
    
        typedef pu_cpp_int< 1 > corecfg_tuningforsdr50_cpp_int_t;
        cpp_int int_var__corecfg_tuningforsdr50;
        void corecfg_tuningforsdr50 (const cpp_int  & l__val);
        cpp_int corecfg_tuningforsdr50() const;
    
        typedef pu_cpp_int< 2 > corecfg_retuningmodes_cpp_int_t;
        cpp_int int_var__corecfg_retuningmodes;
        void corecfg_retuningmodes (const cpp_int  & l__val);
        cpp_int corecfg_retuningmodes() const;
    
        typedef pu_cpp_int< 8 > corecfg_clockmultiplier_cpp_int_t;
        cpp_int int_var__corecfg_clockmultiplier;
        void corecfg_clockmultiplier (const cpp_int  & l__val);
        cpp_int corecfg_clockmultiplier() const;
    
        typedef pu_cpp_int< 1 > corecfg_spisupport_cpp_int_t;
        cpp_int int_var__corecfg_spisupport;
        void corecfg_spisupport (const cpp_int  & l__val);
        cpp_int corecfg_spisupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_spiblkmode_cpp_int_t;
        cpp_int int_var__corecfg_spiblkmode;
        void corecfg_spiblkmode (const cpp_int  & l__val);
        cpp_int corecfg_spiblkmode() const;
    
        typedef pu_cpp_int< 13 > corecfg_initpresetval_cpp_int_t;
        cpp_int int_var__corecfg_initpresetval;
        void corecfg_initpresetval (const cpp_int  & l__val);
        cpp_int corecfg_initpresetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_dspdpresetval_cpp_int_t;
        cpp_int int_var__corecfg_dspdpresetval;
        void corecfg_dspdpresetval (const cpp_int  & l__val);
        cpp_int corecfg_dspdpresetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_hspdpresetval_cpp_int_t;
        cpp_int int_var__corecfg_hspdpresetval;
        void corecfg_hspdpresetval (const cpp_int  & l__val);
        cpp_int corecfg_hspdpresetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_sdr12presetval_cpp_int_t;
        cpp_int int_var__corecfg_sdr12presetval;
        void corecfg_sdr12presetval (const cpp_int  & l__val);
        cpp_int corecfg_sdr12presetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_sdr25presetval_cpp_int_t;
        cpp_int int_var__corecfg_sdr25presetval;
        void corecfg_sdr25presetval (const cpp_int  & l__val);
        cpp_int corecfg_sdr25presetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_sdr50presetval_cpp_int_t;
        cpp_int int_var__corecfg_sdr50presetval;
        void corecfg_sdr50presetval (const cpp_int  & l__val);
        cpp_int corecfg_sdr50presetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_sdr104presetval_cpp_int_t;
        cpp_int int_var__corecfg_sdr104presetval;
        void corecfg_sdr104presetval (const cpp_int  & l__val);
        cpp_int corecfg_sdr104presetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_ddr50presetval_cpp_int_t;
        cpp_int int_var__corecfg_ddr50presetval;
        void corecfg_ddr50presetval (const cpp_int  & l__val);
        cpp_int corecfg_ddr50presetval() const;
    
        typedef pu_cpp_int< 13 > corecfg_hs400presetval_cpp_int_t;
        cpp_int int_var__corecfg_hs400presetval;
        void corecfg_hs400presetval (const cpp_int  & l__val);
        cpp_int corecfg_hs400presetval() const;
    
        typedef pu_cpp_int< 8 > corecfg_maxcurrent1p8v_cpp_int_t;
        cpp_int int_var__corecfg_maxcurrent1p8v;
        void corecfg_maxcurrent1p8v (const cpp_int  & l__val);
        cpp_int corecfg_maxcurrent1p8v() const;
    
        typedef pu_cpp_int< 8 > corecfg_maxcurrent3p0v_cpp_int_t;
        cpp_int int_var__corecfg_maxcurrent3p0v;
        void corecfg_maxcurrent3p0v (const cpp_int  & l__val);
        cpp_int corecfg_maxcurrent3p0v() const;
    
        typedef pu_cpp_int< 8 > corecfg_maxcurrent3p3v_cpp_int_t;
        cpp_int int_var__corecfg_maxcurrent3p3v;
        void corecfg_maxcurrent3p3v (const cpp_int  & l__val);
        cpp_int corecfg_maxcurrent3p3v() const;
    
        typedef pu_cpp_int< 10 > corecfg_cqfval_cpp_int_t;
        cpp_int int_var__corecfg_cqfval;
        void corecfg_cqfval (const cpp_int  & l__val);
        cpp_int corecfg_cqfval() const;
    
        typedef pu_cpp_int< 4 > corecfg_cqfmul_cpp_int_t;
        cpp_int int_var__corecfg_cqfmul;
        void corecfg_cqfmul (const cpp_int  & l__val);
        cpp_int corecfg_cqfmul() const;
    
        typedef pu_cpp_int< 1 > corecfg_timeoutclkunit_cpp_int_t;
        cpp_int int_var__corecfg_timeoutclkunit;
        void corecfg_timeoutclkunit (const cpp_int  & l__val);
        cpp_int corecfg_timeoutclkunit() const;
    
}; // cap_em_csr_em_cfg_core_t
    
class cap_em_csr_em_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_em_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_em_cfg_t(string name = "cap_em_csr_em_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_em_cfg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > debug_port_enable_cpp_int_t;
        cpp_int int_var__debug_port_enable;
        void debug_port_enable (const cpp_int  & l__val);
        cpp_int debug_port_enable() const;
    
        typedef pu_cpp_int< 1 > debug_port_select_cpp_int_t;
        cpp_int int_var__debug_port_select;
        void debug_port_select (const cpp_int  & l__val);
        cpp_int debug_port_select() const;
    
        typedef pu_cpp_int< 6 > mstid_cpp_int_t;
        cpp_int int_var__mstid;
        void mstid (const cpp_int  & l__val);
        cpp_int mstid() const;
    
}; // cap_em_csr_em_cfg_t
    
class cap_em_csr_cfg_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_cfg_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_cfg_ecc_t(string name = "cap_em_csr_cfg_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_cfg_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > disable_det_SRAM1_cpp_int_t;
        cpp_int int_var__disable_det_SRAM1;
        void disable_det_SRAM1 (const cpp_int  & l__val);
        cpp_int disable_det_SRAM1() const;
    
        typedef pu_cpp_int< 1 > disable_cor_SRAM2_cpp_int_t;
        cpp_int int_var__disable_cor_SRAM2;
        void disable_cor_SRAM2 (const cpp_int  & l__val);
        cpp_int disable_cor_SRAM2() const;
    
        typedef pu_cpp_int< 1 > disable_det_SRAM2_cpp_int_t;
        cpp_int int_var__disable_det_SRAM2;
        void disable_det_SRAM2 (const cpp_int  & l__val);
        cpp_int disable_det_SRAM2() const;
    
}; // cap_em_csr_cfg_ecc_t
    
class cap_em_csr_sta_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_sta_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_sta_ecc_t(string name = "cap_em_csr_sta_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_sta_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > correctable_SRAM1_cpp_int_t;
        cpp_int int_var__correctable_SRAM1;
        void correctable_SRAM1 (const cpp_int  & l__val);
        cpp_int correctable_SRAM1() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_SRAM1_cpp_int_t;
        cpp_int int_var__uncorrectable_SRAM1;
        void uncorrectable_SRAM1 (const cpp_int  & l__val);
        cpp_int uncorrectable_SRAM1() const;
    
        typedef pu_cpp_int< 1 > correctable_SRAM2_cpp_int_t;
        cpp_int int_var__correctable_SRAM2;
        void correctable_SRAM2 (const cpp_int  & l__val);
        cpp_int correctable_SRAM2() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_SRAM2_cpp_int_t;
        cpp_int int_var__uncorrectable_SRAM2;
        void uncorrectable_SRAM2 (const cpp_int  & l__val);
        cpp_int uncorrectable_SRAM2() const;
    
        typedef pu_cpp_int< 8 > addr_SRAM1_cpp_int_t;
        cpp_int int_var__addr_SRAM1;
        void addr_SRAM1 (const cpp_int  & l__val);
        cpp_int addr_SRAM1() const;
    
        typedef pu_cpp_int< 8 > addr_SRAM2_cpp_int_t;
        cpp_int int_var__addr_SRAM2;
        void addr_SRAM2 (const cpp_int  & l__val);
        cpp_int addr_SRAM2() const;
    
        typedef pu_cpp_int< 8 > syndrome_SRAM1_cpp_int_t;
        cpp_int int_var__syndrome_SRAM1;
        void syndrome_SRAM1 (const cpp_int  & l__val);
        cpp_int syndrome_SRAM1() const;
    
        typedef pu_cpp_int< 8 > syndrome_SRAM2_cpp_int_t;
        cpp_int int_var__syndrome_SRAM2;
        void syndrome_SRAM2 (const cpp_int  & l__val);
        cpp_int syndrome_SRAM2() const;
    
}; // cap_em_csr_sta_ecc_t
    
class cap_em_csr_sta_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_sta_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_sta_bist_t(string name = "cap_em_csr_sta_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_sta_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > done_fail_SRAM1_cpp_int_t;
        cpp_int int_var__done_fail_SRAM1;
        void done_fail_SRAM1 (const cpp_int  & l__val);
        cpp_int done_fail_SRAM1() const;
    
        typedef pu_cpp_int< 1 > done_fail_SRAM2_cpp_int_t;
        cpp_int int_var__done_fail_SRAM2;
        void done_fail_SRAM2 (const cpp_int  & l__val);
        cpp_int done_fail_SRAM2() const;
    
        typedef pu_cpp_int< 1 > done_pass_SRAM1_cpp_int_t;
        cpp_int int_var__done_pass_SRAM1;
        void done_pass_SRAM1 (const cpp_int  & l__val);
        cpp_int done_pass_SRAM1() const;
    
        typedef pu_cpp_int< 1 > done_pass_SRAM2_cpp_int_t;
        cpp_int int_var__done_pass_SRAM2;
        void done_pass_SRAM2 (const cpp_int  & l__val);
        cpp_int done_pass_SRAM2() const;
    
}; // cap_em_csr_sta_bist_t
    
class cap_em_csr_cfg_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_cfg_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_cfg_bist_t(string name = "cap_em_csr_cfg_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_cfg_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > run_SRAM1_cpp_int_t;
        cpp_int int_var__run_SRAM1;
        void run_SRAM1 (const cpp_int  & l__val);
        cpp_int run_SRAM1() const;
    
        typedef pu_cpp_int< 1 > run_SRAM2_cpp_int_t;
        cpp_int int_var__run_SRAM2;
        void run_SRAM2 (const cpp_int  & l__val);
        cpp_int run_SRAM2() const;
    
}; // cap_em_csr_cfg_bist_t
    
class cap_em_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_base_t(string name = "cap_em_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        cpp_int int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_em_csr_base_t
    
class cap_em_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_em_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_em_csr_t(string name = "cap_em_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_em_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_em_csr_base_t base;
    
        cap_em_csr_cfg_bist_t cfg_bist;
    
        cap_em_csr_sta_bist_t sta_bist;
    
        cap_em_csr_sta_ecc_t sta_ecc;
    
        cap_em_csr_cfg_ecc_t cfg_ecc;
    
        cap_em_csr_em_cfg_t em_cfg;
    
        cap_em_csr_em_cfg_core_t em_cfg_core;
    
        cap_em_csr_csr_intr_t csr_intr;
    
        cap_emm_csr_t emm;
    
        cap_em_csr_intgrp_status_t int_groups;
    
        cap_em_csr_intgrp_t axi;
    
}; // cap_em_csr_t
    
#endif // CAP_EM_CSR_H
        