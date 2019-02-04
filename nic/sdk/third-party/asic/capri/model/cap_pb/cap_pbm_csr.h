
#ifndef CAP_PBM_CSR_H
#define CAP_PBM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbm_csr_int_ecc_col4_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_int_ecc_col4_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_int_ecc_col4_int_enable_clear_t(string name = "cap_pbm_csr_int_ecc_col4_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_int_ecc_col4_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_enable;
        void uncorrectable_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > correctable_enable_cpp_int_t;
        cpp_int int_var__correctable_enable;
        void correctable_enable (const cpp_int  & l__val);
        cpp_int correctable_enable() const;
    
}; // cap_pbm_csr_int_ecc_col4_int_enable_clear_t
    
class cap_pbm_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_intreg_t(string name = "cap_pbm_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_interrupt;
        void uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > correctable_interrupt_cpp_int_t;
        cpp_int int_var__correctable_interrupt;
        void correctable_interrupt (const cpp_int  & l__val);
        cpp_int correctable_interrupt() const;
    
}; // cap_pbm_csr_intreg_t
    
class cap_pbm_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_intgrp_t(string name = "cap_pbm_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_intgrp_t();
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
    
        cap_pbm_csr_intreg_t intreg;
    
        cap_pbm_csr_intreg_t int_test_set;
    
        cap_pbm_csr_int_ecc_col4_int_enable_clear_t int_enable_set;
    
        cap_pbm_csr_int_ecc_col4_int_enable_clear_t int_enable_clear;
    
}; // cap_pbm_csr_intgrp_t
    
class cap_pbm_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_intreg_status_t(string name = "cap_pbm_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_ecc_col0_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_col0_interrupt;
        void int_ecc_col0_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_col0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col1_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_col1_interrupt;
        void int_ecc_col1_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_col1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col2_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_col2_interrupt;
        void int_ecc_col2_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_col2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col3_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_col3_interrupt;
        void int_ecc_col3_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_col3_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col4_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_col4_interrupt;
        void int_ecc_col4_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_col4_interrupt() const;
    
}; // cap_pbm_csr_intreg_status_t
    
class cap_pbm_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pbm_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_ecc_col0_enable_cpp_int_t;
        cpp_int int_var__int_ecc_col0_enable;
        void int_ecc_col0_enable (const cpp_int  & l__val);
        cpp_int int_ecc_col0_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col1_enable_cpp_int_t;
        cpp_int int_var__int_ecc_col1_enable;
        void int_ecc_col1_enable (const cpp_int  & l__val);
        cpp_int int_ecc_col1_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col2_enable_cpp_int_t;
        cpp_int int_var__int_ecc_col2_enable;
        void int_ecc_col2_enable (const cpp_int  & l__val);
        cpp_int int_ecc_col2_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col3_enable_cpp_int_t;
        cpp_int int_var__int_ecc_col3_enable;
        void int_ecc_col3_enable (const cpp_int  & l__val);
        cpp_int int_ecc_col3_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_col4_enable_cpp_int_t;
        cpp_int int_var__int_ecc_col4_enable;
        void int_ecc_col4_enable (const cpp_int  & l__val);
        cpp_int int_ecc_col4_enable() const;
    
}; // cap_pbm_csr_int_groups_int_enable_rw_reg_t
    
class cap_pbm_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_intgrp_status_t(string name = "cap_pbm_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_intgrp_status_t();
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
    
        cap_pbm_csr_intreg_status_t intreg;
    
        cap_pbm_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pbm_csr_intreg_status_t int_rw_reg;
    
}; // cap_pbm_csr_intgrp_status_t
    
