
#ifndef CAP_PBC_DECODERS_H
#define CAP_PBC_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbc_replicate_node32_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_replicate_node32_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_replicate_node32_t(string name = "cap_pbc_replicate_node32_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_replicate_node32_t();
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
    
        typedef pu_cpp_int < 32 > token_cpp_int_t;
        cpp_int int_var__token[15];
        int get_depth_token() { return 15; }
        void token ( const cpp_int & _val, int _idx);
        cpp_int token(int _idx) const;
    
        typedef pu_cpp_int< 1 > last_node_cpp_int_t;
        cpp_int int_var__last_node;
        void last_node (const cpp_int  & l__val);
        cpp_int last_node() const;
    
        typedef pu_cpp_int< 4 > token_cnt_cpp_int_t;
        cpp_int int_var__token_cnt;
        void token_cnt (const cpp_int  & l__val);
        cpp_int token_cnt() const;
    
        typedef pu_cpp_int< 27 > next_ptr_cpp_int_t;
        cpp_int int_var__next_ptr;
        void next_ptr (const cpp_int  & l__val);
        cpp_int next_ptr() const;
    
}; // cap_pbc_replicate_node32_t
    
class cap_pbc_replicate_node48_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_replicate_node48_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_replicate_node48_t(string name = "cap_pbc_replicate_node48_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_replicate_node48_t();
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
    
        typedef pu_cpp_int < 48 > token_cpp_int_t;
        cpp_int int_var__token[10];
        int get_depth_token() { return 10; }
        void token ( const cpp_int & _val, int _idx);
        cpp_int token(int _idx) const;
    
        typedef pu_cpp_int< 1 > last_node_cpp_int_t;
        cpp_int int_var__last_node;
        void last_node (const cpp_int  & l__val);
        cpp_int last_node() const;
    
        typedef pu_cpp_int< 4 > token_cnt_cpp_int_t;
        cpp_int int_var__token_cnt;
        void token_cnt (const cpp_int  & l__val);
        cpp_int token_cnt() const;
    
        typedef pu_cpp_int< 27 > next_ptr_cpp_int_t;
        cpp_int int_var__next_ptr;
        void next_ptr (const cpp_int  & l__val);
        cpp_int next_ptr() const;
    
}; // cap_pbc_replicate_node48_t
    
class cap_pbc_replicate_node64_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_replicate_node64_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_replicate_node64_t(string name = "cap_pbc_replicate_node64_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_replicate_node64_t();
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
    
        typedef pu_cpp_int < 64 > token_cpp_int_t;
        cpp_int int_var__token[7];
        int get_depth_token() { return 7; }
        void token ( const cpp_int & _val, int _idx);
        cpp_int token(int _idx) const;
    
        typedef pu_cpp_int< 32 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > last_node_cpp_int_t;
        cpp_int int_var__last_node;
        void last_node (const cpp_int  & l__val);
        cpp_int last_node() const;
    
        typedef pu_cpp_int< 4 > token_cnt_cpp_int_t;
        cpp_int int_var__token_cnt;
        void token_cnt (const cpp_int  & l__val);
        cpp_int token_cnt() const;
    
        typedef pu_cpp_int< 27 > next_ptr_cpp_int_t;
        cpp_int int_var__next_ptr;
        void next_ptr (const cpp_int  & l__val);
        cpp_int next_ptr() const;
    
}; // cap_pbc_replicate_node64_t
    
class cap_pbc_oq_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_oq_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_oq_map_t(string name = "cap_pbc_oq_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_oq_map_t();
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
    
        typedef pu_cpp_int < 5 > oqueue_cpp_int_t;
        cpp_int int_var__oqueue[8];
        int get_depth_oqueue() { return 8; }
        void oqueue ( const cpp_int & _val, int _idx);
        cpp_int oqueue(int _idx) const;
    
}; // cap_pbc_oq_map_t
    
class cap_pbc_pg8_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_pg8_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_pg8_map_t(string name = "cap_pbc_pg8_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_pg8_map_t();
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
    
        typedef pu_cpp_int < 3 > pg_cpp_int_t;
        cpp_int int_var__pg[8];
        int get_depth_pg() { return 8; }
        void pg ( const cpp_int & _val, int _idx);
        cpp_int pg(int _idx) const;
    
}; // cap_pbc_pg8_map_t
    
