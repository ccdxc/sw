
#ifndef CAP_AP_CSR_H
#define CAP_AP_CSR_H

#include "cap_csr_base.h" 
#include "cap_apb_csr.h" 

using namespace std;
class cap_ap_csr_ap_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_ap_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_ap_cfg_t(string name = "cap_ap_csr_ap_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_ap_cfg_t();
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
    
        typedef pu_cpp_int< 1 > debug_port_enable_cpp_int_t;
        cpp_int int_var__debug_port_enable;
        void debug_port_enable (const cpp_int  & l__val);
        cpp_int debug_port_enable() const;
    
        typedef pu_cpp_int< 1 > debug_port_select_cpp_int_t;
        cpp_int int_var__debug_port_select;
        void debug_port_select (const cpp_int  & l__val);
        cpp_int debug_port_select() const;
    
        typedef pu_cpp_int< 1 > cfg_stall_on_same_id_cpp_int_t;
        cpp_int int_var__cfg_stall_on_same_id;
        void cfg_stall_on_same_id (const cpp_int  & l__val);
        cpp_int cfg_stall_on_same_id() const;
    
}; // cap_ap_csr_ap_cfg_t
    
class cap_ap_csr_cfg_ACP_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_ACP_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_ACP_t(string name = "cap_ap_csr_cfg_ACP_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_ACP_t();
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
    
        typedef pu_cpp_int< 1 > AINACTS_cpp_int_t;
        cpp_int int_var__AINACTS;
        void AINACTS (const cpp_int  & l__val);
        cpp_int AINACTS() const;
    
        typedef pu_cpp_int< 1 > ACINACTM_cpp_int_t;
        cpp_int int_var__ACINACTM;
        void ACINACTM (const cpp_int  & l__val);
        cpp_int ACINACTM() const;
    
}; // cap_ap_csr_cfg_ACP_t
    
class cap_ap_csr_cfg_nts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_nts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_nts_t(string name = "cap_ap_csr_cfg_nts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_nts_t();
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
    
        typedef pu_cpp_int< 7 > TSBITS_TS_cpp_int_t;
        cpp_int int_var__TSBITS_TS;
        void TSBITS_TS (const cpp_int  & l__val);
        cpp_int TSBITS_TS() const;
    
        typedef pu_cpp_int< 2 > TSSYNCS_TS_cpp_int_t;
        cpp_int int_var__TSSYNCS_TS;
        void TSSYNCS_TS (const cpp_int  & l__val);
        cpp_int TSSYNCS_TS() const;
    
}; // cap_ap_csr_cfg_nts_t
    
class cap_ap_csr_sta_nts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_nts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_nts_t(string name = "cap_ap_csr_sta_nts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_nts_t();
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
    
        typedef pu_cpp_int< 1 > TSSYNCREADYS_TS_cpp_int_t;
        cpp_int int_var__TSSYNCREADYS_TS;
        void TSSYNCREADYS_TS (const cpp_int  & l__val);
        cpp_int TSSYNCREADYS_TS() const;
    
}; // cap_ap_csr_sta_nts_t
    
class cap_ap_csr_sta_dstream_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_dstream_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_dstream_t(string name = "cap_ap_csr_sta_dstream_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_dstream_t();
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
    
        typedef pu_cpp_int< 4 > DBGACK_cpp_int_t;
        cpp_int int_var__DBGACK;
        void DBGACK (const cpp_int  & l__val);
        cpp_int DBGACK() const;
    
        typedef pu_cpp_int< 4 > DBGRSTREQ_cpp_int_t;
        cpp_int int_var__DBGRSTREQ;
        void DBGRSTREQ (const cpp_int  & l__val);
        cpp_int DBGRSTREQ() const;
    
}; // cap_ap_csr_sta_dstream_t
    
class cap_ap_csr_cfg_dstream_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_dstream_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_dstream_t(string name = "cap_ap_csr_cfg_dstream_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_dstream_t();
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
    
        typedef pu_cpp_int< 4 > EDBGRQ_cpp_int_t;
        cpp_int int_var__EDBGRQ;
        void EDBGRQ (const cpp_int  & l__val);
        cpp_int EDBGRQ() const;
    
}; // cap_ap_csr_cfg_dstream_t
    
class cap_ap_csr_cfg_misc_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_misc_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_misc_dbg_t(string name = "cap_ap_csr_cfg_misc_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_misc_dbg_t();
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
    
        typedef pu_cpp_int< 1 > DBGROMADDRV_cpp_int_t;
        cpp_int int_var__DBGROMADDRV;
        void DBGROMADDRV (const cpp_int  & l__val);
        cpp_int DBGROMADDRV() const;
    
        typedef pu_cpp_int< 32 > DBGROMADDR_cpp_int_t;
        cpp_int int_var__DBGROMADDR;
        void DBGROMADDR (const cpp_int  & l__val);
        cpp_int DBGROMADDR() const;
    
}; // cap_ap_csr_cfg_misc_dbg_t
    
class cap_ap_csr_sta_misc_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_misc_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_misc_dbg_t(string name = "cap_ap_csr_sta_misc_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_misc_dbg_t();
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
    
        typedef pu_cpp_int< 4 > COMMRX_cpp_int_t;
        cpp_int int_var__COMMRX;
        void COMMRX (const cpp_int  & l__val);
        cpp_int COMMRX() const;
    
        typedef pu_cpp_int< 4 > COMMTX_cpp_int_t;
        cpp_int int_var__COMMTX;
        void COMMTX (const cpp_int  & l__val);
        cpp_int COMMTX() const;
    
        typedef pu_cpp_int< 4 > nCOMMIRQ_cpp_int_t;
        cpp_int int_var__nCOMMIRQ;
        void nCOMMIRQ (const cpp_int  & l__val);
        cpp_int nCOMMIRQ() const;
    
}; // cap_ap_csr_sta_misc_dbg_t
    
