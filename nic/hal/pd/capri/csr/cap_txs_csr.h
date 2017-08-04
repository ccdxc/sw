
#ifndef CAP_TXS_CSR_H
#define CAP_TXS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_txs_csr_dhs_sch_grp_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_grp_sram_entry_t(string name = "cap_txs_csr_dhs_sch_grp_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_grp_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 11 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 11 > qid_offset_cpp_int_t;
        qid_offset_cpp_int_t int_var__qid_offset;
        void qid_offset (const cpp_int  & l__val);
        cpp_int qid_offset() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        lif_cpp_int_t int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 1 > auto_clr_cpp_int_t;
        auto_clr_cpp_int_t int_var__auto_clr;
        void auto_clr (const cpp_int  & l__val);
        cpp_int auto_clr() const;
    
        typedef pu_cpp_int< 4 > rr_sel_cpp_int_t;
        rr_sel_cpp_int_t int_var__rr_sel;
        void rr_sel (const cpp_int  & l__val);
        cpp_int rr_sel() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm0_cpp_int_t;
        qid_fifo_elm0_cpp_int_t int_var__qid_fifo_elm0;
        void qid_fifo_elm0 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm0() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm1_cpp_int_t;
        qid_fifo_elm1_cpp_int_t int_var__qid_fifo_elm1;
        void qid_fifo_elm1 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm1() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm2_cpp_int_t;
        qid_fifo_elm2_cpp_int_t int_var__qid_fifo_elm2;
        void qid_fifo_elm2 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm2() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm3_cpp_int_t;
        qid_fifo_elm3_cpp_int_t int_var__qid_fifo_elm3;
        void qid_fifo_elm3 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm3() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm4_cpp_int_t;
        qid_fifo_elm4_cpp_int_t int_var__qid_fifo_elm4;
        void qid_fifo_elm4 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm4() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm5_cpp_int_t;
        qid_fifo_elm5_cpp_int_t int_var__qid_fifo_elm5;
        void qid_fifo_elm5 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm5() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm6_cpp_int_t;
        qid_fifo_elm6_cpp_int_t int_var__qid_fifo_elm6;
        void qid_fifo_elm6 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm6() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm7_cpp_int_t;
        qid_fifo_elm7_cpp_int_t int_var__qid_fifo_elm7;
        void qid_fifo_elm7 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm7() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm8_cpp_int_t;
        qid_fifo_elm8_cpp_int_t int_var__qid_fifo_elm8;
        void qid_fifo_elm8 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm8() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm9_cpp_int_t;
        qid_fifo_elm9_cpp_int_t int_var__qid_fifo_elm9;
        void qid_fifo_elm9 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm9() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm10_cpp_int_t;
        qid_fifo_elm10_cpp_int_t int_var__qid_fifo_elm10;
        void qid_fifo_elm10 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm10() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm11_cpp_int_t;
        qid_fifo_elm11_cpp_int_t int_var__qid_fifo_elm11;
        void qid_fifo_elm11 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm11() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm12_cpp_int_t;
        qid_fifo_elm12_cpp_int_t int_var__qid_fifo_elm12;
        void qid_fifo_elm12 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm12() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm13_cpp_int_t;
        qid_fifo_elm13_cpp_int_t int_var__qid_fifo_elm13;
        void qid_fifo_elm13 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm13() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm14_cpp_int_t;
        qid_fifo_elm14_cpp_int_t int_var__qid_fifo_elm14;
        void qid_fifo_elm14 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm14() const;
    
        typedef pu_cpp_int< 15 > qid_fifo_elm15_cpp_int_t;
        qid_fifo_elm15_cpp_int_t int_var__qid_fifo_elm15;
        void qid_fifo_elm15 (const cpp_int  & l__val);
        cpp_int qid_fifo_elm15() const;
    
        typedef pu_cpp_int< 4 > hbm_ln_cpp_int_t;
        hbm_ln_cpp_int_t int_var__hbm_ln;
        void hbm_ln (const cpp_int  & l__val);
        cpp_int hbm_ln() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr0_cpp_int_t;
        hbm_ln_ptr0_cpp_int_t int_var__hbm_ln_ptr0;
        void hbm_ln_ptr0 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr0() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr1_cpp_int_t;
        hbm_ln_ptr1_cpp_int_t int_var__hbm_ln_ptr1;
        void hbm_ln_ptr1 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr1() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr2_cpp_int_t;
        hbm_ln_ptr2_cpp_int_t int_var__hbm_ln_ptr2;
        void hbm_ln_ptr2 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr2() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr3_cpp_int_t;
        hbm_ln_ptr3_cpp_int_t int_var__hbm_ln_ptr3;
        void hbm_ln_ptr3 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr3() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr4_cpp_int_t;
        hbm_ln_ptr4_cpp_int_t int_var__hbm_ln_ptr4;
        void hbm_ln_ptr4 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr4() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr5_cpp_int_t;
        hbm_ln_ptr5_cpp_int_t int_var__hbm_ln_ptr5;
        void hbm_ln_ptr5 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr5() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr6_cpp_int_t;
        hbm_ln_ptr6_cpp_int_t int_var__hbm_ln_ptr6;
        void hbm_ln_ptr6 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr6() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr7_cpp_int_t;
        hbm_ln_ptr7_cpp_int_t int_var__hbm_ln_ptr7;
        void hbm_ln_ptr7 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr7() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr8_cpp_int_t;
        hbm_ln_ptr8_cpp_int_t int_var__hbm_ln_ptr8;
        void hbm_ln_ptr8 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr8() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr9_cpp_int_t;
        hbm_ln_ptr9_cpp_int_t int_var__hbm_ln_ptr9;
        void hbm_ln_ptr9 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr9() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr10_cpp_int_t;
        hbm_ln_ptr10_cpp_int_t int_var__hbm_ln_ptr10;
        void hbm_ln_ptr10 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr10() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr11_cpp_int_t;
        hbm_ln_ptr11_cpp_int_t int_var__hbm_ln_ptr11;
        void hbm_ln_ptr11 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr11() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr12_cpp_int_t;
        hbm_ln_ptr12_cpp_int_t int_var__hbm_ln_ptr12;
        void hbm_ln_ptr12 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr12() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr13_cpp_int_t;
        hbm_ln_ptr13_cpp_int_t int_var__hbm_ln_ptr13;
        void hbm_ln_ptr13 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr13() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr14_cpp_int_t;
        hbm_ln_ptr14_cpp_int_t int_var__hbm_ln_ptr14;
        void hbm_ln_ptr14 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr14() const;
    
        typedef pu_cpp_int< 9 > hbm_ln_ptr15_cpp_int_t;
        hbm_ln_ptr15_cpp_int_t int_var__hbm_ln_ptr15;
        void hbm_ln_ptr15 (const cpp_int  & l__val);
        cpp_int hbm_ln_ptr15() const;
    
        typedef pu_cpp_int< 14 > drb_cnt_cpp_int_t;
        drb_cnt_cpp_int_t int_var__drb_cnt;
        void drb_cnt (const cpp_int  & l__val);
        cpp_int drb_cnt() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt0_cpp_int_t;
        hbm_ln_cnt0_cpp_int_t int_var__hbm_ln_cnt0;
        void hbm_ln_cnt0 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt0() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt1_cpp_int_t;
        hbm_ln_cnt1_cpp_int_t int_var__hbm_ln_cnt1;
        void hbm_ln_cnt1 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt1() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt2_cpp_int_t;
        hbm_ln_cnt2_cpp_int_t int_var__hbm_ln_cnt2;
        void hbm_ln_cnt2 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt2() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt3_cpp_int_t;
        hbm_ln_cnt3_cpp_int_t int_var__hbm_ln_cnt3;
        void hbm_ln_cnt3 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt3() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt4_cpp_int_t;
        hbm_ln_cnt4_cpp_int_t int_var__hbm_ln_cnt4;
        void hbm_ln_cnt4 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt4() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt5_cpp_int_t;
        hbm_ln_cnt5_cpp_int_t int_var__hbm_ln_cnt5;
        void hbm_ln_cnt5 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt5() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt6_cpp_int_t;
        hbm_ln_cnt6_cpp_int_t int_var__hbm_ln_cnt6;
        void hbm_ln_cnt6 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt6() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt7_cpp_int_t;
        hbm_ln_cnt7_cpp_int_t int_var__hbm_ln_cnt7;
        void hbm_ln_cnt7 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt7() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt8_cpp_int_t;
        hbm_ln_cnt8_cpp_int_t int_var__hbm_ln_cnt8;
        void hbm_ln_cnt8 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt8() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt9_cpp_int_t;
        hbm_ln_cnt9_cpp_int_t int_var__hbm_ln_cnt9;
        void hbm_ln_cnt9 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt9() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt10_cpp_int_t;
        hbm_ln_cnt10_cpp_int_t int_var__hbm_ln_cnt10;
        void hbm_ln_cnt10 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt10() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt11_cpp_int_t;
        hbm_ln_cnt11_cpp_int_t int_var__hbm_ln_cnt11;
        void hbm_ln_cnt11 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt11() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt12_cpp_int_t;
        hbm_ln_cnt12_cpp_int_t int_var__hbm_ln_cnt12;
        void hbm_ln_cnt12 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt12() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt13_cpp_int_t;
        hbm_ln_cnt13_cpp_int_t int_var__hbm_ln_cnt13;
        void hbm_ln_cnt13 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt13() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt14_cpp_int_t;
        hbm_ln_cnt14_cpp_int_t int_var__hbm_ln_cnt14;
        void hbm_ln_cnt14 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt14() const;
    
        typedef pu_cpp_int< 10 > hbm_ln_cnt15_cpp_int_t;
        hbm_ln_cnt15_cpp_int_t int_var__hbm_ln_cnt15;
        void hbm_ln_cnt15 (const cpp_int  & l__val);
        cpp_int hbm_ln_cnt15() const;
    
        typedef pu_cpp_int< 40 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_sch_grp_sram_entry_t
    
