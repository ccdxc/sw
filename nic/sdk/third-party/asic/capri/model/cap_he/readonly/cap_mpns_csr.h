
#ifndef CAP_MPNS_CSR_H
#define CAP_MPNS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mpns_csr_int_crypto_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_int_crypto_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_int_crypto_int_enable_clear_t(string name = "cap_mpns_csr_int_crypto_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_int_crypto_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > mpp0_enable_cpp_int_t;
        cpp_int int_var__mpp0_enable;
        void mpp0_enable (const cpp_int  & l__val);
        cpp_int mpp0_enable() const;
    
        typedef pu_cpp_int< 1 > mpp1_enable_cpp_int_t;
        cpp_int int_var__mpp1_enable;
        void mpp1_enable (const cpp_int  & l__val);
        cpp_int mpp1_enable() const;
    
        typedef pu_cpp_int< 1 > mpp2_enable_cpp_int_t;
        cpp_int int_var__mpp2_enable;
        void mpp2_enable (const cpp_int  & l__val);
        cpp_int mpp2_enable() const;
    
        typedef pu_cpp_int< 1 > mpp3_enable_cpp_int_t;
        cpp_int int_var__mpp3_enable;
        void mpp3_enable (const cpp_int  & l__val);
        cpp_int mpp3_enable() const;
    
        typedef pu_cpp_int< 1 > mpp4_enable_cpp_int_t;
        cpp_int int_var__mpp4_enable;
        void mpp4_enable (const cpp_int  & l__val);
        cpp_int mpp4_enable() const;
    
        typedef pu_cpp_int< 1 > mpp5_enable_cpp_int_t;
        cpp_int int_var__mpp5_enable;
        void mpp5_enable (const cpp_int  & l__val);
        cpp_int mpp5_enable() const;
    
        typedef pu_cpp_int< 1 > mpp6_enable_cpp_int_t;
        cpp_int int_var__mpp6_enable;
        void mpp6_enable (const cpp_int  & l__val);
        cpp_int mpp6_enable() const;
    
        typedef pu_cpp_int< 1 > mpp7_enable_cpp_int_t;
        cpp_int int_var__mpp7_enable;
        void mpp7_enable (const cpp_int  & l__val);
        cpp_int mpp7_enable() const;
    
}; // cap_mpns_csr_int_crypto_int_enable_clear_t
    
class cap_mpns_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_intreg_t(string name = "cap_mpns_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > mpp0_interrupt_cpp_int_t;
        cpp_int int_var__mpp0_interrupt;
        void mpp0_interrupt (const cpp_int  & l__val);
        cpp_int mpp0_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp1_interrupt_cpp_int_t;
        cpp_int int_var__mpp1_interrupt;
        void mpp1_interrupt (const cpp_int  & l__val);
        cpp_int mpp1_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp2_interrupt_cpp_int_t;
        cpp_int int_var__mpp2_interrupt;
        void mpp2_interrupt (const cpp_int  & l__val);
        cpp_int mpp2_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp3_interrupt_cpp_int_t;
        cpp_int int_var__mpp3_interrupt;
        void mpp3_interrupt (const cpp_int  & l__val);
        cpp_int mpp3_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp4_interrupt_cpp_int_t;
        cpp_int int_var__mpp4_interrupt;
        void mpp4_interrupt (const cpp_int  & l__val);
        cpp_int mpp4_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp5_interrupt_cpp_int_t;
        cpp_int int_var__mpp5_interrupt;
        void mpp5_interrupt (const cpp_int  & l__val);
        cpp_int mpp5_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp6_interrupt_cpp_int_t;
        cpp_int int_var__mpp6_interrupt;
        void mpp6_interrupt (const cpp_int  & l__val);
        cpp_int mpp6_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpp7_interrupt_cpp_int_t;
        cpp_int int_var__mpp7_interrupt;
        void mpp7_interrupt (const cpp_int  & l__val);
        cpp_int mpp7_interrupt() const;
    
}; // cap_mpns_csr_intreg_t
    
