
#ifndef CAP_DPPSTATS_CSR_H
#define CAP_DPPSTATS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dppstats_csr_SAT_dpp_spare_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_SAT_dpp_spare_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_SAT_dpp_spare_err_t(string name = "cap_dppstats_csr_SAT_dpp_spare_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_SAT_dpp_spare_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > spare_0_cpp_int_t;
        cpp_int int_var__spare_0;
        void spare_0 (const cpp_int  & l__val);
        cpp_int spare_0() const;
    
        typedef pu_cpp_int< 8 > spare_1_cpp_int_t;
        cpp_int int_var__spare_1;
        void spare_1 (const cpp_int  & l__val);
        cpp_int spare_1() const;
    
        typedef pu_cpp_int< 8 > spare_2_cpp_int_t;
        cpp_int int_var__spare_2;
        void spare_2 (const cpp_int  & l__val);
        cpp_int spare_2() const;
    
        typedef pu_cpp_int< 8 > spare_3_cpp_int_t;
        cpp_int int_var__spare_3;
        void spare_3 (const cpp_int  & l__val);
        cpp_int spare_3() const;
    
        typedef pu_cpp_int< 8 > spare_4_cpp_int_t;
        cpp_int int_var__spare_4;
        void spare_4 (const cpp_int  & l__val);
        cpp_int spare_4() const;
    
        typedef pu_cpp_int< 8 > spare_5_cpp_int_t;
        cpp_int int_var__spare_5;
        void spare_5 (const cpp_int  & l__val);
        cpp_int spare_5() const;
    
        typedef pu_cpp_int< 8 > spare_6_cpp_int_t;
        cpp_int int_var__spare_6;
        void spare_6 (const cpp_int  & l__val);
        cpp_int spare_6() const;
    
        typedef pu_cpp_int< 8 > spare_7_cpp_int_t;
        cpp_int int_var__spare_7;
        void spare_7 (const cpp_int  & l__val);
        cpp_int spare_7() const;
    
        typedef pu_cpp_int< 8 > spare_8_cpp_int_t;
        cpp_int int_var__spare_8;
        void spare_8 (const cpp_int  & l__val);
        cpp_int spare_8() const;
    
        typedef pu_cpp_int< 8 > spare_9_cpp_int_t;
        cpp_int int_var__spare_9;
        void spare_9 (const cpp_int  & l__val);
        cpp_int spare_9() const;
    
        typedef pu_cpp_int< 8 > spare_10_cpp_int_t;
        cpp_int int_var__spare_10;
        void spare_10 (const cpp_int  & l__val);
        cpp_int spare_10() const;
    
        typedef pu_cpp_int< 8 > spare_11_cpp_int_t;
        cpp_int int_var__spare_11;
        void spare_11 (const cpp_int  & l__val);
        cpp_int spare_11() const;
    
        typedef pu_cpp_int< 8 > spare_12_cpp_int_t;
        cpp_int int_var__spare_12;
        void spare_12 (const cpp_int  & l__val);
        cpp_int spare_12() const;
    
        typedef pu_cpp_int< 8 > spare_13_cpp_int_t;
        cpp_int int_var__spare_13;
        void spare_13 (const cpp_int  & l__val);
        cpp_int spare_13() const;
    
        typedef pu_cpp_int< 8 > spare_14_cpp_int_t;
        cpp_int int_var__spare_14;
        void spare_14 (const cpp_int  & l__val);
        cpp_int spare_14() const;
    
        typedef pu_cpp_int< 8 > spare_15_cpp_int_t;
        cpp_int int_var__spare_15;
        void spare_15 (const cpp_int  & l__val);
        cpp_int spare_15() const;
    
        typedef pu_cpp_int< 8 > spare_16_cpp_int_t;
        cpp_int int_var__spare_16;
        void spare_16 (const cpp_int  & l__val);
        cpp_int spare_16() const;
    
        typedef pu_cpp_int< 8 > spare_17_cpp_int_t;
        cpp_int int_var__spare_17;
        void spare_17 (const cpp_int  & l__val);
        cpp_int spare_17() const;
    
        typedef pu_cpp_int< 8 > spare_18_cpp_int_t;
        cpp_int int_var__spare_18;
        void spare_18 (const cpp_int  & l__val);
        cpp_int spare_18() const;
    
        typedef pu_cpp_int< 8 > spare_19_cpp_int_t;
        cpp_int int_var__spare_19;
        void spare_19 (const cpp_int  & l__val);
        cpp_int spare_19() const;
    
        typedef pu_cpp_int< 8 > spare_20_cpp_int_t;
        cpp_int int_var__spare_20;
        void spare_20 (const cpp_int  & l__val);
        cpp_int spare_20() const;
    
        typedef pu_cpp_int< 8 > spare_21_cpp_int_t;
        cpp_int int_var__spare_21;
        void spare_21 (const cpp_int  & l__val);
        cpp_int spare_21() const;
    
        typedef pu_cpp_int< 8 > spare_22_cpp_int_t;
        cpp_int int_var__spare_22;
        void spare_22 (const cpp_int  & l__val);
        cpp_int spare_22() const;
    
        typedef pu_cpp_int< 8 > spare_23_cpp_int_t;
        cpp_int int_var__spare_23;
        void spare_23 (const cpp_int  & l__val);
        cpp_int spare_23() const;
    
        typedef pu_cpp_int< 8 > spare_24_cpp_int_t;
        cpp_int int_var__spare_24;
        void spare_24 (const cpp_int  & l__val);
        cpp_int spare_24() const;
    
        typedef pu_cpp_int< 8 > spare_25_cpp_int_t;
        cpp_int int_var__spare_25;
        void spare_25 (const cpp_int  & l__val);
        cpp_int spare_25() const;
    
        typedef pu_cpp_int< 8 > spare_26_cpp_int_t;
        cpp_int int_var__spare_26;
        void spare_26 (const cpp_int  & l__val);
        cpp_int spare_26() const;
    
        typedef pu_cpp_int< 8 > spare_27_cpp_int_t;
        cpp_int int_var__spare_27;
        void spare_27 (const cpp_int  & l__val);
        cpp_int spare_27() const;
    
        typedef pu_cpp_int< 8 > spare_28_cpp_int_t;
        cpp_int int_var__spare_28;
        void spare_28 (const cpp_int  & l__val);
        cpp_int spare_28() const;
    
        typedef pu_cpp_int< 8 > spare_29_cpp_int_t;
        cpp_int int_var__spare_29;
        void spare_29 (const cpp_int  & l__val);
        cpp_int spare_29() const;
    
        typedef pu_cpp_int< 8 > spare_30_cpp_int_t;
        cpp_int int_var__spare_30;
        void spare_30 (const cpp_int  & l__val);
        cpp_int spare_30() const;
    
        typedef pu_cpp_int< 8 > spare_31_cpp_int_t;
        cpp_int int_var__spare_31;
        void spare_31 (const cpp_int  & l__val);
        cpp_int spare_31() const;
    
}; // cap_dppstats_csr_SAT_dpp_spare_err_t
    