class cap_ap_csr_sta_VSIG_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_VSIG_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_VSIG_t(string name = "cap_ap_csr_sta_VSIG_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_VSIG_t();
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
    
        typedef pu_cpp_int< 1 > CLREXMONACK_cpp_int_t;
        cpp_int int_var__CLREXMONACK;
        void CLREXMONACK (const cpp_int  & l__val);
        cpp_int CLREXMONACK() const;
    
}; // cap_ap_csr_sta_VSIG_t
    
class cap_ap_csr_cfg_VSIG_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_VSIG_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_VSIG_t(string name = "cap_ap_csr_cfg_VSIG_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_VSIG_t();
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
    
        typedef pu_cpp_int< 1 > CLREXMONREQ_cpp_int_t;
        cpp_int int_var__CLREXMONREQ;
        void CLREXMONREQ (const cpp_int  & l__val);
        cpp_int CLREXMONREQ() const;
    
}; // cap_ap_csr_cfg_VSIG_t
    
class cap_ap_csr_cfg_AUTH_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_AUTH_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_AUTH_t(string name = "cap_ap_csr_cfg_AUTH_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_AUTH_t();
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
    
        typedef pu_cpp_int< 4 > DBGEN_cpp_int_t;
        cpp_int int_var__DBGEN;
        void DBGEN (const cpp_int  & l__val);
        cpp_int DBGEN() const;
    
        typedef pu_cpp_int< 4 > SPNIDEN_cpp_int_t;
        cpp_int int_var__SPNIDEN;
        void SPNIDEN (const cpp_int  & l__val);
        cpp_int SPNIDEN() const;
    
        typedef pu_cpp_int< 4 > SPIDEN_cpp_int_t;
        cpp_int int_var__SPIDEN;
        void SPIDEN (const cpp_int  & l__val);
        cpp_int SPIDEN() const;
    
        typedef pu_cpp_int< 4 > NIDEN_cpp_int_t;
        cpp_int int_var__NIDEN;
        void NIDEN (const cpp_int  & l__val);
        cpp_int NIDEN() const;
    
}; // cap_ap_csr_cfg_AUTH_t
    
class cap_ap_csr_sta_CTI_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_CTI_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_CTI_t(string name = "cap_ap_csr_sta_CTI_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_CTI_t();
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
    
        typedef pu_cpp_int< 4 > CTICHOUT_cpp_int_t;
        cpp_int int_var__CTICHOUT;
        void CTICHOUT (const cpp_int  & l__val);
        cpp_int CTICHOUT() const;
    
        typedef pu_cpp_int< 4 > CTICHINACK_cpp_int_t;
        cpp_int int_var__CTICHINACK;
        void CTICHINACK (const cpp_int  & l__val);
        cpp_int CTICHINACK() const;
    
        typedef pu_cpp_int< 4 > CTIIRQ_cpp_int_t;
        cpp_int int_var__CTIIRQ;
        void CTIIRQ (const cpp_int  & l__val);
        cpp_int CTIIRQ() const;
    
}; // cap_ap_csr_sta_CTI_t
    
class cap_ap_csr_cfg_CTI_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_CTI_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_CTI_t(string name = "cap_ap_csr_cfg_CTI_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_CTI_t();
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
    
        typedef pu_cpp_int< 4 > CTICHIN_cpp_int_t;
        cpp_int int_var__CTICHIN;
        void CTICHIN (const cpp_int  & l__val);
        cpp_int CTICHIN() const;
    
        typedef pu_cpp_int< 4 > CTICHOUTACK_cpp_int_t;
        cpp_int int_var__CTICHOUTACK;
        void CTICHOUTACK (const cpp_int  & l__val);
        cpp_int CTICHOUTACK() const;
    
        typedef pu_cpp_int< 4 > CTIIRQACK_cpp_int_t;
        cpp_int int_var__CTIIRQACK;
        void CTIIRQACK (const cpp_int  & l__val);
        cpp_int CTIIRQACK() const;
    
}; // cap_ap_csr_cfg_CTI_t
    
class cap_ap_csr_sta_misc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_misc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_misc_t(string name = "cap_ap_csr_sta_misc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_misc_t();
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
    
        typedef pu_cpp_int< 4 > WARMRSTREQ_cpp_int_t;
        cpp_int int_var__WARMRSTREQ;
        void WARMRSTREQ (const cpp_int  & l__val);
        cpp_int WARMRSTREQ() const;
    
        typedef pu_cpp_int< 8 > WRMEMATTRM_cpp_int_t;
        cpp_int int_var__WRMEMATTRM;
        void WRMEMATTRM (const cpp_int  & l__val);
        cpp_int WRMEMATTRM() const;
    
        typedef pu_cpp_int< 8 > RDMEMATTRM_cpp_int_t;
        cpp_int int_var__RDMEMATTRM;
        void RDMEMATTRM (const cpp_int  & l__val);
        cpp_int RDMEMATTRM() const;
    
        typedef pu_cpp_int< 4 > SMPEN_cpp_int_t;
        cpp_int int_var__SMPEN;
        void SMPEN (const cpp_int  & l__val);
        cpp_int SMPEN() const;
    
        typedef pu_cpp_int< 4 > STANDBYWFE_cpp_int_t;
        cpp_int int_var__STANDBYWFE;
        void STANDBYWFE (const cpp_int  & l__val);
        cpp_int STANDBYWFE() const;
    
        typedef pu_cpp_int< 4 > STANDBYWFI_cpp_int_t;
        cpp_int int_var__STANDBYWFI;
        void STANDBYWFI (const cpp_int  & l__val);
        cpp_int STANDBYWFI() const;
    
        typedef pu_cpp_int< 1 > STANDBYWFIL2_cpp_int_t;
        cpp_int int_var__STANDBYWFIL2;
        void STANDBYWFIL2 (const cpp_int  & l__val);
        cpp_int STANDBYWFIL2() const;
    
}; // cap_ap_csr_sta_misc_t
    
