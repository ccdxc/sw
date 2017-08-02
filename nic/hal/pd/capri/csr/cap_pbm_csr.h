
#ifndef CAP_PBM_CSR_H
#define CAP_PBM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbm_csr_cfg_debug_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbm_csr_cfg_debug_t(string name = "cap_pbm_csr_cfg_debug_t", cap_csr_base *parent = 0);
        virtual ~cap_pbm_csr_cfg_debug_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 4 > select_cpp_int_t;
        select_cpp_int_t int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_pbm_csr_cfg_debug_t
    
class cap_pbm_csr_cfg_control_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbm_csr_cfg_control_t(string name = "cap_pbm_csr_cfg_control_t", cap_csr_base *parent = 0);
        virtual ~cap_pbm_csr_cfg_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > sw_reset_cpp_int_t;
        sw_reset_cpp_int_t int_var__sw_reset;
        void sw_reset (const cpp_int  & l__val);
        cpp_int sw_reset() const;
    
}; // cap_pbm_csr_cfg_control_t
    
class cap_pbm_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbm_csr_t(string name = "cap_pbm_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pbm_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbm_csr_cfg_control_t cfg_control;
    
        cap_pbm_csr_cfg_debug_t cfg_debug;
    
}; // cap_pbm_csr_t
    
#endif // CAP_PBM_CSR_H
        