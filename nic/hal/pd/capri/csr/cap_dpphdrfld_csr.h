
#ifndef CAP_DPPHDRFLD_CSR_H
#define CAP_DPPHDRFLD_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dpphdrfld_csr_cfg_hdrfld_info_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpphdrfld_csr_cfg_hdrfld_info_t(string name = "cap_dpphdrfld_csr_cfg_hdrfld_info_t", cap_csr_base *parent = 0);
        virtual ~cap_dpphdrfld_csr_cfg_hdrfld_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > size_sel_cpp_int_t;
        size_sel_cpp_int_t int_var__size_sel;
        void size_sel (const cpp_int  & l__val);
        cpp_int size_sel() const;
    
        typedef pu_cpp_int< 16 > size_val_cpp_int_t;
        size_val_cpp_int_t int_var__size_val;
        void size_val (const cpp_int  & l__val);
        cpp_int size_val() const;
    
}; // cap_dpphdrfld_csr_cfg_hdrfld_info_t
    
class cap_dpphdrfld_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpphdrfld_csr_t(string name = "cap_dpphdrfld_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpphdrfld_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dpphdrfld_csr_cfg_hdrfld_info_t cfg_hdrfld_info[256];
        int get_depth_cfg_hdrfld_info() { return 256; }
    
}; // cap_dpphdrfld_csr_t
    
#endif // CAP_DPPHDRFLD_CSR_H
        