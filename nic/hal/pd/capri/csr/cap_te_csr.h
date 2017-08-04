
#ifndef CAP_TE_CSR_H
#define CAP_TE_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_te_csr_dhs_table_profile_ctrl_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_dhs_table_profile_ctrl_sram_entry_t(string name = "cap_te_csr_dhs_table_profile_ctrl_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_dhs_table_profile_ctrl_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > km_mode0_cpp_int_t;
        km_mode0_cpp_int_t int_var__km_mode0;
        void km_mode0 (const cpp_int  & l__val);
        cpp_int km_mode0() const;
    
        typedef pu_cpp_int< 2 > km_mode1_cpp_int_t;
        km_mode1_cpp_int_t int_var__km_mode1;
        void km_mode1 (const cpp_int  & l__val);
        cpp_int km_mode1() const;
    
        typedef pu_cpp_int< 2 > km_mode2_cpp_int_t;
        km_mode2_cpp_int_t int_var__km_mode2;
        void km_mode2 (const cpp_int  & l__val);
        cpp_int km_mode2() const;
    
        typedef pu_cpp_int< 2 > km_mode3_cpp_int_t;
        km_mode3_cpp_int_t int_var__km_mode3;
        void km_mode3 (const cpp_int  & l__val);
        cpp_int km_mode3() const;
    
        typedef pu_cpp_int< 3 > km_profile0_cpp_int_t;
        km_profile0_cpp_int_t int_var__km_profile0;
        void km_profile0 (const cpp_int  & l__val);
        cpp_int km_profile0() const;
    
        typedef pu_cpp_int< 3 > km_profile1_cpp_int_t;
        km_profile1_cpp_int_t int_var__km_profile1;
        void km_profile1 (const cpp_int  & l__val);
        cpp_int km_profile1() const;
    
        typedef pu_cpp_int< 3 > km_profile2_cpp_int_t;
        km_profile2_cpp_int_t int_var__km_profile2;
        void km_profile2 (const cpp_int  & l__val);
        cpp_int km_profile2() const;
    
        typedef pu_cpp_int< 3 > km_profile3_cpp_int_t;
        km_profile3_cpp_int_t int_var__km_profile3;
        void km_profile3 (const cpp_int  & l__val);
        cpp_int km_profile3() const;
    
        typedef pu_cpp_int< 1 > km_new_key0_cpp_int_t;
        km_new_key0_cpp_int_t int_var__km_new_key0;
        void km_new_key0 (const cpp_int  & l__val);
        cpp_int km_new_key0() const;
    
        typedef pu_cpp_int< 1 > km_new_key1_cpp_int_t;
        km_new_key1_cpp_int_t int_var__km_new_key1;
        void km_new_key1 (const cpp_int  & l__val);
        cpp_int km_new_key1() const;
    
        typedef pu_cpp_int< 1 > km_new_key2_cpp_int_t;
        km_new_key2_cpp_int_t int_var__km_new_key2;
        void km_new_key2 (const cpp_int  & l__val);
        cpp_int km_new_key2() const;
    
        typedef pu_cpp_int< 1 > km_new_key3_cpp_int_t;
        km_new_key3_cpp_int_t int_var__km_new_key3;
        void km_new_key3 (const cpp_int  & l__val);
        cpp_int km_new_key3() const;
    
        typedef pu_cpp_int< 3 > lkup_cpp_int_t;
        lkup_cpp_int_t int_var__lkup;
        void lkup (const cpp_int  & l__val);
        cpp_int lkup() const;
    
        typedef pu_cpp_int< 4 > tableid_cpp_int_t;
        tableid_cpp_int_t int_var__tableid;
        void tableid (const cpp_int  & l__val);
        cpp_int tableid() const;
    
        typedef pu_cpp_int< 3 > hash_sel_cpp_int_t;
        hash_sel_cpp_int_t int_var__hash_sel;
        void hash_sel (const cpp_int  & l__val);
        cpp_int hash_sel() const;
    
        typedef pu_cpp_int< 2 > hash_chain_cpp_int_t;
        hash_chain_cpp_int_t int_var__hash_chain;
        void hash_chain (const cpp_int  & l__val);
        cpp_int hash_chain() const;
    
        typedef pu_cpp_int< 2 > hash_store_cpp_int_t;
        hash_store_cpp_int_t int_var__hash_store;
        void hash_store (const cpp_int  & l__val);
        cpp_int hash_store() const;
    
}; // cap_te_csr_dhs_table_profile_ctrl_sram_entry_t
    
