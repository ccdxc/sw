
#ifndef CAP_PTD_CSR_H
#define CAP_PTD_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_ptd_csr_cfg_spare_dbg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_cfg_spare_dbg_t(string name = "cap_ptd_csr_cfg_spare_dbg_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_cfg_spare_dbg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ptd_csr_cfg_spare_dbg_t
    
class cap_ptd_csr_CNT_ptd_pb_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_CNT_ptd_pb_t(string name = "cap_ptd_csr_CNT_ptd_pb_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_CNT_ptd_pb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ptd_csr_CNT_ptd_pb_t
    
class cap_ptd_csr_CNT_ptd_npv_resub_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_CNT_ptd_npv_resub_t(string name = "cap_ptd_csr_CNT_ptd_npv_resub_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_CNT_ptd_npv_resub_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ptd_csr_CNT_ptd_npv_resub_t
    
class cap_ptd_csr_CNT_ptd_ma_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_CNT_ptd_ma_t(string name = "cap_ptd_csr_CNT_ptd_ma_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_CNT_ptd_ma_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ptd_csr_CNT_ptd_ma_t
    
class cap_ptd_csr_cfg_xoff_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_cfg_xoff_t(string name = "cap_ptd_csr_cfg_xoff_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_cfg_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 11 > numphv_thresh_cpp_int_t;
        numphv_thresh_cpp_int_t int_var__numphv_thresh;
        void numphv_thresh (const cpp_int  & l__val);
        cpp_int numphv_thresh() const;
    
}; // cap_ptd_csr_cfg_xoff_t
    
class cap_ptd_csr_axi_attr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_axi_attr_t(string name = "cap_ptd_csr_axi_attr_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_axi_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > arcache_0_cpp_int_t;
        arcache_0_cpp_int_t int_var__arcache_0;
        void arcache_0 (const cpp_int  & l__val);
        cpp_int arcache_0() const;
    
        typedef pu_cpp_int< 4 > awcache_0_cpp_int_t;
        awcache_0_cpp_int_t int_var__awcache_0;
        void awcache_0 (const cpp_int  & l__val);
        cpp_int awcache_0() const;
    
        typedef pu_cpp_int< 4 > arcache_1_cpp_int_t;
        arcache_1_cpp_int_t int_var__arcache_1;
        void arcache_1 (const cpp_int  & l__val);
        cpp_int arcache_1() const;
    
        typedef pu_cpp_int< 4 > awcache_1_cpp_int_t;
        awcache_1_cpp_int_t int_var__awcache_1;
        void awcache_1 (const cpp_int  & l__val);
        cpp_int awcache_1() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > arqos_cpp_int_t;
        arqos_cpp_int_t int_var__arqos;
        void arqos (const cpp_int  & l__val);
        cpp_int arqos() const;
    
        typedef pu_cpp_int< 4 > awqos_0_cpp_int_t;
        awqos_0_cpp_int_t int_var__awqos_0;
        void awqos_0 (const cpp_int  & l__val);
        cpp_int awqos_0() const;
    
        typedef pu_cpp_int< 4 > awqos_1_cpp_int_t;
        awqos_1_cpp_int_t int_var__awqos_1;
        void awqos_1 (const cpp_int  & l__val);
        cpp_int awqos_1() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_ptd_csr_axi_attr_t
    
class cap_ptd_csr_sta_id_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_sta_id_t(string name = "cap_ptd_csr_sta_id_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_sta_id_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > wr_pend_cnt_cpp_int_t;
        wr_pend_cnt_cpp_int_t int_var__wr_pend_cnt;
        void wr_pend_cnt (const cpp_int  & l__val);
        cpp_int wr_pend_cnt() const;
    
        typedef pu_cpp_int< 8 > rd_pend_cnt_cpp_int_t;
        rd_pend_cnt_cpp_int_t int_var__rd_pend_cnt;
        void rd_pend_cnt (const cpp_int  & l__val);
        cpp_int rd_pend_cnt() const;
    
}; // cap_ptd_csr_sta_id_t
    
class cap_ptd_csr_cfg_ctrl_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_cfg_ctrl_t(string name = "cap_ptd_csr_cfg_ctrl_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_cfg_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > max_rd_req_cnt_cpp_int_t;
        max_rd_req_cnt_cpp_int_t int_var__max_rd_req_cnt;
        void max_rd_req_cnt (const cpp_int  & l__val);
        cpp_int max_rd_req_cnt() const;
    
        typedef pu_cpp_int< 8 > max_wr_req_cnt_cpp_int_t;
        max_wr_req_cnt_cpp_int_t int_var__max_wr_req_cnt;
        void max_wr_req_cnt (const cpp_int  & l__val);
        cpp_int max_wr_req_cnt() const;
    
        typedef pu_cpp_int< 9 > wdata_ff_thresh_cpp_int_t;
        wdata_ff_thresh_cpp_int_t int_var__wdata_ff_thresh;
        void wdata_ff_thresh (const cpp_int  & l__val);
        cpp_int wdata_ff_thresh() const;
    
        typedef pu_cpp_int< 4 > pkt_ff_thresh_cpp_int_t;
        pkt_ff_thresh_cpp_int_t int_var__pkt_ff_thresh;
        void pkt_ff_thresh (const cpp_int  & l__val);
        cpp_int pkt_ff_thresh() const;
    
        typedef pu_cpp_int< 4 > mem_ff_thresh_cpp_int_t;
        mem_ff_thresh_cpp_int_t int_var__mem_ff_thresh;
        void mem_ff_thresh (const cpp_int  & l__val);
        cpp_int mem_ff_thresh() const;
    
}; // cap_ptd_csr_cfg_ctrl_t
    
class cap_ptd_csr_cfg_debug_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_cfg_debug_port_t(string name = "cap_ptd_csr_cfg_debug_port_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_cfg_debug_port_t();
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
    
        typedef pu_cpp_int< 1 > select_cpp_int_t;
        select_cpp_int_t int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_ptd_csr_cfg_debug_port_t
    
class cap_ptd_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_base_t(string name = "cap_ptd_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_base_t();
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
    
}; // cap_ptd_csr_base_t
    
class cap_ptd_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_ptd_csr_t(string name = "cap_ptd_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ptd_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ptd_csr_base_t base;
    
        cap_ptd_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_ptd_csr_cfg_ctrl_t cfg_ctrl;
    
        cap_ptd_csr_sta_id_t sta_id;
    
        cap_ptd_csr_axi_attr_t axi_attr;
    
        cap_ptd_csr_cfg_xoff_t cfg_xoff;
    
        cap_ptd_csr_CNT_ptd_ma_t CNT_ptd_ma;
    
        cap_ptd_csr_CNT_ptd_npv_resub_t CNT_ptd_npv_resub;
    
        cap_ptd_csr_CNT_ptd_pb_t CNT_ptd_pb;
    
        cap_ptd_csr_cfg_spare_dbg_t cfg_spare_dbg;
    
}; // cap_ptd_csr_t
    
#endif // CAP_PTD_CSR_H
        