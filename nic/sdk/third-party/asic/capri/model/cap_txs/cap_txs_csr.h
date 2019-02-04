
#ifndef CAP_TXS_CSR_H
#define CAP_TXS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_txs_csr_int_tmr_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_tmr_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_tmr_int_enable_clear_t(string name = "cap_txs_csr_int_tmr_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_tmr_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > tmr_wid_empty_enable_cpp_int_t;
        cpp_int int_var__tmr_wid_empty_enable;
        void tmr_wid_empty_enable (const cpp_int  & l__val);
        cpp_int tmr_wid_empty_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_rid_empty_enable_cpp_int_t;
        cpp_int int_var__tmr_rid_empty_enable;
        void tmr_rid_empty_enable (const cpp_int  & l__val);
        cpp_int tmr_rid_empty_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_wr_txfifo_ovf_enable_cpp_int_t;
        cpp_int int_var__tmr_wr_txfifo_ovf_enable;
        void tmr_wr_txfifo_ovf_enable (const cpp_int  & l__val);
        cpp_int tmr_wr_txfifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_rd_txfifo_ovf_enable_cpp_int_t;
        cpp_int int_var__tmr_rd_txfifo_ovf_enable;
        void tmr_rd_txfifo_ovf_enable (const cpp_int  & l__val);
        cpp_int tmr_rd_txfifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_state_fifo_ovf_enable_cpp_int_t;
        cpp_int int_var__tmr_state_fifo_ovf_enable;
        void tmr_state_fifo_ovf_enable (const cpp_int  & l__val);
        cpp_int tmr_state_fifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_byp_ovf_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_byp_ovf_enable;
        void tmr_hbm_byp_ovf_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_byp_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_byp_wtag_wrap_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_byp_wtag_wrap_enable;
        void tmr_hbm_byp_wtag_wrap_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_byp_wtag_wrap_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_ctime_wrap_enable_cpp_int_t;
        cpp_int int_var__ftmr_ctime_wrap_enable;
        void ftmr_ctime_wrap_enable (const cpp_int  & l__val);
        cpp_int ftmr_ctime_wrap_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_ctime_wrap_enable_cpp_int_t;
        cpp_int int_var__stmr_ctime_wrap_enable;
        void stmr_ctime_wrap_enable (const cpp_int  & l__val);
        cpp_int stmr_ctime_wrap_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_push_out_of_wheel_enable_cpp_int_t;
        cpp_int int_var__stmr_push_out_of_wheel_enable;
        void stmr_push_out_of_wheel_enable (const cpp_int  & l__val);
        cpp_int stmr_push_out_of_wheel_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_push_out_of_wheel_enable_cpp_int_t;
        cpp_int int_var__ftmr_push_out_of_wheel_enable;
        void ftmr_push_out_of_wheel_enable (const cpp_int  & l__val);
        cpp_int ftmr_push_out_of_wheel_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_key_not_push_enable_cpp_int_t;
        cpp_int int_var__ftmr_key_not_push_enable;
        void ftmr_key_not_push_enable (const cpp_int  & l__val);
        cpp_int ftmr_key_not_push_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_key_not_found_enable_cpp_int_t;
        cpp_int int_var__ftmr_key_not_found_enable;
        void ftmr_key_not_found_enable (const cpp_int  & l__val);
        cpp_int ftmr_key_not_found_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_key_not_push_enable_cpp_int_t;
        cpp_int int_var__stmr_key_not_push_enable;
        void stmr_key_not_push_enable (const cpp_int  & l__val);
        cpp_int stmr_key_not_push_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_key_not_found_enable_cpp_int_t;
        cpp_int int_var__stmr_key_not_found_enable;
        void stmr_key_not_found_enable (const cpp_int  & l__val);
        cpp_int stmr_key_not_found_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_stall_enable_cpp_int_t;
        cpp_int int_var__stmr_stall_enable;
        void stmr_stall_enable (const cpp_int  & l__val);
        cpp_int stmr_stall_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_stall_enable_cpp_int_t;
        cpp_int int_var__ftmr_stall_enable;
        void ftmr_stall_enable (const cpp_int  & l__val);
        cpp_int ftmr_stall_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_fifo_ovf_enable_cpp_int_t;
        cpp_int int_var__ftmr_fifo_ovf_enable;
        void ftmr_fifo_ovf_enable (const cpp_int  & l__val);
        cpp_int ftmr_fifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_fifo_ovf_enable_cpp_int_t;
        cpp_int int_var__stmr_fifo_ovf_enable;
        void stmr_fifo_ovf_enable (const cpp_int  & l__val);
        cpp_int stmr_fifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_fifo_efull_enable_cpp_int_t;
        cpp_int int_var__ftmr_fifo_efull_enable;
        void ftmr_fifo_efull_enable (const cpp_int  & l__val);
        cpp_int ftmr_fifo_efull_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_fifo_efull_enable_cpp_int_t;
        cpp_int int_var__stmr_fifo_efull_enable;
        void stmr_fifo_efull_enable (const cpp_int  & l__val);
        cpp_int stmr_fifo_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_rejct_drb_ovf_enable_cpp_int_t;
        cpp_int int_var__tmr_rejct_drb_ovf_enable;
        void tmr_rejct_drb_ovf_enable (const cpp_int  & l__val);
        cpp_int tmr_rejct_drb_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_rejct_drb_efull_enable_cpp_int_t;
        cpp_int int_var__tmr_rejct_drb_efull_enable;
        void tmr_rejct_drb_efull_enable (const cpp_int  & l__val);
        cpp_int tmr_rejct_drb_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_re_efull_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_re_efull_enable;
        void tmr_hbm_re_efull_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_re_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_we_efull_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_we_efull_enable;
        void tmr_hbm_we_efull_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_we_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_byp_pending_efull_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_byp_pending_efull_enable;
        void tmr_hbm_byp_pending_efull_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_byp_pending_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_rd_pending_efull_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_rd_pending_efull_enable;
        void tmr_hbm_rd_pending_efull_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_rd_pending_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_wr_pending_efull_enable_cpp_int_t;
        cpp_int int_var__tmr_hbm_wr_pending_efull_enable;
        void tmr_hbm_wr_pending_efull_enable (const cpp_int  & l__val);
        cpp_int tmr_hbm_wr_pending_efull_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_bresp_err_enable_cpp_int_t;
        cpp_int int_var__tmr_bresp_err_enable;
        void tmr_bresp_err_enable (const cpp_int  & l__val);
        cpp_int tmr_bresp_err_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_bid_err_enable_cpp_int_t;
        cpp_int int_var__tmr_bid_err_enable;
        void tmr_bid_err_enable (const cpp_int  & l__val);
        cpp_int tmr_bid_err_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_rresp_err_enable_cpp_int_t;
        cpp_int int_var__tmr_rresp_err_enable;
        void tmr_rresp_err_enable (const cpp_int  & l__val);
        cpp_int tmr_rresp_err_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_rid_err_enable_cpp_int_t;
        cpp_int int_var__tmr_rid_err_enable;
        void tmr_rid_err_enable (const cpp_int  & l__val);
        cpp_int tmr_rid_err_enable() const;
    
}; // cap_txs_csr_int_tmr_int_enable_clear_t
    
class cap_txs_csr_int_tmr_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_tmr_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_tmr_int_test_set_t(string name = "cap_txs_csr_int_tmr_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_tmr_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > tmr_wid_empty_interrupt_cpp_int_t;
        cpp_int int_var__tmr_wid_empty_interrupt;
        void tmr_wid_empty_interrupt (const cpp_int  & l__val);
        cpp_int tmr_wid_empty_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_rid_empty_interrupt_cpp_int_t;
        cpp_int int_var__tmr_rid_empty_interrupt;
        void tmr_rid_empty_interrupt (const cpp_int  & l__val);
        cpp_int tmr_rid_empty_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_wr_txfifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__tmr_wr_txfifo_ovf_interrupt;
        void tmr_wr_txfifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int tmr_wr_txfifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_rd_txfifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__tmr_rd_txfifo_ovf_interrupt;
        void tmr_rd_txfifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int tmr_rd_txfifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_state_fifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__tmr_state_fifo_ovf_interrupt;
        void tmr_state_fifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int tmr_state_fifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_byp_ovf_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_byp_ovf_interrupt;
        void tmr_hbm_byp_ovf_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_byp_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_byp_wtag_wrap_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_byp_wtag_wrap_interrupt;
        void tmr_hbm_byp_wtag_wrap_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_byp_wtag_wrap_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_ctime_wrap_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_ctime_wrap_interrupt;
        void ftmr_ctime_wrap_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_ctime_wrap_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_ctime_wrap_interrupt_cpp_int_t;
        cpp_int int_var__stmr_ctime_wrap_interrupt;
        void stmr_ctime_wrap_interrupt (const cpp_int  & l__val);
        cpp_int stmr_ctime_wrap_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_push_out_of_wheel_interrupt_cpp_int_t;
        cpp_int int_var__stmr_push_out_of_wheel_interrupt;
        void stmr_push_out_of_wheel_interrupt (const cpp_int  & l__val);
        cpp_int stmr_push_out_of_wheel_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_push_out_of_wheel_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_push_out_of_wheel_interrupt;
        void ftmr_push_out_of_wheel_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_push_out_of_wheel_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_key_not_push_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_key_not_push_interrupt;
        void ftmr_key_not_push_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_key_not_push_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_key_not_found_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_key_not_found_interrupt;
        void ftmr_key_not_found_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_key_not_found_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_key_not_push_interrupt_cpp_int_t;
        cpp_int int_var__stmr_key_not_push_interrupt;
        void stmr_key_not_push_interrupt (const cpp_int  & l__val);
        cpp_int stmr_key_not_push_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_key_not_found_interrupt_cpp_int_t;
        cpp_int int_var__stmr_key_not_found_interrupt;
        void stmr_key_not_found_interrupt (const cpp_int  & l__val);
        cpp_int stmr_key_not_found_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_stall_interrupt_cpp_int_t;
        cpp_int int_var__stmr_stall_interrupt;
        void stmr_stall_interrupt (const cpp_int  & l__val);
        cpp_int stmr_stall_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_stall_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_stall_interrupt;
        void ftmr_stall_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_stall_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_fifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_fifo_ovf_interrupt;
        void ftmr_fifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_fifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_fifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__stmr_fifo_ovf_interrupt;
        void stmr_fifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int stmr_fifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > ftmr_fifo_efull_interrupt_cpp_int_t;
        cpp_int int_var__ftmr_fifo_efull_interrupt;
        void ftmr_fifo_efull_interrupt (const cpp_int  & l__val);
        cpp_int ftmr_fifo_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > stmr_fifo_efull_interrupt_cpp_int_t;
        cpp_int int_var__stmr_fifo_efull_interrupt;
        void stmr_fifo_efull_interrupt (const cpp_int  & l__val);
        cpp_int stmr_fifo_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_rejct_drb_ovf_interrupt_cpp_int_t;
        cpp_int int_var__tmr_rejct_drb_ovf_interrupt;
        void tmr_rejct_drb_ovf_interrupt (const cpp_int  & l__val);
        cpp_int tmr_rejct_drb_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_rejct_drb_efull_interrupt_cpp_int_t;
        cpp_int int_var__tmr_rejct_drb_efull_interrupt;
        void tmr_rejct_drb_efull_interrupt (const cpp_int  & l__val);
        cpp_int tmr_rejct_drb_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_re_efull_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_re_efull_interrupt;
        void tmr_hbm_re_efull_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_re_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_we_efull_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_we_efull_interrupt;
        void tmr_hbm_we_efull_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_we_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_byp_pending_efull_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_byp_pending_efull_interrupt;
        void tmr_hbm_byp_pending_efull_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_byp_pending_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_rd_pending_efull_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_rd_pending_efull_interrupt;
        void tmr_hbm_rd_pending_efull_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_rd_pending_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_hbm_wr_pending_efull_interrupt_cpp_int_t;
        cpp_int int_var__tmr_hbm_wr_pending_efull_interrupt;
        void tmr_hbm_wr_pending_efull_interrupt (const cpp_int  & l__val);
        cpp_int tmr_hbm_wr_pending_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_bresp_err_interrupt_cpp_int_t;
        cpp_int int_var__tmr_bresp_err_interrupt;
        void tmr_bresp_err_interrupt (const cpp_int  & l__val);
        cpp_int tmr_bresp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_bid_err_interrupt_cpp_int_t;
        cpp_int int_var__tmr_bid_err_interrupt;
        void tmr_bid_err_interrupt (const cpp_int  & l__val);
        cpp_int tmr_bid_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_rresp_err_interrupt_cpp_int_t;
        cpp_int int_var__tmr_rresp_err_interrupt;
        void tmr_rresp_err_interrupt (const cpp_int  & l__val);
        cpp_int tmr_rresp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_rid_err_interrupt_cpp_int_t;
        cpp_int int_var__tmr_rid_err_interrupt;
        void tmr_rid_err_interrupt (const cpp_int  & l__val);
        cpp_int tmr_rid_err_interrupt() const;
    
}; // cap_txs_csr_int_tmr_int_test_set_t
    
class cap_txs_csr_int_tmr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_tmr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_tmr_t(string name = "cap_txs_csr_int_tmr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_tmr_t();
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
    
        cap_txs_csr_int_tmr_int_test_set_t intreg;
    
        cap_txs_csr_int_tmr_int_test_set_t int_test_set;
    
        cap_txs_csr_int_tmr_int_enable_clear_t int_enable_set;
    
        cap_txs_csr_int_tmr_int_enable_clear_t int_enable_clear;
    
}; // cap_txs_csr_int_tmr_t
    