class cap_pbm_csr_dhs_col_4_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_4_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_4_entry_t(string name = "cap_pbm_csr_dhs_col_4_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_4_entry_t();
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
    
        typedef pu_cpp_int< 256 > subcol0_data_cpp_int_t;
        cpp_int int_var__subcol0_data;
        void subcol0_data (const cpp_int  & l__val);
        cpp_int subcol0_data() const;
    
        typedef pu_cpp_int< 256 > subcol1_data_cpp_int_t;
        cpp_int int_var__subcol1_data;
        void subcol1_data (const cpp_int  & l__val);
        cpp_int subcol1_data() const;
    
        typedef pu_cpp_int< 10 > subcol0_ecc_cpp_int_t;
        cpp_int int_var__subcol0_ecc;
        void subcol0_ecc (const cpp_int  & l__val);
        cpp_int subcol0_ecc() const;
    
        typedef pu_cpp_int< 10 > subcol1_ecc_cpp_int_t;
        cpp_int int_var__subcol1_ecc;
        void subcol1_ecc (const cpp_int  & l__val);
        cpp_int subcol1_ecc() const;
    
        typedef pu_cpp_int< 6 > subcol0_sideband_cpp_int_t;
        cpp_int int_var__subcol0_sideband;
        void subcol0_sideband (const cpp_int  & l__val);
        cpp_int subcol0_sideband() const;
    
        typedef pu_cpp_int< 6 > subcol1_sideband_cpp_int_t;
        cpp_int int_var__subcol1_sideband;
        void subcol1_sideband (const cpp_int  & l__val);
        cpp_int subcol1_sideband() const;
    
}; // cap_pbm_csr_dhs_col_4_entry_t
    
class cap_pbm_csr_dhs_col_4_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_4_t(string name = "cap_pbm_csr_dhs_col_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_4_t();
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
    
        cap_pbm_csr_dhs_col_4_entry_t entry;
    
}; // cap_pbm_csr_dhs_col_4_t
    
class cap_pbm_csr_dhs_col_3_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_3_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_3_entry_t(string name = "cap_pbm_csr_dhs_col_3_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_3_entry_t();
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
    
        typedef pu_cpp_int< 256 > subcol0_data_cpp_int_t;
        cpp_int int_var__subcol0_data;
        void subcol0_data (const cpp_int  & l__val);
        cpp_int subcol0_data() const;
    
        typedef pu_cpp_int< 256 > subcol1_data_cpp_int_t;
        cpp_int int_var__subcol1_data;
        void subcol1_data (const cpp_int  & l__val);
        cpp_int subcol1_data() const;
    
        typedef pu_cpp_int< 10 > subcol0_ecc_cpp_int_t;
        cpp_int int_var__subcol0_ecc;
        void subcol0_ecc (const cpp_int  & l__val);
        cpp_int subcol0_ecc() const;
    
        typedef pu_cpp_int< 10 > subcol1_ecc_cpp_int_t;
        cpp_int int_var__subcol1_ecc;
        void subcol1_ecc (const cpp_int  & l__val);
        cpp_int subcol1_ecc() const;
    
        typedef pu_cpp_int< 6 > subcol0_sideband_cpp_int_t;
        cpp_int int_var__subcol0_sideband;
        void subcol0_sideband (const cpp_int  & l__val);
        cpp_int subcol0_sideband() const;
    
        typedef pu_cpp_int< 6 > subcol1_sideband_cpp_int_t;
        cpp_int int_var__subcol1_sideband;
        void subcol1_sideband (const cpp_int  & l__val);
        cpp_int subcol1_sideband() const;
    
}; // cap_pbm_csr_dhs_col_3_entry_t
    
class cap_pbm_csr_dhs_col_3_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_3_t(string name = "cap_pbm_csr_dhs_col_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_3_t();
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
    
        cap_pbm_csr_dhs_col_3_entry_t entry;
    
}; // cap_pbm_csr_dhs_col_3_t
    
