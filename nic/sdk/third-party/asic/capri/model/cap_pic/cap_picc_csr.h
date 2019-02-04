
#ifndef CAP_PICC_CSR_H
#define CAP_PICC_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_picc_csr_int_picc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_int_picc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_int_picc_int_enable_clear_t(string name = "cap_picc_csr_int_picc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_int_picc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_ecc_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_ecc_enable;
        void uncorrectable_ecc_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > correctable_ecc_enable_cpp_int_t;
        cpp_int int_var__correctable_ecc_enable;
        void correctable_ecc_enable (const cpp_int  & l__val);
        cpp_int correctable_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > cache_stg_awlen_err_enable_cpp_int_t;
        cpp_int int_var__cache_stg_awlen_err_enable;
        void cache_stg_awlen_err_enable (const cpp_int  & l__val);
        cpp_int cache_stg_awlen_err_enable() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_rresp_slverr_enable_cpp_int_t;
        cpp_int int_var__cache_hbm_rresp_slverr_enable;
        void cache_hbm_rresp_slverr_enable (const cpp_int  & l__val);
        cpp_int cache_hbm_rresp_slverr_enable() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_rresp_decerr_enable_cpp_int_t;
        cpp_int int_var__cache_hbm_rresp_decerr_enable;
        void cache_hbm_rresp_decerr_enable (const cpp_int  & l__val);
        cpp_int cache_hbm_rresp_decerr_enable() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_bresp_slverr_enable_cpp_int_t;
        cpp_int int_var__cache_hbm_bresp_slverr_enable;
        void cache_hbm_bresp_slverr_enable (const cpp_int  & l__val);
        cpp_int cache_hbm_bresp_slverr_enable() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_bresp_decerr_enable_cpp_int_t;
        cpp_int int_var__cache_hbm_bresp_decerr_enable;
        void cache_hbm_bresp_decerr_enable (const cpp_int  & l__val);
        cpp_int cache_hbm_bresp_decerr_enable() const;
    
}; // cap_picc_csr_int_picc_int_enable_clear_t
    
class cap_picc_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_intreg_t(string name = "cap_picc_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_ecc_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_ecc_interrupt;
        void uncorrectable_ecc_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > correctable_ecc_interrupt_cpp_int_t;
        cpp_int int_var__correctable_ecc_interrupt;
        void correctable_ecc_interrupt (const cpp_int  & l__val);
        cpp_int correctable_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_stg_awlen_err_interrupt_cpp_int_t;
        cpp_int int_var__cache_stg_awlen_err_interrupt;
        void cache_stg_awlen_err_interrupt (const cpp_int  & l__val);
        cpp_int cache_stg_awlen_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_rresp_slverr_interrupt_cpp_int_t;
        cpp_int int_var__cache_hbm_rresp_slverr_interrupt;
        void cache_hbm_rresp_slverr_interrupt (const cpp_int  & l__val);
        cpp_int cache_hbm_rresp_slverr_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_rresp_decerr_interrupt_cpp_int_t;
        cpp_int int_var__cache_hbm_rresp_decerr_interrupt;
        void cache_hbm_rresp_decerr_interrupt (const cpp_int  & l__val);
        cpp_int cache_hbm_rresp_decerr_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_bresp_slverr_interrupt_cpp_int_t;
        cpp_int int_var__cache_hbm_bresp_slverr_interrupt;
        void cache_hbm_bresp_slverr_interrupt (const cpp_int  & l__val);
        cpp_int cache_hbm_bresp_slverr_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_hbm_bresp_decerr_interrupt_cpp_int_t;
        cpp_int int_var__cache_hbm_bresp_decerr_interrupt;
        void cache_hbm_bresp_decerr_interrupt (const cpp_int  & l__val);
        cpp_int cache_hbm_bresp_decerr_interrupt() const;
    
}; // cap_picc_csr_intreg_t
    
