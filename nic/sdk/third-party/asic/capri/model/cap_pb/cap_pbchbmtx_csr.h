
#ifndef CAP_PBCHBMTX_CSR_H
#define CAP_PBCHBMTX_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t(string name = "cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t();
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
    
        typedef pu_cpp_int< 2 > commmand_cpp_int_t;
        cpp_int int_var__commmand;
        void commmand (const cpp_int  & l__val);
        cpp_int commmand() const;
    
        typedef pu_cpp_int< 32 > credit_cpp_int_t;
        cpp_int int_var__credit;
        void credit (const cpp_int  & l__val);
        cpp_int credit() const;
    
        typedef pu_cpp_int< 32 > quota_cpp_int_t;
        cpp_int int_var__quota;
        void quota (const cpp_int  & l__val);
        cpp_int quota() const;
    
}; // cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t
    
class cap_pbchbmtx_csr_dhs_hbm_cdt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_dhs_hbm_cdt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_dhs_hbm_cdt_t(string name = "cap_pbchbmtx_csr_dhs_hbm_cdt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_dhs_hbm_cdt_t();
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
    
        cap_pbchbmtx_csr_dhs_hbm_cdt_entry_t entry;
    
}; // cap_pbchbmtx_csr_dhs_hbm_cdt_t
    
