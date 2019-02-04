
#ifndef CAP_DPRHDRFLD_CSR_H
#define CAP_DPRHDRFLD_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dprhdrfld_csr_cfg_spare_hdrfld_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprhdrfld_csr_cfg_spare_hdrfld_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprhdrfld_csr_cfg_spare_hdrfld_t(string name = "cap_dprhdrfld_csr_cfg_spare_hdrfld_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprhdrfld_csr_cfg_spare_hdrfld_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprhdrfld_csr_cfg_spare_hdrfld_t
    
class cap_dprhdrfld_csr_cfg_hdrfld_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprhdrfld_csr_cfg_hdrfld_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprhdrfld_csr_cfg_hdrfld_info_t(string name = "cap_dprhdrfld_csr_cfg_hdrfld_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprhdrfld_csr_cfg_hdrfld_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > source_sel_cpp_int_t;
        cpp_int int_var__source_sel;
        void source_sel (const cpp_int  & l__val);
        cpp_int source_sel() const;
    
        typedef pu_cpp_int< 14 > source_oft_cpp_int_t;
        cpp_int int_var__source_oft;
        void source_oft (const cpp_int  & l__val);
        cpp_int source_oft() const;
    
}; // cap_dprhdrfld_csr_cfg_hdrfld_info_t
    
class cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t(string name = "cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 16 > start_loc_cpp_int_t;
        cpp_int int_var__start_loc;
        void start_loc (const cpp_int  & l__val);
        cpp_int start_loc() const;
    
        typedef pu_cpp_int< 16 > fld_size_cpp_int_t;
        cpp_int int_var__fld_size;
        void fld_size (const cpp_int  & l__val);
        cpp_int fld_size() const;
    
}; // cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t
    
class cap_dprhdrfld_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprhdrfld_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprhdrfld_csr_t(string name = "cap_dprhdrfld_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprhdrfld_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t, 4> cfg_ingress_rw_phv_info;
        #else 
        cap_dprhdrfld_csr_cfg_ingress_rw_phv_info_t cfg_ingress_rw_phv_info[4];
        #endif
        int get_depth_cfg_ingress_rw_phv_info() { return 4; }
    
        #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprhdrfld_csr_cfg_hdrfld_info_t, 256> cfg_hdrfld_info;
        #else 
        cap_dprhdrfld_csr_cfg_hdrfld_info_t cfg_hdrfld_info[256];
        #endif
        int get_depth_cfg_hdrfld_info() { return 256; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprhdrfld_csr_cfg_spare_hdrfld_t, 4> cfg_spare_hdrfld;
        #else 
        cap_dprhdrfld_csr_cfg_spare_hdrfld_t cfg_spare_hdrfld[4];
        #endif
        int get_depth_cfg_spare_hdrfld() { return 4; }
    
}; // cap_dprhdrfld_csr_t
    
#endif // CAP_DPRHDRFLD_CSR_H
        