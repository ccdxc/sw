
#ifndef CAP_PICS_CSR_H
#define CAP_PICS_CSR_H

#include "cap_csr_base.h" 
#include "cap_picc_csr.h" 

using namespace std;
class cap_pics_csr_int_bg_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_bg_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_bg_int_enable_clear_t(string name = "cap_pics_csr_int_bg_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_bg_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > unfinished_bg0_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg0_enable;
        void unfinished_bg0_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg0_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg1_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg1_enable;
        void unfinished_bg1_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg1_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg2_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg2_enable;
        void unfinished_bg2_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg2_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg3_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg3_enable;
        void unfinished_bg3_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg3_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg4_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg4_enable;
        void unfinished_bg4_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg4_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg5_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg5_enable;
        void unfinished_bg5_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg5_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg6_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg6_enable;
        void unfinished_bg6_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg6_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg7_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg7_enable;
        void unfinished_bg7_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg7_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg8_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg8_enable;
        void unfinished_bg8_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg8_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg9_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg9_enable;
        void unfinished_bg9_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg9_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg10_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg10_enable;
        void unfinished_bg10_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg10_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg11_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg11_enable;
        void unfinished_bg11_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg11_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg12_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg12_enable;
        void unfinished_bg12_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg12_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg13_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg13_enable;
        void unfinished_bg13_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg13_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg14_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg14_enable;
        void unfinished_bg14_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg14_enable() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg15_enable_cpp_int_t;
        cpp_int int_var__unfinished_bg15_enable;
        void unfinished_bg15_enable (const cpp_int  & l__val);
        cpp_int unfinished_bg15_enable() const;
    
}; // cap_pics_csr_int_bg_int_enable_clear_t
    
class cap_pics_csr_int_bg_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_bg_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_bg_int_test_set_t(string name = "cap_pics_csr_int_bg_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_bg_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > unfinished_bg0_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg0_interrupt;
        void unfinished_bg0_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg0_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg1_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg1_interrupt;
        void unfinished_bg1_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg1_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg2_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg2_interrupt;
        void unfinished_bg2_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg2_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg3_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg3_interrupt;
        void unfinished_bg3_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg3_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg4_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg4_interrupt;
        void unfinished_bg4_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg4_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg5_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg5_interrupt;
        void unfinished_bg5_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg5_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg6_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg6_interrupt;
        void unfinished_bg6_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg6_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg7_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg7_interrupt;
        void unfinished_bg7_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg7_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg8_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg8_interrupt;
        void unfinished_bg8_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg8_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg9_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg9_interrupt;
        void unfinished_bg9_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg9_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg10_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg10_interrupt;
        void unfinished_bg10_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg10_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg11_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg11_interrupt;
        void unfinished_bg11_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg11_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg12_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg12_interrupt;
        void unfinished_bg12_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg12_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg13_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg13_interrupt;
        void unfinished_bg13_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg13_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg14_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg14_interrupt;
        void unfinished_bg14_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg14_interrupt() const;
    
        typedef pu_cpp_int< 1 > unfinished_bg15_interrupt_cpp_int_t;
        cpp_int int_var__unfinished_bg15_interrupt;
        void unfinished_bg15_interrupt (const cpp_int  & l__val);
        cpp_int unfinished_bg15_interrupt() const;
    
}; // cap_pics_csr_int_bg_int_test_set_t
    
class cap_pics_csr_int_bg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_bg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_bg_t(string name = "cap_pics_csr_int_bg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_bg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_int_bg_int_test_set_t intreg;
    
        cap_pics_csr_int_bg_int_test_set_t int_test_set;
    
        cap_pics_csr_int_bg_int_enable_clear_t int_enable_set;
    
        cap_pics_csr_int_bg_int_enable_clear_t int_enable_clear;
    
}; // cap_pics_csr_int_bg_t
    
class cap_pics_csr_int_badaddr_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_badaddr_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_badaddr_int_enable_clear_t(string name = "cap_pics_csr_int_badaddr_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_badaddr_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > rdreq0_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq0_bad_addr_enable;
        void rdreq0_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq0_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq0_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq0_bad_addr_enable;
        void wrreq0_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq0_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq1_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq1_bad_addr_enable;
        void rdreq1_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq1_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq1_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq1_bad_addr_enable;
        void wrreq1_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq1_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq2_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq2_bad_addr_enable;
        void rdreq2_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq2_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq2_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq2_bad_addr_enable;
        void wrreq2_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq2_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq3_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq3_bad_addr_enable;
        void rdreq3_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq3_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq3_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq3_bad_addr_enable;
        void wrreq3_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq3_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq4_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq4_bad_addr_enable;
        void rdreq4_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq4_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq4_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq4_bad_addr_enable;
        void wrreq4_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq4_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq5_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq5_bad_addr_enable;
        void rdreq5_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq5_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq5_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq5_bad_addr_enable;
        void wrreq5_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq5_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq6_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq6_bad_addr_enable;
        void rdreq6_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq6_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq6_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq6_bad_addr_enable;
        void wrreq6_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq6_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq7_bad_addr_enable_cpp_int_t;
        cpp_int int_var__rdreq7_bad_addr_enable;
        void rdreq7_bad_addr_enable (const cpp_int  & l__val);
        cpp_int rdreq7_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq7_bad_addr_enable_cpp_int_t;
        cpp_int int_var__wrreq7_bad_addr_enable;
        void wrreq7_bad_addr_enable (const cpp_int  & l__val);
        cpp_int wrreq7_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > cpu_bad_addr_enable_cpp_int_t;
        cpp_int int_var__cpu_bad_addr_enable;
        void cpu_bad_addr_enable (const cpp_int  & l__val);
        cpp_int cpu_bad_addr_enable() const;
    
        typedef pu_cpp_int< 1 > bg_bad_addr_enable_cpp_int_t;
        cpp_int int_var__bg_bad_addr_enable;
        void bg_bad_addr_enable (const cpp_int  & l__val);
        cpp_int bg_bad_addr_enable() const;
    
}; // cap_pics_csr_int_badaddr_int_enable_clear_t
    
