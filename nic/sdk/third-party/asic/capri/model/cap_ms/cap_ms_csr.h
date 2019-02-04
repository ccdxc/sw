
#ifndef CAP_MS_CSR_H
#define CAP_MS_CSR_H

#include "cap_csr_base.h" 
#include "cap_msh_csr.h" 
#include "cap_msr_csr.h" 

using namespace std;
class cap_ms_csr_int_gic23_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_gic23_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_gic23_int_enable_clear_t(string name = "cap_ms_csr_int_gic23_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_gic23_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > msar_int_lvl_enable_cpp_int_t;
        cpp_int int_var__msar_int_lvl_enable;
        void msar_int_lvl_enable (const cpp_int  & l__val);
        cpp_int msar_int_lvl_enable() const;
    
}; // cap_ms_csr_int_gic23_int_enable_clear_t
    
class cap_ms_csr_int_gic23_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_gic23_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_gic23_int_test_set_t(string name = "cap_ms_csr_int_gic23_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_gic23_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > msar_int_lvl_interrupt_cpp_int_t;
        cpp_int int_var__msar_int_lvl_interrupt;
        void msar_int_lvl_interrupt (const cpp_int  & l__val);
        cpp_int msar_int_lvl_interrupt() const;
    
}; // cap_ms_csr_int_gic23_int_test_set_t
    
class cap_ms_csr_int_gic23_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_gic23_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_gic23_t(string name = "cap_ms_csr_int_gic23_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_gic23_t();
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
    
        cap_ms_csr_int_gic23_int_test_set_t intreg;
    
        cap_ms_csr_int_gic23_int_test_set_t int_test_set;
    
        cap_ms_csr_int_gic23_int_enable_clear_t int_enable_set;
    
        cap_ms_csr_int_gic23_int_enable_clear_t int_enable_clear;
    
}; // cap_ms_csr_int_gic23_t
    
class cap_ms_csr_int_prp5_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_prp5_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_prp5_int_enable_clear_t(string name = "cap_ms_csr_int_prp5_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_prp5_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > read_enable_cpp_int_t;
        cpp_int int_var__read_enable;
        void read_enable (const cpp_int  & l__val);
        cpp_int read_enable() const;
    
        typedef pu_cpp_int< 1 > security_enable_cpp_int_t;
        cpp_int int_var__security_enable;
        void security_enable (const cpp_int  & l__val);
        cpp_int security_enable() const;
    
        typedef pu_cpp_int< 1 > decode_enable_cpp_int_t;
        cpp_int int_var__decode_enable;
        void decode_enable (const cpp_int  & l__val);
        cpp_int decode_enable() const;
    
        typedef pu_cpp_int< 1 > ack_timeout_enable_cpp_int_t;
        cpp_int int_var__ack_timeout_enable;
        void ack_timeout_enable (const cpp_int  & l__val);
        cpp_int ack_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > stall_timeout_enable_cpp_int_t;
        cpp_int int_var__stall_timeout_enable;
        void stall_timeout_enable (const cpp_int  & l__val);
        cpp_int stall_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > byte_read_enable_cpp_int_t;
        cpp_int int_var__byte_read_enable;
        void byte_read_enable (const cpp_int  & l__val);
        cpp_int byte_read_enable() const;
    
        typedef pu_cpp_int< 1 > byte_write_enable_cpp_int_t;
        cpp_int int_var__byte_write_enable;
        void byte_write_enable (const cpp_int  & l__val);
        cpp_int byte_write_enable() const;
    
}; // cap_ms_csr_int_prp5_int_enable_clear_t
    
class cap_ms_csr_int_prp5_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_prp5_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_prp5_int_test_set_t(string name = "cap_ms_csr_int_prp5_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_prp5_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > read_interrupt_cpp_int_t;
        cpp_int int_var__read_interrupt;
        void read_interrupt (const cpp_int  & l__val);
        cpp_int read_interrupt() const;
    
        typedef pu_cpp_int< 1 > security_interrupt_cpp_int_t;
        cpp_int int_var__security_interrupt;
        void security_interrupt (const cpp_int  & l__val);
        cpp_int security_interrupt() const;
    
        typedef pu_cpp_int< 1 > decode_interrupt_cpp_int_t;
        cpp_int int_var__decode_interrupt;
        void decode_interrupt (const cpp_int  & l__val);
        cpp_int decode_interrupt() const;
    
        typedef pu_cpp_int< 1 > ack_timeout_interrupt_cpp_int_t;
        cpp_int int_var__ack_timeout_interrupt;
        void ack_timeout_interrupt (const cpp_int  & l__val);
        cpp_int ack_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > stall_timeout_interrupt_cpp_int_t;
        cpp_int int_var__stall_timeout_interrupt;
        void stall_timeout_interrupt (const cpp_int  & l__val);
        cpp_int stall_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > byte_read_interrupt_cpp_int_t;
        cpp_int int_var__byte_read_interrupt;
        void byte_read_interrupt (const cpp_int  & l__val);
        cpp_int byte_read_interrupt() const;
    
        typedef pu_cpp_int< 1 > byte_write_interrupt_cpp_int_t;
        cpp_int int_var__byte_write_interrupt;
        void byte_write_interrupt (const cpp_int  & l__val);
        cpp_int byte_write_interrupt() const;
    
}; // cap_ms_csr_int_prp5_int_test_set_t
    
class cap_ms_csr_int_prp5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_prp5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_prp5_t(string name = "cap_ms_csr_int_prp5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_prp5_t();
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
    
        cap_ms_csr_int_prp5_int_test_set_t intreg;
    
        cap_ms_csr_int_prp5_int_test_set_t int_test_set;
    
        cap_ms_csr_int_prp5_int_enable_clear_t int_enable_set;
    
        cap_ms_csr_int_prp5_int_enable_clear_t int_enable_clear;
    
}; // cap_ms_csr_int_prp5_t
    
class cap_ms_csr_int_esecure_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_esecure_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_esecure_int_enable_clear_t(string name = "cap_ms_csr_int_esecure_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_esecure_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > mb_rxint_enable_cpp_int_t;
        cpp_int int_var__mb_rxint_enable;
        void mb_rxint_enable (const cpp_int  & l__val);
        cpp_int mb_rxint_enable() const;
    
        typedef pu_cpp_int< 1 > mb_txint_enable_cpp_int_t;
        cpp_int int_var__mb_txint_enable;
        void mb_txint_enable (const cpp_int  & l__val);
        cpp_int mb_txint_enable() const;
    
}; // cap_ms_csr_int_esecure_int_enable_clear_t
    
class cap_ms_csr_int_esecure_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_esecure_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_esecure_int_test_set_t(string name = "cap_ms_csr_int_esecure_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_esecure_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > mb_rxint_interrupt_cpp_int_t;
        cpp_int int_var__mb_rxint_interrupt;
        void mb_rxint_interrupt (const cpp_int  & l__val);
        cpp_int mb_rxint_interrupt() const;
    
        typedef pu_cpp_int< 1 > mb_txint_interrupt_cpp_int_t;
        cpp_int int_var__mb_txint_interrupt;
        void mb_txint_interrupt (const cpp_int  & l__val);
        cpp_int mb_txint_interrupt() const;
    
}; // cap_ms_csr_int_esecure_int_test_set_t
    
class cap_ms_csr_int_esecure_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_esecure_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_esecure_t(string name = "cap_ms_csr_int_esecure_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_esecure_t();
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
    
        cap_ms_csr_int_esecure_int_test_set_t intreg;
    
        cap_ms_csr_int_esecure_int_test_set_t int_test_set;
    
        cap_ms_csr_int_esecure_int_enable_clear_t int_enable_set;
    
        cap_ms_csr_int_esecure_int_enable_clear_t int_enable_clear;
    
}; // cap_ms_csr_int_esecure_t
    
class cap_ms_csr_int_misc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_misc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_misc_int_enable_clear_t(string name = "cap_ms_csr_int_misc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_misc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > qspi_ram_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__qspi_ram_ecc_uncorrectable_enable;
        void qspi_ram_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int qspi_ram_ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > qspi_ram_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__qspi_ram_ecc_correctable_enable;
        void qspi_ram_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int qspi_ram_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > elam_cap_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__elam_cap_ecc_uncorrectable_enable;
        void elam_cap_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int elam_cap_ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > elam_cap_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__elam_cap_ecc_correctable_enable;
        void elam_cap_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int elam_cap_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_data_ram_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__ahb_esec_data_ram_ecc_uncorrectable_enable;
        void ahb_esec_data_ram_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int ahb_esec_data_ram_ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_data_ram_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__ahb_esec_data_ram_ecc_correctable_enable;
        void ahb_esec_data_ram_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int ahb_esec_data_ram_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_esystem_ram_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__ahb_esec_esystem_ram_ecc_uncorrectable_enable;
        void ahb_esec_esystem_ram_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int ahb_esec_esystem_ram_ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_esystem_ram_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__ahb_esec_esystem_ram_ecc_correctable_enable;
        void ahb_esec_esystem_ram_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int ahb_esec_esystem_ram_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > bl2_ram_ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__bl2_ram_ecc_uncorrectable_enable;
        void bl2_ram_ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int bl2_ram_ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > bl2_ram_ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__bl2_ram_ecc_correctable_enable;
        void bl2_ram_ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int bl2_ram_ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > int_wr_stall_hit0_enable_cpp_int_t;
        cpp_int int_var__int_wr_stall_hit0_enable;
        void int_wr_stall_hit0_enable (const cpp_int  & l__val);
        cpp_int int_wr_stall_hit0_enable() const;
    
        typedef pu_cpp_int< 1 > int_rd_stall_hit0_enable_cpp_int_t;
        cpp_int int_var__int_rd_stall_hit0_enable;
        void int_rd_stall_hit0_enable (const cpp_int  & l__val);
        cpp_int int_rd_stall_hit0_enable() const;
    
        typedef pu_cpp_int< 1 > int_wr_stall_hit1_enable_cpp_int_t;
        cpp_int int_var__int_wr_stall_hit1_enable;
        void int_wr_stall_hit1_enable (const cpp_int  & l__val);
        cpp_int int_wr_stall_hit1_enable() const;
    
        typedef pu_cpp_int< 1 > int_rd_stall_hit1_enable_cpp_int_t;
        cpp_int int_var__int_rd_stall_hit1_enable;
        void int_rd_stall_hit1_enable (const cpp_int  & l__val);
        cpp_int int_rd_stall_hit1_enable() const;
    
        typedef pu_cpp_int< 1 > int_wr_stall_hit2_enable_cpp_int_t;
        cpp_int int_var__int_wr_stall_hit2_enable;
        void int_wr_stall_hit2_enable (const cpp_int  & l__val);
        cpp_int int_wr_stall_hit2_enable() const;
    
        typedef pu_cpp_int< 1 > int_rd_stall_hit2_enable_cpp_int_t;
        cpp_int int_var__int_rd_stall_hit2_enable;
        void int_rd_stall_hit2_enable (const cpp_int  & l__val);
        cpp_int int_rd_stall_hit2_enable() const;
    
        typedef pu_cpp_int< 1 > int_wr_full0_enable_cpp_int_t;
        cpp_int int_var__int_wr_full0_enable;
        void int_wr_full0_enable (const cpp_int  & l__val);
        cpp_int int_wr_full0_enable() const;
    
        typedef pu_cpp_int< 1 > int_rd_full0_enable_cpp_int_t;
        cpp_int int_var__int_rd_full0_enable;
        void int_rd_full0_enable (const cpp_int  & l__val);
        cpp_int int_rd_full0_enable() const;
    
        typedef pu_cpp_int< 1 > int_wr_full1_enable_cpp_int_t;
        cpp_int int_var__int_wr_full1_enable;
        void int_wr_full1_enable (const cpp_int  & l__val);
        cpp_int int_wr_full1_enable() const;
    
        typedef pu_cpp_int< 1 > int_rd_full1_enable_cpp_int_t;
        cpp_int int_var__int_rd_full1_enable;
        void int_rd_full1_enable (const cpp_int  & l__val);
        cpp_int int_rd_full1_enable() const;
    
        typedef pu_cpp_int< 1 > int_wr_full2_enable_cpp_int_t;
        cpp_int int_var__int_wr_full2_enable;
        void int_wr_full2_enable (const cpp_int  & l__val);
        cpp_int int_wr_full2_enable() const;
    
        typedef pu_cpp_int< 1 > int_rd_full2_enable_cpp_int_t;
        cpp_int int_var__int_rd_full2_enable;
        void int_rd_full2_enable (const cpp_int  & l__val);
        cpp_int int_rd_full2_enable() const;
    
}; // cap_ms_csr_int_misc_int_enable_clear_t
    
class cap_ms_csr_int_misc_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_misc_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_misc_int_test_set_t(string name = "cap_ms_csr_int_misc_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_misc_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > qspi_ram_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__qspi_ram_ecc_uncorrectable_interrupt;
        void qspi_ram_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int qspi_ram_ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > qspi_ram_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__qspi_ram_ecc_correctable_interrupt;
        void qspi_ram_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int qspi_ram_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > elam_cap_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__elam_cap_ecc_uncorrectable_interrupt;
        void elam_cap_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int elam_cap_ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > elam_cap_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__elam_cap_ecc_correctable_interrupt;
        void elam_cap_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int elam_cap_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_data_ram_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__ahb_esec_data_ram_ecc_uncorrectable_interrupt;
        void ahb_esec_data_ram_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int ahb_esec_data_ram_ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_data_ram_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__ahb_esec_data_ram_ecc_correctable_interrupt;
        void ahb_esec_data_ram_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int ahb_esec_data_ram_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_esystem_ram_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__ahb_esec_esystem_ram_ecc_uncorrectable_interrupt;
        void ahb_esec_esystem_ram_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int ahb_esec_esystem_ram_ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > ahb_esec_esystem_ram_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__ahb_esec_esystem_ram_ecc_correctable_interrupt;
        void ahb_esec_esystem_ram_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int ahb_esec_esystem_ram_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > bl2_ram_ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__bl2_ram_ecc_uncorrectable_interrupt;
        void bl2_ram_ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int bl2_ram_ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > bl2_ram_ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__bl2_ram_ecc_correctable_interrupt;
        void bl2_ram_ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int bl2_ram_ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_wr_stall_hit0_interrupt_cpp_int_t;
        cpp_int int_var__int_wr_stall_hit0_interrupt;
        void int_wr_stall_hit0_interrupt (const cpp_int  & l__val);
        cpp_int int_wr_stall_hit0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rd_stall_hit0_interrupt_cpp_int_t;
        cpp_int int_var__int_rd_stall_hit0_interrupt;
        void int_rd_stall_hit0_interrupt (const cpp_int  & l__val);
        cpp_int int_rd_stall_hit0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_wr_stall_hit1_interrupt_cpp_int_t;
        cpp_int int_var__int_wr_stall_hit1_interrupt;
        void int_wr_stall_hit1_interrupt (const cpp_int  & l__val);
        cpp_int int_wr_stall_hit1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rd_stall_hit1_interrupt_cpp_int_t;
        cpp_int int_var__int_rd_stall_hit1_interrupt;
        void int_rd_stall_hit1_interrupt (const cpp_int  & l__val);
        cpp_int int_rd_stall_hit1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_wr_stall_hit2_interrupt_cpp_int_t;
        cpp_int int_var__int_wr_stall_hit2_interrupt;
        void int_wr_stall_hit2_interrupt (const cpp_int  & l__val);
        cpp_int int_wr_stall_hit2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rd_stall_hit2_interrupt_cpp_int_t;
        cpp_int int_var__int_rd_stall_hit2_interrupt;
        void int_rd_stall_hit2_interrupt (const cpp_int  & l__val);
        cpp_int int_rd_stall_hit2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_wr_full0_interrupt_cpp_int_t;
        cpp_int int_var__int_wr_full0_interrupt;
        void int_wr_full0_interrupt (const cpp_int  & l__val);
        cpp_int int_wr_full0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rd_full0_interrupt_cpp_int_t;
        cpp_int int_var__int_rd_full0_interrupt;
        void int_rd_full0_interrupt (const cpp_int  & l__val);
        cpp_int int_rd_full0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_wr_full1_interrupt_cpp_int_t;
        cpp_int int_var__int_wr_full1_interrupt;
        void int_wr_full1_interrupt (const cpp_int  & l__val);
        cpp_int int_wr_full1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rd_full1_interrupt_cpp_int_t;
        cpp_int int_var__int_rd_full1_interrupt;
        void int_rd_full1_interrupt (const cpp_int  & l__val);
        cpp_int int_rd_full1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_wr_full2_interrupt_cpp_int_t;
        cpp_int int_var__int_wr_full2_interrupt;
        void int_wr_full2_interrupt (const cpp_int  & l__val);
        cpp_int int_wr_full2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rd_full2_interrupt_cpp_int_t;
        cpp_int int_var__int_rd_full2_interrupt;
        void int_rd_full2_interrupt (const cpp_int  & l__val);
        cpp_int int_rd_full2_interrupt() const;
    
}; // cap_ms_csr_int_misc_int_test_set_t
    
class cap_ms_csr_int_misc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_misc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_misc_t(string name = "cap_ms_csr_int_misc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_misc_t();
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
    
        cap_ms_csr_int_misc_int_test_set_t intreg;
    
        cap_ms_csr_int_misc_int_test_set_t int_test_set;
    
        cap_ms_csr_int_misc_int_enable_clear_t int_enable_set;
    
        cap_ms_csr_int_misc_int_enable_clear_t int_enable_clear;
    
}; // cap_ms_csr_int_misc_t
    
