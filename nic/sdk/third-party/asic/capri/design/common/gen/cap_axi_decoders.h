
#ifndef CAP_AXI_DECODERS_H
#define CAP_AXI_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_axi_ar_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_ar_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_ar_bundle_t(string name = "cap_axi_ar_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_ar_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // cap_axi_ar_bundle_t
    
class cap_axi_r_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_r_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_r_bundle_t(string name = "cap_axi_r_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_r_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        cpp_int int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_axi_r_bundle_t
    
class cap_axi_aw_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_aw_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_aw_bundle_t(string name = "cap_axi_aw_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_aw_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // cap_axi_aw_bundle_t
    
class cap_axi_w_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_w_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_w_bundle_t(string name = "cap_axi_w_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_w_bundle_t();
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
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        cpp_int int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 64 > strb_cpp_int_t;
        cpp_int int_var__strb;
        void strb (const cpp_int  & l__val);
        cpp_int strb() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // cap_axi_w_bundle_t
    
class cap_axi_b_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_b_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_b_bundle_t(string name = "cap_axi_b_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_b_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // cap_axi_b_bundle_t
    
class cap_axi_read_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_read_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_read_bundle_t(string name = "cap_axi_read_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_read_bundle_t();
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
    
        cap_axi_ar_bundle_t ar;
    
        cap_axi_r_bundle_t r;
    
}; // cap_axi_read_bundle_t
    
class cap_axi_write_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_write_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_write_bundle_t(string name = "cap_axi_write_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_write_bundle_t();
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
    
        cap_axi_aw_bundle_t aw;
    
        cap_axi_w_bundle_t w;
    
        cap_axi_b_bundle_t b;
    
}; // cap_axi_write_bundle_t
    
class cap_axi_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_axi_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_axi_bundle_t(string name = "cap_axi_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_axi_bundle_t();
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
    
        cap_axi_ar_bundle_t ar;
    
        cap_axi_r_bundle_t r;
    
        cap_axi_aw_bundle_t aw;
    
        cap_axi_w_bundle_t w;
    
        cap_axi_b_bundle_t b;
    
}; // cap_axi_bundle_t
    
class cap_te_axi_ar_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_ar_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_ar_bundle_t(string name = "cap_te_axi_ar_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_ar_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 65 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // cap_te_axi_ar_bundle_t
    
class cap_te_axi_r_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_r_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_r_bundle_t(string name = "cap_te_axi_r_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_r_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        cpp_int int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_te_axi_r_bundle_t
    
class cap_te_axi_aw_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_aw_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_aw_bundle_t(string name = "cap_te_axi_aw_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_aw_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 65 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // cap_te_axi_aw_bundle_t
    
class cap_te_axi_w_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_w_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_w_bundle_t(string name = "cap_te_axi_w_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_w_bundle_t();
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
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        cpp_int int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 64 > strb_cpp_int_t;
        cpp_int int_var__strb;
        void strb (const cpp_int  & l__val);
        cpp_int strb() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // cap_te_axi_w_bundle_t
    
class cap_te_axi_b_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_b_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_b_bundle_t(string name = "cap_te_axi_b_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_b_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // cap_te_axi_b_bundle_t
    
class cap_te_axi_read_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_read_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_read_bundle_t(string name = "cap_te_axi_read_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_read_bundle_t();
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
    
        cap_te_axi_ar_bundle_t ar;
    
        cap_te_axi_r_bundle_t r;
    
}; // cap_te_axi_read_bundle_t
    
class cap_te_axi_write_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_write_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_write_bundle_t(string name = "cap_te_axi_write_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_write_bundle_t();
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
    
        cap_te_axi_aw_bundle_t aw;
    
        cap_te_axi_w_bundle_t w;
    
        cap_te_axi_b_bundle_t b;
    
}; // cap_te_axi_write_bundle_t
    
class cap_te_axi_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_bundle_t(string name = "cap_te_axi_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_bundle_t();
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
    
        cap_te_axi_ar_bundle_t ar;
    
        cap_te_axi_r_bundle_t r;
    
        cap_te_axi_aw_bundle_t aw;
    
        cap_te_axi_w_bundle_t w;
    
        cap_te_axi_b_bundle_t b;
    
}; // cap_te_axi_bundle_t
    
class cap_he_axi_ar_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_ar_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_ar_bundle_t(string name = "cap_he_axi_ar_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_ar_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 32 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // cap_he_axi_ar_bundle_t
    
class cap_he_axi_r_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_r_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_r_bundle_t(string name = "cap_he_axi_r_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_r_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        cpp_int int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_he_axi_r_bundle_t
    
class cap_he_axi_aw_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_aw_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_aw_bundle_t(string name = "cap_he_axi_aw_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_aw_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 32 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 2 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
}; // cap_he_axi_aw_bundle_t
    
class cap_he_axi_w_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_w_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_w_bundle_t(string name = "cap_he_axi_w_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_w_bundle_t();
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
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > last_cpp_int_t;
        cpp_int int_var__last;
        void last (const cpp_int  & l__val);
        cpp_int last() const;
    
        typedef pu_cpp_int< 64 > strb_cpp_int_t;
        cpp_int int_var__strb;
        void strb (const cpp_int  & l__val);
        cpp_int strb() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // cap_he_axi_w_bundle_t
    
class cap_he_axi_b_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_b_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_b_bundle_t(string name = "cap_he_axi_b_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_b_bundle_t();
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
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
}; // cap_he_axi_b_bundle_t
    
class cap_he_axi_read_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_read_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_read_bundle_t(string name = "cap_he_axi_read_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_read_bundle_t();
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
    
        cap_he_axi_ar_bundle_t ar;
    
        cap_he_axi_r_bundle_t r;
    
}; // cap_he_axi_read_bundle_t
    
class cap_he_axi_write_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_write_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_write_bundle_t(string name = "cap_he_axi_write_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_write_bundle_t();
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
    
        cap_he_axi_aw_bundle_t aw;
    
        cap_he_axi_w_bundle_t w;
    
        cap_he_axi_b_bundle_t b;
    
}; // cap_he_axi_write_bundle_t
    
class cap_he_axi_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_he_axi_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_he_axi_bundle_t(string name = "cap_he_axi_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_he_axi_bundle_t();
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
    
        cap_he_axi_ar_bundle_t ar;
    
        cap_he_axi_r_bundle_t r;
    
        cap_he_axi_aw_bundle_t aw;
    
        cap_he_axi_w_bundle_t w;
    
        cap_he_axi_b_bundle_t b;
    
}; // cap_he_axi_bundle_t
    
class cap_te_axi_addr_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_axi_addr_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_axi_addr_decoder_t(string name = "cap_te_axi_addr_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_axi_addr_decoder_t();
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
    
        typedef pu_cpp_int< 21 > tbl_addr_cpp_int_t;
        cpp_int int_var__tbl_addr;
        void tbl_addr (const cpp_int  & l__val);
        cpp_int tbl_addr() const;
    
        typedef pu_cpp_int< 4 > tbl_id_cpp_int_t;
        cpp_int int_var__tbl_id;
        void tbl_id (const cpp_int  & l__val);
        cpp_int tbl_id() const;
    
        typedef pu_cpp_int< 14 > pktsize_cpp_int_t;
        cpp_int int_var__pktsize;
        void pktsize (const cpp_int  & l__val);
        cpp_int pktsize() const;
    
        typedef pu_cpp_int< 2 > color_bits_cpp_int_t;
        cpp_int int_var__color_bits;
        void color_bits (const cpp_int  & l__val);
        cpp_int color_bits() const;
    
        typedef pu_cpp_int< 23 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_te_axi_addr_decoder_t
    
#endif // CAP_AXI_DECODERS_H
        