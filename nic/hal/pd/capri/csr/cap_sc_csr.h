
#ifndef CAP_SC_CSR_H
#define CAP_SC_CSR_H

#include "cap_csr_base.h" 
#include "cap_pict_csr.h" 
#include "cap_pics_csr.h" 

using namespace std;
class cap_sc_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_sc_csr_t(string name = "cap_sc_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_sc_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pics_csr_t pics;
    
        cap_pict_csr_t pict;
    
}; // cap_sc_csr_t
    
#endif // CAP_SC_CSR_H
        