class cap_txs_csr_int_sch_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_sch_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_sch_int_enable_clear_t(string name = "cap_txs_csr_int_sch_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_sch_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > sch_wid_empty_enable_cpp_int_t;
        cpp_int int_var__sch_wid_empty_enable;
        void sch_wid_empty_enable (const cpp_int  & l__val);
        cpp_int sch_wid_empty_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rid_empty_enable_cpp_int_t;
        cpp_int int_var__sch_rid_empty_enable;
        void sch_rid_empty_enable (const cpp_int  & l__val);
        cpp_int sch_rid_empty_enable() const;
    
        typedef pu_cpp_int< 1 > sch_wr_txfifo_ovf_enable_cpp_int_t;
        cpp_int int_var__sch_wr_txfifo_ovf_enable;
        void sch_wr_txfifo_ovf_enable (const cpp_int  & l__val);
        cpp_int sch_wr_txfifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rd_txfifo_ovf_enable_cpp_int_t;
        cpp_int int_var__sch_rd_txfifo_ovf_enable;
        void sch_rd_txfifo_ovf_enable (const cpp_int  & l__val);
        cpp_int sch_rd_txfifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > sch_state_fifo_ovf_enable_cpp_int_t;
        cpp_int int_var__sch_state_fifo_ovf_enable;
        void sch_state_fifo_ovf_enable (const cpp_int  & l__val);
        cpp_int sch_state_fifo_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > sch_drb_cnt_qid_fifo_enable_cpp_int_t;
        cpp_int int_var__sch_drb_cnt_qid_fifo_enable;
        void sch_drb_cnt_qid_fifo_enable (const cpp_int  & l__val);
        cpp_int sch_drb_cnt_qid_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_byp_ovf_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_byp_ovf_enable;
        void sch_hbm_byp_ovf_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_byp_ovf_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_byp_wtag_wrap_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_byp_wtag_wrap_enable;
        void sch_hbm_byp_wtag_wrap_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_byp_wtag_wrap_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_ovfl_enable_cpp_int_t;
        cpp_int int_var__sch_rlid_ovfl_enable;
        void sch_rlid_ovfl_enable (const cpp_int  & l__val);
        cpp_int sch_rlid_ovfl_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_unfl_enable_cpp_int_t;
        cpp_int int_var__sch_rlid_unfl_enable;
        void sch_rlid_unfl_enable (const cpp_int  & l__val);
        cpp_int sch_rlid_unfl_enable() const;
    
        typedef pu_cpp_int< 1 > sch_null_lif_enable_cpp_int_t;
        cpp_int int_var__sch_null_lif_enable;
        void sch_null_lif_enable (const cpp_int  & l__val);
        cpp_int sch_null_lif_enable() const;
    
        typedef pu_cpp_int< 1 > sch_lif_sg_mismatch_enable_cpp_int_t;
        cpp_int int_var__sch_lif_sg_mismatch_enable;
        void sch_lif_sg_mismatch_enable (const cpp_int  & l__val);
        cpp_int sch_lif_sg_mismatch_enable() const;
    
        typedef pu_cpp_int< 1 > sch_aclr_hbm_ln_rollovr_enable_cpp_int_t;
        cpp_int int_var__sch_aclr_hbm_ln_rollovr_enable;
        void sch_aclr_hbm_ln_rollovr_enable (const cpp_int  & l__val);
        cpp_int sch_aclr_hbm_ln_rollovr_enable() const;
    
        typedef pu_cpp_int< 1 > sch_txdma_msg_efull_enable_cpp_int_t;
        cpp_int int_var__sch_txdma_msg_efull_enable;
        void sch_txdma_msg_efull_enable (const cpp_int  & l__val);
        cpp_int sch_txdma_msg_efull_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_re_efull_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_re_efull_enable;
        void sch_hbm_re_efull_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_re_efull_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_we_efull_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_we_efull_enable;
        void sch_hbm_we_efull_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_we_efull_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_byp_pending_efull_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_byp_pending_efull_enable;
        void sch_hbm_byp_pending_efull_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_byp_pending_efull_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_rd_pending_efull_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_rd_pending_efull_enable;
        void sch_hbm_rd_pending_efull_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_rd_pending_efull_enable() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_wr_pending_efull_enable_cpp_int_t;
        cpp_int int_var__sch_hbm_wr_pending_efull_enable;
        void sch_hbm_wr_pending_efull_enable (const cpp_int  & l__val);
        cpp_int sch_hbm_wr_pending_efull_enable() const;
    
        typedef pu_cpp_int< 1 > sch_bresp_err_enable_cpp_int_t;
        cpp_int int_var__sch_bresp_err_enable;
        void sch_bresp_err_enable (const cpp_int  & l__val);
        cpp_int sch_bresp_err_enable() const;
    
        typedef pu_cpp_int< 1 > sch_bid_err_enable_cpp_int_t;
        cpp_int int_var__sch_bid_err_enable;
        void sch_bid_err_enable (const cpp_int  & l__val);
        cpp_int sch_bid_err_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rresp_err_enable_cpp_int_t;
        cpp_int int_var__sch_rresp_err_enable;
        void sch_rresp_err_enable (const cpp_int  & l__val);
        cpp_int sch_rresp_err_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rid_err_enable_cpp_int_t;
        cpp_int int_var__sch_rid_err_enable;
        void sch_rid_err_enable (const cpp_int  & l__val);
        cpp_int sch_rid_err_enable() const;
    
        typedef pu_cpp_int< 1 > sch_drb_cnt_ovfl_enable_cpp_int_t;
        cpp_int int_var__sch_drb_cnt_ovfl_enable;
        void sch_drb_cnt_ovfl_enable (const cpp_int  & l__val);
        cpp_int sch_drb_cnt_ovfl_enable() const;
    
        typedef pu_cpp_int< 1 > sch_drb_cnt_unfl_enable_cpp_int_t;
        cpp_int int_var__sch_drb_cnt_unfl_enable;
        void sch_drb_cnt_unfl_enable (const cpp_int  & l__val);
        cpp_int sch_drb_cnt_unfl_enable() const;
    
        typedef pu_cpp_int< 1 > sch_txdma_msg_ovfl_enable_cpp_int_t;
        cpp_int int_var__sch_txdma_msg_ovfl_enable;
        void sch_txdma_msg_ovfl_enable (const cpp_int  & l__val);
        cpp_int sch_txdma_msg_ovfl_enable() const;
    
}; // cap_txs_csr_int_sch_int_enable_clear_t
    
class cap_txs_csr_int_sch_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_sch_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_sch_int_test_set_t(string name = "cap_txs_csr_int_sch_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_sch_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > sch_wid_empty_interrupt_cpp_int_t;
        cpp_int int_var__sch_wid_empty_interrupt;
        void sch_wid_empty_interrupt (const cpp_int  & l__val);
        cpp_int sch_wid_empty_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rid_empty_interrupt_cpp_int_t;
        cpp_int int_var__sch_rid_empty_interrupt;
        void sch_rid_empty_interrupt (const cpp_int  & l__val);
        cpp_int sch_rid_empty_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_wr_txfifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__sch_wr_txfifo_ovf_interrupt;
        void sch_wr_txfifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int sch_wr_txfifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rd_txfifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__sch_rd_txfifo_ovf_interrupt;
        void sch_rd_txfifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int sch_rd_txfifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_state_fifo_ovf_interrupt_cpp_int_t;
        cpp_int int_var__sch_state_fifo_ovf_interrupt;
        void sch_state_fifo_ovf_interrupt (const cpp_int  & l__val);
        cpp_int sch_state_fifo_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_drb_cnt_qid_fifo_interrupt_cpp_int_t;
        cpp_int int_var__sch_drb_cnt_qid_fifo_interrupt;
        void sch_drb_cnt_qid_fifo_interrupt (const cpp_int  & l__val);
        cpp_int sch_drb_cnt_qid_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_byp_ovf_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_byp_ovf_interrupt;
        void sch_hbm_byp_ovf_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_byp_ovf_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_byp_wtag_wrap_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_byp_wtag_wrap_interrupt;
        void sch_hbm_byp_wtag_wrap_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_byp_wtag_wrap_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_ovfl_interrupt_cpp_int_t;
        cpp_int int_var__sch_rlid_ovfl_interrupt;
        void sch_rlid_ovfl_interrupt (const cpp_int  & l__val);
        cpp_int sch_rlid_ovfl_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_unfl_interrupt_cpp_int_t;
        cpp_int int_var__sch_rlid_unfl_interrupt;
        void sch_rlid_unfl_interrupt (const cpp_int  & l__val);
        cpp_int sch_rlid_unfl_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_null_lif_interrupt_cpp_int_t;
        cpp_int int_var__sch_null_lif_interrupt;
        void sch_null_lif_interrupt (const cpp_int  & l__val);
        cpp_int sch_null_lif_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_lif_sg_mismatch_interrupt_cpp_int_t;
        cpp_int int_var__sch_lif_sg_mismatch_interrupt;
        void sch_lif_sg_mismatch_interrupt (const cpp_int  & l__val);
        cpp_int sch_lif_sg_mismatch_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_aclr_hbm_ln_rollovr_interrupt_cpp_int_t;
        cpp_int int_var__sch_aclr_hbm_ln_rollovr_interrupt;
        void sch_aclr_hbm_ln_rollovr_interrupt (const cpp_int  & l__val);
        cpp_int sch_aclr_hbm_ln_rollovr_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_txdma_msg_efull_interrupt_cpp_int_t;
        cpp_int int_var__sch_txdma_msg_efull_interrupt;
        void sch_txdma_msg_efull_interrupt (const cpp_int  & l__val);
        cpp_int sch_txdma_msg_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_re_efull_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_re_efull_interrupt;
        void sch_hbm_re_efull_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_re_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_we_efull_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_we_efull_interrupt;
        void sch_hbm_we_efull_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_we_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_byp_pending_efull_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_byp_pending_efull_interrupt;
        void sch_hbm_byp_pending_efull_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_byp_pending_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_rd_pending_efull_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_rd_pending_efull_interrupt;
        void sch_hbm_rd_pending_efull_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_rd_pending_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_hbm_wr_pending_efull_interrupt_cpp_int_t;
        cpp_int int_var__sch_hbm_wr_pending_efull_interrupt;
        void sch_hbm_wr_pending_efull_interrupt (const cpp_int  & l__val);
        cpp_int sch_hbm_wr_pending_efull_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_bresp_err_interrupt_cpp_int_t;
        cpp_int int_var__sch_bresp_err_interrupt;
        void sch_bresp_err_interrupt (const cpp_int  & l__val);
        cpp_int sch_bresp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_bid_err_interrupt_cpp_int_t;
        cpp_int int_var__sch_bid_err_interrupt;
        void sch_bid_err_interrupt (const cpp_int  & l__val);
        cpp_int sch_bid_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rresp_err_interrupt_cpp_int_t;
        cpp_int int_var__sch_rresp_err_interrupt;
        void sch_rresp_err_interrupt (const cpp_int  & l__val);
        cpp_int sch_rresp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rid_err_interrupt_cpp_int_t;
        cpp_int int_var__sch_rid_err_interrupt;
        void sch_rid_err_interrupt (const cpp_int  & l__val);
        cpp_int sch_rid_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_drb_cnt_ovfl_interrupt_cpp_int_t;
        cpp_int int_var__sch_drb_cnt_ovfl_interrupt;
        void sch_drb_cnt_ovfl_interrupt (const cpp_int  & l__val);
        cpp_int sch_drb_cnt_ovfl_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_drb_cnt_unfl_interrupt_cpp_int_t;
        cpp_int int_var__sch_drb_cnt_unfl_interrupt;
        void sch_drb_cnt_unfl_interrupt (const cpp_int  & l__val);
        cpp_int sch_drb_cnt_unfl_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_txdma_msg_ovfl_interrupt_cpp_int_t;
        cpp_int int_var__sch_txdma_msg_ovfl_interrupt;
        void sch_txdma_msg_ovfl_interrupt (const cpp_int  & l__val);
        cpp_int sch_txdma_msg_ovfl_interrupt() const;
    
}; // cap_txs_csr_int_sch_int_test_set_t
    
class cap_txs_csr_int_sch_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_sch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_sch_t(string name = "cap_txs_csr_int_sch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_sch_t();
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
    
        cap_txs_csr_int_sch_int_test_set_t intreg;
    
        cap_txs_csr_int_sch_int_test_set_t int_test_set;
    
        cap_txs_csr_int_sch_int_enable_clear_t int_enable_set;
    
        cap_txs_csr_int_sch_int_enable_clear_t int_enable_clear;
    
}; // cap_txs_csr_int_sch_t
    
class cap_txs_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_intreg_status_t(string name = "cap_txs_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_sch_interrupt_cpp_int_t;
        cpp_int int_var__int_sch_interrupt;
        void int_sch_interrupt (const cpp_int  & l__val);
        cpp_int int_sch_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_tmr_interrupt_cpp_int_t;
        cpp_int int_var__int_tmr_interrupt;
        void int_tmr_interrupt (const cpp_int  & l__val);
        cpp_int int_tmr_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_srams_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_srams_ecc_interrupt;
        void int_srams_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_srams_ecc_interrupt() const;
    
}; // cap_txs_csr_intreg_status_t
    
class cap_txs_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_groups_int_enable_rw_reg_t(string name = "cap_txs_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_sch_enable_cpp_int_t;
        cpp_int int_var__int_sch_enable;
        void int_sch_enable (const cpp_int  & l__val);
        cpp_int int_sch_enable() const;
    
        typedef pu_cpp_int< 1 > int_tmr_enable_cpp_int_t;
        cpp_int int_var__int_tmr_enable;
        void int_tmr_enable (const cpp_int  & l__val);
        cpp_int int_tmr_enable() const;
    
        typedef pu_cpp_int< 1 > int_srams_ecc_enable_cpp_int_t;
        cpp_int int_var__int_srams_ecc_enable;
        void int_srams_ecc_enable (const cpp_int  & l__val);
        cpp_int int_srams_ecc_enable() const;
    
}; // cap_txs_csr_int_groups_int_enable_rw_reg_t
    
class cap_txs_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_intgrp_status_t(string name = "cap_txs_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_intgrp_status_t();
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
    
        cap_txs_csr_intreg_status_t intreg;
    
        cap_txs_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_txs_csr_intreg_status_t int_rw_reg;
    
}; // cap_txs_csr_intgrp_status_t
    
class cap_txs_csr_int_srams_ecc_int_enable_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_srams_ecc_int_enable_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_srams_ecc_int_enable_set_t(string name = "cap_txs_csr_int_srams_ecc_int_enable_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_srams_ecc_int_enable_set_t();
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
    
        typedef pu_cpp_int< 1 > tmr_cnt_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__tmr_cnt_uncorrectable_enable;
        void tmr_cnt_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int tmr_cnt_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > tmr_cnt_correctable_enable_cpp_int_t;
        cpp_int int_var__tmr_cnt_correctable_enable;
        void tmr_cnt_correctable_enable (const cpp_int  & l__val);
        cpp_int tmr_cnt_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > sch_lif_map_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__sch_lif_map_uncorrectable_enable;
        void sch_lif_map_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int sch_lif_map_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > sch_lif_map_correctable_enable_cpp_int_t;
        cpp_int int_var__sch_lif_map_correctable_enable;
        void sch_lif_map_correctable_enable (const cpp_int  & l__val);
        cpp_int sch_lif_map_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_map_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__sch_rlid_map_uncorrectable_enable;
        void sch_rlid_map_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int sch_rlid_map_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_map_correctable_enable_cpp_int_t;
        cpp_int int_var__sch_rlid_map_correctable_enable;
        void sch_rlid_map_correctable_enable (const cpp_int  & l__val);
        cpp_int sch_rlid_map_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > sch_grp_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__sch_grp_uncorrectable_enable;
        void sch_grp_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int sch_grp_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > sch_grp_correctable_enable_cpp_int_t;
        cpp_int int_var__sch_grp_correctable_enable;
        void sch_grp_correctable_enable (const cpp_int  & l__val);
        cpp_int sch_grp_correctable_enable() const;
    
}; // cap_txs_csr_int_srams_ecc_int_enable_set_t
    
