
#ifndef CAP_PXC_CSR_H
#define CAP_PXC_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pxc_csr_int_c_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_c_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_c_ecc_int_enable_clear_t(string name = "cap_pxc_csr_int_c_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_c_ecc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > rxbuf_0_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_0_uncorrectable_enable;
        void rxbuf_0_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_0_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_1_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_1_uncorrectable_enable;
        void rxbuf_1_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_1_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_2_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_2_uncorrectable_enable;
        void rxbuf_2_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_2_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_3_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_3_uncorrectable_enable;
        void rxbuf_3_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_3_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_0_correctable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_0_correctable_enable;
        void rxbuf_0_correctable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_0_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_1_correctable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_1_correctable_enable;
        void rxbuf_1_correctable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_1_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_2_correctable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_2_correctable_enable;
        void rxbuf_2_correctable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_2_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxbuf_3_correctable_enable_cpp_int_t;
        cpp_int int_var__rxbuf_3_correctable_enable;
        void rxbuf_3_correctable_enable (const cpp_int  & l__val);
        cpp_int rxbuf_3_correctable_enable() const;
    
}; // cap_pxc_csr_int_c_ecc_int_enable_clear_t
    
class cap_pxc_csr_int_c_ecc_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_c_ecc_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_c_ecc_int_test_set_t(string name = "cap_pxc_csr_int_c_ecc_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_c_ecc_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > rxbuf_0_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_0_uncorrectable_interrupt;
        void rxbuf_0_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_0_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_1_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_1_uncorrectable_interrupt;
        void rxbuf_1_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_1_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_2_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_2_uncorrectable_interrupt;
        void rxbuf_2_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_2_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_3_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_3_uncorrectable_interrupt;
        void rxbuf_3_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_3_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_0_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_0_correctable_interrupt;
        void rxbuf_0_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_0_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_1_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_1_correctable_interrupt;
        void rxbuf_1_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_1_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_2_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_2_correctable_interrupt;
        void rxbuf_2_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_2_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxbuf_3_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxbuf_3_correctable_interrupt;
        void rxbuf_3_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxbuf_3_correctable_interrupt() const;
    
}; // cap_pxc_csr_int_c_ecc_int_test_set_t
    
