
#ifndef CAP_DPPHDR_CSR_H
#define CAP_DPPHDR_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dpphdr_csr_cfg_spare_hdr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpphdr_csr_cfg_spare_hdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpphdr_csr_cfg_spare_hdr_t(string name = "cap_dpphdr_csr_cfg_spare_hdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpphdr_csr_cfg_spare_hdr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpphdr_csr_cfg_spare_hdr_t
    
class cap_dpphdr_csr_cfg_hdr_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpphdr_csr_cfg_hdr_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpphdr_csr_cfg_hdr_info_t(string name = "cap_dpphdr_csr_cfg_hdr_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpphdr_csr_cfg_hdr_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > fld_start_cpp_int_t;
        cpp_int int_var__fld_start;
        void fld_start (const cpp_int  & l__val);
        cpp_int fld_start() const;
    
        typedef pu_cpp_int< 8 > fld_end_cpp_int_t;
        cpp_int int_var__fld_end;
        void fld_end (const cpp_int  & l__val);
        cpp_int fld_end() const;
    
}; // cap_dpphdr_csr_cfg_hdr_info_t
    
class cap_dpphdr_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpphdr_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpphdr_csr_t(string name = "cap_dpphdr_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpphdr_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dpphdr_csr_cfg_hdr_info_t, 128> cfg_hdr_info;
        #else 
        cap_dpphdr_csr_cfg_hdr_info_t cfg_hdr_info[128];
        #endif
        int get_depth_cfg_hdr_info() { return 128; }
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dpphdr_csr_cfg_spare_hdr_t, 128> cfg_spare_hdr;
        #else 
        cap_dpphdr_csr_cfg_spare_hdr_t cfg_spare_hdr[128];
        #endif
        int get_depth_cfg_spare_hdr() { return 128; }
    
}; // cap_dpphdr_csr_t
    
#endif // CAP_DPPHDR_CSR_H
        