class cap_pics_csr_int_badaddr_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_badaddr_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_badaddr_int_test_set_t(string name = "cap_pics_csr_int_badaddr_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_badaddr_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > rdreq0_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq0_bad_addr_interrupt;
        void rdreq0_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq0_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq0_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq0_bad_addr_interrupt;
        void wrreq0_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq0_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq1_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq1_bad_addr_interrupt;
        void rdreq1_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq1_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq1_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq1_bad_addr_interrupt;
        void wrreq1_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq1_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq2_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq2_bad_addr_interrupt;
        void rdreq2_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq2_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq2_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq2_bad_addr_interrupt;
        void wrreq2_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq2_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq3_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq3_bad_addr_interrupt;
        void rdreq3_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq3_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq3_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq3_bad_addr_interrupt;
        void wrreq3_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq3_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq4_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq4_bad_addr_interrupt;
        void rdreq4_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq4_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq4_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq4_bad_addr_interrupt;
        void wrreq4_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq4_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq5_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq5_bad_addr_interrupt;
        void rdreq5_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq5_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq5_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq5_bad_addr_interrupt;
        void wrreq5_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq5_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq6_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq6_bad_addr_interrupt;
        void rdreq6_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq6_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq6_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq6_bad_addr_interrupt;
        void wrreq6_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq6_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq7_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__rdreq7_bad_addr_interrupt;
        void rdreq7_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int rdreq7_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq7_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__wrreq7_bad_addr_interrupt;
        void wrreq7_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int wrreq7_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpu_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__cpu_bad_addr_interrupt;
        void cpu_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int cpu_bad_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > bg_bad_addr_interrupt_cpp_int_t;
        cpp_int int_var__bg_bad_addr_interrupt;
        void bg_bad_addr_interrupt (const cpp_int  & l__val);
        cpp_int bg_bad_addr_interrupt() const;
    
}; // cap_pics_csr_int_badaddr_int_test_set_t
    
class cap_pics_csr_int_badaddr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_badaddr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_badaddr_t(string name = "cap_pics_csr_int_badaddr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_badaddr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_int_badaddr_int_test_set_t intreg;
    
        cap_pics_csr_int_badaddr_int_test_set_t int_test_set;
    
        cap_pics_csr_int_badaddr_int_enable_clear_t int_enable_set;
    
        cap_pics_csr_int_badaddr_int_enable_clear_t int_enable_clear;
    
}; // cap_pics_csr_int_badaddr_t
    
class cap_pics_csr_int_pics_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_pics_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_pics_int_enable_clear_t(string name = "cap_pics_csr_int_pics_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_pics_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > uncorrectable_ecc_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_ecc_enable;
        void uncorrectable_ecc_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > correctable_ecc_enable_cpp_int_t;
        cpp_int int_var__correctable_ecc_enable;
        void correctable_ecc_enable (const cpp_int  & l__val);
        cpp_int correctable_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > too_many_rl_sch_error_enable_cpp_int_t;
        cpp_int int_var__too_many_rl_sch_error_enable;
        void too_many_rl_sch_error_enable (const cpp_int  & l__val);
        cpp_int too_many_rl_sch_error_enable() const;
    
        typedef pu_cpp_int< 1 > picc_enable_cpp_int_t;
        cpp_int int_var__picc_enable;
        void picc_enable (const cpp_int  & l__val);
        cpp_int picc_enable() const;
    
}; // cap_pics_csr_int_pics_int_enable_clear_t
    
class cap_pics_csr_int_pics_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_pics_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_pics_intreg_t(string name = "cap_pics_csr_int_pics_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_pics_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > uncorrectable_ecc_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_ecc_interrupt;
        void uncorrectable_ecc_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > correctable_ecc_interrupt_cpp_int_t;
        cpp_int int_var__correctable_ecc_interrupt;
        void correctable_ecc_interrupt (const cpp_int  & l__val);
        cpp_int correctable_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > too_many_rl_sch_error_interrupt_cpp_int_t;
        cpp_int int_var__too_many_rl_sch_error_interrupt;
        void too_many_rl_sch_error_interrupt (const cpp_int  & l__val);
        cpp_int too_many_rl_sch_error_interrupt() const;
    
        typedef pu_cpp_int< 1 > picc_interrupt_cpp_int_t;
        cpp_int int_var__picc_interrupt;
        void picc_interrupt (const cpp_int  & l__val);
        cpp_int picc_interrupt() const;
    
}; // cap_pics_csr_int_pics_intreg_t
    
class cap_pics_csr_int_pics_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_pics_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_pics_t(string name = "cap_pics_csr_int_pics_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_pics_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_int_pics_intreg_t intreg;
    
        cap_pics_csr_int_pics_intreg_t int_test_set;
    
        cap_pics_csr_int_pics_int_enable_clear_t int_enable_set;
    
        cap_pics_csr_int_pics_int_enable_clear_t int_enable_clear;
    
}; // cap_pics_csr_int_pics_t
    