class cap_pxc_csr_int_c_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_c_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_c_ecc_t(string name = "cap_pxc_csr_int_c_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_c_ecc_t();
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
    
        cap_pxc_csr_int_c_ecc_int_test_set_t intreg;
    
        cap_pxc_csr_int_c_ecc_int_test_set_t int_test_set;
    
        cap_pxc_csr_int_c_ecc_int_enable_clear_t int_enable_set;
    
        cap_pxc_csr_int_c_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_pxc_csr_int_c_ecc_t
    
class cap_pxc_csr_int_c_mac_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_c_mac_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_c_mac_int_enable_clear_t(string name = "cap_pxc_csr_int_c_mac_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_c_mac_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > link_dn2up_enable_cpp_int_t;
        cpp_int int_var__link_dn2up_enable;
        void link_dn2up_enable (const cpp_int  & l__val);
        cpp_int link_dn2up_enable() const;
    
        typedef pu_cpp_int< 1 > link_up2dn_enable_cpp_int_t;
        cpp_int int_var__link_up2dn_enable;
        void link_up2dn_enable (const cpp_int  & l__val);
        cpp_int link_up2dn_enable() const;
    
        typedef pu_cpp_int< 1 > sec_bus_rst_enable_cpp_int_t;
        cpp_int int_var__sec_bus_rst_enable;
        void sec_bus_rst_enable (const cpp_int  & l__val);
        cpp_int sec_bus_rst_enable() const;
    
        typedef pu_cpp_int< 1 > rst_up2dn_enable_cpp_int_t;
        cpp_int int_var__rst_up2dn_enable;
        void rst_up2dn_enable (const cpp_int  & l__val);
        cpp_int rst_up2dn_enable() const;
    
        typedef pu_cpp_int< 1 > rst_dn2up_enable_cpp_int_t;
        cpp_int int_var__rst_dn2up_enable;
        void rst_dn2up_enable (const cpp_int  & l__val);
        cpp_int rst_dn2up_enable() const;
    
        typedef pu_cpp_int< 1 > portgate_open2close_enable_cpp_int_t;
        cpp_int int_var__portgate_open2close_enable;
        void portgate_open2close_enable (const cpp_int  & l__val);
        cpp_int portgate_open2close_enable() const;
    
        typedef pu_cpp_int< 1 > ltssm_st_changed_enable_cpp_int_t;
        cpp_int int_var__ltssm_st_changed_enable;
        void ltssm_st_changed_enable (const cpp_int  & l__val);
        cpp_int ltssm_st_changed_enable() const;
    
        typedef pu_cpp_int< 1 > sec_busnum_changed_enable_cpp_int_t;
        cpp_int int_var__sec_busnum_changed_enable;
        void sec_busnum_changed_enable (const cpp_int  & l__val);
        cpp_int sec_busnum_changed_enable() const;
    
        typedef pu_cpp_int< 1 > rc_pme_enable_cpp_int_t;
        cpp_int int_var__rc_pme_enable;
        void rc_pme_enable (const cpp_int  & l__val);
        cpp_int rc_pme_enable() const;
    
        typedef pu_cpp_int< 1 > rc_aerr_enable_cpp_int_t;
        cpp_int int_var__rc_aerr_enable;
        void rc_aerr_enable (const cpp_int  & l__val);
        cpp_int rc_aerr_enable() const;
    
        typedef pu_cpp_int< 1 > rc_serr_enable_cpp_int_t;
        cpp_int int_var__rc_serr_enable;
        void rc_serr_enable (const cpp_int  & l__val);
        cpp_int rc_serr_enable() const;
    
        typedef pu_cpp_int< 1 > rc_hpe_enable_cpp_int_t;
        cpp_int int_var__rc_hpe_enable;
        void rc_hpe_enable (const cpp_int  & l__val);
        cpp_int rc_hpe_enable() const;
    
        typedef pu_cpp_int< 1 > rc_eq_req_enable_cpp_int_t;
        cpp_int int_var__rc_eq_req_enable;
        void rc_eq_req_enable (const cpp_int  & l__val);
        cpp_int rc_eq_req_enable() const;
    
        typedef pu_cpp_int< 1 > rc_dpc_enable_cpp_int_t;
        cpp_int int_var__rc_dpc_enable;
        void rc_dpc_enable (const cpp_int  & l__val);
        cpp_int rc_dpc_enable() const;
    
        typedef pu_cpp_int< 1 > pm_turnoff_enable_cpp_int_t;
        cpp_int int_var__pm_turnoff_enable;
        void pm_turnoff_enable (const cpp_int  & l__val);
        cpp_int pm_turnoff_enable() const;
    
        typedef pu_cpp_int< 1 > txbfr_overflow_enable_cpp_int_t;
        cpp_int int_var__txbfr_overflow_enable;
        void txbfr_overflow_enable (const cpp_int  & l__val);
        cpp_int txbfr_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > rxtlp_err_enable_cpp_int_t;
        cpp_int int_var__rxtlp_err_enable;
        void rxtlp_err_enable (const cpp_int  & l__val);
        cpp_int rxtlp_err_enable() const;
    
        typedef pu_cpp_int< 1 > tl_flr_req_enable_cpp_int_t;
        cpp_int int_var__tl_flr_req_enable;
        void tl_flr_req_enable (const cpp_int  & l__val);
        cpp_int tl_flr_req_enable() const;
    
        typedef pu_cpp_int< 1 > rc_legacy_intpin_changed_enable_cpp_int_t;
        cpp_int int_var__rc_legacy_intpin_changed_enable;
        void rc_legacy_intpin_changed_enable (const cpp_int  & l__val);
        cpp_int rc_legacy_intpin_changed_enable() const;
    
}; // cap_pxc_csr_int_c_mac_int_enable_clear_t
    
class cap_pxc_csr_int_c_mac_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_c_mac_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_c_mac_intreg_t(string name = "cap_pxc_csr_int_c_mac_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_c_mac_intreg_t();
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
    
        typedef pu_cpp_int< 1 > link_dn2up_interrupt_cpp_int_t;
        cpp_int int_var__link_dn2up_interrupt;
        void link_dn2up_interrupt (const cpp_int  & l__val);
        cpp_int link_dn2up_interrupt() const;
    
        typedef pu_cpp_int< 1 > link_up2dn_interrupt_cpp_int_t;
        cpp_int int_var__link_up2dn_interrupt;
        void link_up2dn_interrupt (const cpp_int  & l__val);
        cpp_int link_up2dn_interrupt() const;
    
        typedef pu_cpp_int< 1 > sec_bus_rst_interrupt_cpp_int_t;
        cpp_int int_var__sec_bus_rst_interrupt;
        void sec_bus_rst_interrupt (const cpp_int  & l__val);
        cpp_int sec_bus_rst_interrupt() const;
    
        typedef pu_cpp_int< 1 > rst_up2dn_interrupt_cpp_int_t;
        cpp_int int_var__rst_up2dn_interrupt;
        void rst_up2dn_interrupt (const cpp_int  & l__val);
        cpp_int rst_up2dn_interrupt() const;
    
        typedef pu_cpp_int< 1 > rst_dn2up_interrupt_cpp_int_t;
        cpp_int int_var__rst_dn2up_interrupt;
        void rst_dn2up_interrupt (const cpp_int  & l__val);
        cpp_int rst_dn2up_interrupt() const;
    
        typedef pu_cpp_int< 1 > portgate_open2close_interrupt_cpp_int_t;
        cpp_int int_var__portgate_open2close_interrupt;
        void portgate_open2close_interrupt (const cpp_int  & l__val);
        cpp_int portgate_open2close_interrupt() const;
    
        typedef pu_cpp_int< 1 > ltssm_st_changed_interrupt_cpp_int_t;
        cpp_int int_var__ltssm_st_changed_interrupt;
        void ltssm_st_changed_interrupt (const cpp_int  & l__val);
        cpp_int ltssm_st_changed_interrupt() const;
    
        typedef pu_cpp_int< 1 > sec_busnum_changed_interrupt_cpp_int_t;
        cpp_int int_var__sec_busnum_changed_interrupt;
        void sec_busnum_changed_interrupt (const cpp_int  & l__val);
        cpp_int sec_busnum_changed_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_pme_interrupt_cpp_int_t;
        cpp_int int_var__rc_pme_interrupt;
        void rc_pme_interrupt (const cpp_int  & l__val);
        cpp_int rc_pme_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_aerr_interrupt_cpp_int_t;
        cpp_int int_var__rc_aerr_interrupt;
        void rc_aerr_interrupt (const cpp_int  & l__val);
        cpp_int rc_aerr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_serr_interrupt_cpp_int_t;
        cpp_int int_var__rc_serr_interrupt;
        void rc_serr_interrupt (const cpp_int  & l__val);
        cpp_int rc_serr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_hpe_interrupt_cpp_int_t;
        cpp_int int_var__rc_hpe_interrupt;
        void rc_hpe_interrupt (const cpp_int  & l__val);
        cpp_int rc_hpe_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_eq_req_interrupt_cpp_int_t;
        cpp_int int_var__rc_eq_req_interrupt;
        void rc_eq_req_interrupt (const cpp_int  & l__val);
        cpp_int rc_eq_req_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_dpc_interrupt_cpp_int_t;
        cpp_int int_var__rc_dpc_interrupt;
        void rc_dpc_interrupt (const cpp_int  & l__val);
        cpp_int rc_dpc_interrupt() const;
    
        typedef pu_cpp_int< 1 > pm_turnoff_interrupt_cpp_int_t;
        cpp_int int_var__pm_turnoff_interrupt;
        void pm_turnoff_interrupt (const cpp_int  & l__val);
        cpp_int pm_turnoff_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbfr_overflow_interrupt_cpp_int_t;
        cpp_int int_var__txbfr_overflow_interrupt;
        void txbfr_overflow_interrupt (const cpp_int  & l__val);
        cpp_int txbfr_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxtlp_err_interrupt_cpp_int_t;
        cpp_int int_var__rxtlp_err_interrupt;
        void rxtlp_err_interrupt (const cpp_int  & l__val);
        cpp_int rxtlp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tl_flr_req_interrupt_cpp_int_t;
        cpp_int int_var__tl_flr_req_interrupt;
        void tl_flr_req_interrupt (const cpp_int  & l__val);
        cpp_int tl_flr_req_interrupt() const;
    
        typedef pu_cpp_int< 1 > rc_legacy_intpin_changed_interrupt_cpp_int_t;
        cpp_int int_var__rc_legacy_intpin_changed_interrupt;
        void rc_legacy_intpin_changed_interrupt (const cpp_int  & l__val);
        cpp_int rc_legacy_intpin_changed_interrupt() const;
    
}; // cap_pxc_csr_int_c_mac_intreg_t
    
class cap_pxc_csr_int_c_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_c_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_c_mac_t(string name = "cap_pxc_csr_int_c_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_c_mac_t();
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
    
        cap_pxc_csr_int_c_mac_intreg_t intreg;
    
        cap_pxc_csr_int_c_mac_intreg_t int_test_set;
    
        cap_pxc_csr_int_c_mac_int_enable_clear_t int_enable_set;
    
        cap_pxc_csr_int_c_mac_int_enable_clear_t int_enable_clear;
    
}; // cap_pxc_csr_int_c_mac_t
    
class cap_pxc_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_intreg_status_t(string name = "cap_pxc_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_c_mac_interrupt_cpp_int_t;
        cpp_int int_var__int_c_mac_interrupt;
        void int_c_mac_interrupt (const cpp_int  & l__val);
        cpp_int int_c_mac_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_c_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_c_ecc_interrupt;
        void int_c_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_c_ecc_interrupt() const;
    
}; // cap_pxc_csr_intreg_status_t
    
class cap_pxc_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pxc_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_c_mac_enable_cpp_int_t;
        cpp_int int_var__int_c_mac_enable;
        void int_c_mac_enable (const cpp_int  & l__val);
        cpp_int int_c_mac_enable() const;
    
        typedef pu_cpp_int< 1 > int_c_ecc_enable_cpp_int_t;
        cpp_int int_var__int_c_ecc_enable;
        void int_c_ecc_enable (const cpp_int  & l__val);
        cpp_int int_c_ecc_enable() const;
    
}; // cap_pxc_csr_int_groups_int_enable_rw_reg_t
    
class cap_pxc_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_intgrp_status_t(string name = "cap_pxc_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_intgrp_status_t();
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
    
        cap_pxc_csr_intreg_status_t intreg;
    
        cap_pxc_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pxc_csr_intreg_status_t int_rw_reg;
    
}; // cap_pxc_csr_intgrp_status_t
    
class cap_pxc_csr_dhs_c_mac_apb_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_dhs_c_mac_apb_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_dhs_c_mac_apb_entry_t(string name = "cap_pxc_csr_dhs_c_mac_apb_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_dhs_c_mac_apb_entry_t();
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
    
        typedef pu_cpp_int< 32 > cfg_data_cpp_int_t;
        cpp_int int_var__cfg_data;
        void cfg_data (const cpp_int  & l__val);
        cpp_int cfg_data() const;
    
}; // cap_pxc_csr_dhs_c_mac_apb_entry_t
    
