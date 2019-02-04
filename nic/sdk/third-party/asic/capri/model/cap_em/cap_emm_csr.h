
#ifndef CAP_EMM_CSR_H
#define CAP_EMM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_emm_csr_cfg_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_emm_csr_cfg_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_emm_csr_cfg_ecc_t(string name = "cap_emm_csr_cfg_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_emm_csr_cfg_ecc_t();
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
    
        typedef pu_cpp_int< 1 > disable_cor_SRAM1_cpp_int_t;
        cpp_int int_var__disable_cor_SRAM1;
        void disable_cor_SRAM1 (const cpp_int  & l__val);
        cpp_int disable_cor_SRAM1() const;
    
}; // cap_emm_csr_cfg_ecc_t
    
class cap_emm_csr_em_cfg_core_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_emm_csr_em_cfg_core_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_emm_csr_em_cfg_core_t(string name = "cap_emm_csr_em_cfg_core_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_emm_csr_em_cfg_core_t();
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
    
        typedef pu_cpp_int< 1 > corecfg_asyncwkupena_cpp_int_t;
        cpp_int int_var__corecfg_asyncwkupena;
        void corecfg_asyncwkupena (const cpp_int  & l__val);
        cpp_int corecfg_asyncwkupena() const;
    
        typedef pu_cpp_int< 6 > corecfg_tuningcount_cpp_int_t;
        cpp_int int_var__corecfg_tuningcount;
        void corecfg_tuningcount (const cpp_int  & l__val);
        cpp_int corecfg_tuningcount() const;
    
}; // cap_emm_csr_em_cfg_core_t
    
class cap_emm_csr_em_sta_phy_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_emm_csr_em_sta_phy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_emm_csr_em_sta_phy_t(string name = "cap_emm_csr_em_sta_phy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_emm_csr_em_sta_phy_t();
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
    
        typedef pu_cpp_int< 32 > phyctrl_biststatus_cpp_int_t;
        cpp_int int_var__phyctrl_biststatus;
        void phyctrl_biststatus (const cpp_int  & l__val);
        cpp_int phyctrl_biststatus() const;
    
        typedef pu_cpp_int< 1 > phyctrl_bistdone_cpp_int_t;
        cpp_int int_var__phyctrl_bistdone;
        void phyctrl_bistdone (const cpp_int  & l__val);
        cpp_int phyctrl_bistdone() const;
    
        typedef pu_cpp_int< 1 > phyctrl_exr_ninst_cpp_int_t;
        cpp_int int_var__phyctrl_exr_ninst;
        void phyctrl_exr_ninst (const cpp_int  & l__val);
        cpp_int phyctrl_exr_ninst() const;
    
        typedef pu_cpp_int< 1 > phyctrl_dllrdy_cpp_int_t;
        cpp_int int_var__phyctrl_dllrdy;
        void phyctrl_dllrdy (const cpp_int  & l__val);
        cpp_int phyctrl_dllrdy() const;
    
        typedef pu_cpp_int< 4 > phyctrl_rtrim_cpp_int_t;
        cpp_int int_var__phyctrl_rtrim;
        void phyctrl_rtrim (const cpp_int  & l__val);
        cpp_int phyctrl_rtrim() const;
    
        typedef pu_cpp_int< 1 > phyctrl_caldone_cpp_int_t;
        cpp_int int_var__phyctrl_caldone;
        void phyctrl_caldone (const cpp_int  & l__val);
        cpp_int phyctrl_caldone() const;
    
}; // cap_emm_csr_em_sta_phy_t
    