class cap_te_csr_dhs_table_profile_ctrl_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_dhs_table_profile_ctrl_sram_t(string name = "cap_te_csr_dhs_table_profile_ctrl_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_dhs_table_profile_ctrl_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_te_csr_dhs_table_profile_ctrl_sram_entry_t entry[192];
        int get_depth_entry() { return 192; }
    
}; // cap_te_csr_dhs_table_profile_ctrl_sram_t
    
class cap_te_csr_sta_spare_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_sta_spare_t(string name = "cap_te_csr_sta_spare_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_sta_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_te_csr_sta_spare_t
    
class cap_te_csr_sta_wait_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_sta_wait_entry_t(string name = "cap_te_csr_sta_wait_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_sta_wait_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 579 > key_cpp_int_t;
        key_cpp_int_t int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 101 > cam_cpp_int_t;
        cam_cpp_int_t int_var__cam;
        void cam (const cpp_int  & l__val);
        cpp_int cam() const;
    
        typedef pu_cpp_int< 17 > haz_cpp_int_t;
        haz_cpp_int_t int_var__haz;
        void haz (const cpp_int  & l__val);
        cpp_int haz() const;
    
        typedef pu_cpp_int< 512 > dat_cpp_int_t;
        dat_cpp_int_t int_var__dat;
        void dat (const cpp_int  & l__val);
        cpp_int dat() const;
    
}; // cap_te_csr_sta_wait_entry_t
    
class cap_te_csr_cfg_read_wait_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_read_wait_entry_t(string name = "cap_te_csr_cfg_read_wait_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_read_wait_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > idx_cpp_int_t;
        idx_cpp_int_t int_var__idx;
        void idx (const cpp_int  & l__val);
        cpp_int idx() const;
    
}; // cap_te_csr_cfg_read_wait_entry_t
    
class cap_te_csr_cfg_spare_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_spare_t(string name = "cap_te_csr_cfg_spare_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > spare_cpp_int_t;
        spare_cpp_int_t int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_te_csr_cfg_spare_t
    
class cap_te_csr_sta_debug_bus_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_sta_debug_bus_t(string name = "cap_te_csr_sta_debug_bus_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_sta_debug_bus_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 256 > vec_cpp_int_t;
        vec_cpp_int_t int_var__vec;
        void vec (const cpp_int  & l__val);
        cpp_int vec() const;
    
}; // cap_te_csr_sta_debug_bus_t
    
class cap_te_csr_cfg_lock_timeout_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_lock_timeout_t(string name = "cap_te_csr_cfg_lock_timeout_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_lock_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        en_cpp_int_t int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
}; // cap_te_csr_cfg_lock_timeout_t
    
class cap_te_csr_cfg_debug_bus_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_debug_bus_t(string name = "cap_te_csr_cfg_debug_bus_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_debug_bus_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > axi_rdata_sel_cpp_int_t;
        axi_rdata_sel_cpp_int_t int_var__axi_rdata_sel;
        void axi_rdata_sel (const cpp_int  & l__val);
        cpp_int axi_rdata_sel() const;
    
        typedef pu_cpp_int< 3 > tcam_key_sel_cpp_int_t;
        tcam_key_sel_cpp_int_t int_var__tcam_key_sel;
        void tcam_key_sel (const cpp_int  & l__val);
        cpp_int tcam_key_sel() const;
    
        typedef pu_cpp_int< 3 > hash_key_sel_cpp_int_t;
        hash_key_sel_cpp_int_t int_var__hash_key_sel;
        void hash_key_sel (const cpp_int  & l__val);
        cpp_int hash_key_sel() const;
    
        typedef pu_cpp_int< 4 > sel_cpp_int_t;
        sel_cpp_int_t int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_te_csr_cfg_debug_bus_t
    
class cap_te_csr_cfg_cnt_debug_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_cnt_debug_t(string name = "cap_te_csr_cfg_cnt_debug_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_cnt_debug_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > sel_cpp_int_t;
        sel_cpp_int_t int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_te_csr_cfg_cnt_debug_t
    
class cap_te_csr_cnt_debug_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_debug_t(string name = "cap_te_csr_cnt_debug_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_debug_t();
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
    
}; // cap_te_csr_cnt_debug_t
    
class cap_te_csr_cnt_mpu_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_mpu_out_t(string name = "cap_te_csr_cnt_mpu_out_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_mpu_out_t();
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
    
}; // cap_te_csr_cnt_mpu_out_t
    
class cap_te_csr_cnt_tcam_rsp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_tcam_rsp_t(string name = "cap_te_csr_cnt_tcam_rsp_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_tcam_rsp_t();
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
    
}; // cap_te_csr_cnt_tcam_rsp_t
    
class cap_te_csr_cnt_tcam_req_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_tcam_req_t(string name = "cap_te_csr_cnt_tcam_req_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_tcam_req_t();
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
    
}; // cap_te_csr_cnt_tcam_req_t
    
class cap_te_csr_cnt_axi_rdrsp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_axi_rdrsp_t(string name = "cap_te_csr_cnt_axi_rdrsp_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_axi_rdrsp_t();
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
    
}; // cap_te_csr_cnt_axi_rdrsp_t
    
class cap_te_csr_cnt_axi_rdreq_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_axi_rdreq_t(string name = "cap_te_csr_cnt_axi_rdreq_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_axi_rdreq_t();
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
    
}; // cap_te_csr_cnt_axi_rdreq_t
    
class cap_te_csr_cnt_phv_out_eop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_phv_out_eop_t(string name = "cap_te_csr_cnt_phv_out_eop_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_phv_out_eop_t();
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
    
}; // cap_te_csr_cnt_phv_out_eop_t
    
class cap_te_csr_cnt_phv_out_sop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_phv_out_sop_t(string name = "cap_te_csr_cnt_phv_out_sop_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_phv_out_sop_t();
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
    
}; // cap_te_csr_cnt_phv_out_sop_t
    
class cap_te_csr_cnt_phv_in_eop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_phv_in_eop_t(string name = "cap_te_csr_cnt_phv_in_eop_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_phv_in_eop_t();
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
    
}; // cap_te_csr_cnt_phv_in_eop_t
    
class cap_te_csr_cnt_phv_in_sop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cnt_phv_in_sop_t(string name = "cap_te_csr_cnt_phv_in_sop_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cnt_phv_in_sop_t();
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
    
}; // cap_te_csr_cnt_phv_in_sop_t
    
class cap_te_csr_dhs_single_step_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_dhs_single_step_t(string name = "cap_te_csr_dhs_single_step_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_dhs_single_step_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > key_cpp_int_t;
        key_cpp_int_t int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 1 > lkup_cpp_int_t;
        lkup_cpp_int_t int_var__lkup;
        void lkup (const cpp_int  & l__val);
        cpp_int lkup() const;
    
        typedef pu_cpp_int< 1 > mpu_cpp_int_t;
        mpu_cpp_int_t int_var__mpu;
        void mpu (const cpp_int  & l__val);
        cpp_int mpu() const;
    
}; // cap_te_csr_dhs_single_step_t
    
class cap_te_csr_cfg_km_profile_bit_loc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_km_profile_bit_loc_t(string name = "cap_te_csr_cfg_km_profile_bit_loc_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_km_profile_bit_loc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 5 > bit_loc_cpp_int_t;
        bit_loc_cpp_int_t int_var__bit_loc;
        void bit_loc (const cpp_int  & l__val);
        cpp_int bit_loc() const;
    
}; // cap_te_csr_cfg_km_profile_bit_loc_t
    
class cap_te_csr_cfg_km_profile_bit_sel_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_km_profile_bit_sel_t(string name = "cap_te_csr_cfg_km_profile_bit_sel_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_km_profile_bit_sel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 13 > bit_sel_cpp_int_t;
        bit_sel_cpp_int_t int_var__bit_sel;
        void bit_sel (const cpp_int  & l__val);
        cpp_int bit_sel() const;
    
}; // cap_te_csr_cfg_km_profile_bit_sel_t
    
class cap_te_csr_cfg_km_profile_byte_sel_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_km_profile_byte_sel_t(string name = "cap_te_csr_cfg_km_profile_byte_sel_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_km_profile_byte_sel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > byte_sel_cpp_int_t;
        byte_sel_cpp_int_t int_var__byte_sel;
        void byte_sel (const cpp_int  & l__val);
        cpp_int byte_sel() const;
    
}; // cap_te_csr_cfg_km_profile_byte_sel_t
    
class cap_te_csr_cfg_table_mpu_const_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_table_mpu_const_t(string name = "cap_te_csr_cfg_table_mpu_const_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_table_mpu_const_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        value_cpp_int_t int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_te_csr_cfg_table_mpu_const_t
    
class cap_te_csr_cfg_table_property_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_table_property_t(string name = "cap_te_csr_cfg_table_property_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_table_property_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > axi_cpp_int_t;
        axi_cpp_int_t int_var__axi;
        void axi (const cpp_int  & l__val);
        cpp_int axi() const;
    
        typedef pu_cpp_int< 10 > key_mask_hi_cpp_int_t;
        key_mask_hi_cpp_int_t int_var__key_mask_hi;
        void key_mask_hi (const cpp_int  & l__val);
        cpp_int key_mask_hi() const;
    
        typedef pu_cpp_int< 9 > key_mask_lo_cpp_int_t;
        key_mask_lo_cpp_int_t int_var__key_mask_lo;
        void key_mask_lo (const cpp_int  & l__val);
        cpp_int key_mask_lo() const;
    
        typedef pu_cpp_int< 2 > fullkey_km_sel0_cpp_int_t;
        fullkey_km_sel0_cpp_int_t int_var__fullkey_km_sel0;
        void fullkey_km_sel0 (const cpp_int  & l__val);
        cpp_int fullkey_km_sel0() const;
    
        typedef pu_cpp_int< 2 > fullkey_km_sel1_cpp_int_t;
        fullkey_km_sel1_cpp_int_t int_var__fullkey_km_sel1;
        void fullkey_km_sel1 (const cpp_int  & l__val);
        cpp_int fullkey_km_sel1() const;
    
        typedef pu_cpp_int< 1 > lock_en_cpp_int_t;
        lock_en_cpp_int_t int_var__lock_en;
        void lock_en (const cpp_int  & l__val);
        cpp_int lock_en() const;
    
        typedef pu_cpp_int< 1 > tbl_entry_sz_raw_cpp_int_t;
        tbl_entry_sz_raw_cpp_int_t int_var__tbl_entry_sz_raw;
        void tbl_entry_sz_raw (const cpp_int  & l__val);
        cpp_int tbl_entry_sz_raw() const;
    
        typedef pu_cpp_int< 1 > mpu_pc_dyn_cpp_int_t;
        mpu_pc_dyn_cpp_int_t int_var__mpu_pc_dyn;
        void mpu_pc_dyn (const cpp_int  & l__val);
        cpp_int mpu_pc_dyn() const;
    
        typedef pu_cpp_int< 28 > mpu_pc_cpp_int_t;
        mpu_pc_cpp_int_t int_var__mpu_pc;
        void mpu_pc (const cpp_int  & l__val);
        cpp_int mpu_pc() const;
    
        typedef pu_cpp_int< 1 > mpu_pc_raw_cpp_int_t;
        mpu_pc_raw_cpp_int_t int_var__mpu_pc_raw;
        void mpu_pc_raw (const cpp_int  & l__val);
        cpp_int mpu_pc_raw() const;
    
        typedef pu_cpp_int< 9 > mpu_pc_ofst_err_cpp_int_t;
        mpu_pc_ofst_err_cpp_int_t int_var__mpu_pc_ofst_err;
        void mpu_pc_ofst_err (const cpp_int  & l__val);
        cpp_int mpu_pc_ofst_err() const;
    
        typedef pu_cpp_int< 4 > mpu_vec_cpp_int_t;
        mpu_vec_cpp_int_t int_var__mpu_vec;
        void mpu_vec (const cpp_int  & l__val);
        cpp_int mpu_vec() const;
    
        typedef pu_cpp_int< 64 > addr_base_cpp_int_t;
        addr_base_cpp_int_t int_var__addr_base;
        void addr_base (const cpp_int  & l__val);
        cpp_int addr_base() const;
    
        typedef pu_cpp_int< 1 > addr_vf_id_en_cpp_int_t;
        addr_vf_id_en_cpp_int_t int_var__addr_vf_id_en;
        void addr_vf_id_en (const cpp_int  & l__val);
        cpp_int addr_vf_id_en() const;
    
        typedef pu_cpp_int< 6 > addr_vf_id_loc_cpp_int_t;
        addr_vf_id_loc_cpp_int_t int_var__addr_vf_id_loc;
        void addr_vf_id_loc (const cpp_int  & l__val);
        cpp_int addr_vf_id_loc() const;
    
        typedef pu_cpp_int< 5 > addr_shift_cpp_int_t;
        addr_shift_cpp_int_t int_var__addr_shift;
        void addr_shift (const cpp_int  & l__val);
        cpp_int addr_shift() const;
    
        typedef pu_cpp_int< 5 > chain_shift_cpp_int_t;
        chain_shift_cpp_int_t int_var__chain_shift;
        void chain_shift (const cpp_int  & l__val);
        cpp_int chain_shift() const;
    
        typedef pu_cpp_int< 7 > addr_sz_cpp_int_t;
        addr_sz_cpp_int_t int_var__addr_sz;
        void addr_sz (const cpp_int  & l__val);
        cpp_int addr_sz() const;
    
        typedef pu_cpp_int< 3 > lg2_entry_size_cpp_int_t;
        lg2_entry_size_cpp_int_t int_var__lg2_entry_size;
        void lg2_entry_size (const cpp_int  & l__val);
        cpp_int lg2_entry_size() const;
    
        typedef pu_cpp_int< 10 > max_bypass_cnt_cpp_int_t;
        max_bypass_cnt_cpp_int_t int_var__max_bypass_cnt;
        void max_bypass_cnt (const cpp_int  & l__val);
        cpp_int max_bypass_cnt() const;
    
        typedef pu_cpp_int< 1 > lock_en_raw_cpp_int_t;
        lock_en_raw_cpp_int_t int_var__lock_en_raw;
        void lock_en_raw (const cpp_int  & l__val);
        cpp_int lock_en_raw() const;
    
        typedef pu_cpp_int< 1 > mpu_lb_cpp_int_t;
        mpu_lb_cpp_int_t int_var__mpu_lb;
        void mpu_lb (const cpp_int  & l__val);
        cpp_int mpu_lb() const;
    
        typedef pu_cpp_int< 6 > mpu_pc_loc_cpp_int_t;
        mpu_pc_loc_cpp_int_t int_var__mpu_pc_loc;
        void mpu_pc_loc (const cpp_int  & l__val);
        cpp_int mpu_pc_loc() const;
    
        typedef pu_cpp_int< 20 > oflow_base_idx_cpp_int_t;
        oflow_base_idx_cpp_int_t int_var__oflow_base_idx;
        void oflow_base_idx (const cpp_int  & l__val);
        cpp_int oflow_base_idx() const;
    
}; // cap_te_csr_cfg_table_property_t
    
class cap_te_csr_cfg_table_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_table_profile_t(string name = "cap_te_csr_cfg_table_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_table_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > mpu_results_cpp_int_t;
        mpu_results_cpp_int_t int_var__mpu_results;
        void mpu_results (const cpp_int  & l__val);
        cpp_int mpu_results() const;
    
        typedef pu_cpp_int< 8 > seq_base_cpp_int_t;
        seq_base_cpp_int_t int_var__seq_base;
        void seq_base (const cpp_int  & l__val);
        cpp_int seq_base() const;
    
}; // cap_te_csr_cfg_table_profile_t
    
class cap_te_csr_cfg_table_profile_ctrl_sram_ext_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_table_profile_ctrl_sram_ext_t(string name = "cap_te_csr_cfg_table_profile_ctrl_sram_ext_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_table_profile_ctrl_sram_ext_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > adv_phv_flit_cpp_int_t;
        adv_phv_flit_cpp_int_t int_var__adv_phv_flit;
        void adv_phv_flit (const cpp_int  & l__val);
        cpp_int adv_phv_flit() const;
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        done_cpp_int_t int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
}; // cap_te_csr_cfg_table_profile_ctrl_sram_ext_t
    
class cap_te_csr_cfg_table_profile_cam_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_table_profile_cam_t(string name = "cap_te_csr_cfg_table_profile_cam_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_table_profile_cam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        value_cpp_int_t int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
        typedef pu_cpp_int< 8 > mask_cpp_int_t;
        mask_cpp_int_t int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_te_csr_cfg_table_profile_cam_t
    
class cap_te_csr_cfg_table_profile_key_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_table_profile_key_t(string name = "cap_te_csr_cfg_table_profile_key_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_table_profile_key_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > sel0_cpp_int_t;
        sel0_cpp_int_t int_var__sel0;
        void sel0 (const cpp_int  & l__val);
        cpp_int sel0() const;
    
        typedef pu_cpp_int< 8 > sel1_cpp_int_t;
        sel1_cpp_int_t int_var__sel1;
        void sel1 (const cpp_int  & l__val);
        cpp_int sel1() const;
    
        typedef pu_cpp_int< 8 > sel2_cpp_int_t;
        sel2_cpp_int_t int_var__sel2;
        void sel2 (const cpp_int  & l__val);
        cpp_int sel2() const;
    
        typedef pu_cpp_int< 8 > sel3_cpp_int_t;
        sel3_cpp_int_t int_var__sel3;
        void sel3 (const cpp_int  & l__val);
        cpp_int sel3() const;
    
        typedef pu_cpp_int< 8 > sel4_cpp_int_t;
        sel4_cpp_int_t int_var__sel4;
        void sel4 (const cpp_int  & l__val);
        cpp_int sel4() const;
    
        typedef pu_cpp_int< 8 > sel5_cpp_int_t;
        sel5_cpp_int_t int_var__sel5;
        void sel5 (const cpp_int  & l__val);
        cpp_int sel5() const;
    
        typedef pu_cpp_int< 8 > sel6_cpp_int_t;
        sel6_cpp_int_t int_var__sel6;
        void sel6 (const cpp_int  & l__val);
        cpp_int sel6() const;
    
        typedef pu_cpp_int< 8 > sel7_cpp_int_t;
        sel7_cpp_int_t int_var__sel7;
        void sel7 (const cpp_int  & l__val);
        cpp_int sel7() const;
    
}; // cap_te_csr_cfg_table_profile_key_t
    
class cap_te_csr_cfg_global_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_cfg_global_t(string name = "cap_te_csr_cfg_global_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > sw_rst_cpp_int_t;
        sw_rst_cpp_int_t int_var__sw_rst;
        void sw_rst (const cpp_int  & l__val);
        cpp_int sw_rst() const;
    
        typedef pu_cpp_int< 1 > step_key_cpp_int_t;
        step_key_cpp_int_t int_var__step_key;
        void step_key (const cpp_int  & l__val);
        cpp_int step_key() const;
    
        typedef pu_cpp_int< 1 > step_lkup_cpp_int_t;
        step_lkup_cpp_int_t int_var__step_lkup;
        void step_lkup (const cpp_int  & l__val);
        cpp_int step_lkup() const;
    
        typedef pu_cpp_int< 1 > step_mpu_cpp_int_t;
        step_mpu_cpp_int_t int_var__step_mpu;
        void step_mpu (const cpp_int  & l__val);
        cpp_int step_mpu() const;
    
}; // cap_te_csr_cfg_global_t
    
class cap_te_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_base_t(string name = "cap_te_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_base_t();
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
    
}; // cap_te_csr_base_t
    
class cap_te_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_te_csr_t(string name = "cap_te_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_te_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_te_csr_base_t base;
    
        cap_te_csr_cfg_global_t cfg_global;
    
        cap_te_csr_cfg_table_profile_key_t cfg_table_profile_key;
    
        cap_te_csr_cfg_table_profile_cam_t cfg_table_profile_cam[16];
        int get_depth_cfg_table_profile_cam() { return 16; }
    
        cap_te_csr_cfg_table_profile_ctrl_sram_ext_t cfg_table_profile_ctrl_sram_ext[192];
        int get_depth_cfg_table_profile_ctrl_sram_ext() { return 192; }
    
        cap_te_csr_cfg_table_profile_t cfg_table_profile[16];
        int get_depth_cfg_table_profile() { return 16; }
    
        cap_te_csr_cfg_table_property_t cfg_table_property[16];
        int get_depth_cfg_table_property() { return 16; }
    
        cap_te_csr_cfg_table_mpu_const_t cfg_table_mpu_const[16];
        int get_depth_cfg_table_mpu_const() { return 16; }
    
        cap_te_csr_cfg_km_profile_byte_sel_t cfg_km_profile_byte_sel[256];
        int get_depth_cfg_km_profile_byte_sel() { return 256; }
    
        cap_te_csr_cfg_km_profile_bit_sel_t cfg_km_profile_bit_sel[128];
        int get_depth_cfg_km_profile_bit_sel() { return 128; }
    
        cap_te_csr_cfg_km_profile_bit_loc_t cfg_km_profile_bit_loc[16];
        int get_depth_cfg_km_profile_bit_loc() { return 16; }
    
        cap_te_csr_dhs_single_step_t dhs_single_step;
    
        cap_te_csr_cnt_phv_in_sop_t cnt_phv_in_sop;
    
        cap_te_csr_cnt_phv_in_eop_t cnt_phv_in_eop;
    
        cap_te_csr_cnt_phv_out_sop_t cnt_phv_out_sop;
    
        cap_te_csr_cnt_phv_out_eop_t cnt_phv_out_eop;
    
        cap_te_csr_cnt_axi_rdreq_t cnt_axi_rdreq;
    
        cap_te_csr_cnt_axi_rdrsp_t cnt_axi_rdrsp;
    
        cap_te_csr_cnt_tcam_req_t cnt_tcam_req;
    
        cap_te_csr_cnt_tcam_rsp_t cnt_tcam_rsp;
    
        cap_te_csr_cnt_mpu_out_t cnt_mpu_out[4];
        int get_depth_cnt_mpu_out() { return 4; }
    
        cap_te_csr_cnt_debug_t cnt_debug[4];
        int get_depth_cnt_debug() { return 4; }
    
        cap_te_csr_cfg_cnt_debug_t cfg_cnt_debug[4];
        int get_depth_cfg_cnt_debug() { return 4; }
    
        cap_te_csr_cfg_debug_bus_t cfg_debug_bus;
    
        cap_te_csr_cfg_lock_timeout_t cfg_lock_timeout;
    
        cap_te_csr_sta_debug_bus_t sta_debug_bus;
    
        cap_te_csr_cfg_spare_t cfg_spare;
    
        cap_te_csr_cfg_read_wait_entry_t cfg_read_wait_entry;
    
        cap_te_csr_sta_wait_entry_t sta_wait_entry;
    
        cap_te_csr_sta_spare_t sta_spare;
    
        cap_te_csr_dhs_table_profile_ctrl_sram_t dhs_table_profile_ctrl_sram;
    
}; // cap_te_csr_t
    
#endif // CAP_TE_CSR_H
        