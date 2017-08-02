
#ifndef CAP_MPU_CSR_H
#define CAP_MPU_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mpu_csr_CNT_sdp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_CNT_sdp_t(string name = "cap_mpu_csr_CNT_sdp_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_CNT_sdp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > sop_in_cpp_int_t;
        sop_in_cpp_int_t int_var__sop_in;
        void sop_in (const cpp_int  & l__val);
        cpp_int sop_in() const;
    
        typedef pu_cpp_int< 16 > eop_in_cpp_int_t;
        eop_in_cpp_int_t int_var__eop_in;
        void eop_in (const cpp_int  & l__val);
        cpp_int eop_in() const;
    
        typedef pu_cpp_int< 16 > sop_out_cpp_int_t;
        sop_out_cpp_int_t int_var__sop_out;
        void sop_out (const cpp_int  & l__val);
        cpp_int sop_out() const;
    
        typedef pu_cpp_int< 16 > eop_out_cpp_int_t;
        eop_out_cpp_int_t int_var__eop_out;
        void eop_out (const cpp_int  & l__val);
        cpp_int eop_out() const;
    
}; // cap_mpu_csr_CNT_sdp_t
    
class cap_mpu_csr_sta_error_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_sta_error_t(string name = "cap_mpu_csr_sta_error_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_sta_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > icache0_cpp_int_t;
        icache0_cpp_int_t int_var__icache0;
        void icache0 (const cpp_int  & l__val);
        cpp_int icache0() const;
    
        typedef pu_cpp_int< 1 > icache1_cpp_int_t;
        icache1_cpp_int_t int_var__icache1;
        void icache1 (const cpp_int  & l__val);
        cpp_int icache1() const;
    
}; // cap_mpu_csr_sta_error_t
    
