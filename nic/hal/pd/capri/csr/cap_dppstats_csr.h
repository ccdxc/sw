
#ifndef CAP_DPPSTATS_CSR_H
#define CAP_DPPSTATS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dppstats_csr_CNT_dpp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_CNT_dpp_t(string name = "cap_dppstats_csr_CNT_dpp_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_CNT_dpp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > ma_phv_sop_cpp_int_t;
        ma_phv_sop_cpp_int_t int_var__ma_phv_sop;
        void ma_phv_sop (const cpp_int  & l__val);
        cpp_int ma_phv_sop() const;
    
        typedef pu_cpp_int< 32 > ma_phv_eop_cpp_int_t;
        ma_phv_eop_cpp_int_t int_var__ma_phv_eop;
        void ma_phv_eop (const cpp_int  & l__val);
        cpp_int ma_phv_eop() const;
    
        typedef pu_cpp_int< 32 > pa_ohi_sop_cpp_int_t;
        pa_ohi_sop_cpp_int_t int_var__pa_ohi_sop;
        void pa_ohi_sop (const cpp_int  & l__val);
        cpp_int pa_ohi_sop() const;
    
        typedef pu_cpp_int< 32 > pa_ohi_eop_cpp_int_t;
        pa_ohi_eop_cpp_int_t int_var__pa_ohi_eop;
        void pa_ohi_eop (const cpp_int  & l__val);
        cpp_int pa_ohi_eop() const;
    
        typedef pu_cpp_int< 32 > dpr_phv_sop_cpp_int_t;
        dpr_phv_sop_cpp_int_t int_var__dpr_phv_sop;
        void dpr_phv_sop (const cpp_int  & l__val);
        cpp_int dpr_phv_sop() const;
    
        typedef pu_cpp_int< 32 > dpr_phv_eop_cpp_int_t;
        dpr_phv_eop_cpp_int_t int_var__dpr_phv_eop;
        void dpr_phv_eop (const cpp_int  & l__val);
        cpp_int dpr_phv_eop() const;
    
        typedef pu_cpp_int< 32 > dpr_ohi_sop_cpp_int_t;
        dpr_ohi_sop_cpp_int_t int_var__dpr_ohi_sop;
        void dpr_ohi_sop (const cpp_int  & l__val);
        cpp_int dpr_ohi_sop() const;
    
        typedef pu_cpp_int< 32 > dpr_ohi_eop_cpp_int_t;
        dpr_ohi_eop_cpp_int_t int_var__dpr_ohi_eop;
        void dpr_ohi_eop (const cpp_int  & l__val);
        cpp_int dpr_ohi_eop() const;
    
        typedef pu_cpp_int< 32 > dpr_hdrfld_select_arr_vld_cpp_int_t;
        dpr_hdrfld_select_arr_vld_cpp_int_t int_var__dpr_hdrfld_select_arr_vld;
        void dpr_hdrfld_select_arr_vld (const cpp_int  & l__val);
        cpp_int dpr_hdrfld_select_arr_vld() const;
    
        typedef pu_cpp_int< 32 > dpr_csum_crc_bundle_cpp_int_t;
        dpr_csum_crc_bundle_cpp_int_t int_var__dpr_csum_crc_bundle;
        void dpr_csum_crc_bundle (const cpp_int  & l__val);
        cpp_int dpr_csum_crc_bundle() const;
    
}; // cap_dppstats_csr_CNT_dpp_t
    
class cap_dppstats_csr_sym_hdrfld_offset3_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_offset3_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset3_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_offset3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1040 > hdrfld_offset3_cpp_int_t;
        hdrfld_offset3_cpp_int_t int_var__hdrfld_offset3;
        void hdrfld_offset3 (const cpp_int  & l__val);
        cpp_int hdrfld_offset3() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset3_capture_t
    