class cap_txs_csr_dhs_sch_grp_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_grp_sram_t(string name = "cap_txs_csr_dhs_sch_grp_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_grp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_grp_sram_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_grp_sram_t
    
class cap_txs_csr_dhs_sch_rlid_map_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_rlid_map_sram_entry_t(string name = "cap_txs_csr_dhs_sch_rlid_map_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_rlid_map_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 11 > sg_start_cpp_int_t;
        sg_start_cpp_int_t int_var__sg_start;
        void sg_start (const cpp_int  & l__val);
        cpp_int sg_start() const;
    
        typedef pu_cpp_int< 11 > sg_end_cpp_int_t;
        sg_end_cpp_int_t int_var__sg_end;
        void sg_end (const cpp_int  & l__val);
        cpp_int sg_end() const;
    
        typedef pu_cpp_int< 6 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_sch_rlid_map_sram_entry_t
    
class cap_txs_csr_dhs_sch_rlid_map_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_rlid_map_sram_t(string name = "cap_txs_csr_dhs_sch_rlid_map_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_rlid_map_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_rlid_map_sram_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_rlid_map_sram_t
    
class cap_txs_csr_dhs_sch_lif_map_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_lif_map_sram_entry_t(string name = "cap_txs_csr_dhs_sch_lif_map_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_lif_map_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 11 > sg_start_cpp_int_t;
        sg_start_cpp_int_t int_var__sg_start;
        void sg_start (const cpp_int  & l__val);
        cpp_int sg_start() const;
    
        typedef pu_cpp_int< 11 > sg_per_cos_cpp_int_t;
        sg_per_cos_cpp_int_t int_var__sg_per_cos;
        void sg_per_cos (const cpp_int  & l__val);
        cpp_int sg_per_cos() const;
    
        typedef pu_cpp_int< 6 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_sch_lif_map_sram_entry_t
    
class cap_txs_csr_dhs_sch_lif_map_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_lif_map_sram_t(string name = "cap_txs_csr_dhs_sch_lif_map_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_lif_map_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_lif_map_sram_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_lif_map_sram_t
    
class cap_txs_csr_dhs_tmr_cnt_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_tmr_cnt_sram_entry_t(string name = "cap_txs_csr_dhs_tmr_cnt_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_tmr_cnt_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 19 > slow_bcnt_cpp_int_t;
        slow_bcnt_cpp_int_t int_var__slow_bcnt;
        void slow_bcnt (const cpp_int  & l__val);
        cpp_int slow_bcnt() const;
    
        typedef pu_cpp_int< 4 > slow_lcnt_cpp_int_t;
        slow_lcnt_cpp_int_t int_var__slow_lcnt;
        void slow_lcnt (const cpp_int  & l__val);
        cpp_int slow_lcnt() const;
    
        typedef pu_cpp_int< 19 > fast_bcnt_cpp_int_t;
        fast_bcnt_cpp_int_t int_var__fast_bcnt;
        void fast_bcnt (const cpp_int  & l__val);
        cpp_int fast_bcnt() const;
    
        typedef pu_cpp_int< 4 > fast_lcnt_cpp_int_t;
        fast_lcnt_cpp_int_t int_var__fast_lcnt;
        void fast_lcnt (const cpp_int  & l__val);
        cpp_int fast_lcnt() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_txs_csr_dhs_tmr_cnt_sram_entry_t
    
class cap_txs_csr_dhs_tmr_cnt_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_tmr_cnt_sram_t(string name = "cap_txs_csr_dhs_tmr_cnt_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_tmr_cnt_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_tmr_cnt_sram_entry_t entry[4096];
        int get_depth_entry() { return 4096; }
    
}; // cap_txs_csr_dhs_tmr_cnt_sram_t
    
class cap_txs_csr_dhs_rlid_stop_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_rlid_stop_entry_t(string name = "cap_txs_csr_dhs_rlid_stop_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_rlid_stop_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > setmsk_cpp_int_t;
        setmsk_cpp_int_t int_var__setmsk;
        void setmsk (const cpp_int  & l__val);
        cpp_int setmsk() const;
    
}; // cap_txs_csr_dhs_rlid_stop_entry_t
    
class cap_txs_csr_dhs_rlid_stop_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_rlid_stop_t(string name = "cap_txs_csr_dhs_rlid_stop_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_rlid_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_rlid_stop_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_rlid_stop_t
    
class cap_txs_csr_dhs_sch_grp_entry_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_grp_entry_entry_t(string name = "cap_txs_csr_dhs_sch_grp_entry_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_grp_entry_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        lif_cpp_int_t int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 11 > qid_offset_cpp_int_t;
        qid_offset_cpp_int_t int_var__qid_offset;
        void qid_offset (const cpp_int  & l__val);
        cpp_int qid_offset() const;
    
        typedef pu_cpp_int< 1 > auto_clr_cpp_int_t;
        auto_clr_cpp_int_t int_var__auto_clr;
        void auto_clr (const cpp_int  & l__val);
        cpp_int auto_clr() const;
    
        typedef pu_cpp_int< 4 > rr_sel_cpp_int_t;
        rr_sel_cpp_int_t int_var__rr_sel;
        void rr_sel (const cpp_int  & l__val);
        cpp_int rr_sel() const;
    
}; // cap_txs_csr_dhs_sch_grp_entry_entry_t
    
class cap_txs_csr_dhs_sch_grp_entry_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_grp_entry_t(string name = "cap_txs_csr_dhs_sch_grp_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_grp_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_grp_entry_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_sch_grp_entry_t
    
class cap_txs_csr_dhs_doorbell_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_doorbell_entry_t(string name = "cap_txs_csr_dhs_doorbell_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 4 > cos_cpp_int_t;
        cos_cpp_int_t int_var__cos;
        void cos (const cpp_int  & l__val);
        cpp_int cos() const;
    
        typedef pu_cpp_int< 1 > set_cpp_int_t;
        set_cpp_int_t int_var__set;
        void set (const cpp_int  & l__val);
        cpp_int set() const;
    
}; // cap_txs_csr_dhs_doorbell_entry_t
    
class cap_txs_csr_dhs_doorbell_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_doorbell_t(string name = "cap_txs_csr_dhs_doorbell_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_doorbell_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_doorbell_t
    
class cap_txs_csr_dhs_sch_flags15_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags15_entry_t(string name = "cap_txs_csr_dhs_sch_flags15_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags15_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags15_entry_t
    
class cap_txs_csr_dhs_sch_flags15_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags15_t(string name = "cap_txs_csr_dhs_sch_flags15_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags15_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags15_t
    
class cap_txs_csr_dhs_sch_flags14_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags14_entry_t(string name = "cap_txs_csr_dhs_sch_flags14_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags14_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags14_entry_t
    
class cap_txs_csr_dhs_sch_flags14_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags14_t(string name = "cap_txs_csr_dhs_sch_flags14_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags14_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags14_t
    
class cap_txs_csr_dhs_sch_flags13_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags13_entry_t(string name = "cap_txs_csr_dhs_sch_flags13_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags13_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags13_entry_t
    
class cap_txs_csr_dhs_sch_flags13_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags13_t(string name = "cap_txs_csr_dhs_sch_flags13_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags13_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags13_t
    
class cap_txs_csr_dhs_sch_flags12_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags12_entry_t(string name = "cap_txs_csr_dhs_sch_flags12_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags12_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags12_entry_t
    
class cap_txs_csr_dhs_sch_flags12_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags12_t(string name = "cap_txs_csr_dhs_sch_flags12_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags12_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags12_t
    
class cap_txs_csr_dhs_sch_flags11_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags11_entry_t(string name = "cap_txs_csr_dhs_sch_flags11_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags11_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags11_entry_t
    
class cap_txs_csr_dhs_sch_flags11_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags11_t(string name = "cap_txs_csr_dhs_sch_flags11_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags11_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags11_t
    
class cap_txs_csr_dhs_sch_flags10_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags10_entry_t(string name = "cap_txs_csr_dhs_sch_flags10_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags10_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags10_entry_t
    
class cap_txs_csr_dhs_sch_flags10_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags10_t(string name = "cap_txs_csr_dhs_sch_flags10_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags10_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags10_t
    
class cap_txs_csr_dhs_sch_flags9_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags9_entry_t(string name = "cap_txs_csr_dhs_sch_flags9_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags9_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags9_entry_t
    
class cap_txs_csr_dhs_sch_flags9_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags9_t(string name = "cap_txs_csr_dhs_sch_flags9_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags9_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags9_t
    
class cap_txs_csr_dhs_sch_flags8_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags8_entry_t(string name = "cap_txs_csr_dhs_sch_flags8_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags8_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags8_entry_t
    
class cap_txs_csr_dhs_sch_flags8_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags8_t(string name = "cap_txs_csr_dhs_sch_flags8_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags8_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags8_t
    
class cap_txs_csr_dhs_sch_flags7_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags7_entry_t(string name = "cap_txs_csr_dhs_sch_flags7_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags7_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags7_entry_t
    
class cap_txs_csr_dhs_sch_flags7_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags7_t(string name = "cap_txs_csr_dhs_sch_flags7_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags7_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags7_t
    
class cap_txs_csr_dhs_sch_flags6_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags6_entry_t(string name = "cap_txs_csr_dhs_sch_flags6_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags6_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags6_entry_t
    
class cap_txs_csr_dhs_sch_flags6_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags6_t(string name = "cap_txs_csr_dhs_sch_flags6_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags6_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags6_t
    
class cap_txs_csr_dhs_sch_flags5_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags5_entry_t(string name = "cap_txs_csr_dhs_sch_flags5_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags5_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags5_entry_t
    
class cap_txs_csr_dhs_sch_flags5_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags5_t(string name = "cap_txs_csr_dhs_sch_flags5_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags5_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags5_t
    
class cap_txs_csr_dhs_sch_flags4_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags4_entry_t(string name = "cap_txs_csr_dhs_sch_flags4_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags4_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags4_entry_t
    
class cap_txs_csr_dhs_sch_flags4_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags4_t(string name = "cap_txs_csr_dhs_sch_flags4_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags4_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags4_t
    
class cap_txs_csr_dhs_sch_flags3_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags3_entry_t(string name = "cap_txs_csr_dhs_sch_flags3_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags3_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags3_entry_t
    
class cap_txs_csr_dhs_sch_flags3_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags3_t(string name = "cap_txs_csr_dhs_sch_flags3_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags3_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags3_t
    
class cap_txs_csr_dhs_sch_flags2_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags2_entry_t(string name = "cap_txs_csr_dhs_sch_flags2_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags2_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags2_entry_t
    
class cap_txs_csr_dhs_sch_flags2_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags2_t(string name = "cap_txs_csr_dhs_sch_flags2_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags2_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags2_t
    
class cap_txs_csr_dhs_sch_flags1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags1_entry_t(string name = "cap_txs_csr_dhs_sch_flags1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags1_entry_t
    
class cap_txs_csr_dhs_sch_flags1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags1_t(string name = "cap_txs_csr_dhs_sch_flags1_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags1_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags1_t
    
class cap_txs_csr_dhs_sch_flags0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags0_entry_t(string name = "cap_txs_csr_dhs_sch_flags0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sch_flags_cpp_int_t;
        sch_flags_cpp_int_t int_var__sch_flags;
        void sch_flags (const cpp_int  & l__val);
        cpp_int sch_flags() const;
    
}; // cap_txs_csr_dhs_sch_flags0_entry_t
    
class cap_txs_csr_dhs_sch_flags0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_sch_flags0_t(string name = "cap_txs_csr_dhs_sch_flags0_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_sch_flags0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_sch_flags0_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_sch_flags0_t
    
class cap_txs_csr_dhs_dtdmhi_calendar_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_dtdmhi_calendar_entry_t(string name = "cap_txs_csr_dhs_dtdmhi_calendar_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_dtdmhi_calendar_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > dtdm_calendar_cpp_int_t;
        dtdm_calendar_cpp_int_t int_var__dtdm_calendar;
        void dtdm_calendar (const cpp_int  & l__val);
        cpp_int dtdm_calendar() const;
    
}; // cap_txs_csr_dhs_dtdmhi_calendar_entry_t
    
class cap_txs_csr_dhs_dtdmhi_calendar_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_dtdmhi_calendar_t(string name = "cap_txs_csr_dhs_dtdmhi_calendar_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_dtdmhi_calendar_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_dtdmhi_calendar_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_dtdmhi_calendar_t
    
class cap_txs_csr_dhs_dtdmlo_calendar_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_dtdmlo_calendar_entry_t(string name = "cap_txs_csr_dhs_dtdmlo_calendar_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_dtdmlo_calendar_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > dtdm_calendar_cpp_int_t;
        dtdm_calendar_cpp_int_t int_var__dtdm_calendar;
        void dtdm_calendar (const cpp_int  & l__val);
        cpp_int dtdm_calendar() const;
    
}; // cap_txs_csr_dhs_dtdmlo_calendar_entry_t
    
class cap_txs_csr_dhs_dtdmlo_calendar_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_dtdmlo_calendar_t(string name = "cap_txs_csr_dhs_dtdmlo_calendar_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_dtdmlo_calendar_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_dtdmlo_calendar_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_txs_csr_dhs_dtdmlo_calendar_t
    
class cap_txs_csr_dhs_slow_timer_pending_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_slow_timer_pending_entry_t(string name = "cap_txs_csr_dhs_slow_timer_pending_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_slow_timer_pending_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 19 > bcnt_cpp_int_t;
        bcnt_cpp_int_t int_var__bcnt;
        void bcnt (const cpp_int  & l__val);
        cpp_int bcnt() const;
    
        typedef pu_cpp_int< 4 > lcnt_cpp_int_t;
        lcnt_cpp_int_t int_var__lcnt;
        void lcnt (const cpp_int  & l__val);
        cpp_int lcnt() const;
    
}; // cap_txs_csr_dhs_slow_timer_pending_entry_t
    
class cap_txs_csr_dhs_slow_timer_pending_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_slow_timer_pending_t(string name = "cap_txs_csr_dhs_slow_timer_pending_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_slow_timer_pending_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_slow_timer_pending_entry_t entry[4096];
        int get_depth_entry() { return 4096; }
    
}; // cap_txs_csr_dhs_slow_timer_pending_t
    
class cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t(string name = "cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > typ_cpp_int_t;
        typ_cpp_int_t int_var__typ;
        void typ (const cpp_int  & l__val);
        cpp_int typ() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        ring_cpp_int_t int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 10 > dtime_cpp_int_t;
        dtime_cpp_int_t int_var__dtime;
        void dtime (const cpp_int  & l__val);
        cpp_int dtime() const;
    
}; // cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t
    
class cap_txs_csr_dhs_slow_timer_start_no_stop_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_slow_timer_start_no_stop_t(string name = "cap_txs_csr_dhs_slow_timer_start_no_stop_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_slow_timer_start_no_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_slow_timer_start_no_stop_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_slow_timer_start_no_stop_t
    
class cap_txs_csr_dhs_fast_timer_pending_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_fast_timer_pending_entry_t(string name = "cap_txs_csr_dhs_fast_timer_pending_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_fast_timer_pending_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 19 > bcnt_cpp_int_t;
        bcnt_cpp_int_t int_var__bcnt;
        void bcnt (const cpp_int  & l__val);
        cpp_int bcnt() const;
    
        typedef pu_cpp_int< 4 > lcnt_cpp_int_t;
        lcnt_cpp_int_t int_var__lcnt;
        void lcnt (const cpp_int  & l__val);
        cpp_int lcnt() const;
    
}; // cap_txs_csr_dhs_fast_timer_pending_entry_t
    
class cap_txs_csr_dhs_fast_timer_pending_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_fast_timer_pending_t(string name = "cap_txs_csr_dhs_fast_timer_pending_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_fast_timer_pending_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_fast_timer_pending_entry_t entry[4096];
        int get_depth_entry() { return 4096; }
    
}; // cap_txs_csr_dhs_fast_timer_pending_t
    
class cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t(string name = "cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > typ_cpp_int_t;
        typ_cpp_int_t int_var__typ;
        void typ (const cpp_int  & l__val);
        cpp_int typ() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        ring_cpp_int_t int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 10 > dtime_cpp_int_t;
        dtime_cpp_int_t int_var__dtime;
        void dtime (const cpp_int  & l__val);
        cpp_int dtime() const;
    
}; // cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t
    
class cap_txs_csr_dhs_fast_timer_start_no_stop_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_dhs_fast_timer_start_no_stop_t(string name = "cap_txs_csr_dhs_fast_timer_start_no_stop_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_dhs_fast_timer_start_no_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_dhs_fast_timer_start_no_stop_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_txs_csr_dhs_fast_timer_start_no_stop_t
    
class cap_txs_csr_sta_srams_ecc_sch_grp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_srams_ecc_sch_grp_t(string name = "cap_txs_csr_sta_srams_ecc_sch_grp_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_srams_ecc_sch_grp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 40 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_txs_csr_sta_srams_ecc_sch_grp_t
    
class cap_txs_csr_sta_srams_ecc_sch_rlid_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_srams_ecc_sch_rlid_map_t(string name = "cap_txs_csr_sta_srams_ecc_sch_rlid_map_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_srams_ecc_sch_rlid_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 6 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_txs_csr_sta_srams_ecc_sch_rlid_map_t
    
class cap_txs_csr_sta_srams_ecc_sch_lif_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_srams_ecc_sch_lif_map_t(string name = "cap_txs_csr_sta_srams_ecc_sch_lif_map_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_srams_ecc_sch_lif_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 6 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_txs_csr_sta_srams_ecc_sch_lif_map_t
    
class cap_txs_csr_sta_srams_ecc_tmr_cnt_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_srams_ecc_tmr_cnt_t(string name = "cap_txs_csr_sta_srams_ecc_tmr_cnt_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_srams_ecc_tmr_cnt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_txs_csr_sta_srams_ecc_tmr_cnt_t
    
class cap_txs_csr_cfg_srams_ecc_disable_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_srams_ecc_disable_t(string name = "cap_txs_csr_cfg_srams_ecc_disable_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_srams_ecc_disable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > tmr_cnt_cor_cpp_int_t;
        tmr_cnt_cor_cpp_int_t int_var__tmr_cnt_cor;
        void tmr_cnt_cor (const cpp_int  & l__val);
        cpp_int tmr_cnt_cor() const;
    
        typedef pu_cpp_int< 1 > tmr_cnt_det_cpp_int_t;
        tmr_cnt_det_cpp_int_t int_var__tmr_cnt_det;
        void tmr_cnt_det (const cpp_int  & l__val);
        cpp_int tmr_cnt_det() const;
    
        typedef pu_cpp_int< 1 > sch_lif_map_cor_cpp_int_t;
        sch_lif_map_cor_cpp_int_t int_var__sch_lif_map_cor;
        void sch_lif_map_cor (const cpp_int  & l__val);
        cpp_int sch_lif_map_cor() const;
    
        typedef pu_cpp_int< 1 > sch_lif_map_det_cpp_int_t;
        sch_lif_map_det_cpp_int_t int_var__sch_lif_map_det;
        void sch_lif_map_det (const cpp_int  & l__val);
        cpp_int sch_lif_map_det() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_map_cor_cpp_int_t;
        sch_rlid_map_cor_cpp_int_t int_var__sch_rlid_map_cor;
        void sch_rlid_map_cor (const cpp_int  & l__val);
        cpp_int sch_rlid_map_cor() const;
    
        typedef pu_cpp_int< 1 > sch_rlid_map_det_cpp_int_t;
        sch_rlid_map_det_cpp_int_t int_var__sch_rlid_map_det;
        void sch_rlid_map_det (const cpp_int  & l__val);
        cpp_int sch_rlid_map_det() const;
    
        typedef pu_cpp_int< 1 > sch_grp_cor_cpp_int_t;
        sch_grp_cor_cpp_int_t int_var__sch_grp_cor;
        void sch_grp_cor (const cpp_int  & l__val);
        cpp_int sch_grp_cor() const;
    
        typedef pu_cpp_int< 1 > sch_grp_det_cpp_int_t;
        sch_grp_det_cpp_int_t int_var__sch_grp_det;
        void sch_grp_det (const cpp_int  & l__val);
        cpp_int sch_grp_det() const;
    
}; // cap_txs_csr_cfg_srams_ecc_disable_t
    
class cap_txs_csr_cfg_scheduler_dbg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_scheduler_dbg_t(string name = "cap_txs_csr_cfg_scheduler_dbg_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_scheduler_dbg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > hbm_efc_thr_cpp_int_t;
        hbm_efc_thr_cpp_int_t int_var__hbm_efc_thr;
        void hbm_efc_thr (const cpp_int  & l__val);
        cpp_int hbm_efc_thr() const;
    
        typedef pu_cpp_int< 2 > txdma_efc_thr_cpp_int_t;
        txdma_efc_thr_cpp_int_t int_var__txdma_efc_thr;
        void txdma_efc_thr (const cpp_int  & l__val);
        cpp_int txdma_efc_thr() const;
    
        typedef pu_cpp_int< 5 > fifo_mode_thr_cpp_int_t;
        fifo_mode_thr_cpp_int_t int_var__fifo_mode_thr;
        void fifo_mode_thr (const cpp_int  & l__val);
        cpp_int fifo_mode_thr() const;
    
        typedef pu_cpp_int< 8 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_scheduler_dbg_t
    
class cap_txs_csr_sta_scheduler_rr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_scheduler_rr_t(string name = "cap_txs_csr_sta_scheduler_rr_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_scheduler_rr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 11 > curr_ptr0_cpp_int_t;
        curr_ptr0_cpp_int_t int_var__curr_ptr0;
        void curr_ptr0 (const cpp_int  & l__val);
        cpp_int curr_ptr0() const;
    
        typedef pu_cpp_int< 11 > curr_ptr1_cpp_int_t;
        curr_ptr1_cpp_int_t int_var__curr_ptr1;
        void curr_ptr1 (const cpp_int  & l__val);
        cpp_int curr_ptr1() const;
    
        typedef pu_cpp_int< 11 > curr_ptr2_cpp_int_t;
        curr_ptr2_cpp_int_t int_var__curr_ptr2;
        void curr_ptr2 (const cpp_int  & l__val);
        cpp_int curr_ptr2() const;
    
        typedef pu_cpp_int< 11 > curr_ptr3_cpp_int_t;
        curr_ptr3_cpp_int_t int_var__curr_ptr3;
        void curr_ptr3 (const cpp_int  & l__val);
        cpp_int curr_ptr3() const;
    
        typedef pu_cpp_int< 11 > curr_ptr4_cpp_int_t;
        curr_ptr4_cpp_int_t int_var__curr_ptr4;
        void curr_ptr4 (const cpp_int  & l__val);
        cpp_int curr_ptr4() const;
    
        typedef pu_cpp_int< 11 > curr_ptr5_cpp_int_t;
        curr_ptr5_cpp_int_t int_var__curr_ptr5;
        void curr_ptr5 (const cpp_int  & l__val);
        cpp_int curr_ptr5() const;
    
        typedef pu_cpp_int< 11 > curr_ptr6_cpp_int_t;
        curr_ptr6_cpp_int_t int_var__curr_ptr6;
        void curr_ptr6 (const cpp_int  & l__val);
        cpp_int curr_ptr6() const;
    
        typedef pu_cpp_int< 11 > curr_ptr7_cpp_int_t;
        curr_ptr7_cpp_int_t int_var__curr_ptr7;
        void curr_ptr7 (const cpp_int  & l__val);
        cpp_int curr_ptr7() const;
    
        typedef pu_cpp_int< 11 > curr_ptr8_cpp_int_t;
        curr_ptr8_cpp_int_t int_var__curr_ptr8;
        void curr_ptr8 (const cpp_int  & l__val);
        cpp_int curr_ptr8() const;
    
        typedef pu_cpp_int< 11 > curr_ptr9_cpp_int_t;
        curr_ptr9_cpp_int_t int_var__curr_ptr9;
        void curr_ptr9 (const cpp_int  & l__val);
        cpp_int curr_ptr9() const;
    
        typedef pu_cpp_int< 11 > curr_ptr10_cpp_int_t;
        curr_ptr10_cpp_int_t int_var__curr_ptr10;
        void curr_ptr10 (const cpp_int  & l__val);
        cpp_int curr_ptr10() const;
    
        typedef pu_cpp_int< 11 > curr_ptr11_cpp_int_t;
        curr_ptr11_cpp_int_t int_var__curr_ptr11;
        void curr_ptr11 (const cpp_int  & l__val);
        cpp_int curr_ptr11() const;
    
        typedef pu_cpp_int< 11 > curr_ptr12_cpp_int_t;
        curr_ptr12_cpp_int_t int_var__curr_ptr12;
        void curr_ptr12 (const cpp_int  & l__val);
        cpp_int curr_ptr12() const;
    
        typedef pu_cpp_int< 11 > curr_ptr13_cpp_int_t;
        curr_ptr13_cpp_int_t int_var__curr_ptr13;
        void curr_ptr13 (const cpp_int  & l__val);
        cpp_int curr_ptr13() const;
    
        typedef pu_cpp_int< 11 > curr_ptr14_cpp_int_t;
        curr_ptr14_cpp_int_t int_var__curr_ptr14;
        void curr_ptr14 (const cpp_int  & l__val);
        cpp_int curr_ptr14() const;
    
        typedef pu_cpp_int< 11 > curr_ptr15_cpp_int_t;
        curr_ptr15_cpp_int_t int_var__curr_ptr15;
        void curr_ptr15 (const cpp_int  & l__val);
        cpp_int curr_ptr15() const;
    
}; // cap_txs_csr_sta_scheduler_rr_t
    
class cap_txs_csr_sta_scheduler_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_scheduler_t(string name = "cap_txs_csr_sta_scheduler_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_scheduler_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hbm_init_done_cpp_int_t;
        hbm_init_done_cpp_int_t int_var__hbm_init_done;
        void hbm_init_done (const cpp_int  & l__val);
        cpp_int hbm_init_done() const;
    
        typedef pu_cpp_int< 1 > sram_init_done_cpp_int_t;
        sram_init_done_cpp_int_t int_var__sram_init_done;
        void sram_init_done (const cpp_int  & l__val);
        cpp_int sram_init_done() const;
    
}; // cap_txs_csr_sta_scheduler_t
    
class cap_txs_csr_sta_glb_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_glb_t(string name = "cap_txs_csr_sta_glb_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_glb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pb_xoff_cpp_int_t;
        pb_xoff_cpp_int_t int_var__pb_xoff;
        void pb_xoff (const cpp_int  & l__val);
        cpp_int pb_xoff() const;
    
}; // cap_txs_csr_sta_glb_t
    
class cap_txs_csr_cnt_sch_txdma_cos15_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos15_t(string name = "cap_txs_csr_cnt_sch_txdma_cos15_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos15_t
    
class cap_txs_csr_cnt_sch_txdma_cos14_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos14_t(string name = "cap_txs_csr_cnt_sch_txdma_cos14_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos14_t
    
class cap_txs_csr_cnt_sch_txdma_cos13_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos13_t(string name = "cap_txs_csr_cnt_sch_txdma_cos13_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos13_t
    
class cap_txs_csr_cnt_sch_txdma_cos12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos12_t(string name = "cap_txs_csr_cnt_sch_txdma_cos12_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos12_t
    
class cap_txs_csr_cnt_sch_txdma_cos11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos11_t(string name = "cap_txs_csr_cnt_sch_txdma_cos11_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos11_t
    
class cap_txs_csr_cnt_sch_txdma_cos10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos10_t(string name = "cap_txs_csr_cnt_sch_txdma_cos10_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos10_t
    
class cap_txs_csr_cnt_sch_txdma_cos9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos9_t(string name = "cap_txs_csr_cnt_sch_txdma_cos9_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos9_t
    
class cap_txs_csr_cnt_sch_txdma_cos8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos8_t(string name = "cap_txs_csr_cnt_sch_txdma_cos8_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos8_t
    
class cap_txs_csr_cnt_sch_txdma_cos7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos7_t(string name = "cap_txs_csr_cnt_sch_txdma_cos7_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos7_t
    
class cap_txs_csr_cnt_sch_txdma_cos6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos6_t(string name = "cap_txs_csr_cnt_sch_txdma_cos6_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos6_t
    
class cap_txs_csr_cnt_sch_txdma_cos5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos5_t(string name = "cap_txs_csr_cnt_sch_txdma_cos5_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos5_t
    
class cap_txs_csr_cnt_sch_txdma_cos4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos4_t(string name = "cap_txs_csr_cnt_sch_txdma_cos4_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos4_t
    
class cap_txs_csr_cnt_sch_txdma_cos3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos3_t(string name = "cap_txs_csr_cnt_sch_txdma_cos3_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos3_t
    
class cap_txs_csr_cnt_sch_txdma_cos2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos2_t(string name = "cap_txs_csr_cnt_sch_txdma_cos2_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos2_t
    
class cap_txs_csr_cnt_sch_txdma_cos1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos1_t(string name = "cap_txs_csr_cnt_sch_txdma_cos1_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos1_t
    
class cap_txs_csr_cnt_sch_txdma_cos0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_txdma_cos0_t(string name = "cap_txs_csr_cnt_sch_txdma_cos0_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_txdma_cos0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_txdma_cos0_t
    
class cap_txs_csr_cnt_sch_doorbell_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_sch_doorbell_t(string name = "cap_txs_csr_cnt_sch_doorbell_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_sch_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_sch_doorbell_t
    
class cap_txs_csr_cfg_sch_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_sch_t(string name = "cap_txs_csr_cfg_sch_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_sch_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dtdm_lo_map_cpp_int_t;
        dtdm_lo_map_cpp_int_t int_var__dtdm_lo_map;
        void dtdm_lo_map (const cpp_int  & l__val);
        cpp_int dtdm_lo_map() const;
    
        typedef pu_cpp_int< 16 > dtdm_hi_map_cpp_int_t;
        dtdm_hi_map_cpp_int_t int_var__dtdm_hi_map;
        void dtdm_hi_map (const cpp_int  & l__val);
        cpp_int dtdm_hi_map() const;
    
        typedef pu_cpp_int< 16 > timeout_cpp_int_t;
        timeout_cpp_int_t int_var__timeout;
        void timeout (const cpp_int  & l__val);
        cpp_int timeout() const;
    
        typedef pu_cpp_int< 16 > pause_cpp_int_t;
        pause_cpp_int_t int_var__pause;
        void pause (const cpp_int  & l__val);
        cpp_int pause() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_txs_csr_cfg_sch_t
    
class cap_txs_csr_cfw_scheduler_static_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfw_scheduler_static_t(string name = "cap_txs_csr_cfw_scheduler_static_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfw_scheduler_static_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > hbm_base_cpp_int_t;
        hbm_base_cpp_int_t int_var__hbm_base;
        void hbm_base (const cpp_int  & l__val);
        cpp_int hbm_base() const;
    
        typedef pu_cpp_int< 12 > sch_grp_depth_cpp_int_t;
        sch_grp_depth_cpp_int_t int_var__sch_grp_depth;
        void sch_grp_depth (const cpp_int  & l__val);
        cpp_int sch_grp_depth() const;
    
}; // cap_txs_csr_cfw_scheduler_static_t
    
class cap_txs_csr_cfw_scheduler_glb_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfw_scheduler_glb_t(string name = "cap_txs_csr_cfw_scheduler_glb_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfw_scheduler_glb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hbm_hw_init_cpp_int_t;
        hbm_hw_init_cpp_int_t int_var__hbm_hw_init;
        void hbm_hw_init (const cpp_int  & l__val);
        cpp_int hbm_hw_init() const;
    
        typedef pu_cpp_int< 1 > sram_hw_init_cpp_int_t;
        sram_hw_init_cpp_int_t int_var__sram_hw_init;
        void sram_hw_init (const cpp_int  & l__val);
        cpp_int sram_hw_init() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 24 > qid_read_cpp_int_t;
        qid_read_cpp_int_t int_var__qid_read;
        void qid_read (const cpp_int  & l__val);
        cpp_int qid_read() const;
    
        typedef pu_cpp_int< 4 > cos_read_cpp_int_t;
        cos_read_cpp_int_t int_var__cos_read;
        void cos_read (const cpp_int  & l__val);
        cpp_int cos_read() const;
    
}; // cap_txs_csr_cfw_scheduler_glb_t
    
class cap_txs_csr_cnt_tmr_hashovrf_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cnt_tmr_hashovrf_t(string name = "cap_txs_csr_cnt_tmr_hashovrf_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cnt_tmr_hashovrf_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_txs_csr_cnt_tmr_hashovrf_t
    
class cap_txs_csr_sta_slow_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_slow_timer_t(string name = "cap_txs_csr_sta_slow_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_slow_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hbm_init_done_cpp_int_t;
        hbm_init_done_cpp_int_t int_var__hbm_init_done;
        void hbm_init_done (const cpp_int  & l__val);
        cpp_int hbm_init_done() const;
    
        typedef pu_cpp_int< 1 > sram_init_done_cpp_int_t;
        sram_init_done_cpp_int_t int_var__sram_init_done;
        void sram_init_done (const cpp_int  & l__val);
        cpp_int sram_init_done() const;
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        tick_cpp_int_t int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 12 > cTime_cpp_int_t;
        cTime_cpp_int_t int_var__cTime;
        void cTime (const cpp_int  & l__val);
        cpp_int cTime() const;
    
        typedef pu_cpp_int< 12 > pTime_cpp_int_t;
        pTime_cpp_int_t int_var__pTime;
        void pTime (const cpp_int  & l__val);
        cpp_int pTime() const;
    
}; // cap_txs_csr_sta_slow_timer_t
    
class cap_txs_csr_cfg_slow_timer_dbell_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_slow_timer_dbell_t(string name = "cap_txs_csr_cfg_slow_timer_dbell_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_slow_timer_dbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 6 > addr_update_cpp_int_t;
        addr_update_cpp_int_t int_var__addr_update;
        void addr_update (const cpp_int  & l__val);
        cpp_int addr_update() const;
    
        typedef pu_cpp_int< 16 > data_pid_cpp_int_t;
        data_pid_cpp_int_t int_var__data_pid;
        void data_pid (const cpp_int  & l__val);
        cpp_int data_pid() const;
    
        typedef pu_cpp_int< 16 > data_reserved_cpp_int_t;
        data_reserved_cpp_int_t int_var__data_reserved;
        void data_reserved (const cpp_int  & l__val);
        cpp_int data_reserved() const;
    
        typedef pu_cpp_int< 16 > data_index_cpp_int_t;
        data_index_cpp_int_t int_var__data_index;
        void data_index (const cpp_int  & l__val);
        cpp_int data_index() const;
    
}; // cap_txs_csr_cfg_slow_timer_dbell_t
    
class cap_txs_csr_cfg_slow_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_slow_timer_t(string name = "cap_txs_csr_cfg_slow_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_slow_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        tick_cpp_int_t int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 2 > hash_sel_cpp_int_t;
        hash_sel_cpp_int_t int_var__hash_sel;
        void hash_sel (const cpp_int  & l__val);
        cpp_int hash_sel() const;
    
}; // cap_txs_csr_cfg_slow_timer_t
    
class cap_txs_csr_sta_fast_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_fast_timer_t(string name = "cap_txs_csr_sta_fast_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_fast_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hbm_init_done_cpp_int_t;
        hbm_init_done_cpp_int_t int_var__hbm_init_done;
        void hbm_init_done (const cpp_int  & l__val);
        cpp_int hbm_init_done() const;
    
        typedef pu_cpp_int< 1 > sram_init_done_cpp_int_t;
        sram_init_done_cpp_int_t int_var__sram_init_done;
        void sram_init_done (const cpp_int  & l__val);
        cpp_int sram_init_done() const;
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        tick_cpp_int_t int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 12 > cTime_cpp_int_t;
        cTime_cpp_int_t int_var__cTime;
        void cTime (const cpp_int  & l__val);
        cpp_int cTime() const;
    
        typedef pu_cpp_int< 12 > pTime_cpp_int_t;
        pTime_cpp_int_t int_var__pTime;
        void pTime (const cpp_int  & l__val);
        cpp_int pTime() const;
    
}; // cap_txs_csr_sta_fast_timer_t
    
class cap_txs_csr_cfg_fast_timer_dbell_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_fast_timer_dbell_t(string name = "cap_txs_csr_cfg_fast_timer_dbell_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_fast_timer_dbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 6 > addr_update_cpp_int_t;
        addr_update_cpp_int_t int_var__addr_update;
        void addr_update (const cpp_int  & l__val);
        cpp_int addr_update() const;
    
        typedef pu_cpp_int< 16 > data_pid_cpp_int_t;
        data_pid_cpp_int_t int_var__data_pid;
        void data_pid (const cpp_int  & l__val);
        cpp_int data_pid() const;
    
        typedef pu_cpp_int< 16 > data_reserved_cpp_int_t;
        data_reserved_cpp_int_t int_var__data_reserved;
        void data_reserved (const cpp_int  & l__val);
        cpp_int data_reserved() const;
    
        typedef pu_cpp_int< 16 > data_index_cpp_int_t;
        data_index_cpp_int_t int_var__data_index;
        void data_index (const cpp_int  & l__val);
        cpp_int data_index() const;
    
}; // cap_txs_csr_cfg_fast_timer_dbell_t
    
class cap_txs_csr_cfg_fast_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_fast_timer_t(string name = "cap_txs_csr_cfg_fast_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_fast_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > tick_cpp_int_t;
        tick_cpp_int_t int_var__tick;
        void tick (const cpp_int  & l__val);
        cpp_int tick() const;
    
        typedef pu_cpp_int< 2 > hash_sel_cpp_int_t;
        hash_sel_cpp_int_t int_var__hash_sel;
        void hash_sel (const cpp_int  & l__val);
        cpp_int hash_sel() const;
    
}; // cap_txs_csr_cfg_fast_timer_t
    
class cap_txs_csr_sta_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_sta_timer_t(string name = "cap_txs_csr_sta_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_sta_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hbm_init_done_cpp_int_t;
        hbm_init_done_cpp_int_t int_var__hbm_init_done;
        void hbm_init_done (const cpp_int  & l__val);
        cpp_int hbm_init_done() const;
    
        typedef pu_cpp_int< 1 > sram_init_done_cpp_int_t;
        sram_init_done_cpp_int_t int_var__sram_init_done;
        void sram_init_done (const cpp_int  & l__val);
        cpp_int sram_init_done() const;
    
}; // cap_txs_csr_sta_timer_t
    
class cap_txs_csr_cfg_timer_dbg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_timer_dbg_t(string name = "cap_txs_csr_cfg_timer_dbg_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_timer_dbg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > hbm_efc_thr_cpp_int_t;
        hbm_efc_thr_cpp_int_t int_var__hbm_efc_thr;
        void hbm_efc_thr (const cpp_int  & l__val);
        cpp_int hbm_efc_thr() const;
    
        typedef pu_cpp_int< 2 > drb_efc_thr_cpp_int_t;
        drb_efc_thr_cpp_int_t int_var__drb_efc_thr;
        void drb_efc_thr (const cpp_int  & l__val);
        cpp_int drb_efc_thr() const;
    
        typedef pu_cpp_int< 12 > tmr_stall_thr_cpp_int_t;
        tmr_stall_thr_cpp_int_t int_var__tmr_stall_thr;
        void tmr_stall_thr (const cpp_int  & l__val);
        cpp_int tmr_stall_thr() const;
    
        typedef pu_cpp_int< 16 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_timer_dbg_t
    
class cap_txs_csr_cfw_timer_glb_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfw_timer_glb_t(string name = "cap_txs_csr_cfw_timer_glb_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfw_timer_glb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hbm_hw_init_cpp_int_t;
        hbm_hw_init_cpp_int_t int_var__hbm_hw_init;
        void hbm_hw_init (const cpp_int  & l__val);
        cpp_int hbm_hw_init() const;
    
        typedef pu_cpp_int< 1 > sram_hw_init_cpp_int_t;
        sram_hw_init_cpp_int_t int_var__sram_hw_init;
        void sram_hw_init (const cpp_int  & l__val);
        cpp_int sram_hw_init() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > ftmr_enable_cpp_int_t;
        ftmr_enable_cpp_int_t int_var__ftmr_enable;
        void ftmr_enable (const cpp_int  & l__val);
        cpp_int ftmr_enable() const;
    
        typedef pu_cpp_int< 1 > stmr_enable_cpp_int_t;
        stmr_enable_cpp_int_t int_var__stmr_enable;
        void stmr_enable (const cpp_int  & l__val);
        cpp_int stmr_enable() const;
    
        typedef pu_cpp_int< 1 > ftmr_pause_cpp_int_t;
        ftmr_pause_cpp_int_t int_var__ftmr_pause;
        void ftmr_pause (const cpp_int  & l__val);
        cpp_int ftmr_pause() const;
    
        typedef pu_cpp_int< 1 > stmr_pause_cpp_int_t;
        stmr_pause_cpp_int_t int_var__stmr_pause;
        void stmr_pause (const cpp_int  & l__val);
        cpp_int stmr_pause() const;
    
}; // cap_txs_csr_cfw_timer_glb_t
    
class cap_txs_csr_cfg_timer_static_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_timer_static_t(string name = "cap_txs_csr_cfg_timer_static_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_timer_static_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > hbm_base_cpp_int_t;
        hbm_base_cpp_int_t int_var__hbm_base;
        void hbm_base (const cpp_int  & l__val);
        cpp_int hbm_base() const;
    
        typedef pu_cpp_int< 24 > tmr_hsh_depth_cpp_int_t;
        tmr_hsh_depth_cpp_int_t int_var__tmr_hsh_depth;
        void tmr_hsh_depth (const cpp_int  & l__val);
        cpp_int tmr_hsh_depth() const;
    
        typedef pu_cpp_int< 12 > tmr_wheel_depth_cpp_int_t;
        tmr_wheel_depth_cpp_int_t int_var__tmr_wheel_depth;
        void tmr_wheel_depth (const cpp_int  & l__val);
        cpp_int tmr_wheel_depth() const;
    
}; // cap_txs_csr_cfg_timer_static_t
    
class cap_txs_csr_cfg_glb_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_cfg_glb_t(string name = "cap_txs_csr_cfg_glb_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_cfg_glb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_txs_csr_cfg_glb_t
    
class cap_txs_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_base_t(string name = "cap_txs_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_base_t();
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
    
}; // cap_txs_csr_base_t
    
class cap_txs_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_txs_csr_t(string name = "cap_txs_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_txs_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_txs_csr_base_t base;
    
        cap_txs_csr_cfg_glb_t cfg_glb;
    
        cap_txs_csr_cfg_timer_static_t cfg_timer_static;
    
        cap_txs_csr_cfw_timer_glb_t cfw_timer_glb;
    
        cap_txs_csr_cfg_timer_dbg_t cfg_timer_dbg;
    
        cap_txs_csr_sta_timer_t sta_timer;
    
        cap_txs_csr_cfg_fast_timer_t cfg_fast_timer;
    
        cap_txs_csr_cfg_fast_timer_dbell_t cfg_fast_timer_dbell;
    
        cap_txs_csr_sta_fast_timer_t sta_fast_timer;
    
        cap_txs_csr_cfg_slow_timer_t cfg_slow_timer;
    
        cap_txs_csr_cfg_slow_timer_dbell_t cfg_slow_timer_dbell;
    
        cap_txs_csr_sta_slow_timer_t sta_slow_timer;
    
        cap_txs_csr_cnt_tmr_hashovrf_t cnt_tmr_hashovrf;
    
        cap_txs_csr_cfw_scheduler_glb_t cfw_scheduler_glb;
    
        cap_txs_csr_cfw_scheduler_static_t cfw_scheduler_static;
    
        cap_txs_csr_cfg_sch_t cfg_sch;
    
        cap_txs_csr_cnt_sch_doorbell_t cnt_sch_doorbell;
    
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
    
        cap_txs_csr_sta_scheduler_t sta_scheduler;
    
        cap_txs_csr_sta_scheduler_rr_t sta_scheduler_rr;
    
        cap_txs_csr_cfg_scheduler_dbg_t cfg_scheduler_dbg;
    
        cap_txs_csr_cfg_srams_ecc_disable_t cfg_srams_ecc_disable;
    
        cap_txs_csr_sta_srams_ecc_tmr_cnt_t sta_srams_ecc_tmr_cnt;
    
        cap_txs_csr_sta_srams_ecc_sch_lif_map_t sta_srams_ecc_sch_lif_map;
    
        cap_txs_csr_sta_srams_ecc_sch_rlid_map_t sta_srams_ecc_sch_rlid_map;
    
        cap_txs_csr_sta_srams_ecc_sch_grp_t sta_srams_ecc_sch_grp;
    
        cap_txs_csr_dhs_fast_timer_start_no_stop_t dhs_fast_timer_start_no_stop;
    
        cap_txs_csr_dhs_fast_timer_pending_t dhs_fast_timer_pending;
    
        cap_txs_csr_dhs_slow_timer_start_no_stop_t dhs_slow_timer_start_no_stop;
    
        cap_txs_csr_dhs_slow_timer_pending_t dhs_slow_timer_pending;
    
        cap_txs_csr_dhs_dtdmlo_calendar_t dhs_dtdmlo_calendar;
    
        cap_txs_csr_dhs_dtdmhi_calendar_t dhs_dtdmhi_calendar;
    
        cap_txs_csr_dhs_sch_flags0_t dhs_sch_flags0;
    
        cap_txs_csr_dhs_sch_flags1_t dhs_sch_flags1;
    
        cap_txs_csr_dhs_sch_flags2_t dhs_sch_flags2;
    
        cap_txs_csr_dhs_sch_flags3_t dhs_sch_flags3;
    
        cap_txs_csr_dhs_sch_flags4_t dhs_sch_flags4;
    
        cap_txs_csr_dhs_sch_flags5_t dhs_sch_flags5;
    
        cap_txs_csr_dhs_sch_flags6_t dhs_sch_flags6;
    
        cap_txs_csr_dhs_sch_flags7_t dhs_sch_flags7;
    
        cap_txs_csr_dhs_sch_flags8_t dhs_sch_flags8;
    
        cap_txs_csr_dhs_sch_flags9_t dhs_sch_flags9;
    
        cap_txs_csr_dhs_sch_flags10_t dhs_sch_flags10;
    
        cap_txs_csr_dhs_sch_flags11_t dhs_sch_flags11;
    
        cap_txs_csr_dhs_sch_flags12_t dhs_sch_flags12;
    
        cap_txs_csr_dhs_sch_flags13_t dhs_sch_flags13;
    
        cap_txs_csr_dhs_sch_flags14_t dhs_sch_flags14;
    
        cap_txs_csr_dhs_sch_flags15_t dhs_sch_flags15;
    
        cap_txs_csr_dhs_doorbell_t dhs_doorbell;
    
        cap_txs_csr_dhs_sch_grp_entry_t dhs_sch_grp_entry;
    
        cap_txs_csr_dhs_rlid_stop_t dhs_rlid_stop;
    
        cap_txs_csr_dhs_tmr_cnt_sram_t dhs_tmr_cnt_sram;
    
        cap_txs_csr_dhs_sch_lif_map_sram_t dhs_sch_lif_map_sram;
    
        cap_txs_csr_dhs_sch_rlid_map_sram_t dhs_sch_rlid_map_sram;
    
        cap_txs_csr_dhs_sch_grp_sram_t dhs_sch_grp_sram;
    
}; // cap_txs_csr_t
    
#endif // CAP_TXS_CSR_H
        