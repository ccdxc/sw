
#ifndef CAP_MCH_CSR_H
#define CAP_MCH_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mch_csr_int_mc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_int_mc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_int_mc_int_enable_clear_t(string name = "cap_mch_csr_int_mc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_int_mc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > ecc_1bit_thresh_ps1_enable_cpp_int_t;
        cpp_int int_var__ecc_1bit_thresh_ps1_enable;
        void ecc_1bit_thresh_ps1_enable (const cpp_int  & l__val);
        cpp_int ecc_1bit_thresh_ps1_enable() const;
    
        typedef pu_cpp_int< 1 > ecc_1bit_thresh_ps0_enable_cpp_int_t;
        cpp_int int_var__ecc_1bit_thresh_ps0_enable;
        void ecc_1bit_thresh_ps0_enable (const cpp_int  & l__val);
        cpp_int ecc_1bit_thresh_ps0_enable() const;
    
}; // cap_mch_csr_int_mc_int_enable_clear_t
    
class cap_mch_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_intreg_t(string name = "cap_mch_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > ecc_1bit_thresh_ps1_interrupt_cpp_int_t;
        cpp_int int_var__ecc_1bit_thresh_ps1_interrupt;
        void ecc_1bit_thresh_ps1_interrupt (const cpp_int  & l__val);
        cpp_int ecc_1bit_thresh_ps1_interrupt() const;
    
        typedef pu_cpp_int< 1 > ecc_1bit_thresh_ps0_interrupt_cpp_int_t;
        cpp_int int_var__ecc_1bit_thresh_ps0_interrupt;
        void ecc_1bit_thresh_ps0_interrupt (const cpp_int  & l__val);
        cpp_int ecc_1bit_thresh_ps0_interrupt() const;
    
}; // cap_mch_csr_intreg_t
    
class cap_mch_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_intgrp_t(string name = "cap_mch_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_intgrp_t();
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
    
        cap_mch_csr_intreg_t intreg;
    
        cap_mch_csr_intreg_t int_test_set;
    
        cap_mch_csr_int_mc_int_enable_clear_t int_enable_set;
    
        cap_mch_csr_int_mc_int_enable_clear_t int_enable_clear;
    
}; // cap_mch_csr_intgrp_t
    
class cap_mch_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_intreg_status_t(string name = "cap_mch_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_mc_interrupt_cpp_int_t;
        cpp_int int_var__int_mc_interrupt;
        void int_mc_interrupt (const cpp_int  & l__val);
        cpp_int int_mc_interrupt() const;
    
}; // cap_mch_csr_intreg_status_t
    
class cap_mch_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_int_groups_int_enable_rw_reg_t(string name = "cap_mch_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_mc_enable_cpp_int_t;
        cpp_int int_var__int_mc_enable;
        void int_mc_enable (const cpp_int  & l__val);
        cpp_int int_mc_enable() const;
    
}; // cap_mch_csr_int_groups_int_enable_rw_reg_t
    
class cap_mch_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_intgrp_status_t(string name = "cap_mch_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_intgrp_status_t();
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
    
        cap_mch_csr_intreg_status_t intreg;
    
        cap_mch_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_mch_csr_intreg_status_t int_rw_reg;
    
}; // cap_mch_csr_intgrp_status_t
    
class cap_mch_csr_dhs_apb_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_dhs_apb_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_dhs_apb_entry_t(string name = "cap_mch_csr_dhs_apb_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_dhs_apb_entry_t();
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
    
}; // cap_mch_csr_dhs_apb_entry_t
    
class cap_mch_csr_dhs_apb_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_dhs_apb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_dhs_apb_t(string name = "cap_mch_csr_dhs_apb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_dhs_apb_t();
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
    
        #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mch_csr_dhs_apb_entry_t, 65536> entry;
        #else 
        cap_mch_csr_dhs_apb_entry_t entry[65536];
        #endif
        int get_depth_entry() { return 65536; }
    
}; // cap_mch_csr_dhs_apb_t
    
class cap_mch_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_csr_intr_t(string name = "cap_mch_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_csr_intr_t();
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
    
}; // cap_mch_csr_csr_intr_t
    
