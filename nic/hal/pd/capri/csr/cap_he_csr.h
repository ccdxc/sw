
#ifndef CAP_HE_CSR_H
#define CAP_HE_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_consumer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_consumer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_consumer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_consumer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_consumer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_producer_idx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_producer_idx_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_producer_idx_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_producer_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_producer_idx_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_status_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_status_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_status_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_write_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_write_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_write_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_write_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_read_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_read_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_read_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_read_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_desc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_desc_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_desc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_desc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_desc_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_size_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_size_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_size_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_size_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w1_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w1_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w0_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w0_t
    
class cap_he_csr_dhs_cap_he_ipcore_ctl_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_t(string name = "cap_he_csr_dhs_cap_he_ipcore_ctl_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_dhs_cap_he_ipcore_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w0_t dma_xts_enc_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_base_w1_t dma_xts_enc_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_key_array_size_t dma_xts_enc_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_desc_t dma_xts_enc_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_read_t dma_xts_enc_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_data_write_t dma_xts_enc_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_axi_status_t dma_xts_enc_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w0_t dma_xts_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_base_w1_t dma_xts_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_key_array_size_t dma_xts_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_desc_t dma_xts_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_read_t dma_xts_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_data_write_t dma_xts_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_axi_status_t dma_xts_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w0_t dma_gcm0_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_base_w1_t dma_gcm0_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_key_array_size_t dma_gcm0_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_desc_t dma_gcm0_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_read_t dma_gcm0_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_data_write_t dma_gcm0_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_axi_status_t dma_gcm0_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w0_t dma_gcm1_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_base_w1_t dma_gcm1_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_key_array_size_t dma_gcm1_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_desc_t dma_gcm1_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_read_t dma_gcm1_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_data_write_t dma_gcm1_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_axi_status_t dma_gcm1_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w0_t dma_mpp_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_base_w1_t dma_mpp_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_key_array_size_t dma_mpp_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_desc_t dma_mpp_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_read_t dma_mpp_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_data_write_t dma_mpp_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_axi_status_t dma_mpp_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w0_t dma_pk0_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_base_w1_t dma_pk0_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_key_array_size_t dma_pk0_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_desc_t dma_pk0_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_read_t dma_pk0_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_data_write_t dma_pk0_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_axi_status_t dma_pk0_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w0_t dma_pk1_key_array_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_base_w1_t dma_pk1_key_array_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_key_array_size_t dma_pk1_key_array_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_desc_t dma_pk1_axi_desc;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_read_t dma_pk1_axi_data_read;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_data_write_t dma_pk1_axi_data_write;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_axi_status_t dma_pk1_axi_status;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w0_t dma_xts_enc_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_base_w1_t dma_xts_enc_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_ring_size_t dma_xts_enc_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_producer_idx_t dma_xts_enc_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w0_t dma_xts_enc_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_opa_tag_addr_w1_t dma_xts_enc_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w0_t dma_xts_enc_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_dbell_addr_w1_t dma_xts_enc_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_enc_consumer_idx_t dma_xts_enc_consumer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w0_t dma_xts_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_base_w1_t dma_xts_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_ring_size_t dma_xts_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_producer_idx_t dma_xts_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w0_t dma_xts_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_opa_tag_addr_w1_t dma_xts_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w0_t dma_xts_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_dbell_addr_w1_t dma_xts_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_xts_consumer_idx_t dma_xts_consumer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w0_t dma_gcm0_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_base_w1_t dma_gcm0_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_ring_size_t dma_gcm0_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_producer_idx_t dma_gcm0_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w0_t dma_gcm0_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_opa_tag_addr_w1_t dma_gcm0_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w0_t dma_gcm0_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_dbell_addr_w1_t dma_gcm0_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm0_consumer_idx_t dma_gcm0_consumer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w0_t dma_gcm1_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_base_w1_t dma_gcm1_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_ring_size_t dma_gcm1_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_producer_idx_t dma_gcm1_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w0_t dma_gcm1_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_opa_tag_addr_w1_t dma_gcm1_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w0_t dma_gcm1_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_dbell_addr_w1_t dma_gcm1_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_gcm1_consumer_idx_t dma_gcm1_consumer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w0_t dma_mpp_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_base_w1_t dma_mpp_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_ring_size_t dma_mpp_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_producer_idx_t dma_mpp_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w0_t dma_mpp_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_opa_tag_addr_w1_t dma_mpp_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w0_t dma_mpp_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_dbell_addr_w1_t dma_mpp_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_mpp_consumer_idx_t dma_mpp_consumer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w0_t dma_pk0_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_base_w1_t dma_pk0_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_ring_size_t dma_pk0_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_producer_idx_t dma_pk0_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w0_t dma_pk0_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_opa_tag_addr_w1_t dma_pk0_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w0_t dma_pk0_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_dbell_addr_w1_t dma_pk0_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk0_consumer_idx_t dma_pk0_consumer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w0_t dma_pk1_ring_base_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_base_w1_t dma_pk1_ring_base_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_ring_size_t dma_pk1_ring_size;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_producer_idx_t dma_pk1_producer_idx;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w0_t dma_pk1_opa_tag_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_opa_tag_addr_w1_t dma_pk1_opa_tag_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w0_t dma_pk1_dbell_addr_w0;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_dbell_addr_w1_t dma_pk1_dbell_addr_w1;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_dma_pk1_consumer_idx_t dma_pk1_consumer_idx;
    
}; // cap_he_csr_dhs_cap_he_ipcore_ctl_t
    