class cap_dppstats_csr_sym_hdrfld_offset2_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_offset2_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset2_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_offset2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_offset2_cpp_int_t;
        hdrfld_offset2_cpp_int_t int_var__hdrfld_offset2;
        void hdrfld_offset2 (const cpp_int  & l__val);
        cpp_int hdrfld_offset2() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset2_capture_t
    
class cap_dppstats_csr_sym_hdrfld_offset1_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_offset1_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset1_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_offset1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_offset1_cpp_int_t;
        hdrfld_offset1_cpp_int_t int_var__hdrfld_offset1;
        void hdrfld_offset1 (const cpp_int  & l__val);
        cpp_int hdrfld_offset1() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset1_capture_t
    
class cap_dppstats_csr_sym_hdrfld_offset0_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_offset0_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset0_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_offset0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_offset0_cpp_int_t;
        hdrfld_offset0_cpp_int_t int_var__hdrfld_offset0;
        void hdrfld_offset0 (const cpp_int  & l__val);
        cpp_int hdrfld_offset0() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset0_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size3_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_size3_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size3_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_size3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_size3_cpp_int_t;
        hdrfld_size3_cpp_int_t int_var__hdrfld_size3;
        void hdrfld_size3 (const cpp_int  & l__val);
        cpp_int hdrfld_size3() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size3_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size2_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_size2_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size2_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_size2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_size2_cpp_int_t;
        hdrfld_size2_cpp_int_t int_var__hdrfld_size2;
        void hdrfld_size2 (const cpp_int  & l__val);
        cpp_int hdrfld_size2() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size2_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size1_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_size1_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size1_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_size1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_size1_cpp_int_t;
        hdrfld_size1_cpp_int_t int_var__hdrfld_size1;
        void hdrfld_size1 (const cpp_int  & l__val);
        cpp_int hdrfld_size1() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size1_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size0_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_size0_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size0_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_size0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > hdrfld_size0_cpp_int_t;
        hdrfld_size0_cpp_int_t int_var__hdrfld_size0;
        void hdrfld_size0 (const cpp_int  & l__val);
        cpp_int hdrfld_size0() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size0_capture_t
    
class cap_dppstats_csr_sym_hdrfld_vld_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_hdrfld_vld_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_vld_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_hdrfld_vld_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 256 > hdrfld_vld_cpp_int_t;
        hdrfld_vld_cpp_int_t int_var__hdrfld_vld;
        void hdrfld_vld (const cpp_int  & l__val);
        cpp_int hdrfld_vld() const;
    
}; // cap_dppstats_csr_sym_hdrfld_vld_capture_t
    
class cap_dppstats_csr_sym_ohi_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_ohi_capture_t(string name = "cap_dppstats_csr_sym_ohi_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_ohi_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > ohi_cpp_int_t;
        ohi_cpp_int_t int_var__ohi;
        void ohi (const cpp_int  & l__val);
        cpp_int ohi() const;
    
}; // cap_dppstats_csr_sym_ohi_capture_t
    
class cap_dppstats_csr_sym_phv5_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_phv5_capture_t(string name = "cap_dppstats_csr_sym_phv5_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_phv5_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > phv5_cpp_int_t;
        phv5_cpp_int_t int_var__phv5;
        void phv5 (const cpp_int  & l__val);
        cpp_int phv5() const;
    
}; // cap_dppstats_csr_sym_phv5_capture_t
    
class cap_dppstats_csr_sym_phv4_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_phv4_capture_t(string name = "cap_dppstats_csr_sym_phv4_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_phv4_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > phv4_cpp_int_t;
        phv4_cpp_int_t int_var__phv4;
        void phv4 (const cpp_int  & l__val);
        cpp_int phv4() const;
    
}; // cap_dppstats_csr_sym_phv4_capture_t
    
class cap_dppstats_csr_sym_phv3_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_phv3_capture_t(string name = "cap_dppstats_csr_sym_phv3_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_phv3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > phv3_cpp_int_t;
        phv3_cpp_int_t int_var__phv3;
        void phv3 (const cpp_int  & l__val);
        cpp_int phv3() const;
    
}; // cap_dppstats_csr_sym_phv3_capture_t
    
