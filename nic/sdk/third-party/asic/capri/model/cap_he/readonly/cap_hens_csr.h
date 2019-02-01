
#ifndef CAP_HENS_CSR_H
#define CAP_HENS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_hens_csr_int_axi_err_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_axi_err_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_axi_err_int_enable_clear_t(string name = "cap_hens_csr_int_axi_err_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_axi_err_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > wrsp_err_enable_cpp_int_t;
        cpp_int int_var__wrsp_err_enable;
        void wrsp_err_enable (const cpp_int  & l__val);
        cpp_int wrsp_err_enable() const;
    
        typedef pu_cpp_int< 1 > rrsp_err_enable_cpp_int_t;
        cpp_int int_var__rrsp_err_enable;
        void rrsp_err_enable (const cpp_int  & l__val);
        cpp_int rrsp_err_enable() const;
    
}; // cap_hens_csr_int_axi_err_int_enable_clear_t
    
class cap_hens_csr_int_axi_err_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_axi_err_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_axi_err_int_test_set_t(string name = "cap_hens_csr_int_axi_err_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_axi_err_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > wrsp_err_interrupt_cpp_int_t;
        cpp_int int_var__wrsp_err_interrupt;
        void wrsp_err_interrupt (const cpp_int  & l__val);
        cpp_int wrsp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rrsp_err_interrupt_cpp_int_t;
        cpp_int int_var__rrsp_err_interrupt;
        void rrsp_err_interrupt (const cpp_int  & l__val);
        cpp_int rrsp_err_interrupt() const;
    
}; // cap_hens_csr_int_axi_err_int_test_set_t
    
class cap_hens_csr_int_axi_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_axi_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_axi_err_t(string name = "cap_hens_csr_int_axi_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_axi_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hens_csr_int_axi_err_int_test_set_t intreg;
    
        cap_hens_csr_int_axi_err_int_test_set_t int_test_set;
    
        cap_hens_csr_int_axi_err_int_enable_clear_t int_enable_set;
    
        cap_hens_csr_int_axi_err_int_enable_clear_t int_enable_clear;
    
}; // cap_hens_csr_int_axi_err_t
    
class cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t(string name = "cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > correctable_err_enable_cpp_int_t;
        cpp_int int_var__correctable_err_enable;
        void correctable_err_enable (const cpp_int  & l__val);
        cpp_int correctable_err_enable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_err_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_err_enable;
        void uncorrectable_err_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_err_enable() const;
    
}; // cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t
    
class cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t(string name = "cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > correctable_err_interrupt_cpp_int_t;
        cpp_int int_var__correctable_err_interrupt;
        void correctable_err_interrupt (const cpp_int  & l__val);
        cpp_int correctable_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_err_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_err_interrupt;
        void uncorrectable_err_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_err_interrupt() const;
    
}; // cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t
    
