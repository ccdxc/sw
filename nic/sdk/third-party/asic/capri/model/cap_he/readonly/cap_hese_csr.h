
#ifndef CAP_HESE_CSR_H
#define CAP_HESE_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_status_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t
    
class cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t
    
class cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t(string name = "cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_status_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t
    
class cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t(string name = "cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t
    
class cap_hese_csr_dhs_crypto_ctl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_dhs_crypto_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_dhs_crypto_ctl_t(string name = "cap_hese_csr_dhs_crypto_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_dhs_crypto_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w0_t xts_enc_key_array_base_w0;
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_base_w1_t xts_enc_key_array_base_w1;
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_key_array_size_t xts_enc_key_array_size;
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_desc_t xts_enc_axi_desc;
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_read_t xts_enc_axi_data_read;
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_data_write_t xts_enc_axi_data_write;
    
        cap_hese_csr_dhs_crypto_ctl_xts_enc_axi_status_t xts_enc_axi_status;
    
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w0_t xts_key_array_base_w0;
    
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_base_w1_t xts_key_array_base_w1;
    
        cap_hese_csr_dhs_crypto_ctl_xts_key_array_size_t xts_key_array_size;
    
        cap_hese_csr_dhs_crypto_ctl_xts_axi_desc_t xts_axi_desc;
    
        cap_hese_csr_dhs_crypto_ctl_xts_axi_data_read_t xts_axi_data_read;
    
        cap_hese_csr_dhs_crypto_ctl_xts_axi_data_write_t xts_axi_data_write;
    
        cap_hese_csr_dhs_crypto_ctl_xts_axi_status_t xts_axi_status;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w0_t gcm0_key_array_base_w0;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_base_w1_t gcm0_key_array_base_w1;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_key_array_size_t gcm0_key_array_size;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_desc_t gcm0_axi_desc;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_read_t gcm0_axi_data_read;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_data_write_t gcm0_axi_data_write;
    
        cap_hese_csr_dhs_crypto_ctl_gcm0_axi_status_t gcm0_axi_status;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w0_t gcm1_key_array_base_w0;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_base_w1_t gcm1_key_array_base_w1;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_key_array_size_t gcm1_key_array_size;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_desc_t gcm1_axi_desc;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_read_t gcm1_axi_data_read;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_data_write_t gcm1_axi_data_write;
    
        cap_hese_csr_dhs_crypto_ctl_gcm1_axi_status_t gcm1_axi_status;
    
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w0_t pk_key_array_base_w0;
    
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_base_w1_t pk_key_array_base_w1;
    
        cap_hese_csr_dhs_crypto_ctl_pk_key_array_size_t pk_key_array_size;
    
        cap_hese_csr_dhs_crypto_ctl_pk_axi_desc_t pk_axi_desc;
    
        cap_hese_csr_dhs_crypto_ctl_pk_axi_data_read_t pk_axi_data_read;
    
        cap_hese_csr_dhs_crypto_ctl_pk_axi_data_write_t pk_axi_data_write;
    
        cap_hese_csr_dhs_crypto_ctl_pk_axi_status_t pk_axi_status;
    
        cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w0_t pk_ring_base_w0;
    
        cap_hese_csr_dhs_crypto_ctl_pk_ring_base_w1_t pk_ring_base_w1;
    
        cap_hese_csr_dhs_crypto_ctl_pk_ring_size_t pk_ring_size;
    
        cap_hese_csr_dhs_crypto_ctl_pk_producer_idx_t pk_producer_idx;
    
        cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w0_t pk_opa_tag_addr_w0;
    
        cap_hese_csr_dhs_crypto_ctl_pk_opa_tag_addr_w1_t pk_opa_tag_addr_w1;
    
        cap_hese_csr_dhs_crypto_ctl_pk_soft_rst_t pk_soft_rst;
    
        cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w0_t pk_ci_addr_w0;
    
        cap_hese_csr_dhs_crypto_ctl_pk_ci_addr_w1_t pk_ci_addr_w1;
    
        cap_hese_csr_dhs_crypto_ctl_pk_consumer_idx_t pk_consumer_idx;
    
        cap_hese_csr_dhs_crypto_ctl_pk_status_t pk_status;
    
        cap_hese_csr_dhs_crypto_ctl_pk_error_idx_t pk_error_idx;
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t, 128> md_drbg_cryptoram_random_num0;
        #else 
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num0_t md_drbg_cryptoram_random_num0[128];
        #endif
        int get_depth_md_drbg_cryptoram_random_num0() { return 128; }
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t, 128> md_drbg_cryptoram_random_num1;
        #else 
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_random_num1_t md_drbg_cryptoram_random_num1[128];
        #endif
        int get_depth_md_drbg_cryptoram_random_num1() { return 128; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t, 8> md_drbg_cryptoram_psnl_str_p;
        #else 
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_psnl_str_p_t md_drbg_cryptoram_psnl_str_p[8];
        #endif
        int get_depth_md_drbg_cryptoram_psnl_str_p() { return 8; }
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t, 12> md_drbg_cryptoram_entropy_inst;
        #else 
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_inst_t md_drbg_cryptoram_entropy_inst[12];
        #endif
        int get_depth_md_drbg_cryptoram_entropy_inst() { return 12; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t, 8> md_drbg_cryptoram_entropy_reseed;
        #else 
        cap_hese_csr_dhs_crypto_ctl_md_drbg_cryptoram_entropy_reseed_t md_drbg_cryptoram_entropy_reseed[8];
        #endif
        int get_depth_md_drbg_cryptoram_entropy_reseed() { return 8; }
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_isr_t md_drbg_isr;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_msk_t md_drbg_msk;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_icr_t md_drbg_icr;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_ver_t md_drbg_ver;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_gct_t md_drbg_gct;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr0_t md_drbg_rsvr0;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_gs_t md_drbg_gs;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rsvr1_t md_drbg_rsvr1;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_rng_t md_drbg_rng;
    
        cap_hese_csr_dhs_crypto_ctl_md_drbg_drnge_t md_drbg_drnge;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_controlreg_t md_trng_controlreg;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifolevel_t md_trng_fifolevel;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifothresh_t md_trng_fifothresh;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_fifodepth_t md_trng_fifodepth;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_key0_t md_trng_key0;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_key1_t md_trng_key1;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_key2_t md_trng_key2;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_key3_t md_trng_key3;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_testdata_t md_trng_testdata;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_repthresh_t md_trng_repthresh;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_prop1_t md_trng_prop1;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_prop2_t md_trng_prop2;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_status_t md_trng_status;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_initwaitval_t md_trng_initwaitval;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc0_t md_trng_disableosc0;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_disableosc1_t md_trng_disableosc1;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_swofftmrval_t md_trng_swofftmrval;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_clkdiv_t md_trng_clkdiv;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf0_t md_trng_ais31conf0;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf1_t md_trng_ais31conf1;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31conf2_t md_trng_ais31conf2;
    
        cap_hese_csr_dhs_crypto_ctl_md_trng_ais31status_t md_trng_ais31status;
    
}; // cap_hese_csr_dhs_crypto_ctl_t
    
class cap_hese_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_base_t(string name = "cap_hese_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_hese_csr_base_t
    
class cap_hese_csr_cfg_he_ctl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_cfg_he_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_cfg_he_ctl_t(string name = "cap_hese_csr_cfg_he_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_cfg_he_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > hsm_mode_cpp_int_t;
        cpp_int int_var__hsm_mode;
        void hsm_mode (const cpp_int  & l__val);
        cpp_int hsm_mode() const;
    
}; // cap_hese_csr_cfg_he_ctl_t
    
class cap_hese_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_hese_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_hese_csr_t(string name = "cap_hese_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_hese_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_hese_csr_cfg_he_ctl_t cfg_he_ctl;
    
        cap_hese_csr_base_t base;
    
        cap_hese_csr_dhs_crypto_ctl_t dhs_crypto_ctl;
    
}; // cap_hese_csr_t
    
#endif // CAP_HESE_CSR_H
        