class cap_ms_csr_int_ms_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_ms_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_ms_int_enable_clear_t(string name = "cap_ms_csr_int_ms_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_ms_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > sbus_err_enable_cpp_int_t;
        cpp_int int_var__sbus_err_enable;
        void sbus_err_enable (const cpp_int  & l__val);
        cpp_int sbus_err_enable() const;
    
        typedef pu_cpp_int< 1 > rei_err_enable_cpp_int_t;
        cpp_int int_var__rei_err_enable;
        void rei_err_enable (const cpp_int  & l__val);
        cpp_int rei_err_enable() const;
    
        typedef pu_cpp_int< 1 > rei_pll_err_enable_cpp_int_t;
        cpp_int int_var__rei_pll_err_enable;
        void rei_pll_err_enable (const cpp_int  & l__val);
        cpp_int rei_pll_err_enable() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_init_read_timeout_enable_cpp_int_t;
        cpp_int int_var__pk_efuse_init_read_timeout_enable;
        void pk_efuse_init_read_timeout_enable (const cpp_int  & l__val);
        cpp_int pk_efuse_init_read_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_cpu_read_timeout_enable_cpp_int_t;
        cpp_int int_var__pk_efuse_cpu_read_timeout_enable;
        void pk_efuse_cpu_read_timeout_enable (const cpp_int  & l__val);
        cpp_int pk_efuse_cpu_read_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_cpu_burn_timeout_enable_cpp_int_t;
        cpp_int int_var__pk_efuse_cpu_burn_timeout_enable;
        void pk_efuse_cpu_burn_timeout_enable (const cpp_int  & l__val);
        cpp_int pk_efuse_cpu_burn_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_burn_imm_timeout_enable_cpp_int_t;
        cpp_int int_var__pk_efuse_burn_imm_timeout_enable;
        void pk_efuse_burn_imm_timeout_enable (const cpp_int  & l__val);
        cpp_int pk_efuse_burn_imm_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > ds16_efuse_init_read_timeout_enable_cpp_int_t;
        cpp_int int_var__ds16_efuse_init_read_timeout_enable;
        void ds16_efuse_init_read_timeout_enable (const cpp_int  & l__val);
        cpp_int ds16_efuse_init_read_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > ds16_efuse_cpu_read_timeout_enable_cpp_int_t;
        cpp_int int_var__ds16_efuse_cpu_read_timeout_enable;
        void ds16_efuse_cpu_read_timeout_enable (const cpp_int  & l__val);
        cpp_int ds16_efuse_cpu_read_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > ahb_axi_err_enable_cpp_int_t;
        cpp_int int_var__ahb_axi_err_enable;
        void ahb_axi_err_enable (const cpp_int  & l__val);
        cpp_int ahb_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > emmc_pre_axi_err_enable_cpp_int_t;
        cpp_int int_var__emmc_pre_axi_err_enable;
        void emmc_pre_axi_err_enable (const cpp_int  & l__val);
        cpp_int emmc_pre_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > fl_axi_err_enable_cpp_int_t;
        cpp_int int_var__fl_axi_err_enable;
        void fl_axi_err_enable (const cpp_int  & l__val);
        cpp_int fl_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > ms_esec_mb_pre_axi_err_enable_cpp_int_t;
        cpp_int int_var__ms_esec_mb_pre_axi_err_enable;
        void ms_esec_mb_pre_axi_err_enable (const cpp_int  & l__val);
        cpp_int ms_esec_mb_pre_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > msap_pre_axi_err_enable_cpp_int_t;
        cpp_int int_var__msap_pre_axi_err_enable;
        void msap_pre_axi_err_enable (const cpp_int  & l__val);
        cpp_int msap_pre_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > nxms_a13_axi_err_enable_cpp_int_t;
        cpp_int int_var__nxms_a13_axi_err_enable;
        void nxms_a13_axi_err_enable (const cpp_int  & l__val);
        cpp_int nxms_a13_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > rbm_axi_err_enable_cpp_int_t;
        cpp_int int_var__rbm_axi_err_enable;
        void rbm_axi_err_enable (const cpp_int  & l__val);
        cpp_int rbm_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > ssram_axi_err_enable_cpp_int_t;
        cpp_int int_var__ssram_axi_err_enable;
        void ssram_axi_err_enable (const cpp_int  & l__val);
        cpp_int ssram_axi_err_enable() const;
    
        typedef pu_cpp_int< 1 > ahb_axi_werr_enable_cpp_int_t;
        cpp_int int_var__ahb_axi_werr_enable;
        void ahb_axi_werr_enable (const cpp_int  & l__val);
        cpp_int ahb_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > emmc_pre_axi_werr_enable_cpp_int_t;
        cpp_int int_var__emmc_pre_axi_werr_enable;
        void emmc_pre_axi_werr_enable (const cpp_int  & l__val);
        cpp_int emmc_pre_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > fl_axi_werr_enable_cpp_int_t;
        cpp_int int_var__fl_axi_werr_enable;
        void fl_axi_werr_enable (const cpp_int  & l__val);
        cpp_int fl_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > ms_esec_mb_pre_axi_werr_enable_cpp_int_t;
        cpp_int int_var__ms_esec_mb_pre_axi_werr_enable;
        void ms_esec_mb_pre_axi_werr_enable (const cpp_int  & l__val);
        cpp_int ms_esec_mb_pre_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > msap_pre_axi_werr_enable_cpp_int_t;
        cpp_int int_var__msap_pre_axi_werr_enable;
        void msap_pre_axi_werr_enable (const cpp_int  & l__val);
        cpp_int msap_pre_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > nxms_a13_axi_werr_enable_cpp_int_t;
        cpp_int int_var__nxms_a13_axi_werr_enable;
        void nxms_a13_axi_werr_enable (const cpp_int  & l__val);
        cpp_int nxms_a13_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > rbm_axi_werr_enable_cpp_int_t;
        cpp_int int_var__rbm_axi_werr_enable;
        void rbm_axi_werr_enable (const cpp_int  & l__val);
        cpp_int rbm_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > ssram_axi_werr_enable_cpp_int_t;
        cpp_int int_var__ssram_axi_werr_enable;
        void ssram_axi_werr_enable (const cpp_int  & l__val);
        cpp_int ssram_axi_werr_enable() const;
    
        typedef pu_cpp_int< 1 > addr_filter_write_enable_cpp_int_t;
        cpp_int int_var__addr_filter_write_enable;
        void addr_filter_write_enable (const cpp_int  & l__val);
        cpp_int addr_filter_write_enable() const;
    
        typedef pu_cpp_int< 1 > addr_filter_read_enable_cpp_int_t;
        cpp_int int_var__addr_filter_read_enable;
        void addr_filter_read_enable (const cpp_int  & l__val);
        cpp_int addr_filter_read_enable() const;
    
}; // cap_ms_csr_int_ms_int_enable_clear_t
    
class cap_ms_csr_int_ms_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_ms_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_ms_intreg_t(string name = "cap_ms_csr_int_ms_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_ms_intreg_t();
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
    
        typedef pu_cpp_int< 1 > sbus_err_interrupt_cpp_int_t;
        cpp_int int_var__sbus_err_interrupt;
        void sbus_err_interrupt (const cpp_int  & l__val);
        cpp_int sbus_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rei_err_interrupt_cpp_int_t;
        cpp_int int_var__rei_err_interrupt;
        void rei_err_interrupt (const cpp_int  & l__val);
        cpp_int rei_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rei_pll_err_interrupt_cpp_int_t;
        cpp_int int_var__rei_pll_err_interrupt;
        void rei_pll_err_interrupt (const cpp_int  & l__val);
        cpp_int rei_pll_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_init_read_timeout_interrupt_cpp_int_t;
        cpp_int int_var__pk_efuse_init_read_timeout_interrupt;
        void pk_efuse_init_read_timeout_interrupt (const cpp_int  & l__val);
        cpp_int pk_efuse_init_read_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_cpu_read_timeout_interrupt_cpp_int_t;
        cpp_int int_var__pk_efuse_cpu_read_timeout_interrupt;
        void pk_efuse_cpu_read_timeout_interrupt (const cpp_int  & l__val);
        cpp_int pk_efuse_cpu_read_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_cpu_burn_timeout_interrupt_cpp_int_t;
        cpp_int int_var__pk_efuse_cpu_burn_timeout_interrupt;
        void pk_efuse_cpu_burn_timeout_interrupt (const cpp_int  & l__val);
        cpp_int pk_efuse_cpu_burn_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > pk_efuse_burn_imm_timeout_interrupt_cpp_int_t;
        cpp_int int_var__pk_efuse_burn_imm_timeout_interrupt;
        void pk_efuse_burn_imm_timeout_interrupt (const cpp_int  & l__val);
        cpp_int pk_efuse_burn_imm_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > ds16_efuse_init_read_timeout_interrupt_cpp_int_t;
        cpp_int int_var__ds16_efuse_init_read_timeout_interrupt;
        void ds16_efuse_init_read_timeout_interrupt (const cpp_int  & l__val);
        cpp_int ds16_efuse_init_read_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > ds16_efuse_cpu_read_timeout_interrupt_cpp_int_t;
        cpp_int int_var__ds16_efuse_cpu_read_timeout_interrupt;
        void ds16_efuse_cpu_read_timeout_interrupt (const cpp_int  & l__val);
        cpp_int ds16_efuse_cpu_read_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > ahb_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__ahb_axi_err_interrupt;
        void ahb_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int ahb_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > emmc_pre_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__emmc_pre_axi_err_interrupt;
        void emmc_pre_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int emmc_pre_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > fl_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__fl_axi_err_interrupt;
        void fl_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int fl_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ms_esec_mb_pre_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__ms_esec_mb_pre_axi_err_interrupt;
        void ms_esec_mb_pre_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int ms_esec_mb_pre_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > msap_pre_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__msap_pre_axi_err_interrupt;
        void msap_pre_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int msap_pre_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > nxms_a13_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__nxms_a13_axi_err_interrupt;
        void nxms_a13_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int nxms_a13_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rbm_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__rbm_axi_err_interrupt;
        void rbm_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int rbm_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ssram_axi_err_interrupt_cpp_int_t;
        cpp_int int_var__ssram_axi_err_interrupt;
        void ssram_axi_err_interrupt (const cpp_int  & l__val);
        cpp_int ssram_axi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ahb_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__ahb_axi_werr_interrupt;
        void ahb_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int ahb_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > emmc_pre_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__emmc_pre_axi_werr_interrupt;
        void emmc_pre_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int emmc_pre_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > fl_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__fl_axi_werr_interrupt;
        void fl_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int fl_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > ms_esec_mb_pre_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__ms_esec_mb_pre_axi_werr_interrupt;
        void ms_esec_mb_pre_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int ms_esec_mb_pre_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > msap_pre_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__msap_pre_axi_werr_interrupt;
        void msap_pre_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int msap_pre_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > nxms_a13_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__nxms_a13_axi_werr_interrupt;
        void nxms_a13_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int nxms_a13_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rbm_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__rbm_axi_werr_interrupt;
        void rbm_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int rbm_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > ssram_axi_werr_interrupt_cpp_int_t;
        cpp_int int_var__ssram_axi_werr_interrupt;
        void ssram_axi_werr_interrupt (const cpp_int  & l__val);
        cpp_int ssram_axi_werr_interrupt() const;
    
        typedef pu_cpp_int< 1 > addr_filter_write_interrupt_cpp_int_t;
        cpp_int int_var__addr_filter_write_interrupt;
        void addr_filter_write_interrupt (const cpp_int  & l__val);
        cpp_int addr_filter_write_interrupt() const;
    
        typedef pu_cpp_int< 1 > addr_filter_read_interrupt_cpp_int_t;
        cpp_int int_var__addr_filter_read_interrupt;
        void addr_filter_read_interrupt (const cpp_int  & l__val);
        cpp_int addr_filter_read_interrupt() const;
    
}; // cap_ms_csr_int_ms_intreg_t
    
class cap_ms_csr_int_ms_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_ms_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_ms_t(string name = "cap_ms_csr_int_ms_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_ms_t();
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
    
        cap_ms_csr_int_ms_intreg_t intreg;
    
        cap_ms_csr_int_ms_intreg_t int_test_set;
    
        cap_ms_csr_int_ms_int_enable_clear_t int_enable_set;
    
        cap_ms_csr_int_ms_int_enable_clear_t int_enable_clear;
    
}; // cap_ms_csr_int_ms_t
    
class cap_ms_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_intreg_status_t(string name = "cap_ms_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_ms_interrupt_cpp_int_t;
        cpp_int int_var__int_ms_interrupt;
        void int_ms_interrupt (const cpp_int  & l__val);
        cpp_int int_ms_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_misc_interrupt_cpp_int_t;
        cpp_int int_var__int_misc_interrupt;
        void int_misc_interrupt (const cpp_int  & l__val);
        cpp_int int_misc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_esecure_interrupt_cpp_int_t;
        cpp_int int_var__int_esecure_interrupt;
        void int_esecure_interrupt (const cpp_int  & l__val);
        cpp_int int_esecure_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_prp1_interrupt_cpp_int_t;
        cpp_int int_var__int_prp1_interrupt;
        void int_prp1_interrupt (const cpp_int  & l__val);
        cpp_int int_prp1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_prp2_interrupt_cpp_int_t;
        cpp_int int_var__int_prp2_interrupt;
        void int_prp2_interrupt (const cpp_int  & l__val);
        cpp_int int_prp2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_prp3_interrupt_cpp_int_t;
        cpp_int int_var__int_prp3_interrupt;
        void int_prp3_interrupt (const cpp_int  & l__val);
        cpp_int int_prp3_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_prp4_interrupt_cpp_int_t;
        cpp_int int_var__int_prp4_interrupt;
        void int_prp4_interrupt (const cpp_int  & l__val);
        cpp_int int_prp4_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_prp5_interrupt_cpp_int_t;
        cpp_int int_var__int_prp5_interrupt;
        void int_prp5_interrupt (const cpp_int  & l__val);
        cpp_int int_prp5_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic0_interrupt_cpp_int_t;
        cpp_int int_var__int_gic0_interrupt;
        void int_gic0_interrupt (const cpp_int  & l__val);
        cpp_int int_gic0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic1_interrupt_cpp_int_t;
        cpp_int int_var__int_gic1_interrupt;
        void int_gic1_interrupt (const cpp_int  & l__val);
        cpp_int int_gic1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic2_interrupt_cpp_int_t;
        cpp_int int_var__int_gic2_interrupt;
        void int_gic2_interrupt (const cpp_int  & l__val);
        cpp_int int_gic2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic3_interrupt_cpp_int_t;
        cpp_int int_var__int_gic3_interrupt;
        void int_gic3_interrupt (const cpp_int  & l__val);
        cpp_int int_gic3_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic4_interrupt_cpp_int_t;
        cpp_int int_var__int_gic4_interrupt;
        void int_gic4_interrupt (const cpp_int  & l__val);
        cpp_int int_gic4_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic5_interrupt_cpp_int_t;
        cpp_int int_var__int_gic5_interrupt;
        void int_gic5_interrupt (const cpp_int  & l__val);
        cpp_int int_gic5_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic6_interrupt_cpp_int_t;
        cpp_int int_var__int_gic6_interrupt;
        void int_gic6_interrupt (const cpp_int  & l__val);
        cpp_int int_gic6_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic7_interrupt_cpp_int_t;
        cpp_int int_var__int_gic7_interrupt;
        void int_gic7_interrupt (const cpp_int  & l__val);
        cpp_int int_gic7_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic8_interrupt_cpp_int_t;
        cpp_int int_var__int_gic8_interrupt;
        void int_gic8_interrupt (const cpp_int  & l__val);
        cpp_int int_gic8_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic9_interrupt_cpp_int_t;
        cpp_int int_var__int_gic9_interrupt;
        void int_gic9_interrupt (const cpp_int  & l__val);
        cpp_int int_gic9_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic10_interrupt_cpp_int_t;
        cpp_int int_var__int_gic10_interrupt;
        void int_gic10_interrupt (const cpp_int  & l__val);
        cpp_int int_gic10_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic11_interrupt_cpp_int_t;
        cpp_int int_var__int_gic11_interrupt;
        void int_gic11_interrupt (const cpp_int  & l__val);
        cpp_int int_gic11_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic12_interrupt_cpp_int_t;
        cpp_int int_var__int_gic12_interrupt;
        void int_gic12_interrupt (const cpp_int  & l__val);
        cpp_int int_gic12_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic13_interrupt_cpp_int_t;
        cpp_int int_var__int_gic13_interrupt;
        void int_gic13_interrupt (const cpp_int  & l__val);
        cpp_int int_gic13_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic14_interrupt_cpp_int_t;
        cpp_int int_var__int_gic14_interrupt;
        void int_gic14_interrupt (const cpp_int  & l__val);
        cpp_int int_gic14_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic15_interrupt_cpp_int_t;
        cpp_int int_var__int_gic15_interrupt;
        void int_gic15_interrupt (const cpp_int  & l__val);
        cpp_int int_gic15_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic16_interrupt_cpp_int_t;
        cpp_int int_var__int_gic16_interrupt;
        void int_gic16_interrupt (const cpp_int  & l__val);
        cpp_int int_gic16_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic17_interrupt_cpp_int_t;
        cpp_int int_var__int_gic17_interrupt;
        void int_gic17_interrupt (const cpp_int  & l__val);
        cpp_int int_gic17_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic18_interrupt_cpp_int_t;
        cpp_int int_var__int_gic18_interrupt;
        void int_gic18_interrupt (const cpp_int  & l__val);
        cpp_int int_gic18_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic19_interrupt_cpp_int_t;
        cpp_int int_var__int_gic19_interrupt;
        void int_gic19_interrupt (const cpp_int  & l__val);
        cpp_int int_gic19_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic20_interrupt_cpp_int_t;
        cpp_int int_var__int_gic20_interrupt;
        void int_gic20_interrupt (const cpp_int  & l__val);
        cpp_int int_gic20_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic21_interrupt_cpp_int_t;
        cpp_int int_var__int_gic21_interrupt;
        void int_gic21_interrupt (const cpp_int  & l__val);
        cpp_int int_gic21_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic22_interrupt_cpp_int_t;
        cpp_int int_var__int_gic22_interrupt;
        void int_gic22_interrupt (const cpp_int  & l__val);
        cpp_int int_gic22_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_gic23_interrupt_cpp_int_t;
        cpp_int int_var__int_gic23_interrupt;
        void int_gic23_interrupt (const cpp_int  & l__val);
        cpp_int int_gic23_interrupt() const;
    
}; // cap_ms_csr_intreg_status_t
    
class cap_ms_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_int_groups_int_enable_rw_reg_t(string name = "cap_ms_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_ms_enable_cpp_int_t;
        cpp_int int_var__int_ms_enable;
        void int_ms_enable (const cpp_int  & l__val);
        cpp_int int_ms_enable() const;
    
        typedef pu_cpp_int< 1 > int_misc_enable_cpp_int_t;
        cpp_int int_var__int_misc_enable;
        void int_misc_enable (const cpp_int  & l__val);
        cpp_int int_misc_enable() const;
    
        typedef pu_cpp_int< 1 > int_esecure_enable_cpp_int_t;
        cpp_int int_var__int_esecure_enable;
        void int_esecure_enable (const cpp_int  & l__val);
        cpp_int int_esecure_enable() const;
    
        typedef pu_cpp_int< 1 > int_prp1_enable_cpp_int_t;
        cpp_int int_var__int_prp1_enable;
        void int_prp1_enable (const cpp_int  & l__val);
        cpp_int int_prp1_enable() const;
    
        typedef pu_cpp_int< 1 > int_prp2_enable_cpp_int_t;
        cpp_int int_var__int_prp2_enable;
        void int_prp2_enable (const cpp_int  & l__val);
        cpp_int int_prp2_enable() const;
    
        typedef pu_cpp_int< 1 > int_prp3_enable_cpp_int_t;
        cpp_int int_var__int_prp3_enable;
        void int_prp3_enable (const cpp_int  & l__val);
        cpp_int int_prp3_enable() const;
    
        typedef pu_cpp_int< 1 > int_prp4_enable_cpp_int_t;
        cpp_int int_var__int_prp4_enable;
        void int_prp4_enable (const cpp_int  & l__val);
        cpp_int int_prp4_enable() const;
    
        typedef pu_cpp_int< 1 > int_prp5_enable_cpp_int_t;
        cpp_int int_var__int_prp5_enable;
        void int_prp5_enable (const cpp_int  & l__val);
        cpp_int int_prp5_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic0_enable_cpp_int_t;
        cpp_int int_var__int_gic0_enable;
        void int_gic0_enable (const cpp_int  & l__val);
        cpp_int int_gic0_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic1_enable_cpp_int_t;
        cpp_int int_var__int_gic1_enable;
        void int_gic1_enable (const cpp_int  & l__val);
        cpp_int int_gic1_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic2_enable_cpp_int_t;
        cpp_int int_var__int_gic2_enable;
        void int_gic2_enable (const cpp_int  & l__val);
        cpp_int int_gic2_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic3_enable_cpp_int_t;
        cpp_int int_var__int_gic3_enable;
        void int_gic3_enable (const cpp_int  & l__val);
        cpp_int int_gic3_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic4_enable_cpp_int_t;
        cpp_int int_var__int_gic4_enable;
        void int_gic4_enable (const cpp_int  & l__val);
        cpp_int int_gic4_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic5_enable_cpp_int_t;
        cpp_int int_var__int_gic5_enable;
        void int_gic5_enable (const cpp_int  & l__val);
        cpp_int int_gic5_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic6_enable_cpp_int_t;
        cpp_int int_var__int_gic6_enable;
        void int_gic6_enable (const cpp_int  & l__val);
        cpp_int int_gic6_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic7_enable_cpp_int_t;
        cpp_int int_var__int_gic7_enable;
        void int_gic7_enable (const cpp_int  & l__val);
        cpp_int int_gic7_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic8_enable_cpp_int_t;
        cpp_int int_var__int_gic8_enable;
        void int_gic8_enable (const cpp_int  & l__val);
        cpp_int int_gic8_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic9_enable_cpp_int_t;
        cpp_int int_var__int_gic9_enable;
        void int_gic9_enable (const cpp_int  & l__val);
        cpp_int int_gic9_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic10_enable_cpp_int_t;
        cpp_int int_var__int_gic10_enable;
        void int_gic10_enable (const cpp_int  & l__val);
        cpp_int int_gic10_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic11_enable_cpp_int_t;
        cpp_int int_var__int_gic11_enable;
        void int_gic11_enable (const cpp_int  & l__val);
        cpp_int int_gic11_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic12_enable_cpp_int_t;
        cpp_int int_var__int_gic12_enable;
        void int_gic12_enable (const cpp_int  & l__val);
        cpp_int int_gic12_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic13_enable_cpp_int_t;
        cpp_int int_var__int_gic13_enable;
        void int_gic13_enable (const cpp_int  & l__val);
        cpp_int int_gic13_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic14_enable_cpp_int_t;
        cpp_int int_var__int_gic14_enable;
        void int_gic14_enable (const cpp_int  & l__val);
        cpp_int int_gic14_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic15_enable_cpp_int_t;
        cpp_int int_var__int_gic15_enable;
        void int_gic15_enable (const cpp_int  & l__val);
        cpp_int int_gic15_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic16_enable_cpp_int_t;
        cpp_int int_var__int_gic16_enable;
        void int_gic16_enable (const cpp_int  & l__val);
        cpp_int int_gic16_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic17_enable_cpp_int_t;
        cpp_int int_var__int_gic17_enable;
        void int_gic17_enable (const cpp_int  & l__val);
        cpp_int int_gic17_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic18_enable_cpp_int_t;
        cpp_int int_var__int_gic18_enable;
        void int_gic18_enable (const cpp_int  & l__val);
        cpp_int int_gic18_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic19_enable_cpp_int_t;
        cpp_int int_var__int_gic19_enable;
        void int_gic19_enable (const cpp_int  & l__val);
        cpp_int int_gic19_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic20_enable_cpp_int_t;
        cpp_int int_var__int_gic20_enable;
        void int_gic20_enable (const cpp_int  & l__val);
        cpp_int int_gic20_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic21_enable_cpp_int_t;
        cpp_int int_var__int_gic21_enable;
        void int_gic21_enable (const cpp_int  & l__val);
        cpp_int int_gic21_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic22_enable_cpp_int_t;
        cpp_int int_var__int_gic22_enable;
        void int_gic22_enable (const cpp_int  & l__val);
        cpp_int int_gic22_enable() const;
    
        typedef pu_cpp_int< 1 > int_gic23_enable_cpp_int_t;
        cpp_int int_var__int_gic23_enable;
        void int_gic23_enable (const cpp_int  & l__val);
        cpp_int int_gic23_enable() const;
    
}; // cap_ms_csr_int_groups_int_enable_rw_reg_t
    
