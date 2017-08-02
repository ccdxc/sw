
#ifndef CAP_DPPMEM_CSR_H
#define CAP_DPPMEM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t(string name = "cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t
    
class cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t(string name = "cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_entry_t entry[210];
        int get_depth_entry() { return 210; }
    
}; // cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t
    
class cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t(string name = "cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 20 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t
    
class cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t(string name = "cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_entry_t entry[640];
        int get_depth_entry() { return 640; }
    
}; // cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t
    
class cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t(string name = "cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t
    
class cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t(string name = "cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t();
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
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t
    
class cap_dppmem_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dppmem_csr_t(string name = "cap_dppmem_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dppmem_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dppmem_csr_sta_srams_ecc_dpp_phv_fifo_t sta_srams_ecc_dpp_phv_fifo;
    
        cap_dppmem_csr_sta_srams_ecc_dpp_ohi_fifo_t sta_srams_ecc_dpp_ohi_fifo;
    
        cap_dppmem_csr_dhs_dpp_phv_fifo_sram_t dhs_dpp_phv_fifo_sram;
    
        cap_dppmem_csr_dhs_dpp_ohi_fifo_sram_t dhs_dpp_ohi_fifo_sram;
    
}; // cap_dppmem_csr_t
    
#endif // CAP_DPPMEM_CSR_H
        