class cap_pbm_csr_dhs_col_2_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_2_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_2_entry_t(string name = "cap_pbm_csr_dhs_col_2_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_2_entry_t();
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
    
        typedef pu_cpp_int< 256 > subcol0_data_cpp_int_t;
        cpp_int int_var__subcol0_data;
        void subcol0_data (const cpp_int  & l__val);
        cpp_int subcol0_data() const;
    
        typedef pu_cpp_int< 256 > subcol1_data_cpp_int_t;
        cpp_int int_var__subcol1_data;
        void subcol1_data (const cpp_int  & l__val);
        cpp_int subcol1_data() const;
    
        typedef pu_cpp_int< 10 > subcol0_ecc_cpp_int_t;
        cpp_int int_var__subcol0_ecc;
        void subcol0_ecc (const cpp_int  & l__val);
        cpp_int subcol0_ecc() const;
    
        typedef pu_cpp_int< 10 > subcol1_ecc_cpp_int_t;
        cpp_int int_var__subcol1_ecc;
        void subcol1_ecc (const cpp_int  & l__val);
        cpp_int subcol1_ecc() const;
    
        typedef pu_cpp_int< 6 > subcol0_sideband_cpp_int_t;
        cpp_int int_var__subcol0_sideband;
        void subcol0_sideband (const cpp_int  & l__val);
        cpp_int subcol0_sideband() const;
    
        typedef pu_cpp_int< 6 > subcol1_sideband_cpp_int_t;
        cpp_int int_var__subcol1_sideband;
        void subcol1_sideband (const cpp_int  & l__val);
        cpp_int subcol1_sideband() const;
    
}; // cap_pbm_csr_dhs_col_2_entry_t
    
class cap_pbm_csr_dhs_col_2_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_2_t(string name = "cap_pbm_csr_dhs_col_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_2_t();
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
    
        cap_pbm_csr_dhs_col_2_entry_t entry;
    
}; // cap_pbm_csr_dhs_col_2_t
    
class cap_pbm_csr_dhs_col_1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_1_entry_t(string name = "cap_pbm_csr_dhs_col_1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_1_entry_t();
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
    
        typedef pu_cpp_int< 256 > subcol0_data_cpp_int_t;
        cpp_int int_var__subcol0_data;
        void subcol0_data (const cpp_int  & l__val);
        cpp_int subcol0_data() const;
    
        typedef pu_cpp_int< 256 > subcol1_data_cpp_int_t;
        cpp_int int_var__subcol1_data;
        void subcol1_data (const cpp_int  & l__val);
        cpp_int subcol1_data() const;
    
        typedef pu_cpp_int< 10 > subcol0_ecc_cpp_int_t;
        cpp_int int_var__subcol0_ecc;
        void subcol0_ecc (const cpp_int  & l__val);
        cpp_int subcol0_ecc() const;
    
        typedef pu_cpp_int< 10 > subcol1_ecc_cpp_int_t;
        cpp_int int_var__subcol1_ecc;
        void subcol1_ecc (const cpp_int  & l__val);
        cpp_int subcol1_ecc() const;
    
        typedef pu_cpp_int< 6 > subcol0_sideband_cpp_int_t;
        cpp_int int_var__subcol0_sideband;
        void subcol0_sideband (const cpp_int  & l__val);
        cpp_int subcol0_sideband() const;
    
        typedef pu_cpp_int< 6 > subcol1_sideband_cpp_int_t;
        cpp_int int_var__subcol1_sideband;
        void subcol1_sideband (const cpp_int  & l__val);
        cpp_int subcol1_sideband() const;
    
}; // cap_pbm_csr_dhs_col_1_entry_t
    
class cap_pbm_csr_dhs_col_1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_1_t(string name = "cap_pbm_csr_dhs_col_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_1_t();
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
    
        cap_pbm_csr_dhs_col_1_entry_t entry;
    
}; // cap_pbm_csr_dhs_col_1_t
    