class cap_ms_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_intgrp_status_t(string name = "cap_ms_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_intgrp_status_t();
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
    
        cap_ms_csr_intreg_status_t intreg;
    
        cap_ms_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_ms_csr_intreg_status_t int_rw_reg;
    
}; // cap_ms_csr_intgrp_status_t
    
class cap_ms_csr_dhs_elam_capture_en_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_capture_en_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_capture_en_entry_t(string name = "cap_ms_csr_dhs_elam_capture_en_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_capture_en_entry_t();
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
    
        typedef pu_cpp_int< 17 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
        typedef pu_cpp_int< 17 > match_cpp_int_t;
        cpp_int int_var__match;
        void match (const cpp_int  & l__val);
        cpp_int match() const;
    
        typedef pu_cpp_int< 1 > inv_cpp_int_t;
        cpp_int int_var__inv;
        void inv (const cpp_int  & l__val);
        cpp_int inv() const;
    
        typedef pu_cpp_int< 1 > op_cpp_int_t;
        cpp_int int_var__op;
        void op (const cpp_int  & l__val);
        cpp_int op() const;
    
}; // cap_ms_csr_dhs_elam_capture_en_entry_t
    
class cap_ms_csr_dhs_elam_capture_en_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_capture_en_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_capture_en_t(string name = "cap_ms_csr_dhs_elam_capture_en_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_capture_en_t();
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
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ms_csr_dhs_elam_capture_en_entry_t, 4> entry;
        #else 
        cap_ms_csr_dhs_elam_capture_en_entry_t entry[4];
        #endif
        int get_depth_entry() { return 4; }
    
}; // cap_ms_csr_dhs_elam_capture_en_t
    
class cap_ms_csr_dhs_elam_branch_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_branch_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_branch_entry_t(string name = "cap_ms_csr_dhs_elam_branch_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_branch_entry_t();
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
    
        typedef pu_cpp_int< 4 > cur_state_cpp_int_t;
        cpp_int int_var__cur_state;
        void cur_state (const cpp_int  & l__val);
        cpp_int cur_state() const;
    
        typedef pu_cpp_int< 6 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
        typedef pu_cpp_int< 13 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
        typedef pu_cpp_int< 13 > match_cpp_int_t;
        cpp_int int_var__match;
        void match (const cpp_int  & l__val);
        cpp_int match() const;
    
        typedef pu_cpp_int< 1 > inv_cpp_int_t;
        cpp_int int_var__inv;
        void inv (const cpp_int  & l__val);
        cpp_int inv() const;
    
        typedef pu_cpp_int< 1 > op_cpp_int_t;
        cpp_int int_var__op;
        void op (const cpp_int  & l__val);
        cpp_int op() const;
    
        typedef pu_cpp_int< 4 > next_state_cpp_int_t;
        cpp_int int_var__next_state;
        void next_state (const cpp_int  & l__val);
        cpp_int next_state() const;
    
        typedef pu_cpp_int< 1 > trigger_cpp_int_t;
        cpp_int int_var__trigger;
        void trigger (const cpp_int  & l__val);
        cpp_int trigger() const;
    
        typedef pu_cpp_int< 1 > cnt_inc_cpp_int_t;
        cpp_int int_var__cnt_inc;
        void cnt_inc (const cpp_int  & l__val);
        cpp_int cnt_inc() const;
    
        typedef pu_cpp_int< 1 > cnt_clr_cpp_int_t;
        cpp_int int_var__cnt_clr;
        void cnt_clr (const cpp_int  & l__val);
        cpp_int cnt_clr() const;
    
}; // cap_ms_csr_dhs_elam_branch_entry_t
    
class cap_ms_csr_dhs_elam_branch_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_branch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_branch_t(string name = "cap_ms_csr_dhs_elam_branch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_branch_t();
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
    
        #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ms_csr_dhs_elam_branch_entry_t, 24> entry;
        #else 
        cap_ms_csr_dhs_elam_branch_entry_t entry[24];
        #endif
        int get_depth_entry() { return 24; }
    
}; // cap_ms_csr_dhs_elam_branch_t
    
class cap_ms_csr_dhs_elam_eqt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_eqt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_eqt_entry_t(string name = "cap_ms_csr_dhs_elam_eqt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_eqt_entry_t();
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
    
        typedef pu_cpp_int< 1 > inv_cpp_int_t;
        cpp_int int_var__inv;
        void inv (const cpp_int  & l__val);
        cpp_int inv() const;
    
        typedef pu_cpp_int< 1 > op_cpp_int_t;
        cpp_int int_var__op;
        void op (const cpp_int  & l__val);
        cpp_int op() const;
    
        typedef pu_cpp_int< 9 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
        typedef pu_cpp_int< 9 > match_cpp_int_t;
        cpp_int int_var__match;
        void match (const cpp_int  & l__val);
        cpp_int match() const;
    
}; // cap_ms_csr_dhs_elam_eqt_entry_t
    
class cap_ms_csr_dhs_elam_eqt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_eqt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_eqt_t(string name = "cap_ms_csr_dhs_elam_eqt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_eqt_t();
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
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ms_csr_dhs_elam_eqt_entry_t, 4> entry;
        #else 
        cap_ms_csr_dhs_elam_eqt_entry_t entry[4];
        #endif
        int get_depth_entry() { return 4; }
    
}; // cap_ms_csr_dhs_elam_eqt_t
    
class cap_ms_csr_dhs_elam_m_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_m_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_m_entry_t(string name = "cap_ms_csr_dhs_elam_m_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_m_entry_t();
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
    
        typedef pu_cpp_int< 256 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
        typedef pu_cpp_int< 256 > match_cpp_int_t;
        cpp_int int_var__match;
        void match (const cpp_int  & l__val);
        cpp_int match() const;
    
        typedef pu_cpp_int< 1 > op_cpp_int_t;
        cpp_int int_var__op;
        void op (const cpp_int  & l__val);
        cpp_int op() const;
    
}; // cap_ms_csr_dhs_elam_m_entry_t
    
class cap_ms_csr_dhs_elam_m_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_m_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_m_t(string name = "cap_ms_csr_dhs_elam_m_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_m_t();
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
    
        #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ms_csr_dhs_elam_m_entry_t, 5> entry;
        #else 
        cap_ms_csr_dhs_elam_m_entry_t entry[5];
        #endif
        int get_depth_entry() { return 5; }
    
}; // cap_ms_csr_dhs_elam_m_t
    
class cap_ms_csr_dhs_sbus_indir_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_sbus_indir_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_sbus_indir_entry_t(string name = "cap_ms_csr_dhs_sbus_indir_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_sbus_indir_entry_t();
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
    
}; // cap_ms_csr_dhs_sbus_indir_entry_t
    
class cap_ms_csr_dhs_sbus_indir_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_sbus_indir_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_sbus_indir_t(string name = "cap_ms_csr_dhs_sbus_indir_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_sbus_indir_t();
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
    
        cap_ms_csr_dhs_sbus_indir_entry_t entry;
    
}; // cap_ms_csr_dhs_sbus_indir_t
    