class cap_ap_csr_sta_IRQ_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_IRQ_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_IRQ_t(string name = "cap_ap_csr_sta_IRQ_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_IRQ_t();
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
    
        typedef pu_cpp_int< 1 > nEXTERRIRQ_cpp_int_t;
        cpp_int int_var__nEXTERRIRQ;
        void nEXTERRIRQ (const cpp_int  & l__val);
        cpp_int nEXTERRIRQ() const;
    
        typedef pu_cpp_int< 1 > nINTERRIRQ_cpp_int_t;
        cpp_int int_var__nINTERRIRQ;
        void nINTERRIRQ (const cpp_int  & l__val);
        cpp_int nINTERRIRQ() const;
    
        typedef pu_cpp_int< 4 > nVCPUMNTIRQ_cpp_int_t;
        cpp_int int_var__nVCPUMNTIRQ;
        void nVCPUMNTIRQ (const cpp_int  & l__val);
        cpp_int nVCPUMNTIRQ() const;
    
}; // cap_ap_csr_sta_IRQ_t
    
class cap_ap_csr_sta_L2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_L2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_L2_t(string name = "cap_ap_csr_sta_L2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_L2_t();
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
    
        typedef pu_cpp_int< 1 > L2FLUSHDONE_cpp_int_t;
        cpp_int int_var__L2FLUSHDONE;
        void L2FLUSHDONE (const cpp_int  & l__val);
        cpp_int L2FLUSHDONE() const;
    
}; // cap_ap_csr_sta_L2_t
    
class cap_ap_csr_cfg_L2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_L2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_L2_t(string name = "cap_ap_csr_cfg_L2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_L2_t();
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
    
        typedef pu_cpp_int< 1 > L2FLUSHREQ_cpp_int_t;
        cpp_int int_var__L2FLUSHREQ;
        void L2FLUSHREQ (const cpp_int  & l__val);
        cpp_int L2FLUSHREQ() const;
    
}; // cap_ap_csr_cfg_L2_t
    
class cap_ap_csr_sta_EV_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_EV_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_EV_t(string name = "cap_ap_csr_sta_EV_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_EV_t();
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
    
        typedef pu_cpp_int< 1 > EVENTO_cpp_int_t;
        cpp_int int_var__EVENTO;
        void EVENTO (const cpp_int  & l__val);
        cpp_int EVENTO() const;
    
}; // cap_ap_csr_sta_EV_t
    
class cap_ap_csr_cfg_EV_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_EV_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_EV_t(string name = "cap_ap_csr_cfg_EV_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_EV_t();
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
    
        typedef pu_cpp_int< 1 > EVENTI_cpp_int_t;
        cpp_int int_var__EVENTI;
        void EVENTI (const cpp_int  & l__val);
        cpp_int EVENTI() const;
    
}; // cap_ap_csr_cfg_EV_t
    
class cap_ap_csr_cfg_pmu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_pmu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_pmu_t(string name = "cap_ap_csr_cfg_pmu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_pmu_t();
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
    
        typedef pu_cpp_int< 4 > PMUSNAPSHOTREQ_cpp_int_t;
        cpp_int int_var__PMUSNAPSHOTREQ;
        void PMUSNAPSHOTREQ (const cpp_int  & l__val);
        cpp_int PMUSNAPSHOTREQ() const;
    
}; // cap_ap_csr_cfg_pmu_t
    
class cap_ap_csr_sta_pmu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_pmu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_pmu_t(string name = "cap_ap_csr_sta_pmu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_pmu_t();
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
    
        typedef pu_cpp_int< 25 > PMUEVENT0_cpp_int_t;
        cpp_int int_var__PMUEVENT0;
        void PMUEVENT0 (const cpp_int  & l__val);
        cpp_int PMUEVENT0() const;
    
        typedef pu_cpp_int< 25 > PMUEVENT1_cpp_int_t;
        cpp_int int_var__PMUEVENT1;
        void PMUEVENT1 (const cpp_int  & l__val);
        cpp_int PMUEVENT1() const;
    
        typedef pu_cpp_int< 25 > PMUEVENT2_cpp_int_t;
        cpp_int int_var__PMUEVENT2;
        void PMUEVENT2 (const cpp_int  & l__val);
        cpp_int PMUEVENT2() const;
    
        typedef pu_cpp_int< 25 > PMUEVENT3_cpp_int_t;
        cpp_int int_var__PMUEVENT3;
        void PMUEVENT3 (const cpp_int  & l__val);
        cpp_int PMUEVENT3() const;
    
        typedef pu_cpp_int< 4 > nPMUIRQ_cpp_int_t;
        cpp_int int_var__nPMUIRQ;
        void nPMUIRQ (const cpp_int  & l__val);
        cpp_int nPMUIRQ() const;
    
        typedef pu_cpp_int< 4 > PMUSNAPSHOTACK_cpp_int_t;
        cpp_int int_var__PMUSNAPSHOTACK;
        void PMUSNAPSHOTACK (const cpp_int  & l__val);
        cpp_int PMUSNAPSHOTACK() const;
    
}; // cap_ap_csr_sta_pmu_t
    
