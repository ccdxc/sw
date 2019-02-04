
#ifndef CAP_MPU_CSR_H
#define CAP_MPU_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mpu_csr_int_info_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_info_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_info_int_enable_clear_t(string name = "cap_mpu_csr_int_info_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_info_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > trace_full_0_enable_cpp_int_t;
        cpp_int int_var__trace_full_0_enable;
        void trace_full_0_enable (const cpp_int  & l__val);
        cpp_int trace_full_0_enable() const;
    
        typedef pu_cpp_int< 1 > trace_full_1_enable_cpp_int_t;
        cpp_int int_var__trace_full_1_enable;
        void trace_full_1_enable (const cpp_int  & l__val);
        cpp_int trace_full_1_enable() const;
    
        typedef pu_cpp_int< 1 > trace_full_2_enable_cpp_int_t;
        cpp_int int_var__trace_full_2_enable;
        void trace_full_2_enable (const cpp_int  & l__val);
        cpp_int trace_full_2_enable() const;
    
        typedef pu_cpp_int< 1 > trace_full_3_enable_cpp_int_t;
        cpp_int int_var__trace_full_3_enable;
        void trace_full_3_enable (const cpp_int  & l__val);
        cpp_int trace_full_3_enable() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_0_enable_cpp_int_t;
        cpp_int int_var__mpu_stop_0_enable;
        void mpu_stop_0_enable (const cpp_int  & l__val);
        cpp_int mpu_stop_0_enable() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_1_enable_cpp_int_t;
        cpp_int int_var__mpu_stop_1_enable;
        void mpu_stop_1_enable (const cpp_int  & l__val);
        cpp_int mpu_stop_1_enable() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_2_enable_cpp_int_t;
        cpp_int int_var__mpu_stop_2_enable;
        void mpu_stop_2_enable (const cpp_int  & l__val);
        cpp_int mpu_stop_2_enable() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_3_enable_cpp_int_t;
        cpp_int int_var__mpu_stop_3_enable;
        void mpu_stop_3_enable (const cpp_int  & l__val);
        cpp_int mpu_stop_3_enable() const;
    
}; // cap_mpu_csr_int_info_int_enable_clear_t
    
class cap_mpu_csr_int_info_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_info_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_info_int_test_set_t(string name = "cap_mpu_csr_int_info_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_info_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > trace_full_0_interrupt_cpp_int_t;
        cpp_int int_var__trace_full_0_interrupt;
        void trace_full_0_interrupt (const cpp_int  & l__val);
        cpp_int trace_full_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > trace_full_1_interrupt_cpp_int_t;
        cpp_int int_var__trace_full_1_interrupt;
        void trace_full_1_interrupt (const cpp_int  & l__val);
        cpp_int trace_full_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > trace_full_2_interrupt_cpp_int_t;
        cpp_int int_var__trace_full_2_interrupt;
        void trace_full_2_interrupt (const cpp_int  & l__val);
        cpp_int trace_full_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > trace_full_3_interrupt_cpp_int_t;
        cpp_int int_var__trace_full_3_interrupt;
        void trace_full_3_interrupt (const cpp_int  & l__val);
        cpp_int trace_full_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_0_interrupt_cpp_int_t;
        cpp_int int_var__mpu_stop_0_interrupt;
        void mpu_stop_0_interrupt (const cpp_int  & l__val);
        cpp_int mpu_stop_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_1_interrupt_cpp_int_t;
        cpp_int int_var__mpu_stop_1_interrupt;
        void mpu_stop_1_interrupt (const cpp_int  & l__val);
        cpp_int mpu_stop_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_2_interrupt_cpp_int_t;
        cpp_int int_var__mpu_stop_2_interrupt;
        void mpu_stop_2_interrupt (const cpp_int  & l__val);
        cpp_int mpu_stop_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpu_stop_3_interrupt_cpp_int_t;
        cpp_int int_var__mpu_stop_3_interrupt;
        void mpu_stop_3_interrupt (const cpp_int  & l__val);
        cpp_int mpu_stop_3_interrupt() const;
    
}; // cap_mpu_csr_int_info_int_test_set_t
    
class cap_mpu_csr_int_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_info_t(string name = "cap_mpu_csr_int_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_int_info_int_test_set_t intreg;
    
        cap_mpu_csr_int_info_int_test_set_t int_test_set;
    
        cap_mpu_csr_int_info_int_enable_clear_t int_enable_set;
    
        cap_mpu_csr_int_info_int_enable_clear_t int_enable_clear;
    
}; // cap_mpu_csr_int_info_t
    
