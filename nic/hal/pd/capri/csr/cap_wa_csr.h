
#ifndef CAP_WA_CSR_H
#define CAP_WA_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_wa_csr_dhs_lif_qstate_map_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_lif_qstate_map_entry_t(string name = "cap_wa_csr_dhs_lif_qstate_map_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_lif_qstate_map_entry_t();
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
    
}; // cap_wa_csr_dhs_lif_qstate_map_entry_t
    
class cap_wa_csr_dhs_lif_qstate_map_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_lif_qstate_map_t(string name = "cap_wa_csr_dhs_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_wa_csr_dhs_lif_qstate_map_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_wa_csr_dhs_lif_qstate_map_t
    
class cap_wa_csr_dhs_doorbell_err_activity_log_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_doorbell_err_activity_log_entry_t(string name = "cap_wa_csr_dhs_doorbell_err_activity_log_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_doorbell_err_activity_log_entry_t();
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
    
        typedef pu_cpp_int< 29 > qstateaddr_cpp_int_t;
        qstateaddr_cpp_int_t int_var__qstateaddr;
        void qstateaddr (const cpp_int  & l__val);
        cpp_int qstateaddr() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        pid_cpp_int_t int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
        typedef pu_cpp_int< 13 > cnt_cpp_int_t;
        cnt_cpp_int_t int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
        typedef pu_cpp_int< 1 > addr_conflict_cpp_int_t;
        addr_conflict_cpp_int_t int_var__addr_conflict;
        void addr_conflict (const cpp_int  & l__val);
        cpp_int addr_conflict() const;
    
        typedef pu_cpp_int< 1 > tot_ring_err_cpp_int_t;
        tot_ring_err_cpp_int_t int_var__tot_ring_err;
        void tot_ring_err (const cpp_int  & l__val);
        cpp_int tot_ring_err() const;
    
        typedef pu_cpp_int< 1 > host_ring_err_cpp_int_t;
        host_ring_err_cpp_int_t int_var__host_ring_err;
        void host_ring_err (const cpp_int  & l__val);
        cpp_int host_ring_err() const;
    
        typedef pu_cpp_int< 1 > pid_fail_cpp_int_t;
        pid_fail_cpp_int_t int_var__pid_fail;
        void pid_fail (const cpp_int  & l__val);
        cpp_int pid_fail() const;
    
        typedef pu_cpp_int< 1 > qid_ovflow_cpp_int_t;
        qid_ovflow_cpp_int_t int_var__qid_ovflow;
        void qid_ovflow (const cpp_int  & l__val);
        cpp_int qid_ovflow() const;
    
}; // cap_wa_csr_dhs_doorbell_err_activity_log_entry_t
    
class cap_wa_csr_dhs_doorbell_err_activity_log_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_doorbell_err_activity_log_t(string name = "cap_wa_csr_dhs_doorbell_err_activity_log_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_doorbell_err_activity_log_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_wa_csr_dhs_doorbell_err_activity_log_entry_t entry[8];
        int get_depth_entry() { return 8; }
    
}; // cap_wa_csr_dhs_doorbell_err_activity_log_t
    
class cap_wa_csr_dhs_page4k_remap_tbl_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_page4k_remap_tbl_entry_t(string name = "cap_wa_csr_dhs_page4k_remap_tbl_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_page4k_remap_tbl_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 40 > upd_vec_cpp_int_t;
        upd_vec_cpp_int_t int_var__upd_vec;
        void upd_vec (const cpp_int  & l__val);
        cpp_int upd_vec() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        lif_cpp_int_t int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        pid_cpp_int_t int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
}; // cap_wa_csr_dhs_page4k_remap_tbl_entry_t
    
class cap_wa_csr_dhs_page4k_remap_tbl_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_page4k_remap_tbl_t(string name = "cap_wa_csr_dhs_page4k_remap_tbl_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_page4k_remap_tbl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_wa_csr_dhs_page4k_remap_tbl_entry_t entry[32];
        int get_depth_entry() { return 32; }
    
}; // cap_wa_csr_dhs_page4k_remap_tbl_t
    
class cap_wa_csr_dhs_page4k_remap_db_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_page4k_remap_db_entry_t(string name = "cap_wa_csr_dhs_page4k_remap_db_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_page4k_remap_db_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        index_cpp_int_t int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        ring_cpp_int_t int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 16 > rsvd4pid_cpp_int_t;
        rsvd4pid_cpp_int_t int_var__rsvd4pid;
        void rsvd4pid (const cpp_int  & l__val);
        cpp_int rsvd4pid() const;
    
}; // cap_wa_csr_dhs_page4k_remap_db_entry_t
    
class cap_wa_csr_dhs_page4k_remap_db_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_page4k_remap_db_t(string name = "cap_wa_csr_dhs_page4k_remap_db_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_page4k_remap_db_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_wa_csr_dhs_page4k_remap_db_entry_t entry[16384];
        int get_depth_entry() { return 16384; }
    
}; // cap_wa_csr_dhs_page4k_remap_db_t
    
class cap_wa_csr_dhs_32b_doorbell_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_32b_doorbell_entry_t(string name = "cap_wa_csr_dhs_32b_doorbell_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_32b_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        index_cpp_int_t int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 16 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
}; // cap_wa_csr_dhs_32b_doorbell_entry_t
    
class cap_wa_csr_dhs_32b_doorbell_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_32b_doorbell_t(string name = "cap_wa_csr_dhs_32b_doorbell_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_32b_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_32b_doorbell_entry_t, 524288> entry;
        int get_depth_entry() { return 524288; }
    
}; // cap_wa_csr_dhs_32b_doorbell_t
    
class cap_wa_csr_dhs_local_doorbell_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_local_doorbell_entry_t(string name = "cap_wa_csr_dhs_local_doorbell_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_local_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        index_cpp_int_t int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        ring_cpp_int_t int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        pid_cpp_int_t int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
}; // cap_wa_csr_dhs_local_doorbell_entry_t
    
class cap_wa_csr_dhs_local_doorbell_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_local_doorbell_t(string name = "cap_wa_csr_dhs_local_doorbell_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_local_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_local_doorbell_entry_t, 524288> entry;
        int get_depth_entry() { return 524288; }
    
}; // cap_wa_csr_dhs_local_doorbell_t
    
class cap_wa_csr_dhs_host_doorbell_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_host_doorbell_entry_t(string name = "cap_wa_csr_dhs_host_doorbell_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_host_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        index_cpp_int_t int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        ring_cpp_int_t int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        qid_cpp_int_t int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        pid_cpp_int_t int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
}; // cap_wa_csr_dhs_host_doorbell_entry_t
    
class cap_wa_csr_dhs_host_doorbell_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_dhs_host_doorbell_t(string name = "cap_wa_csr_dhs_host_doorbell_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_dhs_host_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_host_doorbell_entry_t, 524288> entry;
        int get_depth_entry() { return 524288; }
    
}; // cap_wa_csr_dhs_host_doorbell_t
    
class cap_wa_csr_sta_ecc_lif_qstate_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sta_ecc_lif_qstate_map_t(string name = "cap_wa_csr_sta_ecc_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sta_ecc_lif_qstate_map_t();
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
    
}; // cap_wa_csr_sta_ecc_lif_qstate_map_t
    
class cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t();
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
    
}; // cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t
    
