
#ifndef CAP_DPRSTATS_CSR_H
#define CAP_DPRSTATS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dprstats_csr_CNT_ecc_err_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprstats_csr_CNT_ecc_err_t(string name = "cap_dprstats_csr_CNT_ecc_err_t", cap_csr_base *parent = 0);
        virtual ~cap_dprstats_csr_CNT_ecc_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_drop_cpp_int_t;
        pkt_drop_cpp_int_t int_var__pkt_drop;
        void pkt_drop (const cpp_int  & l__val);
        cpp_int pkt_drop() const;
    
        typedef pu_cpp_int< 40 > byte_drop_cpp_int_t;
        byte_drop_cpp_int_t int_var__byte_drop;
        void byte_drop (const cpp_int  & l__val);
        cpp_int byte_drop() const;
    
}; // cap_dprstats_csr_CNT_ecc_err_t
    
class cap_dprstats_csr_CNT_drop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprstats_csr_CNT_drop_t(string name = "cap_dprstats_csr_CNT_drop_t", cap_csr_base *parent = 0);
        virtual ~cap_dprstats_csr_CNT_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_drop_cpp_int_t;
        pkt_drop_cpp_int_t int_var__pkt_drop;
        void pkt_drop (const cpp_int  & l__val);
        cpp_int pkt_drop() const;
    
        typedef pu_cpp_int< 40 > byte_drop_cpp_int_t;
        byte_drop_cpp_int_t int_var__byte_drop;
        void byte_drop (const cpp_int  & l__val);
        cpp_int byte_drop() const;
    
}; // cap_dprstats_csr_CNT_drop_t
    
class cap_dprstats_csr_CNT_interface_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprstats_csr_CNT_interface_t(string name = "cap_dprstats_csr_CNT_interface_t", cap_csr_base *parent = 0);
        virtual ~cap_dprstats_csr_CNT_interface_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > dpp_phv_sop_cpp_int_t;
        dpp_phv_sop_cpp_int_t int_var__dpp_phv_sop;
        void dpp_phv_sop (const cpp_int  & l__val);
        cpp_int dpp_phv_sop() const;
    
        typedef pu_cpp_int< 32 > dpp_phv_eop_cpp_int_t;
        dpp_phv_eop_cpp_int_t int_var__dpp_phv_eop;
        void dpp_phv_eop (const cpp_int  & l__val);
        cpp_int dpp_phv_eop() const;
    
        typedef pu_cpp_int< 32 > dpp_ohi_sop_cpp_int_t;
        dpp_ohi_sop_cpp_int_t int_var__dpp_ohi_sop;
        void dpp_ohi_sop (const cpp_int  & l__val);
        cpp_int dpp_ohi_sop() const;
    
        typedef pu_cpp_int< 32 > dpp_ohi_eop_cpp_int_t;
        dpp_ohi_eop_cpp_int_t int_var__dpp_ohi_eop;
        void dpp_ohi_eop (const cpp_int  & l__val);
        cpp_int dpp_ohi_eop() const;
    
        typedef pu_cpp_int< 32 > dpp_csum_crc_vld_cpp_int_t;
        dpp_csum_crc_vld_cpp_int_t int_var__dpp_csum_crc_vld;
        void dpp_csum_crc_vld (const cpp_int  & l__val);
        cpp_int dpp_csum_crc_vld() const;
    
        typedef pu_cpp_int< 32 > dpp_frame_vld_cpp_int_t;
        dpp_frame_vld_cpp_int_t int_var__dpp_frame_vld;
        void dpp_frame_vld (const cpp_int  & l__val);
        cpp_int dpp_frame_vld() const;
    
        typedef pu_cpp_int< 32 > pa_pkt_sop_cpp_int_t;
        pa_pkt_sop_cpp_int_t int_var__pa_pkt_sop;
        void pa_pkt_sop (const cpp_int  & l__val);
        cpp_int pa_pkt_sop() const;
    
        typedef pu_cpp_int< 32 > pa_pkt_eop_cpp_int_t;
        pa_pkt_eop_cpp_int_t int_var__pa_pkt_eop;
        void pa_pkt_eop (const cpp_int  & l__val);
        cpp_int pa_pkt_eop() const;
    
        typedef pu_cpp_int< 32 > pb_pkt_sop_cpp_int_t;
        pb_pkt_sop_cpp_int_t int_var__pb_pkt_sop;
        void pb_pkt_sop (const cpp_int  & l__val);
        cpp_int pb_pkt_sop() const;
    
        typedef pu_cpp_int< 32 > pb_pkt_eop_cpp_int_t;
        pb_pkt_eop_cpp_int_t int_var__pb_pkt_eop;
        void pb_pkt_eop (const cpp_int  & l__val);
        cpp_int pb_pkt_eop() const;
    
        typedef pu_cpp_int< 32 > pkt_drop_cpp_int_t;
        pkt_drop_cpp_int_t int_var__pkt_drop;
        void pkt_drop (const cpp_int  & l__val);
        cpp_int pkt_drop() const;
    
}; // cap_dprstats_csr_CNT_interface_t
    
class cap_dprstats_csr_cfg_spare_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprstats_csr_cfg_spare_t(string name = "cap_dprstats_csr_cfg_spare_t", cap_csr_base *parent = 0);
        virtual ~cap_dprstats_csr_cfg_spare_t();
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
    
}; // cap_dprstats_csr_cfg_spare_t
    
class cap_dprstats_csr_cfg_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprstats_csr_cfg_capture_t(string name = "cap_dprstats_csr_cfg_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dprstats_csr_cfg_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > capture_en_cpp_int_t;
        capture_en_cpp_int_t int_var__capture_en;
        void capture_en (const cpp_int  & l__val);
        cpp_int capture_en() const;
    
        typedef pu_cpp_int< 1 > capture_arm_cpp_int_t;
        capture_arm_cpp_int_t int_var__capture_arm;
        void capture_arm (const cpp_int  & l__val);
        cpp_int capture_arm() const;
    
        typedef pu_cpp_int< 30 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dprstats_csr_cfg_capture_t
    
class cap_dprstats_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprstats_csr_t(string name = "cap_dprstats_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dprstats_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprstats_csr_cfg_capture_t cfg_capture;
    
        cap_dprstats_csr_cfg_spare_t cfg_spare[1024];
        int get_depth_cfg_spare() { return 1024; }
    
        cap_dprstats_csr_CNT_interface_t CNT_interface;
    
        cap_dprstats_csr_CNT_drop_t CNT_drop;
    
        cap_dprstats_csr_CNT_ecc_err_t CNT_ecc_err;
    
}; // cap_dprstats_csr_t
    
#endif // CAP_DPRSTATS_CSR_H
        