class cap_mpu_csr_int_err_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_err_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_err_int_enable_clear_t(string name = "cap_mpu_csr_int_err_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_err_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > results_mismatch_enable_cpp_int_t;
        cpp_int int_var__results_mismatch_enable;
        void results_mismatch_enable (const cpp_int  & l__val);
        cpp_int results_mismatch_enable() const;
    
        typedef pu_cpp_int< 1 > sdp_mem_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__sdp_mem_uncorrectable_enable;
        void sdp_mem_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int sdp_mem_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > sdp_mem_correctable_enable_cpp_int_t;
        cpp_int int_var__sdp_mem_correctable_enable;
        void sdp_mem_correctable_enable (const cpp_int  & l__val);
        cpp_int sdp_mem_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > illegal_op_0_enable_cpp_int_t;
        cpp_int int_var__illegal_op_0_enable;
        void illegal_op_0_enable (const cpp_int  & l__val);
        cpp_int illegal_op_0_enable() const;
    
        typedef pu_cpp_int< 1 > illegal_op_1_enable_cpp_int_t;
        cpp_int int_var__illegal_op_1_enable;
        void illegal_op_1_enable (const cpp_int  & l__val);
        cpp_int illegal_op_1_enable() const;
    
        typedef pu_cpp_int< 1 > illegal_op_2_enable_cpp_int_t;
        cpp_int int_var__illegal_op_2_enable;
        void illegal_op_2_enable (const cpp_int  & l__val);
        cpp_int illegal_op_2_enable() const;
    
        typedef pu_cpp_int< 1 > illegal_op_3_enable_cpp_int_t;
        cpp_int int_var__illegal_op_3_enable;
        void illegal_op_3_enable (const cpp_int  & l__val);
        cpp_int illegal_op_3_enable() const;
    
        typedef pu_cpp_int< 1 > max_inst_0_enable_cpp_int_t;
        cpp_int int_var__max_inst_0_enable;
        void max_inst_0_enable (const cpp_int  & l__val);
        cpp_int max_inst_0_enable() const;
    
        typedef pu_cpp_int< 1 > max_inst_1_enable_cpp_int_t;
        cpp_int int_var__max_inst_1_enable;
        void max_inst_1_enable (const cpp_int  & l__val);
        cpp_int max_inst_1_enable() const;
    
        typedef pu_cpp_int< 1 > max_inst_2_enable_cpp_int_t;
        cpp_int int_var__max_inst_2_enable;
        void max_inst_2_enable (const cpp_int  & l__val);
        cpp_int max_inst_2_enable() const;
    
        typedef pu_cpp_int< 1 > max_inst_3_enable_cpp_int_t;
        cpp_int int_var__max_inst_3_enable;
        void max_inst_3_enable (const cpp_int  & l__val);
        cpp_int max_inst_3_enable() const;
    
        typedef pu_cpp_int< 1 > phvwr_0_enable_cpp_int_t;
        cpp_int int_var__phvwr_0_enable;
        void phvwr_0_enable (const cpp_int  & l__val);
        cpp_int phvwr_0_enable() const;
    
        typedef pu_cpp_int< 1 > phvwr_1_enable_cpp_int_t;
        cpp_int int_var__phvwr_1_enable;
        void phvwr_1_enable (const cpp_int  & l__val);
        cpp_int phvwr_1_enable() const;
    
        typedef pu_cpp_int< 1 > phvwr_2_enable_cpp_int_t;
        cpp_int int_var__phvwr_2_enable;
        void phvwr_2_enable (const cpp_int  & l__val);
        cpp_int phvwr_2_enable() const;
    
        typedef pu_cpp_int< 1 > phvwr_3_enable_cpp_int_t;
        cpp_int int_var__phvwr_3_enable;
        void phvwr_3_enable (const cpp_int  & l__val);
        cpp_int phvwr_3_enable() const;
    
        typedef pu_cpp_int< 1 > write_err_0_enable_cpp_int_t;
        cpp_int int_var__write_err_0_enable;
        void write_err_0_enable (const cpp_int  & l__val);
        cpp_int write_err_0_enable() const;
    
        typedef pu_cpp_int< 1 > write_err_1_enable_cpp_int_t;
        cpp_int int_var__write_err_1_enable;
        void write_err_1_enable (const cpp_int  & l__val);
        cpp_int write_err_1_enable() const;
    
        typedef pu_cpp_int< 1 > write_err_2_enable_cpp_int_t;
        cpp_int int_var__write_err_2_enable;
        void write_err_2_enable (const cpp_int  & l__val);
        cpp_int write_err_2_enable() const;
    
        typedef pu_cpp_int< 1 > write_err_3_enable_cpp_int_t;
        cpp_int int_var__write_err_3_enable;
        void write_err_3_enable (const cpp_int  & l__val);
        cpp_int write_err_3_enable() const;
    
        typedef pu_cpp_int< 1 > cache_axi_0_enable_cpp_int_t;
        cpp_int int_var__cache_axi_0_enable;
        void cache_axi_0_enable (const cpp_int  & l__val);
        cpp_int cache_axi_0_enable() const;
    
        typedef pu_cpp_int< 1 > cache_axi_1_enable_cpp_int_t;
        cpp_int int_var__cache_axi_1_enable;
        void cache_axi_1_enable (const cpp_int  & l__val);
        cpp_int cache_axi_1_enable() const;
    
        typedef pu_cpp_int< 1 > cache_axi_2_enable_cpp_int_t;
        cpp_int int_var__cache_axi_2_enable;
        void cache_axi_2_enable (const cpp_int  & l__val);
        cpp_int cache_axi_2_enable() const;
    
        typedef pu_cpp_int< 1 > cache_axi_3_enable_cpp_int_t;
        cpp_int int_var__cache_axi_3_enable;
        void cache_axi_3_enable (const cpp_int  & l__val);
        cpp_int cache_axi_3_enable() const;
    
        typedef pu_cpp_int< 1 > cache_parity_0_enable_cpp_int_t;
        cpp_int int_var__cache_parity_0_enable;
        void cache_parity_0_enable (const cpp_int  & l__val);
        cpp_int cache_parity_0_enable() const;
    
        typedef pu_cpp_int< 1 > cache_parity_1_enable_cpp_int_t;
        cpp_int int_var__cache_parity_1_enable;
        void cache_parity_1_enable (const cpp_int  & l__val);
        cpp_int cache_parity_1_enable() const;
    
        typedef pu_cpp_int< 1 > cache_parity_2_enable_cpp_int_t;
        cpp_int int_var__cache_parity_2_enable;
        void cache_parity_2_enable (const cpp_int  & l__val);
        cpp_int cache_parity_2_enable() const;
    
        typedef pu_cpp_int< 1 > cache_parity_3_enable_cpp_int_t;
        cpp_int int_var__cache_parity_3_enable;
        void cache_parity_3_enable (const cpp_int  & l__val);
        cpp_int cache_parity_3_enable() const;
    
}; // cap_mpu_csr_int_err_int_enable_clear_t
    
class cap_mpu_csr_int_err_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_err_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_err_intreg_t(string name = "cap_mpu_csr_int_err_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_err_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > results_mismatch_interrupt_cpp_int_t;
        cpp_int int_var__results_mismatch_interrupt;
        void results_mismatch_interrupt (const cpp_int  & l__val);
        cpp_int results_mismatch_interrupt() const;
    
        typedef pu_cpp_int< 1 > sdp_mem_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__sdp_mem_uncorrectable_interrupt;
        void sdp_mem_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int sdp_mem_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > sdp_mem_correctable_interrupt_cpp_int_t;
        cpp_int int_var__sdp_mem_correctable_interrupt;
        void sdp_mem_correctable_interrupt (const cpp_int  & l__val);
        cpp_int sdp_mem_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > illegal_op_0_interrupt_cpp_int_t;
        cpp_int int_var__illegal_op_0_interrupt;
        void illegal_op_0_interrupt (const cpp_int  & l__val);
        cpp_int illegal_op_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > illegal_op_1_interrupt_cpp_int_t;
        cpp_int int_var__illegal_op_1_interrupt;
        void illegal_op_1_interrupt (const cpp_int  & l__val);
        cpp_int illegal_op_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > illegal_op_2_interrupt_cpp_int_t;
        cpp_int int_var__illegal_op_2_interrupt;
        void illegal_op_2_interrupt (const cpp_int  & l__val);
        cpp_int illegal_op_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > illegal_op_3_interrupt_cpp_int_t;
        cpp_int int_var__illegal_op_3_interrupt;
        void illegal_op_3_interrupt (const cpp_int  & l__val);
        cpp_int illegal_op_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > max_inst_0_interrupt_cpp_int_t;
        cpp_int int_var__max_inst_0_interrupt;
        void max_inst_0_interrupt (const cpp_int  & l__val);
        cpp_int max_inst_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > max_inst_1_interrupt_cpp_int_t;
        cpp_int int_var__max_inst_1_interrupt;
        void max_inst_1_interrupt (const cpp_int  & l__val);
        cpp_int max_inst_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > max_inst_2_interrupt_cpp_int_t;
        cpp_int int_var__max_inst_2_interrupt;
        void max_inst_2_interrupt (const cpp_int  & l__val);
        cpp_int max_inst_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > max_inst_3_interrupt_cpp_int_t;
        cpp_int int_var__max_inst_3_interrupt;
        void max_inst_3_interrupt (const cpp_int  & l__val);
        cpp_int max_inst_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > phvwr_0_interrupt_cpp_int_t;
        cpp_int int_var__phvwr_0_interrupt;
        void phvwr_0_interrupt (const cpp_int  & l__val);
        cpp_int phvwr_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > phvwr_1_interrupt_cpp_int_t;
        cpp_int int_var__phvwr_1_interrupt;
        void phvwr_1_interrupt (const cpp_int  & l__val);
        cpp_int phvwr_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > phvwr_2_interrupt_cpp_int_t;
        cpp_int int_var__phvwr_2_interrupt;
        void phvwr_2_interrupt (const cpp_int  & l__val);
        cpp_int phvwr_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > phvwr_3_interrupt_cpp_int_t;
        cpp_int int_var__phvwr_3_interrupt;
        void phvwr_3_interrupt (const cpp_int  & l__val);
        cpp_int phvwr_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > write_err_0_interrupt_cpp_int_t;
        cpp_int int_var__write_err_0_interrupt;
        void write_err_0_interrupt (const cpp_int  & l__val);
        cpp_int write_err_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > write_err_1_interrupt_cpp_int_t;
        cpp_int int_var__write_err_1_interrupt;
        void write_err_1_interrupt (const cpp_int  & l__val);
        cpp_int write_err_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > write_err_2_interrupt_cpp_int_t;
        cpp_int int_var__write_err_2_interrupt;
        void write_err_2_interrupt (const cpp_int  & l__val);
        cpp_int write_err_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > write_err_3_interrupt_cpp_int_t;
        cpp_int int_var__write_err_3_interrupt;
        void write_err_3_interrupt (const cpp_int  & l__val);
        cpp_int write_err_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_axi_0_interrupt_cpp_int_t;
        cpp_int int_var__cache_axi_0_interrupt;
        void cache_axi_0_interrupt (const cpp_int  & l__val);
        cpp_int cache_axi_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_axi_1_interrupt_cpp_int_t;
        cpp_int int_var__cache_axi_1_interrupt;
        void cache_axi_1_interrupt (const cpp_int  & l__val);
        cpp_int cache_axi_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_axi_2_interrupt_cpp_int_t;
        cpp_int int_var__cache_axi_2_interrupt;
        void cache_axi_2_interrupt (const cpp_int  & l__val);
        cpp_int cache_axi_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_axi_3_interrupt_cpp_int_t;
        cpp_int int_var__cache_axi_3_interrupt;
        void cache_axi_3_interrupt (const cpp_int  & l__val);
        cpp_int cache_axi_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_parity_0_interrupt_cpp_int_t;
        cpp_int int_var__cache_parity_0_interrupt;
        void cache_parity_0_interrupt (const cpp_int  & l__val);
        cpp_int cache_parity_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_parity_1_interrupt_cpp_int_t;
        cpp_int int_var__cache_parity_1_interrupt;
        void cache_parity_1_interrupt (const cpp_int  & l__val);
        cpp_int cache_parity_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_parity_2_interrupt_cpp_int_t;
        cpp_int int_var__cache_parity_2_interrupt;
        void cache_parity_2_interrupt (const cpp_int  & l__val);
        cpp_int cache_parity_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > cache_parity_3_interrupt_cpp_int_t;
        cpp_int int_var__cache_parity_3_interrupt;
        void cache_parity_3_interrupt (const cpp_int  & l__val);
        cpp_int cache_parity_3_interrupt() const;
    
}; // cap_mpu_csr_int_err_intreg_t
    