class cap_mch_csr_sta_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_sta_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_sta_bist_t(string name = "cap_mch_csr_sta_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_sta_bist_t();
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
    
        typedef pu_cpp_int< 1 > done_read_data_xram_ps0_fail_cpp_int_t;
        cpp_int int_var__done_read_data_xram_ps0_fail;
        void done_read_data_xram_ps0_fail (const cpp_int  & l__val);
        cpp_int done_read_data_xram_ps0_fail() const;
    
        typedef pu_cpp_int< 1 > done_read_data_xram_ps1_fail_cpp_int_t;
        cpp_int int_var__done_read_data_xram_ps1_fail;
        void done_read_data_xram_ps1_fail (const cpp_int  & l__val);
        cpp_int done_read_data_xram_ps1_fail() const;
    
        typedef pu_cpp_int< 1 > done_read_data_xram_ps0_pass_cpp_int_t;
        cpp_int int_var__done_read_data_xram_ps0_pass;
        void done_read_data_xram_ps0_pass (const cpp_int  & l__val);
        cpp_int done_read_data_xram_ps0_pass() const;
    
        typedef pu_cpp_int< 1 > done_read_data_xram_ps1_pass_cpp_int_t;
        cpp_int int_var__done_read_data_xram_ps1_pass;
        void done_read_data_xram_ps1_pass (const cpp_int  & l__val);
        cpp_int done_read_data_xram_ps1_pass() const;
    
        typedef pu_cpp_int< 1 > done_read_reorder_details_buffer_xram_ps0_fail_cpp_int_t;
        cpp_int int_var__done_read_reorder_details_buffer_xram_ps0_fail;
        void done_read_reorder_details_buffer_xram_ps0_fail (const cpp_int  & l__val);
        cpp_int done_read_reorder_details_buffer_xram_ps0_fail() const;
    
        typedef pu_cpp_int< 1 > done_read_reorder_details_buffer_xram_ps1_fail_cpp_int_t;
        cpp_int int_var__done_read_reorder_details_buffer_xram_ps1_fail;
        void done_read_reorder_details_buffer_xram_ps1_fail (const cpp_int  & l__val);
        cpp_int done_read_reorder_details_buffer_xram_ps1_fail() const;
    
        typedef pu_cpp_int< 1 > done_read_reorder_details_buffer_xram_ps0_pass_cpp_int_t;
        cpp_int int_var__done_read_reorder_details_buffer_xram_ps0_pass;
        void done_read_reorder_details_buffer_xram_ps0_pass (const cpp_int  & l__val);
        cpp_int done_read_reorder_details_buffer_xram_ps0_pass() const;
    
        typedef pu_cpp_int< 1 > done_read_reorder_details_buffer_xram_ps1_pass_cpp_int_t;
        cpp_int int_var__done_read_reorder_details_buffer_xram_ps1_pass;
        void done_read_reorder_details_buffer_xram_ps1_pass (const cpp_int  & l__val);
        cpp_int done_read_reorder_details_buffer_xram_ps1_pass() const;
    
        typedef pu_cpp_int< 1 > done_write_data_xram_ps0_fail_cpp_int_t;
        cpp_int int_var__done_write_data_xram_ps0_fail;
        void done_write_data_xram_ps0_fail (const cpp_int  & l__val);
        cpp_int done_write_data_xram_ps0_fail() const;
    
        typedef pu_cpp_int< 1 > done_write_data_xram_ps1_fail_cpp_int_t;
        cpp_int int_var__done_write_data_xram_ps1_fail;
        void done_write_data_xram_ps1_fail (const cpp_int  & l__val);
        cpp_int done_write_data_xram_ps1_fail() const;
    
        typedef pu_cpp_int< 1 > done_write_data_xram_ps0_pass_cpp_int_t;
        cpp_int int_var__done_write_data_xram_ps0_pass;
        void done_write_data_xram_ps0_pass (const cpp_int  & l__val);
        cpp_int done_write_data_xram_ps0_pass() const;
    
        typedef pu_cpp_int< 1 > done_write_data_xram_ps1_pass_cpp_int_t;
        cpp_int int_var__done_write_data_xram_ps1_pass;
        void done_write_data_xram_ps1_pass (const cpp_int  & l__val);
        cpp_int done_write_data_xram_ps1_pass() const;
    
}; // cap_mch_csr_sta_bist_t
    
