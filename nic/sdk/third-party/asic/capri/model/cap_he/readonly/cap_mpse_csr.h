
#ifndef CAP_MPSE_CSR_H
#define CAP_MPSE_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t
    
class cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t(string name = "cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        cpp_int int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t
    
class cap_mpse_csr_dhs_crypto_ctl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_dhs_crypto_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_dhs_crypto_ctl_t(string name = "cap_mpse_csr_dhs_crypto_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_dhs_crypto_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w0_t mpp0_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_base_w1_t mpp0_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_key_array_size_t mpp0_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_desc_t mpp0_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_read_t mpp0_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_data_write_t mpp0_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp0_axi_status_t mpp0_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w0_t mpp1_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_base_w1_t mpp1_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_key_array_size_t mpp1_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_desc_t mpp1_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_read_t mpp1_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_data_write_t mpp1_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp1_axi_status_t mpp1_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w0_t mpp2_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_base_w1_t mpp2_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_key_array_size_t mpp2_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_desc_t mpp2_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_read_t mpp2_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_data_write_t mpp2_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp2_axi_status_t mpp2_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w0_t mpp3_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_base_w1_t mpp3_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_key_array_size_t mpp3_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_desc_t mpp3_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_read_t mpp3_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_data_write_t mpp3_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp3_axi_status_t mpp3_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w0_t mpp4_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_base_w1_t mpp4_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_key_array_size_t mpp4_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_desc_t mpp4_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_read_t mpp4_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_data_write_t mpp4_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp4_axi_status_t mpp4_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w0_t mpp5_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_base_w1_t mpp5_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_key_array_size_t mpp5_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_desc_t mpp5_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_read_t mpp5_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_data_write_t mpp5_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp5_axi_status_t mpp5_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w0_t mpp6_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_base_w1_t mpp6_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_key_array_size_t mpp6_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_desc_t mpp6_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_read_t mpp6_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_data_write_t mpp6_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp6_axi_status_t mpp6_axi_status;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w0_t mpp7_key_array_base_w0;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_base_w1_t mpp7_key_array_base_w1;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_key_array_size_t mpp7_key_array_size;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_desc_t mpp7_axi_desc;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_read_t mpp7_axi_data_read;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_data_write_t mpp7_axi_data_write;
    
        cap_mpse_csr_dhs_crypto_ctl_mpp7_axi_status_t mpp7_axi_status;
    
}; // cap_mpse_csr_dhs_crypto_ctl_t
    
class cap_mpse_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_base_t(string name = "cap_mpse_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpse_csr_base_t
    
class cap_mpse_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpse_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpse_csr_t(string name = "cap_mpse_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpse_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpse_csr_base_t base;
    
        cap_mpse_csr_dhs_crypto_ctl_t dhs_crypto_ctl;
    
}; // cap_mpse_csr_t
    
#endif // CAP_MPSE_CSR_H
        