class cap_pbm_csr_dhs_col_0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_0_entry_t(string name = "cap_pbm_csr_dhs_col_0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_0_entry_t();
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
    
        typedef pu_cpp_int< 256 > subcol0_data_cpp_int_t;
        cpp_int int_var__subcol0_data;
        void subcol0_data (const cpp_int  & l__val);
        cpp_int subcol0_data() const;
    
        typedef pu_cpp_int< 256 > subcol1_data_cpp_int_t;
        cpp_int int_var__subcol1_data;
        void subcol1_data (const cpp_int  & l__val);
        cpp_int subcol1_data() const;
    
        typedef pu_cpp_int< 10 > subcol0_ecc_cpp_int_t;
        cpp_int int_var__subcol0_ecc;
        void subcol0_ecc (const cpp_int  & l__val);
        cpp_int subcol0_ecc() const;
    
        typedef pu_cpp_int< 10 > subcol1_ecc_cpp_int_t;
        cpp_int int_var__subcol1_ecc;
        void subcol1_ecc (const cpp_int  & l__val);
        cpp_int subcol1_ecc() const;
    
        typedef pu_cpp_int< 6 > subcol0_sideband_cpp_int_t;
        cpp_int int_var__subcol0_sideband;
        void subcol0_sideband (const cpp_int  & l__val);
        cpp_int subcol0_sideband() const;
    
        typedef pu_cpp_int< 6 > subcol1_sideband_cpp_int_t;
        cpp_int int_var__subcol1_sideband;
        void subcol1_sideband (const cpp_int  & l__val);
        cpp_int subcol1_sideband() const;
    
}; // cap_pbm_csr_dhs_col_0_entry_t
    
class cap_pbm_csr_dhs_col_0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_dhs_col_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_dhs_col_0_t(string name = "cap_pbm_csr_dhs_col_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_dhs_col_0_t();
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
    
        cap_pbm_csr_dhs_col_0_entry_t entry;
    
}; // cap_pbm_csr_dhs_col_0_t
    
