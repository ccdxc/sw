
#ifndef CAP_PXC_CSR_H
#define CAP_PXC_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pxc_csr_dhs_c_mac_apb_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_dhs_c_mac_apb_entry_t(string name = "cap_pxc_csr_dhs_c_mac_apb_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_dhs_c_mac_apb_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > cfg_data_cpp_int_t;
        cfg_data_cpp_int_t int_var__cfg_data;
        void cfg_data (const cpp_int  & l__val);
        cpp_int cfg_data() const;
    
}; // cap_pxc_csr_dhs_c_mac_apb_entry_t
    
class cap_pxc_csr_dhs_c_mac_apb_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_dhs_c_mac_apb_t(string name = "cap_pxc_csr_dhs_c_mac_apb_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_dhs_c_mac_apb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxc_csr_dhs_c_mac_apb_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxc_csr_dhs_c_mac_apb_t
    
class cap_pxc_csr_sta_c_port_phystatus_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_port_phystatus_t(string name = "cap_pxc_csr_sta_c_port_phystatus_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_port_phystatus_t();
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
    
}; // cap_pxc_csr_sta_c_port_phystatus_t
    
class cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t(string name = "cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        event_cpp_int_t int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        event_cpp_int_t int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t
    
class cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t(string name = "cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        event_cpp_int_t int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t
    
class cap_pxc_csr_sat_c_port_cnt12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt12_t(string name = "cap_pxc_csr_sat_c_port_cnt12_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt12_t
    
class cap_pxc_csr_sat_c_port_cnt11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt11_t(string name = "cap_pxc_csr_sat_c_port_cnt11_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt11_t
    
class cap_pxc_csr_sat_c_port_cnt10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt10_t(string name = "cap_pxc_csr_sat_c_port_cnt10_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt10_t
    
class cap_pxc_csr_sat_c_port_cnt9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt9_t(string name = "cap_pxc_csr_sat_c_port_cnt9_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt9_t
    
class cap_pxc_csr_sat_c_port_cnt8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt8_t(string name = "cap_pxc_csr_sat_c_port_cnt8_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt8_t
    
class cap_pxc_csr_sat_c_port_cnt7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt7_t(string name = "cap_pxc_csr_sat_c_port_cnt7_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt7_t
    
class cap_pxc_csr_sat_c_port_cnt6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt6_t(string name = "cap_pxc_csr_sat_c_port_cnt6_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt6_t
    
class cap_pxc_csr_sat_c_port_cnt5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt5_t(string name = "cap_pxc_csr_sat_c_port_cnt5_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt5_t
    
class cap_pxc_csr_sat_c_port_cnt4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt4_t(string name = "cap_pxc_csr_sat_c_port_cnt4_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt4_t
    
class cap_pxc_csr_sat_c_port_cnt3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt3_t(string name = "cap_pxc_csr_sat_c_port_cnt3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt3_t
    
class cap_pxc_csr_sat_c_port_cnt2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt2_t(string name = "cap_pxc_csr_sat_c_port_cnt2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt2_t
    
class cap_pxc_csr_sat_c_port_cnt1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt1_t(string name = "cap_pxc_csr_sat_c_port_cnt1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt1_t
    
class cap_pxc_csr_sat_c_port_cnt0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sat_c_port_cnt0_t(string name = "cap_pxc_csr_sat_c_port_cnt0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sat_c_port_cnt0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tbd_cpp_int_t;
        tbd_cpp_int_t int_var__tbd;
        void tbd (const cpp_int  & l__val);
        cpp_int tbd() const;
    
}; // cap_pxc_csr_sat_c_port_cnt0_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_3_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_3_t();
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
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_3_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_2_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_2_t();
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
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_2_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_1_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_1_t();
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
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_1_t
    
class cap_pxc_csr_sta_c_ecc_rxbuf_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_0_t(string name = "cap_pxc_csr_sta_c_ecc_rxbuf_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_ecc_rxbuf_0_t();
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
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxc_csr_sta_c_ecc_rxbuf_0_t
    
class cap_pxc_csr_sta_c_tx_fc_credits_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_tx_fc_credits_t(string name = "cap_pxc_csr_sta_c_tx_fc_credits_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_tx_fc_credits_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > posted_cpp_int_t;
        posted_cpp_int_t int_var__posted;
        void posted (const cpp_int  & l__val);
        cpp_int posted() const;
    
        typedef pu_cpp_int< 32 > non_posted_cpp_int_t;
        non_posted_cpp_int_t int_var__non_posted;
        void non_posted (const cpp_int  & l__val);
        cpp_int non_posted() const;
    
}; // cap_pxc_csr_sta_c_tx_fc_credits_t
    
class cap_pxc_csr_cfg_c_mac_ssvid_cap_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_ssvid_cap_t(string name = "cap_pxc_csr_cfg_c_mac_ssvid_cap_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_ssvid_cap_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > ssvid_cpp_int_t;
        ssvid_cpp_int_t int_var__ssvid;
        void ssvid (const cpp_int  & l__val);
        cpp_int ssvid() const;
    
        typedef pu_cpp_int< 16 > ssid_cpp_int_t;
        ssid_cpp_int_t int_var__ssid;
        void ssid (const cpp_int  & l__val);
        cpp_int ssid() const;
    
}; // cap_pxc_csr_cfg_c_mac_ssvid_cap_t
    
class cap_pxc_csr_cfg_c_mac_test_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_test_in_t(string name = "cap_pxc_csr_cfg_c_mac_test_in_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_test_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > dw0_cpp_int_t;
        dw0_cpp_int_t int_var__dw0;
        void dw0 (const cpp_int  & l__val);
        cpp_int dw0() const;
    
}; // cap_pxc_csr_cfg_c_mac_test_in_t
    
class cap_pxc_csr_sta_c_brsw_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_brsw_t(string name = "cap_pxc_csr_sta_c_brsw_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_brsw_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tl_brsw_out_cpp_int_t;
        tl_brsw_out_cpp_int_t int_var__tl_brsw_out;
        void tl_brsw_out (const cpp_int  & l__val);
        cpp_int tl_brsw_out() const;
    
}; // cap_pxc_csr_sta_c_brsw_t
    
class cap_pxc_csr_cfg_c_brsw_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_brsw_t(string name = "cap_pxc_csr_cfg_c_brsw_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_brsw_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > tl_brsw_in_cpp_int_t;
        tl_brsw_in_cpp_int_t int_var__tl_brsw_in;
        void tl_brsw_in (const cpp_int  & l__val);
        cpp_int tl_brsw_in() const;
    
}; // cap_pxc_csr_cfg_c_brsw_t
    
class cap_pxc_csr_cfg_c_ecc_disable_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_ecc_disable_t(string name = "cap_pxc_csr_cfg_c_ecc_disable_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_ecc_disable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > rxbuf_cor_cpp_int_t;
        rxbuf_cor_cpp_int_t int_var__rxbuf_cor;
        void rxbuf_cor (const cpp_int  & l__val);
        cpp_int rxbuf_cor() const;
    
        typedef pu_cpp_int< 1 > rxbuf_det_cpp_int_t;
        rxbuf_det_cpp_int_t int_var__rxbuf_det;
        void rxbuf_det (const cpp_int  & l__val);
        cpp_int rxbuf_det() const;
    
}; // cap_pxc_csr_cfg_c_ecc_disable_t
    
class cap_pxc_csr_cfg_c_autonomous_linkwidth_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_autonomous_linkwidth_t(string name = "cap_pxc_csr_cfg_c_autonomous_linkwidth_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_autonomous_linkwidth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > pm_bwchange_cpp_int_t;
        pm_bwchange_cpp_int_t int_var__pm_bwchange;
        void pm_bwchange (const cpp_int  & l__val);
        cpp_int pm_bwchange() const;
    
}; // cap_pxc_csr_cfg_c_autonomous_linkwidth_t
    
class cap_pxc_csr_cfg_c_ltr_latency_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_ltr_latency_t(string name = "cap_pxc_csr_cfg_c_ltr_latency_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_ltr_latency_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 33 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_ltr_latency_t
    
class cap_pxc_csr_cfg_c_portgate_close_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_portgate_close_t(string name = "cap_pxc_csr_cfg_c_portgate_close_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_portgate_close_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_portgate_close_t
    
class cap_pxc_csr_cfg_c_portgate_open_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_portgate_open_t(string name = "cap_pxc_csr_cfg_c_portgate_open_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_portgate_open_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxc_csr_cfg_c_portgate_open_t
    
class cap_pxc_csr_sta_c_port_rst_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_port_rst_t(string name = "cap_pxc_csr_sta_c_port_rst_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_port_rst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > status_cpp_int_t;
        status_cpp_int_t int_var__status;
        void status (const cpp_int  & l__val);
        cpp_int status() const;
    
}; // cap_pxc_csr_sta_c_port_rst_t
    
class cap_pxc_csr_sta_c_port_mac_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_sta_c_port_mac_t(string name = "cap_pxc_csr_sta_c_port_mac_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_sta_c_port_mac_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > ltssm_cpp_int_t;
        ltssm_cpp_int_t int_var__ltssm;
        void ltssm (const cpp_int  & l__val);
        cpp_int ltssm() const;
    
        typedef pu_cpp_int< 1 > dl_up_cpp_int_t;
        dl_up_cpp_int_t int_var__dl_up;
        void dl_up (const cpp_int  & l__val);
        cpp_int dl_up() const;
    
        typedef pu_cpp_int< 1 > portgate_open_cpp_int_t;
        portgate_open_cpp_int_t int_var__portgate_open;
        void portgate_open (const cpp_int  & l__val);
        cpp_int portgate_open() const;
    
        typedef pu_cpp_int< 4 > lp_state_cpp_int_t;
        lp_state_cpp_int_t int_var__lp_state;
        void lp_state (const cpp_int  & l__val);
        cpp_int lp_state() const;
    
        typedef pu_cpp_int< 1 > trn2ltssm_l0s_cpp_int_t;
        trn2ltssm_l0s_cpp_int_t int_var__trn2ltssm_l0s;
        void trn2ltssm_l0s (const cpp_int  & l__val);
        cpp_int trn2ltssm_l0s() const;
    
        typedef pu_cpp_int< 1 > trn2ltssm_l1_cpp_int_t;
        trn2ltssm_l1_cpp_int_t int_var__trn2ltssm_l1;
        void trn2ltssm_l1 (const cpp_int  & l__val);
        cpp_int trn2ltssm_l1() const;
    
        typedef pu_cpp_int< 1 > trn2ltssm_l2_cpp_int_t;
        trn2ltssm_l2_cpp_int_t int_var__trn2ltssm_l2;
        void trn2ltssm_l2 (const cpp_int  & l__val);
        cpp_int trn2ltssm_l2() const;
    
        typedef pu_cpp_int< 1 > dpc_active_cpp_int_t;
        dpc_active_cpp_int_t int_var__dpc_active;
        void dpc_active (const cpp_int  & l__val);
        cpp_int dpc_active() const;
    
        typedef pu_cpp_int< 4 > rc_int_pinstate_cpp_int_t;
        rc_int_pinstate_cpp_int_t int_var__rc_int_pinstate;
        void rc_int_pinstate (const cpp_int  & l__val);
        cpp_int rc_int_pinstate() const;
    
        typedef pu_cpp_int< 1 > pm_turnoffstatus_cpp_int_t;
        pm_turnoffstatus_cpp_int_t int_var__pm_turnoffstatus;
        void pm_turnoffstatus (const cpp_int  & l__val);
        cpp_int pm_turnoffstatus() const;
    
        typedef pu_cpp_int< 4 > pm_clkstatus_cpp_int_t;
        pm_clkstatus_cpp_int_t int_var__pm_clkstatus;
        void pm_clkstatus (const cpp_int  & l__val);
        cpp_int pm_clkstatus() const;
    
        typedef pu_cpp_int< 1 > tlp_tx_inhibited_cpp_int_t;
        tlp_tx_inhibited_cpp_int_t int_var__tlp_tx_inhibited;
        void tlp_tx_inhibited (const cpp_int  & l__val);
        cpp_int tlp_tx_inhibited() const;
    
        typedef pu_cpp_int< 1 > dllp_tx_inhibited_cpp_int_t;
        dllp_tx_inhibited_cpp_int_t int_var__dllp_tx_inhibited;
        void dllp_tx_inhibited (const cpp_int  & l__val);
        cpp_int dllp_tx_inhibited() const;
    
}; // cap_pxc_csr_sta_c_port_mac_t
    
class cap_pxc_csr_cnt_c_tl_tx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cnt_c_tl_tx_t(string name = "cap_pxc_csr_cnt_c_tl_tx_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cnt_c_tl_tx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > tlp_cpp_int_t;
        tlp_cpp_int_t int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_tl_tx_t
    
class cap_pxc_csr_cnt_c_tl_rx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cnt_c_tl_rx_t(string name = "cap_pxc_csr_cnt_c_tl_rx_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cnt_c_tl_rx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > tlp_cpp_int_t;
        tlp_cpp_int_t int_var__tlp;
        void tlp (const cpp_int  & l__val);
        cpp_int tlp() const;
    
}; // cap_pxc_csr_cnt_c_tl_rx_t
    
class cap_pxc_csr_cfg_c_port_mac_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_port_mac_t(string name = "cap_pxc_csr_cfg_c_port_mac_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_port_mac_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > reset_cpp_int_t;
        reset_cpp_int_t int_var__reset;
        void reset (const cpp_int  & l__val);
        cpp_int reset() const;
    
        typedef pu_cpp_int< 1 > ltssm_en_cpp_int_t;
        ltssm_en_cpp_int_t int_var__ltssm_en;
        void ltssm_en (const cpp_int  & l__val);
        cpp_int ltssm_en() const;
    
        typedef pu_cpp_int< 1 > ltssm_en_portgate_qual_cpp_int_t;
        ltssm_en_portgate_qual_cpp_int_t int_var__ltssm_en_portgate_qual;
        void ltssm_en_portgate_qual (const cpp_int  & l__val);
        cpp_int ltssm_en_portgate_qual() const;
    
        typedef pu_cpp_int< 1 > cfg_retry_en_cpp_int_t;
        cfg_retry_en_cpp_int_t int_var__cfg_retry_en;
        void cfg_retry_en (const cpp_int  & l__val);
        cpp_int cfg_retry_en() const;
    
        typedef pu_cpp_int< 22 > tl_clock_freq_cpp_int_t;
        tl_clock_freq_cpp_int_t int_var__tl_clock_freq;
        void tl_clock_freq (const cpp_int  & l__val);
        cpp_int tl_clock_freq() const;
    
        typedef pu_cpp_int< 1 > port_type_cpp_int_t;
        port_type_cpp_int_t int_var__port_type;
        void port_type (const cpp_int  & l__val);
        cpp_int port_type() const;
    
        typedef pu_cpp_int< 1 > flush_marker_en_cpp_int_t;
        flush_marker_en_cpp_int_t int_var__flush_marker_en;
        void flush_marker_en (const cpp_int  & l__val);
        cpp_int flush_marker_en() const;
    
        typedef pu_cpp_int< 1 > tx_stream_cpp_int_t;
        tx_stream_cpp_int_t int_var__tx_stream;
        void tx_stream (const cpp_int  & l__val);
        cpp_int tx_stream() const;
    
        typedef pu_cpp_int< 3 > pl_exit_en_cpp_int_t;
        pl_exit_en_cpp_int_t int_var__pl_exit_en;
        void pl_exit_en (const cpp_int  & l__val);
        cpp_int pl_exit_en() const;
    
        typedef pu_cpp_int< 1 > pm_auxpwr_cpp_int_t;
        pm_auxpwr_cpp_int_t int_var__pm_auxpwr;
        void pm_auxpwr (const cpp_int  & l__val);
        cpp_int pm_auxpwr() const;
    
        typedef pu_cpp_int< 1 > pm_event_cpp_int_t;
        pm_event_cpp_int_t int_var__pm_event;
        void pm_event (const cpp_int  & l__val);
        cpp_int pm_event() const;
    
        typedef pu_cpp_int< 10 > pm_data_cpp_int_t;
        pm_data_cpp_int_t int_var__pm_data;
        void pm_data (const cpp_int  & l__val);
        cpp_int pm_data() const;
    
        typedef pu_cpp_int< 1 > pm_turnoffcontrol_cpp_int_t;
        pm_turnoffcontrol_cpp_int_t int_var__pm_turnoffcontrol;
        void pm_turnoffcontrol (const cpp_int  & l__val);
        cpp_int pm_turnoffcontrol() const;
    
        typedef pu_cpp_int< 1 > pm_clkcontrol_cpp_int_t;
        pm_clkcontrol_cpp_int_t int_var__pm_clkcontrol;
        void pm_clkcontrol (const cpp_int  & l__val);
        cpp_int pm_clkcontrol() const;
    
        typedef pu_cpp_int< 1 > inject_lcrc_cpp_int_t;
        inject_lcrc_cpp_int_t int_var__inject_lcrc;
        void inject_lcrc (const cpp_int  & l__val);
        cpp_int inject_lcrc() const;
    
        typedef pu_cpp_int< 1 > inject_ecrc_cpp_int_t;
        inject_ecrc_cpp_int_t int_var__inject_ecrc;
        void inject_ecrc (const cpp_int  & l__val);
        cpp_int inject_ecrc() const;
    
}; // cap_pxc_csr_cfg_c_port_mac_t
    
class cap_pxc_csr_cfg_c_mac_k_bar0windows_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_bar0windows_t(string name = "cap_pxc_csr_cfg_c_mac_k_bar0windows_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_bar0windows_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > bits227_224_cpp_int_t;
        bits227_224_cpp_int_t int_var__bits227_224;
        void bits227_224 (const cpp_int  & l__val);
        cpp_int bits227_224() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_bar0windows_t
    
class cap_pxc_csr_cfg_c_mac_k_equpreset16_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_equpreset16_t(string name = "cap_pxc_csr_cfg_c_mac_k_equpreset16_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_equpreset16_t();
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
    
}; // cap_pxc_csr_cfg_c_mac_k_equpreset16_t
    
class cap_pxc_csr_cfg_c_mac_k_equpreset_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_equpreset_t(string name = "cap_pxc_csr_cfg_c_mac_k_equpreset_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_equpreset_t();
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
    
}; // cap_pxc_csr_cfg_c_mac_k_equpreset_t
    
class cap_pxc_csr_cfg_c_mac_k_pciconf_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_pciconf_t(string name = "cap_pxc_csr_cfg_c_mac_k_pciconf_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_pciconf_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        word0_cpp_int_t int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        word1_cpp_int_t int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word2_cpp_int_t;
        word2_cpp_int_t int_var__word2;
        void word2 (const cpp_int  & l__val);
        cpp_int word2() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        word3_cpp_int_t int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
        typedef pu_cpp_int< 32 > word4_cpp_int_t;
        word4_cpp_int_t int_var__word4;
        void word4 (const cpp_int  & l__val);
        cpp_int word4() const;
    
        typedef pu_cpp_int< 32 > word5_cpp_int_t;
        word5_cpp_int_t int_var__word5;
        void word5 (const cpp_int  & l__val);
        cpp_int word5() const;
    
        typedef pu_cpp_int< 32 > word6_cpp_int_t;
        word6_cpp_int_t int_var__word6;
        void word6 (const cpp_int  & l__val);
        cpp_int word6() const;
    
        typedef pu_cpp_int< 32 > word7_cpp_int_t;
        word7_cpp_int_t int_var__word7;
        void word7 (const cpp_int  & l__val);
        cpp_int word7() const;
    
        typedef pu_cpp_int< 32 > word8_cpp_int_t;
        word8_cpp_int_t int_var__word8;
        void word8 (const cpp_int  & l__val);
        cpp_int word8() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_pciconf_t
    
class cap_pxc_csr_cfg_c_mac_k_pexconf_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_pexconf_t(string name = "cap_pxc_csr_cfg_c_mac_k_pexconf_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_pexconf_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        word0_cpp_int_t int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        word1_cpp_int_t int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word2_cpp_int_t;
        word2_cpp_int_t int_var__word2;
        void word2 (const cpp_int  & l__val);
        cpp_int word2() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        word3_cpp_int_t int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
        typedef pu_cpp_int< 32 > word4_cpp_int_t;
        word4_cpp_int_t int_var__word4;
        void word4 (const cpp_int  & l__val);
        cpp_int word4() const;
    
        typedef pu_cpp_int< 32 > word5_cpp_int_t;
        word5_cpp_int_t int_var__word5;
        void word5 (const cpp_int  & l__val);
        cpp_int word5() const;
    
        typedef pu_cpp_int< 32 > word6_cpp_int_t;
        word6_cpp_int_t int_var__word6;
        void word6 (const cpp_int  & l__val);
        cpp_int word6() const;
    
        typedef pu_cpp_int< 32 > word7_cpp_int_t;
        word7_cpp_int_t int_var__word7;
        void word7 (const cpp_int  & l__val);
        cpp_int word7() const;
    
        typedef pu_cpp_int< 32 > word8_cpp_int_t;
        word8_cpp_int_t int_var__word8;
        void word8 (const cpp_int  & l__val);
        cpp_int word8() const;
    
        typedef pu_cpp_int< 32 > word9_cpp_int_t;
        word9_cpp_int_t int_var__word9;
        void word9 (const cpp_int  & l__val);
        cpp_int word9() const;
    
        typedef pu_cpp_int< 32 > word10_cpp_int_t;
        word10_cpp_int_t int_var__word10;
        void word10 (const cpp_int  & l__val);
        cpp_int word10() const;
    
        typedef pu_cpp_int< 32 > word11_cpp_int_t;
        word11_cpp_int_t int_var__word11;
        void word11 (const cpp_int  & l__val);
        cpp_int word11() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_pexconf_t
    
class cap_pxc_csr_cfg_c_mac_k_lmr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_lmr_t(string name = "cap_pxc_csr_cfg_c_mac_k_lmr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_lmr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        word0_cpp_int_t int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        word1_cpp_int_t int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        word3_cpp_int_t int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_lmr_t
    
class cap_pxc_csr_cfg_c_mac_k_rx_cred_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_rx_cred_t(string name = "cap_pxc_csr_cfg_c_mac_k_rx_cred_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_rx_cred_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > word0_cpp_int_t;
        word0_cpp_int_t int_var__word0;
        void word0 (const cpp_int  & l__val);
        cpp_int word0() const;
    
        typedef pu_cpp_int< 32 > word1_cpp_int_t;
        word1_cpp_int_t int_var__word1;
        void word1 (const cpp_int  & l__val);
        cpp_int word1() const;
    
        typedef pu_cpp_int< 32 > word3_cpp_int_t;
        word3_cpp_int_t int_var__word3;
        void word3 (const cpp_int  & l__val);
        cpp_int word3() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_rx_cred_t
    
class cap_pxc_csr_cfg_c_mac_k_gen_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_cfg_c_mac_k_gen_t(string name = "cap_pxc_csr_cfg_c_mac_k_gen_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_cfg_c_mac_k_gen_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > spec_version_cpp_int_t;
        spec_version_cpp_int_t int_var__spec_version;
        void spec_version (const cpp_int  & l__val);
        cpp_int spec_version() const;
    
        typedef pu_cpp_int< 4 > port_type_cpp_int_t;
        port_type_cpp_int_t int_var__port_type;
        void port_type (const cpp_int  & l__val);
        cpp_int port_type() const;
    
        typedef pu_cpp_int< 1 > sris_mode_cpp_int_t;
        sris_mode_cpp_int_t int_var__sris_mode;
        void sris_mode (const cpp_int  & l__val);
        cpp_int sris_mode() const;
    
        typedef pu_cpp_int< 1 > gen1_supported_cpp_int_t;
        gen1_supported_cpp_int_t int_var__gen1_supported;
        void gen1_supported (const cpp_int  & l__val);
        cpp_int gen1_supported() const;
    
        typedef pu_cpp_int< 1 > gen2_supported_cpp_int_t;
        gen2_supported_cpp_int_t int_var__gen2_supported;
        void gen2_supported (const cpp_int  & l__val);
        cpp_int gen2_supported() const;
    
        typedef pu_cpp_int< 1 > gen3_supported_cpp_int_t;
        gen3_supported_cpp_int_t int_var__gen3_supported;
        void gen3_supported (const cpp_int  & l__val);
        cpp_int gen3_supported() const;
    
        typedef pu_cpp_int< 1 > gen4_supported_cpp_int_t;
        gen4_supported_cpp_int_t int_var__gen4_supported;
        void gen4_supported (const cpp_int  & l__val);
        cpp_int gen4_supported() const;
    
        typedef pu_cpp_int< 3 > reserved1_cpp_int_t;
        reserved1_cpp_int_t int_var__reserved1;
        void reserved1 (const cpp_int  & l__val);
        cpp_int reserved1() const;
    
        typedef pu_cpp_int< 1 > bfm_mode_cpp_int_t;
        bfm_mode_cpp_int_t int_var__bfm_mode;
        void bfm_mode (const cpp_int  & l__val);
        cpp_int bfm_mode() const;
    
        typedef pu_cpp_int< 1 > lane_reverse_en_cpp_int_t;
        lane_reverse_en_cpp_int_t int_var__lane_reverse_en;
        void lane_reverse_en (const cpp_int  & l__val);
        cpp_int lane_reverse_en() const;
    
        typedef pu_cpp_int< 3 > reserved2_cpp_int_t;
        reserved2_cpp_int_t int_var__reserved2;
        void reserved2 (const cpp_int  & l__val);
        cpp_int reserved2() const;
    
        typedef pu_cpp_int< 1 > rx_cutthru_en_cpp_int_t;
        rx_cutthru_en_cpp_int_t int_var__rx_cutthru_en;
        void rx_cutthru_en (const cpp_int  & l__val);
        cpp_int rx_cutthru_en() const;
    
        typedef pu_cpp_int< 1 > tx_cutthru_en_cpp_int_t;
        tx_cutthru_en_cpp_int_t int_var__tx_cutthru_en;
        void tx_cutthru_en (const cpp_int  & l__val);
        cpp_int tx_cutthru_en() const;
    
        typedef pu_cpp_int< 1 > eq_ph23_en_cpp_int_t;
        eq_ph23_en_cpp_int_t int_var__eq_ph23_en;
        void eq_ph23_en (const cpp_int  & l__val);
        cpp_int eq_ph23_en() const;
    
        typedef pu_cpp_int< 4 > disable_lanes_cpp_int_t;
        disable_lanes_cpp_int_t int_var__disable_lanes;
        void disable_lanes (const cpp_int  & l__val);
        cpp_int disable_lanes() const;
    
        typedef pu_cpp_int< 1 > rxelecidle_cfg_cpp_int_t;
        rxelecidle_cfg_cpp_int_t int_var__rxelecidle_cfg;
        void rxelecidle_cfg (const cpp_int  & l__val);
        cpp_int rxelecidle_cfg() const;
    
        typedef pu_cpp_int< 1 > reserved3_cpp_int_t;
        reserved3_cpp_int_t int_var__reserved3;
        void reserved3 (const cpp_int  & l__val);
        cpp_int reserved3() const;
    
        typedef pu_cpp_int< 1 > pie8_cpp_int_t;
        pie8_cpp_int_t int_var__pie8;
        void pie8 (const cpp_int  & l__val);
        cpp_int pie8() const;
    
        typedef pu_cpp_int< 1 > ecrc_gen_mode_cpp_int_t;
        ecrc_gen_mode_cpp_int_t int_var__ecrc_gen_mode;
        void ecrc_gen_mode (const cpp_int  & l__val);
        cpp_int ecrc_gen_mode() const;
    
        typedef pu_cpp_int< 1 > ext_intr_mode_cpp_int_t;
        ext_intr_mode_cpp_int_t int_var__ext_intr_mode;
        void ext_intr_mode (const cpp_int  & l__val);
        cpp_int ext_intr_mode() const;
    
        typedef pu_cpp_int< 1 > tx_ecc_nullify_cpp_int_t;
        tx_ecc_nullify_cpp_int_t int_var__tx_ecc_nullify;
        void tx_ecc_nullify (const cpp_int  & l__val);
        cpp_int tx_ecc_nullify() const;
    
        typedef pu_cpp_int< 1 > tx_ecc_flush_cpp_int_t;
        tx_ecc_flush_cpp_int_t int_var__tx_ecc_flush;
        void tx_ecc_flush (const cpp_int  & l__val);
        cpp_int tx_ecc_flush() const;
    
        typedef pu_cpp_int< 1 > rx_wdog_en_cpp_int_t;
        rx_wdog_en_cpp_int_t int_var__rx_wdog_en;
        void rx_wdog_en (const cpp_int  & l__val);
        cpp_int rx_wdog_en() const;
    
        typedef pu_cpp_int< 1 > upcfg_en_cpp_int_t;
        upcfg_en_cpp_int_t int_var__upcfg_en;
        void upcfg_en (const cpp_int  & l__val);
        cpp_int upcfg_en() const;
    
        typedef pu_cpp_int< 1 > rxvalid_filter_cpp_int_t;
        rxvalid_filter_cpp_int_t int_var__rxvalid_filter;
        void rxvalid_filter (const cpp_int  & l__val);
        cpp_int rxvalid_filter() const;
    
        typedef pu_cpp_int< 26 > reserved4_cpp_int_t;
        reserved4_cpp_int_t int_var__reserved4;
        void reserved4 (const cpp_int  & l__val);
        cpp_int reserved4() const;
    
}; // cap_pxc_csr_cfg_c_mac_k_gen_t
    
class cap_pxc_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxc_csr_t(string name = "cap_pxc_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxc_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxc_csr_cfg_c_mac_k_gen_t cfg_c_mac_k_gen;
    
        cap_pxc_csr_cfg_c_mac_k_rx_cred_t cfg_c_mac_k_rx_cred;
    
        cap_pxc_csr_cfg_c_mac_k_lmr_t cfg_c_mac_k_lmr;
    
        cap_pxc_csr_cfg_c_mac_k_pexconf_t cfg_c_mac_k_pexconf;
    
        cap_pxc_csr_cfg_c_mac_k_pciconf_t cfg_c_mac_k_pciconf;
    
        cap_pxc_csr_cfg_c_mac_k_equpreset_t cfg_c_mac_k_equpreset;
    
        cap_pxc_csr_cfg_c_mac_k_equpreset16_t cfg_c_mac_k_equpreset16;
    
        cap_pxc_csr_cfg_c_mac_k_bar0windows_t cfg_c_mac_k_bar0windows;
    
        cap_pxc_csr_cfg_c_port_mac_t cfg_c_port_mac;
    
        cap_pxc_csr_cnt_c_tl_rx_t cnt_c_tl_rx;
    
        cap_pxc_csr_cnt_c_tl_tx_t cnt_c_tl_tx;
    
        cap_pxc_csr_sta_c_port_mac_t sta_c_port_mac;
    
        cap_pxc_csr_sta_c_port_rst_t sta_c_port_rst;
    
        cap_pxc_csr_cfg_c_portgate_open_t cfg_c_portgate_open;
    
        cap_pxc_csr_cfg_c_portgate_close_t cfg_c_portgate_close;
    
        cap_pxc_csr_cfg_c_ltr_latency_t cfg_c_ltr_latency;
    
        cap_pxc_csr_cfg_c_autonomous_linkwidth_t cfg_c_autonomous_linkwidth;
    
        cap_pxc_csr_cfg_c_ecc_disable_t cfg_c_ecc_disable;
    
        cap_pxc_csr_cfg_c_brsw_t cfg_c_brsw;
    
        cap_pxc_csr_sta_c_brsw_t sta_c_brsw;
    
        cap_pxc_csr_cfg_c_mac_test_in_t cfg_c_mac_test_in;
    
        cap_pxc_csr_cfg_c_mac_ssvid_cap_t cfg_c_mac_ssvid_cap;
    
        cap_pxc_csr_sta_c_tx_fc_credits_t sta_c_tx_fc_credits;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_0_t sta_c_ecc_rxbuf_0;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_1_t sta_c_ecc_rxbuf_1;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_2_t sta_c_ecc_rxbuf_2;
    
        cap_pxc_csr_sta_c_ecc_rxbuf_3_t sta_c_ecc_rxbuf_3;
    
        cap_pxc_csr_sat_c_port_cnt0_t sat_c_port_cnt0;
    
        cap_pxc_csr_sat_c_port_cnt1_t sat_c_port_cnt1;
    
        cap_pxc_csr_sat_c_port_cnt2_t sat_c_port_cnt2;
    
        cap_pxc_csr_sat_c_port_cnt3_t sat_c_port_cnt3;
    
        cap_pxc_csr_sat_c_port_cnt4_t sat_c_port_cnt4;
    
        cap_pxc_csr_sat_c_port_cnt5_t sat_c_port_cnt5;
    
        cap_pxc_csr_sat_c_port_cnt6_t sat_c_port_cnt6;
    
        cap_pxc_csr_sat_c_port_cnt7_t sat_c_port_cnt7;
    
        cap_pxc_csr_sat_c_port_cnt8_t sat_c_port_cnt8;
    
        cap_pxc_csr_sat_c_port_cnt9_t sat_c_port_cnt9;
    
        cap_pxc_csr_sat_c_port_cnt10_t sat_c_port_cnt10;
    
        cap_pxc_csr_sat_c_port_cnt11_t sat_c_port_cnt11;
    
        cap_pxc_csr_sat_c_port_cnt12_t sat_c_port_cnt12;
    
        cap_pxc_csr_sat_c_port_cnt_rx_malform_tlp_t sat_c_port_cnt_rx_malform_tlp;
    
        cap_pxc_csr_sat_c_port_cnt_rx_framing_err_t sat_c_port_cnt_rx_framing_err;
    
        cap_pxc_csr_sat_c_port_cnt_txbfr_overflow_t sat_c_port_cnt_txbfr_overflow;
    
        cap_pxc_csr_sta_c_port_phystatus_t sta_c_port_phystatus;
    
        cap_pxc_csr_dhs_c_mac_apb_t dhs_c_mac_apb;
    
}; // cap_pxc_csr_t
    
#endif // CAP_PXC_CSR_H
        