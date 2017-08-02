
#ifndef CAP_PR_CSR_H
#define CAP_PR_CSR_H

#include "cap_csr_base.h" 
#include "cap_psp_csr.h" 
#include "cap_prd_csr.h" 

using namespace std;
class cap_pr_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pr_csr_base_t(string name = "cap_pr_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pr_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        scratch_reg_cpp_int_t int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_pr_csr_base_t
    
class cap_pr_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pr_csr_t(string name = "cap_pr_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pr_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pr_csr_base_t base;
    
        cap_prd_csr_t prd;
    
        cap_psp_csr_t psp;
    
}; // cap_pr_csr_t
    
#endif // CAP_PR_CSR_H
        