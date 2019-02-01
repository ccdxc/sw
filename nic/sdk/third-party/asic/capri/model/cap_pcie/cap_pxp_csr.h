
#ifndef CAP_PXP_CSR_H
#define CAP_PXP_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pxp_csr_int_p_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_int_p_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_int_p_ecc_int_enable_clear_t(string name = "cap_pxp_csr_int_p_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_int_p_ecc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > rxbfr_overflow_enable_cpp_int_t;
        cpp_int int_var__rxbfr_overflow_enable;
        void rxbfr_overflow_enable (const cpp_int  & l__val);
        cpp_int rxbfr_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > replay_bfr_overflow_enable_cpp_int_t;
        cpp_int int_var__replay_bfr_overflow_enable;
        void replay_bfr_overflow_enable (const cpp_int  & l__val);
        cpp_int replay_bfr_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > rxtlp_err_enable_cpp_int_t;
        cpp_int int_var__rxtlp_err_enable;
        void rxtlp_err_enable (const cpp_int  & l__val);
        cpp_int rxtlp_err_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_0_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__txbuf_0_uncorrectable_enable;
        void txbuf_0_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int txbuf_0_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_1_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__txbuf_1_uncorrectable_enable;
        void txbuf_1_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int txbuf_1_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_2_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__txbuf_2_uncorrectable_enable;
        void txbuf_2_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int txbuf_2_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_3_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__txbuf_3_uncorrectable_enable;
        void txbuf_3_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int txbuf_3_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_0_correctable_enable_cpp_int_t;
        cpp_int int_var__txbuf_0_correctable_enable;
        void txbuf_0_correctable_enable (const cpp_int  & l__val);
        cpp_int txbuf_0_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_1_correctable_enable_cpp_int_t;
        cpp_int int_var__txbuf_1_correctable_enable;
        void txbuf_1_correctable_enable (const cpp_int  & l__val);
        cpp_int txbuf_1_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_2_correctable_enable_cpp_int_t;
        cpp_int int_var__txbuf_2_correctable_enable;
        void txbuf_2_correctable_enable (const cpp_int  & l__val);
        cpp_int txbuf_2_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > txbuf_3_correctable_enable_cpp_int_t;
        cpp_int int_var__txbuf_3_correctable_enable;
        void txbuf_3_correctable_enable (const cpp_int  & l__val);
        cpp_int txbuf_3_correctable_enable() const;
    
}; // cap_pxp_csr_int_p_ecc_int_enable_clear_t
    
class cap_pxp_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_intreg_t(string name = "cap_pxp_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > rxbfr_overflow_interrupt_cpp_int_t;
        cpp_int int_var__rxbfr_overflow_interrupt;
        void rxbfr_overflow_interrupt (const cpp_int  & l__val);
        cpp_int rxbfr_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > replay_bfr_overflow_interrupt_cpp_int_t;
        cpp_int int_var__replay_bfr_overflow_interrupt;
        void replay_bfr_overflow_interrupt (const cpp_int  & l__val);
        cpp_int replay_bfr_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxtlp_err_interrupt_cpp_int_t;
        cpp_int int_var__rxtlp_err_interrupt;
        void rxtlp_err_interrupt (const cpp_int  & l__val);
        cpp_int rxtlp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_0_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_0_uncorrectable_interrupt;
        void txbuf_0_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_0_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_1_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_1_uncorrectable_interrupt;
        void txbuf_1_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_1_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_2_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_2_uncorrectable_interrupt;
        void txbuf_2_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_2_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_3_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_3_uncorrectable_interrupt;
        void txbuf_3_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_3_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_0_correctable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_0_correctable_interrupt;
        void txbuf_0_correctable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_0_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_1_correctable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_1_correctable_interrupt;
        void txbuf_1_correctable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_1_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_2_correctable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_2_correctable_interrupt;
        void txbuf_2_correctable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_2_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > txbuf_3_correctable_interrupt_cpp_int_t;
        cpp_int int_var__txbuf_3_correctable_interrupt;
        void txbuf_3_correctable_interrupt (const cpp_int  & l__val);
        cpp_int txbuf_3_correctable_interrupt() const;
    
}; // cap_pxp_csr_intreg_t
    
