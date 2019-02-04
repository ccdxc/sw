
#ifndef CAP_INTR_CSR_H
#define CAP_INTR_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_intr_csr_int_intr_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_int_intr_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_int_intr_ecc_int_enable_clear_t(string name = "cap_intr_csr_int_intr_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_int_intr_ecc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > intr_state_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__intr_state_uncorrectable_enable;
        void intr_state_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int intr_state_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > intr_state_correctable_enable_cpp_int_t;
        cpp_int int_var__intr_state_correctable_enable;
        void intr_state_correctable_enable (const cpp_int  & l__val);
        cpp_int intr_state_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_uncorrectable_enable;
        void intr_pba_cfg_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_correctable_enable_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_correctable_enable;
        void intr_pba_cfg_correctable_enable (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__intr_coalesce_uncorrectable_enable;
        void intr_coalesce_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int intr_coalesce_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_correctable_enable_cpp_int_t;
        cpp_int int_var__intr_coalesce_correctable_enable;
        void intr_coalesce_correctable_enable (const cpp_int  & l__val);
        cpp_int intr_coalesce_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > intr_legacy_fifo_overrun_enable_cpp_int_t;
        cpp_int int_var__intr_legacy_fifo_overrun_enable;
        void intr_legacy_fifo_overrun_enable (const cpp_int  & l__val);
        cpp_int intr_legacy_fifo_overrun_enable() const;
    
        typedef pu_cpp_int< 1 > int_credit_positive_after_return_enable_cpp_int_t;
        cpp_int int_var__int_credit_positive_after_return_enable;
        void int_credit_positive_after_return_enable (const cpp_int  & l__val);
        cpp_int int_credit_positive_after_return_enable() const;
    
        typedef pu_cpp_int< 1 > int_credits_max_debug_threshold_enable_cpp_int_t;
        cpp_int int_var__int_credits_max_debug_threshold_enable;
        void int_credits_max_debug_threshold_enable (const cpp_int  & l__val);
        cpp_int int_credits_max_debug_threshold_enable() const;
    
        typedef pu_cpp_int< 1 > intr_bresp_err_enable_cpp_int_t;
        cpp_int int_var__intr_bresp_err_enable;
        void intr_bresp_err_enable (const cpp_int  & l__val);
        cpp_int intr_bresp_err_enable() const;
    
}; // cap_intr_csr_int_intr_ecc_int_enable_clear_t
    