class cap_mpu_csr_int_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_err_t(string name = "cap_mpu_csr_int_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_int_err_intreg_t intreg;
    
        cap_mpu_csr_int_err_intreg_t int_test_set;
    
        cap_mpu_csr_int_err_int_enable_clear_t int_enable_set;
    
        cap_mpu_csr_int_err_int_enable_clear_t int_enable_clear;
    
}; // cap_mpu_csr_int_err_t
    
class cap_mpu_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_intreg_status_t(string name = "cap_mpu_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_err_interrupt_cpp_int_t;
        cpp_int int_var__int_err_interrupt;
        void int_err_interrupt (const cpp_int  & l__val);
        cpp_int int_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_info_interrupt_cpp_int_t;
        cpp_int int_var__int_info_interrupt;
        void int_info_interrupt (const cpp_int  & l__val);
        cpp_int int_info_interrupt() const;
    
}; // cap_mpu_csr_intreg_status_t
    
class cap_mpu_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_int_groups_int_enable_rw_reg_t(string name = "cap_mpu_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_err_enable_cpp_int_t;
        cpp_int int_var__int_err_enable;
        void int_err_enable (const cpp_int  & l__val);
        cpp_int int_err_enable() const;
    
        typedef pu_cpp_int< 1 > int_info_enable_cpp_int_t;
        cpp_int int_var__int_info_enable;
        void int_info_enable (const cpp_int  & l__val);
        cpp_int int_info_enable() const;
    
}; // cap_mpu_csr_int_groups_int_enable_rw_reg_t
    
class cap_mpu_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_intgrp_status_t(string name = "cap_mpu_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_intreg_status_t intreg;
    
        cap_mpu_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_mpu_csr_intreg_status_t int_rw_reg;
    
}; // cap_mpu_csr_intgrp_status_t
    