class cap_hens_csr_int_drbg_cryptoram_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_drbg_cryptoram_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_drbg_cryptoram_ecc_t(string name = "cap_hens_csr_int_drbg_cryptoram_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_drbg_cryptoram_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t intreg;
    
        cap_hens_csr_int_drbg_cryptoram_ecc_int_test_set_t int_test_set;
    
        cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t int_enable_set;
    
        cap_hens_csr_int_drbg_cryptoram_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_hens_csr_int_drbg_cryptoram_ecc_t
    
class cap_hens_csr_int_ipcore_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_ipcore_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_ipcore_int_enable_clear_t(string name = "cap_hens_csr_int_ipcore_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_ipcore_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > xts_enc_enable_cpp_int_t;
        cpp_int int_var__xts_enc_enable;
        void xts_enc_enable (const cpp_int  & l__val);
        cpp_int xts_enc_enable() const;
    
        typedef pu_cpp_int< 1 > xts_enable_cpp_int_t;
        cpp_int int_var__xts_enable;
        void xts_enable (const cpp_int  & l__val);
        cpp_int xts_enable() const;
    
        typedef pu_cpp_int< 1 > gcm0_enable_cpp_int_t;
        cpp_int int_var__gcm0_enable;
        void gcm0_enable (const cpp_int  & l__val);
        cpp_int gcm0_enable() const;
    
        typedef pu_cpp_int< 1 > gcm1_enable_cpp_int_t;
        cpp_int int_var__gcm1_enable;
        void gcm1_enable (const cpp_int  & l__val);
        cpp_int gcm1_enable() const;
    
        typedef pu_cpp_int< 1 > drbg_enable_cpp_int_t;
        cpp_int int_var__drbg_enable;
        void drbg_enable (const cpp_int  & l__val);
        cpp_int drbg_enable() const;
    
        typedef pu_cpp_int< 1 > pk_enable_cpp_int_t;
        cpp_int int_var__pk_enable;
        void pk_enable (const cpp_int  & l__val);
        cpp_int pk_enable() const;
    
        typedef pu_cpp_int< 1 > cp_enable_cpp_int_t;
        cpp_int int_var__cp_enable;
        void cp_enable (const cpp_int  & l__val);
        cpp_int cp_enable() const;
    
        typedef pu_cpp_int< 1 > dc_enable_cpp_int_t;
        cpp_int int_var__dc_enable;
        void dc_enable (const cpp_int  & l__val);
        cpp_int dc_enable() const;
    
}; // cap_hens_csr_int_ipcore_int_enable_clear_t
    
class cap_hens_csr_int_ipcore_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_ipcore_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_ipcore_intreg_t(string name = "cap_hens_csr_int_ipcore_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_ipcore_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > xts_enc_interrupt_cpp_int_t;
        cpp_int int_var__xts_enc_interrupt;
        void xts_enc_interrupt (const cpp_int  & l__val);
        cpp_int xts_enc_interrupt() const;
    
        typedef pu_cpp_int< 1 > xts_interrupt_cpp_int_t;
        cpp_int int_var__xts_interrupt;
        void xts_interrupt (const cpp_int  & l__val);
        cpp_int xts_interrupt() const;
    
        typedef pu_cpp_int< 1 > gcm0_interrupt_cpp_int_t;
        cpp_int int_var__gcm0_interrupt;
        void gcm0_interrupt (const cpp_int  & l__val);
        cpp_int gcm0_interrupt() const;
    
        typedef pu_cpp_int< 1 > gcm1_interrupt_cpp_int_t;
        cpp_int int_var__gcm1_interrupt;
        void gcm1_interrupt (const cpp_int  & l__val);
        cpp_int gcm1_interrupt() const;
    
        typedef pu_cpp_int< 1 > drbg_interrupt_cpp_int_t;
        cpp_int int_var__drbg_interrupt;
        void drbg_interrupt (const cpp_int  & l__val);
        cpp_int drbg_interrupt() const;
    
        typedef pu_cpp_int< 1 > pk_interrupt_cpp_int_t;
        cpp_int int_var__pk_interrupt;
        void pk_interrupt (const cpp_int  & l__val);
        cpp_int pk_interrupt() const;
    
        typedef pu_cpp_int< 1 > cp_interrupt_cpp_int_t;
        cpp_int int_var__cp_interrupt;
        void cp_interrupt (const cpp_int  & l__val);
        cpp_int cp_interrupt() const;
    
        typedef pu_cpp_int< 1 > dc_interrupt_cpp_int_t;
        cpp_int int_var__dc_interrupt;
        void dc_interrupt (const cpp_int  & l__val);
        cpp_int dc_interrupt() const;
    
}; // cap_hens_csr_int_ipcore_intreg_t
    
class cap_hens_csr_int_ipcore_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_ipcore_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_ipcore_t(string name = "cap_hens_csr_int_ipcore_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_ipcore_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hens_csr_int_ipcore_intreg_t intreg;
    
        cap_hens_csr_int_ipcore_intreg_t int_test_set;
    
        cap_hens_csr_int_ipcore_int_enable_clear_t int_enable_set;
    
        cap_hens_csr_int_ipcore_int_enable_clear_t int_enable_clear;
    
}; // cap_hens_csr_int_ipcore_t
    
class cap_hens_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_intreg_status_t(string name = "cap_hens_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_ipcore_interrupt_cpp_int_t;
        cpp_int int_var__int_ipcore_interrupt;
        void int_ipcore_interrupt (const cpp_int  & l__val);
        cpp_int int_ipcore_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pk_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_pk_ecc_interrupt;
        void int_pk_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_pk_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_drbg_intram_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_drbg_intram_ecc_interrupt;
        void int_drbg_intram_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_drbg_intram_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_drbg_cryptoram_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_drbg_cryptoram_ecc_interrupt;
        void int_drbg_cryptoram_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_drbg_cryptoram_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__int_axi_err_interrupt;
        void int_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int int_axi_err_interrupt() const;
    
}; // cap_hens_csr_intreg_status_t
    
class cap_hens_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_int_groups_int_enable_rw_reg_t(string name = "cap_hens_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_ipcore_enable_cpp_int_t;
        cpp_int int_var__int_ipcore_enable;
        void int_ipcore_enable (const cpp_int  & l__val);
        cpp_int int_ipcore_enable() const;
    
        typedef pu_cpp_int< 1 > int_pk_ecc_enable_cpp_int_t;
        cpp_int int_var__int_pk_ecc_enable;
        void int_pk_ecc_enable (const cpp_int  & l__val);
        cpp_int int_pk_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_drbg_intram_ecc_enable_cpp_int_t;
        cpp_int int_var__int_drbg_intram_ecc_enable;
        void int_drbg_intram_ecc_enable (const cpp_int  & l__val);
        cpp_int int_drbg_intram_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_drbg_cryptoram_ecc_enable_cpp_int_t;
        cpp_int int_var__int_drbg_cryptoram_ecc_enable;
        void int_drbg_cryptoram_ecc_enable (const cpp_int  & l__val);
        cpp_int int_drbg_cryptoram_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_axi_err_enable_cpp_int_t;
        cpp_int int_var__int_axi_err_enable;
        void int_axi_err_enable (const cpp_int  & l__val);
        cpp_int int_axi_err_enable() const;
    
}; // cap_hens_csr_int_groups_int_enable_rw_reg_t
    
class cap_hens_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_intgrp_status_t(string name = "cap_hens_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hens_csr_intreg_status_t intreg;
    
        cap_hens_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_hens_csr_intreg_status_t int_rw_reg;
    
}; // cap_hens_csr_intgrp_status_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_status_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_status_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t
    
class cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t
    
class cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t(string name = "cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_int_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_int_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_int_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_int_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_int_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_int_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_int_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_int_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_int_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_int_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_int_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_int_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_status_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_status_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_status_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_status_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_status_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_status_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_status_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_status_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t
    
class cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t(string name = "cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t
    
class cap_hens_csr_dhs_crypto_ctl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_dhs_crypto_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_dhs_crypto_ctl_t(string name = "cap_hens_csr_dhs_crypto_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_dhs_crypto_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w0_t xts_enc_ring_base_w0;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_base_w1_t xts_enc_ring_base_w1;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ring_size_t xts_enc_ring_size;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_producer_idx_t xts_enc_producer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w0_t xts_enc_opa_tag_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_opa_tag_addr_w1_t xts_enc_opa_tag_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_soft_rst_t xts_enc_soft_rst;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w0_t xts_enc_ci_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_ci_addr_w1_t xts_enc_ci_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_consumer_idx_t xts_enc_consumer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_status_t xts_enc_status;
    
        cap_hens_csr_dhs_crypto_ctl_xts_enc_error_idx_t xts_enc_error_idx;
    
        cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w0_t xts_ring_base_w0;
    
        cap_hens_csr_dhs_crypto_ctl_xts_ring_base_w1_t xts_ring_base_w1;
    
        cap_hens_csr_dhs_crypto_ctl_xts_ring_size_t xts_ring_size;
    
        cap_hens_csr_dhs_crypto_ctl_xts_producer_idx_t xts_producer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w0_t xts_opa_tag_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_xts_opa_tag_addr_w1_t xts_opa_tag_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_xts_soft_rst_t xts_soft_rst;
    
        cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w0_t xts_ci_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_xts_ci_addr_w1_t xts_ci_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_xts_consumer_idx_t xts_consumer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_xts_status_t xts_status;
    
        cap_hens_csr_dhs_crypto_ctl_xts_error_idx_t xts_error_idx;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w0_t gcm0_ring_base_w0;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_base_w1_t gcm0_ring_base_w1;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_ring_size_t gcm0_ring_size;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_producer_idx_t gcm0_producer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w0_t gcm0_opa_tag_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_opa_tag_addr_w1_t gcm0_opa_tag_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_soft_rst_t gcm0_soft_rst;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w0_t gcm0_ci_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_ci_addr_w1_t gcm0_ci_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_consumer_idx_t gcm0_consumer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_status_t gcm0_status;
    
        cap_hens_csr_dhs_crypto_ctl_gcm0_error_idx_t gcm0_error_idx;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w0_t gcm1_ring_base_w0;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_base_w1_t gcm1_ring_base_w1;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_ring_size_t gcm1_ring_size;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_producer_idx_t gcm1_producer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w0_t gcm1_opa_tag_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_opa_tag_addr_w1_t gcm1_opa_tag_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_soft_rst_t gcm1_soft_rst;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w0_t gcm1_ci_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_ci_addr_w1_t gcm1_ci_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_consumer_idx_t gcm1_consumer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_status_t gcm1_status;
    
        cap_hens_csr_dhs_crypto_ctl_gcm1_error_idx_t gcm1_error_idx;
    
        cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w0_t pk_ring_base_w0;
    
        cap_hens_csr_dhs_crypto_ctl_pk_ring_base_w1_t pk_ring_base_w1;
    
        cap_hens_csr_dhs_crypto_ctl_pk_ring_size_t pk_ring_size;
    
        cap_hens_csr_dhs_crypto_ctl_pk_producer_idx_t pk_producer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t pk_opa_tag_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t pk_opa_tag_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_pk_soft_rst_t pk_soft_rst;
    
        cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w0_t pk_ci_addr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_pk_ci_addr_w1_t pk_ci_addr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_pk_consumer_idx_t pk_consumer_idx;
    
        cap_hens_csr_dhs_crypto_ctl_pk_status_t pk_status;
    
        cap_hens_csr_dhs_crypto_ctl_pk_error_idx_t pk_error_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w0_t cp_cfg_q_base_adr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_base_adr_w1_t cp_cfg_q_base_adr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w0_t cp_cfg_hotq_base_adr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_base_adr_w1_t cp_cfg_hotq_base_adr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_pd_idx_t cp_cfg_q_pd_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_pd_idx_t cp_cfg_hotq_pd_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_t cp_sta_q_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_t cp_sta_hotq_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_q_cp_idx_early_t cp_sta_q_cp_idx_early;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_hotq_cp_idx_early_t cp_sta_hotq_cp_idx_early;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_glb_t cp_cfg_glb;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_dist_t cp_cfg_dist;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w0_t cp_cfg_ueng_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_ueng_w1_t cp_cfg_ueng_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_q_cp_idx_t cp_cfg_q_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_hotq_cp_idx_t cp_cfg_hotq_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_limit_t cp_cfg_limit;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_timeout_t cp_cfg_axi_timeout;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_t cp_cfg_host;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_data_t cp_cfg_host_opaque_tag_data;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w0_t cp_cfg_host_opaque_tag_adr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_host_opaque_tag_adr_w1_t cp_cfg_host_opaque_tag_adr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w0_t cp_cfg_axi_settings_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_axi_settings_w1_t cp_cfg_axi_settings_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_cfg_spare_debug_t cp_cfg_spare_debug;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_t cp_int;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_mask_t cp_int_mask;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_ecc_error_t cp_int_ecc_error;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w0_t cp_int_axi_error_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_axi_error_w1_t cp_int_axi_error_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w0_t cp_int_ueng_error_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_int_ueng_error_w1_t cp_int_ueng_error_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_pass_t cp_sta_bist_done_pass;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_bist_done_fail_t cp_sta_bist_done_fail;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w0_t cp_sta_debug_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w1_t cp_sta_debug_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w2_t cp_sta_debug_w2;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w3_t cp_sta_debug_w3;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w4_t cp_sta_debug_w4;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w5_t cp_sta_debug_w5;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w6_t cp_sta_debug_w6;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_debug_w7_t cp_sta_debug_w7;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w0_t cp_sta_in_bcnt_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_in_bcnt_w1_t cp_sta_in_bcnt_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w0_t cp_sta_out_bcnt_w0;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_out_bcnt_w1_t cp_sta_out_bcnt_w1;
    
        cap_hens_csr_dhs_crypto_ctl_cp_sta_ecc_error_t cp_sta_ecc_error;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w0_t dc_cfg_q_base_adr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_base_adr_w1_t dc_cfg_q_base_adr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w0_t dc_cfg_hotq_base_adr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_base_adr_w1_t dc_cfg_hotq_base_adr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_pd_idx_t dc_cfg_q_pd_idx;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_pd_idx_t dc_cfg_hotq_pd_idx;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_t dc_sta_q_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_t dc_sta_hotq_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_q_cp_idx_early_t dc_sta_q_cp_idx_early;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_hotq_cp_idx_early_t dc_sta_hotq_cp_idx_early;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_glb_t dc_cfg_glb;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_dist_t dc_cfg_dist;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w0_t dc_cfg_ueng_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_ueng_w1_t dc_cfg_ueng_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_q_cp_idx_t dc_cfg_q_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_hotq_cp_idx_t dc_cfg_hotq_cp_idx;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_limit_t dc_cfg_limit;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_timeout_t dc_cfg_axi_timeout;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_t dc_cfg_host;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_data_t dc_cfg_host_opaque_tag_data;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w0_t dc_cfg_host_opaque_tag_adr_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_host_opaque_tag_adr_w1_t dc_cfg_host_opaque_tag_adr_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w0_t dc_cfg_axi_settings_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_axi_settings_w1_t dc_cfg_axi_settings_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_cfg_spare_debug_t dc_cfg_spare_debug;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_t dc_int;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_mask_t dc_int_mask;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_ecc_error_t dc_int_ecc_error;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w0_t dc_int_axi_error_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_axi_error_w1_t dc_int_axi_error_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w0_t dc_int_ueng_error_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_int_ueng_error_w1_t dc_int_ueng_error_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_pass_t dc_sta_bist_done_pass;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_bist_done_fail_t dc_sta_bist_done_fail;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w0_t dc_sta_debug_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w1_t dc_sta_debug_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w2_t dc_sta_debug_w2;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w3_t dc_sta_debug_w3;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w4_t dc_sta_debug_w4;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w5_t dc_sta_debug_w5;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w6_t dc_sta_debug_w6;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_debug_w7_t dc_sta_debug_w7;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w0_t dc_sta_in_bcnt_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_in_bcnt_w1_t dc_sta_in_bcnt_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w0_t dc_sta_out_bcnt_w0;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_out_bcnt_w1_t dc_sta_out_bcnt_w1;
    
        cap_hens_csr_dhs_crypto_ctl_dc_sta_ecc_error_t dc_sta_ecc_error;
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t, 128> md_drbg_cryptoram_random_num0;
        #else 
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t md_drbg_cryptoram_random_num0[128];
        #endif
        int get_depth_md_drbg_cryptoram_random_num0() { return 128; }
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t, 128> md_drbg_cryptoram_random_num1;
        #else 
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t md_drbg_cryptoram_random_num1[128];
        #endif
        int get_depth_md_drbg_cryptoram_random_num1() { return 128; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t, 8> md_drbg_cryptoram_psnl_str_p;
        #else 
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t md_drbg_cryptoram_psnl_str_p[8];
        #endif
        int get_depth_md_drbg_cryptoram_psnl_str_p() { return 8; }
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t, 12> md_drbg_cryptoram_entropy_inst;
        #else 
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t md_drbg_cryptoram_entropy_inst[12];
        #endif
        int get_depth_md_drbg_cryptoram_entropy_inst() { return 12; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t, 8> md_drbg_cryptoram_entropy_reseed;
        #else 
        cap_hens_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t md_drbg_cryptoram_entropy_reseed[8];
        #endif
        int get_depth_md_drbg_cryptoram_entropy_reseed() { return 8; }
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_isr_t md_drbg_isr;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_msk_t md_drbg_msk;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_icr_t md_drbg_icr;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_ver_t md_drbg_ver;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_gct_t md_drbg_gct;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr0_t md_drbg_rsvr0;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_gs_t md_drbg_gs;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rsvr1_t md_drbg_rsvr1;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_rng_t md_drbg_rng;
    
        cap_hens_csr_dhs_crypto_ctl_md_drbg_drnge_t md_drbg_drnge;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_controlreg_t md_trng_controlreg;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifolevel_t md_trng_fifolevel;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifothresh_t md_trng_fifothresh;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_fifodepth_t md_trng_fifodepth;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_key0_t md_trng_key0;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_key1_t md_trng_key1;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_key2_t md_trng_key2;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_key3_t md_trng_key3;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_testdata_t md_trng_testdata;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_repthresh_t md_trng_repthresh;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_prop1_t md_trng_prop1;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_prop2_t md_trng_prop2;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_status_t md_trng_status;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_initwaitval_t md_trng_initwaitval;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc0_t md_trng_disableosc0;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_disableosc1_t md_trng_disableosc1;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_swofftmrval_t md_trng_swofftmrval;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_clkdiv_t md_trng_clkdiv;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf0_t md_trng_ais31conf0;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf1_t md_trng_ais31conf1;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31conf2_t md_trng_ais31conf2;
    
        cap_hens_csr_dhs_crypto_ctl_md_trng_ais31status_t md_trng_ais31status;
    
}; // cap_hens_csr_dhs_crypto_ctl_t
    
class cap_hens_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_base_t(string name = "cap_hens_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_base_t
    
class cap_hens_csr_cnt_axi_rrsp_err_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_m_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_m_t
    
class cap_hens_csr_cnt_axi_wrsp_err_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_m_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_m_t
    
class cap_hens_csr_cnt_axi_dr_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_m_t(string name = "cap_hens_csr_cnt_axi_dr_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_m_t
    
class cap_hens_csr_cnt_axi_ar_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_m_t(string name = "cap_hens_csr_cnt_axi_ar_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_m_t
    
class cap_hens_csr_cnt_axi_wrsp_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_m_t(string name = "cap_hens_csr_cnt_axi_wrsp_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_m_t
    
class cap_hens_csr_cnt_axi_dw_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_m_t(string name = "cap_hens_csr_cnt_axi_dw_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_m_t
    
class cap_hens_csr_cnt_axi_aw_m_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_m_t(string name = "cap_hens_csr_cnt_axi_aw_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_m_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_m_t
    
class cap_hens_csr_cnt_axi_rrsp_err_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_mp_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_mp_t
    
class cap_hens_csr_cnt_axi_wrsp_err_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_mp_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_mp_t
    
class cap_hens_csr_cnt_axi_dr_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_mp_t(string name = "cap_hens_csr_cnt_axi_dr_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_mp_t
    
class cap_hens_csr_cnt_axi_ar_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_mp_t(string name = "cap_hens_csr_cnt_axi_ar_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_mp_t
    
class cap_hens_csr_cnt_axi_wrsp_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_mp_t(string name = "cap_hens_csr_cnt_axi_wrsp_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_mp_t
    
class cap_hens_csr_cnt_axi_dw_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_mp_t(string name = "cap_hens_csr_cnt_axi_dw_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_mp_t
    
class cap_hens_csr_cnt_axi_aw_mp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_mp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_mp_t(string name = "cap_hens_csr_cnt_axi_aw_mp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_mp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_mp_t
    
class cap_hens_csr_cnt_axi_rrsp_err_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_dc_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_dc_t
    
class cap_hens_csr_cnt_axi_wrsp_err_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_dc_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_dc_t
    
class cap_hens_csr_cnt_axi_dr_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_dc_t(string name = "cap_hens_csr_cnt_axi_dr_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_dc_t
    
class cap_hens_csr_cnt_axi_ar_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_dc_t(string name = "cap_hens_csr_cnt_axi_ar_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_dc_t
    
class cap_hens_csr_cnt_axi_wrsp_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_dc_t(string name = "cap_hens_csr_cnt_axi_wrsp_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_dc_t
    
class cap_hens_csr_cnt_axi_dw_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_dc_t(string name = "cap_hens_csr_cnt_axi_dw_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_dc_t
    
class cap_hens_csr_cnt_axi_aw_dc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_dc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_dc_t(string name = "cap_hens_csr_cnt_axi_aw_dc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_dc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_dc_t
    
class cap_hens_csr_cnt_axi_rrsp_err_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_cp_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_cp_t
    
class cap_hens_csr_cnt_axi_wrsp_err_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_cp_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_cp_t
    
class cap_hens_csr_cnt_axi_dr_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_cp_t(string name = "cap_hens_csr_cnt_axi_dr_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_cp_t
    
class cap_hens_csr_cnt_axi_ar_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_cp_t(string name = "cap_hens_csr_cnt_axi_ar_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_cp_t
    
class cap_hens_csr_cnt_axi_wrsp_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_cp_t(string name = "cap_hens_csr_cnt_axi_wrsp_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_cp_t
    
class cap_hens_csr_cnt_axi_dw_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_cp_t(string name = "cap_hens_csr_cnt_axi_dw_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_cp_t
    
class cap_hens_csr_cnt_axi_aw_cp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_cp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_cp_t(string name = "cap_hens_csr_cnt_axi_aw_cp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_cp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_cp_t
    
class cap_hens_csr_cnt_axi_rrsp_err_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_he_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_he_t
    
class cap_hens_csr_cnt_axi_wrsp_err_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_he_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_he_t
    
class cap_hens_csr_cnt_axi_dr_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_he_t(string name = "cap_hens_csr_cnt_axi_dr_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_he_t
    
class cap_hens_csr_cnt_axi_ar_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_he_t(string name = "cap_hens_csr_cnt_axi_ar_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_he_t
    
class cap_hens_csr_cnt_axi_wrsp_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_he_t(string name = "cap_hens_csr_cnt_axi_wrsp_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_he_t
    
class cap_hens_csr_cnt_axi_dw_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_he_t(string name = "cap_hens_csr_cnt_axi_dw_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_he_t
    
class cap_hens_csr_cnt_axi_aw_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_he_t(string name = "cap_hens_csr_cnt_axi_aw_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_he_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_he_t
    
class cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t
    
class cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t
    
class cap_hens_csr_cnt_axi_dr_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_xts_enc_t(string name = "cap_hens_csr_cnt_axi_dr_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_xts_enc_t
    
class cap_hens_csr_cnt_axi_ar_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_xts_enc_t(string name = "cap_hens_csr_cnt_axi_ar_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_xts_enc_t
    
class cap_hens_csr_cnt_axi_wrsp_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_xts_enc_t(string name = "cap_hens_csr_cnt_axi_wrsp_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_xts_enc_t
    
class cap_hens_csr_cnt_axi_dw_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_xts_enc_t(string name = "cap_hens_csr_cnt_axi_dw_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_xts_enc_t
    
class cap_hens_csr_cnt_axi_aw_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_xts_enc_t(string name = "cap_hens_csr_cnt_axi_aw_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_xts_enc_t
    
class cap_hens_csr_cnt_axi_rrsp_err_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_xts_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_xts_t
    
class cap_hens_csr_cnt_axi_wrsp_err_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_xts_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_xts_t
    
class cap_hens_csr_cnt_axi_dr_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_xts_t(string name = "cap_hens_csr_cnt_axi_dr_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_xts_t
    
class cap_hens_csr_cnt_axi_ar_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_xts_t(string name = "cap_hens_csr_cnt_axi_ar_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_xts_t
    
class cap_hens_csr_cnt_axi_wrsp_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_xts_t(string name = "cap_hens_csr_cnt_axi_wrsp_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_xts_t
    
class cap_hens_csr_cnt_axi_dw_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_xts_t(string name = "cap_hens_csr_cnt_axi_dw_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_xts_t
    
class cap_hens_csr_cnt_axi_aw_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_xts_t(string name = "cap_hens_csr_cnt_axi_aw_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_xts_t
    
class cap_hens_csr_cnt_axi_rrsp_err_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_gcm1_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_gcm1_t
    
class cap_hens_csr_cnt_axi_wrsp_err_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_gcm1_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_gcm1_t
    
class cap_hens_csr_cnt_axi_dr_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_gcm1_t(string name = "cap_hens_csr_cnt_axi_dr_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_gcm1_t
    
class cap_hens_csr_cnt_axi_ar_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_gcm1_t(string name = "cap_hens_csr_cnt_axi_ar_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_gcm1_t
    
class cap_hens_csr_cnt_axi_wrsp_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_gcm1_t(string name = "cap_hens_csr_cnt_axi_wrsp_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_gcm1_t
    
class cap_hens_csr_cnt_axi_dw_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_gcm1_t(string name = "cap_hens_csr_cnt_axi_dw_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_gcm1_t
    
class cap_hens_csr_cnt_axi_aw_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_gcm1_t(string name = "cap_hens_csr_cnt_axi_aw_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_gcm1_t
    
class cap_hens_csr_cnt_axi_rrsp_err_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_rrsp_err_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_rrsp_err_gcm0_t(string name = "cap_hens_csr_cnt_axi_rrsp_err_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_rrsp_err_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_rrsp_err_gcm0_t
    
class cap_hens_csr_cnt_axi_wrsp_err_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_err_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_err_gcm0_t(string name = "cap_hens_csr_cnt_axi_wrsp_err_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_err_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hens_csr_cnt_axi_wrsp_err_gcm0_t
    
class cap_hens_csr_cnt_axi_dr_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dr_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dr_gcm0_t(string name = "cap_hens_csr_cnt_axi_dr_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dr_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dr_gcm0_t
    
class cap_hens_csr_cnt_axi_ar_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_ar_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_ar_gcm0_t(string name = "cap_hens_csr_cnt_axi_ar_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_ar_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_ar_gcm0_t
    
class cap_hens_csr_cnt_axi_wrsp_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_wrsp_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_wrsp_gcm0_t(string name = "cap_hens_csr_cnt_axi_wrsp_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_wrsp_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_wrsp_gcm0_t
    
class cap_hens_csr_cnt_axi_dw_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_dw_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_dw_gcm0_t(string name = "cap_hens_csr_cnt_axi_dw_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_dw_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_dw_gcm0_t
    
class cap_hens_csr_cnt_axi_aw_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_axi_aw_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_axi_aw_gcm0_t(string name = "cap_hens_csr_cnt_axi_aw_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_axi_aw_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_axi_aw_gcm0_t
    
class cap_hens_csr_cfg_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_debug_t(string name = "cap_hens_csr_cfg_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_debug_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cfg_debug_t
    
class cap_hens_csr_sta_pk_ErrorStateVector_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_pk_ErrorStateVector_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_pk_ErrorStateVector_t(string name = "cap_hens_csr_sta_pk_ErrorStateVector_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_pk_ErrorStateVector_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > Vec_cpp_int_t;
        cpp_int int_var__Vec;
        void Vec (const cpp_int  & l__val);
        cpp_int Vec() const;
    
}; // cap_hens_csr_sta_pk_ErrorStateVector_t
    
class cap_hens_csr_sta_pk_ErrorStateIndex_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_pk_ErrorStateIndex_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_pk_ErrorStateIndex_t(string name = "cap_hens_csr_sta_pk_ErrorStateIndex_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_pk_ErrorStateIndex_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > Index_cpp_int_t;
        cpp_int int_var__Index;
        void Index (const cpp_int  & l__val);
        cpp_int Index() const;
    
}; // cap_hens_csr_sta_pk_ErrorStateIndex_t
    
class cap_hens_csr_cnt_doorbell_pk_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_doorbell_pk_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_doorbell_pk_t(string name = "cap_hens_csr_cnt_doorbell_pk_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_doorbell_pk_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_doorbell_pk_t
    
class cap_hens_csr_cnt_doorbell_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_doorbell_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_doorbell_gcm1_t(string name = "cap_hens_csr_cnt_doorbell_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_doorbell_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_doorbell_gcm1_t
    
class cap_hens_csr_cnt_doorbell_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_doorbell_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_doorbell_gcm0_t(string name = "cap_hens_csr_cnt_doorbell_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_doorbell_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_doorbell_gcm0_t
    
class cap_hens_csr_cnt_doorbell_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_doorbell_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_doorbell_xts_t(string name = "cap_hens_csr_cnt_doorbell_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_doorbell_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_doorbell_xts_t
    
class cap_hens_csr_cnt_doorbell_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cnt_doorbell_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cnt_doorbell_xts_enc_t(string name = "cap_hens_csr_cnt_doorbell_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cnt_doorbell_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cnt_doorbell_xts_enc_t
    
class cap_hens_csr_sta_bist_drbg_cryptoram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_drbg_cryptoram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_drbg_cryptoram_t(string name = "cap_hens_csr_sta_bist_drbg_cryptoram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_drbg_cryptoram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 1 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_drbg_cryptoram_t
    
class cap_hens_csr_sta_bist_drbg_intram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_drbg_intram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_drbg_intram_t(string name = "cap_hens_csr_sta_bist_drbg_intram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_drbg_intram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 1 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_drbg_intram_t
    
class cap_hens_csr_sta_bist_pk_dma_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_pk_dma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_pk_dma_t(string name = "cap_hens_csr_sta_bist_pk_dma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_pk_dma_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 5 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_pk_dma_t
    
class cap_hens_csr_sta_bist_pk_code_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_pk_code_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_pk_code_t(string name = "cap_hens_csr_sta_bist_pk_code_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_pk_code_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_bist_pk_code_t
    
class cap_hens_csr_sta_bist_pk_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_pk_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_pk_data_t(string name = "cap_hens_csr_sta_bist_pk_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_pk_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_bist_pk_data_t
    
class cap_hens_csr_sta_bist_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_xts_enc_t(string name = "cap_hens_csr_sta_bist_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 12 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_xts_enc_t
    
class cap_hens_csr_sta_bist_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_xts_t(string name = "cap_hens_csr_sta_bist_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 12 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_xts_t
    
class cap_hens_csr_sta_bist_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_gcm1_t(string name = "cap_hens_csr_sta_bist_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 15 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 15 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_gcm1_t
    
class cap_hens_csr_sta_bist_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_bist_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_bist_gcm0_t(string name = "cap_hens_csr_sta_bist_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_bist_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 15 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 15 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_hens_csr_sta_bist_gcm0_t
    
class cap_hens_csr_cfg_bist_drbg_cryptoram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_drbg_cryptoram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_drbg_cryptoram_t(string name = "cap_hens_csr_cfg_bist_drbg_cryptoram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_drbg_cryptoram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_drbg_cryptoram_t
    
class cap_hens_csr_cfg_bist_drbg_intram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_drbg_intram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_drbg_intram_t(string name = "cap_hens_csr_cfg_bist_drbg_intram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_drbg_intram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_drbg_intram_t
    
class cap_hens_csr_cfg_bist_pk_dma_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_pk_dma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_pk_dma_t(string name = "cap_hens_csr_cfg_bist_pk_dma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_pk_dma_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_pk_dma_t
    
class cap_hens_csr_cfg_bist_pk_code_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_pk_code_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_pk_code_t(string name = "cap_hens_csr_cfg_bist_pk_code_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_pk_code_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cfg_bist_pk_code_t
    
class cap_hens_csr_cfg_bist_pk_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_pk_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_pk_data_t(string name = "cap_hens_csr_cfg_bist_pk_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_pk_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cfg_bist_pk_data_t
    
class cap_hens_csr_cfg_bist_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_xts_enc_t(string name = "cap_hens_csr_cfg_bist_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_xts_enc_t
    
class cap_hens_csr_cfg_bist_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_xts_t(string name = "cap_hens_csr_cfg_bist_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_xts_t
    
class cap_hens_csr_cfg_bist_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_gcm1_t(string name = "cap_hens_csr_cfg_bist_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 15 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_gcm1_t
    
class cap_hens_csr_cfg_bist_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_bist_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_bist_gcm0_t(string name = "cap_hens_csr_cfg_bist_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_bist_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 15 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_hens_csr_cfg_bist_gcm0_t
    
class cap_hens_csr_sta_drbg_cryptoram_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_drbg_cryptoram_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_drbg_cryptoram_ecc_t(string name = "cap_hens_csr_sta_drbg_cryptoram_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_drbg_cryptoram_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > address_cpp_int_t;
        cpp_int int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_hens_csr_sta_drbg_cryptoram_ecc_t
    
class cap_hens_csr_cfg_drbg_cryptoram_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_drbg_cryptoram_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_drbg_cryptoram_ecc_t(string name = "cap_hens_csr_cfg_drbg_cryptoram_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_drbg_cryptoram_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > disable_det_cpp_int_t;
        cpp_int int_var__disable_det;
        void disable_det (const cpp_int  & l__val);
        cpp_int disable_det() const;
    
        typedef pu_cpp_int< 1 > disable_cor_cpp_int_t;
        cpp_int int_var__disable_cor;
        void disable_cor (const cpp_int  & l__val);
        cpp_int disable_cor() const;
    
        typedef pu_cpp_int< 1 > clean_syndrome_cpp_int_t;
        cpp_int int_var__clean_syndrome;
        void clean_syndrome (const cpp_int  & l__val);
        cpp_int clean_syndrome() const;
    
        typedef pu_cpp_int< 1 > clean_address_cpp_int_t;
        cpp_int int_var__clean_address;
        void clean_address (const cpp_int  & l__val);
        cpp_int clean_address() const;
    
}; // cap_hens_csr_cfg_drbg_cryptoram_ecc_t
    
class cap_hens_csr_sta_drbg_intram_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_drbg_intram_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_drbg_intram_ecc_t(string name = "cap_hens_csr_sta_drbg_intram_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_drbg_intram_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > address_cpp_int_t;
        cpp_int int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_hens_csr_sta_drbg_intram_ecc_t
    
class cap_hens_csr_cfg_drbg_intram_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_drbg_intram_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_drbg_intram_ecc_t(string name = "cap_hens_csr_cfg_drbg_intram_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_drbg_intram_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > disable_det_cpp_int_t;
        cpp_int int_var__disable_det;
        void disable_det (const cpp_int  & l__val);
        cpp_int disable_det() const;
    
        typedef pu_cpp_int< 1 > disable_cor_cpp_int_t;
        cpp_int int_var__disable_cor;
        void disable_cor (const cpp_int  & l__val);
        cpp_int disable_cor() const;
    
        typedef pu_cpp_int< 1 > clean_syndrome_cpp_int_t;
        cpp_int int_var__clean_syndrome;
        void clean_syndrome (const cpp_int  & l__val);
        cpp_int clean_syndrome() const;
    
        typedef pu_cpp_int< 1 > clean_address_cpp_int_t;
        cpp_int int_var__clean_address;
        void clean_address (const cpp_int  & l__val);
        cpp_int clean_address() const;
    
}; // cap_hens_csr_cfg_drbg_intram_ecc_t
    
class cap_hens_csr_sta_pk_ecc_vec_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_pk_ecc_vec_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_pk_ecc_vec_t(string name = "cap_hens_csr_sta_pk_ecc_vec_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_pk_ecc_vec_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > err_1b_cpp_int_t;
        cpp_int int_var__err_1b;
        void err_1b (const cpp_int  & l__val);
        cpp_int err_1b() const;
    
        typedef pu_cpp_int< 10 > err_2b_cpp_int_t;
        cpp_int int_var__err_2b;
        void err_2b (const cpp_int  & l__val);
        cpp_int err_2b() const;
    
}; // cap_hens_csr_sta_pk_ecc_vec_t
    
class cap_hens_csr_sta_pk_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_pk_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_pk_ecc_t(string name = "cap_hens_csr_sta_pk_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_pk_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > inst_1b_err_cpp_int_t;
        cpp_int int_var__inst_1b_err;
        void inst_1b_err (const cpp_int  & l__val);
        cpp_int inst_1b_err() const;
    
        typedef pu_cpp_int< 5 > inst_2b_err_cpp_int_t;
        cpp_int int_var__inst_2b_err;
        void inst_2b_err (const cpp_int  & l__val);
        cpp_int inst_2b_err() const;
    
        typedef pu_cpp_int< 18 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > address_cpp_int_t;
        cpp_int int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_hens_csr_sta_pk_ecc_t
    
class cap_hens_csr_cfg_pk_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_pk_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_pk_ecc_t(string name = "cap_hens_csr_cfg_pk_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_pk_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > disable_det_cpp_int_t;
        cpp_int int_var__disable_det;
        void disable_det (const cpp_int  & l__val);
        cpp_int disable_det() const;
    
        typedef pu_cpp_int< 1 > disable_cor_cpp_int_t;
        cpp_int int_var__disable_cor;
        void disable_cor (const cpp_int  & l__val);
        cpp_int disable_cor() const;
    
        typedef pu_cpp_int< 1 > clean_1b_err_idx_cpp_int_t;
        cpp_int int_var__clean_1b_err_idx;
        void clean_1b_err_idx (const cpp_int  & l__val);
        cpp_int clean_1b_err_idx() const;
    
        typedef pu_cpp_int< 1 > clean_2b_err_idx_cpp_int_t;
        cpp_int int_var__clean_2b_err_idx;
        void clean_2b_err_idx (const cpp_int  & l__val);
        cpp_int clean_2b_err_idx() const;
    
        typedef pu_cpp_int< 1 > clean_syndrome_cpp_int_t;
        cpp_int int_var__clean_syndrome;
        void clean_syndrome (const cpp_int  & l__val);
        cpp_int clean_syndrome() const;
    
        typedef pu_cpp_int< 1 > clean_address_cpp_int_t;
        cpp_int int_var__clean_address;
        void clean_address (const cpp_int  & l__val);
        cpp_int clean_address() const;
    
}; // cap_hens_csr_cfg_pk_ecc_t
    
class cap_hens_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_csr_intr_t(string name = "cap_hens_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_csr_intr_t
    
class cap_hens_csr_sta_pk_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_pk_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_pk_t(string name = "cap_hens_csr_sta_pk_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_pk_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_pk_t
    
class cap_hens_csr_sta_gcm1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_gcm1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_gcm1_t(string name = "cap_hens_csr_sta_gcm1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_gcm1_t
    
class cap_hens_csr_sta_gcm0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_gcm0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_gcm0_t(string name = "cap_hens_csr_sta_gcm0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_gcm0_t
    
class cap_hens_csr_sta_xts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_xts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_xts_t(string name = "cap_hens_csr_sta_xts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_xts_t
    
class cap_hens_csr_sta_xts_enc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_sta_xts_enc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_sta_xts_enc_t(string name = "cap_hens_csr_sta_xts_enc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_sta_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_sta_xts_enc_t
    
class cap_hens_csr_cfg_he_ctl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_cfg_he_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_cfg_he_ctl_t(string name = "cap_hens_csr_cfg_he_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_cfg_he_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hens_csr_cfg_he_ctl_t
    
class cap_hens_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hens_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hens_csr_t(string name = "cap_hens_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hens_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hens_csr_cfg_he_ctl_t cfg_he_ctl;
    
        cap_hens_csr_sta_xts_enc_t sta_xts_enc;
    
        cap_hens_csr_sta_xts_t sta_xts;
    
        cap_hens_csr_sta_gcm0_t sta_gcm0;
    
        cap_hens_csr_sta_gcm1_t sta_gcm1;
    
        cap_hens_csr_sta_pk_t sta_pk;
    
        cap_hens_csr_csr_intr_t csr_intr;
    
        cap_hens_csr_cfg_pk_ecc_t cfg_pk_ecc;
    
        cap_hens_csr_sta_pk_ecc_t sta_pk_ecc;
    
        cap_hens_csr_sta_pk_ecc_vec_t sta_pk_ecc_vec;
    
        cap_hens_csr_cfg_drbg_intram_ecc_t cfg_drbg_intram_ecc;
    
        cap_hens_csr_sta_drbg_intram_ecc_t sta_drbg_intram_ecc;
    
        cap_hens_csr_cfg_drbg_cryptoram_ecc_t cfg_drbg_cryptoram_ecc;
    
        cap_hens_csr_sta_drbg_cryptoram_ecc_t sta_drbg_cryptoram_ecc;
    
        cap_hens_csr_cfg_bist_gcm0_t cfg_bist_gcm0;
    
        cap_hens_csr_cfg_bist_gcm1_t cfg_bist_gcm1;
    
        cap_hens_csr_cfg_bist_xts_t cfg_bist_xts;
    
        cap_hens_csr_cfg_bist_xts_enc_t cfg_bist_xts_enc;
    
        cap_hens_csr_cfg_bist_pk_data_t cfg_bist_pk_data;
    
        cap_hens_csr_cfg_bist_pk_code_t cfg_bist_pk_code;
    
        cap_hens_csr_cfg_bist_pk_dma_t cfg_bist_pk_dma;
    
        cap_hens_csr_cfg_bist_drbg_intram_t cfg_bist_drbg_intram;
    
        cap_hens_csr_cfg_bist_drbg_cryptoram_t cfg_bist_drbg_cryptoram;
    
        cap_hens_csr_sta_bist_gcm0_t sta_bist_gcm0;
    
        cap_hens_csr_sta_bist_gcm1_t sta_bist_gcm1;
    
        cap_hens_csr_sta_bist_xts_t sta_bist_xts;
    
        cap_hens_csr_sta_bist_xts_enc_t sta_bist_xts_enc;
    
        cap_hens_csr_sta_bist_pk_data_t sta_bist_pk_data;
    
        cap_hens_csr_sta_bist_pk_code_t sta_bist_pk_code;
    
        cap_hens_csr_sta_bist_pk_dma_t sta_bist_pk_dma;
    
        cap_hens_csr_sta_bist_drbg_intram_t sta_bist_drbg_intram;
    
        cap_hens_csr_sta_bist_drbg_cryptoram_t sta_bist_drbg_cryptoram;
    
        cap_hens_csr_cnt_doorbell_xts_enc_t cnt_doorbell_xts_enc;
    
        cap_hens_csr_cnt_doorbell_xts_t cnt_doorbell_xts;
    
        cap_hens_csr_cnt_doorbell_gcm0_t cnt_doorbell_gcm0;
    
        cap_hens_csr_cnt_doorbell_gcm1_t cnt_doorbell_gcm1;
    
        cap_hens_csr_cnt_doorbell_pk_t cnt_doorbell_pk;
    
        cap_hens_csr_sta_pk_ErrorStateIndex_t sta_pk_ErrorStateIndex;
    
        cap_hens_csr_sta_pk_ErrorStateVector_t sta_pk_ErrorStateVector;
    
        cap_hens_csr_cfg_debug_t cfg_debug;
    
        cap_hens_csr_cnt_axi_aw_gcm0_t cnt_axi_aw_gcm0;
    
        cap_hens_csr_cnt_axi_dw_gcm0_t cnt_axi_dw_gcm0;
    
        cap_hens_csr_cnt_axi_wrsp_gcm0_t cnt_axi_wrsp_gcm0;
    
        cap_hens_csr_cnt_axi_ar_gcm0_t cnt_axi_ar_gcm0;
    
        cap_hens_csr_cnt_axi_dr_gcm0_t cnt_axi_dr_gcm0;
    
        cap_hens_csr_cnt_axi_wrsp_err_gcm0_t cnt_axi_wrsp_err_gcm0;
    
        cap_hens_csr_cnt_axi_rrsp_err_gcm0_t cnt_axi_rrsp_err_gcm0;
    
        cap_hens_csr_cnt_axi_aw_gcm1_t cnt_axi_aw_gcm1;
    
        cap_hens_csr_cnt_axi_dw_gcm1_t cnt_axi_dw_gcm1;
    
        cap_hens_csr_cnt_axi_wrsp_gcm1_t cnt_axi_wrsp_gcm1;
    
        cap_hens_csr_cnt_axi_ar_gcm1_t cnt_axi_ar_gcm1;
    
        cap_hens_csr_cnt_axi_dr_gcm1_t cnt_axi_dr_gcm1;
    
        cap_hens_csr_cnt_axi_wrsp_err_gcm1_t cnt_axi_wrsp_err_gcm1;
    
        cap_hens_csr_cnt_axi_rrsp_err_gcm1_t cnt_axi_rrsp_err_gcm1;
    
        cap_hens_csr_cnt_axi_aw_xts_t cnt_axi_aw_xts;
    
        cap_hens_csr_cnt_axi_dw_xts_t cnt_axi_dw_xts;
    
        cap_hens_csr_cnt_axi_wrsp_xts_t cnt_axi_wrsp_xts;
    
        cap_hens_csr_cnt_axi_ar_xts_t cnt_axi_ar_xts;
    
        cap_hens_csr_cnt_axi_dr_xts_t cnt_axi_dr_xts;
    
        cap_hens_csr_cnt_axi_wrsp_err_xts_t cnt_axi_wrsp_err_xts;
    
        cap_hens_csr_cnt_axi_rrsp_err_xts_t cnt_axi_rrsp_err_xts;
    
        cap_hens_csr_cnt_axi_aw_xts_enc_t cnt_axi_aw_xts_enc;
    
        cap_hens_csr_cnt_axi_dw_xts_enc_t cnt_axi_dw_xts_enc;
    
        cap_hens_csr_cnt_axi_wrsp_xts_enc_t cnt_axi_wrsp_xts_enc;
    
        cap_hens_csr_cnt_axi_ar_xts_enc_t cnt_axi_ar_xts_enc;
    
        cap_hens_csr_cnt_axi_dr_xts_enc_t cnt_axi_dr_xts_enc;
    
        cap_hens_csr_cnt_axi_wrsp_err_xts_enc_t cnt_axi_wrsp_err_xts_enc;
    
        cap_hens_csr_cnt_axi_rrsp_err_xts_enc_t cnt_axi_rrsp_err_xts_enc;
    
        cap_hens_csr_cnt_axi_aw_he_t cnt_axi_aw_he;
    
        cap_hens_csr_cnt_axi_dw_he_t cnt_axi_dw_he;
    
        cap_hens_csr_cnt_axi_wrsp_he_t cnt_axi_wrsp_he;
    
        cap_hens_csr_cnt_axi_ar_he_t cnt_axi_ar_he;
    
        cap_hens_csr_cnt_axi_dr_he_t cnt_axi_dr_he;
    
        cap_hens_csr_cnt_axi_wrsp_err_he_t cnt_axi_wrsp_err_he;
    
        cap_hens_csr_cnt_axi_rrsp_err_he_t cnt_axi_rrsp_err_he;
    
        cap_hens_csr_cnt_axi_aw_cp_t cnt_axi_aw_cp;
    
        cap_hens_csr_cnt_axi_dw_cp_t cnt_axi_dw_cp;
    
        cap_hens_csr_cnt_axi_wrsp_cp_t cnt_axi_wrsp_cp;
    
        cap_hens_csr_cnt_axi_ar_cp_t cnt_axi_ar_cp;
    
        cap_hens_csr_cnt_axi_dr_cp_t cnt_axi_dr_cp;
    
        cap_hens_csr_cnt_axi_wrsp_err_cp_t cnt_axi_wrsp_err_cp;
    
        cap_hens_csr_cnt_axi_rrsp_err_cp_t cnt_axi_rrsp_err_cp;
    
        cap_hens_csr_cnt_axi_aw_dc_t cnt_axi_aw_dc;
    
        cap_hens_csr_cnt_axi_dw_dc_t cnt_axi_dw_dc;
    
        cap_hens_csr_cnt_axi_wrsp_dc_t cnt_axi_wrsp_dc;
    
        cap_hens_csr_cnt_axi_ar_dc_t cnt_axi_ar_dc;
    
        cap_hens_csr_cnt_axi_dr_dc_t cnt_axi_dr_dc;
    
        cap_hens_csr_cnt_axi_wrsp_err_dc_t cnt_axi_wrsp_err_dc;
    
        cap_hens_csr_cnt_axi_rrsp_err_dc_t cnt_axi_rrsp_err_dc;
    
        cap_hens_csr_cnt_axi_aw_mp_t cnt_axi_aw_mp;
    
        cap_hens_csr_cnt_axi_dw_mp_t cnt_axi_dw_mp;
    
        cap_hens_csr_cnt_axi_wrsp_mp_t cnt_axi_wrsp_mp;
    
        cap_hens_csr_cnt_axi_ar_mp_t cnt_axi_ar_mp;
    
        cap_hens_csr_cnt_axi_dr_mp_t cnt_axi_dr_mp;
    
        cap_hens_csr_cnt_axi_wrsp_err_mp_t cnt_axi_wrsp_err_mp;
    
        cap_hens_csr_cnt_axi_rrsp_err_mp_t cnt_axi_rrsp_err_mp;
    
        cap_hens_csr_cnt_axi_aw_m_t cnt_axi_aw_m;
    
        cap_hens_csr_cnt_axi_dw_m_t cnt_axi_dw_m;
    
        cap_hens_csr_cnt_axi_wrsp_m_t cnt_axi_wrsp_m;
    
        cap_hens_csr_cnt_axi_ar_m_t cnt_axi_ar_m;
    
        cap_hens_csr_cnt_axi_dr_m_t cnt_axi_dr_m;
    
        cap_hens_csr_cnt_axi_wrsp_err_m_t cnt_axi_wrsp_err_m;
    
        cap_hens_csr_cnt_axi_rrsp_err_m_t cnt_axi_rrsp_err_m;
    
        cap_hens_csr_base_t base;
    
        cap_hens_csr_dhs_crypto_ctl_t dhs_crypto_ctl;
    
        cap_hens_csr_intgrp_status_t int_groups;
    
        cap_hens_csr_int_ipcore_t int_ipcore;
    
        cap_hens_csr_int_drbg_cryptoram_ecc_t int_pk_ecc;
    
        cap_hens_csr_int_drbg_cryptoram_ecc_t int_drbg_intram_ecc;
    
        cap_hens_csr_int_drbg_cryptoram_ecc_t int_drbg_cryptoram_ecc;
    
        cap_hens_csr_int_axi_err_t int_axi_err;
    
}; // cap_hens_csr_t
    
#endif // CAP_HENS_CSR_H
        