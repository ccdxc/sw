
#ifndef CAP_MC_CSR_H
#define CAP_MC_CSR_H

#include "cap_csr_base.h" 
#include "cap_mch_csr.h" 

using namespace std;
class cap_mc_csr_int_mc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_int_mc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_int_mc_int_enable_clear_t(string name = "cap_mc_csr_int_mc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_int_mc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > mch_int_enable_cpp_int_t;
        cpp_int int_var__mch_int_enable;
        void mch_int_enable (const cpp_int  & l__val);
        cpp_int mch_int_enable() const;
    
}; // cap_mc_csr_int_mc_int_enable_clear_t
    
class cap_mc_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_intreg_t(string name = "cap_mc_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > mch_int_interrupt_cpp_int_t;
        cpp_int int_var__mch_int_interrupt;
        void mch_int_interrupt (const cpp_int  & l__val);
        cpp_int mch_int_interrupt() const;
    
}; // cap_mc_csr_intreg_t
    
class cap_mc_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_intgrp_t(string name = "cap_mc_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_intgrp_t();
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
    
        cap_mc_csr_intreg_t intreg;
    
        cap_mc_csr_intreg_t int_test_set;
    
        cap_mc_csr_int_mc_int_enable_clear_t int_enable_set;
    
        cap_mc_csr_int_mc_int_enable_clear_t int_enable_clear;
    
}; // cap_mc_csr_intgrp_t
    
class cap_mc_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_intreg_status_t(string name = "cap_mc_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_intreg_status_t();
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
    
}; // cap_mc_csr_intreg_status_t
    
class cap_mc_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_int_groups_int_enable_rw_reg_t(string name = "cap_mc_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_int_groups_int_enable_rw_reg_t();
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
    
}; // cap_mc_csr_int_groups_int_enable_rw_reg_t
    
class cap_mc_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_intgrp_status_t(string name = "cap_mc_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_intgrp_status_t();
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
    
        cap_mc_csr_intreg_status_t intreg;
    
        cap_mc_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_mc_csr_intreg_status_t int_rw_reg;
    
}; // cap_mc_csr_intgrp_status_t
    
class cap_mc_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_csr_intr_t(string name = "cap_mc_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_csr_intr_t();
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
    
}; // cap_mc_csr_csr_intr_t
    
class cap_mc_csr_mc_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_mc_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_mc_cfg_t(string name = "cap_mc_csr_mc_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_mc_cfg_t();
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
    
}; // cap_mc_csr_mc_cfg_t
    
class cap_mc_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mc_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mc_csr_t(string name = "cap_mc_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mc_csr_t();
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
    
        cap_mc_csr_mc_cfg_t mc_cfg;
    
        cap_mc_csr_csr_intr_t csr_intr;
    
        cap_mch_csr_t mch;
    
        cap_mc_csr_intgrp_status_t int_groups;
    
        cap_mc_csr_intgrp_t int_mc;
    
}; // cap_mc_csr_t
    
#endif // CAP_MC_CSR_H
        