class cap_txs_csr_int_srams_ecc_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_srams_ecc_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_srams_ecc_intreg_t(string name = "cap_txs_csr_int_srams_ecc_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_srams_ecc_intreg_t();
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
    
        typedef pu_cpp_int< 1 > tmr_cnt_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__tmr_cnt_uncorrectable_interrupt;
        void tmr_cnt_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int tmr_cnt_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > tmr_cnt_correctable_interrupt_cpp_int_t;
        cpp_int int_var__tmr_cnt_correctable_interrupt;
        void tmr_cnt_correctable_interrupt (const cpp_int  & l__val);
        cpp_int tmr_cnt_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_lif_map_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__sch_lif_map_uncorrectable_interrupt;
        void sch_lif_map_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int sch_lif_map_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_lif_map_correctable_interrupt_cpp_int_t;
        cpp_int int_var__sch_lif_map_correctable_interrupt;
        void sch_lif_map_correctable_interrupt (const cpp_int  & l__val);
        cpp_int sch_lif_map_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_map_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__sch_rlid_map_uncorrectable_interrupt;
        void sch_rlid_map_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int sch_rlid_map_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_map_correctable_interrupt_cpp_int_t;
        cpp_int int_var__sch_rlid_map_correctable_interrupt;
        void sch_rlid_map_correctable_interrupt (const cpp_int  & l__val);
        cpp_int sch_rlid_map_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_grp_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__sch_grp_uncorrectable_interrupt;
        void sch_grp_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int sch_grp_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sch_grp_correctable_interrupt_cpp_int_t;
        cpp_int int_var__sch_grp_correctable_interrupt;
        void sch_grp_correctable_interrupt (const cpp_int  & l__val);
        cpp_int sch_grp_correctable_interrupt() const;
    
}; // cap_txs_csr_int_srams_ecc_intreg_t
    
class cap_txs_csr_int_srams_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_int_srams_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_int_srams_ecc_t(string name = "cap_txs_csr_int_srams_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_int_srams_ecc_t();
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
    
        cap_txs_csr_int_srams_ecc_intreg_t intreg;
    
        cap_txs_csr_int_srams_ecc_intreg_t int_test_set;
    
        cap_txs_csr_int_srams_ecc_int_enable_set_t int_enable_set;
    
        cap_txs_csr_int_srams_ecc_int_enable_set_t int_enable_clear;
    
}; // cap_txs_csr_int_srams_ecc_t
    
