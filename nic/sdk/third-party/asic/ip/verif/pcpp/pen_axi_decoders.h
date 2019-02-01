
#ifndef PEN_AXI_DECODERS_H
#define PEN_AXI_DECODERS_H

#include "pen_csr_base.h" 

using namespace std;
class pen_axi_ar_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_ar_bundle_t(string name = "pen_axi_ar_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_ar_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        len_cpp_int_t int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        sz_cpp_int_t int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        burst_cpp_int_t int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cache_cpp_int_t int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // pen_axi_ar_bundle_t
    
class pen_axi_r_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_r_bundle_t(string name = "pen_axi_r_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_r_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        resp_cpp_int_t int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        last_cpp_int_t int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // pen_axi_r_bundle_t
    
class pen_axi_aw_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_aw_bundle_t(string name = "pen_axi_aw_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_aw_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        len_cpp_int_t int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        sz_cpp_int_t int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        burst_cpp_int_t int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cache_cpp_int_t int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // pen_axi_aw_bundle_t
    
class pen_axi_w_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_w_bundle_t(string name = "pen_axi_w_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_w_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        last_cpp_int_t int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 64 > strb_cpp_int_t;
        strb_cpp_int_t int_var__strb;
        void strb (const cpp_int  & l__val);
        cpp_int strb() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // pen_axi_w_bundle_t
    
class pen_axi_b_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_b_bundle_t(string name = "pen_axi_b_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_b_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        resp_cpp_int_t int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // pen_axi_b_bundle_t
    
class pen_axi_read_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_read_bundle_t(string name = "pen_axi_read_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_read_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_axi_ar_bundle_t ar;
    
        pen_axi_r_bundle_t r;
    
}; // pen_axi_read_bundle_t
    
class pen_axi_write_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_write_bundle_t(string name = "pen_axi_write_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_write_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_axi_aw_bundle_t aw;
    
        pen_axi_w_bundle_t w;
    
        pen_axi_b_bundle_t b;
    
}; // pen_axi_write_bundle_t
    
class pen_axi_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_axi_bundle_t(string name = "pen_axi_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_axi_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_axi_ar_bundle_t ar;
    
        pen_axi_r_bundle_t r;
    
        pen_axi_aw_bundle_t aw;
    
        pen_axi_w_bundle_t w;
    
        pen_axi_b_bundle_t b;
    
}; // pen_axi_bundle_t
    
class pen_te_axi_ar_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_ar_bundle_t(string name = "pen_te_axi_ar_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_ar_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 65 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        len_cpp_int_t int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        sz_cpp_int_t int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        burst_cpp_int_t int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cache_cpp_int_t int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // pen_te_axi_ar_bundle_t
    
class pen_te_axi_r_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_r_bundle_t(string name = "pen_te_axi_r_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_r_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        resp_cpp_int_t int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        last_cpp_int_t int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // pen_te_axi_r_bundle_t
    
class pen_te_axi_aw_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_aw_bundle_t(string name = "pen_te_axi_aw_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_aw_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 65 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        len_cpp_int_t int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        sz_cpp_int_t int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        burst_cpp_int_t int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cache_cpp_int_t int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // pen_te_axi_aw_bundle_t
    
class pen_te_axi_w_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_w_bundle_t(string name = "pen_te_axi_w_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_w_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        last_cpp_int_t int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 64 > strb_cpp_int_t;
        strb_cpp_int_t int_var__strb;
        void strb (const cpp_int  & l__val);
        cpp_int strb() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // pen_te_axi_w_bundle_t
    
class pen_te_axi_b_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_b_bundle_t(string name = "pen_te_axi_b_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_b_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        resp_cpp_int_t int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // pen_te_axi_b_bundle_t
    
class pen_te_axi_read_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_read_bundle_t(string name = "pen_te_axi_read_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_read_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_te_axi_ar_bundle_t ar;
    
        pen_te_axi_r_bundle_t r;
    
}; // pen_te_axi_read_bundle_t
    
class pen_te_axi_write_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_write_bundle_t(string name = "pen_te_axi_write_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_write_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_te_axi_aw_bundle_t aw;
    
        pen_te_axi_w_bundle_t w;
    
        pen_te_axi_b_bundle_t b;
    
}; // pen_te_axi_write_bundle_t
    
class pen_te_axi_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_bundle_t(string name = "pen_te_axi_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_te_axi_ar_bundle_t ar;
    
        pen_te_axi_r_bundle_t r;
    
        pen_te_axi_aw_bundle_t aw;
    
        pen_te_axi_w_bundle_t w;
    
        pen_te_axi_b_bundle_t b;
    
}; // pen_te_axi_bundle_t
    
class pen_he_axi_ar_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_ar_bundle_t(string name = "pen_he_axi_ar_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_ar_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 32 > len_cpp_int_t;
        len_cpp_int_t int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        sz_cpp_int_t int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        burst_cpp_int_t int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cache_cpp_int_t int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // pen_he_axi_ar_bundle_t
    
class pen_he_axi_r_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_r_bundle_t(string name = "pen_he_axi_r_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_r_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        resp_cpp_int_t int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        last_cpp_int_t int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // pen_he_axi_r_bundle_t
    
class pen_he_axi_aw_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_aw_bundle_t(string name = "pen_he_axi_aw_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_aw_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 32 > len_cpp_int_t;
        len_cpp_int_t int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        sz_cpp_int_t int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        burst_cpp_int_t int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cache_cpp_int_t int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // pen_he_axi_aw_bundle_t
    
class pen_he_axi_w_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_w_bundle_t(string name = "pen_he_axi_w_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_w_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        last_cpp_int_t int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 64 > strb_cpp_int_t;
        strb_cpp_int_t int_var__strb;
        void strb (const cpp_int  & l__val);
        cpp_int strb() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // pen_he_axi_w_bundle_t
    
class pen_he_axi_b_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_b_bundle_t(string name = "pen_he_axi_b_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_b_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        id_cpp_int_t int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        resp_cpp_int_t int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        ready_cpp_int_t int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // pen_he_axi_b_bundle_t
    
class pen_he_axi_read_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_read_bundle_t(string name = "pen_he_axi_read_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_read_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_he_axi_ar_bundle_t ar;
    
        pen_he_axi_r_bundle_t r;
    
}; // pen_he_axi_read_bundle_t
    
class pen_he_axi_write_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_write_bundle_t(string name = "pen_he_axi_write_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_write_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_he_axi_aw_bundle_t aw;
    
        pen_he_axi_w_bundle_t w;
    
        pen_he_axi_b_bundle_t b;
    
}; // pen_he_axi_write_bundle_t
    
class pen_he_axi_bundle_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_he_axi_bundle_t(string name = "pen_he_axi_bundle_t", pen_csr_base *parent = 0);
        virtual ~pen_he_axi_bundle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        pen_he_axi_ar_bundle_t ar;
    
        pen_he_axi_r_bundle_t r;
    
        pen_he_axi_aw_bundle_t aw;
    
        pen_he_axi_w_bundle_t w;
    
        pen_he_axi_b_bundle_t b;
    
}; // pen_he_axi_bundle_t
    
class pen_te_axi_addr_decoder_t : public pen_decoder_base {
    public:
        cpp_int_helper hlp;
    
        pen_te_axi_addr_decoder_t(string name = "pen_te_axi_addr_decoder_t", pen_csr_base *parent = 0);
        virtual ~pen_te_axi_addr_decoder_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 21 > tbl_addr_cpp_int_t;
        tbl_addr_cpp_int_t int_var__tbl_addr;
        void tbl_addr (const cpp_int  & l__val);
        cpp_int tbl_addr() const;
    
        typedef pu_cpp_int< 4 > tbl_id_cpp_int_t;
        tbl_id_cpp_int_t int_var__tbl_id;
        void tbl_id (const cpp_int  & l__val);
        cpp_int tbl_id() const;
    
        typedef pu_cpp_int< 14 > pktsize_cpp_int_t;
        pktsize_cpp_int_t int_var__pktsize;
        void pktsize (const cpp_int  & l__val);
        cpp_int pktsize() const;
    
        typedef pu_cpp_int< 2 > color_bits_cpp_int_t;
        color_bits_cpp_int_t int_var__color_bits;
        void color_bits (const cpp_int  & l__val);
        cpp_int color_bits() const;
    
        typedef pu_cpp_int< 23 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // pen_te_axi_addr_decoder_t
    
#endif // PEN_AXI_DECODERS_H
        