class cap_pbchbmtx_csr_cfg_hbm_arb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_arb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_arb_t(string name = "cap_pbchbmtx_csr_cfg_hbm_arb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_arb_t();
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
    
        typedef pu_cpp_int< 2 > strict_priority_cpp_int_t;
        cpp_int int_var__strict_priority;
        void strict_priority (const cpp_int  & l__val);
        cpp_int strict_priority() const;
    
        typedef pu_cpp_int< 1 > enable_wrr_cpp_int_t;
        cpp_int int_var__enable_wrr;
        void enable_wrr (const cpp_int  & l__val);
        cpp_int enable_wrr() const;
    
        typedef pu_cpp_int< 1 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_arb_t
    
class cap_pbchbmtx_csr_cfg_hbm_read_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_read_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_read_fifo_t(string name = "cap_pbchbmtx_csr_cfg_hbm_read_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_read_fifo_t();
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
    
        typedef pu_cpp_int< 8 > max_packets_cpp_int_t;
        cpp_int int_var__max_packets;
        void max_packets (const cpp_int  & l__val);
        cpp_int max_packets() const;
    
        typedef pu_cpp_int< 9 > max_flits_cpp_int_t;
        cpp_int int_var__max_flits;
        void max_flits (const cpp_int  & l__val);
        cpp_int max_flits() const;
    
        typedef pu_cpp_int< 12 > max_cells_cpp_int_t;
        cpp_int int_var__max_cells;
        void max_cells (const cpp_int  & l__val);
        cpp_int max_cells() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_read_fifo_t
    
class cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t(string name = "cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t();
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
    
        typedef pu_cpp_int< 304 > threshold_cpp_int_t;
        cpp_int int_var__threshold;
        void threshold (const cpp_int  & l__val);
        cpp_int threshold() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t
    
class cap_pbchbmtx_csr_cfg_hbm_tx_payload_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_tx_payload_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_tx_payload_t(string name = "cap_pbchbmtx_csr_cfg_hbm_tx_payload_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_tx_payload_t();
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
    
        typedef pu_cpp_int< 432 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 368 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_tx_payload_t
    
class cap_pbchbmtx_csr_cnt_hbm_cut_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cnt_hbm_cut_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cnt_hbm_cut_t(string name = "cap_pbchbmtx_csr_cnt_hbm_cut_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cnt_hbm_cut_t();
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
    
        typedef pu_cpp_int< 16 > thru_cpp_int_t;
        cpp_int int_var__thru;
        void thru (const cpp_int  & l__val);
        cpp_int thru() const;
    
}; // cap_pbchbmtx_csr_cnt_hbm_cut_t
    
class cap_pbchbmtx_csr_cfg_hbm_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_addr_t(string name = "cap_pbchbmtx_csr_cfg_hbm_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_addr_t();
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
    
        typedef pu_cpp_int< 1 > swizzle_cpp_int_t;
        cpp_int int_var__swizzle;
        void swizzle (const cpp_int  & l__val);
        cpp_int swizzle() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_addr_t
    
class cap_pbchbmtx_csr_cnt_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cnt_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cnt_hbm_t(string name = "cap_pbchbmtx_csr_cnt_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cnt_hbm_t();
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
    
        typedef pu_cpp_int< 32 > flits_sop_in_cpp_int_t;
        cpp_int int_var__flits_sop_in;
        void flits_sop_in (const cpp_int  & l__val);
        cpp_int flits_sop_in() const;
    
        typedef pu_cpp_int< 32 > flits_eop_in_cpp_int_t;
        cpp_int int_var__flits_eop_in;
        void flits_eop_in (const cpp_int  & l__val);
        cpp_int flits_eop_in() const;
    
        typedef pu_cpp_int< 32 > flits_sop_out_cpp_int_t;
        cpp_int int_var__flits_sop_out;
        void flits_sop_out (const cpp_int  & l__val);
        cpp_int flits_sop_out() const;
    
        typedef pu_cpp_int< 32 > flits_eop_out_cpp_int_t;
        cpp_int int_var__flits_eop_out;
        void flits_eop_out (const cpp_int  & l__val);
        cpp_int flits_eop_out() const;
    
        typedef pu_cpp_int< 32 > occupancy_stop_cpp_int_t;
        cpp_int int_var__occupancy_stop;
        void occupancy_stop (const cpp_int  & l__val);
        cpp_int occupancy_stop() const;
    
}; // cap_pbchbmtx_csr_cnt_hbm_t
    
class cap_pbchbmtx_csr_cfg_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_t(string name = "cap_pbchbmtx_csr_cfg_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_t();
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
    
        typedef pu_cpp_int< 4 > rate_limiter_cpp_int_t;
        cpp_int int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        cpp_int int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 7 > depth_shift_cpp_int_t;
        cpp_int int_var__depth_shift;
        void depth_shift (const cpp_int  & l__val);
        cpp_int depth_shift() const;
    
        typedef pu_cpp_int< 1 > max_pop_size_enable_cpp_int_t;
        cpp_int int_var__max_pop_size_enable;
        void max_pop_size_enable (const cpp_int  & l__val);
        cpp_int max_pop_size_enable() const;
    
        typedef pu_cpp_int< 1 > enable_multiple_reads_cpp_int_t;
        cpp_int int_var__enable_multiple_reads;
        void enable_multiple_reads (const cpp_int  & l__val);
        cpp_int enable_multiple_reads() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_t
    
class cap_pbchbmtx_csr_cfg_hbm_context_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_context_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_context_t(string name = "cap_pbchbmtx_csr_cfg_hbm_context_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_context_t();
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
    
        typedef pu_cpp_int< 27 > xoff_timeout_cpp_int_t;
        cpp_int int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 16 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 16 > no_drop_cpp_int_t;
        cpp_int int_var__no_drop;
        void no_drop (const cpp_int  & l__val);
        cpp_int no_drop() const;
    
        typedef pu_cpp_int< 5 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_context_t
    
class cap_pbchbmtx_csr_cfg_hbm_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_mem_t(string name = "cap_pbchbmtx_csr_cfg_hbm_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_mem_t();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 2 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 2 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 2 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_mem_t
    
class cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t(string name = "cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t();
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
    
        typedef pu_cpp_int< 64 > table_cpp_int_t;
        cpp_int int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t
    
class cap_pbchbmtx_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmtx_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmtx_csr_t(string name = "cap_pbchbmtx_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmtx_csr_t();
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
    
        cap_pbchbmtx_csr_cfg_hbm_tc_to_q_t cfg_hbm_tc_to_q;
    
        cap_pbchbmtx_csr_cfg_hbm_mem_t cfg_hbm_mem;
    
        cap_pbchbmtx_csr_cfg_hbm_context_t cfg_hbm_context;
    
        cap_pbchbmtx_csr_cfg_hbm_t cfg_hbm;
    
        cap_pbchbmtx_csr_cnt_hbm_t cnt_hbm;
    
        cap_pbchbmtx_csr_cfg_hbm_addr_t cfg_hbm_addr;
    
        cap_pbchbmtx_csr_cnt_hbm_cut_t cnt_hbm_cut;
    
        cap_pbchbmtx_csr_cfg_hbm_tx_payload_t cfg_hbm_tx_payload;
    
        cap_pbchbmtx_csr_cfg_hbm_tx_payload_occupancy_t cfg_hbm_tx_payload_occupancy;
    
        cap_pbchbmtx_csr_cfg_hbm_read_fifo_t cfg_hbm_read_fifo;
    
        cap_pbchbmtx_csr_cfg_hbm_arb_t cfg_hbm_arb;
    
        cap_pbchbmtx_csr_dhs_hbm_cdt_t dhs_hbm_cdt;
    
}; // cap_pbchbmtx_csr_t
    
#endif // CAP_PBCHBMTX_CSR_H
        