class cap_mpu_csr_spr_reg_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_spr_reg_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_spr_reg_data_t(string name = "cap_mpu_csr_spr_reg_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_spr_reg_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_spr_reg_data_t
    
class cap_mpu_csr_spr_reg_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_spr_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_spr_reg_t(string name = "cap_mpu_csr_spr_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_spr_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_spr_reg_data_t data;
    
}; // cap_mpu_csr_spr_reg_t
    
class cap_mpu_csr_icache_sram_read_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_icache_sram_read_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_icache_sram_read_data_t(string name = "cap_mpu_csr_icache_sram_read_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_icache_sram_read_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_icache_sram_read_data_t
    
class cap_mpu_csr_icache_sram_read_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_icache_sram_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_icache_sram_read_t(string name = "cap_mpu_csr_icache_sram_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_icache_sram_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_icache_sram_read_data_t data;
    
}; // cap_mpu_csr_icache_sram_read_t
    
class cap_mpu_csr_tag_read_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_tag_read_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_tag_read_data_t(string name = "cap_mpu_csr_tag_read_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_tag_read_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_tag_read_data_t
    
class cap_mpu_csr_tag_read_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_tag_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_tag_read_t(string name = "cap_mpu_csr_tag_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_tag_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_tag_read_data_t data;
    
}; // cap_mpu_csr_tag_read_t
    
class cap_mpu_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_csr_intr_t(string name = "cap_mpu_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_csr_intr_t
    
class cap_mpu_csr_sta_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_spare_t(string name = "cap_mpu_csr_sta_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_mpu_csr_sta_spare_t
    
class cap_mpu_csr_cfg_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_spare_t(string name = "cap_mpu_csr_cfg_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_mpu_csr_cfg_spare_t
    
class cap_mpu_csr_sta_mpu3_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu3_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu3_icache_tag_t(string name = "cap_mpu_csr_sta_mpu3_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu3_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_mpu_csr_sta_mpu3_icache_tag_t
    
class cap_mpu_csr_cfg_mpu3_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu3_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu3_icache_tag_t(string name = "cap_mpu_csr_cfg_mpu3_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu3_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu3_icache_tag_t
    
class cap_mpu_csr_sta_mpu2_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu2_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu2_icache_tag_t(string name = "cap_mpu_csr_sta_mpu2_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu2_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_mpu_csr_sta_mpu2_icache_tag_t
    
class cap_mpu_csr_cfg_mpu2_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu2_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu2_icache_tag_t(string name = "cap_mpu_csr_cfg_mpu2_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu2_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu2_icache_tag_t
    
class cap_mpu_csr_sta_mpu1_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu1_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu1_icache_tag_t(string name = "cap_mpu_csr_sta_mpu1_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu1_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_mpu_csr_sta_mpu1_icache_tag_t
    
class cap_mpu_csr_cfg_mpu1_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu1_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu1_icache_tag_t(string name = "cap_mpu_csr_cfg_mpu1_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu1_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu1_icache_tag_t
    
class cap_mpu_csr_sta_mpu0_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu0_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu0_icache_tag_t(string name = "cap_mpu_csr_sta_mpu0_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu0_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_mpu_csr_sta_mpu0_icache_tag_t
    
class cap_mpu_csr_cfg_mpu0_icache_tag_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu0_icache_tag_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu0_icache_tag_t(string name = "cap_mpu_csr_cfg_mpu0_icache_tag_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu0_icache_tag_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu0_icache_tag_t
    
class cap_mpu_csr_sta_mpu3_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu3_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu3_icache_data1_t(string name = "cap_mpu_csr_sta_mpu3_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu3_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu3_icache_data1_t
    
class cap_mpu_csr_cfg_mpu3_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu3_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu3_icache_data1_t(string name = "cap_mpu_csr_cfg_mpu3_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu3_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu3_icache_data1_t
    
class cap_mpu_csr_sta_mpu3_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu3_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu3_icache_data0_t(string name = "cap_mpu_csr_sta_mpu3_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu3_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu3_icache_data0_t
    
class cap_mpu_csr_cfg_mpu3_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu3_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu3_icache_data0_t(string name = "cap_mpu_csr_cfg_mpu3_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu3_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu3_icache_data0_t
    
class cap_mpu_csr_sta_mpu2_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu2_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu2_icache_data1_t(string name = "cap_mpu_csr_sta_mpu2_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu2_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu2_icache_data1_t
    
class cap_mpu_csr_cfg_mpu2_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu2_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu2_icache_data1_t(string name = "cap_mpu_csr_cfg_mpu2_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu2_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu2_icache_data1_t
    
class cap_mpu_csr_sta_mpu2_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu2_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu2_icache_data0_t(string name = "cap_mpu_csr_sta_mpu2_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu2_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu2_icache_data0_t
    
class cap_mpu_csr_cfg_mpu2_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu2_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu2_icache_data0_t(string name = "cap_mpu_csr_cfg_mpu2_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu2_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu2_icache_data0_t
    
class cap_mpu_csr_sta_mpu1_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu1_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu1_icache_data1_t(string name = "cap_mpu_csr_sta_mpu1_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu1_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu1_icache_data1_t
    
class cap_mpu_csr_cfg_mpu1_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu1_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu1_icache_data1_t(string name = "cap_mpu_csr_cfg_mpu1_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu1_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu1_icache_data1_t
    
class cap_mpu_csr_sta_mpu1_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu1_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu1_icache_data0_t(string name = "cap_mpu_csr_sta_mpu1_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu1_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu1_icache_data0_t
    
class cap_mpu_csr_cfg_mpu1_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu1_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu1_icache_data0_t(string name = "cap_mpu_csr_cfg_mpu1_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu1_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu1_icache_data0_t
    
class cap_mpu_csr_sta_mpu0_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu0_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu0_icache_data1_t(string name = "cap_mpu_csr_sta_mpu0_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu0_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu0_icache_data1_t
    
class cap_mpu_csr_cfg_mpu0_icache_data1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu0_icache_data1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu0_icache_data1_t(string name = "cap_mpu_csr_cfg_mpu0_icache_data1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu0_icache_data1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu0_icache_data1_t
    
class cap_mpu_csr_sta_mpu0_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu0_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu0_icache_data0_t(string name = "cap_mpu_csr_sta_mpu0_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu0_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > parity_error_cpp_int_t;
        cpp_int int_var__parity_error;
        void parity_error (const cpp_int  & l__val);
        cpp_int parity_error() const;
    
        typedef pu_cpp_int< 8 > parity_error_addr_cpp_int_t;
        cpp_int int_var__parity_error_addr;
        void parity_error_addr (const cpp_int  & l__val);
        cpp_int parity_error_addr() const;
    
}; // cap_mpu_csr_sta_mpu0_icache_data0_t
    
class cap_mpu_csr_cfg_mpu0_icache_data0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_mpu0_icache_data0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_mpu0_icache_data0_t(string name = "cap_mpu_csr_cfg_mpu0_icache_data0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_mpu0_icache_data0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_cfg_mpu0_icache_data0_t
    
class cap_mpu_csr_sta_sdp_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_sdp_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_sdp_mem_t(string name = "cap_mpu_csr_sta_sdp_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_sdp_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 27 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_mpu_csr_sta_sdp_mem_t
    
class cap_mpu_csr_cfg_sdp_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_sdp_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_sdp_mem_t(string name = "cap_mpu_csr_cfg_sdp_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_sdp_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
}; // cap_mpu_csr_cfg_sdp_mem_t
    
class cap_mpu_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_cfg_debug_port_t(string name = "cap_mpu_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 3 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_mpu_csr_cfg_debug_port_t
    
class cap_mpu_csr_mismatch_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_mismatch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_mismatch_t(string name = "cap_mpu_csr_mismatch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_mismatch_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > update_result_count_cpp_int_t;
        cpp_int int_var__update_result_count;
        void update_result_count (const cpp_int  & l__val);
        cpp_int update_result_count() const;
    
        typedef pu_cpp_int< 4 > sdp_result_count_cpp_int_t;
        cpp_int int_var__sdp_result_count;
        void sdp_result_count (const cpp_int  & l__val);
        cpp_int sdp_result_count() const;
    
        typedef pu_cpp_int< 7 > pkt_id_cpp_int_t;
        cpp_int int_var__pkt_id;
        void pkt_id (const cpp_int  & l__val);
        cpp_int pkt_id() const;
    
}; // cap_mpu_csr_mismatch_t
    
class cap_mpu_csr_STA_stg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_STA_stg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_STA_stg_t(string name = "cap_mpu_csr_STA_stg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_STA_stg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sdp_srdy_in_cpp_int_t;
        cpp_int int_var__sdp_srdy_in;
        void sdp_srdy_in (const cpp_int  & l__val);
        cpp_int sdp_srdy_in() const;
    
        typedef pu_cpp_int< 1 > sdp_drdy_in_cpp_int_t;
        cpp_int int_var__sdp_drdy_in;
        void sdp_drdy_in (const cpp_int  & l__val);
        cpp_int sdp_drdy_in() const;
    
        typedef pu_cpp_int< 1 > srdy_out_cpp_int_t;
        cpp_int int_var__srdy_out;
        void srdy_out (const cpp_int  & l__val);
        cpp_int srdy_out() const;
    
        typedef pu_cpp_int< 1 > drdy_out_cpp_int_t;
        cpp_int int_var__drdy_out;
        void drdy_out (const cpp_int  & l__val);
        cpp_int drdy_out() const;
    
        typedef pu_cpp_int< 1 > srdy_cpp_int_t;
        cpp_int int_var__srdy;
        void srdy (const cpp_int  & l__val);
        cpp_int srdy() const;
    
        typedef pu_cpp_int< 1 > drdy_cpp_int_t;
        cpp_int int_var__drdy;
        void drdy (const cpp_int  & l__val);
        cpp_int drdy() const;
    
        typedef pu_cpp_int< 16 > te_valid_cpp_int_t;
        cpp_int int_var__te_valid;
        void te_valid (const cpp_int  & l__val);
        cpp_int te_valid() const;
    
        typedef pu_cpp_int< 4 > mpu_processing_cpp_int_t;
        cpp_int int_var__mpu_processing;
        void mpu_processing (const cpp_int  & l__val);
        cpp_int mpu_processing() const;
    
}; // cap_mpu_csr_STA_stg_t
    
class cap_mpu_csr_STA_live_sdp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_STA_live_sdp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_STA_live_sdp_t(string name = "cap_mpu_csr_STA_live_sdp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_STA_live_sdp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > update_empty_cpp_int_t;
        cpp_int int_var__update_empty;
        void update_empty (const cpp_int  & l__val);
        cpp_int update_empty() const;
    
        typedef pu_cpp_int< 32 > update_pkt_id_cpp_int_t;
        cpp_int int_var__update_pkt_id;
        void update_pkt_id (const cpp_int  & l__val);
        cpp_int update_pkt_id() const;
    
        typedef pu_cpp_int< 8 > phv_pkt_id_cpp_int_t;
        cpp_int int_var__phv_pkt_id;
        void phv_pkt_id (const cpp_int  & l__val);
        cpp_int phv_pkt_id() const;
    
        typedef pu_cpp_int< 12 > phv_update_valid_cpp_int_t;
        cpp_int int_var__phv_update_valid;
        void phv_update_valid (const cpp_int  & l__val);
        cpp_int phv_update_valid() const;
    
        typedef pu_cpp_int< 4 > update_result_count_cpp_int_t;
        cpp_int int_var__update_result_count;
        void update_result_count (const cpp_int  & l__val);
        cpp_int update_result_count() const;
    
        typedef pu_cpp_int< 4 > staging_full_cpp_int_t;
        cpp_int int_var__staging_full;
        void staging_full (const cpp_int  & l__val);
        cpp_int staging_full() const;
    
        typedef pu_cpp_int< 7 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
        typedef pu_cpp_int< 7 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
}; // cap_mpu_csr_STA_live_sdp_t
    
class cap_mpu_csr_CNT_sdp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_sdp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_sdp_t(string name = "cap_mpu_csr_CNT_sdp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_sdp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > phv_fifo_depth_cpp_int_t;
        cpp_int int_var__phv_fifo_depth;
        void phv_fifo_depth (const cpp_int  & l__val);
        cpp_int phv_fifo_depth() const;
    
        typedef pu_cpp_int< 16 > sop_in_cpp_int_t;
        cpp_int int_var__sop_in;
        void sop_in (const cpp_int  & l__val);
        cpp_int sop_in() const;
    
        typedef pu_cpp_int< 16 > eop_in_cpp_int_t;
        cpp_int int_var__eop_in;
        void eop_in (const cpp_int  & l__val);
        cpp_int eop_in() const;
    
        typedef pu_cpp_int< 16 > sop_out_cpp_int_t;
        cpp_int int_var__sop_out;
        void sop_out (const cpp_int  & l__val);
        cpp_int sop_out() const;
    
        typedef pu_cpp_int< 16 > eop_out_cpp_int_t;
        cpp_int int_var__eop_out;
        void eop_out (const cpp_int  & l__val);
        cpp_int eop_out() const;
    
}; // cap_mpu_csr_CNT_sdp_t
    
class cap_mpu_csr_sta_error_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_error_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_error_t(string name = "cap_mpu_csr_sta_error_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > icache0_cpp_int_t;
        cpp_int int_var__icache0;
        void icache0 (const cpp_int  & l__val);
        cpp_int icache0() const;
    
        typedef pu_cpp_int< 1 > icache1_cpp_int_t;
        cpp_int int_var__icache1;
        void icache1 (const cpp_int  & l__val);
        cpp_int icache1() const;
    
}; // cap_mpu_csr_sta_error_t
    
class cap_mpu_csr_sta_pend_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_pend_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_pend_t(string name = "cap_mpu_csr_sta_pend_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_pend_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > wr_id_cpp_int_t;
        cpp_int int_var__wr_id;
        void wr_id (const cpp_int  & l__val);
        cpp_int wr_id() const;
    
        typedef pu_cpp_int< 1 > pending_table_write_valid0_cpp_int_t;
        cpp_int int_var__pending_table_write_valid0;
        void pending_table_write_valid0 (const cpp_int  & l__val);
        cpp_int pending_table_write_valid0() const;
    
        typedef pu_cpp_int< 1 > pending_table_write_valid1_cpp_int_t;
        cpp_int int_var__pending_table_write_valid1;
        void pending_table_write_valid1 (const cpp_int  & l__val);
        cpp_int pending_table_write_valid1() const;
    
}; // cap_mpu_csr_sta_pend_t
    
class cap_mpu_csr_sta_gpr7_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr7_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr7_mpu_t(string name = "cap_mpu_csr_sta_gpr7_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr7_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr7_mpu_t
    
class cap_mpu_csr_sta_gpr6_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr6_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr6_mpu_t(string name = "cap_mpu_csr_sta_gpr6_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr6_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr6_mpu_t
    
class cap_mpu_csr_sta_gpr5_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr5_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr5_mpu_t(string name = "cap_mpu_csr_sta_gpr5_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr5_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr5_mpu_t
    
class cap_mpu_csr_sta_gpr4_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr4_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr4_mpu_t(string name = "cap_mpu_csr_sta_gpr4_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr4_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr4_mpu_t
    
class cap_mpu_csr_sta_gpr3_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr3_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr3_mpu_t(string name = "cap_mpu_csr_sta_gpr3_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr3_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr3_mpu_t
    
class cap_mpu_csr_sta_gpr2_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr2_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr2_mpu_t(string name = "cap_mpu_csr_sta_gpr2_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr2_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr2_mpu_t
    
class cap_mpu_csr_sta_gpr1_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_gpr1_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_gpr1_mpu_t(string name = "cap_mpu_csr_sta_gpr1_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_gpr1_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mpu_csr_sta_gpr1_mpu_t
    
class cap_mpu_csr_sta_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_mpu_t(string name = "cap_mpu_csr_sta_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > c1_cpp_int_t;
        cpp_int int_var__c1;
        void c1 (const cpp_int  & l__val);
        cpp_int c1() const;
    
        typedef pu_cpp_int< 1 > c2_cpp_int_t;
        cpp_int int_var__c2;
        void c2 (const cpp_int  & l__val);
        cpp_int c2() const;
    
        typedef pu_cpp_int< 1 > c3_cpp_int_t;
        cpp_int int_var__c3;
        void c3 (const cpp_int  & l__val);
        cpp_int c3() const;
    
        typedef pu_cpp_int< 1 > c4_cpp_int_t;
        cpp_int int_var__c4;
        void c4 (const cpp_int  & l__val);
        cpp_int c4() const;
    
        typedef pu_cpp_int< 1 > c5_cpp_int_t;
        cpp_int int_var__c5;
        void c5 (const cpp_int  & l__val);
        cpp_int c5() const;
    
        typedef pu_cpp_int< 1 > c6_cpp_int_t;
        cpp_int int_var__c6;
        void c6 (const cpp_int  & l__val);
        cpp_int c6() const;
    
        typedef pu_cpp_int< 1 > c7_cpp_int_t;
        cpp_int int_var__c7;
        void c7 (const cpp_int  & l__val);
        cpp_int c7() const;
    
}; // cap_mpu_csr_sta_mpu_t
    
class cap_mpu_csr_sta_ctl_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_ctl_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_ctl_mpu_t(string name = "cap_mpu_csr_sta_ctl_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_ctl_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > stopped_cpp_int_t;
        cpp_int int_var__stopped;
        void stopped (const cpp_int  & l__val);
        cpp_int stopped() const;
    
        typedef pu_cpp_int< 1 > ex_valid_cpp_int_t;
        cpp_int int_var__ex_valid;
        void ex_valid (const cpp_int  & l__val);
        cpp_int ex_valid() const;
    
        typedef pu_cpp_int< 1 > ex_execute_cpp_int_t;
        cpp_int int_var__ex_execute;
        void ex_execute (const cpp_int  & l__val);
        cpp_int ex_execute() const;
    
        typedef pu_cpp_int< 31 > ex_pc_cpp_int_t;
        cpp_int int_var__ex_pc;
        void ex_pc (const cpp_int  & l__val);
        cpp_int ex_pc() const;
    
        typedef pu_cpp_int< 3 > icache_state_cpp_int_t;
        cpp_int int_var__icache_state;
        void icache_state (const cpp_int  & l__val);
        cpp_int icache_state() const;
    
        typedef pu_cpp_int< 4 > phv_cmd_fifo_depth_cpp_int_t;
        cpp_int int_var__phv_cmd_fifo_depth;
        void phv_cmd_fifo_depth (const cpp_int  & l__val);
        cpp_int phv_cmd_fifo_depth() const;
    
        typedef pu_cpp_int< 5 > phv_data_fifo_depth_cpp_int_t;
        cpp_int int_var__phv_data_fifo_depth;
        void phv_data_fifo_depth (const cpp_int  & l__val);
        cpp_int phv_data_fifo_depth() const;
    
        typedef pu_cpp_int< 4 > stall_vector_cpp_int_t;
        cpp_int int_var__stall_vector;
        void stall_vector (const cpp_int  & l__val);
        cpp_int stall_vector() const;
    
}; // cap_mpu_csr_sta_ctl_mpu_t
    
class cap_mpu_csr_sta_pc_mpu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_pc_mpu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_pc_mpu_t(string name = "cap_mpu_csr_sta_pc_mpu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_pc_mpu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > inst_cpp_int_t;
        cpp_int int_var__inst;
        void inst (const cpp_int  & l__val);
        cpp_int inst() const;
    
}; // cap_mpu_csr_sta_pc_mpu_t
    
class cap_mpu_csr_sta_key_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_key_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_key_t(string name = "cap_mpu_csr_sta_key_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_key_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > kd_cpp_int_t;
        cpp_int int_var__kd;
        void kd (const cpp_int  & l__val);
        cpp_int kd() const;
    
}; // cap_mpu_csr_sta_key_t
    
class cap_mpu_csr_sta_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_data_t(string name = "cap_mpu_csr_sta_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > td_cpp_int_t;
        cpp_int int_var__td;
        void td (const cpp_int  & l__val);
        cpp_int td() const;
    
}; // cap_mpu_csr_sta_data_t
    
class cap_mpu_csr_sta_table_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_table_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_table_t(string name = "cap_mpu_csr_sta_table_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > mpu_processing_table_pcie_cpp_int_t;
        cpp_int int_var__mpu_processing_table_pcie;
        void mpu_processing_table_pcie (const cpp_int  & l__val);
        cpp_int mpu_processing_table_pcie() const;
    
        typedef pu_cpp_int< 1 > mpu_processing_table_sram_cpp_int_t;
        cpp_int int_var__mpu_processing_table_sram;
        void mpu_processing_table_sram (const cpp_int  & l__val);
        cpp_int mpu_processing_table_sram() const;
    
        typedef pu_cpp_int< 4 > mpu_processing_table_id_cpp_int_t;
        cpp_int int_var__mpu_processing_table_id;
        void mpu_processing_table_id (const cpp_int  & l__val);
        cpp_int mpu_processing_table_id() const;
    
        typedef pu_cpp_int< 8 > mpu_processing_pkt_id_cpp_int_t;
        cpp_int int_var__mpu_processing_pkt_id;
        void mpu_processing_pkt_id (const cpp_int  & l__val);
        cpp_int mpu_processing_pkt_id() const;
    
        typedef pu_cpp_int< 16 > mpu_processing_table_latency_cpp_int_t;
        cpp_int int_var__mpu_processing_table_latency;
        void mpu_processing_table_latency (const cpp_int  & l__val);
        cpp_int mpu_processing_table_latency() const;
    
}; // cap_mpu_csr_sta_table_t
    
class cap_mpu_csr_sta_tbl_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sta_tbl_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sta_tbl_addr_t(string name = "cap_mpu_csr_sta_tbl_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sta_tbl_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > in_mpu_cpp_int_t;
        cpp_int int_var__in_mpu;
        void in_mpu (const cpp_int  & l__val);
        cpp_int in_mpu() const;
    
}; // cap_mpu_csr_sta_tbl_addr_t
    
class cap_mpu_csr_CNT_fence_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_fence_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_fence_stall_t(string name = "cap_mpu_csr_CNT_fence_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_fence_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_fence_stall_t
    
class cap_mpu_csr_CNT_tblwr_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_tblwr_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_tblwr_stall_t(string name = "cap_mpu_csr_CNT_tblwr_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_tblwr_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_tblwr_stall_t
    
class cap_mpu_csr_CNT_memwr_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_memwr_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_memwr_stall_t(string name = "cap_mpu_csr_CNT_memwr_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_memwr_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_memwr_stall_t
    
class cap_mpu_csr_CNT_phvwr_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_phvwr_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_phvwr_stall_t(string name = "cap_mpu_csr_CNT_phvwr_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_phvwr_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_phvwr_stall_t
    
class cap_mpu_csr_CNT_hazard_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_hazard_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_hazard_stall_t(string name = "cap_mpu_csr_CNT_hazard_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_hazard_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_hazard_stall_t
    
class cap_mpu_csr_CNT_icache_fill_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_icache_fill_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_icache_fill_stall_t(string name = "cap_mpu_csr_CNT_icache_fill_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_icache_fill_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 24 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_icache_fill_stall_t
    
class cap_mpu_csr_CNT_icache_miss_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_icache_miss_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_icache_miss_t(string name = "cap_mpu_csr_CNT_icache_miss_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_icache_miss_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_icache_miss_t
    
class cap_mpu_csr_CNT_inst_executed_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_inst_executed_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_inst_executed_t(string name = "cap_mpu_csr_CNT_inst_executed_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_inst_executed_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 25 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_inst_executed_t
    
class cap_mpu_csr_CNT_phv_executed_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_phv_executed_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_phv_executed_t(string name = "cap_mpu_csr_CNT_phv_executed_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_phv_executed_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_phv_executed_t
    
class cap_mpu_csr_CNT_cycles_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_CNT_cycles_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_CNT_cycles_t(string name = "cap_mpu_csr_CNT_cycles_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_CNT_cycles_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 26 > counter_cpp_int_t;
        cpp_int int_var__counter;
        void counter (const cpp_int  & l__val);
        cpp_int counter() const;
    
}; // cap_mpu_csr_CNT_cycles_t
    
class cap_mpu_csr_count_stage_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_count_stage_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_count_stage_t(string name = "cap_mpu_csr_count_stage_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_count_stage_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > always_on_cpp_int_t;
        cpp_int int_var__always_on;
        void always_on (const cpp_int  & l__val);
        cpp_int always_on() const;
    
        typedef pu_cpp_int< 1 > debug_cpp_int_t;
        cpp_int int_var__debug;
        void debug (const cpp_int  & l__val);
        cpp_int debug() const;
    
        typedef pu_cpp_int< 1 > watch_cpp_int_t;
        cpp_int int_var__watch;
        void watch (const cpp_int  & l__val);
        cpp_int watch() const;
    
        typedef pu_cpp_int< 1 > stop_on_saturate_cpp_int_t;
        cpp_int int_var__stop_on_saturate;
        void stop_on_saturate (const cpp_int  & l__val);
        cpp_int stop_on_saturate() const;
    
}; // cap_mpu_csr_count_stage_t
    
class cap_mpu_csr_mpu_run_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_mpu_run_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_mpu_run_t(string name = "cap_mpu_csr_mpu_run_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_mpu_run_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > start_pulse_cpp_int_t;
        cpp_int int_var__start_pulse;
        void start_pulse (const cpp_int  & l__val);
        cpp_int start_pulse() const;
    
        typedef pu_cpp_int< 1 > stop_pulse_cpp_int_t;
        cpp_int int_var__stop_pulse;
        void stop_pulse (const cpp_int  & l__val);
        cpp_int stop_pulse() const;
    
        typedef pu_cpp_int< 1 > step_pulse_cpp_int_t;
        cpp_int int_var__step_pulse;
        void step_pulse (const cpp_int  & l__val);
        cpp_int step_pulse() const;
    
}; // cap_mpu_csr_mpu_run_t
    
class cap_mpu_csr_mpu_cfg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_mpu_cfg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_mpu_cfg_t(string name = "cap_mpu_csr_mpu_cfg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_mpu_cfg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > stall_wb_full_cpp_int_t;
        cpp_int int_var__stall_wb_full;
        void stall_wb_full (const cpp_int  & l__val);
        cpp_int stall_wb_full() const;
    
        typedef pu_cpp_int< 4 > icache_table_id_cpp_int_t;
        cpp_int int_var__icache_table_id;
        void icache_table_id (const cpp_int  & l__val);
        cpp_int icache_table_id() const;
    
        typedef pu_cpp_int< 16 > max_inst_count_cpp_int_t;
        cpp_int int_var__max_inst_count;
        void max_inst_count (const cpp_int  & l__val);
        cpp_int max_inst_count() const;
    
        typedef pu_cpp_int< 1 > te_clean_wb_enable_cpp_int_t;
        cpp_int int_var__te_clean_wb_enable;
        void te_clean_wb_enable (const cpp_int  & l__val);
        cpp_int te_clean_wb_enable() const;
    
}; // cap_mpu_csr_mpu_cfg_t
    
class cap_mpu_csr_mpu_ctl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_mpu_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_mpu_ctl_t(string name = "cap_mpu_csr_mpu_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_mpu_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > enable_stop_cpp_int_t;
        cpp_int int_var__enable_stop;
        void enable_stop (const cpp_int  & l__val);
        cpp_int enable_stop() const;
    
        typedef pu_cpp_int< 1 > enable_error_stop_cpp_int_t;
        cpp_int int_var__enable_error_stop;
        void enable_error_stop (const cpp_int  & l__val);
        cpp_int enable_error_stop() const;
    
        typedef pu_cpp_int< 1 > enable_pipe_freeze_on_stop_cpp_int_t;
        cpp_int int_var__enable_pipe_freeze_on_stop;
        void enable_pipe_freeze_on_stop (const cpp_int  & l__val);
        cpp_int enable_pipe_freeze_on_stop() const;
    
}; // cap_mpu_csr_mpu_ctl_t
    
class cap_mpu_csr_sdp_ctl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_sdp_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_sdp_ctl_t(string name = "cap_mpu_csr_sdp_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_sdp_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > enable_compress_cpp_int_t;
        cpp_int int_var__enable_compress;
        void enable_compress (const cpp_int  & l__val);
        cpp_int enable_compress() const;
    
        typedef pu_cpp_int< 1 > mask_frame_size_cpp_int_t;
        cpp_int int_var__mask_frame_size;
        void mask_frame_size (const cpp_int  & l__val);
        cpp_int mask_frame_size() const;
    
}; // cap_mpu_csr_sdp_ctl_t
    
class cap_mpu_csr_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_axi_attr_t(string name = "cap_mpu_csr_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_axi_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_mpu_csr_axi_attr_t
    
class cap_mpu_csr_icache_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_icache_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_icache_t(string name = "cap_mpu_csr_icache_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_icache_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > invalidate_cpp_int_t;
        cpp_int int_var__invalidate;
        void invalidate (const cpp_int  & l__val);
        cpp_int invalidate() const;
    
}; // cap_mpu_csr_icache_t
    
class cap_mpu_csr_trace_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_trace_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_trace_t(string name = "cap_mpu_csr_trace_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_trace_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > trace_enable_cpp_int_t;
        cpp_int int_var__trace_enable;
        void trace_enable (const cpp_int  & l__val);
        cpp_int trace_enable() const;
    
        typedef pu_cpp_int< 1 > phv_debug_cpp_int_t;
        cpp_int int_var__phv_debug;
        void phv_debug (const cpp_int  & l__val);
        cpp_int phv_debug() const;
    
        typedef pu_cpp_int< 1 > phv_error_cpp_int_t;
        cpp_int int_var__phv_error;
        void phv_error (const cpp_int  & l__val);
        cpp_int phv_error() const;
    
        typedef pu_cpp_int< 1 > watch_enable_cpp_int_t;
        cpp_int int_var__watch_enable;
        void watch_enable (const cpp_int  & l__val);
        cpp_int watch_enable() const;
    
        typedef pu_cpp_int< 31 > watch_pc_cpp_int_t;
        cpp_int int_var__watch_pc;
        void watch_pc (const cpp_int  & l__val);
        cpp_int watch_pc() const;
    
        typedef pu_cpp_int< 1 > table_and_key_cpp_int_t;
        cpp_int int_var__table_and_key;
        void table_and_key (const cpp_int  & l__val);
        cpp_int table_and_key() const;
    
        typedef pu_cpp_int< 1 > instructions_cpp_int_t;
        cpp_int int_var__instructions;
        void instructions (const cpp_int  & l__val);
        cpp_int instructions() const;
    
        typedef pu_cpp_int< 1 > wrap_cpp_int_t;
        cpp_int int_var__wrap;
        void wrap (const cpp_int  & l__val);
        cpp_int wrap() const;
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        cpp_int int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
        typedef pu_cpp_int< 28 > base_addr_cpp_int_t;
        cpp_int int_var__base_addr;
        void base_addr (const cpp_int  & l__val);
        cpp_int base_addr() const;
    
        typedef pu_cpp_int< 5 > buf_size_cpp_int_t;
        cpp_int int_var__buf_size;
        void buf_size (const cpp_int  & l__val);
        cpp_int buf_size() const;
    
        typedef pu_cpp_int< 20 > debug_index_cpp_int_t;
        cpp_int int_var__debug_index;
        void debug_index (const cpp_int  & l__val);
        cpp_int debug_index() const;
    
        typedef pu_cpp_int< 1 > debug_generation_cpp_int_t;
        cpp_int int_var__debug_generation;
        void debug_generation (const cpp_int  & l__val);
        cpp_int debug_generation() const;
    
}; // cap_mpu_csr_trace_t
    
class cap_mpu_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_base_t(string name = "cap_mpu_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mpu_csr_base_t
    
class cap_mpu_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mpu_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mpu_csr_t(string name = "cap_mpu_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mpu_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mpu_csr_base_t base;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_trace_t, 4> trace;
        #else 
        cap_mpu_csr_trace_t trace[4];
        #endif
        int get_depth_trace() { return 4; }
    
        cap_mpu_csr_icache_t icache;
    
        cap_mpu_csr_axi_attr_t axi_attr;
    
        cap_mpu_csr_sdp_ctl_t sdp_ctl;
    
        cap_mpu_csr_mpu_ctl_t mpu_ctl;
    
        cap_mpu_csr_mpu_cfg_t mpu_cfg;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_mpu_run_t, 4> mpu_run;
        #else 
        cap_mpu_csr_mpu_run_t mpu_run[4];
        #endif
        int get_depth_mpu_run() { return 4; }
    
        cap_mpu_csr_count_stage_t count_stage;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_cycles_t, 4> CNT_cycles;
        #else 
        cap_mpu_csr_CNT_cycles_t CNT_cycles[4];
        #endif
        int get_depth_CNT_cycles() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_phv_executed_t, 4> CNT_phv_executed;
        #else 
        cap_mpu_csr_CNT_phv_executed_t CNT_phv_executed[4];
        #endif
        int get_depth_CNT_phv_executed() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_inst_executed_t, 4> CNT_inst_executed;
        #else 
        cap_mpu_csr_CNT_inst_executed_t CNT_inst_executed[4];
        #endif
        int get_depth_CNT_inst_executed() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_icache_miss_t, 4> CNT_icache_miss;
        #else 
        cap_mpu_csr_CNT_icache_miss_t CNT_icache_miss[4];
        #endif
        int get_depth_CNT_icache_miss() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_icache_fill_stall_t, 4> CNT_icache_fill_stall;
        #else 
        cap_mpu_csr_CNT_icache_fill_stall_t CNT_icache_fill_stall[4];
        #endif
        int get_depth_CNT_icache_fill_stall() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_hazard_stall_t, 4> CNT_hazard_stall;
        #else 
        cap_mpu_csr_CNT_hazard_stall_t CNT_hazard_stall[4];
        #endif
        int get_depth_CNT_hazard_stall() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_phvwr_stall_t, 4> CNT_phvwr_stall;
        #else 
        cap_mpu_csr_CNT_phvwr_stall_t CNT_phvwr_stall[4];
        #endif
        int get_depth_CNT_phvwr_stall() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_memwr_stall_t, 4> CNT_memwr_stall;
        #else 
        cap_mpu_csr_CNT_memwr_stall_t CNT_memwr_stall[4];
        #endif
        int get_depth_CNT_memwr_stall() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_tblwr_stall_t, 4> CNT_tblwr_stall;
        #else 
        cap_mpu_csr_CNT_tblwr_stall_t CNT_tblwr_stall[4];
        #endif
        int get_depth_CNT_tblwr_stall() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_CNT_fence_stall_t, 4> CNT_fence_stall;
        #else 
        cap_mpu_csr_CNT_fence_stall_t CNT_fence_stall[4];
        #endif
        int get_depth_CNT_fence_stall() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_tbl_addr_t, 4> sta_tbl_addr;
        #else 
        cap_mpu_csr_sta_tbl_addr_t sta_tbl_addr[4];
        #endif
        int get_depth_sta_tbl_addr() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_table_t, 4> sta_table;
        #else 
        cap_mpu_csr_sta_table_t sta_table[4];
        #endif
        int get_depth_sta_table() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_data_t, 4> sta_data;
        #else 
        cap_mpu_csr_sta_data_t sta_data[4];
        #endif
        int get_depth_sta_data() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_key_t, 4> sta_key;
        #else 
        cap_mpu_csr_sta_key_t sta_key[4];
        #endif
        int get_depth_sta_key() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_pc_mpu_t, 4> sta_pc_mpu;
        #else 
        cap_mpu_csr_sta_pc_mpu_t sta_pc_mpu[4];
        #endif
        int get_depth_sta_pc_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_ctl_mpu_t, 4> sta_ctl_mpu;
        #else 
        cap_mpu_csr_sta_ctl_mpu_t sta_ctl_mpu[4];
        #endif
        int get_depth_sta_ctl_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_mpu_t, 4> sta_mpu;
        #else 
        cap_mpu_csr_sta_mpu_t sta_mpu[4];
        #endif
        int get_depth_sta_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr1_mpu_t, 4> sta_gpr1_mpu;
        #else 
        cap_mpu_csr_sta_gpr1_mpu_t sta_gpr1_mpu[4];
        #endif
        int get_depth_sta_gpr1_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr2_mpu_t, 4> sta_gpr2_mpu;
        #else 
        cap_mpu_csr_sta_gpr2_mpu_t sta_gpr2_mpu[4];
        #endif
        int get_depth_sta_gpr2_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr3_mpu_t, 4> sta_gpr3_mpu;
        #else 
        cap_mpu_csr_sta_gpr3_mpu_t sta_gpr3_mpu[4];
        #endif
        int get_depth_sta_gpr3_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr4_mpu_t, 4> sta_gpr4_mpu;
        #else 
        cap_mpu_csr_sta_gpr4_mpu_t sta_gpr4_mpu[4];
        #endif
        int get_depth_sta_gpr4_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr5_mpu_t, 4> sta_gpr5_mpu;
        #else 
        cap_mpu_csr_sta_gpr5_mpu_t sta_gpr5_mpu[4];
        #endif
        int get_depth_sta_gpr5_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr6_mpu_t, 4> sta_gpr6_mpu;
        #else 
        cap_mpu_csr_sta_gpr6_mpu_t sta_gpr6_mpu[4];
        #endif
        int get_depth_sta_gpr6_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_gpr7_mpu_t, 4> sta_gpr7_mpu;
        #else 
        cap_mpu_csr_sta_gpr7_mpu_t sta_gpr7_mpu[4];
        #endif
        int get_depth_sta_gpr7_mpu() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_pend_t, 4> sta_pend;
        #else 
        cap_mpu_csr_sta_pend_t sta_pend[4];
        #endif
        int get_depth_sta_pend() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_sta_error_t, 4> sta_error;
        #else 
        cap_mpu_csr_sta_error_t sta_error[4];
        #endif
        int get_depth_sta_error() { return 4; }
    
        cap_mpu_csr_CNT_sdp_t CNT_sdp;
    
        cap_mpu_csr_STA_live_sdp_t STA_live_sdp;
    
        cap_mpu_csr_STA_stg_t STA_stg;
    
        cap_mpu_csr_mismatch_t mismatch;
    
        cap_mpu_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_mpu_csr_cfg_sdp_mem_t cfg_sdp_mem;
    
        cap_mpu_csr_sta_sdp_mem_t sta_sdp_mem;
    
        cap_mpu_csr_cfg_mpu0_icache_data0_t cfg_mpu0_icache_data0;
    
        cap_mpu_csr_sta_mpu0_icache_data0_t sta_mpu0_icache_data0;
    
        cap_mpu_csr_cfg_mpu0_icache_data1_t cfg_mpu0_icache_data1;
    
        cap_mpu_csr_sta_mpu0_icache_data1_t sta_mpu0_icache_data1;
    
        cap_mpu_csr_cfg_mpu1_icache_data0_t cfg_mpu1_icache_data0;
    
        cap_mpu_csr_sta_mpu1_icache_data0_t sta_mpu1_icache_data0;
    
        cap_mpu_csr_cfg_mpu1_icache_data1_t cfg_mpu1_icache_data1;
    
        cap_mpu_csr_sta_mpu1_icache_data1_t sta_mpu1_icache_data1;
    
        cap_mpu_csr_cfg_mpu2_icache_data0_t cfg_mpu2_icache_data0;
    
        cap_mpu_csr_sta_mpu2_icache_data0_t sta_mpu2_icache_data0;
    
        cap_mpu_csr_cfg_mpu2_icache_data1_t cfg_mpu2_icache_data1;
    
        cap_mpu_csr_sta_mpu2_icache_data1_t sta_mpu2_icache_data1;
    
        cap_mpu_csr_cfg_mpu3_icache_data0_t cfg_mpu3_icache_data0;
    
        cap_mpu_csr_sta_mpu3_icache_data0_t sta_mpu3_icache_data0;
    
        cap_mpu_csr_cfg_mpu3_icache_data1_t cfg_mpu3_icache_data1;
    
        cap_mpu_csr_sta_mpu3_icache_data1_t sta_mpu3_icache_data1;
    
        cap_mpu_csr_cfg_mpu0_icache_tag_t cfg_mpu0_icache_tag;
    
        cap_mpu_csr_sta_mpu0_icache_tag_t sta_mpu0_icache_tag;
    
        cap_mpu_csr_cfg_mpu1_icache_tag_t cfg_mpu1_icache_tag;
    
        cap_mpu_csr_sta_mpu1_icache_tag_t sta_mpu1_icache_tag;
    
        cap_mpu_csr_cfg_mpu2_icache_tag_t cfg_mpu2_icache_tag;
    
        cap_mpu_csr_sta_mpu2_icache_tag_t sta_mpu2_icache_tag;
    
        cap_mpu_csr_cfg_mpu3_icache_tag_t cfg_mpu3_icache_tag;
    
        cap_mpu_csr_sta_mpu3_icache_tag_t sta_mpu3_icache_tag;
    
        cap_mpu_csr_cfg_spare_t cfg_spare;
    
        cap_mpu_csr_sta_spare_t sta_spare;
    
        cap_mpu_csr_csr_intr_t csr_intr;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_tag_read_t, 4> tag_read;
        #else 
        cap_mpu_csr_tag_read_t tag_read[4];
        #endif
        int get_depth_tag_read() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mpu_csr_icache_sram_read_t, 4> icache_sram_read;
        #else 
        cap_mpu_csr_icache_sram_read_t icache_sram_read[4];
        #endif
        int get_depth_icache_sram_read() { return 4; }
    
        cap_mpu_csr_spr_reg_t spr_reg;
    
        cap_mpu_csr_intgrp_status_t int_groups;
    
        cap_mpu_csr_int_err_t int_err;
    
        cap_mpu_csr_int_info_t int_info;
    
}; // cap_mpu_csr_t
    
#endif // CAP_MPU_CSR_H
        