class cap_ms_csr_dhs_elam_cap_buf_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_cap_buf_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_cap_buf_entry_t(string name = "cap_ms_csr_dhs_elam_cap_buf_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_cap_buf_entry_t();
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
    
        typedef pu_cpp_int< 322 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ms_csr_dhs_elam_cap_buf_entry_t
    
class cap_ms_csr_dhs_elam_cap_buf_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_dhs_elam_cap_buf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_dhs_elam_cap_buf_t(string name = "cap_ms_csr_dhs_elam_cap_buf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_dhs_elam_cap_buf_t();
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
    
        cap_ms_csr_dhs_elam_cap_buf_entry_t entry;
    
}; // cap_ms_csr_dhs_elam_cap_buf_t
    
class cap_ms_csr_cnt_axi_bw_mon_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cnt_axi_bw_mon_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cnt_axi_bw_mon_wr_t(string name = "cap_ms_csr_cnt_axi_bw_mon_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cnt_axi_bw_mon_wr_t();
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
    
}; // cap_ms_csr_cnt_axi_bw_mon_wr_t
    
class cap_ms_csr_sta_axi_bw_mon_wr_transactions_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_axi_bw_mon_wr_transactions_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_axi_bw_mon_wr_transactions_t(string name = "cap_ms_csr_sta_axi_bw_mon_wr_transactions_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_axi_bw_mon_wr_transactions_t();
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
    
        typedef pu_cpp_int< 8 > outstanding_cpp_int_t;
        cpp_int int_var__outstanding;
        void outstanding (const cpp_int  & l__val);
        cpp_int outstanding() const;
    
        typedef pu_cpp_int< 16 > dess_rdy_cpp_int_t;
        cpp_int int_var__dess_rdy;
        void dess_rdy (const cpp_int  & l__val);
        cpp_int dess_rdy() const;
    
}; // cap_ms_csr_sta_axi_bw_mon_wr_transactions_t
    
class cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t(string name = "cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t();
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
    
        typedef pu_cpp_int< 16 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 16 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t
    
class cap_ms_csr_sta_axi_bw_mon_wr_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_axi_bw_mon_wr_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_axi_bw_mon_wr_latency_t(string name = "cap_ms_csr_sta_axi_bw_mon_wr_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_axi_bw_mon_wr_latency_t();
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
    
        typedef pu_cpp_int< 14 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 14 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_ms_csr_sta_axi_bw_mon_wr_latency_t
    
class cap_ms_csr_cnt_axi_bw_mon_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cnt_axi_bw_mon_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cnt_axi_bw_mon_rd_t(string name = "cap_ms_csr_cnt_axi_bw_mon_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cnt_axi_bw_mon_rd_t();
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
    
}; // cap_ms_csr_cnt_axi_bw_mon_rd_t
    
class cap_ms_csr_sta_axi_bw_mon_rd_transactions_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_axi_bw_mon_rd_transactions_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_axi_bw_mon_rd_transactions_t(string name = "cap_ms_csr_sta_axi_bw_mon_rd_transactions_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_axi_bw_mon_rd_transactions_t();
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
    
        typedef pu_cpp_int< 8 > outstanding_cpp_int_t;
        cpp_int int_var__outstanding;
        void outstanding (const cpp_int  & l__val);
        cpp_int outstanding() const;
    
        typedef pu_cpp_int< 16 > dess_rdy_cpp_int_t;
        cpp_int int_var__dess_rdy;
        void dess_rdy (const cpp_int  & l__val);
        cpp_int dess_rdy() const;
    
}; // cap_ms_csr_sta_axi_bw_mon_rd_transactions_t
    
class cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t(string name = "cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t();
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
    
        typedef pu_cpp_int< 16 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 16 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t
    
class cap_ms_csr_sta_axi_bw_mon_rd_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_axi_bw_mon_rd_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_axi_bw_mon_rd_latency_t(string name = "cap_ms_csr_sta_axi_bw_mon_rd_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_axi_bw_mon_rd_latency_t();
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
    
        typedef pu_cpp_int< 14 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 14 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_ms_csr_sta_axi_bw_mon_rd_latency_t
    
class cap_ms_csr_cfg_axi_bw_mon_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_axi_bw_mon_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_axi_bw_mon_t(string name = "cap_ms_csr_cfg_axi_bw_mon_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_axi_bw_mon_t();
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
    
        typedef pu_cpp_int< 8 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
        typedef pu_cpp_int< 16 > cycle_cpp_int_t;
        cpp_int int_var__cycle;
        void cycle (const cpp_int  & l__val);
        cpp_int cycle() const;
    
        typedef pu_cpp_int< 4 > rcache_cpp_int_t;
        cpp_int int_var__rcache;
        void rcache (const cpp_int  & l__val);
        cpp_int rcache() const;
    
        typedef pu_cpp_int< 4 > rcache_msk_cpp_int_t;
        cpp_int int_var__rcache_msk;
        void rcache_msk (const cpp_int  & l__val);
        cpp_int rcache_msk() const;
    
        typedef pu_cpp_int< 4 > wcache_cpp_int_t;
        cpp_int int_var__wcache;
        void wcache (const cpp_int  & l__val);
        cpp_int wcache() const;
    
        typedef pu_cpp_int< 4 > wcache_msk_cpp_int_t;
        cpp_int int_var__wcache_msk;
        void wcache_msk (const cpp_int  & l__val);
        cpp_int wcache_msk() const;
    
}; // cap_ms_csr_cfg_axi_bw_mon_t
    
class cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name = "cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t
    
class cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name = "cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t
    
class cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name = "cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t
    
class cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name = "cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t
    
class cap_ms_csr_cfg_uid2sidLL_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_uid2sidLL_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_uid2sidLL_t(string name = "cap_ms_csr_cfg_uid2sidLL_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_uid2sidLL_t();
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
    
        typedef pu_cpp_int< 7 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 2 > mode_cpp_int_t;
        cpp_int int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
        typedef pu_cpp_int< 7 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
}; // cap_ms_csr_cfg_uid2sidLL_t
    
class cap_ms_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_csr_intr_t(string name = "cap_ms_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_csr_intr_t();
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
    
}; // cap_ms_csr_csr_intr_t
    
class cap_ms_csr_ms_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_axi_t(string name = "cap_ms_csr_ms_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_axi_t();
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
    
        typedef pu_cpp_int< 32 > ahb_axi_err_increment_cpp_int_t;
        cpp_int int_var__ahb_axi_err_increment;
        void ahb_axi_err_increment (const cpp_int  & l__val);
        cpp_int ahb_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > emmc_pre_axi_err_increment_cpp_int_t;
        cpp_int int_var__emmc_pre_axi_err_increment;
        void emmc_pre_axi_err_increment (const cpp_int  & l__val);
        cpp_int emmc_pre_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > fl_axi_err_increment_cpp_int_t;
        cpp_int int_var__fl_axi_err_increment;
        void fl_axi_err_increment (const cpp_int  & l__val);
        cpp_int fl_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > ms_esec_mb_pre_axi_err_increment_cpp_int_t;
        cpp_int int_var__ms_esec_mb_pre_axi_err_increment;
        void ms_esec_mb_pre_axi_err_increment (const cpp_int  & l__val);
        cpp_int ms_esec_mb_pre_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > msap_pre_axi_err_increment_cpp_int_t;
        cpp_int int_var__msap_pre_axi_err_increment;
        void msap_pre_axi_err_increment (const cpp_int  & l__val);
        cpp_int msap_pre_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > nxms_a13_axi_err_increment_cpp_int_t;
        cpp_int int_var__nxms_a13_axi_err_increment;
        void nxms_a13_axi_err_increment (const cpp_int  & l__val);
        cpp_int nxms_a13_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > rbm_axi_err_increment_cpp_int_t;
        cpp_int int_var__rbm_axi_err_increment;
        void rbm_axi_err_increment (const cpp_int  & l__val);
        cpp_int rbm_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > ssram_axi_err_increment_cpp_int_t;
        cpp_int int_var__ssram_axi_err_increment;
        void ssram_axi_err_increment (const cpp_int  & l__val);
        cpp_int ssram_axi_err_increment() const;
    
        typedef pu_cpp_int< 32 > ahb_axi_werr_increment_cpp_int_t;
        cpp_int int_var__ahb_axi_werr_increment;
        void ahb_axi_werr_increment (const cpp_int  & l__val);
        cpp_int ahb_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > emmc_pre_axi_werr_increment_cpp_int_t;
        cpp_int int_var__emmc_pre_axi_werr_increment;
        void emmc_pre_axi_werr_increment (const cpp_int  & l__val);
        cpp_int emmc_pre_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > fl_axi_werr_increment_cpp_int_t;
        cpp_int int_var__fl_axi_werr_increment;
        void fl_axi_werr_increment (const cpp_int  & l__val);
        cpp_int fl_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > ms_esec_mb_pre_axi_werr_increment_cpp_int_t;
        cpp_int int_var__ms_esec_mb_pre_axi_werr_increment;
        void ms_esec_mb_pre_axi_werr_increment (const cpp_int  & l__val);
        cpp_int ms_esec_mb_pre_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > msap_pre_axi_werr_increment_cpp_int_t;
        cpp_int int_var__msap_pre_axi_werr_increment;
        void msap_pre_axi_werr_increment (const cpp_int  & l__val);
        cpp_int msap_pre_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > nxms_a13_axi_werr_increment_cpp_int_t;
        cpp_int int_var__nxms_a13_axi_werr_increment;
        void nxms_a13_axi_werr_increment (const cpp_int  & l__val);
        cpp_int nxms_a13_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > rbm_axi_werr_increment_cpp_int_t;
        cpp_int int_var__rbm_axi_werr_increment;
        void rbm_axi_werr_increment (const cpp_int  & l__val);
        cpp_int rbm_axi_werr_increment() const;
    
        typedef pu_cpp_int< 32 > ssram_axi_werr_increment_cpp_int_t;
        cpp_int int_var__ssram_axi_werr_increment;
        void ssram_axi_werr_increment (const cpp_int  & l__val);
        cpp_int ssram_axi_werr_increment() const;
    
}; // cap_ms_csr_ms_axi_t
    
class cap_ms_csr_sta_nx_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_nx_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_nx_bist_t(string name = "cap_ms_csr_sta_nx_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_nx_bist_t();
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
    
        typedef pu_cpp_int< 1 > rtlgrp0_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrp0_bist_pass;
        void rtlgrp0_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrp0_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrp1_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrp1_bist_pass;
        void rtlgrp1_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrp1_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrp2_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrp2_bist_pass;
        void rtlgrp2_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrp2_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrp3_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrp3_bist_pass;
        void rtlgrp3_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrp3_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrpmd_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrpmd_bist_pass;
        void rtlgrpmd_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrpmd_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrpms_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrpms_bist_pass;
        void rtlgrpms_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrpms_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrppx_bist_pass_cpp_int_t;
        cpp_int int_var__rtlgrppx_bist_pass;
        void rtlgrppx_bist_pass (const cpp_int  & l__val);
        cpp_int rtlgrppx_bist_pass() const;
    
        typedef pu_cpp_int< 1 > rtlgrp0_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrp0_bist_fail;
        void rtlgrp0_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrp0_bist_fail() const;
    
        typedef pu_cpp_int< 1 > rtlgrp1_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrp1_bist_fail;
        void rtlgrp1_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrp1_bist_fail() const;
    
        typedef pu_cpp_int< 1 > rtlgrp2_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrp2_bist_fail;
        void rtlgrp2_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrp2_bist_fail() const;
    
        typedef pu_cpp_int< 1 > rtlgrp3_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrp3_bist_fail;
        void rtlgrp3_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrp3_bist_fail() const;
    
        typedef pu_cpp_int< 1 > rtlgrpmd_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrpmd_bist_fail;
        void rtlgrpmd_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrpmd_bist_fail() const;
    
        typedef pu_cpp_int< 1 > rtlgrpms_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrpms_bist_fail;
        void rtlgrpms_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrpms_bist_fail() const;
    
        typedef pu_cpp_int< 1 > rtlgrppx_bist_fail_cpp_int_t;
        cpp_int int_var__rtlgrppx_bist_fail;
        void rtlgrppx_bist_fail (const cpp_int  & l__val);
        cpp_int rtlgrppx_bist_fail() const;
    
}; // cap_ms_csr_sta_nx_bist_t
    
class cap_ms_csr_cfg_nx_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_nx_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_nx_bist_t(string name = "cap_ms_csr_cfg_nx_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_nx_bist_t();
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
    
        typedef pu_cpp_int< 1 > rtlgrp0_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrp0_bist_run;
        void rtlgrp0_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrp0_bist_run() const;
    
        typedef pu_cpp_int< 1 > rtlgrp1_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrp1_bist_run;
        void rtlgrp1_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrp1_bist_run() const;
    
        typedef pu_cpp_int< 1 > rtlgrp2_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrp2_bist_run;
        void rtlgrp2_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrp2_bist_run() const;
    
        typedef pu_cpp_int< 1 > rtlgrp3_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrp3_bist_run;
        void rtlgrp3_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrp3_bist_run() const;
    
        typedef pu_cpp_int< 1 > rtlgrpmd_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrpmd_bist_run;
        void rtlgrpmd_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrpmd_bist_run() const;
    
        typedef pu_cpp_int< 1 > rtlgrpms_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrpms_bist_run;
        void rtlgrpms_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrpms_bist_run() const;
    
        typedef pu_cpp_int< 1 > rtlgrppx_bist_run_cpp_int_t;
        cpp_int int_var__rtlgrppx_bist_run;
        void rtlgrppx_bist_run (const cpp_int  & l__val);
        cpp_int rtlgrppx_bist_run() const;
    
}; // cap_ms_csr_cfg_nx_bist_t
    
class cap_ms_csr_ms_cfg_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_cfg_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_cfg_debug_t(string name = "cap_ms_csr_ms_cfg_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_cfg_debug_t();
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
    
        typedef pu_cpp_int< 1 > port_enable_cpp_int_t;
        cpp_int int_var__port_enable;
        void port_enable (const cpp_int  & l__val);
        cpp_int port_enable() const;
    
        typedef pu_cpp_int< 2 > port_select_cpp_int_t;
        cpp_int int_var__port_select;
        void port_select (const cpp_int  & l__val);
        cpp_int port_select() const;
    
}; // cap_ms_csr_ms_cfg_debug_t
    
class cap_ms_csr_sta_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_rst_t(string name = "cap_ms_csr_sta_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_rst_t();
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
    
        typedef pu_cpp_int< 2 > apms_fatal_cpp_int_t;
        cpp_int int_var__apms_fatal;
        void apms_fatal (const cpp_int  & l__val);
        cpp_int apms_fatal() const;
    
}; // cap_ms_csr_sta_rst_t
    
class cap_ms_csr_sta_sid2uid_pending_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_sid2uid_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_sid2uid_pending_t(string name = "cap_ms_csr_sta_sid2uid_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_sid2uid_pending_t();
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
    
        typedef pu_cpp_int< 16 > r_nprp_srids_cpp_int_t;
        cpp_int int_var__r_nprp_srids;
        void r_nprp_srids (const cpp_int  & l__val);
        cpp_int r_nprp_srids() const;
    
        typedef pu_cpp_int< 16 > r_nprp_urids_cpp_int_t;
        cpp_int int_var__r_nprp_urids;
        void r_nprp_urids (const cpp_int  & l__val);
        cpp_int r_nprp_urids() const;
    
        typedef pu_cpp_int< 16 > r_prp_rids_cpp_int_t;
        cpp_int int_var__r_prp_rids;
        void r_prp_rids (const cpp_int  & l__val);
        cpp_int r_prp_rids() const;
    
        typedef pu_cpp_int< 16 > w_nprp_srids_cpp_int_t;
        cpp_int int_var__w_nprp_srids;
        void w_nprp_srids (const cpp_int  & l__val);
        cpp_int w_nprp_srids() const;
    
        typedef pu_cpp_int< 16 > w_nprp_urids_cpp_int_t;
        cpp_int int_var__w_nprp_urids;
        void w_nprp_urids (const cpp_int  & l__val);
        cpp_int w_nprp_urids() const;
    
        typedef pu_cpp_int< 16 > w_prp_rids_cpp_int_t;
        cpp_int int_var__w_prp_rids;
        void w_prp_rids (const cpp_int  & l__val);
        cpp_int w_prp_rids() const;
    
}; // cap_ms_csr_sta_sid2uid_pending_t
    
class cap_ms_csr_sta_sid2uid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_sid2uid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_sid2uid_t(string name = "cap_ms_csr_sta_sid2uid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_sid2uid_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
}; // cap_ms_csr_sta_sid2uid_t
    
class cap_ms_csr_cfg_sid2uid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_sid2uid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_sid2uid_t(string name = "cap_ms_csr_cfg_sid2uid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_sid2uid_t();
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
    
        typedef pu_cpp_int< 3 > leg_prp_cpp_int_t;
        cpp_int int_var__leg_prp;
        void leg_prp (const cpp_int  & l__val);
        cpp_int leg_prp() const;
    
        typedef pu_cpp_int< 8 > leg_same_id_grp_cpp_int_t;
        cpp_int int_var__leg_same_id_grp;
        void leg_same_id_grp (const cpp_int  & l__val);
        cpp_int leg_same_id_grp() const;
    
        typedef pu_cpp_int< 8 > leg_takes_same_id_cpp_int_t;
        cpp_int int_var__leg_takes_same_id;
        void leg_takes_same_id (const cpp_int  & l__val);
        cpp_int leg_takes_same_id() const;
    
        typedef pu_cpp_int< 1 > passthrough_cpp_int_t;
        cpp_int int_var__passthrough;
        void passthrough (const cpp_int  & l__val);
        cpp_int passthrough() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
        typedef pu_cpp_int< 3 > rd_stall_en_cpp_int_t;
        cpp_int int_var__rd_stall_en;
        void rd_stall_en (const cpp_int  & l__val);
        cpp_int rd_stall_en() const;
    
        typedef pu_cpp_int< 3 > wr_stall_en_cpp_int_t;
        cpp_int int_var__wr_stall_en;
        void wr_stall_en (const cpp_int  & l__val);
        cpp_int wr_stall_en() const;
    
}; // cap_ms_csr_cfg_sid2uid_t
    
class cap_ms_csr_cfg_uid2sid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_uid2sid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_uid2sid_t(string name = "cap_ms_csr_cfg_uid2sid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_uid2sid_t();
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
    
        typedef pu_cpp_int< 7 > id0_cpp_int_t;
        cpp_int int_var__id0;
        void id0 (const cpp_int  & l__val);
        cpp_int id0() const;
    
        typedef pu_cpp_int< 7 > id1_cpp_int_t;
        cpp_int int_var__id1;
        void id1 (const cpp_int  & l__val);
        cpp_int id1() const;
    
        typedef pu_cpp_int< 7 > id2_cpp_int_t;
        cpp_int int_var__id2;
        void id2 (const cpp_int  & l__val);
        cpp_int id2() const;
    
        typedef pu_cpp_int< 1 > passthrough_cpp_int_t;
        cpp_int int_var__passthrough;
        void passthrough (const cpp_int  & l__val);
        cpp_int passthrough() const;
    
}; // cap_ms_csr_cfg_uid2sid_t
    
class cap_ms_csr_cfg_cache_esec_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_cache_esec_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_cache_esec_t(string name = "cap_ms_csr_cfg_cache_esec_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_cache_esec_t();
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
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        cpp_int int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        cpp_int int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
}; // cap_ms_csr_cfg_cache_esec_t
    
class cap_ms_csr_cfg_msem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_msem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_msem_t(string name = "cap_ms_csr_cfg_msem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_msem_t();
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
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        cpp_int int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        cpp_int int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
}; // cap_ms_csr_cfg_msem_t
    
class cap_ms_csr_cfg_socket_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_socket_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_socket_t(string name = "cap_ms_csr_cfg_socket_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_socket_t();
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
    
        typedef pu_cpp_int< 1 > stall_on_same_id_prp_cpp_int_t;
        cpp_int int_var__stall_on_same_id_prp;
        void stall_on_same_id_prp (const cpp_int  & l__val);
        cpp_int stall_on_same_id_prp() const;
    
        typedef pu_cpp_int< 1 > stall_on_same_id_s18_cpp_int_t;
        cpp_int int_var__stall_on_same_id_s18;
        void stall_on_same_id_s18 (const cpp_int  & l__val);
        cpp_int stall_on_same_id_s18() const;
    
}; // cap_ms_csr_cfg_socket_t
    
class cap_ms_csr_cfg_ms_dap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ms_dap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ms_dap_t(string name = "cap_ms_csr_cfg_ms_dap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ms_dap_t();
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
    
        typedef pu_cpp_int< 2 > mux_sel_cpp_int_t;
        cpp_int int_var__mux_sel;
        void mux_sel (const cpp_int  & l__val);
        cpp_int mux_sel() const;
    
}; // cap_ms_csr_cfg_ms_dap_t
    
class cap_ms_csr_cfg_dci_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_dci_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_dci_t(string name = "cap_ms_csr_cfg_dci_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_dci_t();
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
    
        typedef pu_cpp_int< 16 > unused_GRANT_ovr_cpp_int_t;
        cpp_int int_var__unused_GRANT_ovr;
        void unused_GRANT_ovr (const cpp_int  & l__val);
        cpp_int unused_GRANT_ovr() const;
    
        typedef pu_cpp_int< 16 > unused_GRANT_val_cpp_int_t;
        cpp_int int_var__unused_GRANT_val;
        void unused_GRANT_val (const cpp_int  & l__val);
        cpp_int unused_GRANT_val() const;
    
}; // cap_ms_csr_cfg_dci_t
    
class cap_ms_csr_sta_tap_chipid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_tap_chipid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_tap_chipid_t(string name = "cap_ms_csr_sta_tap_chipid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_tap_chipid_t();
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
    
        typedef pu_cpp_int< 32 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_ms_csr_sta_tap_chipid_t
    
class cap_ms_csr_sta_mdio_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_mdio_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_mdio_status_t(string name = "cap_ms_csr_sta_mdio_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_mdio_status_t();
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
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
        typedef pu_cpp_int< 16 > read_data_cpp_int_t;
        cpp_int int_var__read_data;
        void read_data (const cpp_int  & l__val);
        cpp_int read_data() const;
    
}; // cap_ms_csr_sta_mdio_status_t
    
class cap_ms_csr_cfg_mdio_access_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_mdio_access_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_mdio_access_t(string name = "cap_ms_csr_cfg_mdio_access_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_mdio_access_t();
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
    
        typedef pu_cpp_int< 1 > request_cpp_int_t;
        cpp_int int_var__request;
        void request (const cpp_int  & l__val);
        cpp_int request() const;
    
        typedef pu_cpp_int< 1 > readwrite_cpp_int_t;
        cpp_int int_var__readwrite;
        void readwrite (const cpp_int  & l__val);
        cpp_int readwrite() const;
    
        typedef pu_cpp_int< 1 > addrdata_cpp_int_t;
        cpp_int int_var__addrdata;
        void addrdata (const cpp_int  & l__val);
        cpp_int addrdata() const;
    
        typedef pu_cpp_int< 16 > write_data_cpp_int_t;
        cpp_int int_var__write_data;
        void write_data (const cpp_int  & l__val);
        cpp_int write_data() const;
    
}; // cap_ms_csr_cfg_mdio_access_t
    
class cap_ms_csr_cfg_mdio_general_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_mdio_general_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_mdio_general_t(string name = "cap_ms_csr_cfg_mdio_general_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_mdio_general_t();
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
    
        typedef pu_cpp_int< 16 > clkdiv_cpp_int_t;
        cpp_int int_var__clkdiv;
        void clkdiv (const cpp_int  & l__val);
        cpp_int clkdiv() const;
    
        typedef pu_cpp_int< 1 > cls22_cpp_int_t;
        cpp_int int_var__cls22;
        void cls22 (const cpp_int  & l__val);
        cpp_int cls22() const;
    
        typedef pu_cpp_int< 1 > no_pre_cpp_int_t;
        cpp_int int_var__no_pre;
        void no_pre (const cpp_int  & l__val);
        cpp_int no_pre() const;
    
        typedef pu_cpp_int< 1 > read_inc_cpp_int_t;
        cpp_int int_var__read_inc;
        void read_inc (const cpp_int  & l__val);
        cpp_int read_inc() const;
    
        typedef pu_cpp_int< 5 > phy_addr_cpp_int_t;
        cpp_int int_var__phy_addr;
        void phy_addr (const cpp_int  & l__val);
        cpp_int phy_addr() const;
    
        typedef pu_cpp_int< 5 > dev_type_cpp_int_t;
        cpp_int int_var__dev_type;
        void dev_type (const cpp_int  & l__val);
        cpp_int dev_type() const;
    
}; // cap_ms_csr_cfg_mdio_general_t
    
class cap_ms_csr_sta_c2j_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_c2j_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_c2j_status_t(string name = "cap_ms_csr_sta_c2j_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_c2j_status_t();
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
    
        typedef pu_cpp_int< 1 > shift_done_cpp_int_t;
        cpp_int int_var__shift_done;
        void shift_done (const cpp_int  & l__val);
        cpp_int shift_done() const;
    
        typedef pu_cpp_int< 1 > reset_done_cpp_int_t;
        cpp_int int_var__reset_done;
        void reset_done (const cpp_int  & l__val);
        cpp_int reset_done() const;
    
        typedef pu_cpp_int< 64 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ms_csr_sta_c2j_status_t
    
class cap_ms_csr_cfg_c2j_tdi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_c2j_tdi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_c2j_tdi_t(string name = "cap_ms_csr_cfg_c2j_tdi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_c2j_tdi_t();
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
    
        typedef pu_cpp_int< 96 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_ms_csr_cfg_c2j_tdi_t
    
class cap_ms_csr_cfg_c2j_tms_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_c2j_tms_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_c2j_tms_t(string name = "cap_ms_csr_cfg_c2j_tms_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_c2j_tms_t();
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
    
        typedef pu_cpp_int< 96 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_ms_csr_cfg_c2j_tms_t
    
class cap_ms_csr_cfg_c2j_general_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_c2j_general_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_c2j_general_t(string name = "cap_ms_csr_cfg_c2j_general_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_c2j_general_t();
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
    
        typedef pu_cpp_int< 16 > clkdiv_cpp_int_t;
        cpp_int int_var__clkdiv;
        void clkdiv (const cpp_int  & l__val);
        cpp_int clkdiv() const;
    
        typedef pu_cpp_int< 1 > shift_enable_cpp_int_t;
        cpp_int int_var__shift_enable;
        void shift_enable (const cpp_int  & l__val);
        cpp_int shift_enable() const;
    
        typedef pu_cpp_int< 1 > reset_enable_cpp_int_t;
        cpp_int int_var__reset_enable;
        void reset_enable (const cpp_int  & l__val);
        cpp_int reset_enable() const;
    
        typedef pu_cpp_int< 7 > num_shift_cpp_int_t;
        cpp_int int_var__num_shift;
        void num_shift (const cpp_int  & l__val);
        cpp_int num_shift() const;
    
}; // cap_ms_csr_cfg_c2j_general_t
    
class cap_ms_csr_cfg_ssram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ssram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ssram_t(string name = "cap_ms_csr_cfg_ssram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ssram_t();
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
    
        typedef pu_cpp_int< 1 > secure_bit_cpp_int_t;
        cpp_int int_var__secure_bit;
        void secure_bit (const cpp_int  & l__val);
        cpp_int secure_bit() const;
    
}; // cap_ms_csr_cfg_ssram_t
    
class cap_ms_csr_sta_mem_init_bl2_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_mem_init_bl2_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_mem_init_bl2_ram_t(string name = "cap_ms_csr_sta_mem_init_bl2_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_mem_init_bl2_ram_t();
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
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
}; // cap_ms_csr_sta_mem_init_bl2_ram_t
    
class cap_ms_csr_cfg_ecc_disable_bl2_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ecc_disable_bl2_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ecc_disable_bl2_ram_t(string name = "cap_ms_csr_cfg_ecc_disable_bl2_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ecc_disable_bl2_ram_t();
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
    
        typedef pu_cpp_int< 1 > cor_cpp_int_t;
        cpp_int int_var__cor;
        void cor (const cpp_int  & l__val);
        cpp_int cor() const;
    
        typedef pu_cpp_int< 1 > det_cpp_int_t;
        cpp_int int_var__det;
        void det (const cpp_int  & l__val);
        cpp_int det() const;
    
}; // cap_ms_csr_cfg_ecc_disable_bl2_ram_t
    
class cap_ms_csr_sta_ecc_bl2_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_ecc_bl2_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_ecc_bl2_ram_t(string name = "cap_ms_csr_sta_ecc_bl2_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_ecc_bl2_ram_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_ms_csr_sta_ecc_bl2_ram_t
    
class cap_ms_csr_sta_bist_ssram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_bist_ssram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_bist_ssram_t(string name = "cap_ms_csr_sta_bist_ssram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_bist_ssram_t();
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
    
        typedef pu_cpp_int< 1 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
        typedef pu_cpp_int< 1 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
}; // cap_ms_csr_sta_bist_ssram_t
    
class cap_ms_csr_cfg_bist_ssram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_bist_ssram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_bist_ssram_t(string name = "cap_ms_csr_cfg_bist_ssram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_bist_ssram_t();
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
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_ms_csr_cfg_bist_ssram_t
    
class cap_ms_csr_cfg_qspi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_qspi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_qspi_t(string name = "cap_ms_csr_cfg_qspi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_qspi_t();
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
    
        typedef pu_cpp_int< 1 > ovr_cpp_int_t;
        cpp_int int_var__ovr;
        void ovr (const cpp_int  & l__val);
        cpp_int ovr() const;
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        cpp_int int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
}; // cap_ms_csr_cfg_qspi_t
    
class cap_ms_csr_sta_ecc_qspi_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_ecc_qspi_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_ecc_qspi_ram_t(string name = "cap_ms_csr_sta_ecc_qspi_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_ecc_qspi_ram_t();
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
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_ms_csr_sta_ecc_qspi_ram_t
    
class cap_ms_csr_cfg_ecc_disable_qspi_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ecc_disable_qspi_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ecc_disable_qspi_ram_t(string name = "cap_ms_csr_cfg_ecc_disable_qspi_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ecc_disable_qspi_ram_t();
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
    
        typedef pu_cpp_int< 1 > cor_cpp_int_t;
        cpp_int int_var__cor;
        void cor (const cpp_int  & l__val);
        cpp_int cor() const;
    
        typedef pu_cpp_int< 1 > det_cpp_int_t;
        cpp_int int_var__det;
        void det (const cpp_int  & l__val);
        cpp_int det() const;
    
}; // cap_ms_csr_cfg_ecc_disable_qspi_ram_t
    
class cap_ms_csr_sta_ds16_die_id_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_ds16_die_id_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_ds16_die_id_t(string name = "cap_ms_csr_sta_ds16_die_id_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_ds16_die_id_t();
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
    
        typedef pu_cpp_int< 128 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_ms_csr_sta_ds16_die_id_t
    
class cap_ms_csr_sta_ds16_efuse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_ds16_efuse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_ds16_efuse_t(string name = "cap_ms_csr_sta_ds16_efuse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_ds16_efuse_t();
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
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > ack_cpp_int_t;
        cpp_int int_var__ack;
        void ack (const cpp_int  & l__val);
        cpp_int ack() const;
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
        typedef pu_cpp_int< 1 > init_read_done_cpp_int_t;
        cpp_int int_var__init_read_done;
        void init_read_done (const cpp_int  & l__val);
        cpp_int init_read_done() const;
    
        typedef pu_cpp_int< 1 > cpu_read_done_cpp_int_t;
        cpp_int int_var__cpu_read_done;
        void cpu_read_done (const cpp_int  & l__val);
        cpp_int cpu_read_done() const;
    
        typedef pu_cpp_int< 128 > read_val_cpp_int_t;
        cpp_int int_var__read_val;
        void read_val (const cpp_int  & l__val);
        cpp_int read_val() const;
    
}; // cap_ms_csr_sta_ds16_efuse_t
    
class cap_ms_csr_cfg_ds16_efuse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ds16_efuse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ds16_efuse_t(string name = "cap_ms_csr_cfg_ds16_efuse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ds16_efuse_t();
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
    
        typedef pu_cpp_int< 1 > reset_l_cpp_int_t;
        cpp_int int_var__reset_l;
        void reset_l (const cpp_int  & l__val);
        cpp_int reset_l() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > read_cpp_int_t;
        cpp_int int_var__read;
        void read (const cpp_int  & l__val);
        cpp_int read() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 2 > timeout_sel_cpp_int_t;
        cpp_int int_var__timeout_sel;
        void timeout_sel (const cpp_int  & l__val);
        cpp_int timeout_sel() const;
    
        typedef pu_cpp_int< 1 > override_core_enable_cpp_int_t;
        cpp_int int_var__override_core_enable;
        void override_core_enable (const cpp_int  & l__val);
        cpp_int override_core_enable() const;
    
        typedef pu_cpp_int< 1 > override_core_req_cpp_int_t;
        cpp_int int_var__override_core_req;
        void override_core_req (const cpp_int  & l__val);
        cpp_int override_core_req() const;
    
        typedef pu_cpp_int< 1 > override_core_read_cpp_int_t;
        cpp_int int_var__override_core_read;
        void override_core_read (const cpp_int  & l__val);
        cpp_int override_core_read() const;
    
        typedef pu_cpp_int< 1 > override_core_rotate_cpp_int_t;
        cpp_int int_var__override_core_rotate;
        void override_core_rotate (const cpp_int  & l__val);
        cpp_int override_core_rotate() const;
    
        typedef pu_cpp_int< 1 > override_core_rotate_load_cpp_int_t;
        cpp_int int_var__override_core_rotate_load;
        void override_core_rotate_load (const cpp_int  & l__val);
        cpp_int override_core_rotate_load() const;
    
        typedef pu_cpp_int< 1 > override_core_reset_l_cpp_int_t;
        cpp_int int_var__override_core_reset_l;
        void override_core_reset_l (const cpp_int  & l__val);
        cpp_int override_core_reset_l() const;
    
        typedef pu_cpp_int< 1 > override_load_data_out_cpp_int_t;
        cpp_int int_var__override_load_data_out;
        void override_load_data_out (const cpp_int  & l__val);
        cpp_int override_load_data_out() const;
    
}; // cap_ms_csr_cfg_ds16_efuse_t
    
class cap_ms_csr_sta_pk_efuse_val_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_pk_efuse_val_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_pk_efuse_val_t(string name = "cap_ms_csr_sta_pk_efuse_val_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_pk_efuse_val_t();
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
    
        typedef pu_cpp_int< 512 > pk_cpp_int_t;
        cpp_int int_var__pk;
        void pk (const cpp_int  & l__val);
        cpp_int pk() const;
    
}; // cap_ms_csr_sta_pk_efuse_val_t
    
class cap_ms_csr_sta_pk_efuse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_pk_efuse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_pk_efuse_t(string name = "cap_ms_csr_sta_pk_efuse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_pk_efuse_t();
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
    
        typedef pu_cpp_int< 1 > ready_cpp_int_t;
        cpp_int int_var__ready;
        void ready (const cpp_int  & l__val);
        cpp_int ready() const;
    
        typedef pu_cpp_int< 1 > ack_cpp_int_t;
        cpp_int int_var__ack;
        void ack (const cpp_int  & l__val);
        cpp_int ack() const;
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
        typedef pu_cpp_int< 1 > init_read_done_cpp_int_t;
        cpp_int int_var__init_read_done;
        void init_read_done (const cpp_int  & l__val);
        cpp_int init_read_done() const;
    
        typedef pu_cpp_int< 1 > cpu_read_done_cpp_int_t;
        cpp_int int_var__cpu_read_done;
        void cpu_read_done (const cpp_int  & l__val);
        cpp_int cpu_read_done() const;
    
        typedef pu_cpp_int< 1 > cpu_burn_done_cpp_int_t;
        cpp_int int_var__cpu_burn_done;
        void cpu_burn_done (const cpp_int  & l__val);
        cpp_int cpu_burn_done() const;
    
        typedef pu_cpp_int< 1 > burn_imm_done_cpp_int_t;
        cpp_int int_var__burn_imm_done;
        void burn_imm_done (const cpp_int  & l__val);
        cpp_int burn_imm_done() const;
    
        typedef pu_cpp_int< 128 > read_val_cpp_int_t;
        cpp_int int_var__read_val;
        void read_val (const cpp_int  & l__val);
        cpp_int read_val() const;
    
}; // cap_ms_csr_sta_pk_efuse_t
    
class cap_ms_csr_cfg_pk_efuse_override_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_pk_efuse_override_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_pk_efuse_override_t(string name = "cap_ms_csr_cfg_pk_efuse_override_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_pk_efuse_override_t();
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
    
        typedef pu_cpp_int< 1 > core_enable_cpp_int_t;
        cpp_int int_var__core_enable;
        void core_enable (const cpp_int  & l__val);
        cpp_int core_enable() const;
    
        typedef pu_cpp_int< 1 > core_req_cpp_int_t;
        cpp_int int_var__core_req;
        void core_req (const cpp_int  & l__val);
        cpp_int core_req() const;
    
        typedef pu_cpp_int< 1 > core_read_cpp_int_t;
        cpp_int int_var__core_read;
        void core_read (const cpp_int  & l__val);
        cpp_int core_read() const;
    
        typedef pu_cpp_int< 1 > core_burn_cpp_int_t;
        cpp_int int_var__core_burn;
        void core_burn (const cpp_int  & l__val);
        cpp_int core_burn() const;
    
        typedef pu_cpp_int< 1 > core_shift_cpp_int_t;
        cpp_int int_var__core_shift;
        void core_shift (const cpp_int  & l__val);
        cpp_int core_shift() const;
    
        typedef pu_cpp_int< 1 > core_sin_cpp_int_t;
        cpp_int int_var__core_sin;
        void core_sin (const cpp_int  & l__val);
        cpp_int core_sin() const;
    
        typedef pu_cpp_int< 1 > core_rotate_cpp_int_t;
        cpp_int int_var__core_rotate;
        void core_rotate (const cpp_int  & l__val);
        cpp_int core_rotate() const;
    
        typedef pu_cpp_int< 1 > core_rotate_load_cpp_int_t;
        cpp_int int_var__core_rotate_load;
        void core_rotate_load (const cpp_int  & l__val);
        cpp_int core_rotate_load() const;
    
        typedef pu_cpp_int< 1 > core_reset_l_cpp_int_t;
        cpp_int int_var__core_reset_l;
        void core_reset_l (const cpp_int  & l__val);
        cpp_int core_reset_l() const;
    
        typedef pu_cpp_int< 1 > load_data_out_cpp_int_t;
        cpp_int int_var__load_data_out;
        void load_data_out (const cpp_int  & l__val);
        cpp_int load_data_out() const;
    
        typedef pu_cpp_int< 1 > fuse_reset_cpp_int_t;
        cpp_int int_var__fuse_reset;
        void fuse_reset (const cpp_int  & l__val);
        cpp_int fuse_reset() const;
    
}; // cap_ms_csr_cfg_pk_efuse_override_t
    
class cap_ms_csr_cfg_pk_j2c_efuse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_pk_j2c_efuse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_pk_j2c_efuse_t(string name = "cap_ms_csr_cfg_pk_j2c_efuse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_pk_j2c_efuse_t();
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
    
        typedef pu_cpp_int< 1 > burn_cpp_int_t;
        cpp_int int_var__burn;
        void burn (const cpp_int  & l__val);
        cpp_int burn() const;
    
        typedef pu_cpp_int< 1 > read_cpp_int_t;
        cpp_int int_var__read;
        void read (const cpp_int  & l__val);
        cpp_int read() const;
    
        typedef pu_cpp_int< 5 > addr_idx_cpp_int_t;
        cpp_int int_var__addr_idx;
        void addr_idx (const cpp_int  & l__val);
        cpp_int addr_idx() const;
    
        typedef pu_cpp_int< 2 > timeout_sel_cpp_int_t;
        cpp_int int_var__timeout_sel;
        void timeout_sel (const cpp_int  & l__val);
        cpp_int timeout_sel() const;
    
}; // cap_ms_csr_cfg_pk_j2c_efuse_t
    
class cap_ms_csr_cfg_pk_sw_efuse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_pk_sw_efuse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_pk_sw_efuse_t(string name = "cap_ms_csr_cfg_pk_sw_efuse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_pk_sw_efuse_t();
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
    
        typedef pu_cpp_int< 1 > burn_cpp_int_t;
        cpp_int int_var__burn;
        void burn (const cpp_int  & l__val);
        cpp_int burn() const;
    
        typedef pu_cpp_int< 1 > read_cpp_int_t;
        cpp_int int_var__read;
        void read (const cpp_int  & l__val);
        cpp_int read() const;
    
        typedef pu_cpp_int< 5 > addr_idx_cpp_int_t;
        cpp_int int_var__addr_idx;
        void addr_idx (const cpp_int  & l__val);
        cpp_int addr_idx() const;
    
}; // cap_ms_csr_cfg_pk_sw_efuse_t
    
class cap_ms_csr_cfg_pk_efuse_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_pk_efuse_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_pk_efuse_data_t(string name = "cap_ms_csr_cfg_pk_efuse_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_pk_efuse_data_t();
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
    
        typedef pu_cpp_int< 128 > in_cpp_int_t;
        cpp_int int_var__in;
        void in (const cpp_int  & l__val);
        cpp_int in() const;
    
}; // cap_ms_csr_cfg_pk_efuse_data_t
    
class cap_ms_csr_cfg_esec_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_esec_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_esec_t(string name = "cap_ms_csr_cfg_esec_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_esec_t();
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
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
        typedef pu_cpp_int< 1 > nRstPOReSec_cpp_int_t;
        cpp_int int_var__nRstPOReSec;
        void nRstPOReSec (const cpp_int  & l__val);
        cpp_int nRstPOReSec() const;
    
        typedef pu_cpp_int< 1 > nRstSyseSec_cpp_int_t;
        cpp_int int_var__nRstSyseSec;
        void nRstSyseSec (const cpp_int  & l__val);
        cpp_int nRstSyseSec() const;
    
        typedef pu_cpp_int< 1 > dummy_cpp_int_t;
        cpp_int int_var__dummy;
        void dummy (const cpp_int  & l__val);
        cpp_int dummy() const;
    
        typedef pu_cpp_int< 16 > tamper_rst_indx_cpp_int_t;
        cpp_int int_var__tamper_rst_indx;
        void tamper_rst_indx (const cpp_int  & l__val);
        cpp_int tamper_rst_indx() const;
    
}; // cap_ms_csr_cfg_esec_t
    
class cap_ms_csr_cfg_arm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_arm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_arm_t(string name = "cap_ms_csr_cfg_arm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_arm_t();
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
    
        typedef pu_cpp_int< 1 > cfg_override_cpp_int_t;
        cpp_int int_var__cfg_override;
        void cfg_override (const cpp_int  & l__val);
        cpp_int cfg_override() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETPOR_cpp_int_t;
        cpp_int int_var__ov_nRESETPOR;
        void ov_nRESETPOR (const cpp_int  & l__val);
        cpp_int ov_nRESETPOR() const;
    
        typedef pu_cpp_int< 1 > ov_nL2RESET_cpp_int_t;
        cpp_int int_var__ov_nL2RESET;
        void ov_nL2RESET (const cpp_int  & l__val);
        cpp_int ov_nL2RESET() const;
    
        typedef pu_cpp_int< 1 > ov_filler_cpp_int_t;
        cpp_int int_var__ov_filler;
        void ov_filler (const cpp_int  & l__val);
        cpp_int ov_filler() const;
    
        typedef pu_cpp_int< 4 > ov_nCORERESET_cpp_int_t;
        cpp_int int_var__ov_nCORERESET;
        void ov_nCORERESET (const cpp_int  & l__val);
        cpp_int ov_nCORERESET() const;
    
        typedef pu_cpp_int< 4 > ov_nCPUPORESET_cpp_int_t;
        cpp_int int_var__ov_nCPUPORESET;
        void ov_nCPUPORESET (const cpp_int  & l__val);
        cpp_int ov_nCPUPORESET() const;
    
        typedef pu_cpp_int< 4 > ov_VINITHI_cpp_int_t;
        cpp_int int_var__ov_VINITHI;
        void ov_VINITHI (const cpp_int  & l__val);
        cpp_int ov_VINITHI() const;
    
        typedef pu_cpp_int< 1 > ov_nPRESETDBG_cpp_int_t;
        cpp_int int_var__ov_nPRESETDBG;
        void ov_nPRESETDBG (const cpp_int  & l__val);
        cpp_int ov_nPRESETDBG() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETACE_cpp_int_t;
        cpp_int int_var__ov_nRESETACE;
        void ov_nRESETACE (const cpp_int  & l__val);
        cpp_int ov_nRESETACE() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETACP_cpp_int_t;
        cpp_int int_var__ov_nRESETACP;
        void ov_nRESETACP (const cpp_int  & l__val);
        cpp_int ov_nRESETACP() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETATB_cpp_int_t;
        cpp_int int_var__ov_nRESETATB;
        void ov_nRESETATB (const cpp_int  & l__val);
        cpp_int ov_nRESETATB() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETCNT_cpp_int_t;
        cpp_int int_var__ov_nRESETCNT;
        void ov_nRESETCNT (const cpp_int  & l__val);
        cpp_int ov_nRESETCNT() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETEDBG_cpp_int_t;
        cpp_int int_var__ov_nRESETEDBG;
        void ov_nRESETEDBG (const cpp_int  & l__val);
        cpp_int ov_nRESETEDBG() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETGIC_cpp_int_t;
        cpp_int int_var__ov_nRESETGIC;
        void ov_nRESETGIC (const cpp_int  & l__val);
        cpp_int ov_nRESETGIC() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETRDL_cpp_int_t;
        cpp_int int_var__ov_nRESETRDL;
        void ov_nRESETRDL (const cpp_int  & l__val);
        cpp_int ov_nRESETRDL() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETICDT_cpp_int_t;
        cpp_int int_var__ov_nRESETICDT;
        void ov_nRESETICDT (const cpp_int  & l__val);
        cpp_int ov_nRESETICDT() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETICCT_cpp_int_t;
        cpp_int int_var__ov_nRESETICCT;
        void ov_nRESETICCT (const cpp_int  & l__val);
        cpp_int ov_nRESETICCT() const;
    
        typedef pu_cpp_int< 1 > ov_nRESETTS_cpp_int_t;
        cpp_int int_var__ov_nRESETTS;
        void ov_nRESETTS (const cpp_int  & l__val);
        cpp_int ov_nRESETTS() const;
    
}; // cap_ms_csr_cfg_arm_t
    
class cap_ms_csr_sta_elam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_elam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_elam_t(string name = "cap_ms_csr_sta_elam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_elam_t();
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
    
        typedef pu_cpp_int< 4 > state_cpp_int_t;
        cpp_int int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
        typedef pu_cpp_int< 6 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
        typedef pu_cpp_int< 1 > triggered_cpp_int_t;
        cpp_int int_var__triggered;
        void triggered (const cpp_int  & l__val);
        cpp_int triggered() const;
    
        typedef pu_cpp_int< 1 > capture_stopped_cpp_int_t;
        cpp_int int_var__capture_stopped;
        void capture_stopped (const cpp_int  & l__val);
        cpp_int capture_stopped() const;
    
        typedef pu_cpp_int< 13 > num_sample_cpp_int_t;
        cpp_int int_var__num_sample;
        void num_sample (const cpp_int  & l__val);
        cpp_int num_sample() const;
    
        typedef pu_cpp_int< 13 > num_post_sample_cpp_int_t;
        cpp_int int_var__num_post_sample;
        void num_post_sample (const cpp_int  & l__val);
        cpp_int num_post_sample() const;
    
}; // cap_ms_csr_sta_elam_t
    
class cap_ms_csr_cfg_elam_ext_trig_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_elam_ext_trig_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_elam_ext_trig_t(string name = "cap_ms_csr_cfg_elam_ext_trig_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_elam_ext_trig_t();
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
    
        typedef pu_cpp_int< 4 > next_state_cpp_int_t;
        cpp_int int_var__next_state;
        void next_state (const cpp_int  & l__val);
        cpp_int next_state() const;
    
}; // cap_ms_csr_cfg_elam_ext_trig_t
    
class cap_ms_csr_cfg_elam_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_elam_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_elam_control_t(string name = "cap_ms_csr_cfg_elam_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_elam_control_t();
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
    
        typedef pu_cpp_int< 16 > control_out_cpp_int_t;
        cpp_int int_var__control_out;
        void control_out (const cpp_int  & l__val);
        cpp_int control_out() const;
    
}; // cap_ms_csr_cfg_elam_control_t
    
class cap_ms_csr_cfg_elam_breakout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_elam_breakout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_elam_breakout_t(string name = "cap_ms_csr_cfg_elam_breakout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_elam_breakout_t();
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
    
        typedef pu_cpp_int< 2 > sel0_cpp_int_t;
        cpp_int int_var__sel0;
        void sel0 (const cpp_int  & l__val);
        cpp_int sel0() const;
    
        typedef pu_cpp_int< 2 > sel1_cpp_int_t;
        cpp_int int_var__sel1;
        void sel1 (const cpp_int  & l__val);
        cpp_int sel1() const;
    
        typedef pu_cpp_int< 2 > sel2_cpp_int_t;
        cpp_int int_var__sel2;
        void sel2 (const cpp_int  & l__val);
        cpp_int sel2() const;
    
        typedef pu_cpp_int< 2 > sel3_cpp_int_t;
        cpp_int int_var__sel3;
        void sel3 (const cpp_int  & l__val);
        cpp_int sel3() const;
    
        typedef pu_cpp_int< 4 > op_cpp_int_t;
        cpp_int int_var__op;
        void op (const cpp_int  & l__val);
        cpp_int op() const;
    
}; // cap_ms_csr_cfg_elam_breakout_t
    
class cap_ms_csr_cfg_elam_general_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_elam_general_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_elam_general_t(string name = "cap_ms_csr_cfg_elam_general_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_elam_general_t();
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
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        cpp_int int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
        typedef pu_cpp_int< 1 > arm_cpp_int_t;
        cpp_int int_var__arm;
        void arm (const cpp_int  & l__val);
        cpp_int arm() const;
    
        typedef pu_cpp_int< 1 > stop_sample_cpp_int_t;
        cpp_int int_var__stop_sample;
        void stop_sample (const cpp_int  & l__val);
        cpp_int stop_sample() const;
    
        typedef pu_cpp_int< 13 > num_post_sample_cpp_int_t;
        cpp_int int_var__num_post_sample;
        void num_post_sample (const cpp_int  & l__val);
        cpp_int num_post_sample() const;
    
        typedef pu_cpp_int< 1 > breakout_enable_cpp_int_t;
        cpp_int int_var__breakout_enable;
        void breakout_enable (const cpp_int  & l__val);
        cpp_int breakout_enable() const;
    
}; // cap_ms_csr_cfg_elam_general_t
    
class cap_ms_csr_cfg_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_hbm_t(string name = "cap_ms_csr_cfg_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_hbm_t();
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
    
        typedef pu_cpp_int< 1 > HBM_POWER_ON_RESET_L_cpp_int_t;
        cpp_int int_var__HBM_POWER_ON_RESET_L;
        void HBM_POWER_ON_RESET_L (const cpp_int  & l__val);
        cpp_int HBM_POWER_ON_RESET_L() const;
    
        typedef pu_cpp_int< 1 > POWER_ON_RESET_L_cpp_int_t;
        cpp_int int_var__POWER_ON_RESET_L;
        void POWER_ON_RESET_L (const cpp_int  & l__val);
        cpp_int POWER_ON_RESET_L() const;
    
}; // cap_ms_csr_cfg_hbm_t
    
class cap_ms_csr_ms_prp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_prp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_prp_t(string name = "cap_ms_csr_ms_prp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_prp_t();
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
    
        typedef pu_cpp_int< 32 > read_error1_cpp_int_t;
        cpp_int int_var__read_error1;
        void read_error1 (const cpp_int  & l__val);
        cpp_int read_error1() const;
    
        typedef pu_cpp_int< 32 > security_error1_cpp_int_t;
        cpp_int int_var__security_error1;
        void security_error1 (const cpp_int  & l__val);
        cpp_int security_error1() const;
    
        typedef pu_cpp_int< 32 > decode_error1_cpp_int_t;
        cpp_int int_var__decode_error1;
        void decode_error1 (const cpp_int  & l__val);
        cpp_int decode_error1() const;
    
        typedef pu_cpp_int< 16 > read_cnt1_cpp_int_t;
        cpp_int int_var__read_cnt1;
        void read_cnt1 (const cpp_int  & l__val);
        cpp_int read_cnt1() const;
    
        typedef pu_cpp_int< 16 > write_cnt1_cpp_int_t;
        cpp_int int_var__write_cnt1;
        void write_cnt1 (const cpp_int  & l__val);
        cpp_int write_cnt1() const;
    
        typedef pu_cpp_int< 16 > ack_cnt1_cpp_int_t;
        cpp_int int_var__ack_cnt1;
        void ack_cnt1 (const cpp_int  & l__val);
        cpp_int ack_cnt1() const;
    
        typedef pu_cpp_int< 16 > byte_read_error1_cpp_int_t;
        cpp_int int_var__byte_read_error1;
        void byte_read_error1 (const cpp_int  & l__val);
        cpp_int byte_read_error1() const;
    
        typedef pu_cpp_int< 16 > byte_write_error1_cpp_int_t;
        cpp_int int_var__byte_write_error1;
        void byte_write_error1 (const cpp_int  & l__val);
        cpp_int byte_write_error1() const;
    
        typedef pu_cpp_int< 16 > ack_timeout_error1_cpp_int_t;
        cpp_int int_var__ack_timeout_error1;
        void ack_timeout_error1 (const cpp_int  & l__val);
        cpp_int ack_timeout_error1() const;
    
        typedef pu_cpp_int< 16 > stall_timeout_error1_cpp_int_t;
        cpp_int int_var__stall_timeout_error1;
        void stall_timeout_error1 (const cpp_int  & l__val);
        cpp_int stall_timeout_error1() const;
    
        typedef pu_cpp_int< 32 > read_error2_cpp_int_t;
        cpp_int int_var__read_error2;
        void read_error2 (const cpp_int  & l__val);
        cpp_int read_error2() const;
    
        typedef pu_cpp_int< 32 > security_error2_cpp_int_t;
        cpp_int int_var__security_error2;
        void security_error2 (const cpp_int  & l__val);
        cpp_int security_error2() const;
    
        typedef pu_cpp_int< 32 > decode_error2_cpp_int_t;
        cpp_int int_var__decode_error2;
        void decode_error2 (const cpp_int  & l__val);
        cpp_int decode_error2() const;
    
        typedef pu_cpp_int< 16 > read_cnt2_cpp_int_t;
        cpp_int int_var__read_cnt2;
        void read_cnt2 (const cpp_int  & l__val);
        cpp_int read_cnt2() const;
    
        typedef pu_cpp_int< 16 > write_cnt2_cpp_int_t;
        cpp_int int_var__write_cnt2;
        void write_cnt2 (const cpp_int  & l__val);
        cpp_int write_cnt2() const;
    
        typedef pu_cpp_int< 16 > ack_cnt2_cpp_int_t;
        cpp_int int_var__ack_cnt2;
        void ack_cnt2 (const cpp_int  & l__val);
        cpp_int ack_cnt2() const;
    
        typedef pu_cpp_int< 16 > byte_read_error2_cpp_int_t;
        cpp_int int_var__byte_read_error2;
        void byte_read_error2 (const cpp_int  & l__val);
        cpp_int byte_read_error2() const;
    
        typedef pu_cpp_int< 16 > byte_write_error2_cpp_int_t;
        cpp_int int_var__byte_write_error2;
        void byte_write_error2 (const cpp_int  & l__val);
        cpp_int byte_write_error2() const;
    
        typedef pu_cpp_int< 16 > ack_timeout_error2_cpp_int_t;
        cpp_int int_var__ack_timeout_error2;
        void ack_timeout_error2 (const cpp_int  & l__val);
        cpp_int ack_timeout_error2() const;
    
        typedef pu_cpp_int< 16 > stall_timeout_error2_cpp_int_t;
        cpp_int int_var__stall_timeout_error2;
        void stall_timeout_error2 (const cpp_int  & l__val);
        cpp_int stall_timeout_error2() const;
    
        typedef pu_cpp_int< 32 > read_error3_cpp_int_t;
        cpp_int int_var__read_error3;
        void read_error3 (const cpp_int  & l__val);
        cpp_int read_error3() const;
    
        typedef pu_cpp_int< 32 > security_error3_cpp_int_t;
        cpp_int int_var__security_error3;
        void security_error3 (const cpp_int  & l__val);
        cpp_int security_error3() const;
    
        typedef pu_cpp_int< 32 > decode_error3_cpp_int_t;
        cpp_int int_var__decode_error3;
        void decode_error3 (const cpp_int  & l__val);
        cpp_int decode_error3() const;
    
        typedef pu_cpp_int< 16 > read_cnt3_cpp_int_t;
        cpp_int int_var__read_cnt3;
        void read_cnt3 (const cpp_int  & l__val);
        cpp_int read_cnt3() const;
    
        typedef pu_cpp_int< 16 > write_cnt3_cpp_int_t;
        cpp_int int_var__write_cnt3;
        void write_cnt3 (const cpp_int  & l__val);
        cpp_int write_cnt3() const;
    
        typedef pu_cpp_int< 16 > ack_cnt3_cpp_int_t;
        cpp_int int_var__ack_cnt3;
        void ack_cnt3 (const cpp_int  & l__val);
        cpp_int ack_cnt3() const;
    
        typedef pu_cpp_int< 16 > byte_read_error3_cpp_int_t;
        cpp_int int_var__byte_read_error3;
        void byte_read_error3 (const cpp_int  & l__val);
        cpp_int byte_read_error3() const;
    
        typedef pu_cpp_int< 16 > byte_write_error3_cpp_int_t;
        cpp_int int_var__byte_write_error3;
        void byte_write_error3 (const cpp_int  & l__val);
        cpp_int byte_write_error3() const;
    
        typedef pu_cpp_int< 16 > ack_timeout_error3_cpp_int_t;
        cpp_int int_var__ack_timeout_error3;
        void ack_timeout_error3 (const cpp_int  & l__val);
        cpp_int ack_timeout_error3() const;
    
        typedef pu_cpp_int< 16 > stall_timeout_error3_cpp_int_t;
        cpp_int int_var__stall_timeout_error3;
        void stall_timeout_error3 (const cpp_int  & l__val);
        cpp_int stall_timeout_error3() const;
    
        typedef pu_cpp_int< 32 > read_error4_cpp_int_t;
        cpp_int int_var__read_error4;
        void read_error4 (const cpp_int  & l__val);
        cpp_int read_error4() const;
    
        typedef pu_cpp_int< 32 > security_error4_cpp_int_t;
        cpp_int int_var__security_error4;
        void security_error4 (const cpp_int  & l__val);
        cpp_int security_error4() const;
    
        typedef pu_cpp_int< 32 > decode_error4_cpp_int_t;
        cpp_int int_var__decode_error4;
        void decode_error4 (const cpp_int  & l__val);
        cpp_int decode_error4() const;
    
        typedef pu_cpp_int< 16 > read_cnt4_cpp_int_t;
        cpp_int int_var__read_cnt4;
        void read_cnt4 (const cpp_int  & l__val);
        cpp_int read_cnt4() const;
    
        typedef pu_cpp_int< 16 > write_cnt4_cpp_int_t;
        cpp_int int_var__write_cnt4;
        void write_cnt4 (const cpp_int  & l__val);
        cpp_int write_cnt4() const;
    
        typedef pu_cpp_int< 16 > ack_cnt4_cpp_int_t;
        cpp_int int_var__ack_cnt4;
        void ack_cnt4 (const cpp_int  & l__val);
        cpp_int ack_cnt4() const;
    
        typedef pu_cpp_int< 16 > byte_read_error4_cpp_int_t;
        cpp_int int_var__byte_read_error4;
        void byte_read_error4 (const cpp_int  & l__val);
        cpp_int byte_read_error4() const;
    
        typedef pu_cpp_int< 16 > byte_write_error4_cpp_int_t;
        cpp_int int_var__byte_write_error4;
        void byte_write_error4 (const cpp_int  & l__val);
        cpp_int byte_write_error4() const;
    
        typedef pu_cpp_int< 16 > ack_timeout_error4_cpp_int_t;
        cpp_int int_var__ack_timeout_error4;
        void ack_timeout_error4 (const cpp_int  & l__val);
        cpp_int ack_timeout_error4() const;
    
        typedef pu_cpp_int< 16 > stall_timeout_error4_cpp_int_t;
        cpp_int int_var__stall_timeout_error4;
        void stall_timeout_error4 (const cpp_int  & l__val);
        cpp_int stall_timeout_error4() const;
    
        typedef pu_cpp_int< 32 > read_error5_cpp_int_t;
        cpp_int int_var__read_error5;
        void read_error5 (const cpp_int  & l__val);
        cpp_int read_error5() const;
    
        typedef pu_cpp_int< 32 > security_error5_cpp_int_t;
        cpp_int int_var__security_error5;
        void security_error5 (const cpp_int  & l__val);
        cpp_int security_error5() const;
    
        typedef pu_cpp_int< 32 > decode_error5_cpp_int_t;
        cpp_int int_var__decode_error5;
        void decode_error5 (const cpp_int  & l__val);
        cpp_int decode_error5() const;
    
        typedef pu_cpp_int< 16 > read_cnt5_cpp_int_t;
        cpp_int int_var__read_cnt5;
        void read_cnt5 (const cpp_int  & l__val);
        cpp_int read_cnt5() const;
    
        typedef pu_cpp_int< 16 > write_cnt5_cpp_int_t;
        cpp_int int_var__write_cnt5;
        void write_cnt5 (const cpp_int  & l__val);
        cpp_int write_cnt5() const;
    
        typedef pu_cpp_int< 16 > ack_cnt5_cpp_int_t;
        cpp_int int_var__ack_cnt5;
        void ack_cnt5 (const cpp_int  & l__val);
        cpp_int ack_cnt5() const;
    
        typedef pu_cpp_int< 16 > byte_read_error5_cpp_int_t;
        cpp_int int_var__byte_read_error5;
        void byte_read_error5 (const cpp_int  & l__val);
        cpp_int byte_read_error5() const;
    
        typedef pu_cpp_int< 16 > byte_write_error5_cpp_int_t;
        cpp_int int_var__byte_write_error5;
        void byte_write_error5 (const cpp_int  & l__val);
        cpp_int byte_write_error5() const;
    
        typedef pu_cpp_int< 16 > ack_timeout_error5_cpp_int_t;
        cpp_int int_var__ack_timeout_error5;
        void ack_timeout_error5 (const cpp_int  & l__val);
        cpp_int ack_timeout_error5() const;
    
        typedef pu_cpp_int< 16 > stall_timeout_error5_cpp_int_t;
        cpp_int int_var__stall_timeout_error5;
        void stall_timeout_error5 (const cpp_int  & l__val);
        cpp_int stall_timeout_error5() const;
    
        typedef pu_cpp_int< 32 > error_address_read1_cpp_int_t;
        cpp_int int_var__error_address_read1;
        void error_address_read1 (const cpp_int  & l__val);
        cpp_int error_address_read1() const;
    
        typedef pu_cpp_int< 32 > error_address_security1_cpp_int_t;
        cpp_int int_var__error_address_security1;
        void error_address_security1 (const cpp_int  & l__val);
        cpp_int error_address_security1() const;
    
        typedef pu_cpp_int< 32 > error_address_decode1_cpp_int_t;
        cpp_int int_var__error_address_decode1;
        void error_address_decode1 (const cpp_int  & l__val);
        cpp_int error_address_decode1() const;
    
        typedef pu_cpp_int< 32 > error_address_byte1_cpp_int_t;
        cpp_int int_var__error_address_byte1;
        void error_address_byte1 (const cpp_int  & l__val);
        cpp_int error_address_byte1() const;
    
        typedef pu_cpp_int< 32 > error_address_read2_cpp_int_t;
        cpp_int int_var__error_address_read2;
        void error_address_read2 (const cpp_int  & l__val);
        cpp_int error_address_read2() const;
    
        typedef pu_cpp_int< 32 > error_address_security2_cpp_int_t;
        cpp_int int_var__error_address_security2;
        void error_address_security2 (const cpp_int  & l__val);
        cpp_int error_address_security2() const;
    
        typedef pu_cpp_int< 32 > error_address_decode2_cpp_int_t;
        cpp_int int_var__error_address_decode2;
        void error_address_decode2 (const cpp_int  & l__val);
        cpp_int error_address_decode2() const;
    
        typedef pu_cpp_int< 32 > error_address_byte2_cpp_int_t;
        cpp_int int_var__error_address_byte2;
        void error_address_byte2 (const cpp_int  & l__val);
        cpp_int error_address_byte2() const;
    
        typedef pu_cpp_int< 32 > error_address_read3_cpp_int_t;
        cpp_int int_var__error_address_read3;
        void error_address_read3 (const cpp_int  & l__val);
        cpp_int error_address_read3() const;
    
        typedef pu_cpp_int< 32 > error_address_security3_cpp_int_t;
        cpp_int int_var__error_address_security3;
        void error_address_security3 (const cpp_int  & l__val);
        cpp_int error_address_security3() const;
    
        typedef pu_cpp_int< 32 > error_address_decode3_cpp_int_t;
        cpp_int int_var__error_address_decode3;
        void error_address_decode3 (const cpp_int  & l__val);
        cpp_int error_address_decode3() const;
    
        typedef pu_cpp_int< 32 > error_address_byte3_cpp_int_t;
        cpp_int int_var__error_address_byte3;
        void error_address_byte3 (const cpp_int  & l__val);
        cpp_int error_address_byte3() const;
    
        typedef pu_cpp_int< 32 > error_address_read4_cpp_int_t;
        cpp_int int_var__error_address_read4;
        void error_address_read4 (const cpp_int  & l__val);
        cpp_int error_address_read4() const;
    
        typedef pu_cpp_int< 32 > error_address_security4_cpp_int_t;
        cpp_int int_var__error_address_security4;
        void error_address_security4 (const cpp_int  & l__val);
        cpp_int error_address_security4() const;
    
        typedef pu_cpp_int< 32 > error_address_decode4_cpp_int_t;
        cpp_int int_var__error_address_decode4;
        void error_address_decode4 (const cpp_int  & l__val);
        cpp_int error_address_decode4() const;
    
        typedef pu_cpp_int< 32 > error_address_byte4_cpp_int_t;
        cpp_int int_var__error_address_byte4;
        void error_address_byte4 (const cpp_int  & l__val);
        cpp_int error_address_byte4() const;
    
        typedef pu_cpp_int< 32 > error_address_read5_cpp_int_t;
        cpp_int int_var__error_address_read5;
        void error_address_read5 (const cpp_int  & l__val);
        cpp_int error_address_read5() const;
    
        typedef pu_cpp_int< 32 > error_address_security5_cpp_int_t;
        cpp_int int_var__error_address_security5;
        void error_address_security5 (const cpp_int  & l__val);
        cpp_int error_address_security5() const;
    
        typedef pu_cpp_int< 32 > error_address_decode5_cpp_int_t;
        cpp_int int_var__error_address_decode5;
        void error_address_decode5 (const cpp_int  & l__val);
        cpp_int error_address_decode5() const;
    
        typedef pu_cpp_int< 32 > error_address_byte5_cpp_int_t;
        cpp_int int_var__error_address_byte5;
        void error_address_byte5 (const cpp_int  & l__val);
        cpp_int error_address_byte5() const;
    
        typedef pu_cpp_int< 1 > clr_error_capture_cpp_int_t;
        cpp_int int_var__clr_error_capture;
        void clr_error_capture (const cpp_int  & l__val);
        cpp_int clr_error_capture() const;
    
        typedef pu_cpp_int< 1 > cfg_writeack_cpp_int_t;
        cpp_int int_var__cfg_writeack;
        void cfg_writeack (const cpp_int  & l__val);
        cpp_int cfg_writeack() const;
    
}; // cap_ms_csr_ms_prp_t
    
class cap_ms_csr_ms_security_filter_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_security_filter_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_security_filter_t(string name = "cap_ms_csr_ms_security_filter_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_security_filter_t();
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
    
        typedef pu_cpp_int< 32 > read_cnt_cpp_int_t;
        cpp_int int_var__read_cnt;
        void read_cnt (const cpp_int  & l__val);
        cpp_int read_cnt() const;
    
        typedef pu_cpp_int< 32 > write_cnt_cpp_int_t;
        cpp_int int_var__write_cnt;
        void write_cnt (const cpp_int  & l__val);
        cpp_int write_cnt() const;
    
        typedef pu_cpp_int< 32 > error_address_read_cpp_int_t;
        cpp_int int_var__error_address_read;
        void error_address_read (const cpp_int  & l__val);
        cpp_int error_address_read() const;
    
        typedef pu_cpp_int< 32 > error_address_write_cpp_int_t;
        cpp_int int_var__error_address_write;
        void error_address_write (const cpp_int  & l__val);
        cpp_int error_address_write() const;
    
        typedef pu_cpp_int< 1 > clr_error_capture_cpp_int_t;
        cpp_int int_var__clr_error_capture;
        void clr_error_capture (const cpp_int  & l__val);
        cpp_int clr_error_capture() const;
    
}; // cap_ms_csr_ms_security_filter_t
    
class cap_ms_csr_ms_addr_filter_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_addr_filter_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_addr_filter_t(string name = "cap_ms_csr_ms_addr_filter_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_addr_filter_t();
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
    
        typedef pu_cpp_int< 32 > read_cnt_cpp_int_t;
        cpp_int int_var__read_cnt;
        void read_cnt (const cpp_int  & l__val);
        cpp_int read_cnt() const;
    
        typedef pu_cpp_int< 32 > write_cnt_cpp_int_t;
        cpp_int int_var__write_cnt;
        void write_cnt (const cpp_int  & l__val);
        cpp_int write_cnt() const;
    
        typedef pu_cpp_int< 32 > error_address_read_cpp_int_t;
        cpp_int int_var__error_address_read;
        void error_address_read (const cpp_int  & l__val);
        cpp_int error_address_read() const;
    
        typedef pu_cpp_int< 32 > error_address_write_cpp_int_t;
        cpp_int int_var__error_address_write;
        void error_address_write (const cpp_int  & l__val);
        cpp_int error_address_write() const;
    
        typedef pu_cpp_int< 1 > clr_error_capture_cpp_int_t;
        cpp_int int_var__clr_error_capture;
        void clr_error_capture (const cpp_int  & l__val);
        cpp_int clr_error_capture() const;
    
}; // cap_ms_csr_ms_addr_filter_t
    
class cap_ms_csr_ms_a61_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_a61_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_a61_t(string name = "cap_ms_csr_ms_a61_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_a61_t();
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
    
        typedef pu_cpp_int< 32 > read_cnt_cpp_int_t;
        cpp_int int_var__read_cnt;
        void read_cnt (const cpp_int  & l__val);
        cpp_int read_cnt() const;
    
        typedef pu_cpp_int< 32 > write_cnt_cpp_int_t;
        cpp_int int_var__write_cnt;
        void write_cnt (const cpp_int  & l__val);
        cpp_int write_cnt() const;
    
        typedef pu_cpp_int< 32 > narrow_read_cnt_cpp_int_t;
        cpp_int int_var__narrow_read_cnt;
        void narrow_read_cnt (const cpp_int  & l__val);
        cpp_int narrow_read_cnt() const;
    
        typedef pu_cpp_int< 32 > narrow_write_cnt_cpp_int_t;
        cpp_int int_var__narrow_write_cnt;
        void narrow_write_cnt (const cpp_int  & l__val);
        cpp_int narrow_write_cnt() const;
    
}; // cap_ms_csr_ms_a61_t
    
class cap_ms_csr_cfg_ms_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ms_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ms_t(string name = "cap_ms_csr_cfg_ms_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ms_t();
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
    
        typedef pu_cpp_int< 1 > fixer_enable_cpp_int_t;
        cpp_int int_var__fixer_enable;
        void fixer_enable (const cpp_int  & l__val);
        cpp_int fixer_enable() const;
    
        typedef pu_cpp_int< 1 > filter_enable_cpp_int_t;
        cpp_int int_var__filter_enable;
        void filter_enable (const cpp_int  & l__val);
        cpp_int filter_enable() const;
    
        typedef pu_cpp_int< 3 > prp_security_override_cpp_int_t;
        cpp_int int_var__prp_security_override;
        void prp_security_override (const cpp_int  & l__val);
        cpp_int prp_security_override() const;
    
        typedef pu_cpp_int< 1 > mb_esec_prot_enable_cpp_int_t;
        cpp_int int_var__mb_esec_prot_enable;
        void mb_esec_prot_enable (const cpp_int  & l__val);
        cpp_int mb_esec_prot_enable() const;
    
        typedef pu_cpp_int< 1 > fixer_no_use_wstrb_cpp_int_t;
        cpp_int int_var__fixer_no_use_wstrb;
        void fixer_no_use_wstrb (const cpp_int  & l__val);
        cpp_int fixer_no_use_wstrb() const;
    
}; // cap_ms_csr_cfg_ms_t
    
class cap_ms_csr_ms_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_ms_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_ms_t(string name = "cap_ms_csr_ms_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_ms_t();
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
    
}; // cap_ms_csr_ms_t
    
class cap_ms_csr_pads_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_pads_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_pads_t(string name = "cap_ms_csr_pads_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_pads_t();
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
    
        typedef pu_cpp_int< 5 > software_rev_id_cpp_int_t;
        cpp_int int_var__software_rev_id;
        void software_rev_id (const cpp_int  & l__val);
        cpp_int software_rev_id() const;
    
}; // cap_ms_csr_pads_t
    
class cap_ms_csr_spico_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_spico_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_spico_t(string name = "cap_ms_csr_spico_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_spico_t();
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
    
        typedef pu_cpp_int< 16 > gp_out_hbmsbm_cpp_int_t;
        cpp_int int_var__gp_out_hbmsbm;
        void gp_out_hbmsbm (const cpp_int  & l__val);
        cpp_int gp_out_hbmsbm() const;
    
        typedef pu_cpp_int< 1 > SRAM_BIST_DONE_FAIL_OUT_hbmsbm_cpp_int_t;
        cpp_int int_var__SRAM_BIST_DONE_FAIL_OUT_hbmsbm;
        void SRAM_BIST_DONE_FAIL_OUT_hbmsbm (const cpp_int  & l__val);
        cpp_int SRAM_BIST_DONE_FAIL_OUT_hbmsbm() const;
    
        typedef pu_cpp_int< 1 > SRAM_BIST_DONE_PASS_OUT_hbmsbm_cpp_int_t;
        cpp_int int_var__SRAM_BIST_DONE_PASS_OUT_hbmsbm;
        void SRAM_BIST_DONE_PASS_OUT_hbmsbm (const cpp_int  & l__val);
        cpp_int SRAM_BIST_DONE_PASS_OUT_hbmsbm() const;
    
        typedef pu_cpp_int< 1 > CORE_SRAM_BIST_MODE_hbmsbm_cpp_int_t;
        cpp_int int_var__CORE_SRAM_BIST_MODE_hbmsbm;
        void CORE_SRAM_BIST_MODE_hbmsbm (const cpp_int  & l__val);
        cpp_int CORE_SRAM_BIST_MODE_hbmsbm() const;
    
        typedef pu_cpp_int< 1 > CORE_SRAM_BIST_RUN_hbmsbm_cpp_int_t;
        cpp_int int_var__CORE_SRAM_BIST_RUN_hbmsbm;
        void CORE_SRAM_BIST_RUN_hbmsbm (const cpp_int  & l__val);
        cpp_int CORE_SRAM_BIST_RUN_hbmsbm() const;
    
        typedef pu_cpp_int< 16 > gp_in_hbmsbm_cpp_int_t;
        cpp_int int_var__gp_in_hbmsbm;
        void gp_in_hbmsbm (const cpp_int  & l__val);
        cpp_int gp_in_hbmsbm() const;
    
        typedef pu_cpp_int< 1 > int_hbmsbm_cpp_int_t;
        cpp_int int_var__int_hbmsbm;
        void int_hbmsbm (const cpp_int  & l__val);
        cpp_int int_hbmsbm() const;
    
        typedef pu_cpp_int< 1 > i_rom_enable_hbmsbm_cpp_int_t;
        cpp_int int_var__i_rom_enable_hbmsbm;
        void i_rom_enable_hbmsbm (const cpp_int  & l__val);
        cpp_int i_rom_enable_hbmsbm() const;
    
}; // cap_ms_csr_spico_t
    
class cap_ms_csr_sta_sbus_indir_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_sbus_indir_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_sbus_indir_t(string name = "cap_ms_csr_sta_sbus_indir_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_sbus_indir_t();
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
    
        typedef pu_cpp_int< 1 > execute_cpp_int_t;
        cpp_int int_var__execute;
        void execute (const cpp_int  & l__val);
        cpp_int execute() const;
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
        typedef pu_cpp_int< 1 > rcv_data_vld_cpp_int_t;
        cpp_int int_var__rcv_data_vld;
        void rcv_data_vld (const cpp_int  & l__val);
        cpp_int rcv_data_vld() const;
    
        typedef pu_cpp_int< 3 > result_code_cpp_int_t;
        cpp_int int_var__result_code;
        void result_code (const cpp_int  & l__val);
        cpp_int result_code() const;
    
}; // cap_ms_csr_sta_sbus_indir_t
    
class cap_ms_csr_cfg_sbus_indir_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_sbus_indir_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_sbus_indir_t(string name = "cap_ms_csr_cfg_sbus_indir_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_sbus_indir_t();
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
    
        typedef pu_cpp_int< 8 > rcvr_addr_cpp_int_t;
        cpp_int int_var__rcvr_addr;
        void rcvr_addr (const cpp_int  & l__val);
        cpp_int rcvr_addr() const;
    
        typedef pu_cpp_int< 8 > data_addr_cpp_int_t;
        cpp_int int_var__data_addr;
        void data_addr (const cpp_int  & l__val);
        cpp_int data_addr() const;
    
        typedef pu_cpp_int< 8 > command_cpp_int_t;
        cpp_int int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
}; // cap_ms_csr_cfg_sbus_indir_t
    
class cap_ms_csr_cfg_sbus_result_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_sbus_result_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_sbus_result_t(string name = "cap_ms_csr_cfg_sbus_result_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_sbus_result_t();
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
    
        typedef pu_cpp_int< 1 > mode_cpp_int_t;
        cpp_int int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
}; // cap_ms_csr_cfg_sbus_result_t
    
class cap_ms_csr_sta_rei_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_rei_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_rei_t(string name = "cap_ms_csr_sta_rei_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_rei_t();
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
    
        typedef pu_cpp_int< 1 > p2c_rei_en_cpp_int_t;
        cpp_int int_var__p2c_rei_en;
        void p2c_rei_en (const cpp_int  & l__val);
        cpp_int p2c_rei_en() const;
    
        typedef pu_cpp_int< 1 > done_fail_cur_cpp_int_t;
        cpp_int int_var__done_fail_cur;
        void done_fail_cur (const cpp_int  & l__val);
        cpp_int done_fail_cur() const;
    
        typedef pu_cpp_int< 1 > done_pass_cur_cpp_int_t;
        cpp_int int_var__done_pass_cur;
        void done_pass_cur (const cpp_int  & l__val);
        cpp_int done_pass_cur() const;
    
        typedef pu_cpp_int< 1 > done_fail_cur_reg_cpp_int_t;
        cpp_int int_var__done_fail_cur_reg;
        void done_fail_cur_reg (const cpp_int  & l__val);
        cpp_int done_fail_cur_reg() const;
    
        typedef pu_cpp_int< 1 > done_pass_cur_reg_cpp_int_t;
        cpp_int int_var__done_pass_cur_reg;
        void done_pass_cur_reg (const cpp_int  & l__val);
        cpp_int done_pass_cur_reg() const;
    
        typedef pu_cpp_int< 6 > pll_locked_cpp_int_t;
        cpp_int int_var__pll_locked;
        void pll_locked (const cpp_int  & l__val);
        cpp_int pll_locked() const;
    
}; // cap_ms_csr_sta_rei_t
    
class cap_ms_csr_cfg_rei_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_rei_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_rei_t(string name = "cap_ms_csr_cfg_rei_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_rei_t();
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
    
        typedef pu_cpp_int< 2 > core_glbl_mem_bist_rst_cpp_int_t;
        cpp_int int_var__core_glbl_mem_bist_rst;
        void core_glbl_mem_bist_rst (const cpp_int  & l__val);
        cpp_int core_glbl_mem_bist_rst() const;
    
        typedef pu_cpp_int< 1 > auto_load_enable_cpp_int_t;
        cpp_int int_var__auto_load_enable;
        void auto_load_enable (const cpp_int  & l__val);
        cpp_int auto_load_enable() const;
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        cpp_int int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
        typedef pu_cpp_int< 1 > mode_cpp_int_t;
        cpp_int int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
        typedef pu_cpp_int< 1 > cfg_override_cpp_int_t;
        cpp_int int_var__cfg_override;
        void cfg_override (const cpp_int  & l__val);
        cpp_int cfg_override() const;
    
        typedef pu_cpp_int< 1 > col_disable_0_cpp_int_t;
        cpp_int int_var__col_disable_0;
        void col_disable_0 (const cpp_int  & l__val);
        cpp_int col_disable_0() const;
    
        typedef pu_cpp_int< 1 > col_disable_1_cpp_int_t;
        cpp_int int_var__col_disable_1;
        void col_disable_1 (const cpp_int  & l__val);
        cpp_int col_disable_1() const;
    
        typedef pu_cpp_int< 1 > row_disable_0_cpp_int_t;
        cpp_int int_var__row_disable_0;
        void row_disable_0 (const cpp_int  & l__val);
        cpp_int row_disable_0() const;
    
        typedef pu_cpp_int< 1 > row_disable_1_cpp_int_t;
        cpp_int int_var__row_disable_1;
        void row_disable_1 (const cpp_int  & l__val);
        cpp_int row_disable_1() const;
    
}; // cap_ms_csr_cfg_rei_t
    
class cap_ms_csr_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_spare_t(string name = "cap_ms_csr_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_spare_t();
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
    
        typedef pu_cpp_int< 1 > p2c_spare_0_cpp_int_t;
        cpp_int int_var__p2c_spare_0;
        void p2c_spare_0 (const cpp_int  & l__val);
        cpp_int p2c_spare_0() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_0_oe_cpp_int_t;
        cpp_int int_var__c2p_spare_0_oe;
        void c2p_spare_0_oe (const cpp_int  & l__val);
        cpp_int c2p_spare_0_oe() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_0_cpp_int_t;
        cpp_int int_var__c2p_spare_0;
        void c2p_spare_0 (const cpp_int  & l__val);
        cpp_int c2p_spare_0() const;
    
        typedef pu_cpp_int< 1 > p2c_spare_1_cpp_int_t;
        cpp_int int_var__p2c_spare_1;
        void p2c_spare_1 (const cpp_int  & l__val);
        cpp_int p2c_spare_1() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_1_oe_cpp_int_t;
        cpp_int int_var__c2p_spare_1_oe;
        void c2p_spare_1_oe (const cpp_int  & l__val);
        cpp_int c2p_spare_1_oe() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_1_cpp_int_t;
        cpp_int int_var__c2p_spare_1;
        void c2p_spare_1 (const cpp_int  & l__val);
        cpp_int c2p_spare_1() const;
    
        typedef pu_cpp_int< 1 > p2c_spare_2_cpp_int_t;
        cpp_int int_var__p2c_spare_2;
        void p2c_spare_2 (const cpp_int  & l__val);
        cpp_int p2c_spare_2() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_2_oe_cpp_int_t;
        cpp_int int_var__c2p_spare_2_oe;
        void c2p_spare_2_oe (const cpp_int  & l__val);
        cpp_int c2p_spare_2_oe() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_2_cpp_int_t;
        cpp_int int_var__c2p_spare_2;
        void c2p_spare_2 (const cpp_int  & l__val);
        cpp_int c2p_spare_2() const;
    
        typedef pu_cpp_int< 1 > p2c_spare_3_cpp_int_t;
        cpp_int int_var__p2c_spare_3;
        void p2c_spare_3 (const cpp_int  & l__val);
        cpp_int p2c_spare_3() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_3_oe_cpp_int_t;
        cpp_int int_var__c2p_spare_3_oe;
        void c2p_spare_3_oe (const cpp_int  & l__val);
        cpp_int c2p_spare_3_oe() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_3_cpp_int_t;
        cpp_int int_var__c2p_spare_3;
        void c2p_spare_3 (const cpp_int  & l__val);
        cpp_int c2p_spare_3() const;
    
        typedef pu_cpp_int< 1 > p2c_spare_4_cpp_int_t;
        cpp_int int_var__p2c_spare_4;
        void p2c_spare_4 (const cpp_int  & l__val);
        cpp_int p2c_spare_4() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_4_oe_cpp_int_t;
        cpp_int int_var__c2p_spare_4_oe;
        void c2p_spare_4_oe (const cpp_int  & l__val);
        cpp_int c2p_spare_4_oe() const;
    
        typedef pu_cpp_int< 1 > c2p_spare_4_cpp_int_t;
        cpp_int int_var__c2p_spare_4;
        void c2p_spare_4 (const cpp_int  & l__val);
        cpp_int c2p_spare_4() const;
    
}; // cap_ms_csr_spare_t
    
class cap_ms_csr_sta_pll_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_pll_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_pll_cfg_t(string name = "cap_ms_csr_sta_pll_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_pll_cfg_t();
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
    
        typedef pu_cpp_int< 2 > core_muldiv_cpp_int_t;
        cpp_int int_var__core_muldiv;
        void core_muldiv (const cpp_int  & l__val);
        cpp_int core_muldiv() const;
    
        typedef pu_cpp_int< 1 > core_pll_byp_cpp_int_t;
        cpp_int int_var__core_pll_byp;
        void core_pll_byp (const cpp_int  & l__val);
        cpp_int core_pll_byp() const;
    
        typedef pu_cpp_int< 2 > cpu_muldiv_cpp_int_t;
        cpp_int int_var__cpu_muldiv;
        void cpu_muldiv (const cpp_int  & l__val);
        cpp_int cpu_muldiv() const;
    
        typedef pu_cpp_int< 1 > cpu_pll_byp_cpp_int_t;
        cpp_int int_var__cpu_pll_byp;
        void cpu_pll_byp (const cpp_int  & l__val);
        cpp_int cpu_pll_byp() const;
    
        typedef pu_cpp_int< 2 > flash_muldiv_cpp_int_t;
        cpp_int int_var__flash_muldiv;
        void flash_muldiv (const cpp_int  & l__val);
        cpp_int flash_muldiv() const;
    
        typedef pu_cpp_int< 1 > flash_pll_byp_cpp_int_t;
        cpp_int int_var__flash_pll_byp;
        void flash_pll_byp (const cpp_int  & l__val);
        cpp_int flash_pll_byp() const;
    
        typedef pu_cpp_int< 6 > pll_lock_cpp_int_t;
        cpp_int int_var__pll_lock;
        void pll_lock (const cpp_int  & l__val);
        cpp_int pll_lock() const;
    
        typedef pu_cpp_int< 1 > core_dfs_freq_change_cpp_int_t;
        cpp_int int_var__core_dfs_freq_change;
        void core_dfs_freq_change (const cpp_int  & l__val);
        cpp_int core_dfs_freq_change() const;
    
        typedef pu_cpp_int< 1 > cpu_dfs_freq_change_cpp_int_t;
        cpp_int int_var__cpu_dfs_freq_change;
        void cpu_dfs_freq_change (const cpp_int  & l__val);
        cpp_int cpu_dfs_freq_change() const;
    
        typedef pu_cpp_int< 1 > flash_dfs_freq_change_cpp_int_t;
        cpp_int int_var__flash_dfs_freq_change;
        void flash_dfs_freq_change (const cpp_int  & l__val);
        cpp_int flash_dfs_freq_change() const;
    
        typedef pu_cpp_int< 1 > hbm_dfs_freq_change_cpp_int_t;
        cpp_int int_var__hbm_dfs_freq_change;
        void hbm_dfs_freq_change (const cpp_int  & l__val);
        cpp_int hbm_dfs_freq_change() const;
    
}; // cap_ms_csr_sta_pll_cfg_t
    
class cap_ms_csr_cfg_clk_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_clk_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_clk_t(string name = "cap_ms_csr_cfg_clk_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_clk_t();
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
    
        typedef pu_cpp_int< 3 > pll_select_core_cpp_int_t;
        cpp_int int_var__pll_select_core;
        void pll_select_core (const cpp_int  & l__val);
        cpp_int pll_select_core() const;
    
        typedef pu_cpp_int< 3 > pll_select_cpu_cpp_int_t;
        cpp_int int_var__pll_select_cpu;
        void pll_select_cpu (const cpp_int  & l__val);
        cpp_int pll_select_cpu() const;
    
        typedef pu_cpp_int< 3 > pll_select_ahb_cpp_int_t;
        cpp_int int_var__pll_select_ahb;
        void pll_select_ahb (const cpp_int  & l__val);
        cpp_int pll_select_ahb() const;
    
        typedef pu_cpp_int< 3 > pll_select_apb_cpp_int_t;
        cpp_int int_var__pll_select_apb;
        void pll_select_apb (const cpp_int  & l__val);
        cpp_int pll_select_apb() const;
    
        typedef pu_cpp_int< 3 > pll_select_emmc_cpp_int_t;
        cpp_int int_var__pll_select_emmc;
        void pll_select_emmc (const cpp_int  & l__val);
        cpp_int pll_select_emmc() const;
    
        typedef pu_cpp_int< 3 > pll_select_flash_cpp_int_t;
        cpp_int int_var__pll_select_flash;
        void pll_select_flash (const cpp_int  & l__val);
        cpp_int pll_select_flash() const;
    
}; // cap_ms_csr_cfg_clk_t
    
class cap_ms_csr_cfg_pll_c2p_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_pll_c2p_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_pll_c2p_t(string name = "cap_ms_csr_cfg_pll_c2p_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_pll_c2p_t();
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
    
        typedef pu_cpp_int< 3 > pll_dll_obs_cpp_int_t;
        cpp_int int_var__pll_dll_obs;
        void pll_dll_obs (const cpp_int  & l__val);
        cpp_int pll_dll_obs() const;
    
        typedef pu_cpp_int< 3 > pll_lock_obs_cpp_int_t;
        cpp_int int_var__pll_lock_obs;
        void pll_lock_obs (const cpp_int  & l__val);
        cpp_int pll_lock_obs() const;
    
        typedef pu_cpp_int< 3 > rx_fifo_obs_cpp_int_t;
        cpp_int int_var__rx_fifo_obs;
        void rx_fifo_obs (const cpp_int  & l__val);
        cpp_int rx_fifo_obs() const;
    
        typedef pu_cpp_int< 6 > tx_fifo_obs_cpp_int_t;
        cpp_int int_var__tx_fifo_obs;
        void tx_fifo_obs (const cpp_int  & l__val);
        cpp_int tx_fifo_obs() const;
    
}; // cap_ms_csr_cfg_pll_c2p_t
    
class cap_ms_csr_cfg_pll_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_pll_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_pll_hbm_t(string name = "cap_ms_csr_cfg_pll_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_pll_hbm_t();
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
    
        typedef pu_cpp_int< 1 > prog_fbdiv_23_cpp_int_t;
        cpp_int int_var__prog_fbdiv_23;
        void prog_fbdiv_23 (const cpp_int  & l__val);
        cpp_int prog_fbdiv_23() const;
    
        typedef pu_cpp_int< 8 > prog_fbdiv255_cpp_int_t;
        cpp_int int_var__prog_fbdiv255;
        void prog_fbdiv255 (const cpp_int  & l__val);
        cpp_int prog_fbdiv255() const;
    
        typedef pu_cpp_int< 6 > pll_refcnt_cpp_int_t;
        cpp_int int_var__pll_refcnt;
        void pll_refcnt (const cpp_int  & l__val);
        cpp_int pll_refcnt() const;
    
        typedef pu_cpp_int< 6 > dll_refcnt_cpp_int_t;
        cpp_int int_var__dll_refcnt;
        void dll_refcnt (const cpp_int  & l__val);
        cpp_int dll_refcnt() const;
    
        typedef pu_cpp_int< 6 > dll_fbcnt_cpp_int_t;
        cpp_int int_var__dll_fbcnt;
        void dll_fbcnt (const cpp_int  & l__val);
        cpp_int dll_fbcnt() const;
    
        typedef pu_cpp_int< 6 > dll_out_divcnt_cpp_int_t;
        cpp_int int_var__dll_out_divcnt;
        void dll_out_divcnt (const cpp_int  & l__val);
        cpp_int dll_out_divcnt() const;
    
        typedef pu_cpp_int< 6 > pll_out_divcnt_cpp_int_t;
        cpp_int int_var__pll_out_divcnt;
        void pll_out_divcnt (const cpp_int  & l__val);
        cpp_int pll_out_divcnt() const;
    
        typedef pu_cpp_int< 4 > dll_disable_output_clk_cpp_int_t;
        cpp_int int_var__dll_disable_output_clk;
        void dll_disable_output_clk (const cpp_int  & l__val);
        cpp_int dll_disable_output_clk() const;
    
        typedef pu_cpp_int< 1 > pll_byp_io_cpp_int_t;
        cpp_int int_var__pll_byp_io;
        void pll_byp_io (const cpp_int  & l__val);
        cpp_int pll_byp_io() const;
    
        typedef pu_cpp_int< 1 > dll_out_divcnt_load_cpp_int_t;
        cpp_int int_var__dll_out_divcnt_load;
        void dll_out_divcnt_load (const cpp_int  & l__val);
        cpp_int dll_out_divcnt_load() const;
    
        typedef pu_cpp_int< 1 > pll_out_divcnt_load_cpp_int_t;
        cpp_int int_var__pll_out_divcnt_load;
        void pll_out_divcnt_load (const cpp_int  & l__val);
        cpp_int pll_out_divcnt_load() const;
    
        typedef pu_cpp_int< 4 > pll_disable_output_clk_cpp_int_t;
        cpp_int int_var__pll_disable_output_clk;
        void pll_disable_output_clk (const cpp_int  & l__val);
        cpp_int pll_disable_output_clk() const;
    
        typedef pu_cpp_int< 1 > pll_rst_n_cpp_int_t;
        cpp_int int_var__pll_rst_n;
        void pll_rst_n (const cpp_int  & l__val);
        cpp_int pll_rst_n() const;
    
}; // cap_ms_csr_cfg_pll_hbm_t
    
class cap_ms_csr_sta_ecc_elam_cap_buf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_ecc_elam_cap_buf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_ecc_elam_cap_buf_t(string name = "cap_ms_csr_sta_ecc_elam_cap_buf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_ecc_elam_cap_buf_t();
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
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 18 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_ms_csr_sta_ecc_elam_cap_buf_t
    
class cap_ms_csr_cfg_ecc_elam_cap_buf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_ecc_elam_cap_buf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_ecc_elam_cap_buf_t(string name = "cap_ms_csr_cfg_ecc_elam_cap_buf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_ecc_elam_cap_buf_t();
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
    
        typedef pu_cpp_int< 1 > cor_cpp_int_t;
        cpp_int int_var__cor;
        void cor (const cpp_int  & l__val);
        cpp_int cor() const;
    
        typedef pu_cpp_int< 1 > det_cpp_int_t;
        cpp_int int_var__det;
        void det (const cpp_int  & l__val);
        cpp_int det() const;
    
}; // cap_ms_csr_cfg_ecc_elam_cap_buf_t
    
class cap_ms_csr_sta_bist_elam_cap_buf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_bist_elam_cap_buf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_bist_elam_cap_buf_t(string name = "cap_ms_csr_sta_bist_elam_cap_buf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_bist_elam_cap_buf_t();
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
    
        typedef pu_cpp_int< 1 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
        typedef pu_cpp_int< 1 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
}; // cap_ms_csr_sta_bist_elam_cap_buf_t
    
class cap_ms_csr_cfg_elam_cap_buf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_elam_cap_buf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_elam_cap_buf_t(string name = "cap_ms_csr_cfg_elam_cap_buf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_elam_cap_buf_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
        typedef pu_cpp_int< 1 > eccbypass_cpp_int_t;
        cpp_int int_var__eccbypass;
        void eccbypass (const cpp_int  & l__val);
        cpp_int eccbypass() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_ms_csr_cfg_elam_cap_buf_t
    
class cap_ms_csr_sta_apb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_apb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_apb_t(string name = "cap_ms_csr_sta_apb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_apb_t();
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
    
        typedef pu_cpp_int< 1 > i2c_ic_en_0_cpp_int_t;
        cpp_int int_var__i2c_ic_en_0;
        void i2c_ic_en_0 (const cpp_int  & l__val);
        cpp_int i2c_ic_en_0() const;
    
        typedef pu_cpp_int< 1 > i2c_ic_en_1_cpp_int_t;
        cpp_int int_var__i2c_ic_en_1;
        void i2c_ic_en_1 (const cpp_int  & l__val);
        cpp_int i2c_ic_en_1() const;
    
        typedef pu_cpp_int< 1 > i2c_ic_en_2_cpp_int_t;
        cpp_int int_var__i2c_ic_en_2;
        void i2c_ic_en_2 (const cpp_int  & l__val);
        cpp_int i2c_ic_en_2() const;
    
        typedef pu_cpp_int< 1 > i2c_ic_en_3_cpp_int_t;
        cpp_int int_var__i2c_ic_en_3;
        void i2c_ic_en_3 (const cpp_int  & l__val);
        cpp_int i2c_ic_en_3() const;
    
        typedef pu_cpp_int< 1 > ssi1_sleep_cpp_int_t;
        cpp_int int_var__ssi1_sleep;
        void ssi1_sleep (const cpp_int  & l__val);
        cpp_int ssi1_sleep() const;
    
        typedef pu_cpp_int< 1 > ssi0_sleep_cpp_int_t;
        cpp_int int_var__ssi0_sleep;
        void ssi0_sleep (const cpp_int  & l__val);
        cpp_int ssi0_sleep() const;
    
}; // cap_ms_csr_sta_apb_t
    
class cap_ms_csr_cfg_wdt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_wdt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_wdt_t(string name = "cap_ms_csr_cfg_wdt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_wdt_t();
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
    
        typedef pu_cpp_int< 4 > rst_en_cpp_int_t;
        cpp_int int_var__rst_en;
        void rst_en (const cpp_int  & l__val);
        cpp_int rst_en() const;
    
        typedef pu_cpp_int< 4 > speed_up_cpp_int_t;
        cpp_int int_var__speed_up;
        void speed_up (const cpp_int  & l__val);
        cpp_int speed_up() const;
    
        typedef pu_cpp_int< 4 > pause_cpp_int_t;
        cpp_int int_var__pause;
        void pause (const cpp_int  & l__val);
        cpp_int pause() const;
    
        typedef pu_cpp_int< 4 > clk_en_cpp_int_t;
        cpp_int int_var__clk_en;
        void clk_en (const cpp_int  & l__val);
        cpp_int clk_en() const;
    
}; // cap_ms_csr_cfg_wdt_t
    
class cap_ms_csr_cfg_esec_dma_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_cfg_esec_dma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_cfg_esec_dma_t(string name = "cap_ms_csr_cfg_esec_dma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_cfg_esec_dma_t();
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
    
        typedef pu_cpp_int< 1 > arprot_1_cpp_int_t;
        cpp_int int_var__arprot_1;
        void arprot_1 (const cpp_int  & l__val);
        cpp_int arprot_1() const;
    
        typedef pu_cpp_int< 1 > awprot_1_cpp_int_t;
        cpp_int int_var__awprot_1;
        void awprot_1 (const cpp_int  & l__val);
        cpp_int awprot_1() const;
    
}; // cap_ms_csr_cfg_esec_dma_t
    
class cap_ms_csr_sta_ver_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_ver_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_ver_t(string name = "cap_ms_csr_sta_ver_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_ver_t();
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
    
        typedef pu_cpp_int< 4 > chip_type_cpp_int_t;
        cpp_int int_var__chip_type;
        void chip_type (const cpp_int  & l__val);
        cpp_int chip_type() const;
    
        typedef pu_cpp_int< 12 > chip_version_cpp_int_t;
        cpp_int int_var__chip_version;
        void chip_version (const cpp_int  & l__val);
        cpp_int chip_version() const;
    
        typedef pu_cpp_int< 16 > chip_build_cpp_int_t;
        cpp_int int_var__chip_build;
        void chip_build (const cpp_int  & l__val);
        cpp_int chip_build() const;
    
}; // cap_ms_csr_sta_ver_t
    
class cap_ms_csr_sta_chip_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_sta_chip_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_sta_chip_t(string name = "cap_ms_csr_sta_chip_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_sta_chip_t();
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
    
        typedef pu_cpp_int< 16 > part_number_cpp_int_t;
        cpp_int int_var__part_number;
        void part_number (const cpp_int  & l__val);
        cpp_int part_number() const;
    
}; // cap_ms_csr_sta_chip_t
    
class cap_ms_csr_rdintr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_rdintr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_rdintr_t(string name = "cap_ms_csr_rdintr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_rdintr_t();
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
    
        typedef pu_cpp_int< 32 > ireg_cpp_int_t;
        cpp_int int_var__ireg;
        void ireg (const cpp_int  & l__val);
        cpp_int ireg() const;
    
}; // cap_ms_csr_rdintr_t
    
class cap_ms_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_base_t(string name = "cap_ms_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_base_t();
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
    
}; // cap_ms_csr_base_t
    
class cap_ms_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ms_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ms_csr_t(string name = "cap_ms_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ms_csr_t();
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
    
        cap_ms_csr_base_t base;
    
        cap_ms_csr_rdintr_t rdintr;
    
        cap_ms_csr_sta_chip_t sta_chip;
    
        cap_ms_csr_sta_ver_t sta_ver;
    
        cap_ms_csr_cfg_esec_dma_t cfg_esec_dma;
    
        cap_ms_csr_cfg_wdt_t cfg_wdt;
    
        cap_ms_csr_sta_apb_t sta_apb;
    
        cap_ms_csr_cfg_elam_cap_buf_t cfg_elam_cap_buf;
    
        cap_ms_csr_sta_bist_elam_cap_buf_t sta_bist_elam_cap_buf;
    
        cap_ms_csr_cfg_ecc_elam_cap_buf_t cfg_ecc_elam_cap_buf;
    
        cap_ms_csr_sta_ecc_elam_cap_buf_t sta_ecc_elam_cap_buf;
    
        cap_ms_csr_cfg_pll_hbm_t cfg_pll_hbm;
    
        cap_ms_csr_cfg_pll_c2p_t cfg_pll_c2p;
    
        cap_ms_csr_cfg_clk_t cfg_clk;
    
        cap_ms_csr_sta_pll_cfg_t sta_pll_cfg;
    
        cap_ms_csr_spare_t spare;
    
        cap_ms_csr_cfg_rei_t cfg_rei;
    
        cap_ms_csr_sta_rei_t sta_rei;
    
        cap_ms_csr_cfg_sbus_result_t cfg_sbus_result;
    
        cap_ms_csr_cfg_sbus_indir_t cfg_sbus_indir;
    
        cap_ms_csr_sta_sbus_indir_t sta_sbus_indir;
    
        cap_ms_csr_spico_t spico;
    
        cap_ms_csr_pads_t pads;
    
        cap_ms_csr_ms_t ms;
    
        cap_ms_csr_cfg_ms_t cfg_ms;
    
        cap_ms_csr_ms_a61_t ms_a61;
    
        cap_ms_csr_ms_addr_filter_t ms_addr_filter;
    
        cap_ms_csr_ms_security_filter_t ms_security_filter;
    
        cap_ms_csr_ms_prp_t ms_prp;
    
        cap_ms_csr_cfg_hbm_t cfg_hbm;
    
        cap_ms_csr_cfg_elam_general_t cfg_elam_general;
    
        cap_ms_csr_cfg_elam_breakout_t cfg_elam_breakout;
    
        cap_ms_csr_cfg_elam_control_t cfg_elam_control;
    
        cap_ms_csr_cfg_elam_ext_trig_t cfg_elam_ext_trig;
    
        cap_ms_csr_sta_elam_t sta_elam;
    
        cap_ms_csr_cfg_arm_t cfg_arm;
    
        cap_ms_csr_cfg_esec_t cfg_esec;
    
        cap_ms_csr_cfg_pk_efuse_data_t cfg_pk_efuse_data;
    
        cap_ms_csr_cfg_pk_sw_efuse_t cfg_pk_sw_efuse;
    
        cap_ms_csr_cfg_pk_j2c_efuse_t cfg_pk_j2c_efuse;
    
        cap_ms_csr_cfg_pk_efuse_override_t cfg_pk_efuse_override;
    
        cap_ms_csr_sta_pk_efuse_t sta_pk_efuse;
    
        cap_ms_csr_sta_pk_efuse_val_t sta_pk_efuse_val;
    
        cap_ms_csr_cfg_ds16_efuse_t cfg_ds16_efuse;
    
        cap_ms_csr_sta_ds16_efuse_t sta_ds16_efuse;
    
        cap_ms_csr_sta_ds16_die_id_t sta_ds16_die_id;
    
        cap_ms_csr_cfg_ecc_disable_qspi_ram_t cfg_ecc_disable_qspi_ram;
    
        cap_ms_csr_sta_ecc_qspi_ram_t sta_ecc_qspi_ram;
    
        cap_ms_csr_cfg_qspi_t cfg_qspi;
    
        cap_ms_csr_cfg_bist_ssram_t cfg_bist_ssram;
    
        cap_ms_csr_sta_bist_ssram_t sta_bist_ssram;
    
        cap_ms_csr_sta_ecc_bl2_ram_t sta_ecc_bl2_ram;
    
        cap_ms_csr_cfg_ecc_disable_bl2_ram_t cfg_ecc_disable_bl2_ram;
    
        cap_ms_csr_sta_mem_init_bl2_ram_t sta_mem_init_bl2_ram;
    
        cap_ms_csr_cfg_ssram_t cfg_ssram;
    
        cap_ms_csr_cfg_c2j_general_t cfg_c2j_general;
    
        cap_ms_csr_cfg_c2j_tms_t cfg_c2j_tms;
    
        cap_ms_csr_cfg_c2j_tdi_t cfg_c2j_tdi;
    
        cap_ms_csr_sta_c2j_status_t sta_c2j_status;
    
        cap_ms_csr_cfg_mdio_general_t cfg_mdio_general;
    
        cap_ms_csr_cfg_mdio_access_t cfg_mdio_access;
    
        cap_ms_csr_sta_mdio_status_t sta_mdio_status;
    
        cap_ms_csr_sta_tap_chipid_t sta_tap_chipid;
    
        cap_ms_csr_cfg_dci_t cfg_dci;
    
        cap_ms_csr_cfg_ms_dap_t cfg_ms_dap;
    
        cap_ms_csr_cfg_socket_t cfg_socket;
    
        cap_ms_csr_cfg_msem_t cfg_msem;
    
        cap_ms_csr_cfg_cache_esec_t cfg_cache_esec;
    
        cap_ms_csr_cfg_uid2sid_t cfg_uid2sid;
    
        cap_ms_csr_cfg_sid2uid_t cfg_sid2uid;
    
        cap_ms_csr_sta_sid2uid_t sta_sid2uid;
    
        cap_ms_csr_sta_sid2uid_pending_t sta_sid2uid_pending;
    
        cap_ms_csr_sta_rst_t sta_rst;
    
        cap_ms_csr_ms_cfg_debug_t ms_cfg_debug;
    
        cap_ms_csr_cfg_nx_bist_t cfg_nx_bist;
    
        cap_ms_csr_sta_nx_bist_t sta_nx_bist;
    
        cap_ms_csr_ms_axi_t ms_axi;
    
        cap_ms_csr_csr_intr_t csr_intr;
    
        cap_ms_csr_cfg_uid2sidLL_t cfg_uid2sidLL;
    
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t cfg_uid2sidLL_hbm_hash_msk_bit0;
    
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t cfg_uid2sidLL_hbm_hash_msk_bit1;
    
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t cfg_uid2sidLL_hbm_hash_msk_bit2;
    
        cap_ms_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t cfg_uid2sidLL_hbm_hash_msk_bit3;
    
        cap_ms_csr_cfg_axi_bw_mon_t cfg_axi_bw_mon;
    
        cap_ms_csr_sta_axi_bw_mon_rd_latency_t sta_axi_bw_mon_rd_latency;
    
        cap_ms_csr_sta_axi_bw_mon_rd_bandwidth_t sta_axi_bw_mon_rd_bandwidth;
    
        cap_ms_csr_sta_axi_bw_mon_rd_transactions_t sta_axi_bw_mon_rd_transactions;
    
        cap_ms_csr_cnt_axi_bw_mon_rd_t cnt_axi_bw_mon_rd;
    
        cap_ms_csr_sta_axi_bw_mon_wr_latency_t sta_axi_bw_mon_wr_latency;
    
        cap_ms_csr_sta_axi_bw_mon_wr_bandwidth_t sta_axi_bw_mon_wr_bandwidth;
    
        cap_ms_csr_sta_axi_bw_mon_wr_transactions_t sta_axi_bw_mon_wr_transactions;
    
        cap_ms_csr_cnt_axi_bw_mon_wr_t cnt_axi_bw_mon_wr;
    
        cap_ms_csr_dhs_elam_cap_buf_t dhs_elam_cap_buf;
    
        cap_ms_csr_dhs_sbus_indir_t dhs_sbus_indir;
    
        cap_ms_csr_dhs_elam_m_t dhs_elam_m;
    
        cap_ms_csr_dhs_elam_eqt_t dhs_elam_eqt;
    
        cap_ms_csr_dhs_elam_branch_t dhs_elam_branch;
    
        cap_ms_csr_dhs_elam_capture_en_t dhs_elam_capture_en;
    
        cap_msr_csr_t msr;
    
        cap_msh_csr_t msh;
    
        cap_ms_csr_intgrp_status_t int_groups;
    
        cap_ms_csr_int_ms_t int_ms;
    
        cap_ms_csr_int_misc_t int_misc;
    
        cap_ms_csr_int_esecure_t int_esecure;
    
        cap_ms_csr_int_prp5_t int_prp1;
    
        cap_ms_csr_int_prp5_t int_prp2;
    
        cap_ms_csr_int_prp5_t int_prp3;
    
        cap_ms_csr_int_prp5_t int_prp4;
    
        cap_ms_csr_int_prp5_t int_prp5;
    
        cap_ms_csr_int_gic23_t int_gic0;
    
        cap_ms_csr_int_gic23_t int_gic1;
    
        cap_ms_csr_int_gic23_t int_gic2;
    
        cap_ms_csr_int_gic23_t int_gic3;
    
        cap_ms_csr_int_gic23_t int_gic4;
    
        cap_ms_csr_int_gic23_t int_gic5;
    
        cap_ms_csr_int_gic23_t int_gic6;
    
        cap_ms_csr_int_gic23_t int_gic7;
    
        cap_ms_csr_int_gic23_t int_gic8;
    
        cap_ms_csr_int_gic23_t int_gic9;
    
        cap_ms_csr_int_gic23_t int_gic10;
    
        cap_ms_csr_int_gic23_t int_gic11;
    
        cap_ms_csr_int_gic23_t int_gic12;
    
        cap_ms_csr_int_gic23_t int_gic13;
    
        cap_ms_csr_int_gic23_t int_gic14;
    
        cap_ms_csr_int_gic23_t int_gic15;
    
        cap_ms_csr_int_gic23_t int_gic16;
    
        cap_ms_csr_int_gic23_t int_gic17;
    
        cap_ms_csr_int_gic23_t int_gic18;
    
        cap_ms_csr_int_gic23_t int_gic19;
    
        cap_ms_csr_int_gic23_t int_gic20;
    
        cap_ms_csr_int_gic23_t int_gic21;
    
        cap_ms_csr_int_gic23_t int_gic22;
    
        cap_ms_csr_int_gic23_t int_gic23;
    
}; // cap_ms_csr_t
    
#endif // CAP_MS_CSR_H
        