class cap_emm_csr_em_cfg_phy_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_emm_csr_em_cfg_phy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_emm_csr_em_cfg_phy_t(string name = "cap_emm_csr_em_cfg_phy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_emm_csr_em_cfg_phy_t();
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
    
        typedef pu_cpp_int< 1 > phyctrl_pdb_cpp_int_t;
        cpp_int int_var__phyctrl_pdb;
        void phyctrl_pdb (const cpp_int  & l__val);
        cpp_int phyctrl_pdb() const;
    
        typedef pu_cpp_int< 3 > phyctrl_dr_ty_cpp_int_t;
        cpp_int int_var__phyctrl_dr_ty;
        void phyctrl_dr_ty (const cpp_int  & l__val);
        cpp_int phyctrl_dr_ty() const;
    
        typedef pu_cpp_int< 1 > phyctrl_retrim_cpp_int_t;
        cpp_int int_var__phyctrl_retrim;
        void phyctrl_retrim (const cpp_int  & l__val);
        cpp_int phyctrl_retrim() const;
    
        typedef pu_cpp_int< 1 > phyctrl_en_rtrim_cpp_int_t;
        cpp_int int_var__phyctrl_en_rtrim;
        void phyctrl_en_rtrim (const cpp_int  & l__val);
        cpp_int phyctrl_en_rtrim() const;
    
        typedef pu_cpp_int< 4 > phyctrl_dll_trm_icp_cpp_int_t;
        cpp_int int_var__phyctrl_dll_trm_icp;
        void phyctrl_dll_trm_icp (const cpp_int  & l__val);
        cpp_int phyctrl_dll_trm_icp() const;
    
        typedef pu_cpp_int< 1 > phyctrl_endll_cpp_int_t;
        cpp_int int_var__phyctrl_endll;
        void phyctrl_endll (const cpp_int  & l__val);
        cpp_int phyctrl_endll() const;
    
        typedef pu_cpp_int< 3 > phyctrl_dlliff_cpp_int_t;
        cpp_int int_var__phyctrl_dlliff;
        void phyctrl_dlliff (const cpp_int  & l__val);
        cpp_int phyctrl_dlliff() const;
    
        typedef pu_cpp_int< 1 > phyctrl_reten_cpp_int_t;
        cpp_int int_var__phyctrl_reten;
        void phyctrl_reten (const cpp_int  & l__val);
        cpp_int phyctrl_reten() const;
    
        typedef pu_cpp_int< 1 > phyctrl_retenb_cpp_int_t;
        cpp_int int_var__phyctrl_retenb;
        void phyctrl_retenb (const cpp_int  & l__val);
        cpp_int phyctrl_retenb() const;
    
        typedef pu_cpp_int< 1 > phyctrl_oden_strb_cpp_int_t;
        cpp_int int_var__phyctrl_oden_strb;
        void phyctrl_oden_strb (const cpp_int  & l__val);
        cpp_int phyctrl_oden_strb() const;
    
        typedef pu_cpp_int< 1 > phyctrl_oden_cmd_cpp_int_t;
        cpp_int int_var__phyctrl_oden_cmd;
        void phyctrl_oden_cmd (const cpp_int  & l__val);
        cpp_int phyctrl_oden_cmd() const;
    
        typedef pu_cpp_int< 8 > phyctrl_oden_dat_cpp_int_t;
        cpp_int int_var__phyctrl_oden_dat;
        void phyctrl_oden_dat (const cpp_int  & l__val);
        cpp_int phyctrl_oden_dat() const;
    
        typedef pu_cpp_int< 1 > phyctrl_ren_strb_cpp_int_t;
        cpp_int int_var__phyctrl_ren_strb;
        void phyctrl_ren_strb (const cpp_int  & l__val);
        cpp_int phyctrl_ren_strb() const;
    
        typedef pu_cpp_int< 1 > phyctrl_ren_cmd_cpp_int_t;
        cpp_int int_var__phyctrl_ren_cmd;
        void phyctrl_ren_cmd (const cpp_int  & l__val);
        cpp_int phyctrl_ren_cmd() const;
    
        typedef pu_cpp_int< 8 > phyctrl_ren_dat_cpp_int_t;
        cpp_int int_var__phyctrl_ren_dat;
        void phyctrl_ren_dat (const cpp_int  & l__val);
        cpp_int phyctrl_ren_dat() const;
    
        typedef pu_cpp_int< 1 > phyctrl_pu_strb_cpp_int_t;
        cpp_int int_var__phyctrl_pu_strb;
        void phyctrl_pu_strb (const cpp_int  & l__val);
        cpp_int phyctrl_pu_strb() const;
    
        typedef pu_cpp_int< 1 > phyctrl_pu_cmd_cpp_int_t;
        cpp_int int_var__phyctrl_pu_cmd;
        void phyctrl_pu_cmd (const cpp_int  & l__val);
        cpp_int phyctrl_pu_cmd() const;
    
        typedef pu_cpp_int< 8 > phyctrl_pu_dat_cpp_int_t;
        cpp_int int_var__phyctrl_pu_dat;
        void phyctrl_pu_dat (const cpp_int  & l__val);
        cpp_int phyctrl_pu_dat() const;
    
        typedef pu_cpp_int< 1 > phyctrl_OD_release_strb_cpp_int_t;
        cpp_int int_var__phyctrl_OD_release_strb;
        void phyctrl_OD_release_strb (const cpp_int  & l__val);
        cpp_int phyctrl_OD_release_strb() const;
    
        typedef pu_cpp_int< 1 > phyctrl_OD_release_cmd_cpp_int_t;
        cpp_int int_var__phyctrl_OD_release_cmd;
        void phyctrl_OD_release_cmd (const cpp_int  & l__val);
        cpp_int phyctrl_OD_release_cmd() const;
    
        typedef pu_cpp_int< 8 > phyctrl_OD_release_dat_cpp_int_t;
        cpp_int int_var__phyctrl_OD_release_dat;
        void phyctrl_OD_release_dat (const cpp_int  & l__val);
        cpp_int phyctrl_OD_release_dat() const;
    
        typedef pu_cpp_int< 5 > phyctrl_itapdlysel_cpp_int_t;
        cpp_int int_var__phyctrl_itapdlysel;
        void phyctrl_itapdlysel (const cpp_int  & l__val);
        cpp_int phyctrl_itapdlysel() const;
    
        typedef pu_cpp_int< 1 > phyctrl_itapdlyena_cpp_int_t;
        cpp_int int_var__phyctrl_itapdlyena;
        void phyctrl_itapdlyena (const cpp_int  & l__val);
        cpp_int phyctrl_itapdlyena() const;
    
        typedef pu_cpp_int< 1 > phyctrl_itapchgwin_cpp_int_t;
        cpp_int int_var__phyctrl_itapchgwin;
        void phyctrl_itapchgwin (const cpp_int  & l__val);
        cpp_int phyctrl_itapchgwin() const;
    
        typedef pu_cpp_int< 4 > phyctrl_otapdlysel_cpp_int_t;
        cpp_int int_var__phyctrl_otapdlysel;
        void phyctrl_otapdlysel (const cpp_int  & l__val);
        cpp_int phyctrl_otapdlysel() const;
    
        typedef pu_cpp_int< 1 > phyctrl_otapdlyena_cpp_int_t;
        cpp_int int_var__phyctrl_otapdlyena;
        void phyctrl_otapdlyena (const cpp_int  & l__val);
        cpp_int phyctrl_otapdlyena() const;
    
        typedef pu_cpp_int< 3 > phyctrl_frqsel_cpp_int_t;
        cpp_int int_var__phyctrl_frqsel;
        void phyctrl_frqsel (const cpp_int  & l__val);
        cpp_int phyctrl_frqsel() const;
    
        typedef pu_cpp_int< 4 > phyctrl_strbsel_cpp_int_t;
        cpp_int int_var__phyctrl_strbsel;
        void phyctrl_strbsel (const cpp_int  & l__val);
        cpp_int phyctrl_strbsel() const;
    
        typedef pu_cpp_int< 1 > phyctrl_seldlyrxclk_cpp_int_t;
        cpp_int int_var__phyctrl_seldlyrxclk;
        void phyctrl_seldlyrxclk (const cpp_int  & l__val);
        cpp_int phyctrl_seldlyrxclk() const;
    
        typedef pu_cpp_int< 1 > phyctrl_seldlytxclk_cpp_int_t;
        cpp_int int_var__phyctrl_seldlytxclk;
        void phyctrl_seldlytxclk (const cpp_int  & l__val);
        cpp_int phyctrl_seldlytxclk() const;
    
        typedef pu_cpp_int< 8 > phyctrl_testctrl_cpp_int_t;
        cpp_int int_var__phyctrl_testctrl;
        void phyctrl_testctrl (const cpp_int  & l__val);
        cpp_int phyctrl_testctrl() const;
    
        typedef pu_cpp_int< 3 > phyctrl_clkbufsel_cpp_int_t;
        cpp_int int_var__phyctrl_clkbufsel;
        void phyctrl_clkbufsel (const cpp_int  & l__val);
        cpp_int phyctrl_clkbufsel() const;
    
        typedef pu_cpp_int< 1 > phyctrl_bistenable_cpp_int_t;
        cpp_int int_var__phyctrl_bistenable;
        void phyctrl_bistenable (const cpp_int  & l__val);
        cpp_int phyctrl_bistenable() const;
    
        typedef pu_cpp_int< 4 > phyctrl_bistmode_cpp_int_t;
        cpp_int int_var__phyctrl_bistmode;
        void phyctrl_bistmode (const cpp_int  & l__val);
        cpp_int phyctrl_bistmode() const;
    
        typedef pu_cpp_int< 1 > phyctrl_biststart_cpp_int_t;
        cpp_int int_var__phyctrl_biststart;
        void phyctrl_biststart (const cpp_int  & l__val);
        cpp_int phyctrl_biststart() const;
    
}; // cap_emm_csr_em_cfg_phy_t
    
class cap_emm_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_emm_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_emm_csr_t(string name = "cap_emm_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_emm_csr_t();
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
    
        cap_emm_csr_em_cfg_phy_t em_cfg_phy;
    
        cap_emm_csr_em_sta_phy_t em_sta_phy;
    
        cap_emm_csr_em_cfg_core_t em_cfg_core;
    
        cap_emm_csr_cfg_ecc_t cfg_ecc;
    
}; // cap_emm_csr_t
    
#endif // CAP_EMM_CSR_H
        