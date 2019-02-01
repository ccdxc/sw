
#ifndef CAP_SEMA_CSR_H
#define CAP_SEMA_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_sema_csr_sema_err_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_sema_err_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_sema_err_int_enable_clear_t(string name = "cap_sema_csr_sema_err_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_sema_err_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > write_err_enable_cpp_int_t;
        cpp_int int_var__write_err_enable;
        void write_err_enable (const cpp_int  & l__val);
        cpp_int write_err_enable() const;
    
        typedef pu_cpp_int< 1 > read_err_enable_cpp_int_t;
        cpp_int int_var__read_err_enable;
        void read_err_enable (const cpp_int  & l__val);
        cpp_int read_err_enable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_enable;
        void uncorrectable_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > correctable_enable_cpp_int_t;
        cpp_int int_var__correctable_enable;
        void correctable_enable (const cpp_int  & l__val);
        cpp_int correctable_enable() const;
    
}; // cap_sema_csr_sema_err_int_enable_clear_t
    
class cap_sema_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_intreg_t(string name = "cap_sema_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > write_err_interrupt_cpp_int_t;
        cpp_int int_var__write_err_interrupt;
        void write_err_interrupt (const cpp_int  & l__val);
        cpp_int write_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > read_err_interrupt_cpp_int_t;
        cpp_int int_var__read_err_interrupt;
        void read_err_interrupt (const cpp_int  & l__val);
        cpp_int read_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_interrupt;
        void uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > correctable_interrupt_cpp_int_t;
        cpp_int int_var__correctable_interrupt;
        void correctable_interrupt (const cpp_int  & l__val);
        cpp_int correctable_interrupt() const;
    
}; // cap_sema_csr_intreg_t
    
class cap_sema_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_intgrp_t(string name = "cap_sema_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_intgrp_t();
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
    
        cap_sema_csr_intreg_t intreg;
    
        cap_sema_csr_intreg_t int_test_set;
    
        cap_sema_csr_sema_err_int_enable_clear_t int_enable_set;
    
        cap_sema_csr_sema_err_int_enable_clear_t int_enable_clear;
    
}; // cap_sema_csr_intgrp_t
    
class cap_sema_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_intreg_status_t(string name = "cap_sema_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > sema_err_interrupt_cpp_int_t;
        cpp_int int_var__sema_err_interrupt;
        void sema_err_interrupt (const cpp_int  & l__val);
        cpp_int sema_err_interrupt() const;
    
}; // cap_sema_csr_intreg_status_t
    
class cap_sema_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_int_groups_int_enable_rw_reg_t(string name = "cap_sema_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > sema_err_enable_cpp_int_t;
        cpp_int int_var__sema_err_enable;
        void sema_err_enable (const cpp_int  & l__val);
        cpp_int sema_err_enable() const;
    
}; // cap_sema_csr_int_groups_int_enable_rw_reg_t
    
class cap_sema_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_intgrp_status_t(string name = "cap_sema_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_intgrp_status_t();
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
    
        cap_sema_csr_intreg_status_t intreg;
    
        cap_sema_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_sema_csr_intreg_status_t int_rw_reg;
    
}; // cap_sema_csr_intgrp_status_t
    