class cap_dppstats_csr_sym_phv2_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_phv2_capture_t(string name = "cap_dppstats_csr_sym_phv2_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_phv2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > phv2_cpp_int_t;
        phv2_cpp_int_t int_var__phv2;
        void phv2 (const cpp_int  & l__val);
        cpp_int phv2() const;
    
}; // cap_dppstats_csr_sym_phv2_capture_t
    
class cap_dppstats_csr_sym_phv1_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_phv1_capture_t(string name = "cap_dppstats_csr_sym_phv1_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_phv1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > phv1_cpp_int_t;
        phv1_cpp_int_t int_var__phv1;
        void phv1 (const cpp_int  & l__val);
        cpp_int phv1() const;
    
}; // cap_dppstats_csr_sym_phv1_capture_t
    
class cap_dppstats_csr_sym_phv0_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_sym_phv0_capture_t(string name = "cap_dppstats_csr_sym_phv0_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_sym_phv0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1024 > phv0_cpp_int_t;
        phv0_cpp_int_t int_var__phv0;
        void phv0 (const cpp_int  & l__val);
        cpp_int phv0() const;
    
}; // cap_dppstats_csr_sym_phv0_capture_t
    
class cap_dppstats_csr_cfg_spare_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_cfg_spare_t(string name = "cap_dppstats_csr_cfg_spare_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_cfg_spare_t();
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
    
}; // cap_dppstats_csr_cfg_spare_t
    
class cap_dppstats_csr_cfg_capture_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_cfg_capture_t(string name = "cap_dppstats_csr_cfg_capture_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_cfg_capture_t();
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
    
}; // cap_dppstats_csr_cfg_capture_t
    
class cap_dppstats_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppstats_csr_t(string name = "cap_dppstats_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dppstats_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dppstats_csr_cfg_capture_t cfg_capture;
    
        cap_dppstats_csr_cfg_spare_t cfg_spare[540];
        int get_depth_cfg_spare() { return 540; }
    
        cap_dppstats_csr_sym_phv0_capture_t sym_phv0_capture;
    
        cap_dppstats_csr_sym_phv1_capture_t sym_phv1_capture;
    
        cap_dppstats_csr_sym_phv2_capture_t sym_phv2_capture;
    
        cap_dppstats_csr_sym_phv3_capture_t sym_phv3_capture;
    
        cap_dppstats_csr_sym_phv4_capture_t sym_phv4_capture;
    
        cap_dppstats_csr_sym_phv5_capture_t sym_phv5_capture;
    
        cap_dppstats_csr_sym_ohi_capture_t sym_ohi_capture;
    
        cap_dppstats_csr_sym_hdrfld_vld_capture_t sym_hdrfld_vld_capture;
    
        cap_dppstats_csr_sym_hdrfld_size0_capture_t sym_hdrfld_size0_capture;
    
        cap_dppstats_csr_sym_hdrfld_size1_capture_t sym_hdrfld_size1_capture;
    
        cap_dppstats_csr_sym_hdrfld_size2_capture_t sym_hdrfld_size2_capture;
    
        cap_dppstats_csr_sym_hdrfld_size3_capture_t sym_hdrfld_size3_capture;
    
        cap_dppstats_csr_sym_hdrfld_offset0_capture_t sym_hdrfld_offset0_capture;
    
        cap_dppstats_csr_sym_hdrfld_offset1_capture_t sym_hdrfld_offset1_capture;
    
        cap_dppstats_csr_sym_hdrfld_offset2_capture_t sym_hdrfld_offset2_capture;
    
        cap_dppstats_csr_sym_hdrfld_offset3_capture_t sym_hdrfld_offset3_capture;
    
        cap_dppstats_csr_CNT_dpp_t CNT_dpp;
    
}; // cap_dppstats_csr_t
    
#endif // CAP_DPPSTATS_CSR_H
        