class cap_pics_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_intreg_status_t(string name = "cap_pics_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_pics_interrupt_cpp_int_t;
        cpp_int int_var__int_pics_interrupt;
        void int_pics_interrupt (const cpp_int  & l__val);
        cpp_int int_pics_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_badaddr_interrupt_cpp_int_t;
        cpp_int int_var__int_badaddr_interrupt;
        void int_badaddr_interrupt (const cpp_int  & l__val);
        cpp_int int_badaddr_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_bg_interrupt_cpp_int_t;
        cpp_int int_var__int_bg_interrupt;
        void int_bg_interrupt (const cpp_int  & l__val);
        cpp_int int_bg_interrupt() const;
    
}; // cap_pics_csr_intreg_status_t
    
class cap_pics_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pics_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_pics_enable_cpp_int_t;
        cpp_int int_var__int_pics_enable;
        void int_pics_enable (const cpp_int  & l__val);
        cpp_int int_pics_enable() const;
    
        typedef pu_cpp_int< 1 > int_badaddr_enable_cpp_int_t;
        cpp_int int_var__int_badaddr_enable;
        void int_badaddr_enable (const cpp_int  & l__val);
        cpp_int int_badaddr_enable() const;
    
        typedef pu_cpp_int< 1 > int_bg_enable_cpp_int_t;
        cpp_int int_var__int_bg_enable;
        void int_bg_enable (const cpp_int  & l__val);
        cpp_int int_bg_enable() const;
    
}; // cap_pics_csr_int_groups_int_enable_rw_reg_t
    
class cap_pics_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_intgrp_status_t(string name = "cap_pics_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_intreg_status_t intreg;
    
        cap_pics_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pics_csr_intreg_status_t int_rw_reg;
    
}; // cap_pics_csr_intgrp_status_t
    
