
#ifndef CAP_DPRCFG_CSR_H
#define CAP_DPRCFG_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dprcfg_csr_cfg_static_field_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprcfg_csr_cfg_static_field_t(string name = "cap_dprcfg_csr_cfg_static_field_t", cap_csr_base *parent = 0);
        virtual ~cap_dprcfg_csr_cfg_static_field_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_dprcfg_csr_cfg_static_field_t
    
class cap_dprcfg_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprcfg_csr_t(string name = "cap_dprcfg_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dprcfg_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprcfg_csr_cfg_static_field_t cfg_static_field[64];
        int get_depth_cfg_static_field() { return 64; }
    
}; // cap_dprcfg_csr_t
    
#endif // CAP_DPRCFG_CSR_H
        