class cap_pxp_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_intgrp_t(string name = "cap_pxp_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_intgrp_t();
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
    
        cap_pxp_csr_intreg_t intreg;
    
        cap_pxp_csr_intreg_t int_test_set;
    
        cap_pxp_csr_int_p_ecc_int_enable_clear_t int_enable_set;
    
        cap_pxp_csr_int_p_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_pxp_csr_intgrp_t
    
class cap_pxp_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_intreg_status_t(string name = "cap_pxp_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_p_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_p_ecc_interrupt;
        void int_p_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_p_ecc_interrupt() const;
    
}; // cap_pxp_csr_intreg_status_t
    
class cap_pxp_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pxp_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_p_ecc_enable_cpp_int_t;
        cpp_int int_var__int_p_ecc_enable;
        void int_p_ecc_enable (const cpp_int  & l__val);
        cpp_int int_p_ecc_enable() const;
    
}; // cap_pxp_csr_int_groups_int_enable_rw_reg_t
    
class cap_pxp_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_intgrp_status_t(string name = "cap_pxp_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_intgrp_status_t();
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
    
        cap_pxp_csr_intreg_status_t intreg;
    
        cap_pxp_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pxp_csr_intreg_status_t int_rw_reg;
    
}; // cap_pxp_csr_intgrp_status_t
    
class cap_pxp_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_csr_intr_t(string name = "cap_pxp_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_csr_intr_t();
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
    
        typedef pu_cpp_int< 1 > dowstream_cpp_int_t;
        cpp_int int_var__dowstream;
        void dowstream (const cpp_int  & l__val);
        cpp_int dowstream() const;
    
        typedef pu_cpp_int< 1 > dowstream_enable_cpp_int_t;
        cpp_int int_var__dowstream_enable;
        void dowstream_enable (const cpp_int  & l__val);
        cpp_int dowstream_enable() const;
    
}; // cap_pxp_csr_csr_intr_t
    
class cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t(string name = "cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t
    
class cap_pxp_csr_sat_p_port_cnt_phystatus_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_phystatus_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_phystatus_err_t(string name = "cap_pxp_csr_sat_p_port_cnt_phystatus_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_phystatus_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_phystatus_err_t
    
class cap_pxp_csr_sat_p_port_cnt_deskew_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_deskew_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_deskew_err_t(string name = "cap_pxp_csr_sat_p_port_cnt_deskew_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_deskew_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_deskew_err_t
    
class cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t(string name = "cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t
    
class cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t(string name = "cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t
    
class cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t(string name = "cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t
    
class cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t(string name = "cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t
    
class cap_pxp_csr_sat_p_port_cnt_replay_num_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_replay_num_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_replay_num_err_t(string name = "cap_pxp_csr_sat_p_port_cnt_replay_num_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_replay_num_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_replay_num_err_t
    
class cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t(string name = "cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t
    
class cap_pxp_csr_sat_p_port_cnt_fc_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_fc_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_fc_timeout_t(string name = "cap_pxp_csr_sat_p_port_cnt_fc_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_fc_timeout_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_fc_timeout_t
    
class cap_pxp_csr_sat_p_port_cnt_fcpe_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_fcpe_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_fcpe_t(string name = "cap_pxp_csr_sat_p_port_cnt_fcpe_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_fcpe_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_fcpe_t
    
class cap_pxp_csr_sat_p_port_cnt_rx_nullified_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_rx_nullified_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_rx_nullified_t(string name = "cap_pxp_csr_sat_p_port_cnt_rx_nullified_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_rx_nullified_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_rx_nullified_t
    
class cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t(string name = "cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t
    
class cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t(string name = "cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t
    
class cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t(string name = "cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t
    
class cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t(string name = "cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t();
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
    
        typedef pu_cpp_int< 8 > event_cpp_int_t;
        cpp_int int_var__event;
        void event (const cpp_int  & l__val);
        cpp_int event() const;
    
}; // cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t
    
class cap_pxp_csr_sta_p_ecc_txbuf_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_ecc_txbuf_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_ecc_txbuf_3_t(string name = "cap_pxp_csr_sta_p_ecc_txbuf_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_ecc_txbuf_3_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxp_csr_sta_p_ecc_txbuf_3_t
    
class cap_pxp_csr_sta_p_ecc_txbuf_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_ecc_txbuf_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_ecc_txbuf_2_t(string name = "cap_pxp_csr_sta_p_ecc_txbuf_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_ecc_txbuf_2_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxp_csr_sta_p_ecc_txbuf_2_t
    
class cap_pxp_csr_sta_p_ecc_txbuf_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_ecc_txbuf_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_ecc_txbuf_1_t(string name = "cap_pxp_csr_sta_p_ecc_txbuf_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_ecc_txbuf_1_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxp_csr_sta_p_ecc_txbuf_1_t
    
class cap_pxp_csr_sta_p_ecc_txbuf_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_ecc_txbuf_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_ecc_txbuf_0_t(string name = "cap_pxp_csr_sta_p_ecc_txbuf_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_ecc_txbuf_0_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxp_csr_sta_p_ecc_txbuf_0_t
    
class cap_pxp_csr_sta_p_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_sram_bist_t(string name = "cap_pxp_csr_sta_p_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_sram_bist_t();
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
    
        typedef pu_cpp_int< 8 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
        typedef pu_cpp_int< 8 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
}; // cap_pxp_csr_sta_p_sram_bist_t
    
class cap_pxp_csr_cfg_p_mac_test_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_cfg_p_mac_test_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_cfg_p_mac_test_in_t(string name = "cap_pxp_csr_cfg_p_mac_test_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_cfg_p_mac_test_in_t();
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
    
        typedef pu_cpp_int< 2 > bit_23_22_cpp_int_t;
        cpp_int int_var__bit_23_22;
        void bit_23_22 (const cpp_int  & l__val);
        cpp_int bit_23_22() const;
    
}; // cap_pxp_csr_cfg_p_mac_test_in_t
    
class cap_pxp_csr_cfg_p_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_cfg_p_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_cfg_p_sram_bist_t(string name = "cap_pxp_csr_cfg_p_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_cfg_p_sram_bist_t();
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
    
        typedef pu_cpp_int< 8 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_pxp_csr_cfg_p_sram_bist_t
    
class cap_pxp_csr_cfg_p_ecc_disable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_cfg_p_ecc_disable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_cfg_p_ecc_disable_t(string name = "cap_pxp_csr_cfg_p_ecc_disable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_cfg_p_ecc_disable_t();
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
    
        typedef pu_cpp_int< 1 > txbuf_cor_cpp_int_t;
        cpp_int int_var__txbuf_cor;
        void txbuf_cor (const cpp_int  & l__val);
        cpp_int txbuf_cor() const;
    
        typedef pu_cpp_int< 1 > txbuf_det_cpp_int_t;
        cpp_int int_var__txbuf_det;
        void txbuf_det (const cpp_int  & l__val);
        cpp_int txbuf_det() const;
    
}; // cap_pxp_csr_cfg_p_ecc_disable_t
    
class cap_pxp_csr_sta_p_port_lanes_7_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_port_lanes_7_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_port_lanes_7_0_t(string name = "cap_pxp_csr_sta_p_port_lanes_7_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_port_lanes_7_0_t();
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
    
        typedef pu_cpp_int< 8 > detected_cpp_int_t;
        cpp_int int_var__detected;
        void detected (const cpp_int  & l__val);
        cpp_int detected() const;
    
        typedef pu_cpp_int< 8 > active_cpp_int_t;
        cpp_int int_var__active;
        void active (const cpp_int  & l__val);
        cpp_int active() const;
    
}; // cap_pxp_csr_sta_p_port_lanes_7_0_t
    
class cap_pxp_csr_sta_p_port_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_sta_p_port_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_sta_p_port_mac_t(string name = "cap_pxp_csr_sta_p_port_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_sta_p_port_mac_t();
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
    
        typedef pu_cpp_int< 5 > pl_ltssm_cpp_int_t;
        cpp_int int_var__pl_ltssm;
        void pl_ltssm (const cpp_int  & l__val);
        cpp_int pl_ltssm() const;
    
        typedef pu_cpp_int< 2 > pl_equ_phase_cpp_int_t;
        cpp_int int_var__pl_equ_phase;
        void pl_equ_phase (const cpp_int  & l__val);
        cpp_int pl_equ_phase() const;
    
        typedef pu_cpp_int< 2 > rate_cpp_int_t;
        cpp_int int_var__rate;
        void rate (const cpp_int  & l__val);
        cpp_int rate() const;
    
        typedef pu_cpp_int< 5 > rxbfr_fill_level_cpp_int_t;
        cpp_int int_var__rxbfr_fill_level;
        void rxbfr_fill_level (const cpp_int  & l__val);
        cpp_int rxbfr_fill_level() const;
    
        typedef pu_cpp_int< 2 > rxl0s_substate_cpp_int_t;
        cpp_int int_var__rxl0s_substate;
        void rxl0s_substate (const cpp_int  & l__val);
        cpp_int rxl0s_substate() const;
    
        typedef pu_cpp_int< 2 > txl0s_substate_cpp_int_t;
        cpp_int int_var__txl0s_substate;
        void txl0s_substate (const cpp_int  & l__val);
        cpp_int txl0s_substate() const;
    
        typedef pu_cpp_int< 1 > lanes_reversed_cpp_int_t;
        cpp_int int_var__lanes_reversed;
        void lanes_reversed (const cpp_int  & l__val);
        cpp_int lanes_reversed() const;
    
}; // cap_pxp_csr_sta_p_port_mac_t
    
class cap_pxp_csr_cfg_p_mac_k_eq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_cfg_p_mac_k_eq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_cfg_p_mac_k_eq_t(string name = "cap_pxp_csr_cfg_p_mac_k_eq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_cfg_p_mac_k_eq_t();
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
    
        typedef pu_cpp_int< 6 > k_finetune_max_8gt_cpp_int_t;
        cpp_int int_var__k_finetune_max_8gt;
        void k_finetune_max_8gt (const cpp_int  & l__val);
        cpp_int k_finetune_max_8gt() const;
    
        typedef pu_cpp_int< 11 > k_preset_to_use_8gt_cpp_int_t;
        cpp_int int_var__k_preset_to_use_8gt;
        void k_preset_to_use_8gt (const cpp_int  & l__val);
        cpp_int k_preset_to_use_8gt() const;
    
        typedef pu_cpp_int< 6 > k_finetune_max_16gt_cpp_int_t;
        cpp_int int_var__k_finetune_max_16gt;
        void k_finetune_max_16gt (const cpp_int  & l__val);
        cpp_int k_finetune_max_16gt() const;
    
        typedef pu_cpp_int< 11 > k_preset_to_use_16gt_cpp_int_t;
        cpp_int int_var__k_preset_to_use_16gt;
        void k_preset_to_use_16gt (const cpp_int  & l__val);
        cpp_int k_preset_to_use_16gt() const;
    
        typedef pu_cpp_int< 2 > k_finetune_err_cpp_int_t;
        cpp_int int_var__k_finetune_err;
        void k_finetune_err (const cpp_int  & l__val);
        cpp_int k_finetune_err() const;
    
        typedef pu_cpp_int< 1 > k_phyparam_query_cpp_int_t;
        cpp_int int_var__k_phyparam_query;
        void k_phyparam_query (const cpp_int  & l__val);
        cpp_int k_phyparam_query() const;
    
        typedef pu_cpp_int< 1 > k_query_timeout_cpp_int_t;
        cpp_int int_var__k_query_timeout;
        void k_query_timeout (const cpp_int  & l__val);
        cpp_int k_query_timeout() const;
    
}; // cap_pxp_csr_cfg_p_mac_k_eq_t
    
class cap_pxp_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxp_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxp_csr_t(string name = "cap_pxp_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxp_csr_t();
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
    
        cap_pxp_csr_cfg_p_mac_k_eq_t cfg_p_mac_k_eq;
    
        cap_pxp_csr_sta_p_port_mac_t sta_p_port_mac;
    
        cap_pxp_csr_sta_p_port_lanes_7_0_t sta_p_port_lanes_7_0;
    
        cap_pxp_csr_cfg_p_ecc_disable_t cfg_p_ecc_disable;
    
        cap_pxp_csr_cfg_p_sram_bist_t cfg_p_sram_bist;
    
        cap_pxp_csr_cfg_p_mac_test_in_t cfg_p_mac_test_in;
    
        cap_pxp_csr_sta_p_sram_bist_t sta_p_sram_bist;
    
        cap_pxp_csr_sta_p_ecc_txbuf_0_t sta_p_ecc_txbuf_0;
    
        cap_pxp_csr_sta_p_ecc_txbuf_1_t sta_p_ecc_txbuf_1;
    
        cap_pxp_csr_sta_p_ecc_txbuf_2_t sta_p_ecc_txbuf_2;
    
        cap_pxp_csr_sta_p_ecc_txbuf_3_t sta_p_ecc_txbuf_3;
    
        cap_pxp_csr_sat_p_port_cnt_rx_bad_tlp_t sat_p_port_cnt_rx_bad_tlp;
    
        cap_pxp_csr_sat_p_port_cnt_rx_bad_dllp_t sat_p_port_cnt_rx_bad_dllp;
    
        cap_pxp_csr_sat_p_port_cnt_rx_nak_received_t sat_p_port_cnt_rx_nak_received;
    
        cap_pxp_csr_sat_p_port_cnt_tx_nak_sent_t sat_p_port_cnt_tx_nak_sent;
    
        cap_pxp_csr_sat_p_port_cnt_rx_nullified_t sat_p_port_cnt_rx_nullified;
    
        cap_pxp_csr_sat_p_port_cnt_fcpe_t sat_p_port_cnt_fcpe;
    
        cap_pxp_csr_sat_p_port_cnt_fc_timeout_t sat_p_port_cnt_fc_timeout;
    
        cap_pxp_csr_sat_p_port_cnt_txbuf_ecc_err_t sat_p_port_cnt_txbuf_ecc_err;
    
        cap_pxp_csr_sat_p_port_cnt_replay_num_err_t sat_p_port_cnt_replay_num_err;
    
        cap_pxp_csr_sat_p_port_cnt_replay_timer_err_t sat_p_port_cnt_replay_timer_err;
    
        cap_pxp_csr_sat_p_port_cnt_core_initiated_recovery_t sat_p_port_cnt_core_initiated_recovery;
    
        cap_pxp_csr_sat_p_port_cnt_ltssm_state_changed_t sat_p_port_cnt_ltssm_state_changed;
    
        cap_pxp_csr_sat_p_port_cnt_8b10b_128b130b_skp_os_err_t sat_p_port_cnt_8b10b_128b130b_skp_os_err;
    
        cap_pxp_csr_sat_p_port_cnt_deskew_err_t sat_p_port_cnt_deskew_err;
    
        cap_pxp_csr_sat_p_port_cnt_phystatus_err_t sat_p_port_cnt_phystatus_err;
    
        cap_pxp_csr_sat_p_port_cnt_rxbfr_overflow_t sat_p_port_cnt_rxbfr_overflow;
    
        cap_pxp_csr_csr_intr_t csr_intr;
    
        cap_pxp_csr_intgrp_status_t int_groups;
    
        cap_pxp_csr_intgrp_t int_p_ecc;
    
}; // cap_pxp_csr_t
    
#endif // CAP_PXP_CSR_H
        