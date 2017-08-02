
#ifndef CAP_PSP_CSR_H
#define CAP_PSP_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_psp_csr_dhs_lif_qstate_map_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_dhs_lif_qstate_map_entry_t(string name = "cap_psp_csr_dhs_lif_qstate_map_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_dhs_lif_qstate_map_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        vld_cpp_int_t int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
        typedef pu_cpp_int< 22 > qstate_base_cpp_int_t;
        qstate_base_cpp_int_t int_var__qstate_base;
        void qstate_base (const cpp_int  & l__val);
        cpp_int qstate_base() const;
    
        typedef pu_cpp_int< 5 > length0_cpp_int_t;
        length0_cpp_int_t int_var__length0;
        void length0 (const cpp_int  & l__val);
        cpp_int length0() const;
    
        typedef pu_cpp_int< 3 > size0_cpp_int_t;
        size0_cpp_int_t int_var__size0;
        void size0 (const cpp_int  & l__val);
        cpp_int size0() const;
    
        typedef pu_cpp_int< 5 > length1_cpp_int_t;
        length1_cpp_int_t int_var__length1;
        void length1 (const cpp_int  & l__val);
        cpp_int length1() const;
    
        typedef pu_cpp_int< 3 > size1_cpp_int_t;
        size1_cpp_int_t int_var__size1;
        void size1 (const cpp_int  & l__val);
        cpp_int size1() const;
    
        typedef pu_cpp_int< 5 > length2_cpp_int_t;
        length2_cpp_int_t int_var__length2;
        void length2 (const cpp_int  & l__val);
        cpp_int length2() const;
    
        typedef pu_cpp_int< 3 > size2_cpp_int_t;
        size2_cpp_int_t int_var__size2;
        void size2 (const cpp_int  & l__val);
        cpp_int size2() const;
    
        typedef pu_cpp_int< 5 > length3_cpp_int_t;
        length3_cpp_int_t int_var__length3;
        void length3 (const cpp_int  & l__val);
        cpp_int length3() const;
    
        typedef pu_cpp_int< 3 > size3_cpp_int_t;
        size3_cpp_int_t int_var__size3;
        void size3 (const cpp_int  & l__val);
        cpp_int size3() const;
    
        typedef pu_cpp_int< 5 > length4_cpp_int_t;
        length4_cpp_int_t int_var__length4;
        void length4 (const cpp_int  & l__val);
        cpp_int length4() const;
    
        typedef pu_cpp_int< 3 > size4_cpp_int_t;
        size4_cpp_int_t int_var__size4;
        void size4 (const cpp_int  & l__val);
        cpp_int size4() const;
    
        typedef pu_cpp_int< 5 > length5_cpp_int_t;
        length5_cpp_int_t int_var__length5;
        void length5 (const cpp_int  & l__val);
        cpp_int length5() const;
    
        typedef pu_cpp_int< 3 > size5_cpp_int_t;
        size5_cpp_int_t int_var__size5;
        void size5 (const cpp_int  & l__val);
        cpp_int size5() const;
    
        typedef pu_cpp_int< 5 > length6_cpp_int_t;
        length6_cpp_int_t int_var__length6;
        void length6 (const cpp_int  & l__val);
        cpp_int length6() const;
    
        typedef pu_cpp_int< 3 > size6_cpp_int_t;
        size6_cpp_int_t int_var__size6;
        void size6 (const cpp_int  & l__val);
        cpp_int size6() const;
    
        typedef pu_cpp_int< 5 > length7_cpp_int_t;
        length7_cpp_int_t int_var__length7;
        void length7 (const cpp_int  & l__val);
        cpp_int length7() const;
    
        typedef pu_cpp_int< 3 > size7_cpp_int_t;
        size7_cpp_int_t int_var__size7;
        void size7 (const cpp_int  & l__val);
        cpp_int size7() const;
    
        typedef pu_cpp_int< 9 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_psp_csr_dhs_lif_qstate_map_entry_t
    
