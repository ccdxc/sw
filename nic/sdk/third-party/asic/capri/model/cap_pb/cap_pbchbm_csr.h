
#ifndef CAP_PBCHBM_CSR_H
#define CAP_PBCHBM_CSR_H

#include "cap_csr_base.h" 
#include "cap_pbchbmtx_csr.h" 
#include "cap_pbchbmeth_csr.h" 

using namespace std;
class cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > occupancy_9_enable_cpp_int_t;
        cpp_int int_var__occupancy_9_enable;
        void occupancy_9_enable (const cpp_int  & l__val);
        cpp_int occupancy_9_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_stop_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_stop_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_stop_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_stop_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_stop_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > occupancy_9_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_9_interrupt;
        void occupancy_9_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_9_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_stop_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_stop_t(string name = "cap_pbchbm_csr_int_hbm_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_stop_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_stop_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_stop_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_stop_t
    
class cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > drop_0_enable_cpp_int_t;
        cpp_int int_var__drop_0_enable;
        void drop_0_enable (const cpp_int  & l__val);
        cpp_int drop_0_enable() const;
    
        typedef pu_cpp_int< 1 > drop_1_enable_cpp_int_t;
        cpp_int int_var__drop_1_enable;
        void drop_1_enable (const cpp_int  & l__val);
        cpp_int drop_1_enable() const;
    
        typedef pu_cpp_int< 1 > drop_2_enable_cpp_int_t;
        cpp_int int_var__drop_2_enable;
        void drop_2_enable (const cpp_int  & l__val);
        cpp_int drop_2_enable() const;
    
        typedef pu_cpp_int< 1 > drop_3_enable_cpp_int_t;
        cpp_int int_var__drop_3_enable;
        void drop_3_enable (const cpp_int  & l__val);
        cpp_int drop_3_enable() const;
    
        typedef pu_cpp_int< 1 > drop_4_enable_cpp_int_t;
        cpp_int int_var__drop_4_enable;
        void drop_4_enable (const cpp_int  & l__val);
        cpp_int drop_4_enable() const;
    
        typedef pu_cpp_int< 1 > drop_5_enable_cpp_int_t;
        cpp_int int_var__drop_5_enable;
        void drop_5_enable (const cpp_int  & l__val);
        cpp_int drop_5_enable() const;
    
        typedef pu_cpp_int< 1 > drop_6_enable_cpp_int_t;
        cpp_int int_var__drop_6_enable;
        void drop_6_enable (const cpp_int  & l__val);
        cpp_int drop_6_enable() const;
    
        typedef pu_cpp_int< 1 > drop_7_enable_cpp_int_t;
        cpp_int int_var__drop_7_enable;
        void drop_7_enable (const cpp_int  & l__val);
        cpp_int drop_7_enable() const;
    
        typedef pu_cpp_int< 1 > drop_8_enable_cpp_int_t;
        cpp_int int_var__drop_8_enable;
        void drop_8_enable (const cpp_int  & l__val);
        cpp_int drop_8_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > drop_0_interrupt_cpp_int_t;
        cpp_int int_var__drop_0_interrupt;
        void drop_0_interrupt (const cpp_int  & l__val);
        cpp_int drop_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_1_interrupt_cpp_int_t;
        cpp_int int_var__drop_1_interrupt;
        void drop_1_interrupt (const cpp_int  & l__val);
        cpp_int drop_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_2_interrupt_cpp_int_t;
        cpp_int int_var__drop_2_interrupt;
        void drop_2_interrupt (const cpp_int  & l__val);
        cpp_int drop_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_3_interrupt_cpp_int_t;
        cpp_int int_var__drop_3_interrupt;
        void drop_3_interrupt (const cpp_int  & l__val);
        cpp_int drop_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_4_interrupt_cpp_int_t;
        cpp_int int_var__drop_4_interrupt;
        void drop_4_interrupt (const cpp_int  & l__val);
        cpp_int drop_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_5_interrupt_cpp_int_t;
        cpp_int int_var__drop_5_interrupt;
        void drop_5_interrupt (const cpp_int  & l__val);
        cpp_int drop_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_6_interrupt_cpp_int_t;
        cpp_int int_var__drop_6_interrupt;
        void drop_6_interrupt (const cpp_int  & l__val);
        cpp_int drop_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_7_interrupt_cpp_int_t;
        cpp_int int_var__drop_7_interrupt;
        void drop_7_interrupt (const cpp_int  & l__val);
        cpp_int drop_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > drop_8_interrupt_cpp_int_t;
        cpp_int int_var__drop_8_interrupt;
        void drop_8_interrupt (const cpp_int  & l__val);
        cpp_int drop_8_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_truncate_no_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_truncate_no_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_truncate_no_t(string name = "cap_pbchbm_csr_int_hbm_truncate_no_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_truncate_no_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_truncate_no_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_truncate_no_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_truncate_no_t
    
class cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > crossed_0_enable_cpp_int_t;
        cpp_int int_var__crossed_0_enable;
        void crossed_0_enable (const cpp_int  & l__val);
        cpp_int crossed_0_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_1_enable_cpp_int_t;
        cpp_int int_var__crossed_1_enable;
        void crossed_1_enable (const cpp_int  & l__val);
        cpp_int crossed_1_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_2_enable_cpp_int_t;
        cpp_int int_var__crossed_2_enable;
        void crossed_2_enable (const cpp_int  & l__val);
        cpp_int crossed_2_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_3_enable_cpp_int_t;
        cpp_int int_var__crossed_3_enable;
        void crossed_3_enable (const cpp_int  & l__val);
        cpp_int crossed_3_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_4_enable_cpp_int_t;
        cpp_int int_var__crossed_4_enable;
        void crossed_4_enable (const cpp_int  & l__val);
        cpp_int crossed_4_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_5_enable_cpp_int_t;
        cpp_int int_var__crossed_5_enable;
        void crossed_5_enable (const cpp_int  & l__val);
        cpp_int crossed_5_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_6_enable_cpp_int_t;
        cpp_int int_var__crossed_6_enable;
        void crossed_6_enable (const cpp_int  & l__val);
        cpp_int crossed_6_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_7_enable_cpp_int_t;
        cpp_int int_var__crossed_7_enable;
        void crossed_7_enable (const cpp_int  & l__val);
        cpp_int crossed_7_enable() const;
    
        typedef pu_cpp_int< 1 > crossed_8_enable_cpp_int_t;
        cpp_int int_var__crossed_8_enable;
        void crossed_8_enable (const cpp_int  & l__val);
        cpp_int crossed_8_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > crossed_0_interrupt_cpp_int_t;
        cpp_int int_var__crossed_0_interrupt;
        void crossed_0_interrupt (const cpp_int  & l__val);
        cpp_int crossed_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_1_interrupt_cpp_int_t;
        cpp_int int_var__crossed_1_interrupt;
        void crossed_1_interrupt (const cpp_int  & l__val);
        cpp_int crossed_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_2_interrupt_cpp_int_t;
        cpp_int int_var__crossed_2_interrupt;
        void crossed_2_interrupt (const cpp_int  & l__val);
        cpp_int crossed_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_3_interrupt_cpp_int_t;
        cpp_int int_var__crossed_3_interrupt;
        void crossed_3_interrupt (const cpp_int  & l__val);
        cpp_int crossed_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_4_interrupt_cpp_int_t;
        cpp_int int_var__crossed_4_interrupt;
        void crossed_4_interrupt (const cpp_int  & l__val);
        cpp_int crossed_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_5_interrupt_cpp_int_t;
        cpp_int int_var__crossed_5_interrupt;
        void crossed_5_interrupt (const cpp_int  & l__val);
        cpp_int crossed_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_6_interrupt_cpp_int_t;
        cpp_int int_var__crossed_6_interrupt;
        void crossed_6_interrupt (const cpp_int  & l__val);
        cpp_int crossed_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_7_interrupt_cpp_int_t;
        cpp_int int_var__crossed_7_interrupt;
        void crossed_7_interrupt (const cpp_int  & l__val);
        cpp_int crossed_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > crossed_8_interrupt_cpp_int_t;
        cpp_int int_var__crossed_8_interrupt;
        void crossed_8_interrupt (const cpp_int  & l__val);
        cpp_int crossed_8_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_drop_threshold_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_threshold_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_threshold_t(string name = "cap_pbchbm_csr_int_hbm_drop_threshold_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_threshold_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_drop_threshold_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_drop_threshold_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_drop_threshold_t
    
class cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > full_0_enable_cpp_int_t;
        cpp_int int_var__full_0_enable;
        void full_0_enable (const cpp_int  & l__val);
        cpp_int full_0_enable() const;
    
        typedef pu_cpp_int< 1 > full_1_enable_cpp_int_t;
        cpp_int int_var__full_1_enable;
        void full_1_enable (const cpp_int  & l__val);
        cpp_int full_1_enable() const;
    
        typedef pu_cpp_int< 1 > full_2_enable_cpp_int_t;
        cpp_int int_var__full_2_enable;
        void full_2_enable (const cpp_int  & l__val);
        cpp_int full_2_enable() const;
    
        typedef pu_cpp_int< 1 > full_3_enable_cpp_int_t;
        cpp_int int_var__full_3_enable;
        void full_3_enable (const cpp_int  & l__val);
        cpp_int full_3_enable() const;
    
        typedef pu_cpp_int< 1 > full_4_enable_cpp_int_t;
        cpp_int int_var__full_4_enable;
        void full_4_enable (const cpp_int  & l__val);
        cpp_int full_4_enable() const;
    
        typedef pu_cpp_int< 1 > full_5_enable_cpp_int_t;
        cpp_int int_var__full_5_enable;
        void full_5_enable (const cpp_int  & l__val);
        cpp_int full_5_enable() const;
    
        typedef pu_cpp_int< 1 > full_6_enable_cpp_int_t;
        cpp_int int_var__full_6_enable;
        void full_6_enable (const cpp_int  & l__val);
        cpp_int full_6_enable() const;
    
        typedef pu_cpp_int< 1 > full_7_enable_cpp_int_t;
        cpp_int int_var__full_7_enable;
        void full_7_enable (const cpp_int  & l__val);
        cpp_int full_7_enable() const;
    
        typedef pu_cpp_int< 1 > full_8_enable_cpp_int_t;
        cpp_int int_var__full_8_enable;
        void full_8_enable (const cpp_int  & l__val);
        cpp_int full_8_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > full_0_interrupt_cpp_int_t;
        cpp_int int_var__full_0_interrupt;
        void full_0_interrupt (const cpp_int  & l__val);
        cpp_int full_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_1_interrupt_cpp_int_t;
        cpp_int int_var__full_1_interrupt;
        void full_1_interrupt (const cpp_int  & l__val);
        cpp_int full_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_2_interrupt_cpp_int_t;
        cpp_int int_var__full_2_interrupt;
        void full_2_interrupt (const cpp_int  & l__val);
        cpp_int full_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_3_interrupt_cpp_int_t;
        cpp_int int_var__full_3_interrupt;
        void full_3_interrupt (const cpp_int  & l__val);
        cpp_int full_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_4_interrupt_cpp_int_t;
        cpp_int int_var__full_4_interrupt;
        void full_4_interrupt (const cpp_int  & l__val);
        cpp_int full_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_5_interrupt_cpp_int_t;
        cpp_int int_var__full_5_interrupt;
        void full_5_interrupt (const cpp_int  & l__val);
        cpp_int full_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_6_interrupt_cpp_int_t;
        cpp_int int_var__full_6_interrupt;
        void full_6_interrupt (const cpp_int  & l__val);
        cpp_int full_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_7_interrupt_cpp_int_t;
        cpp_int int_var__full_7_interrupt;
        void full_7_interrupt (const cpp_int  & l__val);
        cpp_int full_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_8_interrupt_cpp_int_t;
        cpp_int int_var__full_8_interrupt;
        void full_8_interrupt (const cpp_int  & l__val);
        cpp_int full_8_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_drop_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_ctrl_t(string name = "cap_pbchbm_csr_int_hbm_drop_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_drop_ctrl_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_drop_ctrl_t
    
class cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > filling_up_0_enable_cpp_int_t;
        cpp_int int_var__filling_up_0_enable;
        void filling_up_0_enable (const cpp_int  & l__val);
        cpp_int filling_up_0_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_1_enable_cpp_int_t;
        cpp_int int_var__filling_up_1_enable;
        void filling_up_1_enable (const cpp_int  & l__val);
        cpp_int filling_up_1_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_2_enable_cpp_int_t;
        cpp_int int_var__filling_up_2_enable;
        void filling_up_2_enable (const cpp_int  & l__val);
        cpp_int filling_up_2_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_3_enable_cpp_int_t;
        cpp_int int_var__filling_up_3_enable;
        void filling_up_3_enable (const cpp_int  & l__val);
        cpp_int filling_up_3_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_4_enable_cpp_int_t;
        cpp_int int_var__filling_up_4_enable;
        void filling_up_4_enable (const cpp_int  & l__val);
        cpp_int filling_up_4_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_5_enable_cpp_int_t;
        cpp_int int_var__filling_up_5_enable;
        void filling_up_5_enable (const cpp_int  & l__val);
        cpp_int filling_up_5_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_6_enable_cpp_int_t;
        cpp_int int_var__filling_up_6_enable;
        void filling_up_6_enable (const cpp_int  & l__val);
        cpp_int filling_up_6_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_7_enable_cpp_int_t;
        cpp_int int_var__filling_up_7_enable;
        void filling_up_7_enable (const cpp_int  & l__val);
        cpp_int filling_up_7_enable() const;
    
        typedef pu_cpp_int< 1 > filling_up_8_enable_cpp_int_t;
        cpp_int int_var__filling_up_8_enable;
        void filling_up_8_enable (const cpp_int  & l__val);
        cpp_int filling_up_8_enable() const;
    
        typedef pu_cpp_int< 1 > full_0_enable_cpp_int_t;
        cpp_int int_var__full_0_enable;
        void full_0_enable (const cpp_int  & l__val);
        cpp_int full_0_enable() const;
    
        typedef pu_cpp_int< 1 > full_1_enable_cpp_int_t;
        cpp_int int_var__full_1_enable;
        void full_1_enable (const cpp_int  & l__val);
        cpp_int full_1_enable() const;
    
        typedef pu_cpp_int< 1 > full_2_enable_cpp_int_t;
        cpp_int int_var__full_2_enable;
        void full_2_enable (const cpp_int  & l__val);
        cpp_int full_2_enable() const;
    
        typedef pu_cpp_int< 1 > full_3_enable_cpp_int_t;
        cpp_int int_var__full_3_enable;
        void full_3_enable (const cpp_int  & l__val);
        cpp_int full_3_enable() const;
    
        typedef pu_cpp_int< 1 > full_4_enable_cpp_int_t;
        cpp_int int_var__full_4_enable;
        void full_4_enable (const cpp_int  & l__val);
        cpp_int full_4_enable() const;
    
        typedef pu_cpp_int< 1 > full_5_enable_cpp_int_t;
        cpp_int int_var__full_5_enable;
        void full_5_enable (const cpp_int  & l__val);
        cpp_int full_5_enable() const;
    
        typedef pu_cpp_int< 1 > full_6_enable_cpp_int_t;
        cpp_int int_var__full_6_enable;
        void full_6_enable (const cpp_int  & l__val);
        cpp_int full_6_enable() const;
    
        typedef pu_cpp_int< 1 > full_7_enable_cpp_int_t;
        cpp_int int_var__full_7_enable;
        void full_7_enable (const cpp_int  & l__val);
        cpp_int full_7_enable() const;
    
        typedef pu_cpp_int< 1 > full_8_enable_cpp_int_t;
        cpp_int int_var__full_8_enable;
        void full_8_enable (const cpp_int  & l__val);
        cpp_int full_8_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > filling_up_0_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_0_interrupt;
        void filling_up_0_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_1_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_1_interrupt;
        void filling_up_1_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_2_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_2_interrupt;
        void filling_up_2_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_3_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_3_interrupt;
        void filling_up_3_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_4_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_4_interrupt;
        void filling_up_4_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_5_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_5_interrupt;
        void filling_up_5_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_6_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_6_interrupt;
        void filling_up_6_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_7_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_7_interrupt;
        void filling_up_7_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > filling_up_8_interrupt_cpp_int_t;
        cpp_int int_var__filling_up_8_interrupt;
        void filling_up_8_interrupt (const cpp_int  & l__val);
        cpp_int filling_up_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_0_interrupt_cpp_int_t;
        cpp_int int_var__full_0_interrupt;
        void full_0_interrupt (const cpp_int  & l__val);
        cpp_int full_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_1_interrupt_cpp_int_t;
        cpp_int int_var__full_1_interrupt;
        void full_1_interrupt (const cpp_int  & l__val);
        cpp_int full_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_2_interrupt_cpp_int_t;
        cpp_int int_var__full_2_interrupt;
        void full_2_interrupt (const cpp_int  & l__val);
        cpp_int full_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_3_interrupt_cpp_int_t;
        cpp_int int_var__full_3_interrupt;
        void full_3_interrupt (const cpp_int  & l__val);
        cpp_int full_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_4_interrupt_cpp_int_t;
        cpp_int int_var__full_4_interrupt;
        void full_4_interrupt (const cpp_int  & l__val);
        cpp_int full_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_5_interrupt_cpp_int_t;
        cpp_int int_var__full_5_interrupt;
        void full_5_interrupt (const cpp_int  & l__val);
        cpp_int full_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_6_interrupt_cpp_int_t;
        cpp_int int_var__full_6_interrupt;
        void full_6_interrupt (const cpp_int  & l__val);
        cpp_int full_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_7_interrupt_cpp_int_t;
        cpp_int int_var__full_7_interrupt;
        void full_7_interrupt (const cpp_int  & l__val);
        cpp_int full_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > full_8_interrupt_cpp_int_t;
        cpp_int int_var__full_8_interrupt;
        void full_8_interrupt (const cpp_int  & l__val);
        cpp_int full_8_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_drop_write_ack_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_write_ack_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_write_ack_t(string name = "cap_pbchbm_csr_int_hbm_drop_write_ack_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_write_ack_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_drop_write_ack_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_drop_write_ack_t
    
class cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > stop_0_enable_cpp_int_t;
        cpp_int int_var__stop_0_enable;
        void stop_0_enable (const cpp_int  & l__val);
        cpp_int stop_0_enable() const;
    
        typedef pu_cpp_int< 1 > stop_1_enable_cpp_int_t;
        cpp_int int_var__stop_1_enable;
        void stop_1_enable (const cpp_int  & l__val);
        cpp_int stop_1_enable() const;
    
        typedef pu_cpp_int< 1 > stop_2_enable_cpp_int_t;
        cpp_int int_var__stop_2_enable;
        void stop_2_enable (const cpp_int  & l__val);
        cpp_int stop_2_enable() const;
    
        typedef pu_cpp_int< 1 > stop_3_enable_cpp_int_t;
        cpp_int int_var__stop_3_enable;
        void stop_3_enable (const cpp_int  & l__val);
        cpp_int stop_3_enable() const;
    
        typedef pu_cpp_int< 1 > stop_4_enable_cpp_int_t;
        cpp_int int_var__stop_4_enable;
        void stop_4_enable (const cpp_int  & l__val);
        cpp_int stop_4_enable() const;
    
        typedef pu_cpp_int< 1 > stop_5_enable_cpp_int_t;
        cpp_int int_var__stop_5_enable;
        void stop_5_enable (const cpp_int  & l__val);
        cpp_int stop_5_enable() const;
    
        typedef pu_cpp_int< 1 > stop_6_enable_cpp_int_t;
        cpp_int int_var__stop_6_enable;
        void stop_6_enable (const cpp_int  & l__val);
        cpp_int stop_6_enable() const;
    
        typedef pu_cpp_int< 1 > stop_7_enable_cpp_int_t;
        cpp_int int_var__stop_7_enable;
        void stop_7_enable (const cpp_int  & l__val);
        cpp_int stop_7_enable() const;
    
        typedef pu_cpp_int< 1 > stop_8_enable_cpp_int_t;
        cpp_int int_var__stop_8_enable;
        void stop_8_enable (const cpp_int  & l__val);
        cpp_int stop_8_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > stop_0_interrupt_cpp_int_t;
        cpp_int int_var__stop_0_interrupt;
        void stop_0_interrupt (const cpp_int  & l__val);
        cpp_int stop_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_1_interrupt_cpp_int_t;
        cpp_int int_var__stop_1_interrupt;
        void stop_1_interrupt (const cpp_int  & l__val);
        cpp_int stop_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_2_interrupt_cpp_int_t;
        cpp_int int_var__stop_2_interrupt;
        void stop_2_interrupt (const cpp_int  & l__val);
        cpp_int stop_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_3_interrupt_cpp_int_t;
        cpp_int int_var__stop_3_interrupt;
        void stop_3_interrupt (const cpp_int  & l__val);
        cpp_int stop_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_4_interrupt_cpp_int_t;
        cpp_int int_var__stop_4_interrupt;
        void stop_4_interrupt (const cpp_int  & l__val);
        cpp_int stop_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_5_interrupt_cpp_int_t;
        cpp_int int_var__stop_5_interrupt;
        void stop_5_interrupt (const cpp_int  & l__val);
        cpp_int stop_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_6_interrupt_cpp_int_t;
        cpp_int int_var__stop_6_interrupt;
        void stop_6_interrupt (const cpp_int  & l__val);
        cpp_int stop_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_7_interrupt_cpp_int_t;
        cpp_int int_var__stop_7_interrupt;
        void stop_7_interrupt (const cpp_int  & l__val);
        cpp_int stop_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > stop_8_interrupt_cpp_int_t;
        cpp_int int_var__stop_8_interrupt;
        void stop_8_interrupt (const cpp_int  & l__val);
        cpp_int stop_8_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_drop_emergency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_emergency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_emergency_t(string name = "cap_pbchbm_csr_int_hbm_drop_emergency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_emergency_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_drop_emergency_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_drop_emergency_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_drop_emergency_t
    
class cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ctrl_enable_cpp_int_t;
        cpp_int int_var__ctrl_enable;
        void ctrl_enable (const cpp_int  & l__val);
        cpp_int ctrl_enable() const;
    
        typedef pu_cpp_int< 1 > pyld_enable_cpp_int_t;
        cpp_int int_var__pyld_enable;
        void pyld_enable (const cpp_int  & l__val);
        cpp_int pyld_enable() const;
    
        typedef pu_cpp_int< 1 > r2a_enable_cpp_int_t;
        cpp_int int_var__r2a_enable;
        void r2a_enable (const cpp_int  & l__val);
        cpp_int r2a_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ctrl_interrupt_cpp_int_t;
        cpp_int int_var__ctrl_interrupt;
        void ctrl_interrupt (const cpp_int  & l__val);
        cpp_int ctrl_interrupt() const;
    
        typedef pu_cpp_int< 1 > pyld_interrupt_cpp_int_t;
        cpp_int int_var__pyld_interrupt;
        void pyld_interrupt (const cpp_int  & l__val);
        cpp_int pyld_interrupt() const;
    
        typedef pu_cpp_int< 1 > r2a_interrupt_cpp_int_t;
        cpp_int int_var__r2a_interrupt;
        void r2a_interrupt (const cpp_int  & l__val);
        cpp_int r2a_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_axi_err_rsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_axi_err_rsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_axi_err_rsp_t(string name = "cap_pbchbm_csr_int_hbm_axi_err_rsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_axi_err_rsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_axi_err_rsp_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_axi_err_rsp_t
    
class cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > occupancy_0_enable_cpp_int_t;
        cpp_int int_var__occupancy_0_enable;
        void occupancy_0_enable (const cpp_int  & l__val);
        cpp_int occupancy_0_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_1_enable_cpp_int_t;
        cpp_int int_var__occupancy_1_enable;
        void occupancy_1_enable (const cpp_int  & l__val);
        cpp_int occupancy_1_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_2_enable_cpp_int_t;
        cpp_int int_var__occupancy_2_enable;
        void occupancy_2_enable (const cpp_int  & l__val);
        cpp_int occupancy_2_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_3_enable_cpp_int_t;
        cpp_int int_var__occupancy_3_enable;
        void occupancy_3_enable (const cpp_int  & l__val);
        cpp_int occupancy_3_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_4_enable_cpp_int_t;
        cpp_int int_var__occupancy_4_enable;
        void occupancy_4_enable (const cpp_int  & l__val);
        cpp_int occupancy_4_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_5_enable_cpp_int_t;
        cpp_int int_var__occupancy_5_enable;
        void occupancy_5_enable (const cpp_int  & l__val);
        cpp_int occupancy_5_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_6_enable_cpp_int_t;
        cpp_int int_var__occupancy_6_enable;
        void occupancy_6_enable (const cpp_int  & l__val);
        cpp_int occupancy_6_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_7_enable_cpp_int_t;
        cpp_int int_var__occupancy_7_enable;
        void occupancy_7_enable (const cpp_int  & l__val);
        cpp_int occupancy_7_enable() const;
    
        typedef pu_cpp_int< 1 > occupancy_8_enable_cpp_int_t;
        cpp_int int_var__occupancy_8_enable;
        void occupancy_8_enable (const cpp_int  & l__val);
        cpp_int occupancy_8_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_drop_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_drop_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > occupancy_0_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_0_interrupt;
        void occupancy_0_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_1_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_1_interrupt;
        void occupancy_1_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_2_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_2_interrupt;
        void occupancy_2_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_3_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_3_interrupt;
        void occupancy_3_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_4_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_4_interrupt;
        void occupancy_4_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_5_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_5_interrupt;
        void occupancy_5_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_6_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_6_interrupt;
        void occupancy_6_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_7_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_7_interrupt;
        void occupancy_7_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > occupancy_8_interrupt_cpp_int_t;
        cpp_int int_var__occupancy_8_interrupt;
        void occupancy_8_interrupt (const cpp_int  & l__val);
        cpp_int occupancy_8_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_drop_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_drop_t(string name = "cap_pbchbm_csr_int_hbm_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_drop_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_drop_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_drop_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_drop_t
    
class cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_out_0_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_0_enable;
        void sop_sop_out_0_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_0_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_1_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_1_enable;
        void sop_sop_out_1_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_1_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_2_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_2_enable;
        void sop_sop_out_2_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_2_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_3_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_3_enable;
        void sop_sop_out_3_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_3_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_4_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_4_enable;
        void sop_sop_out_4_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_4_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_5_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_5_enable;
        void sop_sop_out_5_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_5_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_6_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_6_enable;
        void sop_sop_out_6_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_6_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_7_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_7_enable;
        void sop_sop_out_7_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_7_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_8_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_8_enable;
        void sop_sop_out_8_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_8_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_9_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_9_enable;
        void sop_sop_out_9_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_9_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_0_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_0_enable;
        void eop_eop_out_0_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_0_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_1_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_1_enable;
        void eop_eop_out_1_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_1_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_2_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_2_enable;
        void eop_eop_out_2_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_2_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_3_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_3_enable;
        void eop_eop_out_3_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_3_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_4_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_4_enable;
        void eop_eop_out_4_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_4_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_5_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_5_enable;
        void eop_eop_out_5_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_5_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_6_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_6_enable;
        void eop_eop_out_6_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_6_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_7_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_7_enable;
        void eop_eop_out_7_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_7_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_8_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_8_enable;
        void eop_eop_out_8_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_8_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_9_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_9_enable;
        void eop_eop_out_9_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_9_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_out_0_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_0_interrupt;
        void sop_sop_out_0_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_1_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_1_interrupt;
        void sop_sop_out_1_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_2_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_2_interrupt;
        void sop_sop_out_2_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_3_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_3_interrupt;
        void sop_sop_out_3_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_4_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_4_interrupt;
        void sop_sop_out_4_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_5_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_5_interrupt;
        void sop_sop_out_5_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_6_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_6_interrupt;
        void sop_sop_out_6_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_7_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_7_interrupt;
        void sop_sop_out_7_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_8_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_8_interrupt;
        void sop_sop_out_8_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_9_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_9_interrupt;
        void sop_sop_out_9_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_0_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_0_interrupt;
        void eop_eop_out_0_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_1_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_1_interrupt;
        void eop_eop_out_1_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_2_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_2_interrupt;
        void eop_eop_out_2_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_3_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_3_interrupt;
        void eop_eop_out_3_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_4_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_4_interrupt;
        void eop_eop_out_4_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_5_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_5_interrupt;
        void eop_eop_out_5_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_6_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_6_interrupt;
        void eop_eop_out_6_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_7_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_7_interrupt;
        void eop_eop_out_7_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_8_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_8_interrupt;
        void eop_eop_out_8_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_9_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_9_interrupt;
        void eop_eop_out_9_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_9_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_pbus_violation_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_pbus_violation_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_pbus_violation_out_t(string name = "cap_pbchbm_csr_int_hbm_pbus_violation_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_pbus_violation_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_out_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_pbus_violation_out_t
    
class cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_in_0_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_0_enable;
        void sop_sop_in_0_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_0_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_1_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_1_enable;
        void sop_sop_in_1_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_1_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_2_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_2_enable;
        void sop_sop_in_2_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_2_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_3_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_3_enable;
        void sop_sop_in_3_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_3_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_4_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_4_enable;
        void sop_sop_in_4_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_4_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_5_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_5_enable;
        void sop_sop_in_5_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_5_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_6_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_6_enable;
        void sop_sop_in_6_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_6_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_7_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_7_enable;
        void sop_sop_in_7_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_7_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_8_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_8_enable;
        void sop_sop_in_8_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_8_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_9_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_9_enable;
        void sop_sop_in_9_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_9_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_0_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_0_enable;
        void eop_eop_in_0_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_0_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_1_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_1_enable;
        void eop_eop_in_1_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_1_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_2_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_2_enable;
        void eop_eop_in_2_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_2_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_3_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_3_enable;
        void eop_eop_in_3_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_3_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_4_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_4_enable;
        void eop_eop_in_4_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_4_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_5_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_5_enable;
        void eop_eop_in_5_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_5_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_6_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_6_enable;
        void eop_eop_in_6_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_6_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_7_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_7_enable;
        void eop_eop_in_7_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_7_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_8_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_8_enable;
        void eop_eop_in_8_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_8_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_9_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_9_enable;
        void eop_eop_in_9_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_9_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_in_0_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_0_interrupt;
        void sop_sop_in_0_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_1_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_1_interrupt;
        void sop_sop_in_1_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_2_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_2_interrupt;
        void sop_sop_in_2_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_3_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_3_interrupt;
        void sop_sop_in_3_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_4_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_4_interrupt;
        void sop_sop_in_4_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_5_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_5_interrupt;
        void sop_sop_in_5_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_6_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_6_interrupt;
        void sop_sop_in_6_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_7_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_7_interrupt;
        void sop_sop_in_7_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_8_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_8_interrupt;
        void sop_sop_in_8_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_9_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_9_interrupt;
        void sop_sop_in_9_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_0_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_0_interrupt;
        void eop_eop_in_0_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_1_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_1_interrupt;
        void eop_eop_in_1_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_2_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_2_interrupt;
        void eop_eop_in_2_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_3_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_3_interrupt;
        void eop_eop_in_3_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_4_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_4_interrupt;
        void eop_eop_in_4_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_5_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_5_interrupt;
        void eop_eop_in_5_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_6_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_6_interrupt;
        void eop_eop_in_6_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_7_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_7_interrupt;
        void eop_eop_in_7_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_8_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_8_interrupt;
        void eop_eop_in_8_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_9_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_9_interrupt;
        void eop_eop_in_9_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_9_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_pbus_violation_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_pbus_violation_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_pbus_violation_in_t(string name = "cap_pbchbm_csr_int_hbm_pbus_violation_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_pbus_violation_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_in_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_pbus_violation_in_t
    
class cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t(string name = "cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > timeout_0_enable_cpp_int_t;
        cpp_int int_var__timeout_0_enable;
        void timeout_0_enable (const cpp_int  & l__val);
        cpp_int timeout_0_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_1_enable_cpp_int_t;
        cpp_int int_var__timeout_1_enable;
        void timeout_1_enable (const cpp_int  & l__val);
        cpp_int timeout_1_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_2_enable_cpp_int_t;
        cpp_int int_var__timeout_2_enable;
        void timeout_2_enable (const cpp_int  & l__val);
        cpp_int timeout_2_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_3_enable_cpp_int_t;
        cpp_int int_var__timeout_3_enable;
        void timeout_3_enable (const cpp_int  & l__val);
        cpp_int timeout_3_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_4_enable_cpp_int_t;
        cpp_int int_var__timeout_4_enable;
        void timeout_4_enable (const cpp_int  & l__val);
        cpp_int timeout_4_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_5_enable_cpp_int_t;
        cpp_int int_var__timeout_5_enable;
        void timeout_5_enable (const cpp_int  & l__val);
        cpp_int timeout_5_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_6_enable_cpp_int_t;
        cpp_int int_var__timeout_6_enable;
        void timeout_6_enable (const cpp_int  & l__val);
        cpp_int timeout_6_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_7_enable_cpp_int_t;
        cpp_int int_var__timeout_7_enable;
        void timeout_7_enable (const cpp_int  & l__val);
        cpp_int timeout_7_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_8_enable_cpp_int_t;
        cpp_int int_var__timeout_8_enable;
        void timeout_8_enable (const cpp_int  & l__val);
        cpp_int timeout_8_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_9_enable_cpp_int_t;
        cpp_int int_var__timeout_9_enable;
        void timeout_9_enable (const cpp_int  & l__val);
        cpp_int timeout_9_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_10_enable_cpp_int_t;
        cpp_int int_var__timeout_10_enable;
        void timeout_10_enable (const cpp_int  & l__val);
        cpp_int timeout_10_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_11_enable_cpp_int_t;
        cpp_int int_var__timeout_11_enable;
        void timeout_11_enable (const cpp_int  & l__val);
        cpp_int timeout_11_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_12_enable_cpp_int_t;
        cpp_int int_var__timeout_12_enable;
        void timeout_12_enable (const cpp_int  & l__val);
        cpp_int timeout_12_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_13_enable_cpp_int_t;
        cpp_int int_var__timeout_13_enable;
        void timeout_13_enable (const cpp_int  & l__val);
        cpp_int timeout_13_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_14_enable_cpp_int_t;
        cpp_int int_var__timeout_14_enable;
        void timeout_14_enable (const cpp_int  & l__val);
        cpp_int timeout_14_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_15_enable_cpp_int_t;
        cpp_int int_var__timeout_15_enable;
        void timeout_15_enable (const cpp_int  & l__val);
        cpp_int timeout_15_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_16_enable_cpp_int_t;
        cpp_int int_var__timeout_16_enable;
        void timeout_16_enable (const cpp_int  & l__val);
        cpp_int timeout_16_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_17_enable_cpp_int_t;
        cpp_int int_var__timeout_17_enable;
        void timeout_17_enable (const cpp_int  & l__val);
        cpp_int timeout_17_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_18_enable_cpp_int_t;
        cpp_int int_var__timeout_18_enable;
        void timeout_18_enable (const cpp_int  & l__val);
        cpp_int timeout_18_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_19_enable_cpp_int_t;
        cpp_int int_var__timeout_19_enable;
        void timeout_19_enable (const cpp_int  & l__val);
        cpp_int timeout_19_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_20_enable_cpp_int_t;
        cpp_int int_var__timeout_20_enable;
        void timeout_20_enable (const cpp_int  & l__val);
        cpp_int timeout_20_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_21_enable_cpp_int_t;
        cpp_int int_var__timeout_21_enable;
        void timeout_21_enable (const cpp_int  & l__val);
        cpp_int timeout_21_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_22_enable_cpp_int_t;
        cpp_int int_var__timeout_22_enable;
        void timeout_22_enable (const cpp_int  & l__val);
        cpp_int timeout_22_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_23_enable_cpp_int_t;
        cpp_int int_var__timeout_23_enable;
        void timeout_23_enable (const cpp_int  & l__val);
        cpp_int timeout_23_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_24_enable_cpp_int_t;
        cpp_int int_var__timeout_24_enable;
        void timeout_24_enable (const cpp_int  & l__val);
        cpp_int timeout_24_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_25_enable_cpp_int_t;
        cpp_int int_var__timeout_25_enable;
        void timeout_25_enable (const cpp_int  & l__val);
        cpp_int timeout_25_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_26_enable_cpp_int_t;
        cpp_int int_var__timeout_26_enable;
        void timeout_26_enable (const cpp_int  & l__val);
        cpp_int timeout_26_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_27_enable_cpp_int_t;
        cpp_int int_var__timeout_27_enable;
        void timeout_27_enable (const cpp_int  & l__val);
        cpp_int timeout_27_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_28_enable_cpp_int_t;
        cpp_int int_var__timeout_28_enable;
        void timeout_28_enable (const cpp_int  & l__val);
        cpp_int timeout_28_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_29_enable_cpp_int_t;
        cpp_int int_var__timeout_29_enable;
        void timeout_29_enable (const cpp_int  & l__val);
        cpp_int timeout_29_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_30_enable_cpp_int_t;
        cpp_int int_var__timeout_30_enable;
        void timeout_30_enable (const cpp_int  & l__val);
        cpp_int timeout_30_enable() const;
    
        typedef pu_cpp_int< 1 > timeout_31_enable_cpp_int_t;
        cpp_int int_var__timeout_31_enable;
        void timeout_31_enable (const cpp_int  & l__val);
        cpp_int timeout_31_enable() const;
    
}; // cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t
    
class cap_pbchbm_csr_int_hbm_xoff_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_xoff_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_xoff_int_test_set_t(string name = "cap_pbchbm_csr_int_hbm_xoff_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_xoff_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > timeout_0_interrupt_cpp_int_t;
        cpp_int int_var__timeout_0_interrupt;
        void timeout_0_interrupt (const cpp_int  & l__val);
        cpp_int timeout_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_1_interrupt_cpp_int_t;
        cpp_int int_var__timeout_1_interrupt;
        void timeout_1_interrupt (const cpp_int  & l__val);
        cpp_int timeout_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_2_interrupt_cpp_int_t;
        cpp_int int_var__timeout_2_interrupt;
        void timeout_2_interrupt (const cpp_int  & l__val);
        cpp_int timeout_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_3_interrupt_cpp_int_t;
        cpp_int int_var__timeout_3_interrupt;
        void timeout_3_interrupt (const cpp_int  & l__val);
        cpp_int timeout_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_4_interrupt_cpp_int_t;
        cpp_int int_var__timeout_4_interrupt;
        void timeout_4_interrupt (const cpp_int  & l__val);
        cpp_int timeout_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_5_interrupt_cpp_int_t;
        cpp_int int_var__timeout_5_interrupt;
        void timeout_5_interrupt (const cpp_int  & l__val);
        cpp_int timeout_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_6_interrupt_cpp_int_t;
        cpp_int int_var__timeout_6_interrupt;
        void timeout_6_interrupt (const cpp_int  & l__val);
        cpp_int timeout_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_7_interrupt_cpp_int_t;
        cpp_int int_var__timeout_7_interrupt;
        void timeout_7_interrupt (const cpp_int  & l__val);
        cpp_int timeout_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_8_interrupt_cpp_int_t;
        cpp_int int_var__timeout_8_interrupt;
        void timeout_8_interrupt (const cpp_int  & l__val);
        cpp_int timeout_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_9_interrupt_cpp_int_t;
        cpp_int int_var__timeout_9_interrupt;
        void timeout_9_interrupt (const cpp_int  & l__val);
        cpp_int timeout_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_10_interrupt_cpp_int_t;
        cpp_int int_var__timeout_10_interrupt;
        void timeout_10_interrupt (const cpp_int  & l__val);
        cpp_int timeout_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_11_interrupt_cpp_int_t;
        cpp_int int_var__timeout_11_interrupt;
        void timeout_11_interrupt (const cpp_int  & l__val);
        cpp_int timeout_11_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_12_interrupt_cpp_int_t;
        cpp_int int_var__timeout_12_interrupt;
        void timeout_12_interrupt (const cpp_int  & l__val);
        cpp_int timeout_12_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_13_interrupt_cpp_int_t;
        cpp_int int_var__timeout_13_interrupt;
        void timeout_13_interrupt (const cpp_int  & l__val);
        cpp_int timeout_13_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_14_interrupt_cpp_int_t;
        cpp_int int_var__timeout_14_interrupt;
        void timeout_14_interrupt (const cpp_int  & l__val);
        cpp_int timeout_14_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_15_interrupt_cpp_int_t;
        cpp_int int_var__timeout_15_interrupt;
        void timeout_15_interrupt (const cpp_int  & l__val);
        cpp_int timeout_15_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_16_interrupt_cpp_int_t;
        cpp_int int_var__timeout_16_interrupt;
        void timeout_16_interrupt (const cpp_int  & l__val);
        cpp_int timeout_16_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_17_interrupt_cpp_int_t;
        cpp_int int_var__timeout_17_interrupt;
        void timeout_17_interrupt (const cpp_int  & l__val);
        cpp_int timeout_17_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_18_interrupt_cpp_int_t;
        cpp_int int_var__timeout_18_interrupt;
        void timeout_18_interrupt (const cpp_int  & l__val);
        cpp_int timeout_18_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_19_interrupt_cpp_int_t;
        cpp_int int_var__timeout_19_interrupt;
        void timeout_19_interrupt (const cpp_int  & l__val);
        cpp_int timeout_19_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_20_interrupt_cpp_int_t;
        cpp_int int_var__timeout_20_interrupt;
        void timeout_20_interrupt (const cpp_int  & l__val);
        cpp_int timeout_20_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_21_interrupt_cpp_int_t;
        cpp_int int_var__timeout_21_interrupt;
        void timeout_21_interrupt (const cpp_int  & l__val);
        cpp_int timeout_21_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_22_interrupt_cpp_int_t;
        cpp_int int_var__timeout_22_interrupt;
        void timeout_22_interrupt (const cpp_int  & l__val);
        cpp_int timeout_22_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_23_interrupt_cpp_int_t;
        cpp_int int_var__timeout_23_interrupt;
        void timeout_23_interrupt (const cpp_int  & l__val);
        cpp_int timeout_23_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_24_interrupt_cpp_int_t;
        cpp_int int_var__timeout_24_interrupt;
        void timeout_24_interrupt (const cpp_int  & l__val);
        cpp_int timeout_24_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_25_interrupt_cpp_int_t;
        cpp_int int_var__timeout_25_interrupt;
        void timeout_25_interrupt (const cpp_int  & l__val);
        cpp_int timeout_25_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_26_interrupt_cpp_int_t;
        cpp_int int_var__timeout_26_interrupt;
        void timeout_26_interrupt (const cpp_int  & l__val);
        cpp_int timeout_26_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_27_interrupt_cpp_int_t;
        cpp_int int_var__timeout_27_interrupt;
        void timeout_27_interrupt (const cpp_int  & l__val);
        cpp_int timeout_27_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_28_interrupt_cpp_int_t;
        cpp_int int_var__timeout_28_interrupt;
        void timeout_28_interrupt (const cpp_int  & l__val);
        cpp_int timeout_28_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_29_interrupt_cpp_int_t;
        cpp_int int_var__timeout_29_interrupt;
        void timeout_29_interrupt (const cpp_int  & l__val);
        cpp_int timeout_29_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_30_interrupt_cpp_int_t;
        cpp_int int_var__timeout_30_interrupt;
        void timeout_30_interrupt (const cpp_int  & l__val);
        cpp_int timeout_30_interrupt() const;
    
        typedef pu_cpp_int< 1 > timeout_31_interrupt_cpp_int_t;
        cpp_int int_var__timeout_31_interrupt;
        void timeout_31_interrupt (const cpp_int  & l__val);
        cpp_int timeout_31_interrupt() const;
    
}; // cap_pbchbm_csr_int_hbm_xoff_int_test_set_t
    
class cap_pbchbm_csr_int_hbm_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_hbm_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_hbm_xoff_t(string name = "cap_pbchbm_csr_int_hbm_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_hbm_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_hbm_xoff_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_hbm_xoff_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_hbm_xoff_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_hbm_xoff_t
    
class cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t(string name = "cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > uncorrectable_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_enable;
        void uncorrectable_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > correctable_enable_cpp_int_t;
        cpp_int int_var__correctable_enable;
        void correctable_enable (const cpp_int  & l__val);
        cpp_int correctable_enable() const;
    
}; // cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t
    
class cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t(string name = "cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_interrupt;
        void uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > correctable_interrupt_cpp_int_t;
        cpp_int int_var__correctable_interrupt;
        void correctable_interrupt (const cpp_int  & l__val);
        cpp_int correctable_interrupt() const;
    
}; // cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t
    
class cap_pbchbm_csr_int_ecc_hbm_mtu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_ecc_hbm_mtu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_ecc_hbm_mtu_t(string name = "cap_pbchbm_csr_int_ecc_hbm_mtu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_ecc_hbm_mtu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t intreg;
    
        cap_pbchbm_csr_int_ecc_hbm_mtu_intreg_t int_test_set;
    
        cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_ecc_hbm_ht_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_ecc_hbm_mtu_t
    
class cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t(string name = "cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > rb_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rb_uncorrectable_enable;
        void rb_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rb_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rb_correctable_enable_cpp_int_t;
        cpp_int int_var__rb_correctable_enable;
        void rb_correctable_enable (const cpp_int  & l__val);
        cpp_int rb_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > cdt_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__cdt_uncorrectable_enable;
        void cdt_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int cdt_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > cdt_correctable_enable_cpp_int_t;
        cpp_int int_var__cdt_correctable_enable;
        void cdt_correctable_enable (const cpp_int  & l__val);
        cpp_int cdt_correctable_enable() const;
    
}; // cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t
    
class cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t(string name = "cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > rb_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rb_uncorrectable_interrupt;
        void rb_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rb_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rb_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rb_correctable_interrupt;
        void rb_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rb_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > cdt_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__cdt_uncorrectable_interrupt;
        void cdt_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int cdt_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > cdt_correctable_interrupt_cpp_int_t;
        cpp_int int_var__cdt_correctable_interrupt;
        void cdt_correctable_interrupt (const cpp_int  & l__val);
        cpp_int cdt_correctable_interrupt() const;
    
}; // cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t
    
class cap_pbchbm_csr_int_ecc_hbm_rb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_ecc_hbm_rb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_ecc_hbm_rb_t(string name = "cap_pbchbm_csr_int_ecc_hbm_rb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_ecc_hbm_rb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t intreg;
    
        cap_pbchbm_csr_int_ecc_hbm_rb_int_test_set_t int_test_set;
    
        cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t int_enable_set;
    
        cap_pbchbm_csr_int_ecc_hbm_rb_int_enable_clear_t int_enable_clear;
    
}; // cap_pbchbm_csr_int_ecc_hbm_rb_t
    
class cap_pbchbm_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_intreg_status_t(string name = "cap_pbchbm_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_mtu_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_mtu_interrupt;
        void int_ecc_hbm_mtu_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_mtu_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_rb_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_rb_interrupt;
        void int_ecc_hbm_rb_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_rb_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_wb_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_wb_interrupt;
        void int_ecc_hbm_wb_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_wb_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_ht_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_ht_interrupt;
        void int_ecc_hbm_ht_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_ht_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_xoff_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_xoff_interrupt;
        void int_hbm_xoff_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_xoff_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_pbus_violation_in_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_pbus_violation_in_interrupt;
        void int_hbm_pbus_violation_in_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_pbus_violation_in_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_pbus_violation_out_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_pbus_violation_out_interrupt;
        void int_hbm_pbus_violation_out_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_pbus_violation_out_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_drop_interrupt;
        void int_hbm_drop_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_drop_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_stop_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_stop_interrupt;
        void int_hbm_stop_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_stop_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_axi_err_rsp_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_axi_err_rsp_interrupt;
        void int_hbm_axi_err_rsp_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_axi_err_rsp_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_emergency_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_drop_emergency_interrupt;
        void int_hbm_drop_emergency_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_drop_emergency_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_write_ack_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_drop_write_ack_interrupt;
        void int_hbm_drop_write_ack_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_drop_write_ack_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_ctrl_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_drop_ctrl_interrupt;
        void int_hbm_drop_ctrl_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_drop_ctrl_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_threshold_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_drop_threshold_interrupt;
        void int_hbm_drop_threshold_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_drop_threshold_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_hbm_truncate_no_interrupt_cpp_int_t;
        cpp_int int_var__int_hbm_truncate_no_interrupt;
        void int_hbm_truncate_no_interrupt (const cpp_int  & l__val);
        cpp_int int_hbm_truncate_no_interrupt() const;
    
}; // cap_pbchbm_csr_intreg_status_t
    
class cap_pbchbm_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pbchbm_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_mtu_enable_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_mtu_enable;
        void int_ecc_hbm_mtu_enable (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_mtu_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_rb_enable_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_rb_enable;
        void int_ecc_hbm_rb_enable (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_rb_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_wb_enable_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_wb_enable;
        void int_ecc_hbm_wb_enable (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_wb_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_hbm_ht_enable_cpp_int_t;
        cpp_int int_var__int_ecc_hbm_ht_enable;
        void int_ecc_hbm_ht_enable (const cpp_int  & l__val);
        cpp_int int_ecc_hbm_ht_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_xoff_enable_cpp_int_t;
        cpp_int int_var__int_hbm_xoff_enable;
        void int_hbm_xoff_enable (const cpp_int  & l__val);
        cpp_int int_hbm_xoff_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_pbus_violation_in_enable_cpp_int_t;
        cpp_int int_var__int_hbm_pbus_violation_in_enable;
        void int_hbm_pbus_violation_in_enable (const cpp_int  & l__val);
        cpp_int int_hbm_pbus_violation_in_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_pbus_violation_out_enable_cpp_int_t;
        cpp_int int_var__int_hbm_pbus_violation_out_enable;
        void int_hbm_pbus_violation_out_enable (const cpp_int  & l__val);
        cpp_int int_hbm_pbus_violation_out_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_enable_cpp_int_t;
        cpp_int int_var__int_hbm_drop_enable;
        void int_hbm_drop_enable (const cpp_int  & l__val);
        cpp_int int_hbm_drop_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_stop_enable_cpp_int_t;
        cpp_int int_var__int_hbm_stop_enable;
        void int_hbm_stop_enable (const cpp_int  & l__val);
        cpp_int int_hbm_stop_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_axi_err_rsp_enable_cpp_int_t;
        cpp_int int_var__int_hbm_axi_err_rsp_enable;
        void int_hbm_axi_err_rsp_enable (const cpp_int  & l__val);
        cpp_int int_hbm_axi_err_rsp_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_emergency_enable_cpp_int_t;
        cpp_int int_var__int_hbm_drop_emergency_enable;
        void int_hbm_drop_emergency_enable (const cpp_int  & l__val);
        cpp_int int_hbm_drop_emergency_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_write_ack_enable_cpp_int_t;
        cpp_int int_var__int_hbm_drop_write_ack_enable;
        void int_hbm_drop_write_ack_enable (const cpp_int  & l__val);
        cpp_int int_hbm_drop_write_ack_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_ctrl_enable_cpp_int_t;
        cpp_int int_var__int_hbm_drop_ctrl_enable;
        void int_hbm_drop_ctrl_enable (const cpp_int  & l__val);
        cpp_int int_hbm_drop_ctrl_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_drop_threshold_enable_cpp_int_t;
        cpp_int int_var__int_hbm_drop_threshold_enable;
        void int_hbm_drop_threshold_enable (const cpp_int  & l__val);
        cpp_int int_hbm_drop_threshold_enable() const;
    
        typedef pu_cpp_int< 1 > int_hbm_truncate_no_enable_cpp_int_t;
        cpp_int int_var__int_hbm_truncate_no_enable;
        void int_hbm_truncate_no_enable (const cpp_int  & l__val);
        cpp_int int_hbm_truncate_no_enable() const;
    
}; // cap_pbchbm_csr_int_groups_int_enable_rw_reg_t
    
class cap_pbchbm_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_intgrp_status_t(string name = "cap_pbchbm_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_intreg_status_t intreg;
    
        cap_pbchbm_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pbchbm_csr_intreg_status_t int_rw_reg;
    
}; // cap_pbchbm_csr_intgrp_status_t
    
class cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t(string name = "cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 48 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t
    
class cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t(string name = "cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_entry_t entry;
    
}; // cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t
    
class cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t(string name = "cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t
    
class cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t(string name = "cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_entry_t entry;
    
}; // cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t
    
class cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t(string name = "cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 11 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t
    
class cap_pbchbm_csr_dhs_hbm_wb_wm_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_wb_wm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_wb_wm_t(string name = "cap_pbchbm_csr_dhs_hbm_wb_wm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_wb_wm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_dhs_hbm_wb_wm_entry_t entry;
    
}; // cap_pbchbm_csr_dhs_hbm_wb_wm_t
    
class cap_pbchbm_csr_dhs_hbm_r2a_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_r2a_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_r2a_entry_t(string name = "cap_pbchbm_csr_dhs_hbm_r2a_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_r2a_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > command_cpp_int_t;
        cpp_int int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
        typedef pu_cpp_int< 10 > current_credit_cpp_int_t;
        cpp_int int_var__current_credit;
        void current_credit (const cpp_int  & l__val);
        cpp_int current_credit() const;
    
        typedef pu_cpp_int< 10 > quota_cpp_int_t;
        cpp_int int_var__quota;
        void quota (const cpp_int  & l__val);
        cpp_int quota() const;
    
}; // cap_pbchbm_csr_dhs_hbm_r2a_entry_t
    
class cap_pbchbm_csr_dhs_hbm_r2a_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_r2a_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_r2a_t(string name = "cap_pbchbm_csr_dhs_hbm_r2a_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_r2a_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbchbm_csr_dhs_hbm_r2a_entry_t, 3> entry;
        #else 
        cap_pbchbm_csr_dhs_hbm_r2a_entry_t entry[3];
        #endif
        int get_depth_entry() { return 3; }
    
}; // cap_pbchbm_csr_dhs_hbm_r2a_t
    
class cap_pbchbm_csr_dhs_hbm_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_mem_entry_t(string name = "cap_pbchbm_csr_dhs_hbm_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 768 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_pbchbm_csr_dhs_hbm_mem_entry_t
    
class cap_pbchbm_csr_dhs_hbm_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_mem_t(string name = "cap_pbchbm_csr_dhs_hbm_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_dhs_hbm_mem_entry_t entry;
    
}; // cap_pbchbm_csr_dhs_hbm_mem_t
    
class cap_pbchbm_csr_dhs_hbm_cdt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_cdt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_cdt_entry_t(string name = "cap_pbchbm_csr_dhs_hbm_cdt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_cdt_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > command_cpp_int_t;
        cpp_int int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
        typedef pu_cpp_int< 16 > current_credit_cpp_int_t;
        cpp_int int_var__current_credit;
        void current_credit (const cpp_int  & l__val);
        cpp_int current_credit() const;
    
        typedef pu_cpp_int< 16 > quota_cpp_int_t;
        cpp_int int_var__quota;
        void quota (const cpp_int  & l__val);
        cpp_int quota() const;
    
}; // cap_pbchbm_csr_dhs_hbm_cdt_entry_t
    
class cap_pbchbm_csr_dhs_hbm_cdt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_dhs_hbm_cdt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_dhs_hbm_cdt_t(string name = "cap_pbchbm_csr_dhs_hbm_cdt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_dhs_hbm_cdt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 88 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbchbm_csr_dhs_hbm_cdt_entry_t, 88> entry;
        #else 
        cap_pbchbm_csr_dhs_hbm_cdt_entry_t entry[88];
        #endif
        int get_depth_entry() { return 88; }
    
}; // cap_pbchbm_csr_dhs_hbm_cdt_t
    
class cap_pbchbm_csr_cnt_hbm_axi_timestamp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm_axi_timestamp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm_axi_timestamp_t(string name = "cap_pbchbm_csr_cnt_hbm_axi_timestamp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm_axi_timestamp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > write_cpp_int_t;
        cpp_int int_var__write;
        void write (const cpp_int  & l__val);
        cpp_int write() const;
    
}; // cap_pbchbm_csr_cnt_hbm_axi_timestamp_t
    
class cap_pbchbm_csr_cnt_hbm_axi_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm_axi_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm_axi_ctrl_t(string name = "cap_pbchbm_csr_cnt_hbm_axi_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm_axi_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > read_cpp_int_t;
        cpp_int int_var__read;
        void read (const cpp_int  & l__val);
        cpp_int read() const;
    
}; // cap_pbchbm_csr_cnt_hbm_axi_ctrl_t
    
class cap_pbchbm_csr_cnt_hbm_axi_pyld_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm_axi_pyld_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm_axi_pyld_t(string name = "cap_pbchbm_csr_cnt_hbm_axi_pyld_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm_axi_pyld_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > read_cpp_int_t;
        cpp_int int_var__read;
        void read (const cpp_int  & l__val);
        cpp_int read() const;
    
}; // cap_pbchbm_csr_cnt_hbm_axi_pyld_t
    
class cap_pbchbm_csr_cnt_hbm_axi_rpl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm_axi_rpl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm_axi_rpl_t(string name = "cap_pbchbm_csr_cnt_hbm_axi_rpl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm_axi_rpl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > read_cpp_int_t;
        cpp_int int_var__read;
        void read (const cpp_int  & l__val);
        cpp_int read() const;
    
}; // cap_pbchbm_csr_cnt_hbm_axi_rpl_t
    
class cap_pbchbm_csr_cfg_hbm_tx_clear_context_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_tx_clear_context_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_tx_clear_context_t(string name = "cap_pbchbm_csr_cfg_hbm_tx_clear_context_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_tx_clear_context_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > counters_cpp_int_t;
        cpp_int int_var__counters;
        void counters (const cpp_int  & l__val);
        cpp_int counters() const;
    
}; // cap_pbchbm_csr_cfg_hbm_tx_clear_context_t
    
class cap_pbchbm_csr_cfg_hbm_eth_clear_context_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_clear_context_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_clear_context_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_clear_context_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_clear_context_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > counters_cpp_int_t;
        cpp_int int_var__counters;
        void counters (const cpp_int  & l__val);
        cpp_int counters() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_clear_context_t
    
class cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > xoff_cpp_int_t;
        cpp_int int_var__xoff;
        void xoff (const cpp_int  & l__val);
        cpp_int xoff() const;
    
        typedef pu_cpp_int< 32 > xon_cpp_int_t;
        cpp_int int_var__xon;
        void xon (const cpp_int  & l__val);
        cpp_int xon() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t
    
class cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 32 > auto_xon_cpp_int_t;
        cpp_int int_var__auto_xon;
        void auto_xon (const cpp_int  & l__val);
        cpp_int auto_xon() const;
    
        typedef pu_cpp_int< 32 > auto_clear_cpp_int_t;
        cpp_int int_var__auto_clear;
        void auto_clear (const cpp_int  & l__val);
        cpp_int auto_clear() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t
    
class cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t(string name = "cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > write_cpp_int_t;
        cpp_int int_var__write;
        void write (const cpp_int  & l__val);
        cpp_int write() const;
    
        typedef pu_cpp_int< 16 > gather_cpp_int_t;
        cpp_int int_var__gather;
        void gather (const cpp_int  & l__val);
        cpp_int gather() const;
    
}; // cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t
    
class cap_pbchbm_csr_cfg_hbm_tx_flush_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_tx_flush_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_tx_flush_t(string name = "cap_pbchbm_csr_cfg_hbm_tx_flush_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_tx_flush_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > context_cpp_int_t;
        cpp_int int_var__context;
        void context (const cpp_int  & l__val);
        cpp_int context() const;
    
}; // cap_pbchbm_csr_cfg_hbm_tx_flush_t
    
class cap_pbchbm_csr_cfg_hbm_eth_flush_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_flush_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_flush_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_flush_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_flush_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > context_cpp_int_t;
        cpp_int int_var__context;
        void context (const cpp_int  & l__val);
        cpp_int context() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_flush_t
    
class cap_pbchbm_csr_cfg_hbm_tx_reset_context_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_tx_reset_context_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_tx_reset_context_t(string name = "cap_pbchbm_csr_cfg_hbm_tx_reset_context_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_tx_reset_context_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 4 > index_cpp_int_t;
        cpp_int int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
}; // cap_pbchbm_csr_cfg_hbm_tx_reset_context_t
    
class cap_pbchbm_csr_cfg_hbm_eth_reset_context_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_reset_context_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_reset_context_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_reset_context_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_reset_context_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 5 > index_cpp_int_t;
        cpp_int int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_reset_context_t
    
class cap_pbchbm_csr_cfg_hbm_qdepth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_qdepth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_qdepth_t(string name = "cap_pbchbm_csr_cfg_hbm_qdepth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_qdepth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_pbchbm_csr_cfg_hbm_qdepth_t
    
class cap_pbchbm_csr_cfg_hbm_parser_snap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_parser_snap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_parser_snap_t(string name = "cap_pbchbm_csr_cfg_hbm_parser_snap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_parser_snap_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > ctrl_oui_cpp_int_t;
        cpp_int int_var__ctrl_oui;
        void ctrl_oui (const cpp_int  & l__val);
        cpp_int ctrl_oui() const;
    
}; // cap_pbchbm_csr_cfg_hbm_parser_snap_t
    
class cap_pbchbm_csr_cfg_hbm_cut_thru_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_cut_thru_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_cut_thru_t(string name = "cap_pbchbm_csr_cfg_hbm_cut_thru_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_cut_thru_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_pbchbm_csr_cfg_hbm_cut_thru_t
    
class cap_pbchbm_csr_sta_hbm_timestamp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_timestamp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_timestamp_t(string name = "cap_pbchbm_csr_sta_hbm_timestamp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_timestamp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 48 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pbchbm_csr_sta_hbm_timestamp_t
    
class cap_pbchbm_csr_sta_hbm_tx_pend_acks_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_pend_acks_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_pend_acks_t(string name = "cap_pbchbm_csr_sta_hbm_tx_pend_acks_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_pend_acks_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 48 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_pend_acks_t
    
class cap_pbchbm_csr_sta_hbm_eth_pend_acks_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_pend_acks_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_pend_acks_t(string name = "cap_pbchbm_csr_sta_hbm_eth_pend_acks_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_pend_acks_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 96 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_pend_acks_t
    
class cap_pbchbm_csr_sta_hbm_wb_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_wb_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_wb_depth_t(string name = "cap_pbchbm_csr_sta_hbm_wb_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_wb_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 11 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pbchbm_csr_sta_hbm_wb_depth_t
    
class cap_pbchbm_csr_sat_hbm8_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm8_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm8_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm8_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm8_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm8_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm8_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm8_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm8_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm8_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm8_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm8_truncate_t
    
class cap_pbchbm_csr_cnt_hbm8_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm8_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm8_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm8_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm8_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm8_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm8_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm8_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm8_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm8_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm8_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm8_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm7_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm7_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm7_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm7_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm7_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm7_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm7_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm7_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm7_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm7_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm7_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm7_truncate_t
    
class cap_pbchbm_csr_cnt_hbm7_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm7_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm7_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm7_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm7_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm7_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm7_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm7_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm7_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm7_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm7_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm7_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm6_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm6_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm6_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm6_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm6_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm6_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm6_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm6_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm6_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm6_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm6_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm6_truncate_t
    
class cap_pbchbm_csr_cnt_hbm6_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm6_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm6_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm6_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm6_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm6_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm6_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm6_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm6_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm6_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm6_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm6_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm5_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm5_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm5_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm5_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm5_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm5_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm5_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm5_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm5_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm5_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm5_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm5_truncate_t
    
class cap_pbchbm_csr_cnt_hbm5_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm5_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm5_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm5_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm5_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm5_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm5_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm5_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm5_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm5_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm5_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm5_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm4_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm4_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm4_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm4_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm4_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm4_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm4_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm4_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm4_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm4_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm4_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm4_truncate_t
    
class cap_pbchbm_csr_cnt_hbm4_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm4_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm4_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm4_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm4_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm4_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm4_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm4_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm4_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm4_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm4_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm4_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm3_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm3_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm3_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm3_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm3_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm3_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm3_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm3_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm3_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm3_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm3_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm3_truncate_t
    
class cap_pbchbm_csr_cnt_hbm3_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm3_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm3_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm3_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm3_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm3_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm3_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm3_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm3_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm3_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm3_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm3_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm2_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm2_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm2_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm2_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm2_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm2_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm2_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm2_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm2_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm2_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm2_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm2_truncate_t
    
class cap_pbchbm_csr_cnt_hbm2_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm2_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm2_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm2_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm2_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm2_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm2_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm2_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm2_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm2_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm2_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm2_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm1_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm1_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm1_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm1_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm1_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm1_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm1_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm1_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm1_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm1_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm1_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm1_truncate_t
    
class cap_pbchbm_csr_cnt_hbm1_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm1_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm1_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm1_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm1_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm1_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm1_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm1_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm1_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm1_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm1_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm1_emergency_stop_t
    
class cap_pbchbm_csr_sat_hbm0_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sat_hbm0_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sat_hbm0_ctrl_full_t(string name = "cap_pbchbm_csr_sat_hbm0_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sat_hbm0_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_sat_hbm0_ctrl_full_t
    
class cap_pbchbm_csr_cnt_hbm0_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm0_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm0_truncate_t(string name = "cap_pbchbm_csr_cnt_hbm0_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm0_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm0_truncate_t
    
class cap_pbchbm_csr_cnt_hbm0_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm0_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm0_write_ack_full_t(string name = "cap_pbchbm_csr_cnt_hbm0_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm0_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm0_write_ack_full_t
    
class cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t(string name = "cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t
    
class cap_pbchbm_csr_cnt_hbm0_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cnt_hbm0_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cnt_hbm0_emergency_stop_t(string name = "cap_pbchbm_csr_cnt_hbm0_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cnt_hbm0_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbm_csr_cnt_hbm0_emergency_stop_t
    
class cap_pbchbm_csr_cfg_hbm_r2a_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_r2a_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_r2a_t(string name = "cap_pbchbm_csr_cfg_hbm_r2a_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_r2a_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > timer_cpp_int_t;
        cpp_int int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
        typedef pu_cpp_int< 1 > enable_wrr_cpp_int_t;
        cpp_int int_var__enable_wrr;
        void enable_wrr (const cpp_int  & l__val);
        cpp_int enable_wrr() const;
    
        typedef pu_cpp_int< 3 > strict_priority_cpp_int_t;
        cpp_int int_var__strict_priority;
        void strict_priority (const cpp_int  & l__val);
        cpp_int strict_priority() const;
    
        typedef pu_cpp_int< 3 > strict_priority_bypass_timer_cpp_int_t;
        cpp_int int_var__strict_priority_bypass_timer;
        void strict_priority_bypass_timer (const cpp_int  & l__val);
        cpp_int strict_priority_bypass_timer() const;
    
        typedef pu_cpp_int< 3 > selection_cpp_int_t;
        cpp_int int_var__selection;
        void selection (const cpp_int  & l__val);
        cpp_int selection() const;
    
        typedef pu_cpp_int< 1 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
}; // cap_pbchbm_csr_cfg_hbm_r2a_t
    
class cap_pbchbm_csr_cfg_hbm_axi_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_axi_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_axi_base_t(string name = "cap_pbchbm_csr_cfg_hbm_axi_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_axi_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbchbm_csr_cfg_hbm_axi_base_t
    
class cap_pbchbm_csr_sta_hbm_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_axi_t(string name = "cap_pbchbm_csr_sta_hbm_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_axi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > ctrl_rresp_cpp_int_t;
        cpp_int int_var__ctrl_rresp;
        void ctrl_rresp (const cpp_int  & l__val);
        cpp_int ctrl_rresp() const;
    
        typedef pu_cpp_int< 4 > ctrl_rid_cpp_int_t;
        cpp_int int_var__ctrl_rid;
        void ctrl_rid (const cpp_int  & l__val);
        cpp_int ctrl_rid() const;
    
        typedef pu_cpp_int< 7 > pyld_rid_cpp_int_t;
        cpp_int int_var__pyld_rid;
        void pyld_rid (const cpp_int  & l__val);
        cpp_int pyld_rid() const;
    
        typedef pu_cpp_int< 2 > pyld_rresp_cpp_int_t;
        cpp_int int_var__pyld_rresp;
        void pyld_rresp (const cpp_int  & l__val);
        cpp_int pyld_rresp() const;
    
        typedef pu_cpp_int< 7 > r2a_bid_cpp_int_t;
        cpp_int int_var__r2a_bid;
        void r2a_bid (const cpp_int  & l__val);
        cpp_int r2a_bid() const;
    
        typedef pu_cpp_int< 2 > r2a_bresp_cpp_int_t;
        cpp_int int_var__r2a_bresp;
        void r2a_bresp (const cpp_int  & l__val);
        cpp_int r2a_bresp() const;
    
}; // cap_pbchbm_csr_sta_hbm_axi_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_15_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_15_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_15_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_15_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_15_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_14_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_14_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_14_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_14_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_14_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_13_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_13_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_13_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_13_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_13_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_12_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_12_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_12_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_11_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_11_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_11_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_10_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_10_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_10_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_9_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_9_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_8_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_8_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_7_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_7_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_6_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_6_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_5_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_5_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_4_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_4_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_3_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_3_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_2_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_2_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_1_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_1_t
    
class cap_pbchbm_csr_sta_hbm_tx_context_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_tx_context_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_tx_context_0_t(string name = "cap_pbchbm_csr_sta_hbm_tx_context_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_tx_context_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_tx_context_0_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_31_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_31_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_31_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_31_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_31_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_31_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_30_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_30_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_30_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_30_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_30_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_30_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_29_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_29_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_29_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_29_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_29_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_29_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_28_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_28_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_28_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_28_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_28_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_28_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_27_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_27_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_27_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_27_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_27_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_27_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_26_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_26_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_26_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_26_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_26_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_26_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_25_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_25_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_25_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_25_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_25_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_25_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_24_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_24_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_24_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_24_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_24_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_24_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_23_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_23_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_23_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_23_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_23_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_23_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_22_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_22_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_22_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_22_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_22_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_22_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_21_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_21_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_21_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_21_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_21_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_21_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_20_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_20_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_20_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_20_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_20_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_20_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_19_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_19_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_19_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_19_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_19_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_19_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_18_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_18_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_18_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_18_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_18_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_18_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_17_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_17_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_17_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_17_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_17_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_17_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_16_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_16_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_16_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_16_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_16_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_16_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_15_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_15_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_15_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_15_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_15_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_14_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_14_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_14_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_14_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_14_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_13_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_13_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_13_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_13_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_13_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_12_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_12_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_12_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_11_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_11_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_11_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_10_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_10_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_10_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_9_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_9_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_8_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_8_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_7_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_7_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_6_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_6_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_5_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_5_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_4_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_4_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_3_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_3_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_2_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_2_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_1_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_1_t
    
class cap_pbchbm_csr_sta_hbm_eth_context_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_eth_context_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_eth_context_0_t(string name = "cap_pbchbm_csr_sta_hbm_eth_context_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_eth_context_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 23 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pbchbm_csr_sta_hbm_eth_context_0_t
    
class cap_pbchbm_csr_sta_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_t(string name = "cap_pbchbm_csr_sta_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > xoff_timeout_iq_cpp_int_t;
        cpp_int int_var__xoff_timeout_iq;
        void xoff_timeout_iq (const cpp_int  & l__val);
        cpp_int xoff_timeout_iq() const;
    
        typedef pu_cpp_int< 4 > xoff_timeout_port_cpp_int_t;
        cpp_int int_var__xoff_timeout_port;
        void xoff_timeout_port (const cpp_int  & l__val);
        cpp_int xoff_timeout_port() const;
    
        typedef pu_cpp_int< 1 > tx_ctrl_init_head_done_cpp_int_t;
        cpp_int int_var__tx_ctrl_init_head_done;
        void tx_ctrl_init_head_done (const cpp_int  & l__val);
        cpp_int tx_ctrl_init_head_done() const;
    
        typedef pu_cpp_int< 1 > tx_ctrl_init_tail_done_cpp_int_t;
        cpp_int int_var__tx_ctrl_init_tail_done;
        void tx_ctrl_init_tail_done (const cpp_int  & l__val);
        cpp_int tx_ctrl_init_tail_done() const;
    
        typedef pu_cpp_int< 1 > eth_ctrl_init_head_done_cpp_int_t;
        cpp_int int_var__eth_ctrl_init_head_done;
        void eth_ctrl_init_head_done (const cpp_int  & l__val);
        cpp_int eth_ctrl_init_head_done() const;
    
        typedef pu_cpp_int< 1 > eth_ctrl_init_tail_done_cpp_int_t;
        cpp_int int_var__eth_ctrl_init_tail_done;
        void eth_ctrl_init_tail_done (const cpp_int  & l__val);
        cpp_int eth_ctrl_init_tail_done() const;
    
}; // cap_pbchbm_csr_sta_hbm_t
    
class cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t(string name = "cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > head_start_cpp_int_t;
        cpp_int int_var__head_start;
        void head_start (const cpp_int  & l__val);
        cpp_int head_start() const;
    
        typedef pu_cpp_int< 1 > tail_start_cpp_int_t;
        cpp_int int_var__tail_start;
        void tail_start (const cpp_int  & l__val);
        cpp_int tail_start() const;
    
}; // cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t
    
class cap_pbchbm_csr_cfg_hbm_tx_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_tx_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_tx_ctrl_t(string name = "cap_pbchbm_csr_cfg_hbm_tx_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_tx_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 432 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 368 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbm_csr_cfg_hbm_tx_ctrl_t
    
class cap_pbchbm_csr_cfg_hbm_tx_payload_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_tx_payload_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_tx_payload_t(string name = "cap_pbchbm_csr_cfg_hbm_tx_payload_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_tx_payload_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 432 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 368 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbm_csr_cfg_hbm_tx_payload_t
    
class cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > head_start_cpp_int_t;
        cpp_int int_var__head_start;
        void head_start (const cpp_int  & l__val);
        cpp_int head_start() const;
    
        typedef pu_cpp_int< 1 > tail_start_cpp_int_t;
        cpp_int int_var__tail_start;
        void tail_start (const cpp_int  & l__val);
        cpp_int tail_start() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t
    
class cap_pbchbm_csr_cfg_hbm_eth_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_ctrl_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 864 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 736 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_ctrl_t
    
class cap_pbchbm_csr_cfg_hbm_eth_payload_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_eth_payload_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_eth_payload_t(string name = "cap_pbchbm_csr_cfg_hbm_eth_payload_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_eth_payload_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 864 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 736 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbm_csr_cfg_hbm_eth_payload_t
    
class cap_pbchbm_csr_cfg_hbm_threshold_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_threshold_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_threshold_t(string name = "cap_pbchbm_csr_cfg_hbm_threshold_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_threshold_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 640 > xoff_cpp_int_t;
        cpp_int int_var__xoff;
        void xoff (const cpp_int  & l__val);
        cpp_int xoff() const;
    
        typedef pu_cpp_int< 640 > xon_cpp_int_t;
        cpp_int int_var__xon;
        void xon (const cpp_int  & l__val);
        cpp_int xon() const;
    
}; // cap_pbchbm_csr_cfg_hbm_threshold_t
    
class cap_pbchbm_csr_sta_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_bist_t(string name = "cap_pbchbm_csr_sta_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ht_eth_tail_done_fail_cpp_int_t;
        cpp_int int_var__ht_eth_tail_done_fail;
        void ht_eth_tail_done_fail (const cpp_int  & l__val);
        cpp_int ht_eth_tail_done_fail() const;
    
        typedef pu_cpp_int< 1 > ht_eth_tail_done_pass_cpp_int_t;
        cpp_int int_var__ht_eth_tail_done_pass;
        void ht_eth_tail_done_pass (const cpp_int  & l__val);
        cpp_int ht_eth_tail_done_pass() const;
    
        typedef pu_cpp_int< 1 > ht_eth_head_done_fail_cpp_int_t;
        cpp_int int_var__ht_eth_head_done_fail;
        void ht_eth_head_done_fail (const cpp_int  & l__val);
        cpp_int ht_eth_head_done_fail() const;
    
        typedef pu_cpp_int< 1 > ht_eth_head_done_pass_cpp_int_t;
        cpp_int int_var__ht_eth_head_done_pass;
        void ht_eth_head_done_pass (const cpp_int  & l__val);
        cpp_int ht_eth_head_done_pass() const;
    
        typedef pu_cpp_int< 1 > ht_tx_tail_done_fail_cpp_int_t;
        cpp_int int_var__ht_tx_tail_done_fail;
        void ht_tx_tail_done_fail (const cpp_int  & l__val);
        cpp_int ht_tx_tail_done_fail() const;
    
        typedef pu_cpp_int< 1 > ht_tx_tail_done_pass_cpp_int_t;
        cpp_int int_var__ht_tx_tail_done_pass;
        void ht_tx_tail_done_pass (const cpp_int  & l__val);
        cpp_int ht_tx_tail_done_pass() const;
    
        typedef pu_cpp_int< 1 > ht_tx_head_done_fail_cpp_int_t;
        cpp_int int_var__ht_tx_head_done_fail;
        void ht_tx_head_done_fail (const cpp_int  & l__val);
        cpp_int ht_tx_head_done_fail() const;
    
        typedef pu_cpp_int< 1 > ht_tx_head_done_pass_cpp_int_t;
        cpp_int int_var__ht_tx_head_done_pass;
        void ht_tx_head_done_pass (const cpp_int  & l__val);
        cpp_int ht_tx_head_done_pass() const;
    
        typedef pu_cpp_int< 10 > hbm_write_fifo_done_fail_cpp_int_t;
        cpp_int int_var__hbm_write_fifo_done_fail;
        void hbm_write_fifo_done_fail (const cpp_int  & l__val);
        cpp_int hbm_write_fifo_done_fail() const;
    
        typedef pu_cpp_int< 10 > hbm_write_fifo_done_pass_cpp_int_t;
        cpp_int int_var__hbm_write_fifo_done_pass;
        void hbm_write_fifo_done_pass (const cpp_int  & l__val);
        cpp_int hbm_write_fifo_done_pass() const;
    
        typedef pu_cpp_int< 10 > hbm_read_fifo_done_fail_cpp_int_t;
        cpp_int int_var__hbm_read_fifo_done_fail;
        void hbm_read_fifo_done_fail (const cpp_int  & l__val);
        cpp_int hbm_read_fifo_done_fail() const;
    
        typedef pu_cpp_int< 10 > hbm_read_fifo_done_pass_cpp_int_t;
        cpp_int int_var__hbm_read_fifo_done_pass;
        void hbm_read_fifo_done_pass (const cpp_int  & l__val);
        cpp_int hbm_read_fifo_done_pass() const;
    
}; // cap_pbchbm_csr_sta_bist_t
    
class cap_pbchbm_csr_sta_ecc_hbm_ht_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_ecc_hbm_ht_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_ecc_hbm_ht_t(string name = "cap_pbchbm_csr_sta_ecc_hbm_ht_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_ecc_hbm_ht_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbchbm_csr_sta_ecc_hbm_ht_t
    
class cap_pbchbm_csr_cfg_hbm_ht_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_ht_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_ht_t(string name = "cap_pbchbm_csr_cfg_hbm_ht_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_ht_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > eth_head_ecc_disable_cor_cpp_int_t;
        cpp_int int_var__eth_head_ecc_disable_cor;
        void eth_head_ecc_disable_cor (const cpp_int  & l__val);
        cpp_int eth_head_ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > eth_head_ecc_disable_det_cpp_int_t;
        cpp_int int_var__eth_head_ecc_disable_det;
        void eth_head_ecc_disable_det (const cpp_int  & l__val);
        cpp_int eth_head_ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > eth_head_dhs_eccbypass_cpp_int_t;
        cpp_int int_var__eth_head_dhs_eccbypass;
        void eth_head_dhs_eccbypass (const cpp_int  & l__val);
        cpp_int eth_head_dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > eth_tail_ecc_disable_cor_cpp_int_t;
        cpp_int int_var__eth_tail_ecc_disable_cor;
        void eth_tail_ecc_disable_cor (const cpp_int  & l__val);
        cpp_int eth_tail_ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > eth_tail_ecc_disable_det_cpp_int_t;
        cpp_int int_var__eth_tail_ecc_disable_det;
        void eth_tail_ecc_disable_det (const cpp_int  & l__val);
        cpp_int eth_tail_ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > eth_tail_dhs_eccbypass_cpp_int_t;
        cpp_int int_var__eth_tail_dhs_eccbypass;
        void eth_tail_dhs_eccbypass (const cpp_int  & l__val);
        cpp_int eth_tail_dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > tx_head_dhs_eccbypass_cpp_int_t;
        cpp_int int_var__tx_head_dhs_eccbypass;
        void tx_head_dhs_eccbypass (const cpp_int  & l__val);
        cpp_int tx_head_dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > tx_tail_dhs_eccbypass_cpp_int_t;
        cpp_int int_var__tx_tail_dhs_eccbypass;
        void tx_tail_dhs_eccbypass (const cpp_int  & l__val);
        cpp_int tx_tail_dhs_eccbypass() const;
    
}; // cap_pbchbm_csr_cfg_hbm_ht_t
    
class cap_pbchbm_csr_cfg_hbm_axi_write_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_axi_write_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_axi_write_attr_t(string name = "cap_pbchbm_csr_cfg_hbm_axi_write_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_axi_write_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
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
    
}; // cap_pbchbm_csr_cfg_hbm_axi_write_attr_t
    
class cap_pbchbm_csr_cfg_hbm_axi_read_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_axi_read_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_axi_read_attr_t(string name = "cap_pbchbm_csr_cfg_hbm_axi_read_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_axi_read_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
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
    
        typedef pu_cpp_int< 4 > rpl_cache_cpp_int_t;
        cpp_int int_var__rpl_cache;
        void rpl_cache (const cpp_int  & l__val);
        cpp_int rpl_cache() const;
    
        typedef pu_cpp_int< 3 > rpl_prot_cpp_int_t;
        cpp_int int_var__rpl_prot;
        void rpl_prot (const cpp_int  & l__val);
        cpp_int rpl_prot() const;
    
        typedef pu_cpp_int< 4 > rpl_qos_cpp_int_t;
        cpp_int int_var__rpl_qos;
        void rpl_qos (const cpp_int  & l__val);
        cpp_int rpl_qos() const;
    
        typedef pu_cpp_int< 1 > rpl_lock_cpp_int_t;
        cpp_int int_var__rpl_lock;
        void rpl_lock (const cpp_int  & l__val);
        cpp_int rpl_lock() const;
    
}; // cap_pbchbm_csr_cfg_hbm_axi_read_attr_t
    
class cap_pbchbm_csr_sta_hbm_wb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_wb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_wb_t(string name = "cap_pbchbm_csr_sta_hbm_wb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_wb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 27 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 10 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_pbchbm_csr_sta_hbm_wb_t
    
class cap_pbchbm_csr_sta_hbm_rb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_hbm_rb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_hbm_rb_t(string name = "cap_pbchbm_csr_sta_hbm_rb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_hbm_rb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
        typedef pu_cpp_int< 1 > cdt_ecc_correctable_cpp_int_t;
        cpp_int int_var__cdt_ecc_correctable;
        void cdt_ecc_correctable (const cpp_int  & l__val);
        cpp_int cdt_ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > cdt_ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__cdt_ecc_uncorrectable;
        void cdt_ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int cdt_ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 1 > rb_ecc_correctable_cpp_int_t;
        cpp_int int_var__rb_ecc_correctable;
        void rb_ecc_correctable (const cpp_int  & l__val);
        cpp_int rb_ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > rb_ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__rb_ecc_uncorrectable;
        void rb_ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int rb_ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 36 > rb_ecc_syndrome_cpp_int_t;
        cpp_int int_var__rb_ecc_syndrome;
        void rb_ecc_syndrome (const cpp_int  & l__val);
        cpp_int rb_ecc_syndrome() const;
    
        typedef pu_cpp_int< 10 > rb_ecc_addr_cpp_int_t;
        cpp_int int_var__rb_ecc_addr;
        void rb_ecc_addr (const cpp_int  & l__val);
        cpp_int rb_ecc_addr() const;
    
        typedef pu_cpp_int< 7 > cdt_ecc_syndrome_cpp_int_t;
        cpp_int int_var__cdt_ecc_syndrome;
        void cdt_ecc_syndrome (const cpp_int  & l__val);
        cpp_int cdt_ecc_syndrome() const;
    
        typedef pu_cpp_int< 7 > cdt_ecc_addr_cpp_int_t;
        cpp_int int_var__cdt_ecc_addr;
        void cdt_ecc_addr (const cpp_int  & l__val);
        cpp_int cdt_ecc_addr() const;
    
        typedef pu_cpp_int< 2 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 2 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
        typedef pu_cpp_int< 1 > hct_init_done_cpp_int_t;
        cpp_int int_var__hct_init_done;
        void hct_init_done (const cpp_int  & l__val);
        cpp_int hct_init_done() const;
    
}; // cap_pbchbm_csr_sta_hbm_rb_t
    
class cap_pbchbm_csr_cfg_hbm_wb_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_wb_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_wb_drop_t(string name = "cap_pbchbm_csr_cfg_hbm_wb_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_wb_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > threshold_cpp_int_t;
        cpp_int int_var__threshold;
        void threshold (const cpp_int  & l__val);
        cpp_int threshold() const;
    
}; // cap_pbchbm_csr_cfg_hbm_wb_drop_t
    
class cap_pbchbm_csr_cfg_hbm_wb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_wb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_wb_t(string name = "cap_pbchbm_csr_cfg_hbm_wb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_wb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > xoff_threshold_cpp_int_t;
        cpp_int int_var__xoff_threshold;
        void xoff_threshold (const cpp_int  & l__val);
        cpp_int xoff_threshold() const;
    
        typedef pu_cpp_int< 10 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 3 > ack_fifo_xoff_threshold_cpp_int_t;
        cpp_int int_var__ack_fifo_xoff_threshold;
        void ack_fifo_xoff_threshold (const cpp_int  & l__val);
        cpp_int ack_fifo_xoff_threshold() const;
    
        typedef pu_cpp_int< 3 > ack_fifo_xon_threshold_cpp_int_t;
        cpp_int int_var__ack_fifo_xon_threshold;
        void ack_fifo_xon_threshold (const cpp_int  & l__val);
        cpp_int ack_fifo_xon_threshold() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
        typedef pu_cpp_int< 1 > addr_swizzle_cpp_int_t;
        cpp_int int_var__addr_swizzle;
        void addr_swizzle (const cpp_int  & l__val);
        cpp_int addr_swizzle() const;
    
}; // cap_pbchbm_csr_cfg_hbm_wb_t
    
class cap_pbchbm_csr_cfg_hbm_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_xoff_t(string name = "cap_pbchbm_csr_cfg_hbm_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 27 > timeout_cpp_int_t;
        cpp_int int_var__timeout;
        void timeout (const cpp_int  & l__val);
        cpp_int timeout() const;
    
}; // cap_pbchbm_csr_cfg_hbm_xoff_t
    
class cap_pbchbm_csr_cfg_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_t(string name = "cap_pbchbm_csr_cfg_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > dhs_mem_address_cpp_int_t;
        cpp_int int_var__dhs_mem_address;
        void dhs_mem_address (const cpp_int  & l__val);
        cpp_int dhs_mem_address() const;
    
        typedef pu_cpp_int< 4 > mtu_dhs_selection_cpp_int_t;
        cpp_int int_var__mtu_dhs_selection;
        void mtu_dhs_selection (const cpp_int  & l__val);
        cpp_int mtu_dhs_selection() const;
    
        typedef pu_cpp_int< 1 > debug_port_enable_cpp_int_t;
        cpp_int int_var__debug_port_enable;
        void debug_port_enable (const cpp_int  & l__val);
        cpp_int debug_port_enable() const;
    
        typedef pu_cpp_int< 1 > debug_port_select_cpp_int_t;
        cpp_int int_var__debug_port_select;
        void debug_port_select (const cpp_int  & l__val);
        cpp_int debug_port_select() const;
    
        typedef pu_cpp_int< 1 > bist_run_tx_head_cpp_int_t;
        cpp_int int_var__bist_run_tx_head;
        void bist_run_tx_head (const cpp_int  & l__val);
        cpp_int bist_run_tx_head() const;
    
        typedef pu_cpp_int< 1 > bist_run_tx_tail_cpp_int_t;
        cpp_int int_var__bist_run_tx_tail;
        void bist_run_tx_tail (const cpp_int  & l__val);
        cpp_int bist_run_tx_tail() const;
    
        typedef pu_cpp_int< 1 > bist_run_eth_head_cpp_int_t;
        cpp_int int_var__bist_run_eth_head;
        void bist_run_eth_head (const cpp_int  & l__val);
        cpp_int bist_run_eth_head() const;
    
        typedef pu_cpp_int< 1 > bist_run_eth_tail_cpp_int_t;
        cpp_int int_var__bist_run_eth_tail;
        void bist_run_eth_tail (const cpp_int  & l__val);
        cpp_int bist_run_eth_tail() const;
    
        typedef pu_cpp_int< 1 > control_sw_reset_cpp_int_t;
        cpp_int int_var__control_sw_reset;
        void control_sw_reset (const cpp_int  & l__val);
        cpp_int control_sw_reset() const;
    
}; // cap_pbchbm_csr_cfg_hbm_t
    
class cap_pbchbm_csr_cfg_hbm_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_spare_t(string name = "cap_pbchbm_csr_cfg_hbm_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbchbm_csr_cfg_hbm_spare_t
    
class cap_pbchbm_csr_cfg_hbm_rb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_rb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_rb_t(string name = "cap_pbchbm_csr_cfg_hbm_rb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_rb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > mem_ecc_disable_cor_cpp_int_t;
        cpp_int int_var__mem_ecc_disable_cor;
        void mem_ecc_disable_cor (const cpp_int  & l__val);
        cpp_int mem_ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > mem_ecc_disable_det_cpp_int_t;
        cpp_int int_var__mem_ecc_disable_det;
        void mem_ecc_disable_det (const cpp_int  & l__val);
        cpp_int mem_ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > mem_dhs_eccbypass_cpp_int_t;
        cpp_int int_var__mem_dhs_eccbypass;
        void mem_dhs_eccbypass (const cpp_int  & l__val);
        cpp_int mem_dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > enable_wrr_cpp_int_t;
        cpp_int int_var__enable_wrr;
        void enable_wrr (const cpp_int  & l__val);
        cpp_int enable_wrr() const;
    
        typedef pu_cpp_int< 1 > cdt_dhs_selection_cpp_int_t;
        cpp_int int_var__cdt_dhs_selection;
        void cdt_dhs_selection (const cpp_int  & l__val);
        cpp_int cdt_dhs_selection() const;
    
        typedef pu_cpp_int< 1 > hct_dhs_eccbypass_cpp_int_t;
        cpp_int int_var__hct_dhs_eccbypass;
        void hct_dhs_eccbypass (const cpp_int  & l__val);
        cpp_int hct_dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > ecc_hct_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_hct_disable_cor;
        void ecc_hct_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_hct_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_hct_disable_det_cpp_int_t;
        cpp_int int_var__ecc_hct_disable_det;
        void ecc_hct_disable_det (const cpp_int  & l__val);
        cpp_int ecc_hct_disable_det() const;
    
        typedef pu_cpp_int< 2 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
        typedef pu_cpp_int< 1 > hct_init_start_cpp_int_t;
        cpp_int int_var__hct_init_start;
        void hct_init_start (const cpp_int  & l__val);
        cpp_int hct_init_start() const;
    
        typedef pu_cpp_int< 1 > addr_swizzle_cpp_int_t;
        cpp_int int_var__addr_swizzle;
        void addr_swizzle (const cpp_int  & l__val);
        cpp_int addr_swizzle() const;
    
        typedef pu_cpp_int< 2 > max_arlen_cpp_int_t;
        cpp_int int_var__max_arlen;
        void max_arlen (const cpp_int  & l__val);
        cpp_int max_arlen() const;
    
}; // cap_pbchbm_csr_cfg_hbm_rb_t
    
class cap_pbchbm_csr_cfg_hbm_cdt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_cfg_hbm_cdt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_cfg_hbm_cdt_t(string name = "cap_pbchbm_csr_cfg_hbm_cdt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_cfg_hbm_cdt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > cfg_hbm_cdt_cpp_int_t;
        cpp_int int_var__cfg_hbm_cdt;
        void cfg_hbm_cdt (const cpp_int  & l__val);
        cpp_int cfg_hbm_cdt() const;
    
}; // cap_pbchbm_csr_cfg_hbm_cdt_t
    
class cap_pbchbm_csr_sta_ecc_hbm_mtu_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_sta_ecc_hbm_mtu_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_sta_ecc_hbm_mtu_t(string name = "cap_pbchbm_csr_sta_ecc_hbm_mtu_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_sta_ecc_hbm_mtu_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbchbm_csr_sta_ecc_hbm_mtu_t
    
class cap_pbchbm_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_csr_intr_t(string name = "cap_pbchbm_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbchbm_csr_csr_intr_t
    
class cap_pbchbm_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbm_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbm_csr_t(string name = "cap_pbchbm_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbm_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbm_csr_csr_intr_t csr_intr;
    
        cap_pbchbm_csr_sta_ecc_hbm_mtu_t sta_ecc_hbm_mtu;
    
        cap_pbchbm_csr_cfg_hbm_cdt_t cfg_hbm_cdt;
    
        cap_pbchbm_csr_cfg_hbm_rb_t cfg_hbm_rb;
    
        cap_pbchbm_csr_cfg_hbm_spare_t cfg_hbm_spare;
    
        cap_pbchbm_csr_cfg_hbm_t cfg_hbm;
    
        cap_pbchbm_csr_cfg_hbm_xoff_t cfg_hbm_xoff;
    
        cap_pbchbm_csr_cfg_hbm_wb_t cfg_hbm_wb;
    
        cap_pbchbm_csr_cfg_hbm_wb_drop_t cfg_hbm_wb_drop;
    
        cap_pbchbm_csr_sta_hbm_rb_t sta_hbm_rb;
    
        cap_pbchbm_csr_sta_hbm_wb_t sta_hbm_wb;
    
        cap_pbchbm_csr_cfg_hbm_axi_read_attr_t cfg_hbm_axi_read_attr;
    
        cap_pbchbm_csr_cfg_hbm_axi_write_attr_t cfg_hbm_axi_write_attr;
    
        cap_pbchbm_csr_cfg_hbm_ht_t cfg_hbm_ht;
    
        cap_pbchbm_csr_sta_ecc_hbm_ht_t sta_ecc_hbm_ht;
    
        cap_pbchbm_csr_sta_bist_t sta_bist;
    
        cap_pbchbm_csr_cfg_hbm_threshold_t cfg_hbm_threshold;
    
        cap_pbchbm_csr_cfg_hbm_eth_payload_t cfg_hbm_eth_payload;
    
        cap_pbchbm_csr_cfg_hbm_eth_ctrl_t cfg_hbm_eth_ctrl;
    
        cap_pbchbm_csr_cfg_hbm_eth_ctrl_init_t cfg_hbm_eth_ctrl_init;
    
        cap_pbchbm_csr_cfg_hbm_tx_payload_t cfg_hbm_tx_payload;
    
        cap_pbchbm_csr_cfg_hbm_tx_ctrl_t cfg_hbm_tx_ctrl;
    
        cap_pbchbm_csr_cfg_hbm_tx_ctrl_init_t cfg_hbm_tx_ctrl_init;
    
        cap_pbchbm_csr_sta_hbm_t sta_hbm;
    
        cap_pbchbm_csr_sta_hbm_eth_context_0_t sta_hbm_eth_context_0;
    
        cap_pbchbm_csr_sta_hbm_eth_context_1_t sta_hbm_eth_context_1;
    
        cap_pbchbm_csr_sta_hbm_eth_context_2_t sta_hbm_eth_context_2;
    
        cap_pbchbm_csr_sta_hbm_eth_context_3_t sta_hbm_eth_context_3;
    
        cap_pbchbm_csr_sta_hbm_eth_context_4_t sta_hbm_eth_context_4;
    
        cap_pbchbm_csr_sta_hbm_eth_context_5_t sta_hbm_eth_context_5;
    
        cap_pbchbm_csr_sta_hbm_eth_context_6_t sta_hbm_eth_context_6;
    
        cap_pbchbm_csr_sta_hbm_eth_context_7_t sta_hbm_eth_context_7;
    
        cap_pbchbm_csr_sta_hbm_eth_context_8_t sta_hbm_eth_context_8;
    
        cap_pbchbm_csr_sta_hbm_eth_context_9_t sta_hbm_eth_context_9;
    
        cap_pbchbm_csr_sta_hbm_eth_context_10_t sta_hbm_eth_context_10;
    
        cap_pbchbm_csr_sta_hbm_eth_context_11_t sta_hbm_eth_context_11;
    
        cap_pbchbm_csr_sta_hbm_eth_context_12_t sta_hbm_eth_context_12;
    
        cap_pbchbm_csr_sta_hbm_eth_context_13_t sta_hbm_eth_context_13;
    
        cap_pbchbm_csr_sta_hbm_eth_context_14_t sta_hbm_eth_context_14;
    
        cap_pbchbm_csr_sta_hbm_eth_context_15_t sta_hbm_eth_context_15;
    
        cap_pbchbm_csr_sta_hbm_eth_context_16_t sta_hbm_eth_context_16;
    
        cap_pbchbm_csr_sta_hbm_eth_context_17_t sta_hbm_eth_context_17;
    
        cap_pbchbm_csr_sta_hbm_eth_context_18_t sta_hbm_eth_context_18;
    
        cap_pbchbm_csr_sta_hbm_eth_context_19_t sta_hbm_eth_context_19;
    
        cap_pbchbm_csr_sta_hbm_eth_context_20_t sta_hbm_eth_context_20;
    
        cap_pbchbm_csr_sta_hbm_eth_context_21_t sta_hbm_eth_context_21;
    
        cap_pbchbm_csr_sta_hbm_eth_context_22_t sta_hbm_eth_context_22;
    
        cap_pbchbm_csr_sta_hbm_eth_context_23_t sta_hbm_eth_context_23;
    
        cap_pbchbm_csr_sta_hbm_eth_context_24_t sta_hbm_eth_context_24;
    
        cap_pbchbm_csr_sta_hbm_eth_context_25_t sta_hbm_eth_context_25;
    
        cap_pbchbm_csr_sta_hbm_eth_context_26_t sta_hbm_eth_context_26;
    
        cap_pbchbm_csr_sta_hbm_eth_context_27_t sta_hbm_eth_context_27;
    
        cap_pbchbm_csr_sta_hbm_eth_context_28_t sta_hbm_eth_context_28;
    
        cap_pbchbm_csr_sta_hbm_eth_context_29_t sta_hbm_eth_context_29;
    
        cap_pbchbm_csr_sta_hbm_eth_context_30_t sta_hbm_eth_context_30;
    
        cap_pbchbm_csr_sta_hbm_eth_context_31_t sta_hbm_eth_context_31;
    
        cap_pbchbm_csr_sta_hbm_tx_context_0_t sta_hbm_tx_context_0;
    
        cap_pbchbm_csr_sta_hbm_tx_context_1_t sta_hbm_tx_context_1;
    
        cap_pbchbm_csr_sta_hbm_tx_context_2_t sta_hbm_tx_context_2;
    
        cap_pbchbm_csr_sta_hbm_tx_context_3_t sta_hbm_tx_context_3;
    
        cap_pbchbm_csr_sta_hbm_tx_context_4_t sta_hbm_tx_context_4;
    
        cap_pbchbm_csr_sta_hbm_tx_context_5_t sta_hbm_tx_context_5;
    
        cap_pbchbm_csr_sta_hbm_tx_context_6_t sta_hbm_tx_context_6;
    
        cap_pbchbm_csr_sta_hbm_tx_context_7_t sta_hbm_tx_context_7;
    
        cap_pbchbm_csr_sta_hbm_tx_context_8_t sta_hbm_tx_context_8;
    
        cap_pbchbm_csr_sta_hbm_tx_context_9_t sta_hbm_tx_context_9;
    
        cap_pbchbm_csr_sta_hbm_tx_context_10_t sta_hbm_tx_context_10;
    
        cap_pbchbm_csr_sta_hbm_tx_context_11_t sta_hbm_tx_context_11;
    
        cap_pbchbm_csr_sta_hbm_tx_context_12_t sta_hbm_tx_context_12;
    
        cap_pbchbm_csr_sta_hbm_tx_context_13_t sta_hbm_tx_context_13;
    
        cap_pbchbm_csr_sta_hbm_tx_context_14_t sta_hbm_tx_context_14;
    
        cap_pbchbm_csr_sta_hbm_tx_context_15_t sta_hbm_tx_context_15;
    
        cap_pbchbm_csr_sta_hbm_axi_t sta_hbm_axi;
    
        cap_pbchbm_csr_cfg_hbm_axi_base_t cfg_hbm_axi_base;
    
        cap_pbchbm_csr_cfg_hbm_r2a_t cfg_hbm_r2a;
    
        cap_pbchbm_csr_cnt_hbm0_emergency_stop_t cnt_hbm0_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm0_write_ack_filling_up_t cnt_hbm0_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm0_write_ack_full_t cnt_hbm0_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm0_truncate_t cnt_hbm0_truncate;
    
        cap_pbchbm_csr_sat_hbm0_ctrl_full_t sat_hbm0_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm1_emergency_stop_t cnt_hbm1_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm1_write_ack_filling_up_t cnt_hbm1_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm1_write_ack_full_t cnt_hbm1_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm1_truncate_t cnt_hbm1_truncate;
    
        cap_pbchbm_csr_sat_hbm1_ctrl_full_t sat_hbm1_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm2_emergency_stop_t cnt_hbm2_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm2_write_ack_filling_up_t cnt_hbm2_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm2_write_ack_full_t cnt_hbm2_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm2_truncate_t cnt_hbm2_truncate;
    
        cap_pbchbm_csr_sat_hbm2_ctrl_full_t sat_hbm2_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm3_emergency_stop_t cnt_hbm3_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm3_write_ack_filling_up_t cnt_hbm3_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm3_write_ack_full_t cnt_hbm3_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm3_truncate_t cnt_hbm3_truncate;
    
        cap_pbchbm_csr_sat_hbm3_ctrl_full_t sat_hbm3_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm4_emergency_stop_t cnt_hbm4_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm4_write_ack_filling_up_t cnt_hbm4_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm4_write_ack_full_t cnt_hbm4_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm4_truncate_t cnt_hbm4_truncate;
    
        cap_pbchbm_csr_sat_hbm4_ctrl_full_t sat_hbm4_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm5_emergency_stop_t cnt_hbm5_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm5_write_ack_filling_up_t cnt_hbm5_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm5_write_ack_full_t cnt_hbm5_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm5_truncate_t cnt_hbm5_truncate;
    
        cap_pbchbm_csr_sat_hbm5_ctrl_full_t sat_hbm5_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm6_emergency_stop_t cnt_hbm6_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm6_write_ack_filling_up_t cnt_hbm6_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm6_write_ack_full_t cnt_hbm6_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm6_truncate_t cnt_hbm6_truncate;
    
        cap_pbchbm_csr_sat_hbm6_ctrl_full_t sat_hbm6_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm7_emergency_stop_t cnt_hbm7_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm7_write_ack_filling_up_t cnt_hbm7_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm7_write_ack_full_t cnt_hbm7_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm7_truncate_t cnt_hbm7_truncate;
    
        cap_pbchbm_csr_sat_hbm7_ctrl_full_t sat_hbm7_ctrl_full;
    
        cap_pbchbm_csr_cnt_hbm8_emergency_stop_t cnt_hbm8_emergency_stop;
    
        cap_pbchbm_csr_cnt_hbm8_write_ack_filling_up_t cnt_hbm8_write_ack_filling_up;
    
        cap_pbchbm_csr_cnt_hbm8_write_ack_full_t cnt_hbm8_write_ack_full;
    
        cap_pbchbm_csr_cnt_hbm8_truncate_t cnt_hbm8_truncate;
    
        cap_pbchbm_csr_sat_hbm8_ctrl_full_t sat_hbm8_ctrl_full;
    
        cap_pbchbm_csr_sta_hbm_wb_depth_t sta_hbm_wb_depth;
    
        cap_pbchbm_csr_sta_hbm_eth_pend_acks_t sta_hbm_eth_pend_acks;
    
        cap_pbchbm_csr_sta_hbm_tx_pend_acks_t sta_hbm_tx_pend_acks;
    
        cap_pbchbm_csr_sta_hbm_timestamp_t sta_hbm_timestamp;
    
        cap_pbchbm_csr_cfg_hbm_cut_thru_t cfg_hbm_cut_thru;
    
        cap_pbchbm_csr_cfg_hbm_parser_snap_t cfg_hbm_parser_snap;
    
        cap_pbchbm_csr_cfg_hbm_qdepth_t cfg_hbm_qdepth;
    
        cap_pbchbm_csr_cfg_hbm_eth_reset_context_t cfg_hbm_eth_reset_context;
    
        cap_pbchbm_csr_cfg_hbm_tx_reset_context_t cfg_hbm_tx_reset_context;
    
        cap_pbchbm_csr_cfg_hbm_eth_flush_t cfg_hbm_eth_flush;
    
        cap_pbchbm_csr_cfg_hbm_tx_flush_t cfg_hbm_tx_flush;
    
        cap_pbchbm_csr_cfg_hbm_wb_duty_cycle_t cfg_hbm_wb_duty_cycle;
    
        cap_pbchbm_csr_cfg_hbm_eth_xoff_timeout_t cfg_hbm_eth_xoff_timeout;
    
        cap_pbchbm_csr_cfg_hbm_eth_xoff_force_t cfg_hbm_eth_xoff_force;
    
        cap_pbchbm_csr_cfg_hbm_eth_clear_context_t cfg_hbm_eth_clear_context;
    
        cap_pbchbm_csr_cfg_hbm_tx_clear_context_t cfg_hbm_tx_clear_context;
    
        cap_pbchbm_csr_cnt_hbm_axi_rpl_t cnt_hbm_axi_rpl;
    
        cap_pbchbm_csr_cnt_hbm_axi_pyld_t cnt_hbm_axi_pyld;
    
        cap_pbchbm_csr_cnt_hbm_axi_ctrl_t cnt_hbm_axi_ctrl;
    
        cap_pbchbm_csr_cnt_hbm_axi_timestamp_t cnt_hbm_axi_timestamp;
    
        cap_pbchbm_csr_dhs_hbm_cdt_t dhs_hbm_cdt;
    
        cap_pbchbm_csr_dhs_hbm_mem_t dhs_hbm_mem;
    
        cap_pbchbm_csr_dhs_hbm_r2a_t dhs_hbm_r2a;
    
        cap_pbchbm_csr_dhs_hbm_wb_wm_t dhs_hbm_wb_wm;
    
        cap_pbchbm_csr_dhs_hbm_eth_pend_acks_wm_t dhs_hbm_eth_pend_acks_wm;
    
        cap_pbchbm_csr_dhs_hbm_tx_pend_acks_wm_t dhs_hbm_tx_pend_acks_wm;
    
        cap_pbchbmeth_csr_t hbm_port_0;
    
        cap_pbchbmeth_csr_t hbm_port_1;
    
        cap_pbchbmeth_csr_t hbm_port_2;
    
        cap_pbchbmeth_csr_t hbm_port_3;
    
        cap_pbchbmeth_csr_t hbm_port_4;
    
        cap_pbchbmeth_csr_t hbm_port_5;
    
        cap_pbchbmeth_csr_t hbm_port_6;
    
        cap_pbchbmeth_csr_t hbm_port_7;
    
        cap_pbchbmeth_csr_t hbm_port_8;
    
        cap_pbchbmtx_csr_t hbm_port_9;
    
        cap_pbchbm_csr_intgrp_status_t int_groups;
    
        cap_pbchbm_csr_int_ecc_hbm_mtu_t int_ecc_hbm_mtu;
    
        cap_pbchbm_csr_int_ecc_hbm_rb_t int_ecc_hbm_rb;
    
        cap_pbchbm_csr_int_ecc_hbm_mtu_t int_ecc_hbm_wb;
    
        cap_pbchbm_csr_int_ecc_hbm_mtu_t int_ecc_hbm_ht;
    
        cap_pbchbm_csr_int_hbm_xoff_t int_hbm_xoff;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_in_t int_hbm_pbus_violation_in;
    
        cap_pbchbm_csr_int_hbm_pbus_violation_out_t int_hbm_pbus_violation_out;
    
        cap_pbchbm_csr_int_hbm_drop_t int_hbm_drop;
    
        cap_pbchbm_csr_int_hbm_axi_err_rsp_t int_hbm_axi_err_rsp;
    
        cap_pbchbm_csr_int_hbm_drop_emergency_t int_hbm_drop_emergency;
    
        cap_pbchbm_csr_int_hbm_drop_write_ack_t int_hbm_drop_write_ack;
    
        cap_pbchbm_csr_int_hbm_drop_ctrl_t int_hbm_drop_ctrl;
    
        cap_pbchbm_csr_int_hbm_drop_threshold_t int_hbm_drop_threshold;
    
        cap_pbchbm_csr_int_hbm_truncate_no_t int_hbm_truncate_no;
    
        cap_pbchbm_csr_int_hbm_stop_t int_hbm_stop;
    
}; // cap_pbchbm_csr_t
    
#endif // CAP_PBCHBM_CSR_H
        