class cap_ap_csr_sta_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_timer_t(string name = "cap_ap_csr_sta_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_timer_t();
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
    
        typedef pu_cpp_int< 4 > nCNTHPIRQ_cpp_int_t;
        cpp_int int_var__nCNTHPIRQ;
        void nCNTHPIRQ (const cpp_int  & l__val);
        cpp_int nCNTHPIRQ() const;
    
        typedef pu_cpp_int< 4 > nCNTPNSIRQ_cpp_int_t;
        cpp_int int_var__nCNTPNSIRQ;
        void nCNTPNSIRQ (const cpp_int  & l__val);
        cpp_int nCNTPNSIRQ() const;
    
        typedef pu_cpp_int< 4 > nCNTPSIRQ_cpp_int_t;
        cpp_int int_var__nCNTPSIRQ;
        void nCNTPSIRQ (const cpp_int  & l__val);
        cpp_int nCNTPSIRQ() const;
    
        typedef pu_cpp_int< 4 > nCNTVIRQ_cpp_int_t;
        cpp_int int_var__nCNTVIRQ;
        void nCNTVIRQ (const cpp_int  & l__val);
        cpp_int nCNTVIRQ() const;
    
}; // cap_ap_csr_sta_timer_t
    
class cap_ap_csr_cfg_AT_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_AT_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_AT_t(string name = "cap_ap_csr_cfg_AT_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_AT_t();
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
    
        typedef pu_cpp_int< 1 > AFVALIDM0_cpp_int_t;
        cpp_int int_var__AFVALIDM0;
        void AFVALIDM0 (const cpp_int  & l__val);
        cpp_int AFVALIDM0() const;
    
        typedef pu_cpp_int< 1 > AFVALIDM1_cpp_int_t;
        cpp_int int_var__AFVALIDM1;
        void AFVALIDM1 (const cpp_int  & l__val);
        cpp_int AFVALIDM1() const;
    
        typedef pu_cpp_int< 1 > AFVALIDM2_cpp_int_t;
        cpp_int int_var__AFVALIDM2;
        void AFVALIDM2 (const cpp_int  & l__val);
        cpp_int AFVALIDM2() const;
    
        typedef pu_cpp_int< 1 > AFVALIDM3_cpp_int_t;
        cpp_int int_var__AFVALIDM3;
        void AFVALIDM3 (const cpp_int  & l__val);
        cpp_int AFVALIDM3() const;
    
        typedef pu_cpp_int< 1 > ATREADYM0_cpp_int_t;
        cpp_int int_var__ATREADYM0;
        void ATREADYM0 (const cpp_int  & l__val);
        cpp_int ATREADYM0() const;
    
        typedef pu_cpp_int< 1 > ATREADYM1_cpp_int_t;
        cpp_int int_var__ATREADYM1;
        void ATREADYM1 (const cpp_int  & l__val);
        cpp_int ATREADYM1() const;
    
        typedef pu_cpp_int< 1 > ATREADYM2_cpp_int_t;
        cpp_int int_var__ATREADYM2;
        void ATREADYM2 (const cpp_int  & l__val);
        cpp_int ATREADYM2() const;
    
        typedef pu_cpp_int< 1 > ATREADYM3_cpp_int_t;
        cpp_int int_var__ATREADYM3;
        void ATREADYM3 (const cpp_int  & l__val);
        cpp_int ATREADYM3() const;
    
        typedef pu_cpp_int< 1 > SYNCREQM0_cpp_int_t;
        cpp_int int_var__SYNCREQM0;
        void SYNCREQM0 (const cpp_int  & l__val);
        cpp_int SYNCREQM0() const;
    
        typedef pu_cpp_int< 1 > SYNCREQM1_cpp_int_t;
        cpp_int int_var__SYNCREQM1;
        void SYNCREQM1 (const cpp_int  & l__val);
        cpp_int SYNCREQM1() const;
    
        typedef pu_cpp_int< 1 > SYNCREQM2_cpp_int_t;
        cpp_int int_var__SYNCREQM2;
        void SYNCREQM2 (const cpp_int  & l__val);
        cpp_int SYNCREQM2() const;
    
        typedef pu_cpp_int< 1 > SYNCREQM3_cpp_int_t;
        cpp_int int_var__SYNCREQM3;
        void SYNCREQM3 (const cpp_int  & l__val);
        cpp_int SYNCREQM3() const;
    
}; // cap_ap_csr_cfg_AT_t
    