class cap_intr_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_intreg_t(string name = "cap_intr_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > intr_state_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__intr_state_uncorrectable_interrupt;
        void intr_state_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int intr_state_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_state_correctable_interrupt_cpp_int_t;
        cpp_int int_var__intr_state_correctable_interrupt;
        void intr_state_correctable_interrupt (const cpp_int  & l__val);
        cpp_int intr_state_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_uncorrectable_interrupt;
        void intr_pba_cfg_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_correctable_interrupt_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_correctable_interrupt;
        void intr_pba_cfg_correctable_interrupt (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__intr_coalesce_uncorrectable_interrupt;
        void intr_coalesce_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int intr_coalesce_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_correctable_interrupt_cpp_int_t;
        cpp_int int_var__intr_coalesce_correctable_interrupt;
        void intr_coalesce_correctable_interrupt (const cpp_int  & l__val);
        cpp_int intr_coalesce_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_legacy_fifo_overrun_interrupt_cpp_int_t;
        cpp_int int_var__intr_legacy_fifo_overrun_interrupt;
        void intr_legacy_fifo_overrun_interrupt (const cpp_int  & l__val);
        cpp_int intr_legacy_fifo_overrun_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_credit_positive_after_return_interrupt_cpp_int_t;
        cpp_int int_var__int_credit_positive_after_return_interrupt;
        void int_credit_positive_after_return_interrupt (const cpp_int  & l__val);
        cpp_int int_credit_positive_after_return_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_credits_max_debug_threshold_interrupt_cpp_int_t;
        cpp_int int_var__int_credits_max_debug_threshold_interrupt;
        void int_credits_max_debug_threshold_interrupt (const cpp_int  & l__val);
        cpp_int int_credits_max_debug_threshold_interrupt() const;
    
        typedef pu_cpp_int< 1 > intr_bresp_err_interrupt_cpp_int_t;
        cpp_int int_var__intr_bresp_err_interrupt;
        void intr_bresp_err_interrupt (const cpp_int  & l__val);
        cpp_int intr_bresp_err_interrupt() const;
    
}; // cap_intr_csr_intreg_t
    
class cap_intr_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_intgrp_t(string name = "cap_intr_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_intgrp_t();
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
    
        cap_intr_csr_intreg_t intreg;
    
        cap_intr_csr_intreg_t int_test_set;
    
        cap_intr_csr_int_intr_ecc_int_enable_clear_t int_enable_set;
    
        cap_intr_csr_int_intr_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_intr_csr_intgrp_t
    
class cap_intr_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_intreg_status_t(string name = "cap_intr_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_intr_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_intr_ecc_interrupt;
        void int_intr_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_intr_ecc_interrupt() const;
    
}; // cap_intr_csr_intreg_status_t
    
class cap_intr_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_int_groups_int_enable_rw_reg_t(string name = "cap_intr_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_intr_ecc_enable_cpp_int_t;
        cpp_int int_var__int_intr_ecc_enable;
        void int_intr_ecc_enable (const cpp_int  & l__val);
        cpp_int int_intr_ecc_enable() const;
    
}; // cap_intr_csr_int_groups_int_enable_rw_reg_t
    
class cap_intr_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_intgrp_status_t(string name = "cap_intr_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_intgrp_status_t();
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
    
        cap_intr_csr_intreg_status_t intreg;
    
        cap_intr_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_intr_csr_intreg_status_t int_rw_reg;
    
}; // cap_intr_csr_intgrp_status_t
    
class cap_intr_csr_dhs_intr_state_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_state_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_state_entry_t(string name = "cap_intr_csr_dhs_intr_state_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_state_entry_t();
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
    
        typedef pu_cpp_int< 50 > msixcfg_msg_addr_51_2_cpp_int_t;
        cpp_int int_var__msixcfg_msg_addr_51_2;
        void msixcfg_msg_addr_51_2 (const cpp_int  & l__val);
        cpp_int msixcfg_msg_addr_51_2() const;
    
        typedef pu_cpp_int< 32 > msixcfg_msg_data_cpp_int_t;
        cpp_int int_var__msixcfg_msg_data;
        void msixcfg_msg_data (const cpp_int  & l__val);
        cpp_int msixcfg_msg_data() const;
    
        typedef pu_cpp_int< 1 > msixcfg_vector_ctrl_cpp_int_t;
        cpp_int int_var__msixcfg_vector_ctrl;
        void msixcfg_vector_ctrl (const cpp_int  & l__val);
        cpp_int msixcfg_vector_ctrl() const;
    
        typedef pu_cpp_int< 1 > fwcfg_function_mask_cpp_int_t;
        cpp_int int_var__fwcfg_function_mask;
        void fwcfg_function_mask (const cpp_int  & l__val);
        cpp_int fwcfg_function_mask() const;
    
        typedef pu_cpp_int< 11 > fwcfg_lif_cpp_int_t;
        cpp_int int_var__fwcfg_lif;
        void fwcfg_lif (const cpp_int  & l__val);
        cpp_int fwcfg_lif() const;
    
        typedef pu_cpp_int< 1 > fwcfg_local_int_cpp_int_t;
        cpp_int int_var__fwcfg_local_int;
        void fwcfg_local_int (const cpp_int  & l__val);
        cpp_int fwcfg_local_int() const;
    
        typedef pu_cpp_int< 1 > fwcfg_legacy_int_cpp_int_t;
        cpp_int int_var__fwcfg_legacy_int;
        void fwcfg_legacy_int (const cpp_int  & l__val);
        cpp_int fwcfg_legacy_int() const;
    
        typedef pu_cpp_int< 2 > fwcfg_legacy_pin_cpp_int_t;
        cpp_int int_var__fwcfg_legacy_pin;
        void fwcfg_legacy_pin (const cpp_int  & l__val);
        cpp_int fwcfg_legacy_pin() const;
    
        typedef pu_cpp_int< 1 > drvcfg_mask_cpp_int_t;
        cpp_int int_var__drvcfg_mask;
        void drvcfg_mask (const cpp_int  & l__val);
        cpp_int drvcfg_mask() const;
    
        typedef pu_cpp_int< 16 > drvcfg_int_credits_cpp_int_t;
        cpp_int int_var__drvcfg_int_credits;
        void drvcfg_int_credits (const cpp_int  & l__val);
        cpp_int drvcfg_int_credits() const;
    
        typedef pu_cpp_int< 1 > drvcfg_mask_on_assert_cpp_int_t;
        cpp_int int_var__drvcfg_mask_on_assert;
        void drvcfg_mask_on_assert (const cpp_int  & l__val);
        cpp_int drvcfg_mask_on_assert() const;
    
        typedef pu_cpp_int< 3 > fwcfg_port_id_cpp_int_t;
        cpp_int int_var__fwcfg_port_id;
        void fwcfg_port_id (const cpp_int  & l__val);
        cpp_int fwcfg_port_id() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_intr_csr_dhs_intr_state_entry_t
    
class cap_intr_csr_dhs_intr_state_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_state_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_state_t(string name = "cap_intr_csr_dhs_intr_state_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_state_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_state_entry_t, 4096> entry;
        #else 
        cap_intr_csr_dhs_intr_state_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_intr_csr_dhs_intr_state_t
    
class cap_intr_csr_dhs_intr_coalesce_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_coalesce_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_coalesce_entry_t(string name = "cap_intr_csr_dhs_intr_coalesce_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_coalesce_entry_t();
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
    
        typedef pu_cpp_int< 6 > coal_init_value0_cpp_int_t;
        cpp_int int_var__coal_init_value0;
        void coal_init_value0 (const cpp_int  & l__val);
        cpp_int coal_init_value0() const;
    
        typedef pu_cpp_int< 6 > coal_init_value1_cpp_int_t;
        cpp_int int_var__coal_init_value1;
        void coal_init_value1 (const cpp_int  & l__val);
        cpp_int coal_init_value1() const;
    
        typedef pu_cpp_int< 6 > coal_init_value2_cpp_int_t;
        cpp_int int_var__coal_init_value2;
        void coal_init_value2 (const cpp_int  & l__val);
        cpp_int coal_init_value2() const;
    
        typedef pu_cpp_int< 6 > coal_init_value3_cpp_int_t;
        cpp_int int_var__coal_init_value3;
        void coal_init_value3 (const cpp_int  & l__val);
        cpp_int coal_init_value3() const;
    
        typedef pu_cpp_int< 6 > coal_curr_value0_cpp_int_t;
        cpp_int int_var__coal_curr_value0;
        void coal_curr_value0 (const cpp_int  & l__val);
        cpp_int coal_curr_value0() const;
    
        typedef pu_cpp_int< 6 > coal_curr_value1_cpp_int_t;
        cpp_int int_var__coal_curr_value1;
        void coal_curr_value1 (const cpp_int  & l__val);
        cpp_int coal_curr_value1() const;
    
        typedef pu_cpp_int< 6 > coal_curr_value2_cpp_int_t;
        cpp_int int_var__coal_curr_value2;
        void coal_curr_value2 (const cpp_int  & l__val);
        cpp_int coal_curr_value2() const;
    
        typedef pu_cpp_int< 6 > coal_curr_value3_cpp_int_t;
        cpp_int int_var__coal_curr_value3;
        void coal_curr_value3 (const cpp_int  & l__val);
        cpp_int coal_curr_value3() const;
    
        typedef pu_cpp_int< 4 > hw_use_rsvd_cpp_int_t;
        cpp_int int_var__hw_use_rsvd;
        void hw_use_rsvd (const cpp_int  & l__val);
        cpp_int hw_use_rsvd() const;
    
        typedef pu_cpp_int< 1 > hw_legacy_sent_state0_cpp_int_t;
        cpp_int int_var__hw_legacy_sent_state0;
        void hw_legacy_sent_state0 (const cpp_int  & l__val);
        cpp_int hw_legacy_sent_state0() const;
    
        typedef pu_cpp_int< 1 > hw_legacy_sent_state1_cpp_int_t;
        cpp_int int_var__hw_legacy_sent_state1;
        void hw_legacy_sent_state1 (const cpp_int  & l__val);
        cpp_int hw_legacy_sent_state1() const;
    
        typedef pu_cpp_int< 1 > hw_legacy_sent_state2_cpp_int_t;
        cpp_int int_var__hw_legacy_sent_state2;
        void hw_legacy_sent_state2 (const cpp_int  & l__val);
        cpp_int hw_legacy_sent_state2() const;
    
        typedef pu_cpp_int< 1 > hw_legacy_sent_state3_cpp_int_t;
        cpp_int int_var__hw_legacy_sent_state3;
        void hw_legacy_sent_state3 (const cpp_int  & l__val);
        cpp_int hw_legacy_sent_state3() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_intr_csr_dhs_intr_coalesce_entry_t
    
class cap_intr_csr_dhs_intr_coalesce_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_coalesce_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_coalesce_t(string name = "cap_intr_csr_dhs_intr_coalesce_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_coalesce_t();
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
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_coalesce_entry_t, 1024> entry;
        #else 
        cap_intr_csr_dhs_intr_coalesce_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_intr_csr_dhs_intr_coalesce_t
    
class cap_intr_csr_dhs_intr_assert_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_assert_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_assert_entry_t(string name = "cap_intr_csr_dhs_intr_assert_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_assert_entry_t();
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
    
        typedef pu_cpp_int< 1 > rst_coalesce_cpp_int_t;
        cpp_int int_var__rst_coalesce;
        void rst_coalesce (const cpp_int  & l__val);
        cpp_int rst_coalesce() const;
    
}; // cap_intr_csr_dhs_intr_assert_entry_t
    
class cap_intr_csr_dhs_intr_assert_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_assert_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_assert_t(string name = "cap_intr_csr_dhs_intr_assert_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_assert_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_assert_entry_t, 4096> entry;
        #else 
        cap_intr_csr_dhs_intr_assert_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_intr_csr_dhs_intr_assert_t
    
class cap_intr_csr_dhs_intr_pba_array_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_pba_array_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_pba_array_entry_t(string name = "cap_intr_csr_dhs_intr_pba_array_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_pba_array_entry_t();
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
    
        typedef pu_cpp_int< 64 > vec_cpp_int_t;
        cpp_int int_var__vec;
        void vec (const cpp_int  & l__val);
        cpp_int vec() const;
    
}; // cap_intr_csr_dhs_intr_pba_array_entry_t
    
class cap_intr_csr_dhs_intr_pba_array_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_pba_array_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_pba_array_t(string name = "cap_intr_csr_dhs_intr_pba_array_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_pba_array_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_pba_array_entry_t, 2048> entry;
        #else 
        cap_intr_csr_dhs_intr_pba_array_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_intr_csr_dhs_intr_pba_array_t
    
class cap_intr_csr_dhs_intr_pba_cfg_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_pba_cfg_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_pba_cfg_entry_t(string name = "cap_intr_csr_dhs_intr_pba_cfg_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_pba_cfg_entry_t();
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
    
        typedef pu_cpp_int< 12 > start_cpp_int_t;
        cpp_int int_var__start;
        void start (const cpp_int  & l__val);
        cpp_int start() const;
    
        typedef pu_cpp_int< 6 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
        typedef pu_cpp_int< 6 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_intr_csr_dhs_intr_pba_cfg_entry_t
    
class cap_intr_csr_dhs_intr_pba_cfg_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_pba_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_pba_cfg_t(string name = "cap_intr_csr_dhs_intr_pba_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_pba_cfg_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_pba_cfg_entry_t, 2048> entry;
        #else 
        cap_intr_csr_dhs_intr_pba_cfg_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_intr_csr_dhs_intr_pba_cfg_t
    
class cap_intr_csr_dhs_intr_drvcfg_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_drvcfg_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_drvcfg_entry_t(string name = "cap_intr_csr_dhs_intr_drvcfg_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_drvcfg_entry_t();
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
    
        typedef pu_cpp_int< 6 > coal_init_cpp_int_t;
        cpp_int int_var__coal_init;
        void coal_init (const cpp_int  & l__val);
        cpp_int coal_init() const;
    
        typedef pu_cpp_int< 26 > reserved1_cpp_int_t;
        cpp_int int_var__reserved1;
        void reserved1 (const cpp_int  & l__val);
        cpp_int reserved1() const;
    
        typedef pu_cpp_int< 1 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
        typedef pu_cpp_int< 31 > reserved2_cpp_int_t;
        cpp_int int_var__reserved2;
        void reserved2 (const cpp_int  & l__val);
        cpp_int reserved2() const;
    
        typedef pu_cpp_int< 16 > int_credits_cpp_int_t;
        cpp_int int_var__int_credits;
        void int_credits (const cpp_int  & l__val);
        cpp_int int_credits() const;
    
        typedef pu_cpp_int< 1 > unmask_cpp_int_t;
        cpp_int int_var__unmask;
        void unmask (const cpp_int  & l__val);
        cpp_int unmask() const;
    
        typedef pu_cpp_int< 1 > restart_coal_cpp_int_t;
        cpp_int int_var__restart_coal;
        void restart_coal (const cpp_int  & l__val);
        cpp_int restart_coal() const;
    
        typedef pu_cpp_int< 14 > reserved3_cpp_int_t;
        cpp_int int_var__reserved3;
        void reserved3 (const cpp_int  & l__val);
        cpp_int reserved3() const;
    
        typedef pu_cpp_int< 1 > mask_on_assert_cpp_int_t;
        cpp_int int_var__mask_on_assert;
        void mask_on_assert (const cpp_int  & l__val);
        cpp_int mask_on_assert() const;
    
        typedef pu_cpp_int< 31 > reserved4_cpp_int_t;
        cpp_int int_var__reserved4;
        void reserved4 (const cpp_int  & l__val);
        cpp_int reserved4() const;
    
        typedef pu_cpp_int< 6 > coal_curr_cpp_int_t;
        cpp_int int_var__coal_curr;
        void coal_curr (const cpp_int  & l__val);
        cpp_int coal_curr() const;
    
        typedef pu_cpp_int< 26 > reserved5_cpp_int_t;
        cpp_int int_var__reserved5;
        void reserved5 (const cpp_int  & l__val);
        cpp_int reserved5() const;
    
}; // cap_intr_csr_dhs_intr_drvcfg_entry_t
    
class cap_intr_csr_dhs_intr_drvcfg_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_drvcfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_drvcfg_t(string name = "cap_intr_csr_dhs_intr_drvcfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_drvcfg_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_drvcfg_entry_t, 4096> entry;
        #else 
        cap_intr_csr_dhs_intr_drvcfg_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_intr_csr_dhs_intr_drvcfg_t
    
class cap_intr_csr_dhs_intr_fwcfg_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_fwcfg_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_fwcfg_entry_t(string name = "cap_intr_csr_dhs_intr_fwcfg_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_fwcfg_entry_t();
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
    
        typedef pu_cpp_int< 1 > function_mask_cpp_int_t;
        cpp_int int_var__function_mask;
        void function_mask (const cpp_int  & l__val);
        cpp_int function_mask() const;
    
        typedef pu_cpp_int< 31 > reserved1_cpp_int_t;
        cpp_int int_var__reserved1;
        void reserved1 (const cpp_int  & l__val);
        cpp_int reserved1() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        cpp_int int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 1 > local_int_cpp_int_t;
        cpp_int int_var__local_int;
        void local_int (const cpp_int  & l__val);
        cpp_int local_int() const;
    
        typedef pu_cpp_int< 1 > legacy_cpp_int_t;
        cpp_int int_var__legacy;
        void legacy (const cpp_int  & l__val);
        cpp_int legacy() const;
    
        typedef pu_cpp_int< 2 > intpin_cpp_int_t;
        cpp_int int_var__intpin;
        void intpin (const cpp_int  & l__val);
        cpp_int intpin() const;
    
        typedef pu_cpp_int< 14 > reserved2_cpp_int_t;
        cpp_int int_var__reserved2;
        void reserved2 (const cpp_int  & l__val);
        cpp_int reserved2() const;
    
}; // cap_intr_csr_dhs_intr_fwcfg_entry_t
    
class cap_intr_csr_dhs_intr_fwcfg_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_fwcfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_fwcfg_t(string name = "cap_intr_csr_dhs_intr_fwcfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_fwcfg_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_fwcfg_entry_t, 4096> entry;
        #else 
        cap_intr_csr_dhs_intr_fwcfg_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_intr_csr_dhs_intr_fwcfg_t
    
class cap_intr_csr_dhs_intr_msixcfg_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_msixcfg_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_msixcfg_entry_t(string name = "cap_intr_csr_dhs_intr_msixcfg_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_msixcfg_entry_t();
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
    
        typedef pu_cpp_int< 64 > msg_addr_cpp_int_t;
        cpp_int int_var__msg_addr;
        void msg_addr (const cpp_int  & l__val);
        cpp_int msg_addr() const;
    
        typedef pu_cpp_int< 32 > msg_data_cpp_int_t;
        cpp_int int_var__msg_data;
        void msg_data (const cpp_int  & l__val);
        cpp_int msg_data() const;
    
        typedef pu_cpp_int< 1 > vector_ctrl_cpp_int_t;
        cpp_int int_var__vector_ctrl;
        void vector_ctrl (const cpp_int  & l__val);
        cpp_int vector_ctrl() const;
    
        typedef pu_cpp_int< 31 > reserved1_cpp_int_t;
        cpp_int int_var__reserved1;
        void reserved1 (const cpp_int  & l__val);
        cpp_int reserved1() const;
    
}; // cap_intr_csr_dhs_intr_msixcfg_entry_t
    
class cap_intr_csr_dhs_intr_msixcfg_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_dhs_intr_msixcfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_dhs_intr_msixcfg_t(string name = "cap_intr_csr_dhs_intr_msixcfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_dhs_intr_msixcfg_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_intr_csr_dhs_intr_msixcfg_entry_t, 4096> entry;
        #else 
        cap_intr_csr_dhs_intr_msixcfg_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_intr_csr_dhs_intr_msixcfg_t
    
class cap_intr_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_csr_intr_t(string name = "cap_intr_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_csr_intr_t();
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
    
}; // cap_intr_csr_csr_intr_t
    
class cap_intr_csr_sta_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_sram_bist_t(string name = "cap_intr_csr_sta_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_sram_bist_t();
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
    
        typedef pu_cpp_int< 3 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
        typedef pu_cpp_int< 3 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
}; // cap_intr_csr_sta_sram_bist_t
    
class cap_intr_csr_cfg_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_sram_bist_t(string name = "cap_intr_csr_cfg_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_sram_bist_t();
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
    
        typedef pu_cpp_int< 3 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_intr_csr_cfg_sram_bist_t
    
class cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t(string name = "cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t();
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
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 16 > debug_readout_cpp_int_t;
        cpp_int int_var__debug_readout;
        void debug_readout (const cpp_int  & l__val);
        cpp_int debug_readout() const;
    
}; // cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t
    
class cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t(string name = "cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t();
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
    
        typedef pu_cpp_int< 32 > dw1_cpp_int_t;
        cpp_int int_var__dw1;
        void dw1 (const cpp_int  & l__val);
        cpp_int dw1() const;
    
        typedef pu_cpp_int< 32 > dw2_cpp_int_t;
        cpp_int int_var__dw2;
        void dw2 (const cpp_int  & l__val);
        cpp_int dw2() const;
    
        typedef pu_cpp_int< 32 > dw3_cpp_int_t;
        cpp_int int_var__dw3;
        void dw3 (const cpp_int  & l__val);
        cpp_int dw3() const;
    
}; // cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t
    
class cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t(string name = "cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t();
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
    
        typedef pu_cpp_int< 1 > source_cpp_int_t;
        cpp_int int_var__source;
        void source (const cpp_int  & l__val);
        cpp_int source() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t
    
class cap_intr_csr_cfg_debug_max_credits_limit_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_debug_max_credits_limit_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_debug_max_credits_limit_t(string name = "cap_intr_csr_cfg_debug_max_credits_limit_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_debug_max_credits_limit_t();
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
    
        typedef pu_cpp_int< 16 > int_threshold_cpp_int_t;
        cpp_int int_var__int_threshold;
        void int_threshold (const cpp_int  & l__val);
        cpp_int int_threshold() const;
    
}; // cap_intr_csr_cfg_debug_max_credits_limit_t
    
class cap_intr_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_debug_port_t(string name = "cap_intr_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_debug_port_t();
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
    
        typedef pu_cpp_int< 2 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_intr_csr_cfg_debug_port_t
    
class cap_intr_csr_cfg_intr_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_intr_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_intr_axi_attr_t(string name = "cap_intr_csr_cfg_intr_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_intr_axi_attr_t();
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
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        cpp_int int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_intr_csr_cfg_intr_axi_attr_t
    
class cap_intr_csr_sat_intr_event_counters_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sat_intr_event_counters_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sat_intr_event_counters_t(string name = "cap_intr_csr_sat_intr_event_counters_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sat_intr_event_counters_t();
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
    
        typedef pu_cpp_int< 8 > axi_bresp_errs_cpp_int_t;
        cpp_int int_var__axi_bresp_errs;
        void axi_bresp_errs (const cpp_int  & l__val);
        cpp_int axi_bresp_errs() const;
    
        typedef pu_cpp_int< 8 > delayed_coal_scan_cpp_int_t;
        cpp_int int_var__delayed_coal_scan;
        void delayed_coal_scan (const cpp_int  & l__val);
        cpp_int delayed_coal_scan() const;
    
}; // cap_intr_csr_sat_intr_event_counters_t
    
class cap_intr_csr_cnt_intr_legacy_send_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cnt_intr_legacy_send_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cnt_intr_legacy_send_t(string name = "cap_intr_csr_cnt_intr_legacy_send_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cnt_intr_legacy_send_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_intr_csr_cnt_intr_legacy_send_t
    
class cap_intr_csr_cnt_intr_tot_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cnt_intr_tot_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cnt_intr_tot_axi_wr_t(string name = "cap_intr_csr_cnt_intr_tot_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cnt_intr_tot_axi_wr_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_intr_csr_cnt_intr_tot_axi_wr_t
    
class cap_intr_csr_sta_intr_ecc_intr_coalesce_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_intr_ecc_intr_coalesce_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_intr_ecc_intr_coalesce_t(string name = "cap_intr_csr_sta_intr_ecc_intr_coalesce_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_intr_ecc_intr_coalesce_t();
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
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_intr_csr_sta_intr_ecc_intr_coalesce_t
    
class cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t(string name = "cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t();
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
    
        typedef pu_cpp_int< 6 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t
    
class cap_intr_csr_sta_intr_ecc_intr_state_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_intr_ecc_intr_state_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_intr_ecc_intr_state_t(string name = "cap_intr_csr_sta_intr_ecc_intr_state_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_intr_ecc_intr_state_t();
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
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_intr_csr_sta_intr_ecc_intr_state_t
    
class cap_intr_csr_cfg_intr_ecc_disable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_intr_ecc_disable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_intr_ecc_disable_t(string name = "cap_intr_csr_cfg_intr_ecc_disable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_intr_ecc_disable_t();
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
    
        typedef pu_cpp_int< 1 > intr_state_cor_cpp_int_t;
        cpp_int int_var__intr_state_cor;
        void intr_state_cor (const cpp_int  & l__val);
        cpp_int intr_state_cor() const;
    
        typedef pu_cpp_int< 1 > intr_state_det_cpp_int_t;
        cpp_int int_var__intr_state_det;
        void intr_state_det (const cpp_int  & l__val);
        cpp_int intr_state_det() const;
    
        typedef pu_cpp_int< 1 > intr_state_dhs_cpp_int_t;
        cpp_int int_var__intr_state_dhs;
        void intr_state_dhs (const cpp_int  & l__val);
        cpp_int intr_state_dhs() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_cor_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_cor;
        void intr_pba_cfg_cor (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_cor() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_det_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_det;
        void intr_pba_cfg_det (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_det() const;
    
        typedef pu_cpp_int< 1 > intr_pba_cfg_dhs_cpp_int_t;
        cpp_int int_var__intr_pba_cfg_dhs;
        void intr_pba_cfg_dhs (const cpp_int  & l__val);
        cpp_int intr_pba_cfg_dhs() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_cor_cpp_int_t;
        cpp_int int_var__intr_coalesce_cor;
        void intr_coalesce_cor (const cpp_int  & l__val);
        cpp_int intr_coalesce_cor() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_det_cpp_int_t;
        cpp_int int_var__intr_coalesce_det;
        void intr_coalesce_det (const cpp_int  & l__val);
        cpp_int intr_coalesce_det() const;
    
        typedef pu_cpp_int< 1 > intr_coalesce_dhs_cpp_int_t;
        cpp_int int_var__intr_coalesce_dhs;
        void intr_coalesce_dhs (const cpp_int  & l__val);
        cpp_int intr_coalesce_dhs() const;
    
}; // cap_intr_csr_cfg_intr_ecc_disable_t
    
class cap_intr_csr_sta_intr_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_intr_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_intr_debug_t(string name = "cap_intr_csr_sta_intr_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_intr_debug_t();
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
    
        typedef pu_cpp_int< 7 > legacy_fifodepth_cpp_int_t;
        cpp_int int_var__legacy_fifodepth;
        void legacy_fifodepth (const cpp_int  & l__val);
        cpp_int legacy_fifodepth() const;
    
        typedef pu_cpp_int< 4 > coalexp_fifodepth_cpp_int_t;
        cpp_int int_var__coalexp_fifodepth;
        void coalexp_fifodepth (const cpp_int  & l__val);
        cpp_int coalexp_fifodepth() const;
    
        typedef pu_cpp_int< 3 > wfifo_depth_cpp_int_t;
        cpp_int int_var__wfifo_depth;
        void wfifo_depth (const cpp_int  & l__val);
        cpp_int wfifo_depth() const;
    
        typedef pu_cpp_int< 3 > awfifo_depth_cpp_int_t;
        cpp_int int_var__awfifo_depth;
        void awfifo_depth (const cpp_int  & l__val);
        cpp_int awfifo_depth() const;
    
        typedef pu_cpp_int< 1 > wready_cpp_int_t;
        cpp_int int_var__wready;
        void wready (const cpp_int  & l__val);
        cpp_int wready() const;
    
        typedef pu_cpp_int< 1 > wvalid_cpp_int_t;
        cpp_int int_var__wvalid;
        void wvalid (const cpp_int  & l__val);
        cpp_int wvalid() const;
    
        typedef pu_cpp_int< 1 > awready_cpp_int_t;
        cpp_int int_var__awready;
        void awready (const cpp_int  & l__val);
        cpp_int awready() const;
    
        typedef pu_cpp_int< 1 > awvalid_cpp_int_t;
        cpp_int int_var__awvalid;
        void awvalid (const cpp_int  & l__val);
        cpp_int awvalid() const;
    
        typedef pu_cpp_int< 1 > axi_rdy_cpp_int_t;
        cpp_int int_var__axi_rdy;
        void axi_rdy (const cpp_int  & l__val);
        cpp_int axi_rdy() const;
    
        typedef pu_cpp_int< 1 > coal_scan_active_cpp_int_t;
        cpp_int int_var__coal_scan_active;
        void coal_scan_active (const cpp_int  & l__val);
        cpp_int coal_scan_active() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_intr_csr_sta_intr_debug_t
    
class cap_intr_csr_sta_intr_init_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_intr_init_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_intr_init_t(string name = "cap_intr_csr_sta_intr_init_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_intr_init_t();
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
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
}; // cap_intr_csr_sta_intr_init_t
    
class cap_intr_csr_sta_intr_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_sta_intr_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_sta_intr_axi_t(string name = "cap_intr_csr_sta_intr_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_sta_intr_axi_t();
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
    
        typedef pu_cpp_int< 8 > num_ids_cpp_int_t;
        cpp_int int_var__num_ids;
        void num_ids (const cpp_int  & l__val);
        cpp_int num_ids() const;
    
}; // cap_intr_csr_sta_intr_axi_t
    
class cap_intr_csr_cfg_intr_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_intr_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_intr_axi_t(string name = "cap_intr_csr_cfg_intr_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_intr_axi_t();
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
    
        typedef pu_cpp_int< 8 > num_ids_cpp_int_t;
        cpp_int int_var__num_ids;
        void num_ids (const cpp_int  & l__val);
        cpp_int num_ids() const;
    
}; // cap_intr_csr_cfg_intr_axi_t
    
class cap_intr_csr_cfg_intr_coalesce_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_cfg_intr_coalesce_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_cfg_intr_coalesce_t(string name = "cap_intr_csr_cfg_intr_coalesce_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_cfg_intr_coalesce_t();
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
    
        typedef pu_cpp_int< 16 > resolution_cpp_int_t;
        cpp_int int_var__resolution;
        void resolution (const cpp_int  & l__val);
        cpp_int resolution() const;
    
}; // cap_intr_csr_cfg_intr_coalesce_t
    
class cap_intr_csr_rdintr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_rdintr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_rdintr_t(string name = "cap_intr_csr_rdintr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_rdintr_t();
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
    
        typedef pu_cpp_int< 32 > ireg_cpp_int_t;
        cpp_int int_var__ireg;
        void ireg (const cpp_int  & l__val);
        cpp_int ireg() const;
    
}; // cap_intr_csr_rdintr_t
    
class cap_intr_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_base_t(string name = "cap_intr_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_base_t();
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
    
}; // cap_intr_csr_base_t
    
class cap_intr_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_intr_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_intr_csr_t(string name = "cap_intr_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_intr_csr_t();
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
    
        cap_intr_csr_base_t base;
    
        cap_intr_csr_rdintr_t rdintr;
    
        cap_intr_csr_cfg_intr_coalesce_t cfg_intr_coalesce;
    
        cap_intr_csr_cfg_intr_axi_t cfg_intr_axi;
    
        cap_intr_csr_sta_intr_axi_t sta_intr_axi;
    
        cap_intr_csr_sta_intr_init_t sta_intr_init;
    
        cap_intr_csr_sta_intr_debug_t sta_intr_debug;
    
        cap_intr_csr_cfg_intr_ecc_disable_t cfg_intr_ecc_disable;
    
        cap_intr_csr_sta_intr_ecc_intr_state_t sta_intr_ecc_intr_state;
    
        cap_intr_csr_sta_intr_ecc_intr_pba_cfg_t sta_intr_ecc_intr_pba_cfg;
    
        cap_intr_csr_sta_intr_ecc_intr_coalesce_t sta_intr_ecc_intr_coalesce;
    
        cap_intr_csr_cnt_intr_tot_axi_wr_t cnt_intr_tot_axi_wr;
    
        cap_intr_csr_cnt_intr_legacy_send_t cnt_intr_legacy_send;
    
        cap_intr_csr_sat_intr_event_counters_t sat_intr_event_counters;
    
        cap_intr_csr_cfg_intr_axi_attr_t cfg_intr_axi_attr;
    
        cap_intr_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_intr_csr_cfg_debug_max_credits_limit_t cfg_debug_max_credits_limit;
    
        cap_intr_csr_cfg_todo_bits_legacy_cnt_read_debug_t cfg_todo_bits_legacy_cnt_read_debug;
    
        cap_intr_csr_cfg_legacy_intx_pcie_msg_hdr_t cfg_legacy_intx_pcie_msg_hdr;
    
        cap_intr_csr_sta_todo_bits_legacy_cnt_read_debug_t sta_todo_bits_legacy_cnt_read_debug;
    
        cap_intr_csr_cfg_sram_bist_t cfg_sram_bist;
    
        cap_intr_csr_sta_sram_bist_t sta_sram_bist;
    
        cap_intr_csr_csr_intr_t csr_intr;
    
        cap_intr_csr_dhs_intr_msixcfg_t dhs_intr_msixcfg;
    
        cap_intr_csr_dhs_intr_fwcfg_t dhs_intr_fwcfg;
    
        cap_intr_csr_dhs_intr_drvcfg_t dhs_intr_drvcfg;
    
        cap_intr_csr_dhs_intr_pba_cfg_t dhs_intr_pba_cfg;
    
        cap_intr_csr_dhs_intr_pba_array_t dhs_intr_pba_array;
    
        cap_intr_csr_dhs_intr_assert_t dhs_intr_assert;
    
        cap_intr_csr_dhs_intr_coalesce_t dhs_intr_coalesce;
    
        cap_intr_csr_dhs_intr_state_t dhs_intr_state;
    
        cap_intr_csr_intgrp_status_t int_groups;
    
        cap_intr_csr_intgrp_t int_intr_ecc;
    
}; // cap_intr_csr_t
    
#endif // CAP_INTR_CSR_H
        