class cap_picc_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_intgrp_t(string name = "cap_picc_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_intgrp_t();
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
    
        cap_picc_csr_intreg_t intreg;
    
        cap_picc_csr_intreg_t int_test_set;
    
        cap_picc_csr_int_picc_int_enable_clear_t int_enable_set;
    
        cap_picc_csr_int_picc_int_enable_clear_t int_enable_clear;
    
}; // cap_picc_csr_intgrp_t
    
class cap_picc_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_intreg_status_t(string name = "cap_picc_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_picc_interrupt_cpp_int_t;
        cpp_int int_var__int_picc_interrupt;
        void int_picc_interrupt (const cpp_int  & l__val);
        cpp_int int_picc_interrupt() const;
    
}; // cap_picc_csr_intreg_status_t
    
class cap_picc_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_int_groups_int_enable_rw_reg_t(string name = "cap_picc_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_picc_enable_cpp_int_t;
        cpp_int int_var__int_picc_enable;
        void int_picc_enable (const cpp_int  & l__val);
        cpp_int int_picc_enable() const;
    
}; // cap_picc_csr_int_groups_int_enable_rw_reg_t
    
class cap_picc_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_intgrp_status_t(string name = "cap_picc_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_intgrp_status_t();
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
    
        cap_picc_csr_intreg_status_t intreg;
    
        cap_picc_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_picc_csr_intreg_status_t int_rw_reg;
    
}; // cap_picc_csr_intgrp_status_t
    