class cap_ap_csr_sta_AT_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_AT_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_AT_t(string name = "cap_ap_csr_sta_AT_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_AT_t();
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
    
        typedef pu_cpp_int< 1 > AFREADYM0_cpp_int_t;
        cpp_int int_var__AFREADYM0;
        void AFREADYM0 (const cpp_int  & l__val);
        cpp_int AFREADYM0() const;
    
        typedef pu_cpp_int< 1 > AFREADYM1_cpp_int_t;
        cpp_int int_var__AFREADYM1;
        void AFREADYM1 (const cpp_int  & l__val);
        cpp_int AFREADYM1() const;
    
        typedef pu_cpp_int< 1 > AFREADYM2_cpp_int_t;
        cpp_int int_var__AFREADYM2;
        void AFREADYM2 (const cpp_int  & l__val);
        cpp_int AFREADYM2() const;
    
        typedef pu_cpp_int< 1 > AFREADYM3_cpp_int_t;
        cpp_int int_var__AFREADYM3;
        void AFREADYM3 (const cpp_int  & l__val);
        cpp_int AFREADYM3() const;
    
        typedef pu_cpp_int< 2 > ATBYTESM0_cpp_int_t;
        cpp_int int_var__ATBYTESM0;
        void ATBYTESM0 (const cpp_int  & l__val);
        cpp_int ATBYTESM0() const;
    
        typedef pu_cpp_int< 2 > ATBYTESM1_cpp_int_t;
        cpp_int int_var__ATBYTESM1;
        void ATBYTESM1 (const cpp_int  & l__val);
        cpp_int ATBYTESM1() const;
    
        typedef pu_cpp_int< 2 > ATBYTESM2_cpp_int_t;
        cpp_int int_var__ATBYTESM2;
        void ATBYTESM2 (const cpp_int  & l__val);
        cpp_int ATBYTESM2() const;
    
        typedef pu_cpp_int< 2 > ATBYTESM3_cpp_int_t;
        cpp_int int_var__ATBYTESM3;
        void ATBYTESM3 (const cpp_int  & l__val);
        cpp_int ATBYTESM3() const;
    
        typedef pu_cpp_int< 32 > ATDATAM0_cpp_int_t;
        cpp_int int_var__ATDATAM0;
        void ATDATAM0 (const cpp_int  & l__val);
        cpp_int ATDATAM0() const;
    
        typedef pu_cpp_int< 32 > ATDATAM1_cpp_int_t;
        cpp_int int_var__ATDATAM1;
        void ATDATAM1 (const cpp_int  & l__val);
        cpp_int ATDATAM1() const;
    
        typedef pu_cpp_int< 32 > ATDATAM2_cpp_int_t;
        cpp_int int_var__ATDATAM2;
        void ATDATAM2 (const cpp_int  & l__val);
        cpp_int ATDATAM2() const;
    
        typedef pu_cpp_int< 32 > ATDATAM3_cpp_int_t;
        cpp_int int_var__ATDATAM3;
        void ATDATAM3 (const cpp_int  & l__val);
        cpp_int ATDATAM3() const;
    
        typedef pu_cpp_int< 7 > ATIDM0_cpp_int_t;
        cpp_int int_var__ATIDM0;
        void ATIDM0 (const cpp_int  & l__val);
        cpp_int ATIDM0() const;
    
        typedef pu_cpp_int< 7 > ATIDM1_cpp_int_t;
        cpp_int int_var__ATIDM1;
        void ATIDM1 (const cpp_int  & l__val);
        cpp_int ATIDM1() const;
    
        typedef pu_cpp_int< 7 > ATIDM2_cpp_int_t;
        cpp_int int_var__ATIDM2;
        void ATIDM2 (const cpp_int  & l__val);
        cpp_int ATIDM2() const;
    
        typedef pu_cpp_int< 7 > ATIDM3_cpp_int_t;
        cpp_int int_var__ATIDM3;
        void ATIDM3 (const cpp_int  & l__val);
        cpp_int ATIDM3() const;
    
        typedef pu_cpp_int< 1 > ATVALIDM0_cpp_int_t;
        cpp_int int_var__ATVALIDM0;
        void ATVALIDM0 (const cpp_int  & l__val);
        cpp_int ATVALIDM0() const;
    
        typedef pu_cpp_int< 1 > ATVALIDM1_cpp_int_t;
        cpp_int int_var__ATVALIDM1;
        void ATVALIDM1 (const cpp_int  & l__val);
        cpp_int ATVALIDM1() const;
    
        typedef pu_cpp_int< 1 > ATVALIDM2_cpp_int_t;
        cpp_int int_var__ATVALIDM2;
        void ATVALIDM2 (const cpp_int  & l__val);
        cpp_int ATVALIDM2() const;
    
        typedef pu_cpp_int< 1 > ATVALIDM3_cpp_int_t;
        cpp_int int_var__ATVALIDM3;
        void ATVALIDM3 (const cpp_int  & l__val);
        cpp_int ATVALIDM3() const;
    
}; // cap_ap_csr_sta_AT_t
    
class cap_ap_csr_sta_CORE_BIST_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_CORE_BIST_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_CORE_BIST_t(string name = "cap_ap_csr_sta_CORE_BIST_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_CORE_BIST_t();
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
    
        typedef pu_cpp_int< 444 > DONE_FAIL_OUT_cpp_int_t;
        cpp_int int_var__DONE_FAIL_OUT;
        void DONE_FAIL_OUT (const cpp_int  & l__val);
        cpp_int DONE_FAIL_OUT() const;
    
        typedef pu_cpp_int< 444 > DONE_PASS_OUT_cpp_int_t;
        cpp_int int_var__DONE_PASS_OUT;
        void DONE_PASS_OUT (const cpp_int  & l__val);
        cpp_int DONE_PASS_OUT() const;
    
}; // cap_ap_csr_sta_CORE_BIST_t
    
class cap_ap_csr_cfg_CORE_BIST_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_CORE_BIST_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_CORE_BIST_t(string name = "cap_ap_csr_cfg_CORE_BIST_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_CORE_BIST_t();
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
    
        typedef pu_cpp_int< 1 > PTRN_FILL_cpp_int_t;
        cpp_int int_var__PTRN_FILL;
        void PTRN_FILL (const cpp_int  & l__val);
        cpp_int PTRN_FILL() const;
    
        typedef pu_cpp_int< 1 > RPR_MODE_cpp_int_t;
        cpp_int int_var__RPR_MODE;
        void RPR_MODE (const cpp_int  & l__val);
        cpp_int RPR_MODE() const;
    
        typedef pu_cpp_int< 1 > RUN_cpp_int_t;
        cpp_int int_var__RUN;
        void RUN (const cpp_int  & l__val);
        cpp_int RUN() const;
    
        typedef pu_cpp_int< 1 > SEL_CLK_cpp_int_t;
        cpp_int int_var__SEL_CLK;
        void SEL_CLK (const cpp_int  & l__val);
        cpp_int SEL_CLK() const;
    
        typedef pu_cpp_int< 1 > SEL_L2DCLK_cpp_int_t;
        cpp_int int_var__SEL_L2DCLK;
        void SEL_L2DCLK (const cpp_int  & l__val);
        cpp_int SEL_L2DCLK() const;
    
        typedef pu_cpp_int< 1 > SEL_L2TCLK_cpp_int_t;
        cpp_int int_var__SEL_L2TCLK;
        void SEL_L2TCLK (const cpp_int  & l__val);
        cpp_int SEL_L2TCLK() const;
    
        typedef pu_cpp_int< 1 > MEM_RST_cpp_int_t;
        cpp_int int_var__MEM_RST;
        void MEM_RST (const cpp_int  & l__val);
        cpp_int MEM_RST() const;
    
        typedef pu_cpp_int< 1 > RESET_OVERRIDE_cpp_int_t;
        cpp_int int_var__RESET_OVERRIDE;
        void RESET_OVERRIDE (const cpp_int  & l__val);
        cpp_int RESET_OVERRIDE() const;
    
}; // cap_ap_csr_cfg_CORE_BIST_t
    