class cap_mpu_csr_sta_mpu_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_sta_mpu_t(string name = "cap_mpu_csr_sta_mpu_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_sta_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > stopped_cpp_int_t;
        stopped_cpp_int_t int_var__stopped;
        void stopped (const cpp_int  & l__val);
        cpp_int stopped() const;
    
        typedef pu_cpp_int< 1 > mpu_processing_cpp_int_t;
        mpu_processing_cpp_int_t int_var__mpu_processing;
        void mpu_processing (const cpp_int  & l__val);
        cpp_int mpu_processing() const;
    
        typedef pu_cpp_int< 8 > mpu_processing_pkt_id_cpp_int_t;
        mpu_processing_pkt_id_cpp_int_t int_var__mpu_processing_pkt_id;
        void mpu_processing_pkt_id (const cpp_int  & l__val);
        cpp_int mpu_processing_pkt_id() const;
    
        typedef pu_cpp_int< 4 > mpu_processing_table_id_cpp_int_t;
        mpu_processing_table_id_cpp_int_t int_var__mpu_processing_table_id;
        void mpu_processing_table_id (const cpp_int  & l__val);
        cpp_int mpu_processing_table_id() const;
    
        typedef pu_cpp_int< 64 > mpu_processing_table_addr_cpp_int_t;
        mpu_processing_table_addr_cpp_int_t int_var__mpu_processing_table_addr;
        void mpu_processing_table_addr (const cpp_int  & l__val);
        cpp_int mpu_processing_table_addr() const;
    
        typedef pu_cpp_int< 512 > td_cpp_int_t;
        td_cpp_int_t int_var__td;
        void td (const cpp_int  & l__val);
        cpp_int td() const;
    
        typedef pu_cpp_int< 512 > kd_cpp_int_t;
        kd_cpp_int_t int_var__kd;
        void kd (const cpp_int  & l__val);
        cpp_int kd() const;
    
        typedef pu_cpp_int< 30 > ex_pc_cpp_int_t;
        ex_pc_cpp_int_t int_var__ex_pc;
        void ex_pc (const cpp_int  & l__val);
        cpp_int ex_pc() const;
    
        typedef pu_cpp_int< 64 > gpr1_cpp_int_t;
        gpr1_cpp_int_t int_var__gpr1;
        void gpr1 (const cpp_int  & l__val);
        cpp_int gpr1() const;
    
        typedef pu_cpp_int< 64 > gpr2_cpp_int_t;
        gpr2_cpp_int_t int_var__gpr2;
        void gpr2 (const cpp_int  & l__val);
        cpp_int gpr2() const;
    
        typedef pu_cpp_int< 64 > gpr3_cpp_int_t;
        gpr3_cpp_int_t int_var__gpr3;
        void gpr3 (const cpp_int  & l__val);
        cpp_int gpr3() const;
    
        typedef pu_cpp_int< 64 > gpr4_cpp_int_t;
        gpr4_cpp_int_t int_var__gpr4;
        void gpr4 (const cpp_int  & l__val);
        cpp_int gpr4() const;
    
        typedef pu_cpp_int< 64 > gpr5_cpp_int_t;
        gpr5_cpp_int_t int_var__gpr5;
        void gpr5 (const cpp_int  & l__val);
        cpp_int gpr5() const;
    
        typedef pu_cpp_int< 64 > gpr6_cpp_int_t;
        gpr6_cpp_int_t int_var__gpr6;
        void gpr6 (const cpp_int  & l__val);
        cpp_int gpr6() const;
    
        typedef pu_cpp_int< 64 > gpr7_cpp_int_t;
        gpr7_cpp_int_t int_var__gpr7;
        void gpr7 (const cpp_int  & l__val);
        cpp_int gpr7() const;
    
        typedef pu_cpp_int< 1 > c1_cpp_int_t;
        c1_cpp_int_t int_var__c1;
        void c1 (const cpp_int  & l__val);
        cpp_int c1() const;
    
        typedef pu_cpp_int< 1 > c2_cpp_int_t;
        c2_cpp_int_t int_var__c2;
        void c2 (const cpp_int  & l__val);
        cpp_int c2() const;
    
        typedef pu_cpp_int< 1 > c3_cpp_int_t;
        c3_cpp_int_t int_var__c3;
        void c3 (const cpp_int  & l__val);
        cpp_int c3() const;
    
        typedef pu_cpp_int< 1 > c4_cpp_int_t;
        c4_cpp_int_t int_var__c4;
        void c4 (const cpp_int  & l__val);
        cpp_int c4() const;
    
        typedef pu_cpp_int< 1 > c5_cpp_int_t;
        c5_cpp_int_t int_var__c5;
        void c5 (const cpp_int  & l__val);
        cpp_int c5() const;
    
        typedef pu_cpp_int< 1 > c6_cpp_int_t;
        c6_cpp_int_t int_var__c6;
        void c6 (const cpp_int  & l__val);
        cpp_int c6() const;
    
        typedef pu_cpp_int< 1 > c7_cpp_int_t;
        c7_cpp_int_t int_var__c7;
        void c7 (const cpp_int  & l__val);
        cpp_int c7() const;
    
}; // cap_mpu_csr_sta_mpu_t
    
class cap_mpu_csr_CNT_stall_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_CNT_stall_t(string name = "cap_mpu_csr_CNT_stall_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_CNT_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > inst_executed_cpp_int_t;
        inst_executed_cpp_int_t int_var__inst_executed;
        void inst_executed (const cpp_int  & l__val);
        cpp_int inst_executed() const;
    
        typedef pu_cpp_int< 8 > gensrc_stall_cpp_int_t;
        gensrc_stall_cpp_int_t int_var__gensrc_stall;
        void gensrc_stall (const cpp_int  & l__val);
        cpp_int gensrc_stall() const;
    
        typedef pu_cpp_int< 8 > phvwr_stall_cpp_int_t;
        phvwr_stall_cpp_int_t int_var__phvwr_stall;
        void phvwr_stall (const cpp_int  & l__val);
        cpp_int phvwr_stall() const;
    
        typedef pu_cpp_int< 16 > icache_fill_stall_cpp_int_t;
        icache_fill_stall_cpp_int_t int_var__icache_fill_stall;
        void icache_fill_stall (const cpp_int  & l__val);
        cpp_int icache_fill_stall() const;
    
}; // cap_mpu_csr_CNT_stall_t
    
class cap_mpu_csr_mpu_run_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_mpu_run_t(string name = "cap_mpu_csr_mpu_run_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_mpu_run_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > start_pulse_cpp_int_t;
        start_pulse_cpp_int_t int_var__start_pulse;
        void start_pulse (const cpp_int  & l__val);
        cpp_int start_pulse() const;
    
        typedef pu_cpp_int< 1 > stop_pulse_cpp_int_t;
        stop_pulse_cpp_int_t int_var__stop_pulse;
        void stop_pulse (const cpp_int  & l__val);
        cpp_int stop_pulse() const;
    
        typedef pu_cpp_int< 1 > step_pulse_cpp_int_t;
        step_pulse_cpp_int_t int_var__step_pulse;
        void step_pulse (const cpp_int  & l__val);
        cpp_int step_pulse() const;
    
}; // cap_mpu_csr_mpu_run_t
    