class cap_sema_csr_atomic_add_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_atomic_add_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_atomic_add_entry_t(string name = "cap_sema_csr_atomic_add_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_atomic_add_entry_t();
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
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_sema_csr_atomic_add_entry_t
    
class cap_sema_csr_atomic_add_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_atomic_add_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_atomic_add_t(string name = "cap_sema_csr_atomic_add_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_atomic_add_t();
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
    
        #if 16777216 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_sema_csr_atomic_add_entry_t, 16777216> entry;
        #else 
        cap_sema_csr_atomic_add_entry_t entry[16777216];
        #endif
        int get_depth_entry() { return 16777216; }
    
}; // cap_sema_csr_atomic_add_t
    
class cap_sema_csr_semaphore_inc_not_full_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_inc_not_full_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_inc_not_full_entry_t(string name = "cap_sema_csr_semaphore_inc_not_full_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_inc_not_full_entry_t();
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
    
        typedef pu_cpp_int< 1 > full_cpp_int_t;
        cpp_int int_var__full;
        void full (const cpp_int  & l__val);
        cpp_int full() const;
    
}; // cap_sema_csr_semaphore_inc_not_full_entry_t
    
class cap_sema_csr_semaphore_inc_not_full_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_inc_not_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_inc_not_full_t(string name = "cap_sema_csr_semaphore_inc_not_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_inc_not_full_t();
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
        cap_csr_large_array_wrapper<cap_sema_csr_semaphore_inc_not_full_entry_t, 512> entry;
        #else 
        cap_sema_csr_semaphore_inc_not_full_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_sema_csr_semaphore_inc_not_full_t
    
class cap_sema_csr_semaphore_dec_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_dec_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_dec_entry_t(string name = "cap_sema_csr_semaphore_dec_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_dec_entry_t();
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
    
}; // cap_sema_csr_semaphore_dec_entry_t
    
class cap_sema_csr_semaphore_dec_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_dec_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_dec_t(string name = "cap_sema_csr_semaphore_dec_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_dec_t();
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
        cap_csr_large_array_wrapper<cap_sema_csr_semaphore_dec_entry_t, 1024> entry;
        #else 
        cap_sema_csr_semaphore_dec_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_sema_csr_semaphore_dec_t
    
class cap_sema_csr_semaphore_inc_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_inc_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_inc_entry_t(string name = "cap_sema_csr_semaphore_inc_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_inc_entry_t();
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
    
}; // cap_sema_csr_semaphore_inc_entry_t
    
class cap_sema_csr_semaphore_inc_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_inc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_inc_t(string name = "cap_sema_csr_semaphore_inc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_inc_t();
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
        cap_csr_large_array_wrapper<cap_sema_csr_semaphore_inc_entry_t, 1024> entry;
        #else 
        cap_sema_csr_semaphore_inc_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_sema_csr_semaphore_inc_t
    
class cap_sema_csr_semaphore_raw_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_raw_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_raw_entry_t(string name = "cap_sema_csr_semaphore_raw_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_raw_entry_t();
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
    
}; // cap_sema_csr_semaphore_raw_entry_t
    
class cap_sema_csr_semaphore_raw_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_semaphore_raw_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_semaphore_raw_t(string name = "cap_sema_csr_semaphore_raw_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_semaphore_raw_t();
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
        cap_csr_large_array_wrapper<cap_sema_csr_semaphore_raw_entry_t, 1024> entry;
        #else 
        cap_sema_csr_semaphore_raw_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_sema_csr_semaphore_raw_t
    
class cap_sema_csr_STA_sema_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_STA_sema_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_STA_sema_t(string name = "cap_sema_csr_STA_sema_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_STA_sema_t();
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
    
        typedef pu_cpp_int< 3 > atomic_state_cpp_int_t;
        cpp_int int_var__atomic_state;
        void atomic_state (const cpp_int  & l__val);
        cpp_int atomic_state() const;
    
        typedef pu_cpp_int< 1 > axi_wready_cpp_int_t;
        cpp_int int_var__axi_wready;
        void axi_wready (const cpp_int  & l__val);
        cpp_int axi_wready() const;
    
        typedef pu_cpp_int< 1 > axi_awready_cpp_int_t;
        cpp_int int_var__axi_awready;
        void axi_awready (const cpp_int  & l__val);
        cpp_int axi_awready() const;
    
        typedef pu_cpp_int< 1 > axi_rready_cpp_int_t;
        cpp_int int_var__axi_rready;
        void axi_rready (const cpp_int  & l__val);
        cpp_int axi_rready() const;
    
        typedef pu_cpp_int< 1 > cam_full_cpp_int_t;
        cpp_int int_var__cam_full;
        void cam_full (const cpp_int  & l__val);
        cpp_int cam_full() const;
    
        typedef pu_cpp_int< 1 > cam_hit_cpp_int_t;
        cpp_int int_var__cam_hit;
        void cam_hit (const cpp_int  & l__val);
        cpp_int cam_hit() const;
    
        typedef pu_cpp_int< 1 > wr_pending_hit_cpp_int_t;
        cpp_int int_var__wr_pending_hit;
        void wr_pending_hit (const cpp_int  & l__val);
        cpp_int wr_pending_hit() const;
    
        typedef pu_cpp_int< 7 > pending_count_cpp_int_t;
        cpp_int int_var__pending_count;
        void pending_count (const cpp_int  & l__val);
        cpp_int pending_count() const;
    
}; // cap_sema_csr_STA_sema_t
    
class cap_sema_csr_sta_sema_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_sta_sema_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_sta_sema_mem_t(string name = "cap_sema_csr_sta_sema_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_sta_sema_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_sema_csr_sta_sema_mem_t
    
class cap_sema_csr_cfg_sema_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_cfg_sema_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_cfg_sema_mem_t(string name = "cap_sema_csr_cfg_sema_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_cfg_sema_mem_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_sema_csr_cfg_sema_mem_t
    
class cap_sema_csr_STA_err_resp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_STA_err_resp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_STA_err_resp_t(string name = "cap_sema_csr_STA_err_resp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_STA_err_resp_t();
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
    
        typedef pu_cpp_int< 31 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 2 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_sema_csr_STA_err_resp_t
    
class cap_sema_csr_STA_pending_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_STA_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_STA_pending_t(string name = "cap_sema_csr_STA_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_STA_pending_t();
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
    
        typedef pu_cpp_int< 1 > wr_cpp_int_t;
        cpp_int int_var__wr;
        void wr (const cpp_int  & l__val);
        cpp_int wr() const;
    
        typedef pu_cpp_int< 31 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 58 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_sema_csr_STA_pending_t
    
class cap_sema_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_csr_intr_t(string name = "cap_sema_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_csr_intr_t();
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
    
}; // cap_sema_csr_csr_intr_t
    
class cap_sema_csr_sema_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_sema_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_sema_cfg_t(string name = "cap_sema_csr_sema_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_sema_cfg_t();
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
    
        typedef pu_cpp_int< 1 > allow_merge_cpp_int_t;
        cpp_int int_var__allow_merge;
        void allow_merge (const cpp_int  & l__val);
        cpp_int allow_merge() const;
    
}; // cap_sema_csr_sema_cfg_t
    
class cap_sema_csr_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_axi_attr_t(string name = "cap_sema_csr_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_axi_attr_t();
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
    
}; // cap_sema_csr_axi_attr_t
    
class cap_sema_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_base_t(string name = "cap_sema_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_base_t();
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
    
}; // cap_sema_csr_base_t
    
class cap_sema_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_sema_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_sema_csr_t(string name = "cap_sema_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_sema_csr_t();
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
    
        cap_sema_csr_base_t base;
    
        cap_sema_csr_axi_attr_t axi_attr;
    
        cap_sema_csr_sema_cfg_t sema_cfg;
    
        cap_sema_csr_csr_intr_t csr_intr;
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_sema_csr_STA_pending_t, 64> STA_pending;
        #else 
        cap_sema_csr_STA_pending_t STA_pending[64];
        #endif
        int get_depth_STA_pending() { return 64; }
    
        cap_sema_csr_STA_err_resp_t STA_err_resp;
    
        cap_sema_csr_cfg_sema_mem_t cfg_sema_mem;
    
        cap_sema_csr_sta_sema_mem_t sta_sema_mem;
    
        cap_sema_csr_STA_sema_t STA_sema;
    
        cap_sema_csr_semaphore_raw_t semaphore_raw;
    
        cap_sema_csr_semaphore_inc_t semaphore_inc;
    
        cap_sema_csr_semaphore_dec_t semaphore_dec;
    
        cap_sema_csr_semaphore_inc_not_full_t semaphore_inc_not_full;
    
        cap_sema_csr_atomic_add_t atomic_add;
    
        cap_sema_csr_intgrp_status_t int_groups;
    
        cap_sema_csr_intgrp_t sema_err;
    
}; // cap_sema_csr_t
    
#endif // CAP_SEMA_CSR_H
        