class cap_ap_csr_sta_apms_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_apms_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_apms_t(string name = "cap_ap_csr_sta_apms_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_apms_t();
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
    
        typedef pu_cpp_int< 2 > fatal_cpp_int_t;
        cpp_int int_var__fatal;
        void fatal (const cpp_int  & l__val);
        cpp_int fatal() const;
    
}; // cap_ap_csr_sta_apms_t
    
class cap_ap_csr_sta_gic_mbist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_sta_gic_mbist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_sta_gic_mbist_t(string name = "cap_ap_csr_sta_gic_mbist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_sta_gic_mbist_t();
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
    
        typedef pu_cpp_int< 1 > mbistack_cpp_int_t;
        cpp_int int_var__mbistack;
        void mbistack (const cpp_int  & l__val);
        cpp_int mbistack() const;
    
        typedef pu_cpp_int< 64 > mbistoutdata_cpp_int_t;
        cpp_int int_var__mbistoutdata;
        void mbistoutdata (const cpp_int  & l__val);
        cpp_int mbistoutdata() const;
    
}; // cap_ap_csr_sta_gic_mbist_t
    
class cap_ap_csr_cfg_gic_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_gic_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_gic_t(string name = "cap_ap_csr_cfg_gic_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_gic_t();
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
    
        typedef pu_cpp_int< 3 > base_awuser_cpp_int_t;
        cpp_int int_var__base_awuser;
        void base_awuser (const cpp_int  & l__val);
        cpp_int base_awuser() const;
    
        typedef pu_cpp_int< 3 > base_aruser_cpp_int_t;
        cpp_int int_var__base_aruser;
        void base_aruser (const cpp_int  & l__val);
        cpp_int base_aruser() const;
    
        typedef pu_cpp_int< 17 > awuser_cpp_int_t;
        cpp_int int_var__awuser;
        void awuser (const cpp_int  & l__val);
        cpp_int awuser() const;
    
}; // cap_ap_csr_cfg_gic_t
    
class cap_ap_csr_cfg_gic_mbist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_gic_mbist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_gic_mbist_t(string name = "cap_ap_csr_cfg_gic_mbist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_gic_mbist_t();
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
    
        typedef pu_cpp_int< 12 > mbistaddr_cpp_int_t;
        cpp_int int_var__mbistaddr;
        void mbistaddr (const cpp_int  & l__val);
        cpp_int mbistaddr() const;
    
        typedef pu_cpp_int< 2 > mbistarray_cpp_int_t;
        cpp_int int_var__mbistarray;
        void mbistarray (const cpp_int  & l__val);
        cpp_int mbistarray() const;
    
        typedef pu_cpp_int< 1 > mbistcfg_cpp_int_t;
        cpp_int int_var__mbistcfg;
        void mbistcfg (const cpp_int  & l__val);
        cpp_int mbistcfg() const;
    
        typedef pu_cpp_int< 1 > mbistreaden_cpp_int_t;
        cpp_int int_var__mbistreaden;
        void mbistreaden (const cpp_int  & l__val);
        cpp_int mbistreaden() const;
    
        typedef pu_cpp_int< 1 > mbistreq_cpp_int_t;
        cpp_int int_var__mbistreq;
        void mbistreq (const cpp_int  & l__val);
        cpp_int mbistreq() const;
    
        typedef pu_cpp_int< 1 > mbistresetn_cpp_int_t;
        cpp_int int_var__mbistresetn;
        void mbistresetn (const cpp_int  & l__val);
        cpp_int mbistresetn() const;
    
        typedef pu_cpp_int< 1 > mbistwriteen_cpp_int_t;
        cpp_int int_var__mbistwriteen;
        void mbistwriteen (const cpp_int  & l__val);
        cpp_int mbistwriteen() const;
    
        typedef pu_cpp_int< 64 > mbistindata_cpp_int_t;
        cpp_int int_var__mbistindata;
        void mbistindata (const cpp_int  & l__val);
        cpp_int mbistindata() const;
    
}; // cap_ap_csr_cfg_gic_mbist_t
    
class cap_ap_csr_cfg_irq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_irq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_irq_t(string name = "cap_ap_csr_cfg_irq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_irq_t();
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
    
        typedef pu_cpp_int< 4 > nFIQ_mask_cpp_int_t;
        cpp_int int_var__nFIQ_mask;
        void nFIQ_mask (const cpp_int  & l__val);
        cpp_int nFIQ_mask() const;
    
        typedef pu_cpp_int< 4 > nIRQ_mask_cpp_int_t;
        cpp_int int_var__nIRQ_mask;
        void nIRQ_mask (const cpp_int  & l__val);
        cpp_int nIRQ_mask() const;
    
        typedef pu_cpp_int< 4 > nREI_mask_cpp_int_t;
        cpp_int int_var__nREI_mask;
        void nREI_mask (const cpp_int  & l__val);
        cpp_int nREI_mask() const;
    
        typedef pu_cpp_int< 4 > nSEI_mask_cpp_int_t;
        cpp_int int_var__nSEI_mask;
        void nSEI_mask (const cpp_int  & l__val);
        cpp_int nSEI_mask() const;
    
        typedef pu_cpp_int< 4 > nVSEI_mask_cpp_int_t;
        cpp_int int_var__nVSEI_mask;
        void nVSEI_mask (const cpp_int  & l__val);
        cpp_int nVSEI_mask() const;
    
}; // cap_ap_csr_cfg_irq_t
    