class cap_mpu_csr_mpu_ctl_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_mpu_ctl_t(string name = "cap_mpu_csr_mpu_ctl_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_mpu_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_stop_cpp_int_t;
        enable_stop_cpp_int_t int_var__enable_stop;
        void enable_stop (const cpp_int  & l__val);
        cpp_int enable_stop() const;
    
        typedef pu_cpp_int< 1 > enable_illegal_stop_cpp_int_t;
        enable_illegal_stop_cpp_int_t int_var__enable_illegal_stop;
        void enable_illegal_stop (const cpp_int  & l__val);
        cpp_int enable_illegal_stop() const;
    
}; // cap_mpu_csr_mpu_ctl_t
    
class cap_mpu_csr_count_stage_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_count_stage_t(string name = "cap_mpu_csr_count_stage_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_count_stage_t();
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
    
        typedef pu_cpp_int< 1 > debug_cpp_int_t;
        debug_cpp_int_t int_var__debug;
        void debug (const cpp_int  & l__val);
        cpp_int debug() const;
    
}; // cap_mpu_csr_count_stage_t
    
class cap_mpu_csr_axi_attr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_axi_attr_t(string name = "cap_mpu_csr_axi_attr_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_axi_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        arcache_cpp_int_t int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        awcache_cpp_int_t int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_mpu_csr_axi_attr_t
    
class cap_mpu_csr_icache_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_icache_t(string name = "cap_mpu_csr_icache_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_icache_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > invalidate_cpp_int_t;
        invalidate_cpp_int_t int_var__invalidate;
        void invalidate (const cpp_int  & l__val);
        cpp_int invalidate() const;
    
}; // cap_mpu_csr_icache_t
    
class cap_mpu_csr_trace_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_trace_t(string name = "cap_mpu_csr_trace_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_trace_t();
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
    
        typedef pu_cpp_int< 1 > phv_debug_cpp_int_t;
        phv_debug_cpp_int_t int_var__phv_debug;
        void phv_debug (const cpp_int  & l__val);
        cpp_int phv_debug() const;
    
        typedef pu_cpp_int< 1 > wrap_cpp_int_t;
        wrap_cpp_int_t int_var__wrap;
        void wrap (const cpp_int  & l__val);
        cpp_int wrap() const;
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        rst_cpp_int_t int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
        typedef pu_cpp_int< 27 > base_addr_cpp_int_t;
        base_addr_cpp_int_t int_var__base_addr;
        void base_addr (const cpp_int  & l__val);
        cpp_int base_addr() const;
    
        typedef pu_cpp_int< 5 > buf_size_cpp_int_t;
        buf_size_cpp_int_t int_var__buf_size;
        void buf_size (const cpp_int  & l__val);
        cpp_int buf_size() const;
    
}; // cap_mpu_csr_trace_t
    
class cap_mpu_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_mpu_csr_t(string name = "cap_mpu_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_mpu_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_mpu_csr_trace_t trace[4];
        int get_depth_trace() { return 4; }
    
        cap_mpu_csr_icache_t icache;
    
        cap_mpu_csr_axi_attr_t axi_attr;
    
        cap_mpu_csr_count_stage_t count_stage;
    
        cap_mpu_csr_mpu_ctl_t mpu_ctl;
    
        cap_mpu_csr_mpu_run_t mpu_run[4];
        int get_depth_mpu_run() { return 4; }
    
        cap_mpu_csr_CNT_stall_t CNT_stall[4];
        int get_depth_CNT_stall() { return 4; }
    
        cap_mpu_csr_sta_mpu_t sta_mpu[4];
        int get_depth_sta_mpu() { return 4; }
    
        cap_mpu_csr_sta_error_t sta_error[4];
        int get_depth_sta_error() { return 4; }
    
        cap_mpu_csr_CNT_sdp_t CNT_sdp;
    
}; // cap_mpu_csr_t
    
#endif // CAP_MPU_CSR_H
        