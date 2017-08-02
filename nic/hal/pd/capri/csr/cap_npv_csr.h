
#ifndef CAP_NPV_CSR_H
#define CAP_NPV_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_npv_csr_dhs_lif_qstate_map_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_dhs_lif_qstate_map_entry_t(string name = "cap_npv_csr_dhs_lif_qstate_map_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_dhs_lif_qstate_map_entry_t();
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
    
}; // cap_npv_csr_dhs_lif_qstate_map_entry_t
    
class cap_npv_csr_dhs_lif_qstate_map_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_dhs_lif_qstate_map_t(string name = "cap_npv_csr_dhs_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_dhs_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_npv_csr_dhs_lif_qstate_map_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_npv_csr_dhs_lif_qstate_map_t
    
class cap_npv_csr_sta_ecc_lif_qstate_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_sta_ecc_lif_qstate_map_t(string name = "cap_npv_csr_sta_ecc_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_sta_ecc_lif_qstate_map_t();
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
    
}; // cap_npv_csr_sta_ecc_lif_qstate_map_t
    
class cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t();
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
    
}; // cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t
    
class cap_npv_csr_cfg_qstate_map_rsp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_cfg_qstate_map_rsp_t(string name = "cap_npv_csr_cfg_qstate_map_rsp_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_cfg_qstate_map_rsp_t();
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
    
}; // cap_npv_csr_cfg_qstate_map_rsp_t
    
class cap_npv_csr_cfg_qstate_map_req_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_cfg_qstate_map_req_t(string name = "cap_npv_csr_cfg_qstate_map_req_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_cfg_qstate_map_req_t();
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
    
}; // cap_npv_csr_cfg_qstate_map_req_t
    
class cap_npv_csr_cfg_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_cfg_profile_t(string name = "cap_npv_csr_cfg_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_cfg_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > start_offset_cpp_int_t;
        start_offset_cpp_int_t int_var__start_offset;
        void start_offset (const cpp_int  & l__val);
        cpp_int start_offset() const;
    
}; // cap_npv_csr_cfg_profile_t
    
class cap_npv_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_npv_csr_t(string name = "cap_npv_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_npv_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_npv_csr_cfg_profile_t cfg_profile[8];
        int get_depth_cfg_profile() { return 8; }
    
        cap_npv_csr_cfg_qstate_map_req_t cfg_qstate_map_req;
    
        cap_npv_csr_cfg_qstate_map_rsp_t cfg_qstate_map_rsp;
    
        cap_npv_csr_cfg_ecc_disable_lif_qstate_map_t cfg_ecc_disable_lif_qstate_map;
    
        cap_npv_csr_sta_ecc_lif_qstate_map_t sta_ecc_lif_qstate_map;
    
        cap_npv_csr_dhs_lif_qstate_map_t dhs_lif_qstate_map;
    
}; // cap_npv_csr_t
    
#endif // CAP_NPV_CSR_H
        