class cap_dppstats_csr_SAT_dpp_ff_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_SAT_dpp_ff_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_SAT_dpp_ff_err_t(string name = "cap_dppstats_csr_SAT_dpp_ff_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_SAT_dpp_ff_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > phv_ff_overflow_cpp_int_t;
        cpp_int int_var__phv_ff_overflow;
        void phv_ff_overflow (const cpp_int  & l__val);
        cpp_int phv_ff_overflow() const;
    
        typedef pu_cpp_int< 8 > ohi_ff_overflow_cpp_int_t;
        cpp_int int_var__ohi_ff_overflow;
        void ohi_ff_overflow (const cpp_int  & l__val);
        cpp_int ohi_ff_overflow() const;
    
        typedef pu_cpp_int< 8 > pkt_size_ff_ovflow_cpp_int_t;
        cpp_int int_var__pkt_size_ff_ovflow;
        void pkt_size_ff_ovflow (const cpp_int  & l__val);
        cpp_int pkt_size_ff_ovflow() const;
    
        typedef pu_cpp_int< 8 > pkt_size_ff_undflow_cpp_int_t;
        cpp_int int_var__pkt_size_ff_undflow;
        void pkt_size_ff_undflow (const cpp_int  & l__val);
        cpp_int pkt_size_ff_undflow() const;
    
        typedef pu_cpp_int< 8 > csum_phv_ff_ovflow_cpp_int_t;
        cpp_int int_var__csum_phv_ff_ovflow;
        void csum_phv_ff_ovflow (const cpp_int  & l__val);
        cpp_int csum_phv_ff_ovflow() const;
    
        typedef pu_cpp_int< 8 > csum_phv_ff_undflow_cpp_int_t;
        cpp_int int_var__csum_phv_ff_undflow;
        void csum_phv_ff_undflow (const cpp_int  & l__val);
        cpp_int csum_phv_ff_undflow() const;
    
        typedef pu_cpp_int< 8 > ptr_credit_ovflow_cpp_int_t;
        cpp_int int_var__ptr_credit_ovflow;
        void ptr_credit_ovflow (const cpp_int  & l__val);
        cpp_int ptr_credit_ovflow() const;
    
        typedef pu_cpp_int< 8 > ptr_credit_undflow_cpp_int_t;
        cpp_int int_var__ptr_credit_undflow;
        void ptr_credit_undflow (const cpp_int  & l__val);
        cpp_int ptr_credit_undflow() const;
    
        typedef pu_cpp_int< 8 > pkt_credit_ovflow_cpp_int_t;
        cpp_int int_var__pkt_credit_ovflow;
        void pkt_credit_ovflow (const cpp_int  & l__val);
        cpp_int pkt_credit_ovflow() const;
    
        typedef pu_cpp_int< 8 > pkt_credit_undflow_cpp_int_t;
        cpp_int int_var__pkt_credit_undflow;
        void pkt_credit_undflow (const cpp_int  & l__val);
        cpp_int pkt_credit_undflow() const;
    
}; // cap_dppstats_csr_SAT_dpp_ff_err_t
    