class cap_pxc_csr_dhs_c_mac_apb_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_dhs_c_mac_apb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_dhs_c_mac_apb_t(string name = "cap_pxc_csr_dhs_c_mac_apb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_dhs_c_mac_apb_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxc_csr_dhs_c_mac_apb_entry_t, 1024> entry;
        #else 
        cap_pxc_csr_dhs_c_mac_apb_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxc_csr_dhs_c_mac_apb_t
    
class cap_pxc_csr_cfg_c_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_spare_t(string name = "cap_pxc_csr_cfg_c_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_spare_t();
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
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_spare_t
    
class cap_pxc_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_csr_intr_t(string name = "cap_pxc_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_csr_intr_t();
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
    
}; // cap_pxc_csr_csr_intr_t
    
class cap_pxc_csr_sta_c_port_phystatus_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_port_phystatus_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_port_phystatus_t(string name = "cap_pxc_csr_sta_c_port_phystatus_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_port_phystatus_t();
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
    
        typedef pu_cpp_int< 16 > per_lane_cpp_int_t;
        cpp_int int_var__per_lane;
        void per_lane (const cpp_int  & l__val);
        cpp_int per_lane() const;
    
}; // cap_pxc_csr_sta_c_port_phystatus_t
    
class cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t(string name = "cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_nullify_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rx_nullify_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rx_nullify_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_nullify_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_nullify_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_nullify_t
    
class cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t(string name = "cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t
    
class cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t(string name = "cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t();
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
    
        typedef pu_cpp_int< 16 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t
    
class cap_pxc_csr_sat_c_port_cnt_tx_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sat_c_port_cnt_tx_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sat_c_port_cnt_tx_drop_t(string name = "cap_pxc_csr_sat_c_port_cnt_tx_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sat_c_port_cnt_tx_drop_t();
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
    
        typedef pu_cpp_int< 8 > nullified_cpp_int_t;
        cpp_int int_var__nullified;
        void nullified (const cpp_int  & l__val);
        cpp_int nullified() const;
    
        typedef pu_cpp_int< 8 > portgate_cpp_int_t;
        cpp_int int_var__portgate;
        void portgate (const cpp_int  & l__val);
        cpp_int portgate() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_tx_drop_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_ecc_rxbuf_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_ecc_rxbuf_3_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_3_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_3_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_ecc_rxbuf_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_ecc_rxbuf_2_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_2_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_2_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_ecc_rxbuf_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_ecc_rxbuf_1_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_1_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_1_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_ecc_rxbuf_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_ecc_rxbuf_0_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_0_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_0_t
    
class cap_pxc_csr_sta_c_tx_fc_credits_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_tx_fc_credits_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_tx_fc_credits_t(string name = "cap_pxc_csr_sta_c_tx_fc_credits_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_tx_fc_credits_t();
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
    
        typedef pu_cpp_int< 32 > posted_cpp_int_t;
        cpp_int int_var__posted;
        void posted (const cpp_int  & l__val);
        cpp_int posted() const;
    
        typedef pu_cpp_int< 32 > non_posted_cpp_int_t;
        cpp_int int_var__non_posted;
        void non_posted (const cpp_int  & l__val);
        cpp_int non_posted() const;
    
}; // cap_pxc_csr_sta_c_tx_fc_credits_t
    
class cap_pxc_csr_cfg_c_mac_ssvid_cap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_ssvid_cap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_ssvid_cap_t(string name = "cap_pxc_csr_cfg_c_mac_ssvid_cap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_ssvid_cap_t();
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
    
        typedef pu_cpp_int< 16 > ssvid_cpp_int_t;
        cpp_int int_var__ssvid;
        void ssvid (const cpp_int  & l__val);
        cpp_int ssvid() const;
    
        typedef pu_cpp_int< 16 > ssid_cpp_int_t;
        cpp_int int_var__ssid;
        void ssid (const cpp_int  & l__val);
        cpp_int ssid() const;
    
}; // cap_pxc_csr_cfg_c_mac_ssvid_cap_t
    
class cap_pxc_csr_cfg_c_mac_test_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_test_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_test_in_t(string name = "cap_pxc_csr_cfg_c_mac_test_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_test_in_t();
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
    
        typedef pu_cpp_int< 32 > dw0_cpp_int_t;
        cpp_int int_var__dw0;
        void dw0 (const cpp_int  & l__val);
        cpp_int dw0() const;
    
}; // cap_pxc_csr_cfg_c_mac_test_in_t
    
class cap_pxc_csr_sta_c_brsw_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_brsw_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_brsw_t(string name = "cap_pxc_csr_sta_c_brsw_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_brsw_t();
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
    
        typedef pu_cpp_int< 8 > tl_brsw_out_cpp_int_t;
        cpp_int int_var__tl_brsw_out;
        void tl_brsw_out (const cpp_int  & l__val);
        cpp_int tl_brsw_out() const;
    
}; // cap_pxc_csr_sta_c_brsw_t
    
class cap_pxc_csr_cfg_c_brsw_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_brsw_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_brsw_t(string name = "cap_pxc_csr_cfg_c_brsw_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_brsw_t();
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
    
        typedef pu_cpp_int< 8 > tl_brsw_in_cpp_int_t;
        cpp_int int_var__tl_brsw_in;
        void tl_brsw_in (const cpp_int  & l__val);
        cpp_int tl_brsw_in() const;
    
}; // cap_pxc_csr_cfg_c_brsw_t
    
class cap_pxc_csr_cfg_c_ecc_disable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_ecc_disable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_ecc_disable_t(string name = "cap_pxc_csr_cfg_c_ecc_disable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_ecc_disable_t();
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
    
        typedef pu_cpp_int< 1 > rxbuf_cor_cpp_int_t;
        cpp_int int_var__rxbuf_cor;
        void rxbuf_cor (const cpp_int  & l__val);
        cpp_int rxbuf_cor() const;
    
        typedef pu_cpp_int< 1 > rxbuf_det_cpp_int_t;
        cpp_int int_var__rxbuf_det;
        void rxbuf_det (const cpp_int  & l__val);
        cpp_int rxbuf_det() const;
    
}; // cap_pxc_csr_cfg_c_ecc_disable_t
    
class cap_pxc_csr_cfg_c_tl_report_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_tl_report_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_tl_report_t(string name = "cap_pxc_csr_cfg_c_tl_report_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_tl_report_t();
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
    
        typedef pu_cpp_int< 1 > flr_ack_cpp_int_t;
        cpp_int int_var__flr_ack;
        void flr_ack (const cpp_int  & l__val);
        cpp_int flr_ack() const;
    
        typedef pu_cpp_int< 1 > cpl_timeout_cpp_int_t;
        cpp_int int_var__cpl_timeout;
        void cpl_timeout (const cpp_int  & l__val);
        cpp_int cpl_timeout() const;
    
        typedef pu_cpp_int< 1 > cpl_unexp_cpp_int_t;
        cpp_int int_var__cpl_unexp;
        void cpl_unexp (const cpp_int  & l__val);
        cpp_int cpl_unexp() const;
    
        typedef pu_cpp_int< 1 > cpl_malform_cpp_int_t;
        cpp_int int_var__cpl_malform;
        void cpl_malform (const cpp_int  & l__val);
        cpp_int cpl_malform() const;
    
        typedef pu_cpp_int< 1 > cpl_ca_cpp_int_t;
        cpp_int int_var__cpl_ca;
        void cpl_ca (const cpp_int  & l__val);
        cpp_int cpl_ca() const;
    
        typedef pu_cpp_int< 1 > cpl_ur_cpp_int_t;
        cpp_int int_var__cpl_ur;
        void cpl_ur (const cpp_int  & l__val);
        cpp_int cpl_ur() const;
    
        typedef pu_cpp_int< 1 > dropped_posted_cpp_int_t;
        cpp_int int_var__dropped_posted;
        void dropped_posted (const cpp_int  & l__val);
        cpp_int dropped_posted() const;
    
        typedef pu_cpp_int< 1 > atomic_egress_blocked_cpp_int_t;
        cpp_int int_var__atomic_egress_blocked;
        void atomic_egress_blocked (const cpp_int  & l__val);
        cpp_int atomic_egress_blocked() const;
    
        typedef pu_cpp_int< 1 > tlp_prefix_blocked_cpp_int_t;
        cpp_int int_var__tlp_prefix_blocked;
        void tlp_prefix_blocked (const cpp_int  & l__val);
        cpp_int tlp_prefix_blocked() const;
    
        typedef pu_cpp_int< 1 > unc_err_cpp_int_t;
        cpp_int int_var__unc_err;
        void unc_err (const cpp_int  & l__val);
        cpp_int unc_err() const;
    
        typedef pu_cpp_int< 1 > cor_err_cpp_int_t;
        cpp_int int_var__cor_err;
        void cor_err (const cpp_int  & l__val);
        cpp_int cor_err() const;
    
}; // cap_pxc_csr_cfg_c_tl_report_t
    
class cap_pxc_csr_cfg_c_autonomous_linkwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_autonomous_linkwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_autonomous_linkwidth_t(string name = "cap_pxc_csr_cfg_c_autonomous_linkwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_autonomous_linkwidth_t();
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
    
        typedef pu_cpp_int< 4 > pm_bwchange_cpp_int_t;
        cpp_int int_var__pm_bwchange;
        void pm_bwchange (const cpp_int  & l__val);
        cpp_int pm_bwchange() const;
    
}; // cap_pxc_csr_cfg_c_autonomous_linkwidth_t
    
class cap_pxc_csr_cfg_c_ltr_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_ltr_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_ltr_latency_t(string name = "cap_pxc_csr_cfg_c_ltr_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_ltr_latency_t();
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
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_ltr_latency_t
    
class cap_pxc_csr_cfg_c_portgate_close_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_portgate_close_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_portgate_close_t(string name = "cap_pxc_csr_cfg_c_portgate_close_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_portgate_close_t();
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
    
        typedef pu_cpp_int< 1 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_portgate_close_t
    
class cap_pxc_csr_cfg_c_portgate_open_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_portgate_open_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_portgate_open_t(string name = "cap_pxc_csr_cfg_c_portgate_open_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_portgate_open_t();
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
    
        typedef pu_cpp_int< 1 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_portgate_open_t
    
class cap_pxc_csr_sta_c_port_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_port_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_port_rst_t(string name = "cap_pxc_csr_sta_c_port_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_port_rst_t();
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
    
        typedef pu_cpp_int< 5 > mac_ltssm_cpp_int_t;
        cpp_int int_var__mac_ltssm;
        void mac_ltssm (const cpp_int  & l__val);
        cpp_int mac_ltssm() const;
    
        typedef pu_cpp_int< 1 > mac_dl_up_cpp_int_t;
        cpp_int int_var__mac_dl_up;
        void mac_dl_up (const cpp_int  & l__val);
        cpp_int mac_dl_up() const;
    
        typedef pu_cpp_int< 1 > sec_bus_rst_cpp_int_t;
        cpp_int int_var__sec_bus_rst;
        void sec_bus_rst (const cpp_int  & l__val);
        cpp_int sec_bus_rst() const;
    
        typedef pu_cpp_int< 1 > perstn_cpp_int_t;
        cpp_int int_var__perstn;
        void perstn (const cpp_int  & l__val);
        cpp_int perstn() const;
    
        typedef pu_cpp_int< 1 > pl_rstn_cpp_int_t;
        cpp_int int_var__pl_rstn;
        void pl_rstn (const cpp_int  & l__val);
        cpp_int pl_rstn() const;
    
        typedef pu_cpp_int< 1 > pl_npor_cpp_int_t;
        cpp_int int_var__pl_npor;
        void pl_npor (const cpp_int  & l__val);
        cpp_int pl_npor() const;
    
        typedef pu_cpp_int< 1 > phystatus_or_cpp_int_t;
        cpp_int int_var__phystatus_or;
        void phystatus_or (const cpp_int  & l__val);
        cpp_int phystatus_or() const;
    
        typedef pu_cpp_int< 1 > tl_npor_cpp_int_t;
        cpp_int int_var__tl_npor;
        void tl_npor (const cpp_int  & l__val);
        cpp_int tl_npor() const;
    
        typedef pu_cpp_int< 1 > tl_rstn_cpp_int_t;
        cpp_int int_var__tl_rstn;
        void tl_rstn (const cpp_int  & l__val);
        cpp_int tl_rstn() const;
    
        typedef pu_cpp_int< 1 > tl_crstn_cpp_int_t;
        cpp_int int_var__tl_crstn;
        void tl_crstn (const cpp_int  & l__val);
        cpp_int tl_crstn() const;
    
        typedef pu_cpp_int< 1 > app_hperst_cpp_int_t;
        cpp_int int_var__app_hperst;
        void app_hperst (const cpp_int  & l__val);
        cpp_int app_hperst() const;
    
        typedef pu_cpp_int< 1 > app_sperst_cpp_int_t;
        cpp_int int_var__app_sperst;
        void app_sperst (const cpp_int  & l__val);
        cpp_int app_sperst() const;
    
        typedef pu_cpp_int< 1 > app_xperst_cpp_int_t;
        cpp_int int_var__app_xperst;
        void app_xperst (const cpp_int  & l__val);
        cpp_int app_xperst() const;
    
        typedef pu_cpp_int< 1 > app_xrst_cpp_int_t;
        cpp_int int_var__app_xrst;
        void app_xrst (const cpp_int  & l__val);
        cpp_int app_xrst() const;
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
}; // cap_pxc_csr_sta_c_port_rst_t
    
class cap_pxc_csr_sta_c_port_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_sta_c_port_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_sta_c_port_mac_t(string name = "cap_pxc_csr_sta_c_port_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_sta_c_port_mac_t();
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
    
        typedef pu_cpp_int< 5 > ltssm_cpp_int_t;
        cpp_int int_var__ltssm;
        void ltssm (const cpp_int  & l__val);
        cpp_int ltssm() const;
    
        typedef pu_cpp_int< 1 > dl_up_cpp_int_t;
        cpp_int int_var__dl_up;
        void dl_up (const cpp_int  & l__val);
        cpp_int dl_up() const;
    
        typedef pu_cpp_int< 1 > portgate_open_cpp_int_t;
        cpp_int int_var__portgate_open;
        void portgate_open (const cpp_int  & l__val);
        cpp_int portgate_open() const;
    
        typedef pu_cpp_int< 4 > lp_state_cpp_int_t;
        cpp_int int_var__lp_state;
        void lp_state (const cpp_int  & l__val);
        cpp_int lp_state() const;
    
        typedef pu_cpp_int< 1 > trn2ltssm_l0s_cpp_int_t;
        cpp_int int_var__trn2ltssm_l0s;
        void trn2ltssm_l0s (const cpp_int  & l__val);
        cpp_int trn2ltssm_l0s() const;
    
        typedef pu_cpp_int< 1 > trn2ltssm_l1_cpp_int_t;
        cpp_int int_var__trn2ltssm_l1;
        void trn2ltssm_l1 (const cpp_int  & l__val);
        cpp_int trn2ltssm_l1() const;
    
        typedef pu_cpp_int< 1 > trn2ltssm_l2_cpp_int_t;
        cpp_int int_var__trn2ltssm_l2;
        void trn2ltssm_l2 (const cpp_int  & l__val);
        cpp_int trn2ltssm_l2() const;
    
        typedef pu_cpp_int< 1 > dpc_active_cpp_int_t;
        cpp_int int_var__dpc_active;
        void dpc_active (const cpp_int  & l__val);
        cpp_int dpc_active() const;
    
        typedef pu_cpp_int< 4 > rc_int_pinstate_cpp_int_t;
        cpp_int int_var__rc_int_pinstate;
        void rc_int_pinstate (const cpp_int  & l__val);
        cpp_int rc_int_pinstate() const;
    
        typedef pu_cpp_int< 1 > pm_turnoffstatus_cpp_int_t;
        cpp_int int_var__pm_turnoffstatus;
        void pm_turnoffstatus (const cpp_int  & l__val);
        cpp_int pm_turnoffstatus() const;
    
        typedef pu_cpp_int< 4 > pm_clkstatus_cpp_int_t;
        cpp_int int_var__pm_clkstatus;
        void pm_clkstatus (const cpp_int  & l__val);
        cpp_int pm_clkstatus() const;
    
        typedef pu_cpp_int< 1 > tlp_tx_inhibited_cpp_int_t;
        cpp_int int_var__tlp_tx_inhibited;
        void tlp_tx_inhibited (const cpp_int  & l__val);
        cpp_int tlp_tx_inhibited() const;
    
        typedef pu_cpp_int< 1 > dllp_tx_inhibited_cpp_int_t;
        cpp_int int_var__dllp_tx_inhibited;
        void dllp_tx_inhibited (const cpp_int  & l__val);
        cpp_int dllp_tx_inhibited() const;
    
}; // cap_pxc_csr_sta_c_port_mac_t
    
class cap_pxc_csr_cnt_c_port_rx_cfg0_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cnt_c_port_rx_cfg0_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cnt_c_port_rx_cfg0_req_t(string name = "cap_pxc_csr_cnt_c_port_rx_cfg0_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cnt_c_port_rx_cfg0_req_t();
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
    
        typedef pu_cpp_int< 16 > tlp_cpp_int_t;
        cpp_int int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_port_rx_cfg0_req_t
    
class cap_pxc_csr_cnt_c_tl_tx_cpl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cnt_c_tl_tx_cpl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cnt_c_tl_tx_cpl_t(string name = "cap_pxc_csr_cnt_c_tl_tx_cpl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cnt_c_tl_tx_cpl_t();
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
    
        typedef pu_cpp_int< 40 > tlp_cpp_int_t;
        cpp_int int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_tl_tx_cpl_t
    
class cap_pxc_csr_cnt_c_tl_tx_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cnt_c_tl_tx_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cnt_c_tl_tx_req_t(string name = "cap_pxc_csr_cnt_c_tl_tx_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cnt_c_tl_tx_req_t();
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
    
        typedef pu_cpp_int< 40 > tlp_cpp_int_t;
        cpp_int int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_tl_tx_req_t
    
class cap_pxc_csr_cnt_c_tl_rx_cpl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cnt_c_tl_rx_cpl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cnt_c_tl_rx_cpl_t(string name = "cap_pxc_csr_cnt_c_tl_rx_cpl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cnt_c_tl_rx_cpl_t();
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
    
        typedef pu_cpp_int< 40 > tlp_cpp_int_t;
        cpp_int int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_tl_rx_cpl_t
    
class cap_pxc_csr_cnt_c_tl_rx_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cnt_c_tl_rx_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cnt_c_tl_rx_req_t(string name = "cap_pxc_csr_cnt_c_tl_rx_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cnt_c_tl_rx_req_t();
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
    
        typedef pu_cpp_int< 40 > tlp_cpp_int_t;
        cpp_int int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_tl_rx_req_t
    
class cap_pxc_csr_cfg_c_port_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_port_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_port_mac_t(string name = "cap_pxc_csr_cfg_c_port_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_port_mac_t();
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
    
        typedef pu_cpp_int< 1 > reset_cpp_int_t;
        cpp_int int_var__reset;
        void reset (const cpp_int  & l__val);
        cpp_int reset() const;
    
        typedef pu_cpp_int< 1 > ltssm_en_cpp_int_t;
        cpp_int int_var__ltssm_en;
        void ltssm_en (const cpp_int  & l__val);
        cpp_int ltssm_en() const;
    
        typedef pu_cpp_int< 1 > ltssm_en_portgate_qual_cpp_int_t;
        cpp_int int_var__ltssm_en_portgate_qual;
        void ltssm_en_portgate_qual (const cpp_int  & l__val);
        cpp_int ltssm_en_portgate_qual() const;
    
        typedef pu_cpp_int< 1 > cfg_retry_en_cpp_int_t;
        cpp_int int_var__cfg_retry_en;
        void cfg_retry_en (const cpp_int  & l__val);
        cpp_int cfg_retry_en() const;
    
        typedef pu_cpp_int< 22 > tl_clock_freq_cpp_int_t;
        cpp_int int_var__tl_clock_freq;
        void tl_clock_freq (const cpp_int  & l__val);
        cpp_int tl_clock_freq() const;
    
        typedef pu_cpp_int< 1 > port_type_cpp_int_t;
        cpp_int int_var__port_type;
        void port_type (const cpp_int  & l__val);
        cpp_int port_type() const;
    
        typedef pu_cpp_int< 1 > flush_marker_en_cpp_int_t;
        cpp_int int_var__flush_marker_en;
        void flush_marker_en (const cpp_int  & l__val);
        cpp_int flush_marker_en() const;
    
        typedef pu_cpp_int< 1 > tx_stream_cpp_int_t;
        cpp_int int_var__tx_stream;
        void tx_stream (const cpp_int  & l__val);
        cpp_int tx_stream() const;
    
        typedef pu_cpp_int< 3 > pl_exit_en_cpp_int_t;
        cpp_int int_var__pl_exit_en;
        void pl_exit_en (const cpp_int  & l__val);
        cpp_int pl_exit_en() const;
    
        typedef pu_cpp_int< 1 > pm_auxpwr_cpp_int_t;
        cpp_int int_var__pm_auxpwr;
        void pm_auxpwr (const cpp_int  & l__val);
        cpp_int pm_auxpwr() const;
    
        typedef pu_cpp_int< 1 > pm_event_cpp_int_t;
        cpp_int int_var__pm_event;
        void pm_event (const cpp_int  & l__val);
        cpp_int pm_event() const;
    
        typedef pu_cpp_int< 10 > pm_data_cpp_int_t;
        cpp_int int_var__pm_data;
        void pm_data (const cpp_int  & l__val);
        cpp_int pm_data() const;
    
        typedef pu_cpp_int< 1 > pm_turnoffcontrol_cpp_int_t;
        cpp_int int_var__pm_turnoffcontrol;
        void pm_turnoffcontrol (const cpp_int  & l__val);
        cpp_int pm_turnoffcontrol() const;
    
        typedef pu_cpp_int< 1 > pm_clkcontrol_cpp_int_t;
        cpp_int int_var__pm_clkcontrol;
        void pm_clkcontrol (const cpp_int  & l__val);
        cpp_int pm_clkcontrol() const;
    
        typedef pu_cpp_int< 1 > inject_lcrc_cpp_int_t;
        cpp_int int_var__inject_lcrc;
        void inject_lcrc (const cpp_int  & l__val);
        cpp_int inject_lcrc() const;
    
        typedef pu_cpp_int< 1 > inject_ecrc_cpp_int_t;
        cpp_int int_var__inject_ecrc;
        void inject_ecrc (const cpp_int  & l__val);
        cpp_int inject_ecrc() const;
    
        typedef pu_cpp_int< 1 > auto_retry_en_cpp_int_t;
        cpp_int int_var__auto_retry_en;
        void auto_retry_en (const cpp_int  & l__val);
        cpp_int auto_retry_en() const;
    
        typedef pu_cpp_int< 1 > margining_ready_cpp_int_t;
        cpp_int int_var__margining_ready;
        void margining_ready (const cpp_int  & l__val);
        cpp_int margining_ready() const;
    
        typedef pu_cpp_int< 1 > margining_sw_ready_cpp_int_t;
        cpp_int int_var__margining_sw_ready;
        void margining_sw_ready (const cpp_int  & l__val);
        cpp_int margining_sw_ready() const;
    
        typedef pu_cpp_int< 1 > aer_common_en_cpp_int_t;
        cpp_int int_var__aer_common_en;
        void aer_common_en (const cpp_int  & l__val);
        cpp_int aer_common_en() const;
    
        typedef pu_cpp_int< 1 > rc_legacy_axi_en_cpp_int_t;
        cpp_int int_var__rc_legacy_axi_en;
        void rc_legacy_axi_en (const cpp_int  & l__val);
        cpp_int rc_legacy_axi_en() const;
    
}; // cap_pxc_csr_cfg_c_port_mac_t
    
class cap_pxc_csr_cfg_c_mac_k_bar0windows_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_bar0windows_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_bar0windows_t(string name = "cap_pxc_csr_cfg_c_mac_k_bar0windows_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_bar0windows_t();
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
    
        typedef pu_cpp_int< 4 > bits227_224_cpp_int_t;
        cpp_int int_var__bits227_224;
        void bits227_224 (const cpp_int  & l__val);
        cpp_int bits227_224() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_bar0windows_t
    
class cap_pxc_csr_cfg_c_mac_k_equpreset16_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_equpreset16_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_equpreset16_t(string name = "cap_pxc_csr_cfg_c_mac_k_equpreset16_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_equpreset16_t();
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
    
        typedef pu_cpp_int< 16 > lane0_cpp_int_t;
        cpp_int int_var__lane0;
        void lane0 (const cpp_int  & l__val);
        cpp_int lane0() const;
    
        typedef pu_cpp_int< 16 > lane1_cpp_int_t;
        cpp_int int_var__lane1;
        void lane1 (const cpp_int  & l__val);
        cpp_int lane1() const;
    
        typedef pu_cpp_int< 16 > lane2_cpp_int_t;
        cpp_int int_var__lane2;
        void lane2 (const cpp_int  & l__val);
        cpp_int lane2() const;
    
        typedef pu_cpp_int< 16 > lane3_cpp_int_t;
        cpp_int int_var__lane3;
        void lane3 (const cpp_int  & l__val);
        cpp_int lane3() const;
    
        typedef pu_cpp_int< 16 > lane4_cpp_int_t;
        cpp_int int_var__lane4;
        void lane4 (const cpp_int  & l__val);
        cpp_int lane4() const;
    
        typedef pu_cpp_int< 16 > lane5_cpp_int_t;
        cpp_int int_var__lane5;
        void lane5 (const cpp_int  & l__val);
        cpp_int lane5() const;
    
        typedef pu_cpp_int< 16 > lane6_cpp_int_t;
        cpp_int int_var__lane6;
        void lane6 (const cpp_int  & l__val);
        cpp_int lane6() const;
    
        typedef pu_cpp_int< 16 > lane7_cpp_int_t;
        cpp_int int_var__lane7;
        void lane7 (const cpp_int  & l__val);
        cpp_int lane7() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_equpreset16_t
    
class cap_pxc_csr_cfg_c_mac_k_equpreset_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_equpreset_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_equpreset_t(string name = "cap_pxc_csr_cfg_c_mac_k_equpreset_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_equpreset_t();
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
    
        typedef pu_cpp_int< 16 > lane0_cpp_int_t;
        cpp_int int_var__lane0;
        void lane0 (const cpp_int  & l__val);
        cpp_int lane0() const;
    
        typedef pu_cpp_int< 16 > lane1_cpp_int_t;
        cpp_int int_var__lane1;
        void lane1 (const cpp_int  & l__val);
        cpp_int lane1() const;
    
        typedef pu_cpp_int< 16 > lane2_cpp_int_t;
        cpp_int int_var__lane2;
        void lane2 (const cpp_int  & l__val);
        cpp_int lane2() const;
    
        typedef pu_cpp_int< 16 > lane3_cpp_int_t;
        cpp_int int_var__lane3;
        void lane3 (const cpp_int  & l__val);
        cpp_int lane3() const;
    
        typedef pu_cpp_int< 16 > lane4_cpp_int_t;
        cpp_int int_var__lane4;
        void lane4 (const cpp_int  & l__val);
        cpp_int lane4() const;
    
        typedef pu_cpp_int< 16 > lane5_cpp_int_t;
        cpp_int int_var__lane5;
        void lane5 (const cpp_int  & l__val);
        cpp_int lane5() const;
    
        typedef pu_cpp_int< 16 > lane6_cpp_int_t;
        cpp_int int_var__lane6;
        void lane6 (const cpp_int  & l__val);
        cpp_int lane6() const;
    
        typedef pu_cpp_int< 16 > lane7_cpp_int_t;
        cpp_int int_var__lane7;
        void lane7 (const cpp_int  & l__val);
        cpp_int lane7() const;
    
        typedef pu_cpp_int< 16 > lane8_cpp_int_t;
        cpp_int int_var__lane8;
        void lane8 (const cpp_int  & l__val);
        cpp_int lane8() const;
    
        typedef pu_cpp_int< 16 > lane9_cpp_int_t;
        cpp_int int_var__lane9;
        void lane9 (const cpp_int  & l__val);
        cpp_int lane9() const;
    
        typedef pu_cpp_int< 16 > lane10_cpp_int_t;
        cpp_int int_var__lane10;
        void lane10 (const cpp_int  & l__val);
        cpp_int lane10() const;
    
        typedef pu_cpp_int< 16 > lane11_cpp_int_t;
        cpp_int int_var__lane11;
        void lane11 (const cpp_int  & l__val);
        cpp_int lane11() const;
    
        typedef pu_cpp_int< 16 > lane12_cpp_int_t;
        cpp_int int_var__lane12;
        void lane12 (const cpp_int  & l__val);
        cpp_int lane12() const;
    
        typedef pu_cpp_int< 16 > lane13_cpp_int_t;
        cpp_int int_var__lane13;
        void lane13 (const cpp_int  & l__val);
        cpp_int lane13() const;
    
        typedef pu_cpp_int< 16 > lane14_cpp_int_t;
        cpp_int int_var__lane14;
        void lane14 (const cpp_int  & l__val);
        cpp_int lane14() const;
    
        typedef pu_cpp_int< 16 > lane15_cpp_int_t;
        cpp_int int_var__lane15;
        void lane15 (const cpp_int  & l__val);
        cpp_int lane15() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_equpreset_t
    
class cap_pxc_csr_cfg_c_mac_k_pciconf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_pciconf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_pciconf_t(string name = "cap_pxc_csr_cfg_c_mac_k_pciconf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_pciconf_t();
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
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        cpp_int int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        cpp_int int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word2_cpp_int_t;
        cpp_int int_var__word2;
        void word2 (const cpp_int  & l__val);
        cpp_int word2() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        cpp_int int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
        typedef pu_cpp_int< 32 > word4_cpp_int_t;
        cpp_int int_var__word4;
        void word4 (const cpp_int  & l__val);
        cpp_int word4() const;
    
        typedef pu_cpp_int< 32 > word5_cpp_int_t;
        cpp_int int_var__word5;
        void word5 (const cpp_int  & l__val);
        cpp_int word5() const;
    
        typedef pu_cpp_int< 32 > word6_cpp_int_t;
        cpp_int int_var__word6;
        void word6 (const cpp_int  & l__val);
        cpp_int word6() const;
    
        typedef pu_cpp_int< 32 > word7_cpp_int_t;
        cpp_int int_var__word7;
        void word7 (const cpp_int  & l__val);
        cpp_int word7() const;
    
        typedef pu_cpp_int< 32 > word8_cpp_int_t;
        cpp_int int_var__word8;
        void word8 (const cpp_int  & l__val);
        cpp_int word8() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_pciconf_t
    
class cap_pxc_csr_cfg_c_mac_k_pexconf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_pexconf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_pexconf_t(string name = "cap_pxc_csr_cfg_c_mac_k_pexconf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_pexconf_t();
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
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        cpp_int int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        cpp_int int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word2_cpp_int_t;
        cpp_int int_var__word2;
        void word2 (const cpp_int  & l__val);
        cpp_int word2() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        cpp_int int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
        typedef pu_cpp_int< 32 > word4_cpp_int_t;
        cpp_int int_var__word4;
        void word4 (const cpp_int  & l__val);
        cpp_int word4() const;
    
        typedef pu_cpp_int< 32 > word5_cpp_int_t;
        cpp_int int_var__word5;
        void word5 (const cpp_int  & l__val);
        cpp_int word5() const;
    
        typedef pu_cpp_int< 32 > word6_cpp_int_t;
        cpp_int int_var__word6;
        void word6 (const cpp_int  & l__val);
        cpp_int word6() const;
    
        typedef pu_cpp_int< 32 > word7_cpp_int_t;
        cpp_int int_var__word7;
        void word7 (const cpp_int  & l__val);
        cpp_int word7() const;
    
        typedef pu_cpp_int< 32 > word8_cpp_int_t;
        cpp_int int_var__word8;
        void word8 (const cpp_int  & l__val);
        cpp_int word8() const;
    
        typedef pu_cpp_int< 32 > word9_cpp_int_t;
        cpp_int int_var__word9;
        void word9 (const cpp_int  & l__val);
        cpp_int word9() const;
    
        typedef pu_cpp_int< 32 > word10_cpp_int_t;
        cpp_int int_var__word10;
        void word10 (const cpp_int  & l__val);
        cpp_int word10() const;
    
        typedef pu_cpp_int< 32 > word11_cpp_int_t;
        cpp_int int_var__word11;
        void word11 (const cpp_int  & l__val);
        cpp_int word11() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_pexconf_t
    
class cap_pxc_csr_cfg_c_mac_k_lmr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_lmr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_lmr_t(string name = "cap_pxc_csr_cfg_c_mac_k_lmr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_lmr_t();
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
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        cpp_int int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        cpp_int int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        cpp_int int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_lmr_t
    
class cap_pxc_csr_cfg_c_mac_k_rx_cred_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_rx_cred_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_rx_cred_t(string name = "cap_pxc_csr_cfg_c_mac_k_rx_cred_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_rx_cred_t();
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
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        cpp_int int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        cpp_int int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        cpp_int int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_rx_cred_t
    
class cap_pxc_csr_cfg_c_mac_k_gen_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_cfg_c_mac_k_gen_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_cfg_c_mac_k_gen_t(string name = "cap_pxc_csr_cfg_c_mac_k_gen_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_cfg_c_mac_k_gen_t();
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
    
        typedef pu_cpp_int< 4 > spec_version_cpp_int_t;
        cpp_int int_var__spec_version;
        void spec_version (const cpp_int  & l__val);
        cpp_int spec_version() const;
    
        typedef pu_cpp_int< 4 > port_type_cpp_int_t;
        cpp_int int_var__port_type;
        void port_type (const cpp_int  & l__val);
        cpp_int port_type() const;
    
        typedef pu_cpp_int< 1 > sris_mode_cpp_int_t;
        cpp_int int_var__sris_mode;
        void sris_mode (const cpp_int  & l__val);
        cpp_int sris_mode() const;
    
        typedef pu_cpp_int< 1 > gen1_supported_cpp_int_t;
        cpp_int int_var__gen1_supported;
        void gen1_supported (const cpp_int  & l__val);
        cpp_int gen1_supported() const;
    
        typedef pu_cpp_int< 1 > gen2_supported_cpp_int_t;
        cpp_int int_var__gen2_supported;
        void gen2_supported (const cpp_int  & l__val);
        cpp_int gen2_supported() const;
    
        typedef pu_cpp_int< 1 > gen3_supported_cpp_int_t;
        cpp_int int_var__gen3_supported;
        void gen3_supported (const cpp_int  & l__val);
        cpp_int gen3_supported() const;
    
        typedef pu_cpp_int< 1 > gen4_supported_cpp_int_t;
        cpp_int int_var__gen4_supported;
        void gen4_supported (const cpp_int  & l__val);
        cpp_int gen4_supported() const;
    
        typedef pu_cpp_int< 3 > reserved1_cpp_int_t;
        cpp_int int_var__reserved1;
        void reserved1 (const cpp_int  & l__val);
        cpp_int reserved1() const;
    
        typedef pu_cpp_int< 1 > bfm_mode_cpp_int_t;
        cpp_int int_var__bfm_mode;
        void bfm_mode (const cpp_int  & l__val);
        cpp_int bfm_mode() const;
    
        typedef pu_cpp_int< 1 > lane_reverse_en_cpp_int_t;
        cpp_int int_var__lane_reverse_en;
        void lane_reverse_en (const cpp_int  & l__val);
        cpp_int lane_reverse_en() const;
    
        typedef pu_cpp_int< 3 > reserved2_cpp_int_t;
        cpp_int int_var__reserved2;
        void reserved2 (const cpp_int  & l__val);
        cpp_int reserved2() const;
    
        typedef pu_cpp_int< 1 > rx_cutthru_en_cpp_int_t;
        cpp_int int_var__rx_cutthru_en;
        void rx_cutthru_en (const cpp_int  & l__val);
        cpp_int rx_cutthru_en() const;
    
        typedef pu_cpp_int< 1 > tx_cutthru_en_cpp_int_t;
        cpp_int int_var__tx_cutthru_en;
        void tx_cutthru_en (const cpp_int  & l__val);
        cpp_int tx_cutthru_en() const;
    
        typedef pu_cpp_int< 1 > eq_ph23_en_cpp_int_t;
        cpp_int int_var__eq_ph23_en;
        void eq_ph23_en (const cpp_int  & l__val);
        cpp_int eq_ph23_en() const;
    
        typedef pu_cpp_int< 4 > disable_lanes_cpp_int_t;
        cpp_int int_var__disable_lanes;
        void disable_lanes (const cpp_int  & l__val);
        cpp_int disable_lanes() const;
    
        typedef pu_cpp_int< 1 > rxelecidle_cfg_cpp_int_t;
        cpp_int int_var__rxelecidle_cfg;
        void rxelecidle_cfg (const cpp_int  & l__val);
        cpp_int rxelecidle_cfg() const;
    
        typedef pu_cpp_int< 1 > reserved3_cpp_int_t;
        cpp_int int_var__reserved3;
        void reserved3 (const cpp_int  & l__val);
        cpp_int reserved3() const;
    
        typedef pu_cpp_int< 1 > reverved4_cpp_int_t;
        cpp_int int_var__reverved4;
        void reverved4 (const cpp_int  & l__val);
        cpp_int reverved4() const;
    
        typedef pu_cpp_int< 1 > ecrc_gen_mode_cpp_int_t;
        cpp_int int_var__ecrc_gen_mode;
        void ecrc_gen_mode (const cpp_int  & l__val);
        cpp_int ecrc_gen_mode() const;
    
        typedef pu_cpp_int< 1 > pf_intr_tx_mode_cpp_int_t;
        cpp_int int_var__pf_intr_tx_mode;
        void pf_intr_tx_mode (const cpp_int  & l__val);
        cpp_int pf_intr_tx_mode() const;
    
        typedef pu_cpp_int< 2 > tx_err_mgmt_cpp_int_t;
        cpp_int int_var__tx_err_mgmt;
        void tx_err_mgmt (const cpp_int  & l__val);
        cpp_int tx_err_mgmt() const;
    
        typedef pu_cpp_int< 1 > rx_wdog_en_cpp_int_t;
        cpp_int int_var__rx_wdog_en;
        void rx_wdog_en (const cpp_int  & l__val);
        cpp_int rx_wdog_en() const;
    
        typedef pu_cpp_int< 1 > upcfg_en_cpp_int_t;
        cpp_int int_var__upcfg_en;
        void upcfg_en (const cpp_int  & l__val);
        cpp_int upcfg_en() const;
    
        typedef pu_cpp_int< 1 > rxvalid_filter_cpp_int_t;
        cpp_int int_var__rxvalid_filter;
        void rxvalid_filter (const cpp_int  & l__val);
        cpp_int rxvalid_filter() const;
    
        typedef pu_cpp_int< 1 > vf_intr_tx_mode_cpp_int_t;
        cpp_int int_var__vf_intr_tx_mode;
        void vf_intr_tx_mode (const cpp_int  & l__val);
        cpp_int vf_intr_tx_mode() const;
    
        typedef pu_cpp_int< 9 > reserved5_cpp_int_t;
        cpp_int int_var__reserved5;
        void reserved5 (const cpp_int  & l__val);
        cpp_int reserved5() const;
    
        typedef pu_cpp_int< 4 > rx_wdog_thres_low_cpp_int_t;
        cpp_int int_var__rx_wdog_thres_low;
        void rx_wdog_thres_low (const cpp_int  & l__val);
        cpp_int rx_wdog_thres_low() const;
    
        typedef pu_cpp_int< 4 > rx_wdog_thres_high_cpp_int_t;
        cpp_int int_var__rx_wdog_thres_high;
        void rx_wdog_thres_high (const cpp_int  & l__val);
        cpp_int rx_wdog_thres_high() const;
    
        typedef pu_cpp_int< 4 > limit_tx_bfr_size_cpp_int_t;
        cpp_int int_var__limit_tx_bfr_size;
        void limit_tx_bfr_size (const cpp_int  & l__val);
        cpp_int limit_tx_bfr_size() const;
    
        typedef pu_cpp_int< 4 > reserved4_cpp_int_t;
        cpp_int int_var__reserved4;
        void reserved4 (const cpp_int  & l__val);
        cpp_int reserved4() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_gen_t
    
class cap_pxc_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxc_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxc_csr_t(string name = "cap_pxc_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxc_csr_t();
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
    
        cap_pxc_csr_cfg_c_mac_k_gen_t cfg_c_mac_k_gen;
    
        cap_pxc_csr_cfg_c_mac_k_rx_cred_t cfg_c_mac_k_rx_cred;
    
        cap_pxc_csr_cfg_c_mac_k_lmr_t cfg_c_mac_k_lmr;
    
        cap_pxc_csr_cfg_c_mac_k_pexconf_t cfg_c_mac_k_pexconf;
    
        cap_pxc_csr_cfg_c_mac_k_pciconf_t cfg_c_mac_k_pciconf;
    
        cap_pxc_csr_cfg_c_mac_k_equpreset_t cfg_c_mac_k_equpreset;
    
        cap_pxc_csr_cfg_c_mac_k_equpreset16_t cfg_c_mac_k_equpreset16;
    
        cap_pxc_csr_cfg_c_mac_k_bar0windows_t cfg_c_mac_k_bar0windows;
    
        cap_pxc_csr_cfg_c_port_mac_t cfg_c_port_mac;
    
        cap_pxc_csr_cnt_c_tl_rx_req_t cnt_c_tl_rx_req;
    
        cap_pxc_csr_cnt_c_tl_rx_cpl_t cnt_c_tl_rx_cpl;
    
        cap_pxc_csr_cnt_c_tl_tx_req_t cnt_c_tl_tx_req;
    
        cap_pxc_csr_cnt_c_tl_tx_cpl_t cnt_c_tl_tx_cpl;
    
        cap_pxc_csr_cnt_c_port_rx_cfg0_req_t cnt_c_port_rx_cfg0_req;
    
        cap_pxc_csr_sta_c_port_mac_t sta_c_port_mac;
    
        cap_pxc_csr_sta_c_port_rst_t sta_c_port_rst;
    
        cap_pxc_csr_cfg_c_portgate_open_t cfg_c_portgate_open;
    
        cap_pxc_csr_cfg_c_portgate_close_t cfg_c_portgate_close;
    
        cap_pxc_csr_cfg_c_ltr_latency_t cfg_c_ltr_latency;
    
        cap_pxc_csr_cfg_c_autonomous_linkwidth_t cfg_c_autonomous_linkwidth;
    
        cap_pxc_csr_cfg_c_tl_report_t cfg_c_tl_report;
    
        cap_pxc_csr_cfg_c_ecc_disable_t cfg_c_ecc_disable;
    
        cap_pxc_csr_cfg_c_brsw_t cfg_c_brsw;
    
        cap_pxc_csr_sta_c_brsw_t sta_c_brsw;
    
        cap_pxc_csr_cfg_c_mac_test_in_t cfg_c_mac_test_in;
    
        cap_pxc_csr_cfg_c_mac_ssvid_cap_t cfg_c_mac_ssvid_cap;
    
        cap_pxc_csr_sta_c_tx_fc_credits_t sta_c_tx_fc_credits;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_0_t sta_c_ecc_rxbuf_0;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_1_t sta_c_ecc_rxbuf_1;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_2_t sta_c_ecc_rxbuf_2;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_3_t sta_c_ecc_rxbuf_3;
    
        cap_pxc_csr_sat_c_port_cnt_tx_drop_t sat_c_port_cnt_tx_drop;
    
        cap_pxc_csr_sat_c_port_cnt_rc_legacy_int_rx_t sat_c_port_cnt_rc_legacy_int_rx;
    
        cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t sat_c_port_cnt_rx_malform_tlp;
    
        cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t sat_c_port_cnt_rx_framing_err;
    
        cap_pxc_csr_sat_c_port_cnt_rx_ecrc_err_t sat_c_port_cnt_rx_ecrc_err;
    
        cap_pxc_csr_sat_c_port_cnt_rxbuf_ecc_err_t sat_c_port_cnt_rxbuf_ecc_err;
    
        cap_pxc_csr_sat_c_port_cnt_rx_nullify_t sat_c_port_cnt_rx_nullify;
    
        cap_pxc_csr_sat_c_port_cnt_rx_watchdog_nullify_t sat_c_port_cnt_rx_watchdog_nullify;
    
        cap_pxc_csr_sat_c_port_cnt_rx_unsupp_t sat_c_port_cnt_rx_unsupp;
    
        cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t sat_c_port_cnt_txbfr_overflow;
    
        cap_pxc_csr_sta_c_port_phystatus_t sta_c_port_phystatus;
    
        cap_pxc_csr_csr_intr_t csr_intr;
    
        cap_pxc_csr_cfg_c_spare_t cfg_c_spare;
    
        cap_pxc_csr_dhs_c_mac_apb_t dhs_c_mac_apb;
    
        cap_pxc_csr_intgrp_status_t int_groups;
    
        cap_pxc_csr_int_c_mac_t int_c_mac;
    
        cap_pxc_csr_int_c_ecc_t int_c_ecc;
    
}; // cap_pxc_csr_t
    
#endif // CAP_PXC_CSR_H
        