class cap_pbc_pg16_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_pg16_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_pg16_map_t(string name = "cap_pbc_pg16_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_pg16_map_t();
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
    
        typedef pu_cpp_int < 4 > pg_cpp_int_t;
        cpp_int int_var__pg[16];
        int get_depth_pg() { return 16; }
        void pg ( const cpp_int & _val, int _idx);
        cpp_int pg(int _idx) const;
    
}; // cap_pbc_pg16_map_t
    
class cap_pbc_pg32_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_pg32_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_pg32_map_t(string name = "cap_pbc_pg32_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_pg32_map_t();
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
    
        typedef pu_cpp_int < 5 > pg_cpp_int_t;
        cpp_int int_var__pg[32];
        int get_depth_pg() { return 32; }
        void pg ( const cpp_int & _val, int _idx);
        cpp_int pg(int _idx) const;
    
}; // cap_pbc_pg32_map_t
    
class cap_pbc_max_growth_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_max_growth_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_max_growth_map_t(string name = "cap_pbc_max_growth_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_max_growth_map_t();
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
    
        typedef pu_cpp_int < 5 > max_growth_cpp_int_t;
        cpp_int int_var__max_growth[32];
        int get_depth_max_growth() { return 32; }
        void max_growth ( const cpp_int & _val, int _idx);
        cpp_int max_growth(int _idx) const;
    
}; // cap_pbc_max_growth_map_t
    
class cap_pbc_hbm_eth_ctl_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_eth_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_eth_ctl_t(string name = "cap_pbc_hbm_eth_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_eth_ctl_t();
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
    
        typedef pu_cpp_int < 27 > base_cpp_int_t;
        cpp_int int_var__base[32];
        int get_depth_base() { return 32; }
        void base ( const cpp_int & _val, int _idx);
        cpp_int base(int _idx) const;
    
        typedef pu_cpp_int < 23 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz[32];
        int get_depth_mem_sz() { return 32; }
        void mem_sz ( const cpp_int & _val, int _idx);
        cpp_int mem_sz(int _idx) const;
    
}; // cap_pbc_hbm_eth_ctl_t
    
class cap_pbc_hbm_tx_ctl_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_tx_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_tx_ctl_t(string name = "cap_pbc_hbm_tx_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_tx_ctl_t();
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
    
        typedef pu_cpp_int < 27 > base_cpp_int_t;
        cpp_int int_var__base[16];
        int get_depth_base() { return 16; }
        void base ( const cpp_int & _val, int _idx);
        cpp_int base(int _idx) const;
    
        typedef pu_cpp_int < 23 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz[16];
        int get_depth_mem_sz() { return 16; }
        void mem_sz ( const cpp_int & _val, int _idx);
        cpp_int mem_sz(int _idx) const;
    
}; // cap_pbc_hbm_tx_ctl_t
    
class cap_pbc_hbm_eth_port_ctl_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_eth_port_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_eth_port_ctl_t(string name = "cap_pbc_hbm_eth_port_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_eth_port_ctl_t();
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
    
        typedef pu_cpp_int < 27 > base_cpp_int_t;
        cpp_int int_var__base[8];
        int get_depth_base() { return 8; }
        void base ( const cpp_int & _val, int _idx);
        cpp_int base(int _idx) const;
    
        typedef pu_cpp_int < 23 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz[8];
        int get_depth_mem_sz() { return 8; }
        void mem_sz ( const cpp_int & _val, int _idx);
        cpp_int mem_sz(int _idx) const;
    
}; // cap_pbc_hbm_eth_port_ctl_t
    
class cap_pbc_hbm_eth_occ_thr_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_eth_occ_thr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_eth_occ_thr_t(string name = "cap_pbc_hbm_eth_occ_thr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_eth_occ_thr_t();
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
    
        typedef pu_cpp_int < 19 > thr_cpp_int_t;
        cpp_int int_var__thr[8];
        int get_depth_thr() { return 8; }
        void thr ( const cpp_int & _val, int _idx);
        cpp_int thr(int _idx) const;
    
}; // cap_pbc_hbm_eth_occ_thr_t
    
class cap_pbc_hbm_tx_occ_thr_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_tx_occ_thr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_tx_occ_thr_t(string name = "cap_pbc_hbm_tx_occ_thr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_tx_occ_thr_t();
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
    
        typedef pu_cpp_int < 19 > thr_cpp_int_t;
        cpp_int int_var__thr[16];
        int get_depth_thr() { return 16; }
        void thr ( const cpp_int & _val, int _idx);
        cpp_int thr(int _idx) const;
    
}; // cap_pbc_hbm_tx_occ_thr_t
    