class cap_ap_csr_cfg_apar_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_apar_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_apar_t(string name = "cap_ap_csr_cfg_apar_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_apar_t();
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
    
        typedef pu_cpp_int< 24 > addr_top24_cpp_int_t;
        cpp_int int_var__addr_top24;
        void addr_top24 (const cpp_int  & l__val);
        cpp_int addr_top24() const;
    
        typedef pu_cpp_int< 2 > awusers_cpp_int_t;
        cpp_int int_var__awusers;
        void awusers (const cpp_int  & l__val);
        cpp_int awusers() const;
    
        typedef pu_cpp_int< 2 > arusers_cpp_int_t;
        cpp_int int_var__arusers;
        void arusers (const cpp_int  & l__val);
        cpp_int arusers() const;
    
        typedef pu_cpp_int< 1 > addr_sel_cpp_int_t;
        cpp_int int_var__addr_sel;
        void addr_sel (const cpp_int  & l__val);
        cpp_int addr_sel() const;
    
}; // cap_ap_csr_cfg_apar_t
    
class cap_ap_csr_cfg_cpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_cpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_cpu_t(string name = "cap_ap_csr_cfg_cpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_cpu_t();
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
    
        typedef pu_cpp_int< 4 > active_cpp_int_t;
        cpp_int int_var__active;
        void active (const cpp_int  & l__val);
        cpp_int active() const;
    
        typedef pu_cpp_int< 4 > sta_wake_request_cpp_int_t;
        cpp_int int_var__sta_wake_request;
        void sta_wake_request (const cpp_int  & l__val);
        cpp_int sta_wake_request() const;
    
}; // cap_ap_csr_cfg_cpu_t
    
class cap_ap_csr_cfg_cpu3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_cpu3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_cpu3_t(string name = "cap_ap_csr_cfg_cpu3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_cpu3_t();
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
    
        typedef pu_cpp_int< 44 > RVBARADDR_cpp_int_t;
        cpp_int int_var__RVBARADDR;
        void RVBARADDR (const cpp_int  & l__val);
        cpp_int RVBARADDR() const;
    
}; // cap_ap_csr_cfg_cpu3_t
    
class cap_ap_csr_cfg_cpu2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_cpu2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_cpu2_t(string name = "cap_ap_csr_cfg_cpu2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_cpu2_t();
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
    
        typedef pu_cpp_int< 44 > RVBARADDR_cpp_int_t;
        cpp_int int_var__RVBARADDR;
        void RVBARADDR (const cpp_int  & l__val);
        cpp_int RVBARADDR() const;
    
}; // cap_ap_csr_cfg_cpu2_t
    
class cap_ap_csr_cfg_cpu1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_cpu1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_cpu1_t(string name = "cap_ap_csr_cfg_cpu1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_cpu1_t();
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
    
        typedef pu_cpp_int< 44 > RVBARADDR_cpp_int_t;
        cpp_int int_var__RVBARADDR;
        void RVBARADDR (const cpp_int  & l__val);
        cpp_int RVBARADDR() const;
    
}; // cap_ap_csr_cfg_cpu1_t
    
class cap_ap_csr_cfg_cpu0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_cpu0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_cpu0_t(string name = "cap_ap_csr_cfg_cpu0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_cpu0_t();
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
    
        typedef pu_cpp_int< 44 > RVBARADDR_cpp_int_t;
        cpp_int int_var__RVBARADDR;
        void RVBARADDR (const cpp_int  & l__val);
        cpp_int RVBARADDR() const;
    
}; // cap_ap_csr_cfg_cpu0_t
    
class cap_ap_csr_cfg_cpu0_flash_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_cpu0_flash_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_cpu0_flash_t(string name = "cap_ap_csr_cfg_cpu0_flash_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_cpu0_flash_t();
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
    
        typedef pu_cpp_int< 44 > RVBARADDR_cpp_int_t;
        cpp_int int_var__RVBARADDR;
        void RVBARADDR (const cpp_int  & l__val);
        cpp_int RVBARADDR() const;
    
}; // cap_ap_csr_cfg_cpu0_flash_t
    