class cap_psp_csr_dhs_lif_qstate_map_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_dhs_lif_qstate_map_t(string name = "cap_psp_csr_dhs_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_dhs_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_psp_csr_dhs_lif_qstate_map_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_psp_csr_dhs_lif_qstate_map_t
    
class cap_psp_csr_dhs_sw_phv_mem_0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_dhs_sw_phv_mem_0_entry_t(string name = "cap_psp_csr_dhs_sw_phv_mem_0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_dhs_sw_phv_mem_0_entry_t();
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
    
        typedef pu_cpp_int< 1 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        err_cpp_int_t int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
        typedef pu_cpp_int< 20 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_psp_csr_dhs_sw_phv_mem_0_entry_t
    
class cap_psp_csr_dhs_sw_phv_mem_0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_dhs_sw_phv_mem_0_t(string name = "cap_psp_csr_dhs_sw_phv_mem_0_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_dhs_sw_phv_mem_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_psp_csr_dhs_sw_phv_mem_0_entry_t entry;
    
}; // cap_psp_csr_dhs_sw_phv_mem_0_t
    
class cap_psp_csr_sta_ecc_lif_qstate_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_sta_ecc_lif_qstate_map_t(string name = "cap_psp_csr_sta_ecc_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_sta_ecc_lif_qstate_map_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_psp_csr_sta_ecc_lif_qstate_map_t
    
class cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > cor_cpp_int_t;
        cor_cpp_int_t int_var__cor;
        void cor (const cpp_int  & l__val);
        cpp_int cor() const;
    
        typedef pu_cpp_int< 1 > det_cpp_int_t;
        det_cpp_int_t int_var__det;
        void det (const cpp_int  & l__val);
        cpp_int det() const;
    
        typedef pu_cpp_int< 1 > dhs_cpp_int_t;
        dhs_cpp_int_t int_var__dhs;
        void dhs (const cpp_int  & l__val);
        cpp_int dhs() const;
    
}; // cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t
    
class cap_psp_csr_sta_sw_phv_mem_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_sta_sw_phv_mem_t(string name = "cap_psp_csr_sta_sw_phv_mem_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_sta_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_psp_csr_sta_sw_phv_mem_t
    
class cap_psp_csr_cfg_sw_phv_mem_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_sw_phv_mem_0_t(string name = "cap_psp_csr_cfg_sw_phv_mem_0_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_sw_phv_mem_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > cfg_ecc_disable_det_cpp_int_t;
        cfg_ecc_disable_det_cpp_int_t int_var__cfg_ecc_disable_det;
        void cfg_ecc_disable_det (const cpp_int  & l__val);
        cpp_int cfg_ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > cfg_ecc_disable_cor_cpp_int_t;
        cfg_ecc_disable_cor_cpp_int_t int_var__cfg_ecc_disable_cor;
        void cfg_ecc_disable_cor (const cpp_int  & l__val);
        cpp_int cfg_ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_psp_csr_cfg_sw_phv_mem_0_t
    
class cap_psp_csr_cfg_sw_phv_profiles_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_sw_phv_profiles_t(string name = "cap_psp_csr_cfg_sw_phv_profiles_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_sw_phv_profiles_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > start_enable_cpp_int_t;
        start_enable_cpp_int_t int_var__start_enable;
        void start_enable (const cpp_int  & l__val);
        cpp_int start_enable() const;
    
        typedef pu_cpp_int< 3 > start_addr_cpp_int_t;
        start_addr_cpp_int_t int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 42 > insertion_period_clocks_cpp_int_t;
        insertion_period_clocks_cpp_int_t int_var__insertion_period_clocks;
        void insertion_period_clocks (const cpp_int  & l__val);
        cpp_int insertion_period_clocks() const;
    
        typedef pu_cpp_int< 16 > counter_max_cpp_int_t;
        counter_max_cpp_int_t int_var__counter_max;
        void counter_max (const cpp_int  & l__val);
        cpp_int counter_max() const;
    
        typedef pu_cpp_int< 1 > counter_repeat_cpp_int_t;
        counter_repeat_cpp_int_t int_var__counter_repeat;
        void counter_repeat (const cpp_int  & l__val);
        cpp_int counter_repeat() const;
    
}; // cap_psp_csr_cfg_sw_phv_profiles_t
    
class cap_psp_csr_cfg_qstate_map_rsp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_qstate_map_rsp_t(string name = "cap_psp_csr_cfg_qstate_map_rsp_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_qstate_map_rsp_t();
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
    
        typedef pu_cpp_int< 1 > qid_enable_cpp_int_t;
        qid_enable_cpp_int_t int_var__qid_enable;
        void qid_enable (const cpp_int  & l__val);
        cpp_int qid_enable() const;
    
        typedef pu_cpp_int< 1 > qtype_enable_cpp_int_t;
        qtype_enable_cpp_int_t int_var__qtype_enable;
        void qtype_enable (const cpp_int  & l__val);
        cpp_int qtype_enable() const;
    
        typedef pu_cpp_int< 1 > addr_enable_cpp_int_t;
        addr_enable_cpp_int_t int_var__addr_enable;
        void addr_enable (const cpp_int  & l__val);
        cpp_int addr_enable() const;
    
        typedef pu_cpp_int< 1 > no_data_enable_cpp_int_t;
        no_data_enable_cpp_int_t int_var__no_data_enable;
        void no_data_enable (const cpp_int  & l__val);
        cpp_int no_data_enable() const;
    
}; // cap_psp_csr_cfg_qstate_map_rsp_t
    
class cap_psp_csr_cfg_qstate_map_req_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_qstate_map_req_t(string name = "cap_psp_csr_cfg_qstate_map_req_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_qstate_map_req_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > lif_override_enable_cpp_int_t;
        lif_override_enable_cpp_int_t int_var__lif_override_enable;
        void lif_override_enable (const cpp_int  & l__val);
        cpp_int lif_override_enable() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        lif_cpp_int_t int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 1 > qtype_override_enable_cpp_int_t;
        qtype_override_enable_cpp_int_t int_var__qtype_override_enable;
        void qtype_override_enable (const cpp_int  & l__val);
        cpp_int qtype_override_enable() const;
    
        typedef pu_cpp_int< 3 > qtype_cpp_int_t;
        qtype_cpp_int_t int_var__qtype;
        void qtype (const cpp_int  & l__val);
        cpp_int qtype() const;
    
        typedef pu_cpp_int< 1 > qid_override_enable_cpp_int_t;
        qid_override_enable_cpp_int_t int_var__qid_override_enable;
        void qid_override_enable (const cpp_int  & l__val);
        cpp_int qid_override_enable() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
}; // cap_psp_csr_cfg_qstate_map_req_t
    
class cap_psp_csr_cfg_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_profile_t(string name = "cap_psp_csr_cfg_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ptd_npv_phv_full_enable_cpp_int_t;
        ptd_npv_phv_full_enable_cpp_int_t int_var__ptd_npv_phv_full_enable;
        void ptd_npv_phv_full_enable (const cpp_int  & l__val);
        cpp_int ptd_npv_phv_full_enable() const;
    
}; // cap_psp_csr_cfg_profile_t
    
class cap_psp_csr_cfg_debug_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_cfg_debug_port_t(string name = "cap_psp_csr_cfg_debug_port_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_cfg_debug_port_t();
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
    
}; // cap_psp_csr_cfg_debug_port_t
    
class cap_psp_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_base_t(string name = "cap_psp_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_base_t();
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
    
}; // cap_psp_csr_base_t
    
class cap_psp_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_psp_csr_t(string name = "cap_psp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_psp_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_psp_csr_base_t base;
    
        cap_psp_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_psp_csr_cfg_profile_t cfg_profile;
    
        cap_psp_csr_cfg_qstate_map_req_t cfg_qstate_map_req;
    
        cap_psp_csr_cfg_qstate_map_rsp_t cfg_qstate_map_rsp;
    
        cap_psp_csr_cfg_sw_phv_profiles_t cfg_sw_phv_profiles[8];
        int get_depth_cfg_sw_phv_profiles() { return 8; }
    
        cap_psp_csr_cfg_sw_phv_mem_0_t cfg_sw_phv_mem_0;
    
        cap_psp_csr_sta_sw_phv_mem_t sta_sw_phv_mem;
    
        cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t cfg_ecc_disable_lif_qstate_map;
    
        cap_psp_csr_sta_ecc_lif_qstate_map_t sta_ecc_lif_qstate_map;
    
        cap_psp_csr_dhs_sw_phv_mem_0_t dhs_sw_phv_mem_0;
    
        cap_psp_csr_dhs_lif_qstate_map_t dhs_lif_qstate_map;
    
}; // cap_psp_csr_t
    
#endif // CAP_PSP_CSR_H
        