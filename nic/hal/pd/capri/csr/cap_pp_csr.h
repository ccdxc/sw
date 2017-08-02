
#ifndef CAP_PP_CSR_H
#define CAP_PP_CSR_H

#include "cap_csr_base.h" 
#include "cap_pxc_csr.h" 
#include "cap_pxp_csr.h" 

using namespace std;
class cap_pp_csr_dhs_sbus_indir_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_dhs_sbus_indir_entry_t(string name = "cap_pp_csr_dhs_sbus_indir_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_dhs_sbus_indir_entry_t();
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
    
}; // cap_pp_csr_dhs_sbus_indir_entry_t
    
class cap_pp_csr_dhs_sbus_indir_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_dhs_sbus_indir_t(string name = "cap_pp_csr_dhs_sbus_indir_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_dhs_sbus_indir_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pp_csr_dhs_sbus_indir_entry_t entry;
    
}; // cap_pp_csr_dhs_sbus_indir_t
    
class cap_pp_csr_cfg_debug_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_debug_port_t(string name = "cap_pp_csr_cfg_debug_port_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > select_cpp_int_t;
        select_cpp_int_t int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_pp_csr_cfg_debug_port_t
    
class cap_pp_csr_cfg_pp_sd_core_to_cntl_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_sd_core_to_cntl_t(string name = "cap_pp_csr_cfg_pp_sd_core_to_cntl_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_sd_core_to_cntl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > port0_cpp_int_t;
        port0_cpp_int_t int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 16 > port1_cpp_int_t;
        port1_cpp_int_t int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 16 > port2_cpp_int_t;
        port2_cpp_int_t int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 16 > port3_cpp_int_t;
        port3_cpp_int_t int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 16 > port4_cpp_int_t;
        port4_cpp_int_t int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 16 > port5_cpp_int_t;
        port5_cpp_int_t int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 16 > port6_cpp_int_t;
        port6_cpp_int_t int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 16 > port7_cpp_int_t;
        port7_cpp_int_t int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
        typedef pu_cpp_int< 16 > port8_cpp_int_t;
        port8_cpp_int_t int_var__port8;
        void port8 (const cpp_int  & l__val);
        cpp_int port8() const;
    
        typedef pu_cpp_int< 16 > port9_cpp_int_t;
        port9_cpp_int_t int_var__port9;
        void port9 (const cpp_int  & l__val);
        cpp_int port9() const;
    
        typedef pu_cpp_int< 16 > port10_cpp_int_t;
        port10_cpp_int_t int_var__port10;
        void port10 (const cpp_int  & l__val);
        cpp_int port10() const;
    
        typedef pu_cpp_int< 16 > port11_cpp_int_t;
        port11_cpp_int_t int_var__port11;
        void port11 (const cpp_int  & l__val);
        cpp_int port11() const;
    
        typedef pu_cpp_int< 16 > port12_cpp_int_t;
        port12_cpp_int_t int_var__port12;
        void port12 (const cpp_int  & l__val);
        cpp_int port12() const;
    
        typedef pu_cpp_int< 16 > port13_cpp_int_t;
        port13_cpp_int_t int_var__port13;
        void port13 (const cpp_int  & l__val);
        cpp_int port13() const;
    
        typedef pu_cpp_int< 16 > port14_cpp_int_t;
        port14_cpp_int_t int_var__port14;
        void port14 (const cpp_int  & l__val);
        cpp_int port14() const;
    
        typedef pu_cpp_int< 16 > port15_cpp_int_t;
        port15_cpp_int_t int_var__port15;
        void port15 (const cpp_int  & l__val);
        cpp_int port15() const;
    
}; // cap_pp_csr_cfg_pp_sd_core_to_cntl_t
    
class cap_pp_csr_cfg_pp_pcsd_control_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcsd_control_t(string name = "cap_pp_csr_cfg_pp_pcsd_control_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcsd_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_0_cpp_int_t;
        fts_align_grp_0_cpp_int_t int_var__fts_align_grp_0;
        void fts_align_grp_0 (const cpp_int  & l__val);
        cpp_int fts_align_grp_0() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_0_cpp_int_t;
        sris_en_grp_0_cpp_int_t int_var__sris_en_grp_0;
        void sris_en_grp_0 (const cpp_int  & l__val);
        cpp_int sris_en_grp_0() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_0_cpp_int_t;
        rx8b10b_realign_grp_0_cpp_int_t int_var__rx8b10b_realign_grp_0;
        void rx8b10b_realign_grp_0 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_0() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_1_cpp_int_t;
        fts_align_grp_1_cpp_int_t int_var__fts_align_grp_1;
        void fts_align_grp_1 (const cpp_int  & l__val);
        cpp_int fts_align_grp_1() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_1_cpp_int_t;
        sris_en_grp_1_cpp_int_t int_var__sris_en_grp_1;
        void sris_en_grp_1 (const cpp_int  & l__val);
        cpp_int sris_en_grp_1() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_1_cpp_int_t;
        rx8b10b_realign_grp_1_cpp_int_t int_var__rx8b10b_realign_grp_1;
        void rx8b10b_realign_grp_1 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_1() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_2_cpp_int_t;
        fts_align_grp_2_cpp_int_t int_var__fts_align_grp_2;
        void fts_align_grp_2 (const cpp_int  & l__val);
        cpp_int fts_align_grp_2() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_2_cpp_int_t;
        sris_en_grp_2_cpp_int_t int_var__sris_en_grp_2;
        void sris_en_grp_2 (const cpp_int  & l__val);
        cpp_int sris_en_grp_2() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_2_cpp_int_t;
        rx8b10b_realign_grp_2_cpp_int_t int_var__rx8b10b_realign_grp_2;
        void rx8b10b_realign_grp_2 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_2() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_3_cpp_int_t;
        fts_align_grp_3_cpp_int_t int_var__fts_align_grp_3;
        void fts_align_grp_3 (const cpp_int  & l__val);
        cpp_int fts_align_grp_3() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_3_cpp_int_t;
        sris_en_grp_3_cpp_int_t int_var__sris_en_grp_3;
        void sris_en_grp_3 (const cpp_int  & l__val);
        cpp_int sris_en_grp_3() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_3_cpp_int_t;
        rx8b10b_realign_grp_3_cpp_int_t int_var__rx8b10b_realign_grp_3;
        void rx8b10b_realign_grp_3 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_3() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_4_cpp_int_t;
        fts_align_grp_4_cpp_int_t int_var__fts_align_grp_4;
        void fts_align_grp_4 (const cpp_int  & l__val);
        cpp_int fts_align_grp_4() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_4_cpp_int_t;
        sris_en_grp_4_cpp_int_t int_var__sris_en_grp_4;
        void sris_en_grp_4 (const cpp_int  & l__val);
        cpp_int sris_en_grp_4() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_4_cpp_int_t;
        rx8b10b_realign_grp_4_cpp_int_t int_var__rx8b10b_realign_grp_4;
        void rx8b10b_realign_grp_4 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_4() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_5_cpp_int_t;
        fts_align_grp_5_cpp_int_t int_var__fts_align_grp_5;
        void fts_align_grp_5 (const cpp_int  & l__val);
        cpp_int fts_align_grp_5() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_5_cpp_int_t;
        sris_en_grp_5_cpp_int_t int_var__sris_en_grp_5;
        void sris_en_grp_5 (const cpp_int  & l__val);
        cpp_int sris_en_grp_5() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_5_cpp_int_t;
        rx8b10b_realign_grp_5_cpp_int_t int_var__rx8b10b_realign_grp_5;
        void rx8b10b_realign_grp_5 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_5() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_6_cpp_int_t;
        fts_align_grp_6_cpp_int_t int_var__fts_align_grp_6;
        void fts_align_grp_6 (const cpp_int  & l__val);
        cpp_int fts_align_grp_6() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_6_cpp_int_t;
        sris_en_grp_6_cpp_int_t int_var__sris_en_grp_6;
        void sris_en_grp_6 (const cpp_int  & l__val);
        cpp_int sris_en_grp_6() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_6_cpp_int_t;
        rx8b10b_realign_grp_6_cpp_int_t int_var__rx8b10b_realign_grp_6;
        void rx8b10b_realign_grp_6 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_6() const;
    
        typedef pu_cpp_int< 2 > fts_align_grp_7_cpp_int_t;
        fts_align_grp_7_cpp_int_t int_var__fts_align_grp_7;
        void fts_align_grp_7 (const cpp_int  & l__val);
        cpp_int fts_align_grp_7() const;
    
        typedef pu_cpp_int< 2 > sris_en_grp_7_cpp_int_t;
        sris_en_grp_7_cpp_int_t int_var__sris_en_grp_7;
        void sris_en_grp_7 (const cpp_int  & l__val);
        cpp_int sris_en_grp_7() const;
    
        typedef pu_cpp_int< 2 > rx8b10b_realign_grp_7_cpp_int_t;
        rx8b10b_realign_grp_7_cpp_int_t int_var__rx8b10b_realign_grp_7;
        void rx8b10b_realign_grp_7 (const cpp_int  & l__val);
        cpp_int rx8b10b_realign_grp_7() const;
    
}; // cap_pp_csr_cfg_pp_pcsd_control_t
    
class cap_pp_csr_sta_pp_sbus_master_bist_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_sbus_master_bist_t(string name = "cap_pp_csr_sta_pp_sbus_master_bist_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_sbus_master_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > fail_cpp_int_t;
        fail_cpp_int_t int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
        typedef pu_cpp_int< 1 > pass_cpp_int_t;
        pass_cpp_int_t int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
}; // cap_pp_csr_sta_pp_sbus_master_bist_t
    
class cap_pp_csr_cfg_pp_sbus_master_bist_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_sbus_master_bist_t(string name = "cap_pp_csr_cfg_pp_sbus_master_bist_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_sbus_master_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > mode_cpp_int_t;
        mode_cpp_int_t int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        rst_cpp_int_t int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        run_cpp_int_t int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_pp_csr_cfg_pp_sbus_master_bist_t
    
class cap_pp_csr_sta_pp_sd_rdy_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_sd_rdy_t(string name = "cap_pp_csr_sta_pp_sd_rdy_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_sd_rdy_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > rx_rdy_cpp_int_t;
        rx_rdy_cpp_int_t int_var__rx_rdy;
        void rx_rdy (const cpp_int  & l__val);
        cpp_int rx_rdy() const;
    
        typedef pu_cpp_int< 16 > tx_rdy_cpp_int_t;
        tx_rdy_cpp_int_t int_var__tx_rdy;
        void tx_rdy (const cpp_int  & l__val);
        cpp_int tx_rdy() const;
    
}; // cap_pp_csr_sta_pp_sd_rdy_t
    
class cap_pp_csr_sta_pp_sd_core_status_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_sd_core_status_t(string name = "cap_pp_csr_sta_pp_sd_core_status_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_sd_core_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > lane0_cpp_int_t;
        lane0_cpp_int_t int_var__lane0;
        void lane0 (const cpp_int  & l__val);
        cpp_int lane0() const;
    
        typedef pu_cpp_int< 16 > lane1_cpp_int_t;
        lane1_cpp_int_t int_var__lane1;
        void lane1 (const cpp_int  & l__val);
        cpp_int lane1() const;
    
        typedef pu_cpp_int< 16 > lane2_cpp_int_t;
        lane2_cpp_int_t int_var__lane2;
        void lane2 (const cpp_int  & l__val);
        cpp_int lane2() const;
    
        typedef pu_cpp_int< 16 > lane3_cpp_int_t;
        lane3_cpp_int_t int_var__lane3;
        void lane3 (const cpp_int  & l__val);
        cpp_int lane3() const;
    
        typedef pu_cpp_int< 16 > lane4_cpp_int_t;
        lane4_cpp_int_t int_var__lane4;
        void lane4 (const cpp_int  & l__val);
        cpp_int lane4() const;
    
        typedef pu_cpp_int< 16 > lane5_cpp_int_t;
        lane5_cpp_int_t int_var__lane5;
        void lane5 (const cpp_int  & l__val);
        cpp_int lane5() const;
    
        typedef pu_cpp_int< 16 > lane6_cpp_int_t;
        lane6_cpp_int_t int_var__lane6;
        void lane6 (const cpp_int  & l__val);
        cpp_int lane6() const;
    
        typedef pu_cpp_int< 16 > lane7_cpp_int_t;
        lane7_cpp_int_t int_var__lane7;
        void lane7 (const cpp_int  & l__val);
        cpp_int lane7() const;
    
        typedef pu_cpp_int< 16 > lane8_cpp_int_t;
        lane8_cpp_int_t int_var__lane8;
        void lane8 (const cpp_int  & l__val);
        cpp_int lane8() const;
    
        typedef pu_cpp_int< 16 > lane9_cpp_int_t;
        lane9_cpp_int_t int_var__lane9;
        void lane9 (const cpp_int  & l__val);
        cpp_int lane9() const;
    
        typedef pu_cpp_int< 16 > lane10_cpp_int_t;
        lane10_cpp_int_t int_var__lane10;
        void lane10 (const cpp_int  & l__val);
        cpp_int lane10() const;
    
        typedef pu_cpp_int< 16 > lane11_cpp_int_t;
        lane11_cpp_int_t int_var__lane11;
        void lane11 (const cpp_int  & l__val);
        cpp_int lane11() const;
    
        typedef pu_cpp_int< 16 > lane12_cpp_int_t;
        lane12_cpp_int_t int_var__lane12;
        void lane12 (const cpp_int  & l__val);
        cpp_int lane12() const;
    
        typedef pu_cpp_int< 16 > lane13_cpp_int_t;
        lane13_cpp_int_t int_var__lane13;
        void lane13 (const cpp_int  & l__val);
        cpp_int lane13() const;
    
        typedef pu_cpp_int< 16 > lane14_cpp_int_t;
        lane14_cpp_int_t int_var__lane14;
        void lane14 (const cpp_int  & l__val);
        cpp_int lane14() const;
    
        typedef pu_cpp_int< 16 > lane15_cpp_int_t;
        lane15_cpp_int_t int_var__lane15;
        void lane15 (const cpp_int  & l__val);
        cpp_int lane15() const;
    
}; // cap_pp_csr_sta_pp_sd_core_status_t
    
class cap_pp_csr_sta_pp_sd_spico_gp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_sd_spico_gp_t(string name = "cap_pp_csr_sta_pp_sd_spico_gp_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_sd_spico_gp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > out_cpp_int_t;
        out_cpp_int_t int_var__out;
        void out (const cpp_int  & l__val);
        cpp_int out() const;
    
}; // cap_pp_csr_sta_pp_sd_spico_gp_t
    
class cap_pp_csr_cfg_pp_sd_spico_gp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_sd_spico_gp_t(string name = "cap_pp_csr_cfg_pp_sd_spico_gp_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_sd_spico_gp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > in_cpp_int_t;
        in_cpp_int_t int_var__in;
        void in (const cpp_int  & l__val);
        cpp_int in() const;
    
}; // cap_pp_csr_cfg_pp_sd_spico_gp_t
    
class cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t(string name = "cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > lane0_cpp_int_t;
        lane0_cpp_int_t int_var__lane0;
        void lane0 (const cpp_int  & l__val);
        cpp_int lane0() const;
    
        typedef pu_cpp_int< 16 > lane1_cpp_int_t;
        lane1_cpp_int_t int_var__lane1;
        void lane1 (const cpp_int  & l__val);
        cpp_int lane1() const;
    
        typedef pu_cpp_int< 16 > lane2_cpp_int_t;
        lane2_cpp_int_t int_var__lane2;
        void lane2 (const cpp_int  & l__val);
        cpp_int lane2() const;
    
        typedef pu_cpp_int< 16 > lane3_cpp_int_t;
        lane3_cpp_int_t int_var__lane3;
        void lane3 (const cpp_int  & l__val);
        cpp_int lane3() const;
    
        typedef pu_cpp_int< 16 > lane4_cpp_int_t;
        lane4_cpp_int_t int_var__lane4;
        void lane4 (const cpp_int  & l__val);
        cpp_int lane4() const;
    
        typedef pu_cpp_int< 16 > lane5_cpp_int_t;
        lane5_cpp_int_t int_var__lane5;
        void lane5 (const cpp_int  & l__val);
        cpp_int lane5() const;
    
        typedef pu_cpp_int< 16 > lane6_cpp_int_t;
        lane6_cpp_int_t int_var__lane6;
        void lane6 (const cpp_int  & l__val);
        cpp_int lane6() const;
    
        typedef pu_cpp_int< 16 > lane7_cpp_int_t;
        lane7_cpp_int_t int_var__lane7;
        void lane7 (const cpp_int  & l__val);
        cpp_int lane7() const;
    
        typedef pu_cpp_int< 16 > lane8_cpp_int_t;
        lane8_cpp_int_t int_var__lane8;
        void lane8 (const cpp_int  & l__val);
        cpp_int lane8() const;
    
        typedef pu_cpp_int< 16 > lane9_cpp_int_t;
        lane9_cpp_int_t int_var__lane9;
        void lane9 (const cpp_int  & l__val);
        cpp_int lane9() const;
    
        typedef pu_cpp_int< 16 > lane10_cpp_int_t;
        lane10_cpp_int_t int_var__lane10;
        void lane10 (const cpp_int  & l__val);
        cpp_int lane10() const;
    
        typedef pu_cpp_int< 16 > lane11_cpp_int_t;
        lane11_cpp_int_t int_var__lane11;
        void lane11 (const cpp_int  & l__val);
        cpp_int lane11() const;
    
        typedef pu_cpp_int< 16 > lane12_cpp_int_t;
        lane12_cpp_int_t int_var__lane12;
        void lane12 (const cpp_int  & l__val);
        cpp_int lane12() const;
    
        typedef pu_cpp_int< 16 > lane13_cpp_int_t;
        lane13_cpp_int_t int_var__lane13;
        void lane13 (const cpp_int  & l__val);
        cpp_int lane13() const;
    
        typedef pu_cpp_int< 16 > lane14_cpp_int_t;
        lane14_cpp_int_t int_var__lane14;
        void lane14 (const cpp_int  & l__val);
        cpp_int lane14() const;
    
        typedef pu_cpp_int< 16 > lane15_cpp_int_t;
        lane15_cpp_int_t int_var__lane15;
        void lane15 (const cpp_int  & l__val);
        cpp_int lane15() const;
    
}; // cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t
    
class cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t(string name = "cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > per_lane_cpp_int_t;
        per_lane_cpp_int_t int_var__per_lane;
        void per_lane (const cpp_int  & l__val);
        cpp_int per_lane() const;
    
}; // cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t
    
class cap_pp_csr_cfg_pp_pcsd_interrupt_request_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcsd_interrupt_request_t(string name = "cap_pp_csr_cfg_pp_pcsd_interrupt_request_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcsd_interrupt_request_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > lanemask_cpp_int_t;
        lanemask_cpp_int_t int_var__lanemask;
        void lanemask (const cpp_int  & l__val);
        cpp_int lanemask() const;
    
}; // cap_pp_csr_cfg_pp_pcsd_interrupt_request_t
    
class cap_pp_csr_cfg_pp_pcsd_interrupt_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcsd_interrupt_t(string name = "cap_pp_csr_cfg_pp_pcsd_interrupt_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcsd_interrupt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > code_cpp_int_t;
        code_cpp_int_t int_var__code;
        void code (const cpp_int  & l__val);
        cpp_int code() const;
    
        typedef pu_cpp_int< 16 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pp_csr_cfg_pp_pcsd_interrupt_t
    
class cap_pp_csr_sta_pp_pcie_pll_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_pp_pcie_pll_t(string name = "cap_pp_csr_sta_pp_pcie_pll_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_pp_pcie_pll_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > pcie_0_dll_lock_cpp_int_t;
        pcie_0_dll_lock_cpp_int_t int_var__pcie_0_dll_lock;
        void pcie_0_dll_lock (const cpp_int  & l__val);
        cpp_int pcie_0_dll_lock() const;
    
        typedef pu_cpp_int< 1 > pcie_0_pll_lock_cpp_int_t;
        pcie_0_pll_lock_cpp_int_t int_var__pcie_0_pll_lock;
        void pcie_0_pll_lock (const cpp_int  & l__val);
        cpp_int pcie_0_pll_lock() const;
    
        typedef pu_cpp_int< 1 > pcie_1_dll_lock_cpp_int_t;
        pcie_1_dll_lock_cpp_int_t int_var__pcie_1_dll_lock;
        void pcie_1_dll_lock (const cpp_int  & l__val);
        cpp_int pcie_1_dll_lock() const;
    
        typedef pu_cpp_int< 1 > pcie_1_pll_lock_cpp_int_t;
        pcie_1_pll_lock_cpp_int_t int_var__pcie_1_pll_lock;
        void pcie_1_pll_lock (const cpp_int  & l__val);
        cpp_int pcie_1_pll_lock() const;
    
}; // cap_pp_csr_sta_pp_pcie_pll_t
    
class cap_pp_csr_cfg_pp_pcie_pll_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcie_pll_1_t(string name = "cap_pp_csr_cfg_pp_pcie_pll_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcie_pll_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > dll_disable_output_clk_cpp_int_t;
        dll_disable_output_clk_cpp_int_t int_var__dll_disable_output_clk;
        void dll_disable_output_clk (const cpp_int  & l__val);
        cpp_int dll_disable_output_clk() const;
    
        typedef pu_cpp_int< 6 > dll_fbcnt_cpp_int_t;
        dll_fbcnt_cpp_int_t int_var__dll_fbcnt;
        void dll_fbcnt (const cpp_int  & l__val);
        cpp_int dll_fbcnt() const;
    
        typedef pu_cpp_int< 6 > dll_out_divcnt_cpp_int_t;
        dll_out_divcnt_cpp_int_t int_var__dll_out_divcnt;
        void dll_out_divcnt (const cpp_int  & l__val);
        cpp_int dll_out_divcnt() const;
    
        typedef pu_cpp_int< 6 > dll_refcnt_cpp_int_t;
        dll_refcnt_cpp_int_t int_var__dll_refcnt;
        void dll_refcnt (const cpp_int  & l__val);
        cpp_int dll_refcnt() const;
    
        typedef pu_cpp_int< 4 > pll_disable_output_clk_cpp_int_t;
        pll_disable_output_clk_cpp_int_t int_var__pll_disable_output_clk;
        void pll_disable_output_clk (const cpp_int  & l__val);
        cpp_int pll_disable_output_clk() const;
    
        typedef pu_cpp_int< 6 > pll_out_divcnt_cpp_int_t;
        pll_out_divcnt_cpp_int_t int_var__pll_out_divcnt;
        void pll_out_divcnt (const cpp_int  & l__val);
        cpp_int pll_out_divcnt() const;
    
        typedef pu_cpp_int< 6 > pll_refcnt_cpp_int_t;
        pll_refcnt_cpp_int_t int_var__pll_refcnt;
        void pll_refcnt (const cpp_int  & l__val);
        cpp_int pll_refcnt() const;
    
        typedef pu_cpp_int< 8 > prog_fbdiv255_cpp_int_t;
        prog_fbdiv255_cpp_int_t int_var__prog_fbdiv255;
        void prog_fbdiv255 (const cpp_int  & l__val);
        cpp_int prog_fbdiv255() const;
    
        typedef pu_cpp_int< 1 > prog_fbdiv_23_cpp_int_t;
        prog_fbdiv_23_cpp_int_t int_var__prog_fbdiv_23;
        void prog_fbdiv_23 (const cpp_int  & l__val);
        cpp_int prog_fbdiv_23() const;
    
}; // cap_pp_csr_cfg_pp_pcie_pll_1_t
    
class cap_pp_csr_cfg_pp_pcie_pll_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcie_pll_0_t(string name = "cap_pp_csr_cfg_pp_pcie_pll_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcie_pll_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > dll_disable_output_clk_cpp_int_t;
        dll_disable_output_clk_cpp_int_t int_var__dll_disable_output_clk;
        void dll_disable_output_clk (const cpp_int  & l__val);
        cpp_int dll_disable_output_clk() const;
    
        typedef pu_cpp_int< 6 > dll_fbcnt_cpp_int_t;
        dll_fbcnt_cpp_int_t int_var__dll_fbcnt;
        void dll_fbcnt (const cpp_int  & l__val);
        cpp_int dll_fbcnt() const;
    
        typedef pu_cpp_int< 6 > dll_out_divcnt_cpp_int_t;
        dll_out_divcnt_cpp_int_t int_var__dll_out_divcnt;
        void dll_out_divcnt (const cpp_int  & l__val);
        cpp_int dll_out_divcnt() const;
    
        typedef pu_cpp_int< 6 > dll_refcnt_cpp_int_t;
        dll_refcnt_cpp_int_t int_var__dll_refcnt;
        void dll_refcnt (const cpp_int  & l__val);
        cpp_int dll_refcnt() const;
    
        typedef pu_cpp_int< 4 > pll_disable_output_clk_cpp_int_t;
        pll_disable_output_clk_cpp_int_t int_var__pll_disable_output_clk;
        void pll_disable_output_clk (const cpp_int  & l__val);
        cpp_int pll_disable_output_clk() const;
    
        typedef pu_cpp_int< 6 > pll_out_divcnt_cpp_int_t;
        pll_out_divcnt_cpp_int_t int_var__pll_out_divcnt;
        void pll_out_divcnt (const cpp_int  & l__val);
        cpp_int pll_out_divcnt() const;
    
        typedef pu_cpp_int< 6 > pll_refcnt_cpp_int_t;
        pll_refcnt_cpp_int_t int_var__pll_refcnt;
        void pll_refcnt (const cpp_int  & l__val);
        cpp_int pll_refcnt() const;
    
        typedef pu_cpp_int< 8 > prog_fbdiv255_cpp_int_t;
        prog_fbdiv255_cpp_int_t int_var__prog_fbdiv255;
        void prog_fbdiv255 (const cpp_int  & l__val);
        cpp_int prog_fbdiv255() const;
    
        typedef pu_cpp_int< 1 > prog_fbdiv_23_cpp_int_t;
        prog_fbdiv_23_cpp_int_t int_var__prog_fbdiv_23;
        void prog_fbdiv_23 (const cpp_int  & l__val);
        cpp_int prog_fbdiv_23() const;
    
}; // cap_pp_csr_cfg_pp_pcie_pll_0_t
    
class cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t(string name = "cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > p0_cpp_int_t;
        p0_cpp_int_t int_var__p0;
        void p0 (const cpp_int  & l__val);
        cpp_int p0() const;
    
        typedef pu_cpp_int< 1 > p1_cpp_int_t;
        p1_cpp_int_t int_var__p1;
        void p1 (const cpp_int  & l__val);
        cpp_int p1() const;
    
        typedef pu_cpp_int< 1 > p2_cpp_int_t;
        p2_cpp_int_t int_var__p2;
        void p2 (const cpp_int  & l__val);
        cpp_int p2() const;
    
        typedef pu_cpp_int< 1 > p3_cpp_int_t;
        p3_cpp_int_t int_var__p3;
        void p3 (const cpp_int  & l__val);
        cpp_int p3() const;
    
        typedef pu_cpp_int< 1 > p4_cpp_int_t;
        p4_cpp_int_t int_var__p4;
        void p4 (const cpp_int  & l__val);
        cpp_int p4() const;
    
        typedef pu_cpp_int< 1 > p5_cpp_int_t;
        p5_cpp_int_t int_var__p5;
        void p5 (const cpp_int  & l__val);
        cpp_int p5() const;
    
        typedef pu_cpp_int< 1 > p6_cpp_int_t;
        p6_cpp_int_t int_var__p6;
        void p6 (const cpp_int  & l__val);
        cpp_int p6() const;
    
        typedef pu_cpp_int< 1 > p7_cpp_int_t;
        p7_cpp_int_t int_var__p7;
        void p7 (const cpp_int  & l__val);
        cpp_int p7() const;
    
}; // cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t
    
class cap_pp_csr_cfg_pp_pcie_pll_rst_n_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcie_pll_rst_n_t(string name = "cap_pp_csr_cfg_pp_pcie_pll_rst_n_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcie_pll_rst_n_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > pll_0_cpp_int_t;
        pll_0_cpp_int_t int_var__pll_0;
        void pll_0 (const cpp_int  & l__val);
        cpp_int pll_0() const;
    
        typedef pu_cpp_int< 1 > pll_1_cpp_int_t;
        pll_1_cpp_int_t int_var__pll_1;
        void pll_1 (const cpp_int  & l__val);
        cpp_int pll_1() const;
    
}; // cap_pp_csr_cfg_pp_pcie_pll_rst_n_t
    
class cap_pp_csr_cfg_pp_rc_perstn_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_rc_perstn_t(string name = "cap_pp_csr_cfg_pp_rc_perstn_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_rc_perstn_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > p0_out_en_cpp_int_t;
        p0_out_en_cpp_int_t int_var__p0_out_en;
        void p0_out_en (const cpp_int  & l__val);
        cpp_int p0_out_en() const;
    
        typedef pu_cpp_int< 1 > p0_out_data_cpp_int_t;
        p0_out_data_cpp_int_t int_var__p0_out_data;
        void p0_out_data (const cpp_int  & l__val);
        cpp_int p0_out_data() const;
    
        typedef pu_cpp_int< 1 > p1_out_en_cpp_int_t;
        p1_out_en_cpp_int_t int_var__p1_out_en;
        void p1_out_en (const cpp_int  & l__val);
        cpp_int p1_out_en() const;
    
        typedef pu_cpp_int< 1 > p1_out_data_cpp_int_t;
        p1_out_data_cpp_int_t int_var__p1_out_data;
        void p1_out_data (const cpp_int  & l__val);
        cpp_int p1_out_data() const;
    
        typedef pu_cpp_int< 1 > p2_out_en_cpp_int_t;
        p2_out_en_cpp_int_t int_var__p2_out_en;
        void p2_out_en (const cpp_int  & l__val);
        cpp_int p2_out_en() const;
    
        typedef pu_cpp_int< 1 > p2_out_data_cpp_int_t;
        p2_out_data_cpp_int_t int_var__p2_out_data;
        void p2_out_data (const cpp_int  & l__val);
        cpp_int p2_out_data() const;
    
        typedef pu_cpp_int< 1 > p3_out_en_cpp_int_t;
        p3_out_en_cpp_int_t int_var__p3_out_en;
        void p3_out_en (const cpp_int  & l__val);
        cpp_int p3_out_en() const;
    
        typedef pu_cpp_int< 1 > p3_out_data_cpp_int_t;
        p3_out_data_cpp_int_t int_var__p3_out_data;
        void p3_out_data (const cpp_int  & l__val);
        cpp_int p3_out_data() const;
    
        typedef pu_cpp_int< 1 > p4_out_en_cpp_int_t;
        p4_out_en_cpp_int_t int_var__p4_out_en;
        void p4_out_en (const cpp_int  & l__val);
        cpp_int p4_out_en() const;
    
        typedef pu_cpp_int< 1 > p4_out_data_cpp_int_t;
        p4_out_data_cpp_int_t int_var__p4_out_data;
        void p4_out_data (const cpp_int  & l__val);
        cpp_int p4_out_data() const;
    
        typedef pu_cpp_int< 1 > p5_out_en_cpp_int_t;
        p5_out_en_cpp_int_t int_var__p5_out_en;
        void p5_out_en (const cpp_int  & l__val);
        cpp_int p5_out_en() const;
    
        typedef pu_cpp_int< 1 > p5_out_data_cpp_int_t;
        p5_out_data_cpp_int_t int_var__p5_out_data;
        void p5_out_data (const cpp_int  & l__val);
        cpp_int p5_out_data() const;
    
        typedef pu_cpp_int< 1 > p6_out_en_cpp_int_t;
        p6_out_en_cpp_int_t int_var__p6_out_en;
        void p6_out_en (const cpp_int  & l__val);
        cpp_int p6_out_en() const;
    
        typedef pu_cpp_int< 1 > p6_out_data_cpp_int_t;
        p6_out_data_cpp_int_t int_var__p6_out_data;
        void p6_out_data (const cpp_int  & l__val);
        cpp_int p6_out_data() const;
    
        typedef pu_cpp_int< 1 > p7_out_en_cpp_int_t;
        p7_out_en_cpp_int_t int_var__p7_out_en;
        void p7_out_en (const cpp_int  & l__val);
        cpp_int p7_out_en() const;
    
        typedef pu_cpp_int< 1 > p7_out_data_cpp_int_t;
        p7_out_data_cpp_int_t int_var__p7_out_data;
        void p7_out_data (const cpp_int  & l__val);
        cpp_int p7_out_data() const;
    
}; // cap_pp_csr_cfg_pp_rc_perstn_t
    
class cap_pp_csr_cfg_pp_sbus_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_sbus_t(string name = "cap_pp_csr_cfg_pp_sbus_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_sbus_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > rom_en_cpp_int_t;
        rom_en_cpp_int_t int_var__rom_en;
        void rom_en (const cpp_int  & l__val);
        cpp_int rom_en() const;
    
}; // cap_pp_csr_cfg_pp_sbus_t
    
class cap_pp_csr_cfg_pp_pcs_reset_n_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcs_reset_n_t(string name = "cap_pp_csr_cfg_pp_pcs_reset_n_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcs_reset_n_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > lanemask_cpp_int_t;
        lanemask_cpp_int_t int_var__lanemask;
        void lanemask (const cpp_int  & l__val);
        cpp_int lanemask() const;
    
}; // cap_pp_csr_cfg_pp_pcs_reset_n_t
    
class cap_pp_csr_cfg_pp_sd_async_reset_n_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_sd_async_reset_n_t(string name = "cap_pp_csr_cfg_pp_sd_async_reset_n_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_sd_async_reset_n_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > lanemask_cpp_int_t;
        lanemask_cpp_int_t int_var__lanemask;
        void lanemask (const cpp_int  & l__val);
        cpp_int lanemask() const;
    
}; // cap_pp_csr_cfg_pp_sd_async_reset_n_t
    
class cap_pp_csr_cfg_pp_pcs_interrupt_disable_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_pcs_interrupt_disable_t(string name = "cap_pp_csr_cfg_pp_pcs_interrupt_disable_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_pcs_interrupt_disable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > lanemask_cpp_int_t;
        lanemask_cpp_int_t int_var__lanemask;
        void lanemask (const cpp_int  & l__val);
        cpp_int lanemask() const;
    
}; // cap_pp_csr_cfg_pp_pcs_interrupt_disable_t
    
class cap_pp_csr_sta_sbus_indir_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_sta_sbus_indir_t(string name = "cap_pp_csr_sta_sbus_indir_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_sta_sbus_indir_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > execute_cpp_int_t;
        execute_cpp_int_t int_var__execute;
        void execute (const cpp_int  & l__val);
        cpp_int execute() const;
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        done_cpp_int_t int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
        typedef pu_cpp_int< 1 > rcv_data_vld_cpp_int_t;
        rcv_data_vld_cpp_int_t int_var__rcv_data_vld;
        void rcv_data_vld (const cpp_int  & l__val);
        cpp_int rcv_data_vld() const;
    
        typedef pu_cpp_int< 3 > result_code_cpp_int_t;
        result_code_cpp_int_t int_var__result_code;
        void result_code (const cpp_int  & l__val);
        cpp_int result_code() const;
    
}; // cap_pp_csr_sta_sbus_indir_t
    
class cap_pp_csr_cfg_sbus_indir_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_sbus_indir_t(string name = "cap_pp_csr_cfg_sbus_indir_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_sbus_indir_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > rcvr_addr_cpp_int_t;
        rcvr_addr_cpp_int_t int_var__rcvr_addr;
        void rcvr_addr (const cpp_int  & l__val);
        cpp_int rcvr_addr() const;
    
        typedef pu_cpp_int< 8 > data_addr_cpp_int_t;
        data_addr_cpp_int_t int_var__data_addr;
        void data_addr (const cpp_int  & l__val);
        cpp_int data_addr() const;
    
        typedef pu_cpp_int< 8 > command_cpp_int_t;
        command_cpp_int_t int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
}; // cap_pp_csr_cfg_sbus_indir_t
    
class cap_pp_csr_cfg_sbus_result_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_sbus_result_t(string name = "cap_pp_csr_cfg_sbus_result_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_sbus_result_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > mode_cpp_int_t;
        mode_cpp_int_t int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
}; // cap_pp_csr_cfg_sbus_result_t
    
class cap_pp_csr_cfg_pp_sw_reset_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_sw_reset_t(string name = "cap_pp_csr_cfg_pp_sw_reset_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_sw_reset_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > p0_hrst_cpp_int_t;
        p0_hrst_cpp_int_t int_var__p0_hrst;
        void p0_hrst (const cpp_int  & l__val);
        cpp_int p0_hrst() const;
    
        typedef pu_cpp_int< 1 > p0_srst_cpp_int_t;
        p0_srst_cpp_int_t int_var__p0_srst;
        void p0_srst (const cpp_int  & l__val);
        cpp_int p0_srst() const;
    
        typedef pu_cpp_int< 1 > p1_hrst_cpp_int_t;
        p1_hrst_cpp_int_t int_var__p1_hrst;
        void p1_hrst (const cpp_int  & l__val);
        cpp_int p1_hrst() const;
    
        typedef pu_cpp_int< 1 > p1_srst_cpp_int_t;
        p1_srst_cpp_int_t int_var__p1_srst;
        void p1_srst (const cpp_int  & l__val);
        cpp_int p1_srst() const;
    
        typedef pu_cpp_int< 1 > p2_hrst_cpp_int_t;
        p2_hrst_cpp_int_t int_var__p2_hrst;
        void p2_hrst (const cpp_int  & l__val);
        cpp_int p2_hrst() const;
    
        typedef pu_cpp_int< 1 > p2_srst_cpp_int_t;
        p2_srst_cpp_int_t int_var__p2_srst;
        void p2_srst (const cpp_int  & l__val);
        cpp_int p2_srst() const;
    
        typedef pu_cpp_int< 1 > p3_hrst_cpp_int_t;
        p3_hrst_cpp_int_t int_var__p3_hrst;
        void p3_hrst (const cpp_int  & l__val);
        cpp_int p3_hrst() const;
    
        typedef pu_cpp_int< 1 > p3_srst_cpp_int_t;
        p3_srst_cpp_int_t int_var__p3_srst;
        void p3_srst (const cpp_int  & l__val);
        cpp_int p3_srst() const;
    
        typedef pu_cpp_int< 1 > p4_hrst_cpp_int_t;
        p4_hrst_cpp_int_t int_var__p4_hrst;
        void p4_hrst (const cpp_int  & l__val);
        cpp_int p4_hrst() const;
    
        typedef pu_cpp_int< 1 > p4_srst_cpp_int_t;
        p4_srst_cpp_int_t int_var__p4_srst;
        void p4_srst (const cpp_int  & l__val);
        cpp_int p4_srst() const;
    
        typedef pu_cpp_int< 1 > p5_hrst_cpp_int_t;
        p5_hrst_cpp_int_t int_var__p5_hrst;
        void p5_hrst (const cpp_int  & l__val);
        cpp_int p5_hrst() const;
    
        typedef pu_cpp_int< 1 > p5_srst_cpp_int_t;
        p5_srst_cpp_int_t int_var__p5_srst;
        void p5_srst (const cpp_int  & l__val);
        cpp_int p5_srst() const;
    
        typedef pu_cpp_int< 1 > p6_hrst_cpp_int_t;
        p6_hrst_cpp_int_t int_var__p6_hrst;
        void p6_hrst (const cpp_int  & l__val);
        cpp_int p6_hrst() const;
    
        typedef pu_cpp_int< 1 > p6_srst_cpp_int_t;
        p6_srst_cpp_int_t int_var__p6_srst;
        void p6_srst (const cpp_int  & l__val);
        cpp_int p6_srst() const;
    
        typedef pu_cpp_int< 1 > p7_hrst_cpp_int_t;
        p7_hrst_cpp_int_t int_var__p7_hrst;
        void p7_hrst (const cpp_int  & l__val);
        cpp_int p7_hrst() const;
    
        typedef pu_cpp_int< 1 > p7_srst_cpp_int_t;
        p7_srst_cpp_int_t int_var__p7_srst;
        void p7_srst (const cpp_int  & l__val);
        cpp_int p7_srst() const;
    
}; // cap_pp_csr_cfg_pp_sw_reset_t
    
class cap_pp_csr_cfg_pp_linkwidth_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_cfg_pp_linkwidth_t(string name = "cap_pp_csr_cfg_pp_linkwidth_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_cfg_pp_linkwidth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > p0_lw_cpp_int_t;
        p0_lw_cpp_int_t int_var__p0_lw;
        void p0_lw (const cpp_int  & l__val);
        cpp_int p0_lw() const;
    
        typedef pu_cpp_int< 2 > p1_lw_cpp_int_t;
        p1_lw_cpp_int_t int_var__p1_lw;
        void p1_lw (const cpp_int  & l__val);
        cpp_int p1_lw() const;
    
        typedef pu_cpp_int< 2 > p2_lw_cpp_int_t;
        p2_lw_cpp_int_t int_var__p2_lw;
        void p2_lw (const cpp_int  & l__val);
        cpp_int p2_lw() const;
    
        typedef pu_cpp_int< 2 > p3_lw_cpp_int_t;
        p3_lw_cpp_int_t int_var__p3_lw;
        void p3_lw (const cpp_int  & l__val);
        cpp_int p3_lw() const;
    
        typedef pu_cpp_int< 2 > p4_lw_cpp_int_t;
        p4_lw_cpp_int_t int_var__p4_lw;
        void p4_lw (const cpp_int  & l__val);
        cpp_int p4_lw() const;
    
        typedef pu_cpp_int< 2 > p5_lw_cpp_int_t;
        p5_lw_cpp_int_t int_var__p5_lw;
        void p5_lw (const cpp_int  & l__val);
        cpp_int p5_lw() const;
    
        typedef pu_cpp_int< 2 > p6_lw_cpp_int_t;
        p6_lw_cpp_int_t int_var__p6_lw;
        void p6_lw (const cpp_int  & l__val);
        cpp_int p6_lw() const;
    
        typedef pu_cpp_int< 2 > p7_lw_cpp_int_t;
        p7_lw_cpp_int_t int_var__p7_lw;
        void p7_lw (const cpp_int  & l__val);
        cpp_int p7_lw() const;
    
}; // cap_pp_csr_cfg_pp_linkwidth_t
    
class cap_pp_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_base_t(string name = "cap_pp_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_base_t();
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
    
}; // cap_pp_csr_base_t
    
class cap_pp_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pp_csr_t(string name = "cap_pp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pp_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pp_csr_base_t base;
    
        cap_pp_csr_cfg_pp_linkwidth_t cfg_pp_linkwidth;
    
        cap_pp_csr_cfg_pp_sw_reset_t cfg_pp_sw_reset;
    
        cap_pp_csr_cfg_sbus_result_t cfg_sbus_result;
    
        cap_pp_csr_cfg_sbus_indir_t cfg_sbus_indir;
    
        cap_pp_csr_sta_sbus_indir_t sta_sbus_indir;
    
        cap_pp_csr_cfg_pp_pcs_interrupt_disable_t cfg_pp_pcs_interrupt_disable;
    
        cap_pp_csr_cfg_pp_sd_async_reset_n_t cfg_pp_sd_async_reset_n;
    
        cap_pp_csr_cfg_pp_pcs_reset_n_t cfg_pp_pcs_reset_n;
    
        cap_pp_csr_cfg_pp_sbus_t cfg_pp_sbus;
    
        cap_pp_csr_cfg_pp_rc_perstn_t cfg_pp_rc_perstn;
    
        cap_pp_csr_cfg_pp_pcie_pll_rst_n_t cfg_pp_pcie_pll_rst_n;
    
        cap_pp_csr_cfg_pp_pcie_pll_refclk_sel_t cfg_pp_pcie_pll_refclk_sel;
    
        cap_pp_csr_cfg_pp_pcie_pll_0_t cfg_pp_pcie_pll_0;
    
        cap_pp_csr_cfg_pp_pcie_pll_1_t cfg_pp_pcie_pll_1;
    
        cap_pp_csr_sta_pp_pcie_pll_t sta_pp_pcie_pll;
    
        cap_pp_csr_cfg_pp_pcsd_interrupt_t cfg_pp_pcsd_interrupt[16];
        int get_depth_cfg_pp_pcsd_interrupt() { return 16; }
    
        cap_pp_csr_cfg_pp_pcsd_interrupt_request_t cfg_pp_pcsd_interrupt_request;
    
        cap_pp_csr_sta_pp_pcsd_interrupt_in_progress_t sta_pp_pcsd_interrupt_in_progress;
    
        cap_pp_csr_sta_pp_pcsd_interrupt_data_out_t sta_pp_pcsd_interrupt_data_out;
    
        cap_pp_csr_cfg_pp_sd_spico_gp_t cfg_pp_sd_spico_gp;
    
        cap_pp_csr_sta_pp_sd_spico_gp_t sta_pp_sd_spico_gp;
    
        cap_pp_csr_sta_pp_sd_core_status_t sta_pp_sd_core_status;
    
        cap_pp_csr_sta_pp_sd_rdy_t sta_pp_sd_rdy;
    
        cap_pp_csr_cfg_pp_sbus_master_bist_t cfg_pp_sbus_master_bist;
    
        cap_pp_csr_sta_pp_sbus_master_bist_t sta_pp_sbus_master_bist;
    
        cap_pp_csr_cfg_pp_pcsd_control_t cfg_pp_pcsd_control;
    
        cap_pp_csr_cfg_pp_sd_core_to_cntl_t cfg_pp_sd_core_to_cntl;
    
        cap_pp_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pp_csr_dhs_sbus_indir_t dhs_sbus_indir;
    
        cap_pxp_csr_t port_p[8];
        int get_depth_port_p() { return 8; }
    
        cap_pxc_csr_t port_c[8];
        int get_depth_port_c() { return 8; }
    
}; // cap_pp_csr_t
    
#endif // CAP_PP_CSR_H
        