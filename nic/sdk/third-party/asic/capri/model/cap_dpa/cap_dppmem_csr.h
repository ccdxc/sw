
#ifndef CAP_DPPMEM_CSR_H
#define CAP_DPPMEM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t(string name = "cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t
    
class cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t(string name = "cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t();
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
    
        #if 210 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t, 210> entry;
        #else 
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t entry[210];
        #endif
        int get_depth_entry() { return 210; }
    
}; // cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t
    
class cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t(string name = "cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 20 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t
    
class cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t(string name = "cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t();
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
    
        #if 640 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t, 640> entry;
        #else 
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t entry[640];
        #endif
        int get_depth_entry() { return 640; }
    
}; // cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t
    
class cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t(string name = "cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 8 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 8 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t
    
class cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t(string name = "cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 8 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 8 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t
    
class cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t(string name = "cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 36 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t
    
class cap_dppmem_csr_cfg_dpp_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_cfg_dpp_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_cfg_dpp_ohi_fifo_t(string name = "cap_dppmem_csr_cfg_dpp_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_cfg_dpp_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dppmem_csr_cfg_dpp_ohi_fifo_t
    
class cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t(string name = "cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 10 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 10 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t
    
class cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t(string name = "cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t();
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
    
        typedef pu_cpp_int< 10 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 10 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t
    
class cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t(string name = "cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t();
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
    
        typedef pu_cpp_int< 20 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t
    
class cap_dppmem_csr_cfg_dpp_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_cfg_dpp_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_cfg_dpp_phv_fifo_t(string name = "cap_dppmem_csr_cfg_dpp_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_cfg_dpp_phv_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dppmem_csr_cfg_dpp_phv_fifo_t
    
class cap_dppmem_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppmem_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppmem_csr_t(string name = "cap_dppmem_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppmem_csr_t();
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
    
        cap_dppmem_csr_cfg_dpp_phv_fifo_t cfg_dpp_phv_fifo;
    
        cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t sta_srams_ecc_dpp_phv_fifo;
    
        cap_dppmem_csr_sta_ff_ptr_dpp_phv_fifo_t sta_ff_ptr_dpp_phv_fifo;
    
        cap_dppmem_csr_cfw_ff_dpp_phv_fifo_t cfw_ff_dpp_phv_fifo;
    
        cap_dppmem_csr_cfg_dpp_ohi_fifo_t cfg_dpp_ohi_fifo;
    
        cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t sta_srams_ecc_dpp_ohi_fifo;
    
        cap_dppmem_csr_sta_ff_ptr_dpp_ohi_fifo_t sta_ff_ptr_dpp_ohi_fifo;
    
        cap_dppmem_csr_cfw_ff_dpp_ohi_fifo_t cfw_ff_dpp_ohi_fifo;
    
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t dhs_dpp_phv_fifo_sram;
    
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t dhs_dpp_ohi_fifo_sram;
    
}; // cap_dppmem_csr_t
    
#endif // CAP_DPPMEM_CSR_H
        