class cap_picc_csr_dhs_cache_cnt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_cnt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_cnt_entry_t(string name = "cap_picc_csr_dhs_cache_cnt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_cnt_entry_t();
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
    
        typedef pu_cpp_int< 32 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_dhs_cache_cnt_entry_t
    
class cap_picc_csr_dhs_cache_cnt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_cnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_cnt_t(string name = "cap_picc_csr_dhs_cache_cnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_cnt_t();
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
    
        #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_cnt_entry_t, 5> entry;
        #else 
        cap_picc_csr_dhs_cache_cnt_entry_t entry[5];
        #endif
        int get_depth_entry() { return 5; }
    
}; // cap_picc_csr_dhs_cache_cnt_t
    
class cap_picc_csr_dhs_cache_data_sram3_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram3_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram3_entry_t(string name = "cap_picc_csr_dhs_cache_data_sram3_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram3_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_picc_csr_dhs_cache_data_sram3_entry_t
    
class cap_picc_csr_dhs_cache_data_sram3_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram3_t(string name = "cap_picc_csr_dhs_cache_data_sram3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram3_t();
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
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_data_sram3_entry_t, 4096> entry;
        #else 
        cap_picc_csr_dhs_cache_data_sram3_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_picc_csr_dhs_cache_data_sram3_t
    
class cap_picc_csr_dhs_cache_data_sram2_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram2_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram2_entry_t(string name = "cap_picc_csr_dhs_cache_data_sram2_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram2_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_picc_csr_dhs_cache_data_sram2_entry_t
    
class cap_picc_csr_dhs_cache_data_sram2_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram2_t(string name = "cap_picc_csr_dhs_cache_data_sram2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram2_t();
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
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_data_sram2_entry_t, 4096> entry;
        #else 
        cap_picc_csr_dhs_cache_data_sram2_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_picc_csr_dhs_cache_data_sram2_t
    
class cap_picc_csr_dhs_cache_data_sram1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram1_entry_t(string name = "cap_picc_csr_dhs_cache_data_sram1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram1_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_picc_csr_dhs_cache_data_sram1_entry_t
    
class cap_picc_csr_dhs_cache_data_sram1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram1_t(string name = "cap_picc_csr_dhs_cache_data_sram1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram1_t();
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
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_data_sram1_entry_t, 4096> entry;
        #else 
        cap_picc_csr_dhs_cache_data_sram1_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_picc_csr_dhs_cache_data_sram1_t
    
class cap_picc_csr_dhs_cache_data_sram0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram0_entry_t(string name = "cap_picc_csr_dhs_cache_data_sram0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram0_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_picc_csr_dhs_cache_data_sram0_entry_t
    
class cap_picc_csr_dhs_cache_data_sram0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_data_sram0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_data_sram0_t(string name = "cap_picc_csr_dhs_cache_data_sram0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_data_sram0_t();
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
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_data_sram0_entry_t, 4096> entry;
        #else 
        cap_picc_csr_dhs_cache_data_sram0_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_picc_csr_dhs_cache_data_sram0_t
    
class cap_picc_csr_dhs_cache_tag_sram1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_tag_sram1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_tag_sram1_entry_t(string name = "cap_picc_csr_dhs_cache_tag_sram1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_tag_sram1_entry_t();
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
    
        typedef pu_cpp_int< 108 > ecc_data_cpp_int_t;
        cpp_int int_var__ecc_data;
        void ecc_data (const cpp_int  & l__val);
        cpp_int ecc_data() const;
    
}; // cap_picc_csr_dhs_cache_tag_sram1_entry_t
    
class cap_picc_csr_dhs_cache_tag_sram1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_tag_sram1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_tag_sram1_t(string name = "cap_picc_csr_dhs_cache_tag_sram1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_tag_sram1_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_tag_sram1_entry_t, 512> entry;
        #else 
        cap_picc_csr_dhs_cache_tag_sram1_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_picc_csr_dhs_cache_tag_sram1_t
    
class cap_picc_csr_dhs_cache_tag_sram0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_tag_sram0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_tag_sram0_entry_t(string name = "cap_picc_csr_dhs_cache_tag_sram0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_tag_sram0_entry_t();
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
    
        typedef pu_cpp_int< 108 > ecc_data_cpp_int_t;
        cpp_int int_var__ecc_data;
        void ecc_data (const cpp_int  & l__val);
        cpp_int ecc_data() const;
    
}; // cap_picc_csr_dhs_cache_tag_sram0_entry_t
    
class cap_picc_csr_dhs_cache_tag_sram0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_tag_sram0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_tag_sram0_t(string name = "cap_picc_csr_dhs_cache_tag_sram0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_tag_sram0_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_dhs_cache_tag_sram0_entry_t, 512> entry;
        #else 
        cap_picc_csr_dhs_cache_tag_sram0_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_picc_csr_dhs_cache_tag_sram0_t
    
class cap_picc_csr_dhs_cache_invalidate_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_invalidate_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_invalidate_entry_t(string name = "cap_picc_csr_dhs_cache_invalidate_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_invalidate_entry_t();
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
    
        typedef pu_cpp_int< 1 > inval_all_cpp_int_t;
        cpp_int int_var__inval_all;
        void inval_all (const cpp_int  & l__val);
        cpp_int inval_all() const;
    
        typedef pu_cpp_int< 28 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_picc_csr_dhs_cache_invalidate_entry_t
    
class cap_picc_csr_dhs_cache_invalidate_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_dhs_cache_invalidate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_dhs_cache_invalidate_t(string name = "cap_picc_csr_dhs_cache_invalidate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_dhs_cache_invalidate_t();
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
    
        cap_picc_csr_dhs_cache_invalidate_entry_t entry;
    
}; // cap_picc_csr_dhs_cache_invalidate_t
    
class cap_picc_csr_filter_addr_ctl_s_value_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_ctl_s_value_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_ctl_s_value_t(string name = "cap_picc_csr_filter_addr_ctl_s_value_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_ctl_s_value_t();
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
    
        typedef pu_cpp_int< 4 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_filter_addr_ctl_s_value_t
    
class cap_picc_csr_filter_addr_ctl_s_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_ctl_s_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_ctl_s_t(string name = "cap_picc_csr_filter_addr_ctl_s_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_ctl_s_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_filter_addr_ctl_s_value_t, 8> value;
        #else 
        cap_picc_csr_filter_addr_ctl_s_value_t value[8];
        #endif
        int get_depth_value() { return 8; }
    
}; // cap_picc_csr_filter_addr_ctl_s_t
    
class cap_picc_csr_filter_addr_hi_s_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_hi_s_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_hi_s_data_t(string name = "cap_picc_csr_filter_addr_hi_s_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_hi_s_data_t();
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
    
        typedef pu_cpp_int< 28 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_filter_addr_hi_s_data_t
    
class cap_picc_csr_filter_addr_hi_s_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_hi_s_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_hi_s_t(string name = "cap_picc_csr_filter_addr_hi_s_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_hi_s_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_filter_addr_hi_s_data_t, 8> data;
        #else 
        cap_picc_csr_filter_addr_hi_s_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_picc_csr_filter_addr_hi_s_t
    
class cap_picc_csr_filter_addr_lo_s_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_lo_s_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_lo_s_data_t(string name = "cap_picc_csr_filter_addr_lo_s_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_lo_s_data_t();
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
    
        typedef pu_cpp_int< 28 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_filter_addr_lo_s_data_t
    
class cap_picc_csr_filter_addr_lo_s_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_lo_s_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_lo_s_t(string name = "cap_picc_csr_filter_addr_lo_s_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_lo_s_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_filter_addr_lo_s_data_t, 8> data;
        #else 
        cap_picc_csr_filter_addr_lo_s_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_picc_csr_filter_addr_lo_s_t
    
class cap_picc_csr_filter_addr_ctl_m_value_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_ctl_m_value_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_ctl_m_value_t(string name = "cap_picc_csr_filter_addr_ctl_m_value_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_ctl_m_value_t();
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
    
        typedef pu_cpp_int< 4 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_filter_addr_ctl_m_value_t
    
class cap_picc_csr_filter_addr_ctl_m_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_ctl_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_ctl_m_t(string name = "cap_picc_csr_filter_addr_ctl_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_ctl_m_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_filter_addr_ctl_m_value_t, 8> value;
        #else 
        cap_picc_csr_filter_addr_ctl_m_value_t value[8];
        #endif
        int get_depth_value() { return 8; }
    
}; // cap_picc_csr_filter_addr_ctl_m_t
    
class cap_picc_csr_filter_addr_hi_m_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_hi_m_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_hi_m_data_t(string name = "cap_picc_csr_filter_addr_hi_m_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_hi_m_data_t();
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
    
        typedef pu_cpp_int< 28 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_filter_addr_hi_m_data_t
    
class cap_picc_csr_filter_addr_hi_m_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_hi_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_hi_m_t(string name = "cap_picc_csr_filter_addr_hi_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_hi_m_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_filter_addr_hi_m_data_t, 8> data;
        #else 
        cap_picc_csr_filter_addr_hi_m_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_picc_csr_filter_addr_hi_m_t
    
class cap_picc_csr_filter_addr_lo_m_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_lo_m_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_lo_m_data_t(string name = "cap_picc_csr_filter_addr_lo_m_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_lo_m_data_t();
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
    
        typedef pu_cpp_int< 28 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_picc_csr_filter_addr_lo_m_data_t
    
class cap_picc_csr_filter_addr_lo_m_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_filter_addr_lo_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_filter_addr_lo_m_t(string name = "cap_picc_csr_filter_addr_lo_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_filter_addr_lo_m_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_filter_addr_lo_m_data_t, 8> data;
        #else 
        cap_picc_csr_filter_addr_lo_m_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_picc_csr_filter_addr_lo_m_t
    
class cap_picc_csr_sta_inval_cam_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_sta_inval_cam_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_sta_inval_cam_entry_t(string name = "cap_picc_csr_sta_inval_cam_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_sta_inval_cam_entry_t();
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
    
        typedef pu_cpp_int< 38 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_picc_csr_sta_inval_cam_entry_t
    
class cap_picc_csr_sta_inval_cam_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_sta_inval_cam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_sta_inval_cam_t(string name = "cap_picc_csr_sta_inval_cam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_sta_inval_cam_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_picc_csr_sta_inval_cam_entry_t, 64> entry;
        #else 
        cap_picc_csr_sta_inval_cam_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_picc_csr_sta_inval_cam_t
    
class cap_picc_csr_sta_cache_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_sta_cache_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_sta_cache_debug_t(string name = "cap_picc_csr_sta_cache_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_sta_cache_debug_t();
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
    
        typedef pu_cpp_int< 28 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > hit_cpp_int_t;
        cpp_int int_var__hit;
        void hit (const cpp_int  & l__val);
        cpp_int hit() const;
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        cpp_int int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
}; // cap_picc_csr_sta_cache_debug_t
    
class cap_picc_csr_cfg_cache_debug_range_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_debug_range_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_debug_range_t(string name = "cap_picc_csr_cfg_cache_debug_range_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_debug_range_t();
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
    
        typedef pu_cpp_int< 28 > addr_lo_cpp_int_t;
        cpp_int int_var__addr_lo;
        void addr_lo (const cpp_int  & l__val);
        cpp_int addr_lo() const;
    
        typedef pu_cpp_int< 28 > addr_hi_cpp_int_t;
        cpp_int int_var__addr_hi;
        void addr_hi (const cpp_int  & l__val);
        cpp_int addr_hi() const;
    
}; // cap_picc_csr_cfg_cache_debug_range_t
    
class cap_picc_csr_cfg_cache_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_debug_t(string name = "cap_picc_csr_cfg_cache_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_debug_t();
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
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 2 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_picc_csr_cfg_cache_debug_t
    
class cap_picc_csr_cfg_cache_cnt_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_cnt_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_cnt_ctrl_t(string name = "cap_picc_csr_cfg_cache_cnt_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_cnt_ctrl_t();
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
    
        typedef pu_cpp_int< 1 > clr_cnt_cpp_int_t;
        cpp_int int_var__clr_cnt;
        void clr_cnt (const cpp_int  & l__val);
        cpp_int clr_cnt() const;
    
        typedef pu_cpp_int< 1 > saturate_stop_cpp_int_t;
        cpp_int int_var__saturate_stop;
        void saturate_stop (const cpp_int  & l__val);
        cpp_int saturate_stop() const;
    
}; // cap_picc_csr_cfg_cache_cnt_ctrl_t
    
class cap_picc_csr_cfg_cache_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_bist_t(string name = "cap_picc_csr_cfg_cache_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_bist_t();
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
    
        typedef pu_cpp_int< 1 > tag_sram0_run_cpp_int_t;
        cpp_int int_var__tag_sram0_run;
        void tag_sram0_run (const cpp_int  & l__val);
        cpp_int tag_sram0_run() const;
    
        typedef pu_cpp_int< 1 > tag_sram1_run_cpp_int_t;
        cpp_int int_var__tag_sram1_run;
        void tag_sram1_run (const cpp_int  & l__val);
        cpp_int tag_sram1_run() const;
    
        typedef pu_cpp_int< 1 > data_sram0_run_cpp_int_t;
        cpp_int int_var__data_sram0_run;
        void data_sram0_run (const cpp_int  & l__val);
        cpp_int data_sram0_run() const;
    
        typedef pu_cpp_int< 1 > data_sram1_run_cpp_int_t;
        cpp_int int_var__data_sram1_run;
        void data_sram1_run (const cpp_int  & l__val);
        cpp_int data_sram1_run() const;
    
        typedef pu_cpp_int< 1 > data_sram2_run_cpp_int_t;
        cpp_int int_var__data_sram2_run;
        void data_sram2_run (const cpp_int  & l__val);
        cpp_int data_sram2_run() const;
    
        typedef pu_cpp_int< 1 > data_sram3_run_cpp_int_t;
        cpp_int int_var__data_sram3_run;
        void data_sram3_run (const cpp_int  & l__val);
        cpp_int data_sram3_run() const;
    
}; // cap_picc_csr_cfg_cache_bist_t
    
class cap_picc_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_csr_intr_t(string name = "cap_picc_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_csr_intr_t();
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
    
}; // cap_picc_csr_csr_intr_t
    
class cap_picc_csr_sta_cache_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_sta_cache_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_sta_cache_bist_t(string name = "cap_picc_csr_sta_cache_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_sta_cache_bist_t();
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
    
        typedef pu_cpp_int< 1 > tag_sram0_fail_cpp_int_t;
        cpp_int int_var__tag_sram0_fail;
        void tag_sram0_fail (const cpp_int  & l__val);
        cpp_int tag_sram0_fail() const;
    
        typedef pu_cpp_int< 1 > tag_sram0_pass_cpp_int_t;
        cpp_int int_var__tag_sram0_pass;
        void tag_sram0_pass (const cpp_int  & l__val);
        cpp_int tag_sram0_pass() const;
    
        typedef pu_cpp_int< 1 > tag_sram1_fail_cpp_int_t;
        cpp_int int_var__tag_sram1_fail;
        void tag_sram1_fail (const cpp_int  & l__val);
        cpp_int tag_sram1_fail() const;
    
        typedef pu_cpp_int< 1 > tag_sram1_pass_cpp_int_t;
        cpp_int int_var__tag_sram1_pass;
        void tag_sram1_pass (const cpp_int  & l__val);
        cpp_int tag_sram1_pass() const;
    
        typedef pu_cpp_int< 1 > data_sram0_fail_cpp_int_t;
        cpp_int int_var__data_sram0_fail;
        void data_sram0_fail (const cpp_int  & l__val);
        cpp_int data_sram0_fail() const;
    
        typedef pu_cpp_int< 1 > data_sram0_pass_cpp_int_t;
        cpp_int int_var__data_sram0_pass;
        void data_sram0_pass (const cpp_int  & l__val);
        cpp_int data_sram0_pass() const;
    
        typedef pu_cpp_int< 1 > data_sram1_fail_cpp_int_t;
        cpp_int int_var__data_sram1_fail;
        void data_sram1_fail (const cpp_int  & l__val);
        cpp_int data_sram1_fail() const;
    
        typedef pu_cpp_int< 1 > data_sram1_pass_cpp_int_t;
        cpp_int int_var__data_sram1_pass;
        void data_sram1_pass (const cpp_int  & l__val);
        cpp_int data_sram1_pass() const;
    
        typedef pu_cpp_int< 1 > data_sram2_fail_cpp_int_t;
        cpp_int int_var__data_sram2_fail;
        void data_sram2_fail (const cpp_int  & l__val);
        cpp_int data_sram2_fail() const;
    
        typedef pu_cpp_int< 1 > data_sram2_pass_cpp_int_t;
        cpp_int int_var__data_sram2_pass;
        void data_sram2_pass (const cpp_int  & l__val);
        cpp_int data_sram2_pass() const;
    
        typedef pu_cpp_int< 1 > data_sram3_fail_cpp_int_t;
        cpp_int int_var__data_sram3_fail;
        void data_sram3_fail (const cpp_int  & l__val);
        cpp_int data_sram3_fail() const;
    
        typedef pu_cpp_int< 1 > data_sram3_pass_cpp_int_t;
        cpp_int int_var__data_sram3_pass;
        void data_sram3_pass (const cpp_int  & l__val);
        cpp_int data_sram3_pass() const;
    
}; // cap_picc_csr_sta_cache_bist_t
    
class cap_picc_csr_sta_cache_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_sta_cache_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_sta_cache_ecc_t(string name = "cap_picc_csr_sta_cache_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_sta_cache_ecc_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 15 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_picc_csr_sta_cache_ecc_t
    
class cap_picc_csr_cfg_cache_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_ecc_t(string name = "cap_picc_csr_cfg_cache_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_ecc_t();
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
    
        typedef pu_cpp_int< 1 > disable_cor_cpp_int_t;
        cpp_int int_var__disable_cor;
        void disable_cor (const cpp_int  & l__val);
        cpp_int disable_cor() const;
    
        typedef pu_cpp_int< 1 > disable_det_cpp_int_t;
        cpp_int int_var__disable_det;
        void disable_det (const cpp_int  & l__val);
        cpp_int disable_det() const;
    
        typedef pu_cpp_int< 1 > bypass_cpp_int_t;
        cpp_int int_var__bypass;
        void bypass (const cpp_int  & l__val);
        cpp_int bypass() const;
    
}; // cap_picc_csr_cfg_cache_ecc_t
    
class cap_picc_csr_cfg_cache_fill_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_fill_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_fill_axi_t(string name = "cap_picc_csr_cfg_cache_fill_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_fill_axi_t();
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
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        cpp_int int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 1 > arlock_cpp_int_t;
        cpp_int int_var__arlock;
        void arlock (const cpp_int  & l__val);
        cpp_int arlock() const;
    
        typedef pu_cpp_int< 3 > arprot_cpp_int_t;
        cpp_int int_var__arprot;
        void arprot (const cpp_int  & l__val);
        cpp_int arprot() const;
    
        typedef pu_cpp_int< 4 > arqos_cpp_int_t;
        cpp_int int_var__arqos;
        void arqos (const cpp_int  & l__val);
        cpp_int arqos() const;
    
}; // cap_picc_csr_cfg_cache_fill_axi_t
    
class cap_picc_csr_cfg_cache_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_cache_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_cache_global_t(string name = "cap_picc_csr_cfg_cache_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_cache_global_t();
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
    
        typedef pu_cpp_int< 1 > bypass_cpp_int_t;
        cpp_int int_var__bypass;
        void bypass (const cpp_int  & l__val);
        cpp_int bypass() const;
    
        typedef pu_cpp_int< 1 > hash_mode_cpp_int_t;
        cpp_int int_var__hash_mode;
        void hash_mode (const cpp_int  & l__val);
        cpp_int hash_mode() const;
    
}; // cap_picc_csr_cfg_cache_global_t
    
class cap_picc_csr_cfg_filter_s_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_filter_s_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_filter_s_t(string name = "cap_picc_csr_cfg_filter_s_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_filter_s_t();
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
    
        typedef pu_cpp_int< 4 > awcache_mask_cpp_int_t;
        cpp_int int_var__awcache_mask;
        void awcache_mask (const cpp_int  & l__val);
        cpp_int awcache_mask() const;
    
        typedef pu_cpp_int< 4 > awcache_match_cpp_int_t;
        cpp_int int_var__awcache_match;
        void awcache_match (const cpp_int  & l__val);
        cpp_int awcache_match() const;
    
        typedef pu_cpp_int< 4 > arcache_mask_cpp_int_t;
        cpp_int int_var__arcache_mask;
        void arcache_mask (const cpp_int  & l__val);
        cpp_int arcache_mask() const;
    
        typedef pu_cpp_int< 4 > arcache_match_cpp_int_t;
        cpp_int int_var__arcache_match;
        void arcache_match (const cpp_int  & l__val);
        cpp_int arcache_match() const;
    
}; // cap_picc_csr_cfg_filter_s_t
    
class cap_picc_csr_cfg_filter_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_cfg_filter_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_cfg_filter_m_t(string name = "cap_picc_csr_cfg_filter_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_cfg_filter_m_t();
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
    
        typedef pu_cpp_int< 4 > awcache_mask_cpp_int_t;
        cpp_int int_var__awcache_mask;
        void awcache_mask (const cpp_int  & l__val);
        cpp_int awcache_mask() const;
    
        typedef pu_cpp_int< 4 > awcache_match_cpp_int_t;
        cpp_int int_var__awcache_match;
        void awcache_match (const cpp_int  & l__val);
        cpp_int awcache_match() const;
    
        typedef pu_cpp_int< 4 > arcache_mask_cpp_int_t;
        cpp_int int_var__arcache_mask;
        void arcache_mask (const cpp_int  & l__val);
        cpp_int arcache_mask() const;
    
        typedef pu_cpp_int< 4 > arcache_match_cpp_int_t;
        cpp_int int_var__arcache_match;
        void arcache_match (const cpp_int  & l__val);
        cpp_int arcache_match() const;
    
}; // cap_picc_csr_cfg_filter_m_t
    
class cap_picc_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_picc_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_picc_csr_t(string name = "cap_picc_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_picc_csr_t();
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
    
        cap_picc_csr_cfg_filter_m_t cfg_filter_m;
    
        cap_picc_csr_cfg_filter_s_t cfg_filter_s;
    
        cap_picc_csr_cfg_cache_global_t cfg_cache_global;
    
        cap_picc_csr_cfg_cache_fill_axi_t cfg_cache_fill_axi;
    
        cap_picc_csr_cfg_cache_ecc_t cfg_cache_ecc;
    
        cap_picc_csr_sta_cache_ecc_t sta_cache_ecc;
    
        cap_picc_csr_sta_cache_bist_t sta_cache_bist;
    
        cap_picc_csr_csr_intr_t csr_intr;
    
        cap_picc_csr_cfg_cache_bist_t cfg_cache_bist;
    
        cap_picc_csr_cfg_cache_cnt_ctrl_t cfg_cache_cnt_ctrl;
    
        cap_picc_csr_cfg_cache_debug_t cfg_cache_debug;
    
        cap_picc_csr_cfg_cache_debug_range_t cfg_cache_debug_range;
    
        cap_picc_csr_sta_cache_debug_t sta_cache_debug;
    
        cap_picc_csr_sta_inval_cam_t sta_inval_cam;
    
        cap_picc_csr_filter_addr_lo_m_t filter_addr_lo_m;
    
        cap_picc_csr_filter_addr_hi_m_t filter_addr_hi_m;
    
        cap_picc_csr_filter_addr_ctl_m_t filter_addr_ctl_m;
    
        cap_picc_csr_filter_addr_lo_s_t filter_addr_lo_s;
    
        cap_picc_csr_filter_addr_hi_s_t filter_addr_hi_s;
    
        cap_picc_csr_filter_addr_ctl_s_t filter_addr_ctl_s;
    
        cap_picc_csr_dhs_cache_invalidate_t dhs_cache_invalidate;
    
        cap_picc_csr_dhs_cache_tag_sram0_t dhs_cache_tag_sram0;
    
        cap_picc_csr_dhs_cache_tag_sram1_t dhs_cache_tag_sram1;
    
        cap_picc_csr_dhs_cache_data_sram0_t dhs_cache_data_sram0;
    
        cap_picc_csr_dhs_cache_data_sram1_t dhs_cache_data_sram1;
    
        cap_picc_csr_dhs_cache_data_sram2_t dhs_cache_data_sram2;
    
        cap_picc_csr_dhs_cache_data_sram3_t dhs_cache_data_sram3;
    
        cap_picc_csr_dhs_cache_cnt_t dhs_cache_cnt;
    
        cap_picc_csr_intgrp_status_t int_groups;
    
        cap_picc_csr_intgrp_t int_picc;
    
}; // cap_picc_csr_t
    
#endif // CAP_PICC_CSR_H
        