class cap_mch_csr_cfg_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_cfg_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_cfg_bist_t(string name = "cap_mch_csr_cfg_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_cfg_bist_t();
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
    
        typedef pu_cpp_int< 1 > read_data_xram_ps0_run_cpp_int_t;
        cpp_int int_var__read_data_xram_ps0_run;
        void read_data_xram_ps0_run (const cpp_int  & l__val);
        cpp_int read_data_xram_ps0_run() const;
    
        typedef pu_cpp_int< 1 > read_data_xram_ps1_run_cpp_int_t;
        cpp_int int_var__read_data_xram_ps1_run;
        void read_data_xram_ps1_run (const cpp_int  & l__val);
        cpp_int read_data_xram_ps1_run() const;
    
        typedef pu_cpp_int< 1 > read_reorder_details_buffer_xram_ps0_run_cpp_int_t;
        cpp_int int_var__read_reorder_details_buffer_xram_ps0_run;
        void read_reorder_details_buffer_xram_ps0_run (const cpp_int  & l__val);
        cpp_int read_reorder_details_buffer_xram_ps0_run() const;
    
        typedef pu_cpp_int< 1 > read_reorder_details_buffer_xram_ps1_run_cpp_int_t;
        cpp_int int_var__read_reorder_details_buffer_xram_ps1_run;
        void read_reorder_details_buffer_xram_ps1_run (const cpp_int  & l__val);
        cpp_int read_reorder_details_buffer_xram_ps1_run() const;
    
        typedef pu_cpp_int< 1 > write_data_xram_ps0_run_cpp_int_t;
        cpp_int int_var__write_data_xram_ps0_run;
        void write_data_xram_ps0_run (const cpp_int  & l__val);
        cpp_int write_data_xram_ps0_run() const;
    
        typedef pu_cpp_int< 1 > write_data_xram_ps1_run_cpp_int_t;
        cpp_int int_var__write_data_xram_ps1_run;
        void write_data_xram_ps1_run (const cpp_int  & l__val);
        cpp_int write_data_xram_ps1_run() const;
    
}; // cap_mch_csr_cfg_bist_t
    
