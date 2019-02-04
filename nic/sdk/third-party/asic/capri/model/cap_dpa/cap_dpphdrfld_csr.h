
#ifndef CAP_DPPHDRFLD_CSR_H
#define CAP_DPPHDRFLD_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dpphdrfld_csr_cfg_spare_hdrfld_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpphdrfld_csr_cfg_spare_hdrfld_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpphdrfld_csr_cfg_spare_hdrfld_t(string name = "cap_dpphdrfld_csr_cfg_spare_hdrfld_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpphdrfld_csr_cfg_spare_hdrfld_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpphdrfld_csr_cfg_spare_hdrfld_t
    
class cap_dpphdrfld_csr_cfg_hdrfld_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpphdrfld_csr_cfg_hdrfld_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpphdrfld_csr_cfg_hdrfld_info_t(string name = "cap_dpphdrfld_csr_cfg_hdrfld_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpphdrfld_csr_cfg_hdrfld_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > size_sel_cpp_int_t;
        cpp_int int_var__size_sel;
        void size_sel (const cpp_int  & l__val);
        cpp_int size_sel() const;
    
        typedef pu_cpp_int< 14 > size_val_cpp_int_t;
        cpp_int int_var__size_val;
        void size_val (const cpp_int  & l__val);
        cpp_int size_val() const;
    
        typedef pu_cpp_int< 1 > allow_size0_cpp_int_t;
        cpp_int int_var__allow_size0;
        void allow_size0 (const cpp_int  & l__val);
        cpp_int allow_size0() const;
    
}; // cap_dpphdrfld_csr_cfg_hdrfld_info_t
    
class cap_dpphdrfld_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpphdrfld_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpphdrfld_csr_t(string name = "cap_dpphdrfld_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpphdrfld_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dpphdrfld_csr_cfg_hdrfld_info_t, 256> cfg_hdrfld_info;
        #else 
        cap_dpphdrfld_csr_cfg_hdrfld_info_t cfg_hdrfld_info[256];
        #endif
        int get_depth_cfg_hdrfld_info() { return 256; }
    
        #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dpphdrfld_csr_cfg_spare_hdrfld_t, 32> cfg_spare_hdrfld;
        #else 
        cap_dpphdrfld_csr_cfg_spare_hdrfld_t cfg_spare_hdrfld[32];
        #endif
        int get_depth_cfg_spare_hdrfld() { return 32; }
    
}; // cap_dpphdrfld_csr_t
    
#endif // CAP_DPPHDRFLD_CSR_H
        