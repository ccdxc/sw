
#ifndef CAP_DPPHDR_CSR_H
#define CAP_DPPHDR_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dpphdr_csr_cfg_hdr_info_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpphdr_csr_cfg_hdr_info_t(string name = "cap_dpphdr_csr_cfg_hdr_info_t", cap_csr_base *parent = 0);
        virtual ~cap_dpphdr_csr_cfg_hdr_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > fld_start_cpp_int_t;
        fld_start_cpp_int_t int_var__fld_start;
        void fld_start (const cpp_int  & l__val);
        cpp_int fld_start() const;
    
        typedef pu_cpp_int< 8 > fld_end_cpp_int_t;
        fld_end_cpp_int_t int_var__fld_end;
        void fld_end (const cpp_int  & l__val);
        cpp_int fld_end() const;
    
}; // cap_dpphdr_csr_cfg_hdr_info_t
    
class cap_dpphdr_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpphdr_csr_t(string name = "cap_dpphdr_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpphdr_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dpphdr_csr_cfg_hdr_info_t cfg_hdr_info[128];
        int get_depth_cfg_hdr_info() { return 128; }
    
}; // cap_dpphdr_csr_t
    
#endif // CAP_DPPHDR_CSR_H
        