class cap_mch_csr_mc_sta_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_mc_sta_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_mc_sta_t(string name = "cap_mch_csr_mc_sta_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_mc_sta_t();
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
    
        typedef pu_cpp_int< 4 > ecc_error_1bit_ps0_cpp_int_t;
        cpp_int int_var__ecc_error_1bit_ps0;
        void ecc_error_1bit_ps0 (const cpp_int  & l__val);
        cpp_int ecc_error_1bit_ps0() const;
    
        typedef pu_cpp_int< 4 > ecc_error_2bit_ps0_cpp_int_t;
        cpp_int int_var__ecc_error_2bit_ps0;
        void ecc_error_2bit_ps0 (const cpp_int  & l__val);
        cpp_int ecc_error_2bit_ps0() const;
    
        typedef pu_cpp_int< 28 > ecc_error_pos_ps0_cpp_int_t;
        cpp_int int_var__ecc_error_pos_ps0;
        void ecc_error_pos_ps0 (const cpp_int  & l__val);
        cpp_int ecc_error_pos_ps0() const;
    
        typedef pu_cpp_int< 4 > ecc_error_addr_ps0_cpp_int_t;
        cpp_int int_var__ecc_error_addr_ps0;
        void ecc_error_addr_ps0 (const cpp_int  & l__val);
        cpp_int ecc_error_addr_ps0() const;
    
        typedef pu_cpp_int< 1 > ecc_rmw_error_ps0_cpp_int_t;
        cpp_int int_var__ecc_rmw_error_ps0;
        void ecc_rmw_error_ps0 (const cpp_int  & l__val);
        cpp_int ecc_rmw_error_ps0() const;
    
        typedef pu_cpp_int< 4 > ecc_error_1bit_ps1_cpp_int_t;
        cpp_int int_var__ecc_error_1bit_ps1;
        void ecc_error_1bit_ps1 (const cpp_int  & l__val);
        cpp_int ecc_error_1bit_ps1() const;
    
        typedef pu_cpp_int< 4 > ecc_error_2bit_ps1_cpp_int_t;
        cpp_int int_var__ecc_error_2bit_ps1;
        void ecc_error_2bit_ps1 (const cpp_int  & l__val);
        cpp_int ecc_error_2bit_ps1() const;
    
        typedef pu_cpp_int< 28 > ecc_error_pos_ps1_cpp_int_t;
        cpp_int int_var__ecc_error_pos_ps1;
        void ecc_error_pos_ps1 (const cpp_int  & l__val);
        cpp_int ecc_error_pos_ps1() const;
    
        typedef pu_cpp_int< 4 > ecc_error_addr_ps1_cpp_int_t;
        cpp_int int_var__ecc_error_addr_ps1;
        void ecc_error_addr_ps1 (const cpp_int  & l__val);
        cpp_int ecc_error_addr_ps1() const;
    
        typedef pu_cpp_int< 1 > ecc_rmw_error_ps1_cpp_int_t;
        cpp_int int_var__ecc_rmw_error_ps1;
        void ecc_rmw_error_ps1 (const cpp_int  & l__val);
        cpp_int ecc_rmw_error_ps1() const;
    
        typedef pu_cpp_int< 32 > ecc_error_cnt_1bit_ps0_cpp_int_t;
        cpp_int int_var__ecc_error_cnt_1bit_ps0;
        void ecc_error_cnt_1bit_ps0 (const cpp_int  & l__val);
        cpp_int ecc_error_cnt_1bit_ps0() const;
    
        typedef pu_cpp_int< 32 > ecc_error_cnt_1bit_ps1_cpp_int_t;
        cpp_int int_var__ecc_error_cnt_1bit_ps1;
        void ecc_error_cnt_1bit_ps1 (const cpp_int  & l__val);
        cpp_int ecc_error_cnt_1bit_ps1() const;
    
        typedef pu_cpp_int< 32 > ecc_error_cnt_2bit_ps0_cpp_int_t;
        cpp_int int_var__ecc_error_cnt_2bit_ps0;
        void ecc_error_cnt_2bit_ps0 (const cpp_int  & l__val);
        cpp_int ecc_error_cnt_2bit_ps0() const;
    
        typedef pu_cpp_int< 32 > ecc_error_cnt_2bit_ps1_cpp_int_t;
        cpp_int int_var__ecc_error_cnt_2bit_ps1;
        void ecc_error_cnt_2bit_ps1 (const cpp_int  & l__val);
        cpp_int ecc_error_cnt_2bit_ps1() const;
    
}; // cap_mch_csr_mc_sta_t
    
class cap_mch_csr_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_cfg_t(string name = "cap_mch_csr_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_cfg_t();
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
    
        typedef pu_cpp_int< 4 > address_mode_cpp_int_t;
        cpp_int int_var__address_mode;
        void address_mode (const cpp_int  & l__val);
        cpp_int address_mode() const;
    
}; // cap_mch_csr_cfg_t
    
class cap_mch_csr_cfg_l_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_cfg_l_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_cfg_l_t(string name = "cap_mch_csr_cfg_l_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_cfg_l_t();
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
    
        typedef pu_cpp_int< 8 > self_refresh_cpp_int_t;
        cpp_int int_var__self_refresh;
        void self_refresh (const cpp_int  & l__val);
        cpp_int self_refresh() const;
    
        typedef pu_cpp_int< 8 > power_down_cpp_int_t;
        cpp_int int_var__power_down;
        void power_down (const cpp_int  & l__val);
        cpp_int power_down() const;
    
}; // cap_mch_csr_cfg_l_t
    
class cap_mch_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_base_t(string name = "cap_mch_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_base_t();
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
    
}; // cap_mch_csr_base_t
    
class cap_mch_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mch_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mch_csr_t(string name = "cap_mch_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mch_csr_t();
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
    
        cap_mch_csr_base_t base;
    
        cap_mch_csr_cfg_l_t cfg_l;
    
        cap_mch_csr_cfg_t cfg;
    
        cap_mch_csr_mc_sta_t mc_sta;
    
        cap_mch_csr_cfg_bist_t cfg_bist;
    
        cap_mch_csr_sta_bist_t sta_bist;
    
        cap_mch_csr_csr_intr_t csr_intr;
    
        cap_mch_csr_dhs_apb_t dhs_apb;
    
        cap_mch_csr_intgrp_status_t int_groups;
    
        cap_mch_csr_intgrp_t int_mc;
    
}; // cap_mch_csr_t
    
#endif // CAP_MCH_CSR_H
        