class cap_pbc_hbm_eth_xoff_thr_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_eth_xoff_thr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_eth_xoff_thr_t(string name = "cap_pbc_hbm_eth_xoff_thr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_eth_xoff_thr_t();
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
    
        typedef pu_cpp_int < 20 > xoff_cpp_int_t;
        cpp_int int_var__xoff[32];
        int get_depth_xoff() { return 32; }
        void xoff ( const cpp_int & _val, int _idx);
        cpp_int xoff(int _idx) const;
    
        typedef pu_cpp_int < 20 > xon_cpp_int_t;
        cpp_int int_var__xon[32];
        int get_depth_xon() { return 32; }
        void xon ( const cpp_int & _val, int _idx);
        cpp_int xon(int _idx) const;
    
}; // cap_pbc_hbm_eth_xoff_thr_t
    
class cap_pbc_hbm_eth_hdr_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_hbm_eth_hdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_hbm_eth_hdr_t(string name = "cap_pbc_hbm_eth_hdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_hbm_eth_hdr_t();
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
    
        typedef pu_cpp_int< 14 > hbm_q_depth_cpp_int_t;
        cpp_int int_var__hbm_q_depth;
        void hbm_q_depth (const cpp_int  & l__val);
        cpp_int hbm_q_depth() const;
    
        typedef pu_cpp_int< 48 > hbm_timestamp_cpp_int_t;
        cpp_int int_var__hbm_timestamp;
        void hbm_timestamp (const cpp_int  & l__val);
        cpp_int hbm_timestamp() const;
    
        typedef pu_cpp_int< 3 > hbm_qnum_cpp_int_t;
        cpp_int int_var__hbm_qnum;
        void hbm_qnum (const cpp_int  & l__val);
        cpp_int hbm_qnum() const;
    
        typedef pu_cpp_int< 1 > hbm_q_depth_vld_cpp_int_t;
        cpp_int int_var__hbm_q_depth_vld;
        void hbm_q_depth_vld (const cpp_int  & l__val);
        cpp_int hbm_q_depth_vld() const;
    
        typedef pu_cpp_int< 6 > hbm_spare_cpp_int_t;
        cpp_int int_var__hbm_spare;
        void hbm_spare (const cpp_int  & l__val);
        cpp_int hbm_spare() const;
    
}; // cap_pbc_hbm_eth_hdr_t
    
class cap_pbc_egr_tstamp_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_egr_tstamp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_egr_tstamp_t(string name = "cap_pbc_egr_tstamp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_egr_tstamp_t();
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
    
        typedef pu_cpp_int < 27 > base_cpp_int_t;
        cpp_int int_var__base[9];
        int get_depth_base() { return 9; }
        void base ( const cpp_int & _val, int _idx);
        cpp_int base(int _idx) const;
    
}; // cap_pbc_egr_tstamp_t
    
class cap_pbc_eth_oq_xoff_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_eth_oq_xoff_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_eth_oq_xoff_map_t(string name = "cap_pbc_eth_oq_xoff_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_eth_oq_xoff_map_t();
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
    
        typedef pu_cpp_int < 3 > xoff_pri_cpp_int_t;
        cpp_int int_var__xoff_pri[32];
        int get_depth_xoff_pri() { return 32; }
        void xoff_pri ( const cpp_int & _val, int _idx);
        cpp_int xoff_pri(int _idx) const;
    
}; // cap_pbc_eth_oq_xoff_map_t
    
class cap_pbc_p4_oq_xoff_map_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_p4_oq_xoff_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_p4_oq_xoff_map_t(string name = "cap_pbc_p4_oq_xoff_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_p4_oq_xoff_map_t();
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
    
        typedef pu_cpp_int < 5 > xoff_pri_cpp_int_t;
        cpp_int int_var__xoff_pri[32];
        int get_depth_xoff_pri() { return 32; }
        void xoff_pri ( const cpp_int & _val, int _idx);
        cpp_int xoff_pri(int _idx) const;
    
}; // cap_pbc_p4_oq_xoff_map_t
    
#endif // CAP_PBC_DECODERS_H
        