class cap_txs_csr_dhs_sch_grp_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_grp_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_grp_sram_entry_t(string name = "cap_txs_csr_dhs_sch_grp_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_grp_sram_entry_t();
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
    
        typedef pu_cpp_int< 1 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 5 > clr_pend_cnt_cpp_int_t;
        cpp_int int_var__clr_pend_cnt;
        void clr_pend_cnt (const cpp_int  & l__val);
        cpp_int clr_pend_cnt() const;
    
        typedef pu_cpp_int< 1 > rl_thr_cpp_int_t;
        cpp_int int_var__rl_thr;
        void rl_thr (const cpp_int  & l__val);
        cpp_int rl_thr() const;
    
        typedef pu_cpp_int< 4 > bckgr_cnt_cpp_int_t;
        cpp_int int_var__bckgr_cnt;
        void bckgr_cnt (const cpp_int  & l__val);
        cpp_int bckgr_cnt() const;
    
        typedef pu_cpp_int< 1 > bckgr_cpp_int_t;
        cpp_int int_var__bckgr;
        void bckgr (const cpp_int  & l__val);
        cpp_int bckgr() const;
    
        typedef pu_cpp_int< 1 > hbm_mode_cpp_int_t;
        cpp_int int_var__hbm_mode;
        void hbm_mode (const cpp_int  & l__val);
        cpp_int hbm_mode() const;
    
        typedef pu_cpp_int< 11 > qid_offset_cpp_int_t;
        cpp_int int_var__qid_offset;
        void qid_offset (const cpp_int  & l__val);
        cpp_int qid_offset() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 1 > auto_clr_cpp_int_t;
        cpp_int int_var__auto_clr;
        void auto_clr (const cpp_int  & l__val);
        cpp_int auto_clr() const;
    
        typedef pu_cpp_int< 4 > rr_sel_cpp_int_t;
        cpp_int int_var__rr_sel;
        void rr_sel (const cpp_int  & l__val);
        cpp_int rr_sel() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm0_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm0_vld;
        void qid_fifo_elm0_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm0_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm0_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm0_jnk;
        void qid_fifo_elm0_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm0_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm0_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm0_qid;
        void qid_fifo_elm0_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm0_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm1_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm1_vld;
        void qid_fifo_elm1_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm1_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm1_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm1_jnk;
        void qid_fifo_elm1_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm1_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm1_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm1_qid;
        void qid_fifo_elm1_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm1_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm2_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm2_vld;
        void qid_fifo_elm2_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm2_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm2_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm2_jnk;
        void qid_fifo_elm2_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm2_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm2_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm2_qid;
        void qid_fifo_elm2_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm2_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm3_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm3_vld;
        void qid_fifo_elm3_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm3_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm3_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm3_jnk;
        void qid_fifo_elm3_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm3_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm3_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm3_qid;
        void qid_fifo_elm3_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm3_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm4_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm4_vld;
        void qid_fifo_elm4_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm4_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm4_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm4_jnk;
        void qid_fifo_elm4_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm4_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm4_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm4_qid;
        void qid_fifo_elm4_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm4_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm5_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm5_vld;
        void qid_fifo_elm5_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm5_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm5_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm5_jnk;
        void qid_fifo_elm5_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm5_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm5_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm5_qid;
        void qid_fifo_elm5_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm5_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm6_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm6_vld;
        void qid_fifo_elm6_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm6_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm6_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm6_jnk;
        void qid_fifo_elm6_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm6_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm6_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm6_qid;
        void qid_fifo_elm6_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm6_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm7_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm7_vld;
        void qid_fifo_elm7_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm7_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm7_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm7_jnk;
        void qid_fifo_elm7_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm7_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm7_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm7_qid;
        void qid_fifo_elm7_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm7_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm8_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm8_vld;
        void qid_fifo_elm8_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm8_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm8_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm8_jnk;
        void qid_fifo_elm8_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm8_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm8_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm8_qid;
        void qid_fifo_elm8_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm8_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm9_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm9_vld;
        void qid_fifo_elm9_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm9_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm9_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm9_jnk;
        void qid_fifo_elm9_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm9_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm9_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm9_qid;
        void qid_fifo_elm9_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm9_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm10_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm10_vld;
        void qid_fifo_elm10_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm10_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm10_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm10_jnk;
        void qid_fifo_elm10_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm10_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm10_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm10_qid;
        void qid_fifo_elm10_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm10_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm11_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm11_vld;
        void qid_fifo_elm11_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm11_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm11_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm11_jnk;
        void qid_fifo_elm11_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm11_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm11_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm11_qid;
        void qid_fifo_elm11_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm11_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm12_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm12_vld;
        void qid_fifo_elm12_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm12_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm12_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm12_jnk;
        void qid_fifo_elm12_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm12_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm12_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm12_qid;
        void qid_fifo_elm12_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm12_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm13_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm13_vld;
        void qid_fifo_elm13_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm13_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm13_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm13_jnk;
        void qid_fifo_elm13_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm13_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm13_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm13_qid;
        void qid_fifo_elm13_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm13_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm14_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm14_vld;
        void qid_fifo_elm14_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm14_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm14_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm14_jnk;
        void qid_fifo_elm14_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm14_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm14_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm14_qid;
        void qid_fifo_elm14_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm14_qid() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm15_vld_cpp_int_t;
        cpp_int int_var__qid_fifo_elm15_vld;
        void qid_fifo_elm15_vld (const cpp_int  & l__val);
        cpp_int qid_fifo_elm15_vld() const;
    
        typedef pu_cpp_int< 1 > qid_fifo_elm15_jnk_cpp_int_t;
        cpp_int int_var__qid_fifo_elm15_jnk;
        void qid_fifo_elm15_jnk (const cpp_int  & l__val);
        cpp_int qid_fifo_elm15_jnk() const;
    
        typedef pu_cpp_int< 13 > qid_fifo_elm15_qid_cpp_int_t;
        cpp_int int_var__qid_fifo_elm15_qid;
        void qid_fifo_elm15_qid (const cpp_int  & l__val);
        cpp_int qid_fifo_elm15_qid() const;
    
        typedef pu_cpp_int< 4 > hbm_ln_cpp_int_t;
        cpp_int int_var__hbm_ln;
        void hbm_ln (const cpp_int  & l__val);
        cpp_int hbm_ln() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr_cpp_int_t;
        cpp_int int_var__hbm_ln_ptr;
        void hbm_ln_ptr (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr() const;
    
        typedef pu_cpp_int< 9 > hbm_rr_cnt_cpp_int_t;
        cpp_int int_var__hbm_rr_cnt;
        void hbm_rr_cnt (const cpp_int  & l__val);
        cpp_int hbm_rr_cnt() const;
    
        typedef pu_cpp_int< 14 > drb_cnt_cpp_int_t;
        cpp_int int_var__drb_cnt;
        void drb_cnt (const cpp_int  & l__val);
        cpp_int drb_cnt() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt0_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt0;
        void hbm_ln_cnt0 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt0() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt1_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt1;
        void hbm_ln_cnt1 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt1() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt2_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt2;
        void hbm_ln_cnt2 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt2() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt3_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt3;
        void hbm_ln_cnt3 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt3() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt4_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt4;
        void hbm_ln_cnt4 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt4() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt5_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt5;
        void hbm_ln_cnt5 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt5() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt6_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt6;
        void hbm_ln_cnt6 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt6() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt7_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt7;
        void hbm_ln_cnt7 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt7() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt8_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt8;
        void hbm_ln_cnt8 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt8() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt9_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt9;
        void hbm_ln_cnt9 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt9() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt10_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt10;
        void hbm_ln_cnt10 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt10() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt11_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt11;
        void hbm_ln_cnt11 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt11() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt12_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt12;
        void hbm_ln_cnt12 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt12() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt13_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt13;
        void hbm_ln_cnt13 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt13() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt14_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt14;
        void hbm_ln_cnt14 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt14() const;
    
        typedef pu_cpp_int< 11 > hbm_ln_cnt15_cpp_int_t;
        cpp_int int_var__hbm_ln_cnt15;
        void hbm_ln_cnt15 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt15() const;
    
        typedef pu_cpp_int< 8 > popcnt_cpp_int_t;
        cpp_int int_var__popcnt;
        void popcnt (const cpp_int  & l__val);
        cpp_int popcnt() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_sch_grp_sram_entry_t
    
class cap_txs_csr_dhs_sch_grp_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_grp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_grp_sram_t(string name = "cap_txs_csr_dhs_sch_grp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_grp_sram_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_sch_grp_sram_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_sch_grp_sram_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_grp_sram_t
    
class cap_txs_csr_dhs_sch_rlid_map_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_rlid_map_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_rlid_map_sram_entry_t(string name = "cap_txs_csr_dhs_sch_rlid_map_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_rlid_map_sram_entry_t();
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
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 11 > sg_start_cpp_int_t;
        cpp_int int_var__sg_start;
        void sg_start (const cpp_int  & l__val);
        cpp_int sg_start() const;
    
        typedef pu_cpp_int< 11 > sg_end_cpp_int_t;
        cpp_int int_var__sg_end;
        void sg_end (const cpp_int  & l__val);
        cpp_int sg_end() const;
    
        typedef pu_cpp_int< 6 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_sch_rlid_map_sram_entry_t
    
class cap_txs_csr_dhs_sch_rlid_map_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_rlid_map_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_rlid_map_sram_t(string name = "cap_txs_csr_dhs_sch_rlid_map_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_rlid_map_sram_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_sch_rlid_map_sram_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_sch_rlid_map_sram_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_rlid_map_sram_t
    
class cap_txs_csr_dhs_sch_lif_map_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_lif_map_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_lif_map_sram_entry_t(string name = "cap_txs_csr_dhs_sch_lif_map_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_lif_map_sram_entry_t();
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
    
        typedef pu_cpp_int< 2 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > sg_active_cpp_int_t;
        cpp_int int_var__sg_active;
        void sg_active (const cpp_int  & l__val);
        cpp_int sg_active() const;
    
        typedef pu_cpp_int< 11 > sg_start_cpp_int_t;
        cpp_int int_var__sg_start;
        void sg_start (const cpp_int  & l__val);
        cpp_int sg_start() const;
    
        typedef pu_cpp_int< 11 > sg_per_cos_cpp_int_t;
        cpp_int int_var__sg_per_cos;
        void sg_per_cos (const cpp_int  & l__val);
        cpp_int sg_per_cos() const;
    
        typedef pu_cpp_int< 16 > sg_act_cos_cpp_int_t;
        cpp_int int_var__sg_act_cos;
        void sg_act_cos (const cpp_int  & l__val);
        cpp_int sg_act_cos() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_sch_lif_map_sram_entry_t
    
class cap_txs_csr_dhs_sch_lif_map_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_lif_map_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_lif_map_sram_t(string name = "cap_txs_csr_dhs_sch_lif_map_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_lif_map_sram_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_sch_lif_map_sram_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_sch_lif_map_sram_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_lif_map_sram_t
    
class cap_txs_csr_dhs_tmr_cnt_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_tmr_cnt_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_tmr_cnt_sram_entry_t(string name = "cap_txs_csr_dhs_tmr_cnt_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_tmr_cnt_sram_entry_t();
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
    
        typedef pu_cpp_int< 5 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 18 > slow_cbcnt_cpp_int_t;
        cpp_int int_var__slow_cbcnt;
        void slow_cbcnt (const cpp_int  & l__val);
        cpp_int slow_cbcnt() const;
    
        typedef pu_cpp_int< 18 > slow_bcnt_cpp_int_t;
        cpp_int int_var__slow_bcnt;
        void slow_bcnt (const cpp_int  & l__val);
        cpp_int slow_bcnt() const;
    
        typedef pu_cpp_int< 4 > slow_lcnt_cpp_int_t;
        cpp_int int_var__slow_lcnt;
        void slow_lcnt (const cpp_int  & l__val);
        cpp_int slow_lcnt() const;
    
        typedef pu_cpp_int< 18 > fast_cbcnt_cpp_int_t;
        cpp_int int_var__fast_cbcnt;
        void fast_cbcnt (const cpp_int  & l__val);
        cpp_int fast_cbcnt() const;
    
        typedef pu_cpp_int< 18 > fast_bcnt_cpp_int_t;
        cpp_int int_var__fast_bcnt;
        void fast_bcnt (const cpp_int  & l__val);
        cpp_int fast_bcnt() const;
    
        typedef pu_cpp_int< 4 > fast_lcnt_cpp_int_t;
        cpp_int int_var__fast_lcnt;
        void fast_lcnt (const cpp_int  & l__val);
        cpp_int fast_lcnt() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_tmr_cnt_sram_entry_t
    
class cap_txs_csr_dhs_tmr_cnt_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_tmr_cnt_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_tmr_cnt_sram_t(string name = "cap_txs_csr_dhs_tmr_cnt_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_tmr_cnt_sram_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_tmr_cnt_sram_entry_t, 4096> entry;
        #else 
        cap_txs_csr_dhs_tmr_cnt_sram_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_txs_csr_dhs_tmr_cnt_sram_t
    
class cap_txs_csr_dhs_rlid_stop_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_rlid_stop_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_rlid_stop_entry_t(string name = "cap_txs_csr_dhs_rlid_stop_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_rlid_stop_entry_t();
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
    
        typedef pu_cpp_int< 1 > setmsk_cpp_int_t;
        cpp_int int_var__setmsk;
        void setmsk (const cpp_int  & l__val);
        cpp_int setmsk() const;
    
}; // cap_txs_csr_dhs_rlid_stop_entry_t
    
class cap_txs_csr_dhs_rlid_stop_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_rlid_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_rlid_stop_t(string name = "cap_txs_csr_dhs_rlid_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_rlid_stop_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_rlid_stop_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_rlid_stop_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_rlid_stop_t
    
class cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t(string name = "cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t();
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
    
        typedef pu_cpp_int< 8 > popcnt_cpp_int_t;
        cpp_int int_var__popcnt;
        void popcnt (const cpp_int  & l__val);
        cpp_int popcnt() const;
    
        typedef pu_cpp_int< 14 > drb_cnt_cpp_int_t;
        cpp_int int_var__drb_cnt;
        void drb_cnt (const cpp_int  & l__val);
        cpp_int drb_cnt() const;
    
}; // cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t
    
class cap_txs_csr_dhs_sch_grp_cnt_entry_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_grp_cnt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_grp_cnt_entry_t(string name = "cap_txs_csr_dhs_sch_grp_cnt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_grp_cnt_entry_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_sch_grp_cnt_entry_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_grp_cnt_entry_t
    
class cap_txs_csr_dhs_sch_grp_entry_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_grp_entry_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_grp_entry_entry_t(string name = "cap_txs_csr_dhs_sch_grp_entry_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_grp_entry_entry_t();
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
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 11 > qid_offset_cpp_int_t;
        cpp_int int_var__qid_offset;
        void qid_offset (const cpp_int  & l__val);
        cpp_int qid_offset() const;
    
        typedef pu_cpp_int< 1 > auto_clr_cpp_int_t;
        cpp_int int_var__auto_clr;
        void auto_clr (const cpp_int  & l__val);
        cpp_int auto_clr() const;
    
        typedef pu_cpp_int< 4 > rr_sel_cpp_int_t;
        cpp_int int_var__rr_sel;
        void rr_sel (const cpp_int  & l__val);
        cpp_int rr_sel() const;
    
        typedef pu_cpp_int< 1 > rl_thr_cpp_int_t;
        cpp_int int_var__rl_thr;
        void rl_thr (const cpp_int  & l__val);
        cpp_int rl_thr() const;
    
}; // cap_txs_csr_dhs_sch_grp_entry_entry_t
    
class cap_txs_csr_dhs_sch_grp_entry_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_grp_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_grp_entry_t(string name = "cap_txs_csr_dhs_sch_grp_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_grp_entry_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_sch_grp_entry_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_sch_grp_entry_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_grp_entry_t
    
class cap_txs_csr_dhs_doorbell_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_doorbell_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_doorbell_entry_t(string name = "cap_txs_csr_dhs_doorbell_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_doorbell_entry_t();
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
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 4 > cos_cpp_int_t;
        cpp_int int_var__cos;
        void cos (const cpp_int  & l__val);
        cpp_int cos() const;
    
        typedef pu_cpp_int< 1 > set_cpp_int_t;
        cpp_int int_var__set;
        void set (const cpp_int  & l__val);
        cpp_int set() const;
    
}; // cap_txs_csr_dhs_doorbell_entry_t
    
class cap_txs_csr_dhs_doorbell_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_doorbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_doorbell_t(string name = "cap_txs_csr_dhs_doorbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_doorbell_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_doorbell_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_doorbell_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_doorbell_t
    
class cap_txs_csr_dhs_sch_flags_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_flags_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_flags_entry_t(string name = "cap_txs_csr_dhs_sch_flags_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_flags_entry_t();
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
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        cpp_int int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags_entry_t
    
class cap_txs_csr_dhs_sch_flags_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_sch_flags_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_sch_flags_t(string name = "cap_txs_csr_dhs_sch_flags_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_sch_flags_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_sch_flags_entry_t, 64> entry;
        #else 
        cap_txs_csr_dhs_sch_flags_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags_t
    
class cap_txs_csr_dhs_dtdmhi_calendar_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_dtdmhi_calendar_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_dtdmhi_calendar_entry_t(string name = "cap_txs_csr_dhs_dtdmhi_calendar_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_dtdmhi_calendar_entry_t();
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
    
        typedef pu_cpp_int< 4 > dtdm_calendar_cpp_int_t;
        cpp_int int_var__dtdm_calendar;
        void dtdm_calendar (const cpp_int  & l__val);
        cpp_int dtdm_calendar() const;
    
}; // cap_txs_csr_dhs_dtdmhi_calendar_entry_t
    
class cap_txs_csr_dhs_dtdmhi_calendar_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_dtdmhi_calendar_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_dtdmhi_calendar_t(string name = "cap_txs_csr_dhs_dtdmhi_calendar_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_dtdmhi_calendar_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_dtdmhi_calendar_entry_t, 64> entry;
        #else 
        cap_txs_csr_dhs_dtdmhi_calendar_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_dtdmhi_calendar_t
    
class cap_txs_csr_dhs_dtdmlo_calendar_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_dtdmlo_calendar_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_dtdmlo_calendar_entry_t(string name = "cap_txs_csr_dhs_dtdmlo_calendar_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_dtdmlo_calendar_entry_t();
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
    
        typedef pu_cpp_int< 4 > dtdm_calendar_cpp_int_t;
        cpp_int int_var__dtdm_calendar;
        void dtdm_calendar (const cpp_int  & l__val);
        cpp_int dtdm_calendar() const;
    
}; // cap_txs_csr_dhs_dtdmlo_calendar_entry_t
    
class cap_txs_csr_dhs_dtdmlo_calendar_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_dtdmlo_calendar_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_dtdmlo_calendar_t(string name = "cap_txs_csr_dhs_dtdmlo_calendar_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_dtdmlo_calendar_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_dtdmlo_calendar_entry_t, 64> entry;
        #else 
        cap_txs_csr_dhs_dtdmlo_calendar_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_dtdmlo_calendar_t
    
class cap_txs_csr_dhs_slow_timer_pending_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_slow_timer_pending_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_slow_timer_pending_entry_t(string name = "cap_txs_csr_dhs_slow_timer_pending_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_slow_timer_pending_entry_t();
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
    
        typedef pu_cpp_int< 18 > bcnt_cpp_int_t;
        cpp_int int_var__bcnt;
        void bcnt (const cpp_int  & l__val);
        cpp_int bcnt() const;
    
        typedef pu_cpp_int< 4 > lcnt_cpp_int_t;
        cpp_int int_var__lcnt;
        void lcnt (const cpp_int  & l__val);
        cpp_int lcnt() const;
    
}; // cap_txs_csr_dhs_slow_timer_pending_entry_t
    
class cap_txs_csr_dhs_slow_timer_pending_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_slow_timer_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_slow_timer_pending_t(string name = "cap_txs_csr_dhs_slow_timer_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_slow_timer_pending_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_slow_timer_pending_entry_t, 4096> entry;
        #else 
        cap_txs_csr_dhs_slow_timer_pending_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_txs_csr_dhs_slow_timer_pending_t
    
class cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t(string name = "cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t();
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
    
        typedef pu_cpp_int< 3 > typ_cpp_int_t;
        cpp_int int_var__typ;
        void typ (const cpp_int  & l__val);
        cpp_int typ() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        cpp_int int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 10 > dtime_cpp_int_t;
        cpp_int int_var__dtime;
        void dtime (const cpp_int  & l__val);
        cpp_int dtime() const;
    
}; // cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t
    
class cap_txs_csr_dhs_slow_timer_start_no_stop_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_slow_timer_start_no_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_slow_timer_start_no_stop_t(string name = "cap_txs_csr_dhs_slow_timer_start_no_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_slow_timer_start_no_stop_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_slow_timer_start_no_stop_t
    
class cap_txs_csr_dhs_fast_timer_pending_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_fast_timer_pending_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_fast_timer_pending_entry_t(string name = "cap_txs_csr_dhs_fast_timer_pending_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_fast_timer_pending_entry_t();
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
    
        typedef pu_cpp_int< 18 > bcnt_cpp_int_t;
        cpp_int int_var__bcnt;
        void bcnt (const cpp_int  & l__val);
        cpp_int bcnt() const;
    
        typedef pu_cpp_int< 4 > lcnt_cpp_int_t;
        cpp_int int_var__lcnt;
        void lcnt (const cpp_int  & l__val);
        cpp_int lcnt() const;
    
}; // cap_txs_csr_dhs_fast_timer_pending_entry_t
    
class cap_txs_csr_dhs_fast_timer_pending_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_fast_timer_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_fast_timer_pending_t(string name = "cap_txs_csr_dhs_fast_timer_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_fast_timer_pending_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_fast_timer_pending_entry_t, 4096> entry;
        #else 
        cap_txs_csr_dhs_fast_timer_pending_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_txs_csr_dhs_fast_timer_pending_t
    
class cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t(string name = "cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t();
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
    
        typedef pu_cpp_int< 3 > typ_cpp_int_t;
        cpp_int int_var__typ;
        void typ (const cpp_int  & l__val);
        cpp_int typ() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        cpp_int int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 10 > dtime_cpp_int_t;
        cpp_int int_var__dtime;
        void dtime (const cpp_int  & l__val);
        cpp_int dtime() const;
    
}; // cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t
    
class cap_txs_csr_dhs_fast_timer_start_no_stop_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_dhs_fast_timer_start_no_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_dhs_fast_timer_start_no_stop_t(string name = "cap_txs_csr_dhs_fast_timer_start_no_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_dhs_fast_timer_start_no_stop_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t, 2048> entry;
        #else 
        cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_fast_timer_start_no_stop_t
    
class cap_txs_csr_cnt_tmr_axi_bid_uexp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_bid_uexp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_bid_uexp_t(string name = "cap_txs_csr_cnt_tmr_axi_bid_uexp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_bid_uexp_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_bid_uexp_t
    
class cap_txs_csr_cnt_tmr_axi_bid_rerr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_bid_rerr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_bid_rerr_t(string name = "cap_txs_csr_cnt_tmr_axi_bid_rerr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_bid_rerr_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_bid_rerr_t
    
class cap_txs_csr_cnt_tmr_axi_bid_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_bid_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_bid_err_t(string name = "cap_txs_csr_cnt_tmr_axi_bid_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_bid_err_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_bid_err_t
    
class cap_txs_csr_cnt_tmr_axi_bid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_bid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_bid_t(string name = "cap_txs_csr_cnt_tmr_axi_bid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_bid_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_bid_t
    
class cap_txs_csr_cnt_tmr_axi_wr_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_wr_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_wr_req_t(string name = "cap_txs_csr_cnt_tmr_axi_wr_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_wr_req_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_wr_req_t
    
class cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t(string name = "cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t
    
class cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t(string name = "cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t
    
class cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t(string name = "cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t
    
class cap_txs_csr_cnt_tmr_axi_rd_rsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_rd_rsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_rd_rsp_t(string name = "cap_txs_csr_cnt_tmr_axi_rd_rsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_rd_rsp_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_rd_rsp_t
    
class cap_txs_csr_cnt_tmr_axi_rd_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_tmr_axi_rd_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_tmr_axi_rd_req_t(string name = "cap_txs_csr_cnt_tmr_axi_rd_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_tmr_axi_rd_req_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_axi_rd_req_t
    
class cap_txs_csr_cnt_sch_axi_bid_uexp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_bid_uexp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_bid_uexp_t(string name = "cap_txs_csr_cnt_sch_axi_bid_uexp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_bid_uexp_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_bid_uexp_t
    
class cap_txs_csr_cnt_sch_axi_bid_rerr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_bid_rerr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_bid_rerr_t(string name = "cap_txs_csr_cnt_sch_axi_bid_rerr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_bid_rerr_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_bid_rerr_t
    
class cap_txs_csr_cnt_sch_axi_bid_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_bid_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_bid_err_t(string name = "cap_txs_csr_cnt_sch_axi_bid_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_bid_err_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_bid_err_t
    
class cap_txs_csr_cnt_sch_axi_bid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_bid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_bid_t(string name = "cap_txs_csr_cnt_sch_axi_bid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_bid_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_bid_t
    
class cap_txs_csr_cnt_sch_axi_wr_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_wr_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_wr_req_t(string name = "cap_txs_csr_cnt_sch_axi_wr_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_wr_req_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_wr_req_t
    
class cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t(string name = "cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t
    
class cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t(string name = "cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t
    
class cap_txs_csr_cnt_sch_axi_rd_rsp_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_rd_rsp_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_rd_rsp_err_t(string name = "cap_txs_csr_cnt_sch_axi_rd_rsp_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_rd_rsp_err_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_rd_rsp_err_t
    
class cap_txs_csr_cnt_sch_axi_rd_rsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_rd_rsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_rd_rsp_t(string name = "cap_txs_csr_cnt_sch_axi_rd_rsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_rd_rsp_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_rd_rsp_t
    
class cap_txs_csr_cnt_sch_axi_rd_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_axi_rd_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_axi_rd_req_t(string name = "cap_txs_csr_cnt_sch_axi_rd_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_axi_rd_req_t();
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
    
        typedef pu_cpp_int< 48 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_axi_rd_req_t
    
class cap_txs_csr_sta_stmr_max_bcnt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_stmr_max_bcnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_stmr_max_bcnt_t(string name = "cap_txs_csr_sta_stmr_max_bcnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_stmr_max_bcnt_t();
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
    
        typedef pu_cpp_int< 18 > bcnt_cpp_int_t;
        cpp_int int_var__bcnt;
        void bcnt (const cpp_int  & l__val);
        cpp_int bcnt() const;
    
}; // cap_txs_csr_sta_stmr_max_bcnt_t
    
class cap_txs_csr_sta_ftmr_max_bcnt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_ftmr_max_bcnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_ftmr_max_bcnt_t(string name = "cap_txs_csr_sta_ftmr_max_bcnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_ftmr_max_bcnt_t();
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
    
        typedef pu_cpp_int< 18 > bcnt_cpp_int_t;
        cpp_int int_var__bcnt;
        void bcnt (const cpp_int  & l__val);
        cpp_int bcnt() const;
    
}; // cap_txs_csr_sta_ftmr_max_bcnt_t
    
class cap_txs_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_csr_intr_t(string name = "cap_txs_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_csr_intr_t();
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
    
        typedef pu_cpp_int< 1 > dowstream_cpp_int_t;
        cpp_int int_var__dowstream;
        void dowstream (const cpp_int  & l__val);
        cpp_int dowstream() const;
    
        typedef pu_cpp_int< 1 > dowstream_enable_cpp_int_t;
        cpp_int int_var__dowstream_enable;
        void dowstream_enable (const cpp_int  & l__val);
        cpp_int dowstream_enable() const;
    
}; // cap_txs_csr_csr_intr_t
    
class cap_txs_csr_sta_srams_sch_hbm_byp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_sch_hbm_byp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_sch_hbm_byp_t(string name = "cap_txs_csr_sta_srams_sch_hbm_byp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_sch_hbm_byp_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_sch_hbm_byp_t
    
class cap_txs_csr_sta_srams_sch_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_sch_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_sch_hbm_t(string name = "cap_txs_csr_sta_srams_sch_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_sch_hbm_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_sch_hbm_t
    
class cap_txs_csr_cfg_sch_hbm_byp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_sch_hbm_byp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_sch_hbm_byp_sram_t(string name = "cap_txs_csr_cfg_sch_hbm_byp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_sch_hbm_byp_sram_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_sch_hbm_byp_sram_t
    
class cap_txs_csr_cfg_sch_hbm_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_sch_hbm_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_sch_hbm_sram_t(string name = "cap_txs_csr_cfg_sch_hbm_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_sch_hbm_sram_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_sch_hbm_sram_t
    
class cap_txs_csr_sta_srams_ecc_sch_grp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_ecc_sch_grp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_ecc_sch_grp_t(string name = "cap_txs_csr_sta_srams_ecc_sch_grp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_ecc_sch_grp_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 36 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_ecc_sch_grp_t
    
class cap_txs_csr_sta_srams_ecc_sch_rlid_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_ecc_sch_rlid_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_ecc_sch_rlid_map_t(string name = "cap_txs_csr_sta_srams_ecc_sch_rlid_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_ecc_sch_rlid_map_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 6 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_ecc_sch_rlid_map_t
    
class cap_txs_csr_sta_srams_ecc_sch_lif_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_ecc_sch_lif_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_ecc_sch_lif_map_t(string name = "cap_txs_csr_sta_srams_ecc_sch_lif_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_ecc_sch_lif_map_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_ecc_sch_lif_map_t
    
class cap_txs_csr_sta_srams_ecc_tmr_cnt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_ecc_tmr_cnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_ecc_tmr_cnt_t(string name = "cap_txs_csr_sta_srams_ecc_tmr_cnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_ecc_tmr_cnt_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_ecc_tmr_cnt_t
    
class cap_txs_csr_sta_scheduler_dbg2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_scheduler_dbg2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_scheduler_dbg2_t(string name = "cap_txs_csr_sta_scheduler_dbg2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_scheduler_dbg2_t();
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
    
        typedef pu_cpp_int< 6 > hbm_byp_pending_cnt_cpp_int_t;
        cpp_int int_var__hbm_byp_pending_cnt;
        void hbm_byp_pending_cnt (const cpp_int  & l__val);
        cpp_int hbm_byp_pending_cnt() const;
    
        typedef pu_cpp_int< 6 > hbm_rd_pending_cnt_cpp_int_t;
        cpp_int int_var__hbm_rd_pending_cnt;
        void hbm_rd_pending_cnt (const cpp_int  & l__val);
        cpp_int hbm_rd_pending_cnt() const;
    
        typedef pu_cpp_int< 6 > hbm_wr_pending_cnt_cpp_int_t;
        cpp_int int_var__hbm_wr_pending_cnt;
        void hbm_wr_pending_cnt (const cpp_int  & l__val);
        cpp_int hbm_wr_pending_cnt() const;
    
}; // cap_txs_csr_sta_scheduler_dbg2_t
    
class cap_txs_csr_sta_scheduler_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_scheduler_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_scheduler_dbg_t(string name = "cap_txs_csr_sta_scheduler_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_scheduler_dbg_t();
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
    
        typedef pu_cpp_int< 1 > txdma_drdy_cpp_int_t;
        cpp_int int_var__txdma_drdy;
        void txdma_drdy (const cpp_int  & l__val);
        cpp_int txdma_drdy() const;
    
        typedef pu_cpp_int< 1 > hbm_re_efull_cpp_int_t;
        cpp_int int_var__hbm_re_efull;
        void hbm_re_efull (const cpp_int  & l__val);
        cpp_int hbm_re_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_we_efull_cpp_int_t;
        cpp_int int_var__hbm_we_efull;
        void hbm_we_efull (const cpp_int  & l__val);
        cpp_int hbm_we_efull() const;
    
        typedef pu_cpp_int< 1 > txdma_msg_efull_cpp_int_t;
        cpp_int int_var__txdma_msg_efull;
        void txdma_msg_efull (const cpp_int  & l__val);
        cpp_int txdma_msg_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_byp_pending_efull_cpp_int_t;
        cpp_int int_var__hbm_byp_pending_efull;
        void hbm_byp_pending_efull (const cpp_int  & l__val);
        cpp_int hbm_byp_pending_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_rd_pending_efull_cpp_int_t;
        cpp_int int_var__hbm_rd_pending_efull;
        void hbm_rd_pending_efull (const cpp_int  & l__val);
        cpp_int hbm_rd_pending_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_wr_pending_efull_cpp_int_t;
        cpp_int int_var__hbm_wr_pending_efull;
        void hbm_wr_pending_efull (const cpp_int  & l__val);
        cpp_int hbm_wr_pending_efull() const;
    
}; // cap_txs_csr_sta_scheduler_dbg_t
    
class cap_txs_csr_sta_sch_lif_sg_mismatch_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_sch_lif_sg_mismatch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_sch_lif_sg_mismatch_t(string name = "cap_txs_csr_sta_sch_lif_sg_mismatch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_sch_lif_sg_mismatch_t();
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
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 13 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 1 > set_cpp_int_t;
        cpp_int int_var__set;
        void set (const cpp_int  & l__val);
        cpp_int set() const;
    
}; // cap_txs_csr_sta_sch_lif_sg_mismatch_t
    
class cap_txs_csr_sta_sch_lif_map_notactive_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_sch_lif_map_notactive_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_sch_lif_map_notactive_t(string name = "cap_txs_csr_sta_sch_lif_map_notactive_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_sch_lif_map_notactive_t();
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
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 4 > cos_cpp_int_t;
        cpp_int int_var__cos;
        void cos (const cpp_int  & l__val);
        cpp_int cos() const;
    
        typedef pu_cpp_int< 1 > set_cpp_int_t;
        cpp_int int_var__set;
        void set (const cpp_int  & l__val);
        cpp_int set() const;
    
}; // cap_txs_csr_sta_sch_lif_map_notactive_t
    
class cap_txs_csr_cfg_sch_grp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_sch_grp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_sch_grp_sram_t(string name = "cap_txs_csr_cfg_sch_grp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_sch_grp_sram_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_sch_grp_sram_t
    
class cap_txs_csr_cfg_sch_rlid_map_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_sch_rlid_map_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_sch_rlid_map_sram_t(string name = "cap_txs_csr_cfg_sch_rlid_map_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_sch_rlid_map_sram_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_sch_rlid_map_sram_t
    
class cap_txs_csr_cfg_sch_lif_map_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_sch_lif_map_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_sch_lif_map_sram_t(string name = "cap_txs_csr_cfg_sch_lif_map_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_sch_lif_map_sram_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_sch_lif_map_sram_t
    
class cap_txs_csr_cfg_tmr_cnt_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_tmr_cnt_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_tmr_cnt_sram_t(string name = "cap_txs_csr_cfg_tmr_cnt_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_tmr_cnt_sram_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_tmr_cnt_sram_t
    
class cap_txs_csr_sta_sch_max_hbm_byp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_sch_max_hbm_byp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_sch_max_hbm_byp_t(string name = "cap_txs_csr_sta_sch_max_hbm_byp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_sch_max_hbm_byp_t();
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
    
        typedef pu_cpp_int< 6 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_txs_csr_sta_sch_max_hbm_byp_t
    
class cap_txs_csr_cfg_scheduler_dbg2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_scheduler_dbg2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_scheduler_dbg2_t(string name = "cap_txs_csr_cfg_scheduler_dbg2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_scheduler_dbg2_t();
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
    
        typedef pu_cpp_int< 24 > qid_read_cpp_int_t;
        cpp_int int_var__qid_read;
        void qid_read (const cpp_int  & l__val);
        cpp_int qid_read() const;
    
        typedef pu_cpp_int< 4 > cos_read_cpp_int_t;
        cpp_int int_var__cos_read;
        void cos_read (const cpp_int  & l__val);
        cpp_int cos_read() const;
    
}; // cap_txs_csr_cfg_scheduler_dbg2_t
    
class cap_txs_csr_cfg_scheduler_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_scheduler_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_scheduler_dbg_t(string name = "cap_txs_csr_cfg_scheduler_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_scheduler_dbg_t();
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
    
        typedef pu_cpp_int< 3 > hbm_efc_thr_cpp_int_t;
        cpp_int int_var__hbm_efc_thr;
        void hbm_efc_thr (const cpp_int  & l__val);
        cpp_int hbm_efc_thr() const;
    
        typedef pu_cpp_int< 2 > txdma_efc_thr_cpp_int_t;
        cpp_int int_var__txdma_efc_thr;
        void txdma_efc_thr (const cpp_int  & l__val);
        cpp_int txdma_efc_thr() const;
    
        typedef pu_cpp_int< 5 > fifo_mode_thr_cpp_int_t;
        cpp_int int_var__fifo_mode_thr;
        void fifo_mode_thr (const cpp_int  & l__val);
        cpp_int fifo_mode_thr() const;
    
        typedef pu_cpp_int< 5 > max_hbm_byp_cpp_int_t;
        cpp_int int_var__max_hbm_byp;
        void max_hbm_byp (const cpp_int  & l__val);
        cpp_int max_hbm_byp() const;
    
        typedef pu_cpp_int< 5 > max_hbm_wr_cpp_int_t;
        cpp_int int_var__max_hbm_wr;
        void max_hbm_wr (const cpp_int  & l__val);
        cpp_int max_hbm_wr() const;
    
        typedef pu_cpp_int< 5 > max_hbm_rd_cpp_int_t;
        cpp_int int_var__max_hbm_rd;
        void max_hbm_rd (const cpp_int  & l__val);
        cpp_int max_hbm_rd() const;
    
        typedef pu_cpp_int< 4 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_scheduler_dbg_t
    
class cap_txs_csr_sta_scheduler_rr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_scheduler_rr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_scheduler_rr_t(string name = "cap_txs_csr_sta_scheduler_rr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_scheduler_rr_t();
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
    
        typedef pu_cpp_int< 11 > curr_ptr0_cpp_int_t;
        cpp_int int_var__curr_ptr0;
        void curr_ptr0 (const cpp_int  & l__val);
        cpp_int curr_ptr0() const;
    
        typedef pu_cpp_int< 11 > curr_ptr1_cpp_int_t;
        cpp_int int_var__curr_ptr1;
        void curr_ptr1 (const cpp_int  & l__val);
        cpp_int curr_ptr1() const;
    
        typedef pu_cpp_int< 11 > curr_ptr2_cpp_int_t;
        cpp_int int_var__curr_ptr2;
        void curr_ptr2 (const cpp_int  & l__val);
        cpp_int curr_ptr2() const;
    
        typedef pu_cpp_int< 11 > curr_ptr3_cpp_int_t;
        cpp_int int_var__curr_ptr3;
        void curr_ptr3 (const cpp_int  & l__val);
        cpp_int curr_ptr3() const;
    
        typedef pu_cpp_int< 11 > curr_ptr4_cpp_int_t;
        cpp_int int_var__curr_ptr4;
        void curr_ptr4 (const cpp_int  & l__val);
        cpp_int curr_ptr4() const;
    
        typedef pu_cpp_int< 11 > curr_ptr5_cpp_int_t;
        cpp_int int_var__curr_ptr5;
        void curr_ptr5 (const cpp_int  & l__val);
        cpp_int curr_ptr5() const;
    
        typedef pu_cpp_int< 11 > curr_ptr6_cpp_int_t;
        cpp_int int_var__curr_ptr6;
        void curr_ptr6 (const cpp_int  & l__val);
        cpp_int curr_ptr6() const;
    
        typedef pu_cpp_int< 11 > curr_ptr7_cpp_int_t;
        cpp_int int_var__curr_ptr7;
        void curr_ptr7 (const cpp_int  & l__val);
        cpp_int curr_ptr7() const;
    
        typedef pu_cpp_int< 11 > curr_ptr8_cpp_int_t;
        cpp_int int_var__curr_ptr8;
        void curr_ptr8 (const cpp_int  & l__val);
        cpp_int curr_ptr8() const;
    
        typedef pu_cpp_int< 11 > curr_ptr9_cpp_int_t;
        cpp_int int_var__curr_ptr9;
        void curr_ptr9 (const cpp_int  & l__val);
        cpp_int curr_ptr9() const;
    
        typedef pu_cpp_int< 11 > curr_ptr10_cpp_int_t;
        cpp_int int_var__curr_ptr10;
        void curr_ptr10 (const cpp_int  & l__val);
        cpp_int curr_ptr10() const;
    
        typedef pu_cpp_int< 11 > curr_ptr11_cpp_int_t;
        cpp_int int_var__curr_ptr11;
        void curr_ptr11 (const cpp_int  & l__val);
        cpp_int curr_ptr11() const;
    
        typedef pu_cpp_int< 11 > curr_ptr12_cpp_int_t;
        cpp_int int_var__curr_ptr12;
        void curr_ptr12 (const cpp_int  & l__val);
        cpp_int curr_ptr12() const;
    
        typedef pu_cpp_int< 11 > curr_ptr13_cpp_int_t;
        cpp_int int_var__curr_ptr13;
        void curr_ptr13 (const cpp_int  & l__val);
        cpp_int curr_ptr13() const;
    
        typedef pu_cpp_int< 11 > curr_ptr14_cpp_int_t;
        cpp_int int_var__curr_ptr14;
        void curr_ptr14 (const cpp_int  & l__val);
        cpp_int curr_ptr14() const;
    
        typedef pu_cpp_int< 11 > curr_ptr15_cpp_int_t;
        cpp_int int_var__curr_ptr15;
        void curr_ptr15 (const cpp_int  & l__val);
        cpp_int curr_ptr15() const;
    
}; // cap_txs_csr_sta_scheduler_rr_t
    
class cap_txs_csr_sta_scheduler_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_scheduler_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_scheduler_t(string name = "cap_txs_csr_sta_scheduler_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_scheduler_t();
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
    
        typedef pu_cpp_int< 1 > hbm_init_done_cpp_int_t;
        cpp_int int_var__hbm_init_done;
        void hbm_init_done (const cpp_int  & l__val);
        cpp_int hbm_init_done() const;
    
        typedef pu_cpp_int< 1 > sram_init_done_cpp_int_t;
        cpp_int int_var__sram_init_done;
        void sram_init_done (const cpp_int  & l__val);
        cpp_int sram_init_done() const;
    
}; // cap_txs_csr_sta_scheduler_t
    
class cap_txs_csr_cnt_sch_rlid_start_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_rlid_start_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_rlid_start_t(string name = "cap_txs_csr_cnt_sch_rlid_start_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_rlid_start_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_rlid_start_t
    
class cap_txs_csr_cnt_sch_rlid_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_rlid_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_rlid_stop_t(string name = "cap_txs_csr_cnt_sch_rlid_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_rlid_stop_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_rlid_stop_t
    
class cap_txs_csr_sta_glb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_glb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_glb_t(string name = "cap_txs_csr_sta_glb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_glb_t();
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
    
        typedef pu_cpp_int< 16 > pb_xoff_cpp_int_t;
        cpp_int int_var__pb_xoff;
        void pb_xoff (const cpp_int  & l__val);
        cpp_int pb_xoff() const;
    
}; // cap_txs_csr_sta_glb_t
    
class cap_txs_csr_cnt_sch_txdma_cos15_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos15_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos15_t(string name = "cap_txs_csr_cnt_sch_txdma_cos15_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos15_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos15_t
    
class cap_txs_csr_cnt_sch_txdma_cos14_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos14_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos14_t(string name = "cap_txs_csr_cnt_sch_txdma_cos14_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos14_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos14_t
    
class cap_txs_csr_cnt_sch_txdma_cos13_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos13_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos13_t(string name = "cap_txs_csr_cnt_sch_txdma_cos13_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos13_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos13_t
    
class cap_txs_csr_cnt_sch_txdma_cos12_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos12_t(string name = "cap_txs_csr_cnt_sch_txdma_cos12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos12_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos12_t
    
class cap_txs_csr_cnt_sch_txdma_cos11_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos11_t(string name = "cap_txs_csr_cnt_sch_txdma_cos11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos11_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos11_t
    
class cap_txs_csr_cnt_sch_txdma_cos10_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos10_t(string name = "cap_txs_csr_cnt_sch_txdma_cos10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos10_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos10_t
    
class cap_txs_csr_cnt_sch_txdma_cos9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos9_t(string name = "cap_txs_csr_cnt_sch_txdma_cos9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos9_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos9_t
    
class cap_txs_csr_cnt_sch_txdma_cos8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos8_t(string name = "cap_txs_csr_cnt_sch_txdma_cos8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos8_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos8_t
    
class cap_txs_csr_cnt_sch_txdma_cos7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos7_t(string name = "cap_txs_csr_cnt_sch_txdma_cos7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos7_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos7_t
    
class cap_txs_csr_cnt_sch_txdma_cos6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos6_t(string name = "cap_txs_csr_cnt_sch_txdma_cos6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos6_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos6_t
    
class cap_txs_csr_cnt_sch_txdma_cos5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos5_t(string name = "cap_txs_csr_cnt_sch_txdma_cos5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos5_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos5_t
    
class cap_txs_csr_cnt_sch_txdma_cos4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos4_t(string name = "cap_txs_csr_cnt_sch_txdma_cos4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos4_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos4_t
    
class cap_txs_csr_cnt_sch_txdma_cos3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos3_t(string name = "cap_txs_csr_cnt_sch_txdma_cos3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos3_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos3_t
    
class cap_txs_csr_cnt_sch_txdma_cos2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos2_t(string name = "cap_txs_csr_cnt_sch_txdma_cos2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos2_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos2_t
    
class cap_txs_csr_cnt_sch_txdma_cos1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos1_t(string name = "cap_txs_csr_cnt_sch_txdma_cos1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos1_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos1_t
    
class cap_txs_csr_cnt_sch_txdma_cos0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_txdma_cos0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_txdma_cos0_t(string name = "cap_txs_csr_cnt_sch_txdma_cos0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_txdma_cos0_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos0_t
    
class cap_txs_csr_cnt_sch_fifo_empty_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_fifo_empty_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_fifo_empty_t(string name = "cap_txs_csr_cnt_sch_fifo_empty_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_fifo_empty_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_fifo_empty_t
    
class cap_txs_csr_cnt_sch_doorbell_clr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_doorbell_clr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_doorbell_clr_t(string name = "cap_txs_csr_cnt_sch_doorbell_clr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_doorbell_clr_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_doorbell_clr_t
    
class cap_txs_csr_cnt_sch_doorbell_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_sch_doorbell_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_sch_doorbell_set_t(string name = "cap_txs_csr_cnt_sch_doorbell_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_sch_doorbell_set_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_doorbell_set_t
    
class cap_txs_csr_cfg_sch_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_sch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_sch_t(string name = "cap_txs_csr_cfg_sch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_sch_t();
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
    
        typedef pu_cpp_int< 16 > dtdm_lo_map_cpp_int_t;
        cpp_int int_var__dtdm_lo_map;
        void dtdm_lo_map (const cpp_int  & l__val);
        cpp_int dtdm_lo_map() const;
    
        typedef pu_cpp_int< 16 > dtdm_hi_map_cpp_int_t;
        cpp_int int_var__dtdm_hi_map;
        void dtdm_hi_map (const cpp_int  & l__val);
        cpp_int dtdm_hi_map() const;
    
        typedef pu_cpp_int< 16 > timeout_cpp_int_t;
        cpp_int int_var__timeout;
        void timeout (const cpp_int  & l__val);
        cpp_int timeout() const;
    
        typedef pu_cpp_int< 16 > pause_cpp_int_t;
        cpp_int int_var__pause;
        void pause (const cpp_int  & l__val);
        cpp_int pause() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_txs_csr_cfg_sch_t
    
class cap_txs_csr_cfw_scheduler_static_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfw_scheduler_static_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfw_scheduler_static_t(string name = "cap_txs_csr_cfw_scheduler_static_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfw_scheduler_static_t();
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
    
        typedef pu_cpp_int< 64 > hbm_base_cpp_int_t;
        cpp_int int_var__hbm_base;
        void hbm_base (const cpp_int  & l__val);
        cpp_int hbm_base() const;
    
        typedef pu_cpp_int< 12 > sch_grp_depth_cpp_int_t;
        cpp_int int_var__sch_grp_depth;
        void sch_grp_depth (const cpp_int  & l__val);
        cpp_int sch_grp_depth() const;
    
}; // cap_txs_csr_cfw_scheduler_static_t
    
class cap_txs_csr_cfw_scheduler_glb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfw_scheduler_glb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfw_scheduler_glb_t(string name = "cap_txs_csr_cfw_scheduler_glb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfw_scheduler_glb_t();
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
    
        typedef pu_cpp_int< 1 > hbm_hw_init_cpp_int_t;
        cpp_int int_var__hbm_hw_init;
        void hbm_hw_init (const cpp_int  & l__val);
        cpp_int hbm_hw_init() const;
    
        typedef pu_cpp_int< 1 > sram_hw_init_cpp_int_t;
        cpp_int int_var__sram_hw_init;
        void sram_hw_init (const cpp_int  & l__val);
        cpp_int sram_hw_init() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > enable_set_lkup_cpp_int_t;
        cpp_int int_var__enable_set_lkup;
        void enable_set_lkup (const cpp_int  & l__val);
        cpp_int enable_set_lkup() const;
    
        typedef pu_cpp_int< 1 > enable_set_byp_cpp_int_t;
        cpp_int int_var__enable_set_byp;
        void enable_set_byp (const cpp_int  & l__val);
        cpp_int enable_set_byp() const;
    
}; // cap_txs_csr_cfw_scheduler_glb_t
    
class cap_txs_csr_sta_srams_tmr_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_tmr_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_tmr_fifo_t(string name = "cap_txs_csr_sta_srams_tmr_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_tmr_fifo_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_tmr_fifo_t
    
class cap_txs_csr_sta_srams_tmr_hbm_byp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_tmr_hbm_byp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_tmr_hbm_byp_t(string name = "cap_txs_csr_sta_srams_tmr_hbm_byp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_tmr_hbm_byp_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_tmr_hbm_byp_t
    
class cap_txs_csr_sta_srams_tmr_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_srams_tmr_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_srams_tmr_hbm_t(string name = "cap_txs_csr_sta_srams_tmr_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_srams_tmr_hbm_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_txs_csr_sta_srams_tmr_hbm_t
    
class cap_txs_csr_cfg_tmr_fifo_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_tmr_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_tmr_fifo_sram_t(string name = "cap_txs_csr_cfg_tmr_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_tmr_fifo_sram_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_tmr_fifo_sram_t
    
class cap_txs_csr_cfg_tmr_hbm_byp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_tmr_hbm_byp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_tmr_hbm_byp_sram_t(string name = "cap_txs_csr_cfg_tmr_hbm_byp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_tmr_hbm_byp_sram_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_tmr_hbm_byp_sram_t
    
class cap_txs_csr_cfg_tmr_hbm_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_tmr_hbm_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_tmr_hbm_sram_t(string name = "cap_txs_csr_cfg_tmr_hbm_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_tmr_hbm_sram_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_txs_csr_cfg_tmr_hbm_sram_t
    
class cap_txs_csr_cnt_stmr_pop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_stmr_pop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_stmr_pop_t(string name = "cap_txs_csr_cnt_stmr_pop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_stmr_pop_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_stmr_pop_t
    
class cap_txs_csr_cnt_stmr_key_not_found_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_stmr_key_not_found_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_stmr_key_not_found_t(string name = "cap_txs_csr_cnt_stmr_key_not_found_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_stmr_key_not_found_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_stmr_key_not_found_t
    
class cap_txs_csr_cnt_stmr_push_out_of_wheel_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_stmr_push_out_of_wheel_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_stmr_push_out_of_wheel_t(string name = "cap_txs_csr_cnt_stmr_push_out_of_wheel_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_stmr_push_out_of_wheel_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_stmr_push_out_of_wheel_t
    
class cap_txs_csr_cnt_stmr_key_not_push_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_stmr_key_not_push_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_stmr_key_not_push_t(string name = "cap_txs_csr_cnt_stmr_key_not_push_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_stmr_key_not_push_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_stmr_key_not_push_t
    
class cap_txs_csr_cnt_stmr_push_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_stmr_push_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_stmr_push_t(string name = "cap_txs_csr_cnt_stmr_push_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_stmr_push_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_stmr_push_t
    
class cap_txs_csr_sta_slow_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_slow_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_slow_timer_t(string name = "cap_txs_csr_sta_slow_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_slow_timer_t();
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
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        cpp_int int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 12 > cTime_cpp_int_t;
        cpp_int int_var__cTime;
        void cTime (const cpp_int  & l__val);
        cpp_int cTime() const;
    
        typedef pu_cpp_int< 12 > pTime_cpp_int_t;
        cpp_int int_var__pTime;
        void pTime (const cpp_int  & l__val);
        cpp_int pTime() const;
    
}; // cap_txs_csr_sta_slow_timer_t
    
class cap_txs_csr_cfg_slow_timer_dbell_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_slow_timer_dbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_slow_timer_dbell_t(string name = "cap_txs_csr_cfg_slow_timer_dbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_slow_timer_dbell_t();
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
    
        typedef pu_cpp_int< 6 > addr_update_cpp_int_t;
        cpp_int int_var__addr_update;
        void addr_update (const cpp_int  & l__val);
        cpp_int addr_update() const;
    
        typedef pu_cpp_int< 16 > data_pid_cpp_int_t;
        cpp_int int_var__data_pid;
        void data_pid (const cpp_int  & l__val);
        cpp_int data_pid() const;
    
        typedef pu_cpp_int< 16 > data_reserved_cpp_int_t;
        cpp_int int_var__data_reserved;
        void data_reserved (const cpp_int  & l__val);
        cpp_int data_reserved() const;
    
        typedef pu_cpp_int< 16 > data_index_cpp_int_t;
        cpp_int int_var__data_index;
        void data_index (const cpp_int  & l__val);
        cpp_int data_index() const;
    
}; // cap_txs_csr_cfg_slow_timer_dbell_t
    
class cap_txs_csr_cfg_force_slow_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_force_slow_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_force_slow_timer_t(string name = "cap_txs_csr_cfg_force_slow_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_force_slow_timer_t();
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
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 12 > ctime_cpp_int_t;
        cpp_int int_var__ctime;
        void ctime (const cpp_int  & l__val);
        cpp_int ctime() const;
    
}; // cap_txs_csr_cfg_force_slow_timer_t
    
class cap_txs_csr_cfg_slow_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_slow_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_slow_timer_t(string name = "cap_txs_csr_cfg_slow_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_slow_timer_t();
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
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        cpp_int int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 2 > hash_sel_cpp_int_t;
        cpp_int int_var__hash_sel;
        void hash_sel (const cpp_int  & l__val);
        cpp_int hash_sel() const;
    
}; // cap_txs_csr_cfg_slow_timer_t
    
class cap_txs_csr_cnt_ftmr_pop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_ftmr_pop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_ftmr_pop_t(string name = "cap_txs_csr_cnt_ftmr_pop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_ftmr_pop_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_ftmr_pop_t
    
class cap_txs_csr_cnt_ftmr_key_not_found_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_ftmr_key_not_found_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_ftmr_key_not_found_t(string name = "cap_txs_csr_cnt_ftmr_key_not_found_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_ftmr_key_not_found_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_ftmr_key_not_found_t
    
class cap_txs_csr_cnt_ftmr_push_out_of_wheel_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_ftmr_push_out_of_wheel_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_ftmr_push_out_of_wheel_t(string name = "cap_txs_csr_cnt_ftmr_push_out_of_wheel_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_ftmr_push_out_of_wheel_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_ftmr_push_out_of_wheel_t
    
class cap_txs_csr_cnt_ftmr_key_not_push_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_ftmr_key_not_push_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_ftmr_key_not_push_t(string name = "cap_txs_csr_cnt_ftmr_key_not_push_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_ftmr_key_not_push_t();
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
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_ftmr_key_not_push_t
    
class cap_txs_csr_cnt_ftmr_push_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cnt_ftmr_push_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cnt_ftmr_push_t(string name = "cap_txs_csr_cnt_ftmr_push_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cnt_ftmr_push_t();
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
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_ftmr_push_t
    
class cap_txs_csr_sta_fast_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_fast_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_fast_timer_t(string name = "cap_txs_csr_sta_fast_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_fast_timer_t();
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
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        cpp_int int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 12 > cTime_cpp_int_t;
        cpp_int int_var__cTime;
        void cTime (const cpp_int  & l__val);
        cpp_int cTime() const;
    
        typedef pu_cpp_int< 12 > pTime_cpp_int_t;
        cpp_int int_var__pTime;
        void pTime (const cpp_int  & l__val);
        cpp_int pTime() const;
    
}; // cap_txs_csr_sta_fast_timer_t
    
class cap_txs_csr_cfg_fast_timer_dbell_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_fast_timer_dbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_fast_timer_dbell_t(string name = "cap_txs_csr_cfg_fast_timer_dbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_fast_timer_dbell_t();
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
    
        typedef pu_cpp_int< 6 > addr_update_cpp_int_t;
        cpp_int int_var__addr_update;
        void addr_update (const cpp_int  & l__val);
        cpp_int addr_update() const;
    
        typedef pu_cpp_int< 16 > data_pid_cpp_int_t;
        cpp_int int_var__data_pid;
        void data_pid (const cpp_int  & l__val);
        cpp_int data_pid() const;
    
        typedef pu_cpp_int< 16 > data_reserved_cpp_int_t;
        cpp_int int_var__data_reserved;
        void data_reserved (const cpp_int  & l__val);
        cpp_int data_reserved() const;
    
        typedef pu_cpp_int< 16 > data_index_cpp_int_t;
        cpp_int int_var__data_index;
        void data_index (const cpp_int  & l__val);
        cpp_int data_index() const;
    
}; // cap_txs_csr_cfg_fast_timer_dbell_t
    
class cap_txs_csr_cfg_force_fast_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_force_fast_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_force_fast_timer_t(string name = "cap_txs_csr_cfg_force_fast_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_force_fast_timer_t();
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
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 12 > ctime_cpp_int_t;
        cpp_int int_var__ctime;
        void ctime (const cpp_int  & l__val);
        cpp_int ctime() const;
    
}; // cap_txs_csr_cfg_force_fast_timer_t
    
class cap_txs_csr_cfg_fast_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_fast_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_fast_timer_t(string name = "cap_txs_csr_cfg_fast_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_fast_timer_t();
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
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        cpp_int int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 2 > hash_sel_cpp_int_t;
        cpp_int int_var__hash_sel;
        void hash_sel (const cpp_int  & l__val);
        cpp_int hash_sel() const;
    
}; // cap_txs_csr_cfg_fast_timer_t
    
class cap_txs_csr_sta_timer_dbg2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_timer_dbg2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_timer_dbg2_t(string name = "cap_txs_csr_sta_timer_dbg2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_timer_dbg2_t();
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
    
        typedef pu_cpp_int< 5 > hbm_byp_pending_cnt_cpp_int_t;
        cpp_int int_var__hbm_byp_pending_cnt;
        void hbm_byp_pending_cnt (const cpp_int  & l__val);
        cpp_int hbm_byp_pending_cnt() const;
    
        typedef pu_cpp_int< 5 > hbm_rd_pending_cnt_cpp_int_t;
        cpp_int int_var__hbm_rd_pending_cnt;
        void hbm_rd_pending_cnt (const cpp_int  & l__val);
        cpp_int hbm_rd_pending_cnt() const;
    
        typedef pu_cpp_int< 5 > hbm_wr_pending_cnt_cpp_int_t;
        cpp_int int_var__hbm_wr_pending_cnt;
        void hbm_wr_pending_cnt (const cpp_int  & l__val);
        cpp_int hbm_wr_pending_cnt() const;
    
        typedef pu_cpp_int< 1 > stmr_stall_cpp_int_t;
        cpp_int int_var__stmr_stall;
        void stmr_stall (const cpp_int  & l__val);
        cpp_int stmr_stall() const;
    
        typedef pu_cpp_int< 1 > ftmr_stall_cpp_int_t;
        cpp_int int_var__ftmr_stall;
        void ftmr_stall (const cpp_int  & l__val);
        cpp_int ftmr_stall() const;
    
}; // cap_txs_csr_sta_timer_dbg2_t
    
class cap_txs_csr_sta_timer_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_timer_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_timer_dbg_t(string name = "cap_txs_csr_sta_timer_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_timer_dbg_t();
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
    
        typedef pu_cpp_int< 1 > hbm_re_efull_cpp_int_t;
        cpp_int int_var__hbm_re_efull;
        void hbm_re_efull (const cpp_int  & l__val);
        cpp_int hbm_re_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_we_efull_cpp_int_t;
        cpp_int int_var__hbm_we_efull;
        void hbm_we_efull (const cpp_int  & l__val);
        cpp_int hbm_we_efull() const;
    
        typedef pu_cpp_int< 1 > rejct_drb_efull_cpp_int_t;
        cpp_int int_var__rejct_drb_efull;
        void rejct_drb_efull (const cpp_int  & l__val);
        cpp_int rejct_drb_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_byp_pending_efull_cpp_int_t;
        cpp_int int_var__hbm_byp_pending_efull;
        void hbm_byp_pending_efull (const cpp_int  & l__val);
        cpp_int hbm_byp_pending_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_rd_pending_efull_cpp_int_t;
        cpp_int int_var__hbm_rd_pending_efull;
        void hbm_rd_pending_efull (const cpp_int  & l__val);
        cpp_int hbm_rd_pending_efull() const;
    
        typedef pu_cpp_int< 1 > hbm_wr_pending_efull_cpp_int_t;
        cpp_int int_var__hbm_wr_pending_efull;
        void hbm_wr_pending_efull (const cpp_int  & l__val);
        cpp_int hbm_wr_pending_efull() const;
    
        typedef pu_cpp_int< 4 > stmr_fifo_raddr_cpp_int_t;
        cpp_int int_var__stmr_fifo_raddr;
        void stmr_fifo_raddr (const cpp_int  & l__val);
        cpp_int stmr_fifo_raddr() const;
    
        typedef pu_cpp_int< 4 > ftmr_fifo_raddr_cpp_int_t;
        cpp_int int_var__ftmr_fifo_raddr;
        void ftmr_fifo_raddr (const cpp_int  & l__val);
        cpp_int ftmr_fifo_raddr() const;
    
        typedef pu_cpp_int< 4 > stmr_fifo_waddr_cpp_int_t;
        cpp_int int_var__stmr_fifo_waddr;
        void stmr_fifo_waddr (const cpp_int  & l__val);
        cpp_int stmr_fifo_waddr() const;
    
        typedef pu_cpp_int< 4 > ftmr_fifo_waddr_cpp_int_t;
        cpp_int int_var__ftmr_fifo_waddr;
        void ftmr_fifo_waddr (const cpp_int  & l__val);
        cpp_int ftmr_fifo_waddr() const;
    
        typedef pu_cpp_int< 1 > stmr_fifo_efull_cpp_int_t;
        cpp_int int_var__stmr_fifo_efull;
        void stmr_fifo_efull (const cpp_int  & l__val);
        cpp_int stmr_fifo_efull() const;
    
        typedef pu_cpp_int< 1 > ftmr_fifo_efull_cpp_int_t;
        cpp_int int_var__ftmr_fifo_efull;
        void ftmr_fifo_efull (const cpp_int  & l__val);
        cpp_int ftmr_fifo_efull() const;
    
}; // cap_txs_csr_sta_timer_dbg_t
    
class cap_txs_csr_sta_tmr_max_keys_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_tmr_max_keys_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_tmr_max_keys_t(string name = "cap_txs_csr_sta_tmr_max_keys_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_tmr_max_keys_t();
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
    
        typedef pu_cpp_int< 5 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_txs_csr_sta_tmr_max_keys_t
    
class cap_txs_csr_sta_tmr_max_hbm_byp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_tmr_max_hbm_byp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_tmr_max_hbm_byp_t(string name = "cap_txs_csr_sta_tmr_max_hbm_byp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_tmr_max_hbm_byp_t();
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
    
        typedef pu_cpp_int< 5 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_txs_csr_sta_tmr_max_hbm_byp_t
    
class cap_txs_csr_sta_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_sta_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_sta_timer_t(string name = "cap_txs_csr_sta_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_sta_timer_t();
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
    
        typedef pu_cpp_int< 1 > hbm_init_done_cpp_int_t;
        cpp_int int_var__hbm_init_done;
        void hbm_init_done (const cpp_int  & l__val);
        cpp_int hbm_init_done() const;
    
        typedef pu_cpp_int< 1 > sram_init_done_cpp_int_t;
        cpp_int int_var__sram_init_done;
        void sram_init_done (const cpp_int  & l__val);
        cpp_int sram_init_done() const;
    
}; // cap_txs_csr_sta_timer_t
    
class cap_txs_csr_cfg_timer_dbg2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_timer_dbg2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_timer_dbg2_t(string name = "cap_txs_csr_cfg_timer_dbg2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_timer_dbg2_t();
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
    
        typedef pu_cpp_int< 18 > max_bcnt_cpp_int_t;
        cpp_int int_var__max_bcnt;
        void max_bcnt (const cpp_int  & l__val);
        cpp_int max_bcnt() const;
    
        typedef pu_cpp_int< 16 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_timer_dbg2_t
    
class cap_txs_csr_cfg_timer_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_timer_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_timer_dbg_t(string name = "cap_txs_csr_cfg_timer_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_timer_dbg_t();
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
    
        typedef pu_cpp_int< 3 > hbm_efc_thr_cpp_int_t;
        cpp_int int_var__hbm_efc_thr;
        void hbm_efc_thr (const cpp_int  & l__val);
        cpp_int hbm_efc_thr() const;
    
        typedef pu_cpp_int< 2 > drb_efc_thr_cpp_int_t;
        cpp_int int_var__drb_efc_thr;
        void drb_efc_thr (const cpp_int  & l__val);
        cpp_int drb_efc_thr() const;
    
        typedef pu_cpp_int< 12 > tmr_stall_thr_hi_cpp_int_t;
        cpp_int int_var__tmr_stall_thr_hi;
        void tmr_stall_thr_hi (const cpp_int  & l__val);
        cpp_int tmr_stall_thr_hi() const;
    
        typedef pu_cpp_int< 12 > tmr_stall_thr_lo_cpp_int_t;
        cpp_int int_var__tmr_stall_thr_lo;
        void tmr_stall_thr_lo (const cpp_int  & l__val);
        cpp_int tmr_stall_thr_lo() const;
    
        typedef pu_cpp_int< 4 > max_hbm_wr_cpp_int_t;
        cpp_int int_var__max_hbm_wr;
        void max_hbm_wr (const cpp_int  & l__val);
        cpp_int max_hbm_wr() const;
    
        typedef pu_cpp_int< 4 > max_hbm_rd_cpp_int_t;
        cpp_int int_var__max_hbm_rd;
        void max_hbm_rd (const cpp_int  & l__val);
        cpp_int max_hbm_rd() const;
    
        typedef pu_cpp_int< 4 > max_hbm_byp_cpp_int_t;
        cpp_int int_var__max_hbm_byp;
        void max_hbm_byp (const cpp_int  & l__val);
        cpp_int max_hbm_byp() const;
    
        typedef pu_cpp_int< 4 > max_tmr_fifo_cpp_int_t;
        cpp_int int_var__max_tmr_fifo;
        void max_tmr_fifo (const cpp_int  & l__val);
        cpp_int max_tmr_fifo() const;
    
        typedef pu_cpp_int< 8 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_timer_dbg_t
    
class cap_txs_csr_cfw_timer_glb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfw_timer_glb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfw_timer_glb_t(string name = "cap_txs_csr_cfw_timer_glb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfw_timer_glb_t();
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
    
        typedef pu_cpp_int< 1 > hbm_hw_init_cpp_int_t;
        cpp_int int_var__hbm_hw_init;
        void hbm_hw_init (const cpp_int  & l__val);
        cpp_int hbm_hw_init() const;
    
        typedef pu_cpp_int< 1 > sram_hw_init_cpp_int_t;
        cpp_int int_var__sram_hw_init;
        void sram_hw_init (const cpp_int  & l__val);
        cpp_int sram_hw_init() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > ftmr_enable_cpp_int_t;
        cpp_int int_var__ftmr_enable;
        void ftmr_enable (const cpp_int  & l__val);
        cpp_int ftmr_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_enable_cpp_int_t;
        cpp_int int_var__stmr_enable;
        void stmr_enable (const cpp_int  & l__val);
        cpp_int stmr_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_pause_cpp_int_t;
        cpp_int int_var__ftmr_pause;
        void ftmr_pause (const cpp_int  & l__val);
        cpp_int ftmr_pause() const;
    
        typedef pu_cpp_int< 1 > stmr_pause_cpp_int_t;
        cpp_int int_var__stmr_pause;
        void stmr_pause (const cpp_int  & l__val);
        cpp_int stmr_pause() const;
    
}; // cap_txs_csr_cfw_timer_glb_t
    
class cap_txs_csr_cfg_timer_static_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_timer_static_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_timer_static_t(string name = "cap_txs_csr_cfg_timer_static_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_timer_static_t();
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
    
        typedef pu_cpp_int< 64 > hbm_base_cpp_int_t;
        cpp_int int_var__hbm_base;
        void hbm_base (const cpp_int  & l__val);
        cpp_int hbm_base() const;
    
        typedef pu_cpp_int< 24 > tmr_hsh_depth_cpp_int_t;
        cpp_int int_var__tmr_hsh_depth;
        void tmr_hsh_depth (const cpp_int  & l__val);
        cpp_int tmr_hsh_depth() const;
    
        typedef pu_cpp_int< 12 > tmr_wheel_depth_cpp_int_t;
        cpp_int int_var__tmr_wheel_depth;
        void tmr_wheel_depth (const cpp_int  & l__val);
        cpp_int tmr_wheel_depth() const;
    
}; // cap_txs_csr_cfg_timer_static_t
    
class cap_txs_csr_cfg_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_axi_attr_t(string name = "cap_txs_csr_cfg_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_axi_attr_t();
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
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        cpp_int int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        cpp_int int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_txs_csr_cfg_axi_attr_t
    
class cap_txs_csr_cfg_glb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_cfg_glb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_cfg_glb_t(string name = "cap_txs_csr_cfg_glb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_cfg_glb_t();
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
    
        typedef pu_cpp_int< 2 > dbg_port_select_cpp_int_t;
        cpp_int int_var__dbg_port_select;
        void dbg_port_select (const cpp_int  & l__val);
        cpp_int dbg_port_select() const;
    
        typedef pu_cpp_int< 1 > dbg_port_enable_cpp_int_t;
        cpp_int int_var__dbg_port_enable;
        void dbg_port_enable (const cpp_int  & l__val);
        cpp_int dbg_port_enable() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_glb_t
    
class cap_txs_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_base_t(string name = "cap_txs_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_base_t();
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
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        cpp_int int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_txs_csr_base_t
    
class cap_txs_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_txs_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_txs_csr_t(string name = "cap_txs_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_txs_csr_t();
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
    
        cap_txs_csr_base_t base;
    
        cap_txs_csr_cfg_glb_t cfg_glb;
    
        cap_txs_csr_cfg_axi_attr_t cfg_axi_attr;
    
        cap_txs_csr_cfg_timer_static_t cfg_timer_static;
    
        cap_txs_csr_cfw_timer_glb_t cfw_timer_glb;
    
        cap_txs_csr_cfg_timer_dbg_t cfg_timer_dbg;
    
        cap_txs_csr_cfg_timer_dbg2_t cfg_timer_dbg2;
    
        cap_txs_csr_sta_timer_t sta_timer;
    
        cap_txs_csr_sta_tmr_max_hbm_byp_t sta_tmr_max_hbm_byp;
    
        cap_txs_csr_sta_tmr_max_keys_t sta_tmr_max_keys;
    
        cap_txs_csr_sta_timer_dbg_t sta_timer_dbg;
    
        cap_txs_csr_sta_timer_dbg2_t sta_timer_dbg2;
    
        cap_txs_csr_cfg_fast_timer_t cfg_fast_timer;
    
        cap_txs_csr_cfg_force_fast_timer_t cfg_force_fast_timer;
    
        cap_txs_csr_cfg_fast_timer_dbell_t cfg_fast_timer_dbell;
    
        cap_txs_csr_sta_fast_timer_t sta_fast_timer;
    
        cap_txs_csr_cnt_ftmr_push_t cnt_ftmr_push;
    
        cap_txs_csr_cnt_ftmr_key_not_push_t cnt_ftmr_key_not_push;
    
        cap_txs_csr_cnt_ftmr_push_out_of_wheel_t cnt_ftmr_push_out_of_wheel;
    
        cap_txs_csr_cnt_ftmr_key_not_found_t cnt_ftmr_key_not_found;
    
        cap_txs_csr_cnt_ftmr_pop_t cnt_ftmr_pop;
    
        cap_txs_csr_cfg_slow_timer_t cfg_slow_timer;
    
        cap_txs_csr_cfg_force_slow_timer_t cfg_force_slow_timer;
    
        cap_txs_csr_cfg_slow_timer_dbell_t cfg_slow_timer_dbell;
    
        cap_txs_csr_sta_slow_timer_t sta_slow_timer;
    
        cap_txs_csr_cnt_stmr_push_t cnt_stmr_push;
    
        cap_txs_csr_cnt_stmr_key_not_push_t cnt_stmr_key_not_push;
    
        cap_txs_csr_cnt_stmr_push_out_of_wheel_t cnt_stmr_push_out_of_wheel;
    
        cap_txs_csr_cnt_stmr_key_not_found_t cnt_stmr_key_not_found;
    
        cap_txs_csr_cnt_stmr_pop_t cnt_stmr_pop;
    
        cap_txs_csr_cfg_tmr_hbm_sram_t cfg_tmr_hbm_sram;
    
        cap_txs_csr_cfg_tmr_hbm_byp_sram_t cfg_tmr_hbm_byp_sram;
    
        cap_txs_csr_cfg_tmr_fifo_sram_t cfg_tmr_fifo_sram;
    
        cap_txs_csr_sta_srams_tmr_hbm_t sta_srams_tmr_hbm;
    
        cap_txs_csr_sta_srams_tmr_hbm_byp_t sta_srams_tmr_hbm_byp;
    
        cap_txs_csr_sta_srams_tmr_fifo_t sta_srams_tmr_fifo;
    
        cap_txs_csr_cfw_scheduler_glb_t cfw_scheduler_glb;
    
        cap_txs_csr_cfw_scheduler_static_t cfw_scheduler_static;
    
        cap_txs_csr_cfg_sch_t cfg_sch;
    
        cap_txs_csr_cnt_sch_doorbell_set_t cnt_sch_doorbell_set;
    
        cap_txs_csr_cnt_sch_doorbell_clr_t cnt_sch_doorbell_clr;
    
        cap_txs_csr_cnt_sch_fifo_empty_t cnt_sch_fifo_empty;
    
        cap_txs_csr_cnt_sch_txdma_cos0_t cnt_sch_txdma_cos0;
    
        cap_txs_csr_cnt_sch_txdma_cos1_t cnt_sch_txdma_cos1;
    
        cap_txs_csr_cnt_sch_txdma_cos2_t cnt_sch_txdma_cos2;
    
        cap_txs_csr_cnt_sch_txdma_cos3_t cnt_sch_txdma_cos3;
    
        cap_txs_csr_cnt_sch_txdma_cos4_t cnt_sch_txdma_cos4;
    
        cap_txs_csr_cnt_sch_txdma_cos5_t cnt_sch_txdma_cos5;
    
        cap_txs_csr_cnt_sch_txdma_cos6_t cnt_sch_txdma_cos6;
    
        cap_txs_csr_cnt_sch_txdma_cos7_t cnt_sch_txdma_cos7;
    
        cap_txs_csr_cnt_sch_txdma_cos8_t cnt_sch_txdma_cos8;
    
        cap_txs_csr_cnt_sch_txdma_cos9_t cnt_sch_txdma_cos9;
    
        cap_txs_csr_cnt_sch_txdma_cos10_t cnt_sch_txdma_cos10;
    
        cap_txs_csr_cnt_sch_txdma_cos11_t cnt_sch_txdma_cos11;
    
        cap_txs_csr_cnt_sch_txdma_cos12_t cnt_sch_txdma_cos12;
    
        cap_txs_csr_cnt_sch_txdma_cos13_t cnt_sch_txdma_cos13;
    
        cap_txs_csr_cnt_sch_txdma_cos14_t cnt_sch_txdma_cos14;
    
        cap_txs_csr_cnt_sch_txdma_cos15_t cnt_sch_txdma_cos15;
    
        cap_txs_csr_sta_glb_t sta_glb;
    
        cap_txs_csr_cnt_sch_rlid_stop_t cnt_sch_rlid_stop;
    
        cap_txs_csr_cnt_sch_rlid_start_t cnt_sch_rlid_start;
    
        cap_txs_csr_sta_scheduler_t sta_scheduler;
    
        cap_txs_csr_sta_scheduler_rr_t sta_scheduler_rr;
    
        cap_txs_csr_cfg_scheduler_dbg_t cfg_scheduler_dbg;
    
        cap_txs_csr_cfg_scheduler_dbg2_t cfg_scheduler_dbg2;
    
        cap_txs_csr_sta_sch_max_hbm_byp_t sta_sch_max_hbm_byp;
    
        cap_txs_csr_cfg_tmr_cnt_sram_t cfg_tmr_cnt_sram;
    
        cap_txs_csr_cfg_sch_lif_map_sram_t cfg_sch_lif_map_sram;
    
        cap_txs_csr_cfg_sch_rlid_map_sram_t cfg_sch_rlid_map_sram;
    
        cap_txs_csr_cfg_sch_grp_sram_t cfg_sch_grp_sram;
    
        cap_txs_csr_sta_sch_lif_map_notactive_t sta_sch_lif_map_notactive;
    
        cap_txs_csr_sta_sch_lif_sg_mismatch_t sta_sch_lif_sg_mismatch;
    
        cap_txs_csr_sta_scheduler_dbg_t sta_scheduler_dbg;
    
        cap_txs_csr_sta_scheduler_dbg2_t sta_scheduler_dbg2;
    
        cap_txs_csr_sta_srams_ecc_tmr_cnt_t sta_srams_ecc_tmr_cnt;
    
        cap_txs_csr_sta_srams_ecc_sch_lif_map_t sta_srams_ecc_sch_lif_map;
    
        cap_txs_csr_sta_srams_ecc_sch_rlid_map_t sta_srams_ecc_sch_rlid_map;
    
        cap_txs_csr_sta_srams_ecc_sch_grp_t sta_srams_ecc_sch_grp;
    
        cap_txs_csr_cfg_sch_hbm_sram_t cfg_sch_hbm_sram;
    
        cap_txs_csr_cfg_sch_hbm_byp_sram_t cfg_sch_hbm_byp_sram;
    
        cap_txs_csr_sta_srams_sch_hbm_t sta_srams_sch_hbm;
    
        cap_txs_csr_sta_srams_sch_hbm_byp_t sta_srams_sch_hbm_byp;
    
        cap_txs_csr_csr_intr_t csr_intr;
    
        cap_txs_csr_sta_ftmr_max_bcnt_t sta_ftmr_max_bcnt;
    
        cap_txs_csr_sta_stmr_max_bcnt_t sta_stmr_max_bcnt;
    
        cap_txs_csr_cnt_sch_axi_rd_req_t cnt_sch_axi_rd_req;
    
        cap_txs_csr_cnt_sch_axi_rd_rsp_t cnt_sch_axi_rd_rsp;
    
        cap_txs_csr_cnt_sch_axi_rd_rsp_err_t cnt_sch_axi_rd_rsp_err;
    
        cap_txs_csr_cnt_sch_axi_rd_rsp_rerr_t cnt_sch_axi_rd_rsp_rerr;
    
        cap_txs_csr_cnt_sch_axi_rd_rsp_uexp_t cnt_sch_axi_rd_rsp_uexp;
    
        cap_txs_csr_cnt_sch_axi_wr_req_t cnt_sch_axi_wr_req;
    
        cap_txs_csr_cnt_sch_axi_bid_t cnt_sch_axi_bid;
    
        cap_txs_csr_cnt_sch_axi_bid_err_t cnt_sch_axi_bid_err;
    
        cap_txs_csr_cnt_sch_axi_bid_rerr_t cnt_sch_axi_bid_rerr;
    
        cap_txs_csr_cnt_sch_axi_bid_uexp_t cnt_sch_axi_bid_uexp;
    
        cap_txs_csr_cnt_tmr_axi_rd_req_t cnt_tmr_axi_rd_req;
    
        cap_txs_csr_cnt_tmr_axi_rd_rsp_t cnt_tmr_axi_rd_rsp;
    
        cap_txs_csr_cnt_tmr_axi_rd_rsp_err_t cnt_tmr_axi_rd_rsp_err;
    
        cap_txs_csr_cnt_tmr_axi_rd_rsp_rerr_t cnt_tmr_axi_rd_rsp_rerr;
    
        cap_txs_csr_cnt_tmr_axi_rd_rsp_uexp_t cnt_tmr_axi_rd_rsp_uexp;
    
        cap_txs_csr_cnt_tmr_axi_wr_req_t cnt_tmr_axi_wr_req;
    
        cap_txs_csr_cnt_tmr_axi_bid_t cnt_tmr_axi_bid;
    
        cap_txs_csr_cnt_tmr_axi_bid_err_t cnt_tmr_axi_bid_err;
    
        cap_txs_csr_cnt_tmr_axi_bid_rerr_t cnt_tmr_axi_bid_rerr;
    
        cap_txs_csr_cnt_tmr_axi_bid_uexp_t cnt_tmr_axi_bid_uexp;
    
        cap_txs_csr_dhs_fast_timer_start_no_stop_t dhs_fast_timer_start_no_stop;
    
        cap_txs_csr_dhs_fast_timer_pending_t dhs_fast_timer_pending;
    
        cap_txs_csr_dhs_slow_timer_start_no_stop_t dhs_slow_timer_start_no_stop;
    
        cap_txs_csr_dhs_slow_timer_pending_t dhs_slow_timer_pending;
    
        cap_txs_csr_dhs_dtdmlo_calendar_t dhs_dtdmlo_calendar;
    
        cap_txs_csr_dhs_dtdmhi_calendar_t dhs_dtdmhi_calendar;
    
        cap_txs_csr_dhs_sch_flags_t dhs_sch_flags;
    
        cap_txs_csr_dhs_doorbell_t dhs_doorbell;
    
        cap_txs_csr_dhs_sch_grp_entry_t dhs_sch_grp_entry;
    
        cap_txs_csr_dhs_sch_grp_cnt_entry_t dhs_sch_grp_cnt_entry;
    
        cap_txs_csr_dhs_rlid_stop_t dhs_rlid_stop;
    
        cap_txs_csr_dhs_tmr_cnt_sram_t dhs_tmr_cnt_sram;
    
        cap_txs_csr_dhs_sch_lif_map_sram_t dhs_sch_lif_map_sram;
    
        cap_txs_csr_dhs_sch_rlid_map_sram_t dhs_sch_rlid_map_sram;
    
        cap_txs_csr_dhs_sch_grp_sram_t dhs_sch_grp_sram;
    
        cap_txs_csr_int_srams_ecc_t int_srams_ecc;
    
        cap_txs_csr_intgrp_status_t int_groups;
    
        cap_txs_csr_int_sch_t int_sch;
    
        cap_txs_csr_int_tmr_t int_tmr;
    
}; // cap_txs_csr_t
    
#endif // CAP_TXS_CSR_H
        