class cap_mpns_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_intgrp_t(string name = "cap_mpns_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_intgrp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpns_csr_intreg_t intreg;
    
        cap_mpns_csr_intreg_t int_test_set;
    
        cap_mpns_csr_int_crypto_int_enable_clear_t int_enable_set;
    
        cap_mpns_csr_int_crypto_int_enable_clear_t int_enable_clear;
    
}; // cap_mpns_csr_intgrp_t
    
class cap_mpns_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_intreg_status_t(string name = "cap_mpns_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_crypto_interrupt_cpp_int_t;
        cpp_int int_var__int_crypto_interrupt;
        void int_crypto_interrupt (const cpp_int  & l__val);
        cpp_int int_crypto_interrupt() const;
    
}; // cap_mpns_csr_intreg_status_t
    
class cap_mpns_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_int_groups_int_enable_rw_reg_t(string name = "cap_mpns_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_crypto_enable_cpp_int_t;
        cpp_int int_var__int_crypto_enable;
        void int_crypto_enable (const cpp_int  & l__val);
        cpp_int int_crypto_enable() const;
    
}; // cap_mpns_csr_int_groups_int_enable_rw_reg_t
    
class cap_mpns_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_intgrp_status_t(string name = "cap_mpns_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpns_csr_intreg_status_t intreg;
    
        cap_mpns_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_mpns_csr_intreg_status_t int_rw_reg;
    
}; // cap_mpns_csr_intgrp_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t
    
class cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t(string name = "cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t
    
class cap_mpns_csr_dhs_crypto_ctl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_dhs_crypto_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_dhs_crypto_ctl_t(string name = "cap_mpns_csr_dhs_crypto_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_dhs_crypto_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w0_t mpp0_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_base_w1_t mpp0_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ring_size_t mpp0_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_producer_idx_t mpp0_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w0_t mpp0_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_opa_tag_addr_w1_t mpp0_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_soft_rst_t mpp0_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w0_t mpp0_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_ci_addr_w1_t mpp0_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_consumer_idx_t mpp0_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_status_t mpp0_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp0_error_idx_t mpp0_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w0_t mpp1_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_base_w1_t mpp1_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ring_size_t mpp1_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_producer_idx_t mpp1_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w0_t mpp1_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_opa_tag_addr_w1_t mpp1_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_soft_rst_t mpp1_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w0_t mpp1_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_ci_addr_w1_t mpp1_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_consumer_idx_t mpp1_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_status_t mpp1_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp1_error_idx_t mpp1_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w0_t mpp2_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_base_w1_t mpp2_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ring_size_t mpp2_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_producer_idx_t mpp2_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w0_t mpp2_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_opa_tag_addr_w1_t mpp2_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_soft_rst_t mpp2_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w0_t mpp2_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_ci_addr_w1_t mpp2_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_consumer_idx_t mpp2_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_status_t mpp2_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp2_error_idx_t mpp2_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w0_t mpp3_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_base_w1_t mpp3_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ring_size_t mpp3_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_producer_idx_t mpp3_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w0_t mpp3_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_opa_tag_addr_w1_t mpp3_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_soft_rst_t mpp3_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w0_t mpp3_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_ci_addr_w1_t mpp3_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_consumer_idx_t mpp3_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_status_t mpp3_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp3_error_idx_t mpp3_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w0_t mpp4_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_base_w1_t mpp4_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ring_size_t mpp4_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_producer_idx_t mpp4_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w0_t mpp4_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_opa_tag_addr_w1_t mpp4_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_soft_rst_t mpp4_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w0_t mpp4_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_ci_addr_w1_t mpp4_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_consumer_idx_t mpp4_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_status_t mpp4_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp4_error_idx_t mpp4_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w0_t mpp5_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_base_w1_t mpp5_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ring_size_t mpp5_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_producer_idx_t mpp5_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w0_t mpp5_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_opa_tag_addr_w1_t mpp5_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_soft_rst_t mpp5_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w0_t mpp5_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_ci_addr_w1_t mpp5_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_consumer_idx_t mpp5_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_status_t mpp5_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp5_error_idx_t mpp5_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w0_t mpp6_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_base_w1_t mpp6_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ring_size_t mpp6_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_producer_idx_t mpp6_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w0_t mpp6_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_opa_tag_addr_w1_t mpp6_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_soft_rst_t mpp6_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w0_t mpp6_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_ci_addr_w1_t mpp6_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_consumer_idx_t mpp6_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_status_t mpp6_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp6_error_idx_t mpp6_error_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w0_t mpp7_ring_base_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_base_w1_t mpp7_ring_base_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ring_size_t mpp7_ring_size;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_producer_idx_t mpp7_producer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w0_t mpp7_opa_tag_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_opa_tag_addr_w1_t mpp7_opa_tag_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_soft_rst_t mpp7_soft_rst;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w0_t mpp7_ci_addr_w0;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_ci_addr_w1_t mpp7_ci_addr_w1;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_consumer_idx_t mpp7_consumer_idx;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_status_t mpp7_status;
    
        cap_mpns_csr_dhs_crypto_ctl_mpp7_error_idx_t mpp7_error_idx;
    
}; // cap_mpns_csr_dhs_crypto_ctl_t
    
class cap_mpns_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_base_t(string name = "cap_mpns_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpns_csr_base_t
    
class cap_mpns_csr_cfg_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_debug_t(string name = "cap_mpns_csr_cfg_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_debug_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        cpp_int int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
        typedef pu_cpp_int< 4 > dport_cpp_int_t;
        cpp_int int_var__dport;
        void dport (const cpp_int  & l__val);
        cpp_int dport() const;
    
}; // cap_mpns_csr_cfg_debug_t
    
class cap_mpns_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_csr_intr_t(string name = "cap_mpns_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpns_csr_csr_intr_t
    
class cap_mpns_csr_cnt_doorbell_mpp7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp7_t(string name = "cap_mpns_csr_cnt_doorbell_mpp7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp7_t
    
class cap_mpns_csr_cnt_doorbell_mpp6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp6_t(string name = "cap_mpns_csr_cnt_doorbell_mpp6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp6_t
    
class cap_mpns_csr_cnt_doorbell_mpp5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp5_t(string name = "cap_mpns_csr_cnt_doorbell_mpp5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp5_t
    
class cap_mpns_csr_cnt_doorbell_mpp4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp4_t(string name = "cap_mpns_csr_cnt_doorbell_mpp4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp4_t
    
class cap_mpns_csr_cnt_doorbell_mpp3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp3_t(string name = "cap_mpns_csr_cnt_doorbell_mpp3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp3_t
    
class cap_mpns_csr_cnt_doorbell_mpp2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp2_t(string name = "cap_mpns_csr_cnt_doorbell_mpp2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp2_t
    
class cap_mpns_csr_cnt_doorbell_mpp1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp1_t(string name = "cap_mpns_csr_cnt_doorbell_mpp1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp1_t
    
class cap_mpns_csr_cnt_doorbell_mpp0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cnt_doorbell_mpp0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cnt_doorbell_mpp0_t(string name = "cap_mpns_csr_cnt_doorbell_mpp0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cnt_doorbell_mpp0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpns_csr_cnt_doorbell_mpp0_t
    
class cap_mpns_csr_sta_bist_mpp7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp7_t(string name = "cap_mpns_csr_sta_bist_mpp7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp7_t
    
class cap_mpns_csr_sta_bist_mpp6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp6_t(string name = "cap_mpns_csr_sta_bist_mpp6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp6_t
    
class cap_mpns_csr_sta_bist_mpp5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp5_t(string name = "cap_mpns_csr_sta_bist_mpp5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp5_t
    
class cap_mpns_csr_sta_bist_mpp4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp4_t(string name = "cap_mpns_csr_sta_bist_mpp4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp4_t
    
class cap_mpns_csr_sta_bist_mpp3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp3_t(string name = "cap_mpns_csr_sta_bist_mpp3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp3_t
    
class cap_mpns_csr_sta_bist_mpp2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp2_t(string name = "cap_mpns_csr_sta_bist_mpp2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp2_t
    
class cap_mpns_csr_sta_bist_mpp1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp1_t(string name = "cap_mpns_csr_sta_bist_mpp1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp1_t
    
class cap_mpns_csr_sta_bist_mpp0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_bist_mpp0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_bist_mpp0_t(string name = "cap_mpns_csr_sta_bist_mpp0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_bist_mpp0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 10 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_mpns_csr_sta_bist_mpp0_t
    
class cap_mpns_csr_sta_crypto_mpp7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp7_t(string name = "cap_mpns_csr_sta_crypto_mpp7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp7_t
    
class cap_mpns_csr_sta_crypto_mpp6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp6_t(string name = "cap_mpns_csr_sta_crypto_mpp6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp6_t
    
class cap_mpns_csr_sta_crypto_mpp5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp5_t(string name = "cap_mpns_csr_sta_crypto_mpp5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp5_t
    
class cap_mpns_csr_sta_crypto_mpp4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp4_t(string name = "cap_mpns_csr_sta_crypto_mpp4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp4_t
    
class cap_mpns_csr_sta_crypto_mpp3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp3_t(string name = "cap_mpns_csr_sta_crypto_mpp3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp3_t
    
class cap_mpns_csr_sta_crypto_mpp2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp2_t(string name = "cap_mpns_csr_sta_crypto_mpp2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp2_t
    
class cap_mpns_csr_sta_crypto_mpp1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp1_t(string name = "cap_mpns_csr_sta_crypto_mpp1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp1_t
    
class cap_mpns_csr_sta_crypto_mpp0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_sta_crypto_mpp0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_sta_crypto_mpp0_t(string name = "cap_mpns_csr_sta_crypto_mpp0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_sta_crypto_mpp0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > busy_cpp_int_t;
        cpp_int int_var__busy;
        void busy (const cpp_int  & l__val);
        cpp_int busy() const;
    
}; // cap_mpns_csr_sta_crypto_mpp0_t
    
class cap_mpns_csr_cfg_bist_mpp7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp7_t(string name = "cap_mpns_csr_cfg_bist_mpp7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp7_t
    
class cap_mpns_csr_cfg_bist_mpp6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp6_t(string name = "cap_mpns_csr_cfg_bist_mpp6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp6_t
    
class cap_mpns_csr_cfg_bist_mpp5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp5_t(string name = "cap_mpns_csr_cfg_bist_mpp5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp5_t
    
class cap_mpns_csr_cfg_bist_mpp4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp4_t(string name = "cap_mpns_csr_cfg_bist_mpp4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp4_t
    
class cap_mpns_csr_cfg_bist_mpp3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp3_t(string name = "cap_mpns_csr_cfg_bist_mpp3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp3_t
    
class cap_mpns_csr_cfg_bist_mpp2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp2_t(string name = "cap_mpns_csr_cfg_bist_mpp2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp2_t
    
class cap_mpns_csr_cfg_bist_mpp1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp1_t(string name = "cap_mpns_csr_cfg_bist_mpp1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp1_t
    
class cap_mpns_csr_cfg_bist_mpp0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_bist_mpp0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_bist_mpp0_t(string name = "cap_mpns_csr_cfg_bist_mpp0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_bist_mpp0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_mpns_csr_cfg_bist_mpp0_t
    
class cap_mpns_csr_cfg_mp_ctl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_cfg_mp_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_cfg_mp_ctl_t(string name = "cap_mpns_csr_cfg_mp_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_cfg_mp_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > sw_rst_cpp_int_t;
        cpp_int int_var__sw_rst;
        void sw_rst (const cpp_int  & l__val);
        cpp_int sw_rst() const;
    
        typedef pu_cpp_int< 8 > clk_en_cpp_int_t;
        cpp_int int_var__clk_en;
        void clk_en (const cpp_int  & l__val);
        cpp_int clk_en() const;
    
        typedef pu_cpp_int< 2 > crypto_apb_mux_mode_cpp_int_t;
        cpp_int int_var__crypto_apb_mux_mode;
        void crypto_apb_mux_mode (const cpp_int  & l__val);
        cpp_int crypto_apb_mux_mode() const;
    
}; // cap_mpns_csr_cfg_mp_ctl_t
    
class cap_mpns_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpns_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpns_csr_t(string name = "cap_mpns_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpns_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpns_csr_cfg_mp_ctl_t cfg_mp_ctl;
    
        cap_mpns_csr_cfg_bist_mpp0_t cfg_bist_mpp0;
    
        cap_mpns_csr_cfg_bist_mpp1_t cfg_bist_mpp1;
    
        cap_mpns_csr_cfg_bist_mpp2_t cfg_bist_mpp2;
    
        cap_mpns_csr_cfg_bist_mpp3_t cfg_bist_mpp3;
    
        cap_mpns_csr_cfg_bist_mpp4_t cfg_bist_mpp4;
    
        cap_mpns_csr_cfg_bist_mpp5_t cfg_bist_mpp5;
    
        cap_mpns_csr_cfg_bist_mpp6_t cfg_bist_mpp6;
    
        cap_mpns_csr_cfg_bist_mpp7_t cfg_bist_mpp7;
    
        cap_mpns_csr_sta_crypto_mpp0_t sta_crypto_mpp0;
    
        cap_mpns_csr_sta_crypto_mpp1_t sta_crypto_mpp1;
    
        cap_mpns_csr_sta_crypto_mpp2_t sta_crypto_mpp2;
    
        cap_mpns_csr_sta_crypto_mpp3_t sta_crypto_mpp3;
    
        cap_mpns_csr_sta_crypto_mpp4_t sta_crypto_mpp4;
    
        cap_mpns_csr_sta_crypto_mpp5_t sta_crypto_mpp5;
    
        cap_mpns_csr_sta_crypto_mpp6_t sta_crypto_mpp6;
    
        cap_mpns_csr_sta_crypto_mpp7_t sta_crypto_mpp7;
    
        cap_mpns_csr_sta_bist_mpp0_t sta_bist_mpp0;
    
        cap_mpns_csr_sta_bist_mpp1_t sta_bist_mpp1;
    
        cap_mpns_csr_sta_bist_mpp2_t sta_bist_mpp2;
    
        cap_mpns_csr_sta_bist_mpp3_t sta_bist_mpp3;
    
        cap_mpns_csr_sta_bist_mpp4_t sta_bist_mpp4;
    
        cap_mpns_csr_sta_bist_mpp5_t sta_bist_mpp5;
    
        cap_mpns_csr_sta_bist_mpp6_t sta_bist_mpp6;
    
        cap_mpns_csr_sta_bist_mpp7_t sta_bist_mpp7;
    
        cap_mpns_csr_cnt_doorbell_mpp0_t cnt_doorbell_mpp0;
    
        cap_mpns_csr_cnt_doorbell_mpp1_t cnt_doorbell_mpp1;
    
        cap_mpns_csr_cnt_doorbell_mpp2_t cnt_doorbell_mpp2;
    
        cap_mpns_csr_cnt_doorbell_mpp3_t cnt_doorbell_mpp3;
    
        cap_mpns_csr_cnt_doorbell_mpp4_t cnt_doorbell_mpp4;
    
        cap_mpns_csr_cnt_doorbell_mpp5_t cnt_doorbell_mpp5;
    
        cap_mpns_csr_cnt_doorbell_mpp6_t cnt_doorbell_mpp6;
    
        cap_mpns_csr_cnt_doorbell_mpp7_t cnt_doorbell_mpp7;
    
        cap_mpns_csr_csr_intr_t csr_intr;
    
        cap_mpns_csr_cfg_debug_t cfg_debug;
    
        cap_mpns_csr_base_t base;
    
        cap_mpns_csr_dhs_crypto_ctl_t dhs_crypto_ctl;
    
        cap_mpns_csr_intgrp_status_t int_groups;
    
        cap_mpns_csr_intgrp_t int_crypto;
    
}; // cap_mpns_csr_t
    
#endif // CAP_MPNS_CSR_H
        