class cap_pics_csr_dhs_sram_update_data_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_sram_update_data_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_sram_update_data_entry_t(string name = "cap_pics_csr_dhs_sram_update_data_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_sram_update_data_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 128 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_pics_csr_dhs_sram_update_data_entry_t
    
class cap_pics_csr_dhs_sram_update_data_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_sram_update_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_sram_update_data_t(string name = "cap_pics_csr_dhs_sram_update_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_sram_update_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_dhs_sram_update_data_entry_t entry;
    
}; // cap_pics_csr_dhs_sram_update_data_t
    
class cap_pics_csr_dhs_sram_update_addr_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_sram_update_addr_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_sram_update_addr_entry_t(string name = "cap_pics_csr_dhs_sram_update_addr_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_sram_update_addr_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > address_cpp_int_t;
        cpp_int int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_pics_csr_dhs_sram_update_addr_entry_t
    
class cap_pics_csr_dhs_sram_update_addr_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_sram_update_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_sram_update_addr_t(string name = "cap_pics_csr_dhs_sram_update_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_sram_update_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_dhs_sram_update_addr_entry_t entry;
    
}; // cap_pics_csr_dhs_sram_update_addr_t
    
class cap_pics_csr_dhs_bg_sm_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_bg_sm_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_bg_sm_entry_t(string name = "cap_pics_csr_dhs_bg_sm_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_bg_sm_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > active_cpp_int_t;
        cpp_int int_var__active;
        void active (const cpp_int  & l__val);
        cpp_int active() const;
    
        typedef pu_cpp_int< 16 > timer_cpp_int_t;
        cpp_int int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
        typedef pu_cpp_int< 20 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pics_csr_dhs_bg_sm_entry_t
    
class cap_pics_csr_dhs_bg_sm_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_bg_sm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_bg_sm_t(string name = "cap_pics_csr_dhs_bg_sm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_bg_sm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_dhs_bg_sm_entry_t, 16> entry;
        #else 
        cap_pics_csr_dhs_bg_sm_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_pics_csr_dhs_bg_sm_t
    
class cap_pics_csr_dhs_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_sram_entry_t(string name = "cap_pics_csr_dhs_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pics_csr_dhs_sram_entry_t
    
class cap_pics_csr_dhs_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_dhs_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_dhs_sram_t(string name = "cap_pics_csr_dhs_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_dhs_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 40960 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_dhs_sram_entry_t, 40960> entry;
        #else 
        cap_pics_csr_dhs_sram_entry_t entry[40960];
        #endif
        int get_depth_entry() { return 40960; }
    
}; // cap_pics_csr_dhs_sram_t
    
class cap_pics_csr_cnt_axi_bw_mon_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_bw_mon_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_bw_mon_wr_t(string name = "cap_pics_csr_cnt_axi_bw_mon_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_bw_mon_wr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_bw_mon_wr_t
    
class cap_pics_csr_sta_axi_bw_mon_wr_transactions_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_axi_bw_mon_wr_transactions_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_axi_bw_mon_wr_transactions_t(string name = "cap_pics_csr_sta_axi_bw_mon_wr_transactions_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_axi_bw_mon_wr_transactions_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_sta_axi_bw_mon_wr_transactions_t
    
class cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t(string name = "cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t
    
class cap_pics_csr_sta_axi_bw_mon_wr_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_axi_bw_mon_wr_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_axi_bw_mon_wr_latency_t(string name = "cap_pics_csr_sta_axi_bw_mon_wr_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_axi_bw_mon_wr_latency_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_sta_axi_bw_mon_wr_latency_t
    
class cap_pics_csr_cnt_axi_bw_mon_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_bw_mon_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_bw_mon_rd_t(string name = "cap_pics_csr_cnt_axi_bw_mon_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_bw_mon_rd_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_bw_mon_rd_t
    
class cap_pics_csr_sta_axi_bw_mon_rd_transactions_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_axi_bw_mon_rd_transactions_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_axi_bw_mon_rd_transactions_t(string name = "cap_pics_csr_sta_axi_bw_mon_rd_transactions_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_axi_bw_mon_rd_transactions_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_sta_axi_bw_mon_rd_transactions_t
    
class cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t(string name = "cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t
    
class cap_pics_csr_sta_axi_bw_mon_rd_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_axi_bw_mon_rd_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_axi_bw_mon_rd_latency_t(string name = "cap_pics_csr_sta_axi_bw_mon_rd_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_axi_bw_mon_rd_latency_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_sta_axi_bw_mon_rd_latency_t
    
class cap_pics_csr_cfg_axi_bw_mon_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_axi_bw_mon_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_axi_bw_mon_t(string name = "cap_pics_csr_cfg_axi_bw_mon_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_axi_bw_mon_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cfg_axi_bw_mon_t
    
class cap_pics_csr_sta_wrrsp_fifo_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_wrrsp_fifo_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_wrrsp_fifo_depth_t(string name = "cap_pics_csr_sta_wrrsp_fifo_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_wrrsp_fifo_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > fifo0_cpp_int_t;
        cpp_int int_var__fifo0;
        void fifo0 (const cpp_int  & l__val);
        cpp_int fifo0() const;
    
        typedef pu_cpp_int< 4 > fifo1_cpp_int_t;
        cpp_int int_var__fifo1;
        void fifo1 (const cpp_int  & l__val);
        cpp_int fifo1() const;
    
        typedef pu_cpp_int< 4 > fifo2_cpp_int_t;
        cpp_int int_var__fifo2;
        void fifo2 (const cpp_int  & l__val);
        cpp_int fifo2() const;
    
        typedef pu_cpp_int< 4 > fifo3_cpp_int_t;
        cpp_int int_var__fifo3;
        void fifo3 (const cpp_int  & l__val);
        cpp_int fifo3() const;
    
        typedef pu_cpp_int< 4 > fifo4_cpp_int_t;
        cpp_int int_var__fifo4;
        void fifo4 (const cpp_int  & l__val);
        cpp_int fifo4() const;
    
        typedef pu_cpp_int< 4 > fifo5_cpp_int_t;
        cpp_int int_var__fifo5;
        void fifo5 (const cpp_int  & l__val);
        cpp_int fifo5() const;
    
        typedef pu_cpp_int< 4 > fifo6_cpp_int_t;
        cpp_int int_var__fifo6;
        void fifo6 (const cpp_int  & l__val);
        cpp_int fifo6() const;
    
        typedef pu_cpp_int< 4 > fifo7_cpp_int_t;
        cpp_int int_var__fifo7;
        void fifo7 (const cpp_int  & l__val);
        cpp_int fifo7() const;
    
}; // cap_pics_csr_sta_wrrsp_fifo_depth_t
    
class cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t(string name = "cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > fifo0_cpp_int_t;
        cpp_int int_var__fifo0;
        void fifo0 (const cpp_int  & l__val);
        cpp_int fifo0() const;
    
        typedef pu_cpp_int< 2 > fifo1_cpp_int_t;
        cpp_int int_var__fifo1;
        void fifo1 (const cpp_int  & l__val);
        cpp_int fifo1() const;
    
        typedef pu_cpp_int< 2 > fifo2_cpp_int_t;
        cpp_int int_var__fifo2;
        void fifo2 (const cpp_int  & l__val);
        cpp_int fifo2() const;
    
        typedef pu_cpp_int< 2 > fifo3_cpp_int_t;
        cpp_int int_var__fifo3;
        void fifo3 (const cpp_int  & l__val);
        cpp_int fifo3() const;
    
        typedef pu_cpp_int< 2 > fifo4_cpp_int_t;
        cpp_int int_var__fifo4;
        void fifo4 (const cpp_int  & l__val);
        cpp_int fifo4() const;
    
        typedef pu_cpp_int< 2 > fifo5_cpp_int_t;
        cpp_int int_var__fifo5;
        void fifo5 (const cpp_int  & l__val);
        cpp_int fifo5() const;
    
        typedef pu_cpp_int< 2 > fifo6_cpp_int_t;
        cpp_int int_var__fifo6;
        void fifo6 (const cpp_int  & l__val);
        cpp_int fifo6() const;
    
        typedef pu_cpp_int< 2 > fifo7_cpp_int_t;
        cpp_int int_var__fifo7;
        void fifo7 (const cpp_int  & l__val);
        cpp_int fifo7() const;
    
}; // cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t
    
class cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t(string name = "cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > fifo0_cpp_int_t;
        cpp_int int_var__fifo0;
        void fifo0 (const cpp_int  & l__val);
        cpp_int fifo0() const;
    
        typedef pu_cpp_int< 2 > fifo1_cpp_int_t;
        cpp_int int_var__fifo1;
        void fifo1 (const cpp_int  & l__val);
        cpp_int fifo1() const;
    
        typedef pu_cpp_int< 2 > fifo2_cpp_int_t;
        cpp_int int_var__fifo2;
        void fifo2 (const cpp_int  & l__val);
        cpp_int fifo2() const;
    
        typedef pu_cpp_int< 2 > fifo3_cpp_int_t;
        cpp_int int_var__fifo3;
        void fifo3 (const cpp_int  & l__val);
        cpp_int fifo3() const;
    
        typedef pu_cpp_int< 2 > fifo4_cpp_int_t;
        cpp_int int_var__fifo4;
        void fifo4 (const cpp_int  & l__val);
        cpp_int fifo4() const;
    
        typedef pu_cpp_int< 2 > fifo5_cpp_int_t;
        cpp_int int_var__fifo5;
        void fifo5 (const cpp_int  & l__val);
        cpp_int fifo5() const;
    
        typedef pu_cpp_int< 2 > fifo6_cpp_int_t;
        cpp_int int_var__fifo6;
        void fifo6 (const cpp_int  & l__val);
        cpp_int fifo6() const;
    
        typedef pu_cpp_int< 2 > fifo7_cpp_int_t;
        cpp_int int_var__fifo7;
        void fifo7 (const cpp_int  & l__val);
        cpp_int fifo7() const;
    
}; // cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t
    
class cap_pics_csr_sta_rdrsp_fifo_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_rdrsp_fifo_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_rdrsp_fifo_depth_t(string name = "cap_pics_csr_sta_rdrsp_fifo_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_rdrsp_fifo_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 9 > fifo0_cpp_int_t;
        cpp_int int_var__fifo0;
        void fifo0 (const cpp_int  & l__val);
        cpp_int fifo0() const;
    
        typedef pu_cpp_int< 9 > fifo1_cpp_int_t;
        cpp_int int_var__fifo1;
        void fifo1 (const cpp_int  & l__val);
        cpp_int fifo1() const;
    
        typedef pu_cpp_int< 9 > fifo2_cpp_int_t;
        cpp_int int_var__fifo2;
        void fifo2 (const cpp_int  & l__val);
        cpp_int fifo2() const;
    
        typedef pu_cpp_int< 9 > fifo3_cpp_int_t;
        cpp_int int_var__fifo3;
        void fifo3 (const cpp_int  & l__val);
        cpp_int fifo3() const;
    
        typedef pu_cpp_int< 9 > fifo4_cpp_int_t;
        cpp_int int_var__fifo4;
        void fifo4 (const cpp_int  & l__val);
        cpp_int fifo4() const;
    
        typedef pu_cpp_int< 9 > fifo5_cpp_int_t;
        cpp_int int_var__fifo5;
        void fifo5 (const cpp_int  & l__val);
        cpp_int fifo5() const;
    
        typedef pu_cpp_int< 9 > fifo6_cpp_int_t;
        cpp_int int_var__fifo6;
        void fifo6 (const cpp_int  & l__val);
        cpp_int fifo6() const;
    
        typedef pu_cpp_int< 9 > fifo7_cpp_int_t;
        cpp_int int_var__fifo7;
        void fifo7 (const cpp_int  & l__val);
        cpp_int fifo7() const;
    
}; // cap_pics_csr_sta_rdrsp_fifo_depth_t
    
class cap_pics_csr_sta_rdreq_arb_fifo_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_rdreq_arb_fifo_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_rdreq_arb_fifo_depth_t(string name = "cap_pics_csr_sta_rdreq_arb_fifo_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_rdreq_arb_fifo_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > fifo0_cpp_int_t;
        cpp_int int_var__fifo0;
        void fifo0 (const cpp_int  & l__val);
        cpp_int fifo0() const;
    
        typedef pu_cpp_int< 8 > fifo1_cpp_int_t;
        cpp_int int_var__fifo1;
        void fifo1 (const cpp_int  & l__val);
        cpp_int fifo1() const;
    
        typedef pu_cpp_int< 8 > fifo2_cpp_int_t;
        cpp_int int_var__fifo2;
        void fifo2 (const cpp_int  & l__val);
        cpp_int fifo2() const;
    
        typedef pu_cpp_int< 8 > fifo3_cpp_int_t;
        cpp_int int_var__fifo3;
        void fifo3 (const cpp_int  & l__val);
        cpp_int fifo3() const;
    
        typedef pu_cpp_int< 8 > fifo4_cpp_int_t;
        cpp_int int_var__fifo4;
        void fifo4 (const cpp_int  & l__val);
        cpp_int fifo4() const;
    
        typedef pu_cpp_int< 8 > fifo5_cpp_int_t;
        cpp_int int_var__fifo5;
        void fifo5 (const cpp_int  & l__val);
        cpp_int fifo5() const;
    
        typedef pu_cpp_int< 8 > fifo6_cpp_int_t;
        cpp_int int_var__fifo6;
        void fifo6 (const cpp_int  & l__val);
        cpp_int fifo6() const;
    
        typedef pu_cpp_int< 8 > fifo7_cpp_int_t;
        cpp_int int_var__fifo7;
        void fifo7 (const cpp_int  & l__val);
        cpp_int fifo7() const;
    
}; // cap_pics_csr_sta_rdreq_arb_fifo_depth_t
    
class cap_pics_csr_sta_wrreq_bad_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_wrreq_bad_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_wrreq_bad_addr_t(string name = "cap_pics_csr_sta_wrreq_bad_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_wrreq_bad_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > src_cpp_int_t;
        cpp_int int_var__src;
        void src (const cpp_int  & l__val);
        cpp_int src() const;
    
        typedef pu_cpp_int< 20 > tbl_addr_cpp_int_t;
        cpp_int int_var__tbl_addr;
        void tbl_addr (const cpp_int  & l__val);
        cpp_int tbl_addr() const;
    
}; // cap_pics_csr_sta_wrreq_bad_addr_t
    
class cap_pics_csr_sta_rdreq_bad_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_rdreq_bad_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_rdreq_bad_addr_t(string name = "cap_pics_csr_sta_rdreq_bad_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_rdreq_bad_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > src_cpp_int_t;
        cpp_int int_var__src;
        void src (const cpp_int  & l__val);
        cpp_int src() const;
    
        typedef pu_cpp_int< 20 > tbl_addr_cpp_int_t;
        cpp_int int_var__tbl_addr;
        void tbl_addr (const cpp_int  & l__val);
        cpp_int tbl_addr() const;
    
}; // cap_pics_csr_sta_rdreq_bad_addr_t
    
class cap_pics_csr_sta_cpu_bad_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_cpu_bad_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_cpu_bad_addr_t(string name = "cap_pics_csr_sta_cpu_bad_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_cpu_bad_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pics_csr_sta_cpu_bad_addr_t
    
class cap_pics_csr_sta_bg_bad_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_bg_bad_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_bg_bad_addr_t(string name = "cap_pics_csr_sta_bg_bad_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_bg_bad_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 20 > start_addr_cpp_int_t;
        cpp_int int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
}; // cap_pics_csr_sta_bg_bad_addr_t
    
class cap_pics_csr_sta_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_ecc_t(string name = "cap_pics_csr_sta_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 16 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pics_csr_sta_ecc_t
    
class cap_pics_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_csr_intr_t(string name = "cap_pics_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_csr_intr_t
    
class cap_pics_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_debug_port_t(string name = "cap_pics_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 4 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_pics_csr_cfg_debug_port_t
    
class cap_pics_csr_cnt_soc_rl_msg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_soc_rl_msg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_soc_rl_msg_t(string name = "cap_pics_csr_cnt_soc_rl_msg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_soc_rl_msg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_soc_rl_msg_t
    
class cap_pics_csr_cnt_sram_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_sram_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_sram_write_t(string name = "cap_pics_csr_cnt_sram_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_sram_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_sram_write_t
    
class cap_pics_csr_cnt_sram_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_sram_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_sram_read_t(string name = "cap_pics_csr_cnt_sram_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_sram_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_sram_read_t
    
class cap_pics_csr_cnt_axi_por_wrrsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_por_wrrsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_por_wrrsp_t(string name = "cap_pics_csr_cnt_axi_por_wrrsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_por_wrrsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_por_wrrsp_t
    
class cap_pics_csr_cnt_axi_por_wrreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_por_wrreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_por_wrreq_t(string name = "cap_pics_csr_cnt_axi_por_wrreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_por_wrreq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_por_wrreq_t
    
class cap_pics_csr_cnt_axi_por_rdrsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_por_rdrsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_por_rdrsp_t(string name = "cap_pics_csr_cnt_axi_por_rdrsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_por_rdrsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_por_rdrsp_t
    
class cap_pics_csr_cnt_axi_por_rmwreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_por_rmwreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_por_rmwreq_t(string name = "cap_pics_csr_cnt_axi_por_rmwreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_por_rmwreq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_por_rmwreq_t
    
class cap_pics_csr_cnt_axi_por_rdreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cnt_axi_por_rdreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cnt_axi_por_rdreq_t(string name = "cap_pics_csr_cnt_axi_por_rdreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cnt_axi_por_rdreq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_cnt_axi_por_rdreq_t
    
class cap_pics_csr_cfg_scheduler_rl_disable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_scheduler_rl_disable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_scheduler_rl_disable_t(string name = "cap_pics_csr_cfg_scheduler_rl_disable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_scheduler_rl_disable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_pics_csr_cfg_scheduler_rl_disable_t
    
class cap_pics_csr_cfg_scheduler_rl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_scheduler_rl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_scheduler_rl_t(string name = "cap_pics_csr_cfg_scheduler_rl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_scheduler_rl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 11 > address_offset_cpp_int_t;
        cpp_int int_var__address_offset;
        void address_offset (const cpp_int  & l__val);
        cpp_int address_offset() const;
    
}; // cap_pics_csr_cfg_scheduler_rl_t
    
class cap_pics_csr_sta_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_sta_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_sta_sram_t(string name = "cap_pics_csr_sta_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_sta_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 10 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_pics_csr_sta_sram_t
    
class cap_pics_csr_cfg_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_sram_t(string name = "cap_pics_csr_cfg_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 10 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 10 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 10 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pics_csr_cfg_sram_t
    
class cap_pics_csr_cfg_bg_update_profile_enable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_bg_update_profile_enable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_bg_update_profile_enable_t(string name = "cap_pics_csr_cfg_bg_update_profile_enable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_bg_update_profile_enable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > vector_cpp_int_t;
        cpp_int int_var__vector;
        void vector (const cpp_int  & l__val);
        cpp_int vector() const;
    
}; // cap_pics_csr_cfg_bg_update_profile_enable_t
    
class cap_pics_csr_cfg_bg_update_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_bg_update_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_bg_update_profile_t(string name = "cap_pics_csr_cfg_bg_update_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_bg_update_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > start_addr_cpp_int_t;
        cpp_int int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 20 > end_addr_cpp_int_t;
        cpp_int int_var__end_addr;
        void end_addr (const cpp_int  & l__val);
        cpp_int end_addr() const;
    
        typedef pu_cpp_int< 6 > scale_cpp_int_t;
        cpp_int int_var__scale;
        void scale (const cpp_int  & l__val);
        cpp_int scale() const;
    
        typedef pu_cpp_int< 16 > timer_cpp_int_t;
        cpp_int int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
        typedef pu_cpp_int< 11 > opcode_cpp_int_t;
        cpp_int int_var__opcode;
        void opcode (const cpp_int  & l__val);
        cpp_int opcode() const;
    
        typedef pu_cpp_int< 8 > max_cycle_cpp_int_t;
        cpp_int int_var__max_cycle;
        void max_cycle (const cpp_int  & l__val);
        cpp_int max_cycle() const;
    
        typedef pu_cpp_int< 1 > rlimit_en_cpp_int_t;
        cpp_int int_var__rlimit_en;
        void rlimit_en (const cpp_int  & l__val);
        cpp_int rlimit_en() const;
    
        typedef pu_cpp_int< 1 > mode_cpp_int_t;
        cpp_int int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
}; // cap_pics_csr_cfg_bg_update_profile_t
    
class cap_pics_csr_cfg_table_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_table_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_table_profile_t(string name = "cap_pics_csr_cfg_table_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_table_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 20 > start_addr_cpp_int_t;
        cpp_int int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 20 > end_addr_cpp_int_t;
        cpp_int int_var__end_addr;
        void end_addr (const cpp_int  & l__val);
        cpp_int end_addr() const;
    
        typedef pu_cpp_int< 6 > width_cpp_int_t;
        cpp_int int_var__width;
        void width (const cpp_int  & l__val);
        cpp_int width() const;
    
        typedef pu_cpp_int< 1 > hash_cpp_int_t;
        cpp_int int_var__hash;
        void hash (const cpp_int  & l__val);
        cpp_int hash() const;
    
        typedef pu_cpp_int< 11 > opcode_cpp_int_t;
        cpp_int int_var__opcode;
        void opcode (const cpp_int  & l__val);
        cpp_int opcode() const;
    
        typedef pu_cpp_int< 3 > log2bkts_cpp_int_t;
        cpp_int int_var__log2bkts;
        void log2bkts (const cpp_int  & l__val);
        cpp_int log2bkts() const;
    
        typedef pu_cpp_int< 5 > axishift_cpp_int_t;
        cpp_int int_var__axishift;
        void axishift (const cpp_int  & l__val);
        cpp_int axishift() const;
    
        typedef pu_cpp_int< 1 > rlimit_en_cpp_int_t;
        cpp_int int_var__rlimit_en;
        void rlimit_en (const cpp_int  & l__val);
        cpp_int rlimit_en() const;
    
}; // cap_pics_csr_cfg_table_profile_t
    
class cap_pics_csr_cfg_stage_id_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_stage_id_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_stage_id_t(string name = "cap_pics_csr_cfg_stage_id_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_stage_id_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > val0_cpp_int_t;
        cpp_int int_var__val0;
        void val0 (const cpp_int  & l__val);
        cpp_int val0() const;
    
        typedef pu_cpp_int< 3 > val1_cpp_int_t;
        cpp_int int_var__val1;
        void val1 (const cpp_int  & l__val);
        cpp_int val1() const;
    
        typedef pu_cpp_int< 3 > val2_cpp_int_t;
        cpp_int int_var__val2;
        void val2 (const cpp_int  & l__val);
        cpp_int val2() const;
    
        typedef pu_cpp_int< 3 > val3_cpp_int_t;
        cpp_int int_var__val3;
        void val3 (const cpp_int  & l__val);
        cpp_int val3() const;
    
        typedef pu_cpp_int< 3 > val4_cpp_int_t;
        cpp_int int_var__val4;
        void val4 (const cpp_int  & l__val);
        cpp_int val4() const;
    
        typedef pu_cpp_int< 3 > val5_cpp_int_t;
        cpp_int int_var__val5;
        void val5 (const cpp_int  & l__val);
        cpp_int val5() const;
    
        typedef pu_cpp_int< 3 > val6_cpp_int_t;
        cpp_int int_var__val6;
        void val6 (const cpp_int  & l__val);
        cpp_int val6() const;
    
        typedef pu_cpp_int< 3 > val7_cpp_int_t;
        cpp_int int_var__val7;
        void val7 (const cpp_int  & l__val);
        cpp_int val7() const;
    
}; // cap_pics_csr_cfg_stage_id_t
    
class cap_pics_csr_cfg_cpu_req_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_cfg_cpu_req_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_cfg_cpu_req_timeout_t(string name = "cap_pics_csr_cfg_cpu_req_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_cfg_cpu_req_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > max_cycle_cpp_int_t;
        cpp_int int_var__max_cycle;
        void max_cycle (const cpp_int  & l__val);
        cpp_int max_cycle() const;
    
}; // cap_pics_csr_cfg_cpu_req_timeout_t
    
class cap_pics_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_base_t(string name = "cap_pics_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pics_csr_base_t
    
class cap_pics_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pics_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pics_csr_t(string name = "cap_pics_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pics_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pics_csr_base_t base;
    
        cap_pics_csr_cfg_cpu_req_timeout_t cfg_cpu_req_timeout;
    
        cap_pics_csr_cfg_stage_id_t cfg_stage_id;
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cfg_table_profile_t, 128> cfg_table_profile;
        #else 
        cap_pics_csr_cfg_table_profile_t cfg_table_profile[128];
        #endif
        int get_depth_cfg_table_profile() { return 128; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cfg_bg_update_profile_t, 16> cfg_bg_update_profile;
        #else 
        cap_pics_csr_cfg_bg_update_profile_t cfg_bg_update_profile[16];
        #endif
        int get_depth_cfg_bg_update_profile() { return 16; }
    
        cap_pics_csr_cfg_bg_update_profile_enable_t cfg_bg_update_profile_enable;
    
        cap_pics_csr_cfg_sram_t cfg_sram;
    
        cap_pics_csr_sta_sram_t sta_sram;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cfg_scheduler_rl_t, 4> cfg_scheduler_rl;
        #else 
        cap_pics_csr_cfg_scheduler_rl_t cfg_scheduler_rl[4];
        #endif
        int get_depth_cfg_scheduler_rl() { return 4; }
    
        cap_pics_csr_cfg_scheduler_rl_disable_t cfg_scheduler_rl_disable;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_axi_por_rdreq_t, 8> cnt_axi_por_rdreq;
        #else 
        cap_pics_csr_cnt_axi_por_rdreq_t cnt_axi_por_rdreq[8];
        #endif
        int get_depth_cnt_axi_por_rdreq() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_axi_por_rmwreq_t, 8> cnt_axi_por_rmwreq;
        #else 
        cap_pics_csr_cnt_axi_por_rmwreq_t cnt_axi_por_rmwreq[8];
        #endif
        int get_depth_cnt_axi_por_rmwreq() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_axi_por_rdrsp_t, 8> cnt_axi_por_rdrsp;
        #else 
        cap_pics_csr_cnt_axi_por_rdrsp_t cnt_axi_por_rdrsp[8];
        #endif
        int get_depth_cnt_axi_por_rdrsp() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_axi_por_wrreq_t, 8> cnt_axi_por_wrreq;
        #else 
        cap_pics_csr_cnt_axi_por_wrreq_t cnt_axi_por_wrreq[8];
        #endif
        int get_depth_cnt_axi_por_wrreq() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_axi_por_wrrsp_t, 8> cnt_axi_por_wrrsp;
        #else 
        cap_pics_csr_cnt_axi_por_wrrsp_t cnt_axi_por_wrrsp[8];
        #endif
        int get_depth_cnt_axi_por_wrrsp() { return 8; }
    
        #if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_sram_read_t, 10> cnt_sram_read;
        #else 
        cap_pics_csr_cnt_sram_read_t cnt_sram_read[10];
        #endif
        int get_depth_cnt_sram_read() { return 10; }
    
        #if 10 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pics_csr_cnt_sram_write_t, 10> cnt_sram_write;
        #else 
        cap_pics_csr_cnt_sram_write_t cnt_sram_write[10];
        #endif
        int get_depth_cnt_sram_write() { return 10; }
    
        cap_pics_csr_cnt_soc_rl_msg_t cnt_soc_rl_msg;
    
        cap_pics_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pics_csr_csr_intr_t csr_intr;
    
        cap_pics_csr_sta_ecc_t sta_ecc;
    
        cap_pics_csr_sta_bg_bad_addr_t sta_bg_bad_addr;
    
        cap_pics_csr_sta_cpu_bad_addr_t sta_cpu_bad_addr;
    
        cap_pics_csr_sta_rdreq_bad_addr_t sta_rdreq_bad_addr;
    
        cap_pics_csr_sta_wrreq_bad_addr_t sta_wrreq_bad_addr;
    
        cap_pics_csr_sta_rdreq_arb_fifo_depth_t sta_rdreq_arb_fifo_depth;
    
        cap_pics_csr_sta_rdrsp_fifo_depth_t sta_rdrsp_fifo_depth;
    
        cap_pics_csr_sta_wrreq_addr_arb_fifo_depth_t sta_wrreq_addr_arb_fifo_depth;
    
        cap_pics_csr_sta_wrreq_data_arb_fifo_depth_t sta_wrreq_data_arb_fifo_depth;
    
        cap_pics_csr_sta_wrrsp_fifo_depth_t sta_wrrsp_fifo_depth;
    
        cap_pics_csr_cfg_axi_bw_mon_t cfg_axi_bw_mon;
    
        cap_pics_csr_sta_axi_bw_mon_rd_latency_t sta_axi_bw_mon_rd_latency;
    
        cap_pics_csr_sta_axi_bw_mon_rd_bandwidth_t sta_axi_bw_mon_rd_bandwidth;
    
        cap_pics_csr_sta_axi_bw_mon_rd_transactions_t sta_axi_bw_mon_rd_transactions;
    
        cap_pics_csr_cnt_axi_bw_mon_rd_t cnt_axi_bw_mon_rd;
    
        cap_pics_csr_sta_axi_bw_mon_wr_latency_t sta_axi_bw_mon_wr_latency;
    
        cap_pics_csr_sta_axi_bw_mon_wr_bandwidth_t sta_axi_bw_mon_wr_bandwidth;
    
        cap_pics_csr_sta_axi_bw_mon_wr_transactions_t sta_axi_bw_mon_wr_transactions;
    
        cap_pics_csr_cnt_axi_bw_mon_wr_t cnt_axi_bw_mon_wr;
    
        cap_pics_csr_dhs_sram_t dhs_sram;
    
        cap_pics_csr_dhs_bg_sm_t dhs_bg_sm;
    
        cap_pics_csr_dhs_sram_update_addr_t dhs_sram_update_addr;
    
        cap_pics_csr_dhs_sram_update_data_t dhs_sram_update_data;
    
        cap_picc_csr_t picc;
    
        cap_pics_csr_intgrp_status_t int_groups;
    
        cap_pics_csr_int_pics_t int_pics;
    
        cap_pics_csr_int_badaddr_t int_badaddr;
    
        cap_pics_csr_int_bg_t int_bg;
    
}; // cap_pics_csr_t
    
#endif // CAP_PICS_CSR_H
        