class cap_pbm_csr_cfg_dhs_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_dhs_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_dhs_t(string name = "cap_pbm_csr_cfg_dhs_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_dhs_t();
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
    
        typedef pu_cpp_int< 14 > address_cpp_int_t;
        cpp_int int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_pbm_csr_cfg_dhs_t
    
class cap_pbm_csr_sta_bist_row_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_bist_row_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_bist_row_1_t(string name = "cap_pbm_csr_sta_bist_row_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_bist_row_1_t();
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
    
        typedef pu_cpp_int< 40 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
        typedef pu_cpp_int< 40 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
}; // cap_pbm_csr_sta_bist_row_1_t
    
class cap_pbm_csr_cfg_bist_row_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_bist_row_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_bist_row_1_t(string name = "cap_pbm_csr_cfg_bist_row_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_bist_row_1_t();
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
    
        typedef pu_cpp_int< 40 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_pbm_csr_cfg_bist_row_1_t
    
class cap_pbm_csr_sta_bist_row_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_bist_row_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_bist_row_0_t(string name = "cap_pbm_csr_sta_bist_row_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_bist_row_0_t();
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
    
        typedef pu_cpp_int< 40 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
        typedef pu_cpp_int< 40 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
}; // cap_pbm_csr_sta_bist_row_0_t
    
class cap_pbm_csr_cfg_bist_row_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_bist_row_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_bist_row_0_t(string name = "cap_pbm_csr_cfg_bist_row_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_bist_row_0_t();
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
    
        typedef pu_cpp_int< 40 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_pbm_csr_cfg_bist_row_0_t
    
class cap_pbm_csr_cnt_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cnt_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cnt_read_t(string name = "cap_pbm_csr_cnt_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cnt_read_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_cnt_read_t
    
class cap_pbm_csr_cnt_write1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cnt_write1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cnt_write1_t(string name = "cap_pbm_csr_cnt_write1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cnt_write1_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_cnt_write1_t
    
class cap_pbm_csr_cnt_write0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cnt_write0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cnt_write0_t(string name = "cap_pbm_csr_cnt_write0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cnt_write0_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_cnt_write0_t
    
class cap_pbm_csr_cfg_col_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_col_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_col_4_t(string name = "cap_pbm_csr_cfg_col_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_col_4_t();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbm_csr_cfg_col_4_t
    
class cap_pbm_csr_sat_ecc_col4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sat_ecc_col4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sat_ecc_col4_t(string name = "cap_pbm_csr_sat_ecc_col4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sat_ecc_col4_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_sat_ecc_col4_t
    
class cap_pbm_csr_sta_ecc_col4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_ecc_col4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_ecc_col4_t(string name = "cap_pbm_csr_sta_ecc_col4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_ecc_col4_t();
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
    
        typedef pu_cpp_int< 10 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbm_csr_sta_ecc_col4_t
    
class cap_pbm_csr_cfg_col_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_col_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_col_3_t(string name = "cap_pbm_csr_cfg_col_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_col_3_t();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbm_csr_cfg_col_3_t
    
class cap_pbm_csr_sat_ecc_col3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sat_ecc_col3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sat_ecc_col3_t(string name = "cap_pbm_csr_sat_ecc_col3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sat_ecc_col3_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_sat_ecc_col3_t
    
class cap_pbm_csr_sta_ecc_col3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_ecc_col3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_ecc_col3_t(string name = "cap_pbm_csr_sta_ecc_col3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_ecc_col3_t();
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
    
        typedef pu_cpp_int< 10 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbm_csr_sta_ecc_col3_t
    
class cap_pbm_csr_cfg_col_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_col_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_col_2_t(string name = "cap_pbm_csr_cfg_col_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_col_2_t();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbm_csr_cfg_col_2_t
    
class cap_pbm_csr_sat_ecc_col2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sat_ecc_col2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sat_ecc_col2_t(string name = "cap_pbm_csr_sat_ecc_col2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sat_ecc_col2_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_sat_ecc_col2_t
    
class cap_pbm_csr_sta_ecc_col2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_ecc_col2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_ecc_col2_t(string name = "cap_pbm_csr_sta_ecc_col2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_ecc_col2_t();
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
    
        typedef pu_cpp_int< 10 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbm_csr_sta_ecc_col2_t
    
class cap_pbm_csr_cfg_col_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_col_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_col_1_t(string name = "cap_pbm_csr_cfg_col_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_col_1_t();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbm_csr_cfg_col_1_t
    
class cap_pbm_csr_sat_ecc_col1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sat_ecc_col1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sat_ecc_col1_t(string name = "cap_pbm_csr_sat_ecc_col1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sat_ecc_col1_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_sat_ecc_col1_t
    
class cap_pbm_csr_sta_ecc_col1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_ecc_col1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_ecc_col1_t(string name = "cap_pbm_csr_sta_ecc_col1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_ecc_col1_t();
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
    
        typedef pu_cpp_int< 10 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbm_csr_sta_ecc_col1_t
    
class cap_pbm_csr_cfg_col_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_col_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_col_0_t(string name = "cap_pbm_csr_cfg_col_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_col_0_t();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbm_csr_cfg_col_0_t
    
class cap_pbm_csr_sat_ecc_col0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sat_ecc_col0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sat_ecc_col0_t(string name = "cap_pbm_csr_sat_ecc_col0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sat_ecc_col0_t();
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
    
        typedef pu_cpp_int< 8 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbm_csr_sat_ecc_col0_t
    
class cap_pbm_csr_sta_ecc_col0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_sta_ecc_col0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_sta_ecc_col0_t(string name = "cap_pbm_csr_sta_ecc_col0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_sta_ecc_col0_t();
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
    
        typedef pu_cpp_int< 10 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbm_csr_sta_ecc_col0_t
    
class cap_pbm_csr_cfg_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_spare_t(string name = "cap_pbm_csr_cfg_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_spare_t();
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
    
}; // cap_pbm_csr_cfg_spare_t
    
class cap_pbm_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_debug_port_t(string name = "cap_pbm_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_debug_port_t();
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
    
        typedef pu_cpp_int< 1 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_pbm_csr_cfg_debug_port_t
    
class cap_pbm_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_csr_intr_t(string name = "cap_pbm_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_csr_intr_t();
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
    
}; // cap_pbm_csr_csr_intr_t
    
class cap_pbm_csr_cfg_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_cfg_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_cfg_control_t(string name = "cap_pbm_csr_cfg_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_cfg_control_t();
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
    
        typedef pu_cpp_int< 1 > sw_reset_cpp_int_t;
        cpp_int int_var__sw_reset;
        void sw_reset (const cpp_int  & l__val);
        cpp_int sw_reset() const;
    
}; // cap_pbm_csr_cfg_control_t
    
class cap_pbm_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_base_t(string name = "cap_pbm_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_base_t();
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
    
}; // cap_pbm_csr_base_t
    
class cap_pbm_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbm_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbm_csr_t(string name = "cap_pbm_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbm_csr_t();
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
    
        cap_pbm_csr_base_t base;
    
        cap_pbm_csr_cfg_control_t cfg_control;
    
        cap_pbm_csr_csr_intr_t csr_intr;
    
        cap_pbm_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pbm_csr_cfg_spare_t cfg_spare;
    
        cap_pbm_csr_sta_ecc_col0_t sta_ecc_col0;
    
        cap_pbm_csr_sat_ecc_col0_t sat_ecc_col0;
    
        cap_pbm_csr_cfg_col_0_t cfg_col_0;
    
        cap_pbm_csr_sta_ecc_col1_t sta_ecc_col1;
    
        cap_pbm_csr_sat_ecc_col1_t sat_ecc_col1;
    
        cap_pbm_csr_cfg_col_1_t cfg_col_1;
    
        cap_pbm_csr_sta_ecc_col2_t sta_ecc_col2;
    
        cap_pbm_csr_sat_ecc_col2_t sat_ecc_col2;
    
        cap_pbm_csr_cfg_col_2_t cfg_col_2;
    
        cap_pbm_csr_sta_ecc_col3_t sta_ecc_col3;
    
        cap_pbm_csr_sat_ecc_col3_t sat_ecc_col3;
    
        cap_pbm_csr_cfg_col_3_t cfg_col_3;
    
        cap_pbm_csr_sta_ecc_col4_t sta_ecc_col4;
    
        cap_pbm_csr_sat_ecc_col4_t sat_ecc_col4;
    
        cap_pbm_csr_cfg_col_4_t cfg_col_4;
    
        cap_pbm_csr_cnt_write0_t cnt_write0;
    
        cap_pbm_csr_cnt_write1_t cnt_write1;
    
        cap_pbm_csr_cnt_read_t cnt_read;
    
        cap_pbm_csr_cfg_bist_row_0_t cfg_bist_row_0;
    
        cap_pbm_csr_sta_bist_row_0_t sta_bist_row_0;
    
        cap_pbm_csr_cfg_bist_row_1_t cfg_bist_row_1;
    
        cap_pbm_csr_sta_bist_row_1_t sta_bist_row_1;
    
        cap_pbm_csr_cfg_dhs_t cfg_dhs;
    
        cap_pbm_csr_dhs_col_0_t dhs_col_0;
    
        cap_pbm_csr_dhs_col_1_t dhs_col_1;
    
        cap_pbm_csr_dhs_col_2_t dhs_col_2;
    
        cap_pbm_csr_dhs_col_3_t dhs_col_3;
    
        cap_pbm_csr_dhs_col_4_t dhs_col_4;
    
        cap_pbm_csr_intgrp_status_t int_groups;
    
        cap_pbm_csr_intgrp_t int_ecc_col0;
    
        cap_pbm_csr_intgrp_t int_ecc_col1;
    
        cap_pbm_csr_intgrp_t int_ecc_col2;
    
        cap_pbm_csr_intgrp_t int_ecc_col3;
    
        cap_pbm_csr_intgrp_t int_ecc_col4;
    
}; // cap_pbm_csr_t
    
#endif // CAP_PBM_CSR_H
        