class cap_wa_csr_sat_wa_pid_chkfail_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sat_wa_pid_chkfail_t(string name = "cap_wa_csr_sat_wa_pid_chkfail_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sat_wa_pid_chkfail_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cnt_cpp_int_t int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_pid_chkfail_t
    
class cap_wa_csr_sat_wa_qid_overflow_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sat_wa_qid_overflow_t(string name = "cap_wa_csr_sat_wa_qid_overflow_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sat_wa_qid_overflow_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cnt_cpp_int_t int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_qid_overflow_t
    
class cap_wa_csr_sat_wa_qaddr_cam_conflict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sat_wa_qaddr_cam_conflict_t(string name = "cap_wa_csr_sat_wa_qaddr_cam_conflict_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sat_wa_qaddr_cam_conflict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > cnt_cpp_int_t;
        cnt_cpp_int_t int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_qaddr_cam_conflict_t
    
class cap_wa_csr_sat_wa_ring_access_err_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sat_wa_ring_access_err_t(string name = "cap_wa_csr_sat_wa_ring_access_err_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sat_wa_ring_access_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cnt_cpp_int_t int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_ring_access_err_t
    
class cap_wa_csr_sat_wa_host_access_err_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sat_wa_host_access_err_t(string name = "cap_wa_csr_sat_wa_host_access_err_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sat_wa_host_access_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cnt_cpp_int_t int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_host_access_err_t
    
class cap_wa_csr_sta_wa_axi_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_sta_wa_axi_t(string name = "cap_wa_csr_sta_wa_axi_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_sta_wa_axi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > num_ids_cpp_int_t;
        num_ids_cpp_int_t int_var__num_ids;
        void num_ids (const cpp_int  & l__val);
        cpp_int num_ids() const;
    
}; // cap_wa_csr_sta_wa_axi_t
    
class cap_wa_csr_cfg_wa_axi_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_cfg_wa_axi_t(string name = "cap_wa_csr_cfg_wa_axi_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_cfg_wa_axi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > num_ids_cpp_int_t;
        num_ids_cpp_int_t int_var__num_ids;
        void num_ids (const cpp_int  & l__val);
        cpp_int num_ids() const;
    
}; // cap_wa_csr_cfg_wa_axi_t
    
class cap_wa_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_base_t(string name = "cap_wa_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_base_t();
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
    
}; // cap_wa_csr_base_t
    
class cap_wa_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_wa_csr_t(string name = "cap_wa_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_wa_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_wa_csr_base_t base;
    
        cap_wa_csr_cfg_wa_axi_t cfg_wa_axi;
    
        cap_wa_csr_sta_wa_axi_t sta_wa_axi;
    
        cap_wa_csr_sat_wa_host_access_err_t sat_wa_host_access_err;
    
        cap_wa_csr_sat_wa_ring_access_err_t sat_wa_ring_access_err;
    
        cap_wa_csr_sat_wa_qaddr_cam_conflict_t sat_wa_qaddr_cam_conflict;
    
        cap_wa_csr_sat_wa_qid_overflow_t sat_wa_qid_overflow;
    
        cap_wa_csr_sat_wa_pid_chkfail_t sat_wa_pid_chkfail;
    
        cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t cfg_ecc_disable_lif_qstate_map;
    
        cap_wa_csr_sta_ecc_lif_qstate_map_t sta_ecc_lif_qstate_map;
    
        cap_wa_csr_dhs_host_doorbell_t dhs_host_doorbell;
    
        cap_wa_csr_dhs_local_doorbell_t dhs_local_doorbell;
    
        cap_wa_csr_dhs_32b_doorbell_t dhs_32b_doorbell;
    
        cap_wa_csr_dhs_page4k_remap_db_t dhs_page4k_remap_db;
    
        cap_wa_csr_dhs_page4k_remap_tbl_t dhs_page4k_remap_tbl;
    
        cap_wa_csr_dhs_doorbell_err_activity_log_t dhs_doorbell_err_activity_log;
    
        cap_wa_csr_dhs_lif_qstate_map_t dhs_lif_qstate_map;
    
}; // cap_wa_csr_t
    
#endif // CAP_WA_CSR_H
        