class cap_he_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_base_t(string name = "cap_he_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_base_t();
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
    
}; // cap_he_csr_base_t
    
class cap_he_csr_sta_cap_he_baip_dma_pk1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_pk1_t(string name = "cap_he_csr_sta_cap_he_baip_dma_pk1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_pk1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_pk1_t
    
class cap_he_csr_sta_cap_he_baip_dma_pk0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_pk0_t(string name = "cap_he_csr_sta_cap_he_baip_dma_pk0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_pk0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_pk0_t
    
class cap_he_csr_sta_cap_he_baip_dma_mpp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_mpp_t(string name = "cap_he_csr_sta_cap_he_baip_dma_mpp_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_mpp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_mpp_t
    
class cap_he_csr_sta_cap_he_baip_dma_gcm1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_gcm1_t(string name = "cap_he_csr_sta_cap_he_baip_dma_gcm1_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_gcm1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_gcm1_t
    
class cap_he_csr_sta_cap_he_baip_dma_gcm0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_gcm0_t(string name = "cap_he_csr_sta_cap_he_baip_dma_gcm0_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_gcm0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_gcm0_t
    
class cap_he_csr_sta_cap_he_baip_dma_xts_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_xts_t(string name = "cap_he_csr_sta_cap_he_baip_dma_xts_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_xts_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_xts_t
    
class cap_he_csr_sta_cap_he_baip_dma_xts_enc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_sta_cap_he_baip_dma_xts_enc_t(string name = "cap_he_csr_sta_cap_he_baip_dma_xts_enc_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_sta_cap_he_baip_dma_xts_enc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > fld_cpp_int_t;
        fld_cpp_int_t int_var__fld;
        void fld (const cpp_int  & l__val);
        cpp_int fld() const;
    
}; // cap_he_csr_sta_cap_he_baip_dma_xts_enc_t
    
class cap_he_csr_cfg_cap_he_ctl_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_cfg_cap_he_ctl_t(string name = "cap_he_csr_cfg_cap_he_ctl_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_cfg_cap_he_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > sw_rst_cpp_int_t;
        sw_rst_cpp_int_t int_var__sw_rst;
        void sw_rst (const cpp_int  & l__val);
        cpp_int sw_rst() const;
    
        typedef pu_cpp_int< 8 > clk_en_cpp_int_t;
        clk_en_cpp_int_t int_var__clk_en;
        void clk_en (const cpp_int  & l__val);
        cpp_int clk_en() const;
    
}; // cap_he_csr_cfg_cap_he_ctl_t
    
class cap_he_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_he_csr_t(string name = "cap_he_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_he_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_he_csr_cfg_cap_he_ctl_t cfg_cap_he_ctl;
    
        cap_he_csr_sta_cap_he_baip_dma_xts_enc_t sta_cap_he_baip_dma_xts_enc;
    
        cap_he_csr_sta_cap_he_baip_dma_xts_t sta_cap_he_baip_dma_xts;
    
        cap_he_csr_sta_cap_he_baip_dma_gcm0_t sta_cap_he_baip_dma_gcm0;
    
        cap_he_csr_sta_cap_he_baip_dma_gcm1_t sta_cap_he_baip_dma_gcm1;
    
        cap_he_csr_sta_cap_he_baip_dma_mpp_t sta_cap_he_baip_dma_mpp;
    
        cap_he_csr_sta_cap_he_baip_dma_pk0_t sta_cap_he_baip_dma_pk0;
    
        cap_he_csr_sta_cap_he_baip_dma_pk1_t sta_cap_he_baip_dma_pk1;
    
        cap_he_csr_base_t base;
    
        cap_he_csr_dhs_cap_he_ipcore_ctl_t dhs_cap_he_ipcore_ctl;
    
}; // cap_he_csr_t
    
#endif // CAP_HE_CSR_H
        