class cap_ap_csr_cfg_static_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_cfg_static_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_cfg_static_t(string name = "cap_ap_csr_cfg_static_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_cfg_static_t();
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
    
        typedef pu_cpp_int< 1 > DBGL1RSTDISABLE_cpp_int_t;
        cpp_int int_var__DBGL1RSTDISABLE;
        void DBGL1RSTDISABLE (const cpp_int  & l__val);
        cpp_int DBGL1RSTDISABLE() const;
    
        typedef pu_cpp_int< 1 > L2RSTDISABLE_cpp_int_t;
        cpp_int int_var__L2RSTDISABLE;
        void L2RSTDISABLE (const cpp_int  & l__val);
        cpp_int L2RSTDISABLE() const;
    
        typedef pu_cpp_int< 4 > CP15SDISABLE_cpp_int_t;
        cpp_int int_var__CP15SDISABLE;
        void CP15SDISABLE (const cpp_int  & l__val);
        cpp_int CP15SDISABLE() const;
    
        typedef pu_cpp_int< 8 > CLUSTERIDAFF1_cpp_int_t;
        cpp_int int_var__CLUSTERIDAFF1;
        void CLUSTERIDAFF1 (const cpp_int  & l__val);
        cpp_int CLUSTERIDAFF1() const;
    
        typedef pu_cpp_int< 8 > CLUSTERIDAFF2_cpp_int_t;
        cpp_int int_var__CLUSTERIDAFF2;
        void CLUSTERIDAFF2 (const cpp_int  & l__val);
        cpp_int CLUSTERIDAFF2() const;
    
        typedef pu_cpp_int< 4 > CRYPTODISABLE_cpp_int_t;
        cpp_int int_var__CRYPTODISABLE;
        void CRYPTODISABLE (const cpp_int  & l__val);
        cpp_int CRYPTODISABLE() const;
    
        typedef pu_cpp_int< 4 > AA64nAA32_cpp_int_t;
        cpp_int int_var__AA64nAA32;
        void AA64nAA32 (const cpp_int  & l__val);
        cpp_int AA64nAA32() const;
    
        typedef pu_cpp_int< 1 > SYSBARDISABLE_cpp_int_t;
        cpp_int int_var__SYSBARDISABLE;
        void SYSBARDISABLE (const cpp_int  & l__val);
        cpp_int SYSBARDISABLE() const;
    
        typedef pu_cpp_int< 1 > BROADCASTCACHEMAINT_cpp_int_t;
        cpp_int int_var__BROADCASTCACHEMAINT;
        void BROADCASTCACHEMAINT (const cpp_int  & l__val);
        cpp_int BROADCASTCACHEMAINT() const;
    
        typedef pu_cpp_int< 1 > BROADCASTINNER_cpp_int_t;
        cpp_int int_var__BROADCASTINNER;
        void BROADCASTINNER (const cpp_int  & l__val);
        cpp_int BROADCASTINNER() const;
    
        typedef pu_cpp_int< 1 > BROADCASTOUTER_cpp_int_t;
        cpp_int int_var__BROADCASTOUTER;
        void BROADCASTOUTER (const cpp_int  & l__val);
        cpp_int BROADCASTOUTER() const;
    
        typedef pu_cpp_int< 4 > CFGEND_cpp_int_t;
        cpp_int int_var__CFGEND;
        void CFGEND (const cpp_int  & l__val);
        cpp_int CFGEND() const;
    
        typedef pu_cpp_int< 4 > CFGTE_cpp_int_t;
        cpp_int int_var__CFGTE;
        void CFGTE (const cpp_int  & l__val);
        cpp_int CFGTE() const;
    
        typedef pu_cpp_int< 26 > PERIPHBASE_cpp_int_t;
        cpp_int int_var__PERIPHBASE;
        void PERIPHBASE (const cpp_int  & l__val);
        cpp_int PERIPHBASE() const;
    
}; // cap_ap_csr_cfg_static_t
    
class cap_ap_csr_dummy_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_dummy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_dummy_t(string name = "cap_ap_csr_dummy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_dummy_t();
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
    
        typedef pu_cpp_int< 64 > cycle_counter_cpp_int_t;
        cpp_int int_var__cycle_counter;
        void cycle_counter (const cpp_int  & l__val);
        cpp_int cycle_counter() const;
    
}; // cap_ap_csr_dummy_t
    
class cap_ap_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_base_t(string name = "cap_ap_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_base_t();
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
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        cpp_int int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_ap_csr_base_t
    
class cap_ap_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ap_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ap_csr_t(string name = "cap_ap_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ap_csr_t();
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
    
        cap_ap_csr_base_t base;
    
        cap_ap_csr_dummy_t dummy;
    
        cap_ap_csr_cfg_static_t cfg_static;
    
        cap_ap_csr_cfg_cpu0_flash_t cfg_cpu0_flash;
    
        cap_ap_csr_cfg_cpu0_t cfg_cpu0;
    
        cap_ap_csr_cfg_cpu1_t cfg_cpu1;
    
        cap_ap_csr_cfg_cpu2_t cfg_cpu2;
    
        cap_ap_csr_cfg_cpu3_t cfg_cpu3;
    
        cap_ap_csr_cfg_cpu_t cfg_cpu;
    
        cap_ap_csr_cfg_apar_t cfg_apar;
    
        cap_ap_csr_cfg_irq_t cfg_irq;
    
        cap_ap_csr_cfg_gic_mbist_t cfg_gic_mbist;
    
        cap_ap_csr_cfg_gic_t cfg_gic;
    
        cap_ap_csr_sta_gic_mbist_t sta_gic_mbist;
    
        cap_ap_csr_sta_apms_t sta_apms;
    
        cap_ap_csr_cfg_CORE_BIST_t cfg_CORE_BIST;
    
        cap_ap_csr_sta_CORE_BIST_t sta_CORE_BIST;
    
        cap_ap_csr_sta_AT_t sta_AT;
    
        cap_ap_csr_cfg_AT_t cfg_AT;
    
        cap_ap_csr_sta_timer_t sta_timer;
    
        cap_ap_csr_sta_pmu_t sta_pmu;
    
        cap_ap_csr_cfg_pmu_t cfg_pmu;
    
        cap_ap_csr_cfg_EV_t cfg_EV;
    
        cap_ap_csr_sta_EV_t sta_EV;
    
        cap_ap_csr_cfg_L2_t cfg_L2;
    
        cap_ap_csr_sta_L2_t sta_L2;
    
        cap_ap_csr_sta_IRQ_t sta_IRQ;
    
        cap_ap_csr_sta_misc_t sta_misc;
    
        cap_ap_csr_cfg_CTI_t cfg_CTI;
    
        cap_ap_csr_sta_CTI_t sta_CTI;
    
        cap_ap_csr_cfg_AUTH_t cfg_AUTH;
    
        cap_ap_csr_cfg_VSIG_t cfg_VSIG;
    
        cap_ap_csr_sta_VSIG_t sta_VSIG;
    
        cap_ap_csr_sta_misc_dbg_t sta_misc_dbg;
    
        cap_ap_csr_cfg_misc_dbg_t cfg_misc_dbg;
    
        cap_ap_csr_cfg_dstream_t cfg_dstream;
    
        cap_ap_csr_sta_dstream_t sta_dstream;
    
        cap_ap_csr_sta_nts_t sta_nts;
    
        cap_ap_csr_cfg_nts_t cfg_nts;
    
        cap_ap_csr_cfg_ACP_t cfg_ACP;
    
        cap_ap_csr_ap_cfg_t ap_cfg;
    
        cap_apb_csr_t apb;
    
}; // cap_ap_csr_t
    
#endif // CAP_AP_CSR_H
        