class cap_dppstats_csr_SAT_dpp_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_SAT_dpp_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_SAT_dpp_err_t(string name = "cap_dppstats_csr_SAT_dpp_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_SAT_dpp_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_phv_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_phv_sop_no_eop;
        void err_phv_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop() const;
    
        typedef pu_cpp_int< 8 > err_phv_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_phv_eop_no_sop;
        void err_phv_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop() const;
    
        typedef pu_cpp_int< 8 > err_ohi_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_ohi_sop_no_eop;
        void err_ohi_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop() const;
    
        typedef pu_cpp_int< 8 > err_ohi_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_ohi_eop_no_sop;
        void err_ohi_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop() const;
    
        typedef pu_cpp_int< 8 > err_framer_credit_overrun_cpp_int_t;
        cpp_int int_var__err_framer_credit_overrun;
        void err_framer_credit_overrun (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun() const;
    
        typedef pu_cpp_int< 8 > err_packets_in_flight_credit_overrun_cpp_int_t;
        cpp_int int_var__err_packets_in_flight_credit_overrun;
        void err_packets_in_flight_credit_overrun (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun() const;
    
        typedef pu_cpp_int< 8 > err_null_hdr_vld_cpp_int_t;
        cpp_int int_var__err_null_hdr_vld;
        void err_null_hdr_vld (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld() const;
    
        typedef pu_cpp_int< 8 > err_null_hdrfld_vld_cpp_int_t;
        cpp_int int_var__err_null_hdrfld_vld;
        void err_null_hdrfld_vld (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld() const;
    
        typedef pu_cpp_int< 8 > err_max_pkt_size_cpp_int_t;
        cpp_int int_var__err_max_pkt_size;
        void err_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_max_pkt_size() const;
    
        typedef pu_cpp_int< 8 > err_max_active_hdrs_cpp_int_t;
        cpp_int int_var__err_max_active_hdrs;
        void err_max_active_hdrs (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs() const;
    
        typedef pu_cpp_int< 8 > err_phv_no_data_reference_ohi_cpp_int_t;
        cpp_int int_var__err_phv_no_data_reference_ohi;
        void err_phv_no_data_reference_ohi (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi() const;
    
        typedef pu_cpp_int< 8 > err_csum_multiple_hdr_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr;
        void err_csum_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr() const;
    
        typedef pu_cpp_int< 8 > err_csum_multiple_hdr_copy_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_copy;
        void err_csum_multiple_hdr_copy (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_copy() const;
    
        typedef pu_cpp_int< 8 > err_crc_multiple_hdr_cpp_int_t;
        cpp_int int_var__err_crc_multiple_hdr;
        void err_crc_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr() const;
    
        typedef pu_cpp_int< 8 > err_ptr_fifo_credit_overrun_cpp_int_t;
        cpp_int int_var__err_ptr_fifo_credit_overrun;
        void err_ptr_fifo_credit_overrun (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun() const;
    
        typedef pu_cpp_int< 8 > err_clip_max_pkt_size_cpp_int_t;
        cpp_int int_var__err_clip_max_pkt_size;
        void err_clip_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size() const;
    
        typedef pu_cpp_int< 8 > err_min_pkt_size_cpp_int_t;
        cpp_int int_var__err_min_pkt_size;
        void err_min_pkt_size (const cpp_int  & l__val);
        cpp_int err_min_pkt_size() const;
    
}; // cap_dppstats_csr_SAT_dpp_err_t
    
class cap_dppstats_csr_CNT_dpp_spare_15_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_15_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_15_t(string name = "cap_dppstats_csr_CNT_dpp_spare_15_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_15_t
    
class cap_dppstats_csr_CNT_dpp_spare_14_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_14_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_14_t(string name = "cap_dppstats_csr_CNT_dpp_spare_14_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_14_t
    
class cap_dppstats_csr_CNT_dpp_spare_13_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_13_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_13_t(string name = "cap_dppstats_csr_CNT_dpp_spare_13_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_13_t
    
class cap_dppstats_csr_CNT_dpp_spare_12_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_12_t(string name = "cap_dppstats_csr_CNT_dpp_spare_12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_12_t
    
class cap_dppstats_csr_CNT_dpp_spare_11_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_11_t(string name = "cap_dppstats_csr_CNT_dpp_spare_11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_11_t
    
class cap_dppstats_csr_CNT_dpp_spare_10_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_10_t(string name = "cap_dppstats_csr_CNT_dpp_spare_10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_10_t
    
class cap_dppstats_csr_CNT_dpp_spare_9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_9_t(string name = "cap_dppstats_csr_CNT_dpp_spare_9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_9_t
    
class cap_dppstats_csr_CNT_dpp_spare_8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_8_t(string name = "cap_dppstats_csr_CNT_dpp_spare_8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_8_t
    
class cap_dppstats_csr_CNT_dpp_spare_7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_7_t(string name = "cap_dppstats_csr_CNT_dpp_spare_7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_7_t
    
class cap_dppstats_csr_CNT_dpp_spare_6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_6_t(string name = "cap_dppstats_csr_CNT_dpp_spare_6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_6_t
    
class cap_dppstats_csr_CNT_dpp_spare_5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_5_t(string name = "cap_dppstats_csr_CNT_dpp_spare_5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_5_t
    
class cap_dppstats_csr_CNT_dpp_spare_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_4_t(string name = "cap_dppstats_csr_CNT_dpp_spare_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_4_t
    
class cap_dppstats_csr_CNT_dpp_spare_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_3_t(string name = "cap_dppstats_csr_CNT_dpp_spare_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_3_t
    
class cap_dppstats_csr_CNT_dpp_spare_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_2_t(string name = "cap_dppstats_csr_CNT_dpp_spare_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_2_t
    
class cap_dppstats_csr_CNT_dpp_spare_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_1_t(string name = "cap_dppstats_csr_CNT_dpp_spare_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_1_t
    
class cap_dppstats_csr_CNT_dpp_spare_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_spare_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_spare_0_t(string name = "cap_dppstats_csr_CNT_dpp_spare_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_spare_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_spare_0_t
    
class cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t(string name = "cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t
    
class cap_dppstats_csr_CNT_dpp_dpr_xn_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_dpr_xn_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_dpr_xn_t(string name = "cap_dppstats_csr_CNT_dpp_dpr_xn_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_dpr_xn_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_dpr_xn_t
    
class cap_dppstats_csr_CNT_dpp_ohi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_ohi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_ohi_t(string name = "cap_dppstats_csr_CNT_dpp_ohi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_ohi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_ohi_t
    
class cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t(string name = "cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t
    
class cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t(string name = "cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t
    
class cap_dppstats_csr_CNT_dpp_phv_no_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_phv_no_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_phv_no_data_t(string name = "cap_dppstats_csr_CNT_dpp_phv_no_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_phv_no_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_phv_no_data_t
    
class cap_dppstats_csr_CNT_dpp_phv_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_phv_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_phv_drop_t(string name = "cap_dppstats_csr_CNT_dpp_phv_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_phv_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_phv_drop_t
    
class cap_dppstats_csr_CNT_dpp_phv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_phv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_phv_t(string name = "cap_dppstats_csr_CNT_dpp_phv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_phv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_phv_t
    
class cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t(string name = "cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t
    
class cap_dppstats_csr_CNT_dpp_ohi_lines_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_ohi_lines_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_ohi_lines_t(string name = "cap_dppstats_csr_CNT_dpp_ohi_lines_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_ohi_lines_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_ohi_lines_t
    
class cap_dppstats_csr_CNT_dpp_phv_lines_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_phv_lines_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_phv_lines_t(string name = "cap_dppstats_csr_CNT_dpp_phv_lines_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_phv_lines_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dppstats_csr_CNT_dpp_phv_lines_t
    
class cap_dppstats_csr_CNT_dpp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_CNT_dpp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_CNT_dpp_t(string name = "cap_dppstats_csr_CNT_dpp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_CNT_dpp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > ma_phv_sop_cpp_int_t;
        cpp_int int_var__ma_phv_sop;
        void ma_phv_sop (const cpp_int  & l__val);
        cpp_int ma_phv_sop() const;
    
        typedef pu_cpp_int< 32 > ma_phv_eop_cpp_int_t;
        cpp_int int_var__ma_phv_eop;
        void ma_phv_eop (const cpp_int  & l__val);
        cpp_int ma_phv_eop() const;
    
        typedef pu_cpp_int< 32 > pa_ohi_sop_cpp_int_t;
        cpp_int int_var__pa_ohi_sop;
        void pa_ohi_sop (const cpp_int  & l__val);
        cpp_int pa_ohi_sop() const;
    
        typedef pu_cpp_int< 32 > pa_ohi_eop_cpp_int_t;
        cpp_int int_var__pa_ohi_eop;
        void pa_ohi_eop (const cpp_int  & l__val);
        cpp_int pa_ohi_eop() const;
    
        typedef pu_cpp_int< 32 > dpr_phv_sop_cpp_int_t;
        cpp_int int_var__dpr_phv_sop;
        void dpr_phv_sop (const cpp_int  & l__val);
        cpp_int dpr_phv_sop() const;
    
        typedef pu_cpp_int< 32 > dpr_phv_eop_cpp_int_t;
        cpp_int int_var__dpr_phv_eop;
        void dpr_phv_eop (const cpp_int  & l__val);
        cpp_int dpr_phv_eop() const;
    
        typedef pu_cpp_int< 32 > dpr_ohi_sop_cpp_int_t;
        cpp_int int_var__dpr_ohi_sop;
        void dpr_ohi_sop (const cpp_int  & l__val);
        cpp_int dpr_ohi_sop() const;
    
        typedef pu_cpp_int< 32 > dpr_ohi_eop_cpp_int_t;
        cpp_int int_var__dpr_ohi_eop;
        void dpr_ohi_eop (const cpp_int  & l__val);
        cpp_int dpr_ohi_eop() const;
    
        typedef pu_cpp_int< 32 > dpr_hdrfld_select_arr_vld_cpp_int_t;
        cpp_int int_var__dpr_hdrfld_select_arr_vld;
        void dpr_hdrfld_select_arr_vld (const cpp_int  & l__val);
        cpp_int dpr_hdrfld_select_arr_vld() const;
    
        typedef pu_cpp_int< 32 > dpr_csum_crc_bundle_cpp_int_t;
        cpp_int int_var__dpr_csum_crc_bundle;
        void dpr_csum_crc_bundle (const cpp_int  & l__val);
        cpp_int dpr_csum_crc_bundle() const;
    
}; // cap_dppstats_csr_CNT_dpp_t
    
class cap_dppstats_csr_sta_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sta_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sta_spare_t(string name = "cap_dppstats_csr_sta_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sta_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > status_spare_cpp_int_t;
        cpp_int int_var__status_spare;
        void status_spare (const cpp_int  & l__val);
        cpp_int status_spare() const;
    
}; // cap_dppstats_csr_sta_spare_t
    
class cap_dppstats_csr_sta_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sta_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sta_global_t(string name = "cap_dppstats_csr_sta_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sta_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > phv_ff_srdy_cpp_int_t;
        cpp_int int_var__phv_ff_srdy;
        void phv_ff_srdy (const cpp_int  & l__val);
        cpp_int phv_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > phv_ff_drdy_cpp_int_t;
        cpp_int int_var__phv_ff_drdy;
        void phv_ff_drdy (const cpp_int  & l__val);
        cpp_int phv_ff_drdy() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_srdy_cpp_int_t;
        cpp_int int_var__ohi_ff_srdy;
        void ohi_ff_srdy (const cpp_int  & l__val);
        cpp_int ohi_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_drdy_cpp_int_t;
        cpp_int int_var__ohi_ff_drdy;
        void ohi_ff_drdy (const cpp_int  & l__val);
        cpp_int ohi_ff_drdy() const;
    
        typedef pu_cpp_int< 1 > dprphv_ff_srdy_cpp_int_t;
        cpp_int int_var__dprphv_ff_srdy;
        void dprphv_ff_srdy (const cpp_int  & l__val);
        cpp_int dprphv_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > dprphv_ff_drdy_cpp_int_t;
        cpp_int int_var__dprphv_ff_drdy;
        void dprphv_ff_drdy (const cpp_int  & l__val);
        cpp_int dprphv_ff_drdy() const;
    
        typedef pu_cpp_int< 1 > dprohi_ff_srdy_cpp_int_t;
        cpp_int int_var__dprohi_ff_srdy;
        void dprohi_ff_srdy (const cpp_int  & l__val);
        cpp_int dprohi_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > dprohi_ff_drdy_cpp_int_t;
        cpp_int int_var__dprohi_ff_drdy;
        void dprohi_ff_drdy (const cpp_int  & l__val);
        cpp_int dprohi_ff_drdy() const;
    
        typedef pu_cpp_int< 2 > phv_framer_credit_cpp_int_t;
        cpp_int int_var__phv_framer_credit;
        void phv_framer_credit (const cpp_int  & l__val);
        cpp_int phv_framer_credit() const;
    
        typedef pu_cpp_int< 4 > packet_in_flight_credit_cpp_int_t;
        cpp_int int_var__packet_in_flight_credit;
        void packet_in_flight_credit (const cpp_int  & l__val);
        cpp_int packet_in_flight_credit() const;
    
        typedef pu_cpp_int< 6 > framer_ptr_fifo_credit_cpp_int_t;
        cpp_int int_var__framer_ptr_fifo_credit;
        void framer_ptr_fifo_credit (const cpp_int  & l__val);
        cpp_int framer_ptr_fifo_credit() const;
    
}; // cap_dppstats_csr_sta_global_t
    
class cap_dppstats_csr_sym_hdrfld_offset3_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_offset3_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_offset3_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset3_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_offset3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1040 > hdrfld_offset3_cpp_int_t;
        cpp_int int_var__hdrfld_offset3;
        void hdrfld_offset3 (const cpp_int  & l__val);
        cpp_int hdrfld_offset3() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset3_capture_t
    
class cap_dppstats_csr_sym_hdrfld_offset2_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_offset2_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_offset2_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset2_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_offset2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_offset2_cpp_int_t;
        cpp_int int_var__hdrfld_offset2;
        void hdrfld_offset2 (const cpp_int  & l__val);
        cpp_int hdrfld_offset2() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset2_capture_t
    
class cap_dppstats_csr_sym_hdrfld_offset1_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_offset1_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_offset1_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset1_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_offset1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_offset1_cpp_int_t;
        cpp_int int_var__hdrfld_offset1;
        void hdrfld_offset1 (const cpp_int  & l__val);
        cpp_int hdrfld_offset1() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset1_capture_t
    
class cap_dppstats_csr_sym_hdrfld_offset0_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_offset0_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_offset0_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_offset0_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_offset0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_offset0_cpp_int_t;
        cpp_int int_var__hdrfld_offset0;
        void hdrfld_offset0 (const cpp_int  & l__val);
        cpp_int hdrfld_offset0() const;
    
}; // cap_dppstats_csr_sym_hdrfld_offset0_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size3_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_size3_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_size3_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size3_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_size3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_size3_cpp_int_t;
        cpp_int int_var__hdrfld_size3;
        void hdrfld_size3 (const cpp_int  & l__val);
        cpp_int hdrfld_size3() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size3_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size2_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_size2_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_size2_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size2_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_size2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_size2_cpp_int_t;
        cpp_int int_var__hdrfld_size2;
        void hdrfld_size2 (const cpp_int  & l__val);
        cpp_int hdrfld_size2() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size2_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size1_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_size1_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_size1_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size1_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_size1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_size1_cpp_int_t;
        cpp_int int_var__hdrfld_size1;
        void hdrfld_size1 (const cpp_int  & l__val);
        cpp_int hdrfld_size1() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size1_capture_t
    
class cap_dppstats_csr_sym_hdrfld_size0_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_size0_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_size0_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_size0_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_size0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > hdrfld_size0_cpp_int_t;
        cpp_int int_var__hdrfld_size0;
        void hdrfld_size0 (const cpp_int  & l__val);
        cpp_int hdrfld_size0() const;
    
}; // cap_dppstats_csr_sym_hdrfld_size0_capture_t
    
class cap_dppstats_csr_sym_hdrfld_vld_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_hdrfld_vld_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_hdrfld_vld_capture_t(string name = "cap_dppstats_csr_sym_hdrfld_vld_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_hdrfld_vld_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 256 > hdrfld_vld_cpp_int_t;
        cpp_int int_var__hdrfld_vld;
        void hdrfld_vld (const cpp_int  & l__val);
        cpp_int hdrfld_vld() const;
    
}; // cap_dppstats_csr_sym_hdrfld_vld_capture_t
    
class cap_dppstats_csr_sym_ohi_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_ohi_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_ohi_capture_t(string name = "cap_dppstats_csr_sym_ohi_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_ohi_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > ohi_cpp_int_t;
        cpp_int int_var__ohi;
        void ohi (const cpp_int  & l__val);
        cpp_int ohi() const;
    
}; // cap_dppstats_csr_sym_ohi_capture_t
    
class cap_dppstats_csr_sym_phv5_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_phv5_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_phv5_capture_t(string name = "cap_dppstats_csr_sym_phv5_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_phv5_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > phv5_cpp_int_t;
        cpp_int int_var__phv5;
        void phv5 (const cpp_int  & l__val);
        cpp_int phv5() const;
    
}; // cap_dppstats_csr_sym_phv5_capture_t
    
class cap_dppstats_csr_sym_phv4_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_phv4_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_phv4_capture_t(string name = "cap_dppstats_csr_sym_phv4_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_phv4_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > phv4_cpp_int_t;
        cpp_int int_var__phv4;
        void phv4 (const cpp_int  & l__val);
        cpp_int phv4() const;
    
}; // cap_dppstats_csr_sym_phv4_capture_t
    
class cap_dppstats_csr_sym_phv3_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_phv3_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_phv3_capture_t(string name = "cap_dppstats_csr_sym_phv3_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_phv3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > phv3_cpp_int_t;
        cpp_int int_var__phv3;
        void phv3 (const cpp_int  & l__val);
        cpp_int phv3() const;
    
}; // cap_dppstats_csr_sym_phv3_capture_t
    
class cap_dppstats_csr_sym_phv2_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_phv2_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_phv2_capture_t(string name = "cap_dppstats_csr_sym_phv2_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_phv2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > phv2_cpp_int_t;
        cpp_int int_var__phv2;
        void phv2 (const cpp_int  & l__val);
        cpp_int phv2() const;
    
}; // cap_dppstats_csr_sym_phv2_capture_t
    
class cap_dppstats_csr_sym_phv1_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_phv1_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_phv1_capture_t(string name = "cap_dppstats_csr_sym_phv1_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_phv1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > phv1_cpp_int_t;
        cpp_int int_var__phv1;
        void phv1 (const cpp_int  & l__val);
        cpp_int phv1() const;
    
}; // cap_dppstats_csr_sym_phv1_capture_t
    
class cap_dppstats_csr_sym_phv0_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_sym_phv0_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_sym_phv0_capture_t(string name = "cap_dppstats_csr_sym_phv0_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_sym_phv0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1024 > phv0_cpp_int_t;
        cpp_int int_var__phv0;
        void phv0 (const cpp_int  & l__val);
        cpp_int phv0() const;
    
}; // cap_dppstats_csr_sym_phv0_capture_t
    
class cap_dppstats_csr_cfg_spare_stats_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_cfg_spare_stats_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_cfg_spare_stats_t(string name = "cap_dppstats_csr_cfg_spare_stats_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_cfg_spare_stats_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_dppstats_csr_cfg_spare_stats_t
    
class cap_dppstats_csr_cfg_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_cfg_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_cfg_capture_t(string name = "cap_dppstats_csr_cfg_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_cfg_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > capture_en_cpp_int_t;
        cpp_int int_var__capture_en;
        void capture_en (const cpp_int  & l__val);
        cpp_int capture_en() const;
    
        typedef pu_cpp_int< 1 > capture_arm_cpp_int_t;
        cpp_int int_var__capture_arm;
        void capture_arm (const cpp_int  & l__val);
        cpp_int capture_arm() const;
    
        typedef pu_cpp_int< 30 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dppstats_csr_cfg_capture_t
    
class cap_dppstats_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppstats_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppstats_csr_t(string name = "cap_dppstats_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppstats_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dppstats_csr_cfg_capture_t cfg_capture;
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppstats_csr_cfg_spare_stats_t, 64> cfg_spare_stats;
        #else 
        cap_dppstats_csr_cfg_spare_stats_t cfg_spare_stats[64];
        #endif
        int get_depth_cfg_spare_stats() { return 64; }
    
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
    
        cap_dppstats_csr_sta_global_t sta_global;
    
        cap_dppstats_csr_sta_spare_t sta_spare;
    
        cap_dppstats_csr_CNT_dpp_t CNT_dpp;
    
        cap_dppstats_csr_CNT_dpp_phv_lines_t CNT_dpp_phv_lines;
    
        cap_dppstats_csr_CNT_dpp_ohi_lines_t CNT_dpp_ohi_lines;
    
        cap_dppstats_csr_CNT_dpp_hdrfld_sel_lines_t CNT_dpp_hdrfld_sel_lines;
    
        cap_dppstats_csr_CNT_dpp_phv_t CNT_dpp_phv;
    
        cap_dppstats_csr_CNT_dpp_phv_drop_t CNT_dpp_phv_drop;
    
        cap_dppstats_csr_CNT_dpp_phv_no_data_t CNT_dpp_phv_no_data;
    
        cap_dppstats_csr_CNT_dpp_phv_no_data_drop_t CNT_dpp_phv_no_data_drop;
    
        cap_dppstats_csr_CNT_dpp_phv_no_data_drop_drop_t CNT_dpp_phv_no_data_drop_drop;
    
        cap_dppstats_csr_CNT_dpp_ohi_t CNT_dpp_ohi;
    
        cap_dppstats_csr_CNT_dpp_dpr_xn_t CNT_dpp_dpr_xn;
    
        cap_dppstats_csr_CNT_dpp_dpr_csum_crc_xn_t CNT_dpp_dpr_csum_crc_xn;
    
        cap_dppstats_csr_CNT_dpp_spare_0_t CNT_dpp_spare_0;
    
        cap_dppstats_csr_CNT_dpp_spare_1_t CNT_dpp_spare_1;
    
        cap_dppstats_csr_CNT_dpp_spare_2_t CNT_dpp_spare_2;
    
        cap_dppstats_csr_CNT_dpp_spare_3_t CNT_dpp_spare_3;
    
        cap_dppstats_csr_CNT_dpp_spare_4_t CNT_dpp_spare_4;
    
        cap_dppstats_csr_CNT_dpp_spare_5_t CNT_dpp_spare_5;
    
        cap_dppstats_csr_CNT_dpp_spare_6_t CNT_dpp_spare_6;
    
        cap_dppstats_csr_CNT_dpp_spare_7_t CNT_dpp_spare_7;
    
        cap_dppstats_csr_CNT_dpp_spare_8_t CNT_dpp_spare_8;
    
        cap_dppstats_csr_CNT_dpp_spare_9_t CNT_dpp_spare_9;
    
        cap_dppstats_csr_CNT_dpp_spare_10_t CNT_dpp_spare_10;
    
        cap_dppstats_csr_CNT_dpp_spare_11_t CNT_dpp_spare_11;
    
        cap_dppstats_csr_CNT_dpp_spare_12_t CNT_dpp_spare_12;
    
        cap_dppstats_csr_CNT_dpp_spare_13_t CNT_dpp_spare_13;
    
        cap_dppstats_csr_CNT_dpp_spare_14_t CNT_dpp_spare_14;
    
        cap_dppstats_csr_CNT_dpp_spare_15_t CNT_dpp_spare_15;
    
        cap_dppstats_csr_SAT_dpp_err_t SAT_dpp_err;
    
        cap_dppstats_csr_SAT_dpp_ff_err_t SAT_dpp_ff_err;
    
        cap_dppstats_csr_SAT_dpp_spare_err_t SAT_dpp_spare_err;
    
}; // cap_dppstats_csr_t
    
#endif // CAP_DPPSTATS_CSR_H
        