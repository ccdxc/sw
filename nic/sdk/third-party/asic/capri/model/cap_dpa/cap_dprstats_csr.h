
#ifndef CAP_DPRSTATS_CSR_H
#define CAP_DPRSTATS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dprstats_csr_CNT_dpr_spare_byte_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_byte_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_byte_3_t(string name = "cap_dprstats_csr_CNT_dpr_spare_byte_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_byte_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 56 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dprstats_csr_CNT_dpr_spare_byte_3_t
    
class cap_dprstats_csr_CNT_dpr_spare_byte_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_byte_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_byte_2_t(string name = "cap_dprstats_csr_CNT_dpr_spare_byte_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_byte_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 56 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dprstats_csr_CNT_dpr_spare_byte_2_t
    
class cap_dprstats_csr_CNT_dpr_spare_byte_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_byte_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_byte_1_t(string name = "cap_dprstats_csr_CNT_dpr_spare_byte_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_byte_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 56 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dprstats_csr_CNT_dpr_spare_byte_1_t
    
class cap_dprstats_csr_CNT_dpr_spare_byte_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_byte_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_byte_0_t(string name = "cap_dprstats_csr_CNT_dpr_spare_byte_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_byte_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 56 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_dprstats_csr_CNT_dpr_spare_byte_0_t
    
class cap_dprstats_csr_CNT_dpr_spare_15_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_15_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_15_t(string name = "cap_dprstats_csr_CNT_dpr_spare_15_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_15_t
    
class cap_dprstats_csr_CNT_dpr_spare_14_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_14_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_14_t(string name = "cap_dprstats_csr_CNT_dpr_spare_14_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_14_t
    
class cap_dprstats_csr_CNT_dpr_spare_13_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_13_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_13_t(string name = "cap_dprstats_csr_CNT_dpr_spare_13_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_13_t
    
class cap_dprstats_csr_CNT_dpr_spare_12_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_12_t(string name = "cap_dprstats_csr_CNT_dpr_spare_12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_12_t
    
class cap_dprstats_csr_CNT_dpr_spare_11_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_11_t(string name = "cap_dprstats_csr_CNT_dpr_spare_11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_11_t
    
class cap_dprstats_csr_CNT_dpr_spare_10_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_10_t(string name = "cap_dprstats_csr_CNT_dpr_spare_10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_10_t
    
class cap_dprstats_csr_CNT_dpr_spare_9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_9_t(string name = "cap_dprstats_csr_CNT_dpr_spare_9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_9_t
    
class cap_dprstats_csr_CNT_dpr_spare_8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_8_t(string name = "cap_dprstats_csr_CNT_dpr_spare_8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_8_t
    
class cap_dprstats_csr_CNT_dpr_spare_7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_7_t(string name = "cap_dprstats_csr_CNT_dpr_spare_7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_7_t
    
class cap_dprstats_csr_CNT_dpr_spare_6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_6_t(string name = "cap_dprstats_csr_CNT_dpr_spare_6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_6_t
    
class cap_dprstats_csr_CNT_dpr_spare_5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_5_t(string name = "cap_dprstats_csr_CNT_dpr_spare_5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_5_t
    
class cap_dprstats_csr_CNT_dpr_spare_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_4_t(string name = "cap_dprstats_csr_CNT_dpr_spare_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_4_t
    
class cap_dprstats_csr_CNT_dpr_spare_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_3_t(string name = "cap_dprstats_csr_CNT_dpr_spare_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_3_t
    
class cap_dprstats_csr_CNT_dpr_spare_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_2_t(string name = "cap_dprstats_csr_CNT_dpr_spare_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_2_t
    
class cap_dprstats_csr_CNT_dpr_spare_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_1_t(string name = "cap_dprstats_csr_CNT_dpr_spare_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_1_t
    
class cap_dprstats_csr_CNT_dpr_spare_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_spare_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_spare_0_t(string name = "cap_dprstats_csr_CNT_dpr_spare_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_spare_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_spare_0_t
    
class cap_dprstats_csr_CNT_dpr_csum_rw_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_csum_rw_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_csum_rw_4_t(string name = "cap_dprstats_csr_CNT_dpr_csum_rw_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_csum_rw_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_csum_rw_4_t
    
class cap_dprstats_csr_CNT_dpr_csum_rw_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_csum_rw_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_csum_rw_3_t(string name = "cap_dprstats_csr_CNT_dpr_csum_rw_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_csum_rw_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_csum_rw_3_t
    
class cap_dprstats_csr_CNT_dpr_csum_rw_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_csum_rw_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_csum_rw_2_t(string name = "cap_dprstats_csr_CNT_dpr_csum_rw_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_csum_rw_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_csum_rw_2_t
    
class cap_dprstats_csr_CNT_dpr_csum_rw_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_csum_rw_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_csum_rw_1_t(string name = "cap_dprstats_csr_CNT_dpr_csum_rw_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_csum_rw_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_csum_rw_1_t
    
class cap_dprstats_csr_CNT_dpr_csum_rw_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_csum_rw_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_csum_rw_0_t(string name = "cap_dprstats_csr_CNT_dpr_csum_rw_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_csum_rw_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_csum_rw_0_t
    
class cap_dprstats_csr_CNT_dpr_crc_rw_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_crc_rw_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_crc_rw_t(string name = "cap_dprstats_csr_CNT_dpr_crc_rw_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_crc_rw_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_crc_rw_t
    
class cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t(string name = "cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t
    
class cap_dprstats_csr_CNT_dpr_padded_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_padded_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_padded_t(string name = "cap_dprstats_csr_CNT_dpr_padded_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_padded_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_padded_t
    
class cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t(string name = "cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t
    
class cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t(string name = "cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t
    
class cap_dprstats_csr_CNT_dpr_phv_no_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_phv_no_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_phv_no_data_t(string name = "cap_dprstats_csr_CNT_dpr_phv_no_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_phv_no_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_phv_no_data_t
    
class cap_dprstats_csr_CNT_dpr_phv_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_phv_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_phv_drop_t(string name = "cap_dprstats_csr_CNT_dpr_phv_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_phv_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_phv_drop_t
    
class cap_dprstats_csr_CNT_dpr_pktout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_pktout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_pktout_t(string name = "cap_dprstats_csr_CNT_dpr_pktout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_pktout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_pktout_t
    
class cap_dprstats_csr_CNT_dpr_pktin_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_pktin_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_pktin_t(string name = "cap_dprstats_csr_CNT_dpr_pktin_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_pktin_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_pktin_t
    
class cap_dprstats_csr_CNT_dpr_ohi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_ohi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_ohi_t(string name = "cap_dprstats_csr_CNT_dpr_ohi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_ohi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_ohi_t
    
class cap_dprstats_csr_CNT_dpr_phv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_dpr_phv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_dpr_phv_t(string name = "cap_dprstats_csr_CNT_dpr_phv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_dpr_phv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_CNT_dpr_phv_t
    
class cap_dprstats_csr_CNT_ecc_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_ecc_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_ecc_err_t(string name = "cap_dprstats_csr_CNT_ecc_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_ecc_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_drop_cpp_int_t;
        cpp_int int_var__pkt_drop;
        void pkt_drop (const cpp_int  & l__val);
        cpp_int pkt_drop() const;
    
        typedef pu_cpp_int< 40 > byte_drop_cpp_int_t;
        cpp_int int_var__byte_drop;
        void byte_drop (const cpp_int  & l__val);
        cpp_int byte_drop() const;
    
}; // cap_dprstats_csr_CNT_ecc_err_t
    
class cap_dprstats_csr_SAT_dpr_spare_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_SAT_dpr_spare_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_SAT_dpr_spare_err_t(string name = "cap_dprstats_csr_SAT_dpr_spare_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_SAT_dpr_spare_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_SAT_dpr_spare_err_t
    
class cap_dprstats_csr_SAT_dpr_credit_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_SAT_dpr_credit_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_SAT_dpr_credit_err_t(string name = "cap_dprstats_csr_SAT_dpr_credit_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_SAT_dpr_credit_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > egress_credit_ovflow_cpp_int_t;
        cpp_int int_var__egress_credit_ovflow;
        void egress_credit_ovflow (const cpp_int  & l__val);
        cpp_int egress_credit_ovflow() const;
    
        typedef pu_cpp_int< 8 > egress_credit_undflow_cpp_int_t;
        cpp_int int_var__egress_credit_undflow;
        void egress_credit_undflow (const cpp_int  & l__val);
        cpp_int egress_credit_undflow() const;
    
        typedef pu_cpp_int< 8 > pktout_credit_ovflow_cpp_int_t;
        cpp_int int_var__pktout_credit_ovflow;
        void pktout_credit_ovflow (const cpp_int  & l__val);
        cpp_int pktout_credit_ovflow() const;
    
        typedef pu_cpp_int< 8 > pktout_credit_undflow_cpp_int_t;
        cpp_int int_var__pktout_credit_undflow;
        void pktout_credit_undflow (const cpp_int  & l__val);
        cpp_int pktout_credit_undflow() const;
    
}; // cap_dprstats_csr_SAT_dpr_credit_err_t
    
class cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t(string name = "cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > data_mux_force_bypass_crc_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc;
        void data_mux_force_bypass_crc (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc() const;
    
        typedef pu_cpp_int< 8 > dpr_crc_info_cpp_int_t;
        cpp_int int_var__dpr_crc_info;
        void dpr_crc_info (const cpp_int  & l__val);
        cpp_int dpr_crc_info() const;
    
        typedef pu_cpp_int< 8 > dpr_crc_update_info_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info;
        void dpr_crc_update_info (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info() const;
    
        typedef pu_cpp_int< 8 > dpr_csum_info_cpp_int_t;
        cpp_int int_var__dpr_csum_info;
        void dpr_csum_info (const cpp_int  & l__val);
        cpp_int dpr_csum_info() const;
    
        typedef pu_cpp_int< 8 > data_mux_force_bypass_csum_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum;
        void data_mux_force_bypass_csum (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum() const;
    
        typedef pu_cpp_int< 8 > dpr_csum_update_info_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info;
        void dpr_csum_update_info (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info() const;
    
        typedef pu_cpp_int< 8 > ptr_early_pkt_eop_info_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info;
        void ptr_early_pkt_eop_info (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info() const;
    
        typedef pu_cpp_int< 8 > data_mux_eop_err_cpp_int_t;
        cpp_int int_var__data_mux_eop_err;
        void data_mux_eop_err (const cpp_int  & l__val);
        cpp_int data_mux_eop_err() const;
    
        typedef pu_cpp_int< 8 > pktin_eop_err_cpp_int_t;
        cpp_int int_var__pktin_eop_err;
        void pktin_eop_err (const cpp_int  & l__val);
        cpp_int pktin_eop_err() const;
    
        typedef pu_cpp_int< 8 > csum_err_cpp_int_t;
        cpp_int int_var__csum_err;
        void csum_err (const cpp_int  & l__val);
        cpp_int csum_err() const;
    
        typedef pu_cpp_int< 8 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        typedef pu_cpp_int< 8 > data_mux_drop_cpp_int_t;
        cpp_int int_var__data_mux_drop;
        void data_mux_drop (const cpp_int  & l__val);
        cpp_int data_mux_drop() const;
    
        typedef pu_cpp_int< 8 > phv_pkt_data_cpp_int_t;
        cpp_int int_var__phv_pkt_data;
        void phv_pkt_data (const cpp_int  & l__val);
        cpp_int phv_pkt_data() const;
    
        typedef pu_cpp_int< 8 > pktout_len_cell_cpp_int_t;
        cpp_int int_var__pktout_len_cell;
        void pktout_len_cell (const cpp_int  & l__val);
        cpp_int pktout_len_cell() const;
    
        typedef pu_cpp_int< 8 > padding_size_cpp_int_t;
        cpp_int int_var__padding_size;
        void padding_size (const cpp_int  & l__val);
        cpp_int padding_size() const;
    
        typedef pu_cpp_int< 8 > pktin_size_cpp_int_t;
        cpp_int int_var__pktin_size;
        void pktin_size (const cpp_int  & l__val);
        cpp_int pktin_size() const;
    
        typedef pu_cpp_int< 8 > pktin_err_cpp_int_t;
        cpp_int int_var__pktin_err;
        void pktin_err (const cpp_int  & l__val);
        cpp_int pktin_err() const;
    
        typedef pu_cpp_int< 8 > phv_no_data_cpp_int_t;
        cpp_int int_var__phv_no_data;
        void phv_no_data (const cpp_int  & l__val);
        cpp_int phv_no_data() const;
    
        typedef pu_cpp_int< 8 > ptr_lookahaed_cpp_int_t;
        cpp_int int_var__ptr_lookahaed;
        void ptr_lookahaed (const cpp_int  & l__val);
        cpp_int ptr_lookahaed() const;
    
        typedef pu_cpp_int< 8 > eop_vld_cpp_int_t;
        cpp_int int_var__eop_vld;
        void eop_vld (const cpp_int  & l__val);
        cpp_int eop_vld() const;
    
        typedef pu_cpp_int< 8 > csum_cal_vld_cpp_int_t;
        cpp_int int_var__csum_cal_vld;
        void csum_cal_vld (const cpp_int  & l__val);
        cpp_int csum_cal_vld() const;
    
}; // cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t
    
class cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t(string name = "cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > data_mux_force_bypass_crc_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc;
        void data_mux_force_bypass_crc (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc() const;
    
        typedef pu_cpp_int< 8 > dpr_crc_info_cpp_int_t;
        cpp_int int_var__dpr_crc_info;
        void dpr_crc_info (const cpp_int  & l__val);
        cpp_int dpr_crc_info() const;
    
        typedef pu_cpp_int< 8 > dpr_crc_update_info_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info;
        void dpr_crc_update_info (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info() const;
    
        typedef pu_cpp_int< 8 > dpr_csum_info_cpp_int_t;
        cpp_int int_var__dpr_csum_info;
        void dpr_csum_info (const cpp_int  & l__val);
        cpp_int dpr_csum_info() const;
    
        typedef pu_cpp_int< 8 > data_mux_force_bypass_csum_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum;
        void data_mux_force_bypass_csum (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum() const;
    
        typedef pu_cpp_int< 8 > dpr_csum_update_info_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info;
        void dpr_csum_update_info (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info() const;
    
        typedef pu_cpp_int< 8 > ptr_early_pkt_eop_info_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info;
        void ptr_early_pkt_eop_info (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info() const;
    
        typedef pu_cpp_int< 8 > data_mux_eop_err_cpp_int_t;
        cpp_int int_var__data_mux_eop_err;
        void data_mux_eop_err (const cpp_int  & l__val);
        cpp_int data_mux_eop_err() const;
    
        typedef pu_cpp_int< 8 > pktin_eop_err_cpp_int_t;
        cpp_int int_var__pktin_eop_err;
        void pktin_eop_err (const cpp_int  & l__val);
        cpp_int pktin_eop_err() const;
    
        typedef pu_cpp_int< 8 > csum_err_cpp_int_t;
        cpp_int int_var__csum_err;
        void csum_err (const cpp_int  & l__val);
        cpp_int csum_err() const;
    
        typedef pu_cpp_int< 8 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        typedef pu_cpp_int< 8 > data_mux_drop_cpp_int_t;
        cpp_int int_var__data_mux_drop;
        void data_mux_drop (const cpp_int  & l__val);
        cpp_int data_mux_drop() const;
    
        typedef pu_cpp_int< 8 > phv_pkt_data_cpp_int_t;
        cpp_int int_var__phv_pkt_data;
        void phv_pkt_data (const cpp_int  & l__val);
        cpp_int phv_pkt_data() const;
    
        typedef pu_cpp_int< 8 > pktout_len_cell_cpp_int_t;
        cpp_int int_var__pktout_len_cell;
        void pktout_len_cell (const cpp_int  & l__val);
        cpp_int pktout_len_cell() const;
    
        typedef pu_cpp_int< 8 > padding_size_cpp_int_t;
        cpp_int int_var__padding_size;
        void padding_size (const cpp_int  & l__val);
        cpp_int padding_size() const;
    
        typedef pu_cpp_int< 8 > pktin_size_cpp_int_t;
        cpp_int int_var__pktin_size;
        void pktin_size (const cpp_int  & l__val);
        cpp_int pktin_size() const;
    
        typedef pu_cpp_int< 8 > pktin_err_cpp_int_t;
        cpp_int int_var__pktin_err;
        void pktin_err (const cpp_int  & l__val);
        cpp_int pktin_err() const;
    
        typedef pu_cpp_int< 8 > phv_no_data_cpp_int_t;
        cpp_int int_var__phv_no_data;
        void phv_no_data (const cpp_int  & l__val);
        cpp_int phv_no_data() const;
    
        typedef pu_cpp_int< 8 > ptr_lookahaed_cpp_int_t;
        cpp_int int_var__ptr_lookahaed;
        void ptr_lookahaed (const cpp_int  & l__val);
        cpp_int ptr_lookahaed() const;
    
        typedef pu_cpp_int< 8 > eop_vld_cpp_int_t;
        cpp_int int_var__eop_vld;
        void eop_vld (const cpp_int  & l__val);
        cpp_int eop_vld() const;
    
        typedef pu_cpp_int< 8 > csum_cal_vld_cpp_int_t;
        cpp_int int_var__csum_cal_vld;
        void csum_cal_vld (const cpp_int  & l__val);
        cpp_int csum_cal_vld() const;
    
}; // cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t
    
class cap_dprstats_csr_SAT_dpr_ff_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_SAT_dpr_ff_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_SAT_dpr_ff_err_t(string name = "cap_dprstats_csr_SAT_dpr_ff_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_SAT_dpr_ff_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 8 > pktin_ff_ovflow_cpp_int_t;
        cpp_int int_var__pktin_ff_ovflow;
        void pktin_ff_ovflow (const cpp_int  & l__val);
        cpp_int pktin_ff_ovflow() const;
    
        typedef pu_cpp_int< 8 > pktout_ff_undflow_cpp_int_t;
        cpp_int int_var__pktout_ff_undflow;
        void pktout_ff_undflow (const cpp_int  & l__val);
        cpp_int pktout_ff_undflow() const;
    
        typedef pu_cpp_int< 8 > csum_ff_ovflow_cpp_int_t;
        cpp_int int_var__csum_ff_ovflow;
        void csum_ff_ovflow (const cpp_int  & l__val);
        cpp_int csum_ff_ovflow() const;
    
        typedef pu_cpp_int< 8 > ptr_ff_ovflow_cpp_int_t;
        cpp_int int_var__ptr_ff_ovflow;
        void ptr_ff_ovflow (const cpp_int  & l__val);
        cpp_int ptr_ff_ovflow() const;
    
        typedef pu_cpp_int< 8 > egress_credit_ovflow_cpp_int_t;
        cpp_int int_var__egress_credit_ovflow;
        void egress_credit_ovflow (const cpp_int  & l__val);
        cpp_int egress_credit_ovflow() const;
    
        typedef pu_cpp_int< 8 > egress_credit_undflow_cpp_int_t;
        cpp_int int_var__egress_credit_undflow;
        void egress_credit_undflow (const cpp_int  & l__val);
        cpp_int egress_credit_undflow() const;
    
        typedef pu_cpp_int< 8 > pktout_credit_ovflow_cpp_int_t;
        cpp_int int_var__pktout_credit_ovflow;
        void pktout_credit_ovflow (const cpp_int  & l__val);
        cpp_int pktout_credit_ovflow() const;
    
        typedef pu_cpp_int< 8 > pktout_credit_undflow_cpp_int_t;
        cpp_int int_var__pktout_credit_undflow;
        void pktout_credit_undflow (const cpp_int  & l__val);
        cpp_int pktout_credit_undflow() const;
    
}; // cap_dprstats_csr_SAT_dpr_ff_err_t
    
class cap_dprstats_csr_SAT_dpr_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_SAT_dpr_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_SAT_dpr_err_t(string name = "cap_dprstats_csr_SAT_dpr_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_SAT_dpr_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 8 > err_pktin_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_pktin_sop_no_eop;
        void err_pktin_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop() const;
    
        typedef pu_cpp_int< 8 > err_pktin_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_pktin_eop_no_sop;
        void err_pktin_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop() const;
    
        typedef pu_cpp_int< 8 > err_csum_offset_gt_pkt_size_0_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_0;
        void err_csum_offset_gt_pkt_size_0 (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_0() const;
    
        typedef pu_cpp_int< 8 > err_csum_offset_gt_pkt_size_1_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_1;
        void err_csum_offset_gt_pkt_size_1 (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_1() const;
    
        typedef pu_cpp_int< 8 > err_csum_offset_gt_pkt_size_2_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_2;
        void err_csum_offset_gt_pkt_size_2 (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_2() const;
    
        typedef pu_cpp_int< 8 > err_csum_offset_gt_pkt_size_3_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_3;
        void err_csum_offset_gt_pkt_size_3 (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_3() const;
    
        typedef pu_cpp_int< 8 > err_csum_offset_gt_pkt_size_4_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_4;
        void err_csum_offset_gt_pkt_size_4 (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_4() const;
    
        typedef pu_cpp_int< 8 > err_csum_phdr_offset_gt_pkt_size_0_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_0;
        void err_csum_phdr_offset_gt_pkt_size_0 (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_0() const;
    
        typedef pu_cpp_int< 8 > err_csum_phdr_offset_gt_pkt_size_1_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_1;
        void err_csum_phdr_offset_gt_pkt_size_1 (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_1() const;
    
        typedef pu_cpp_int< 8 > err_csum_phdr_offset_gt_pkt_size_2_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_2;
        void err_csum_phdr_offset_gt_pkt_size_2 (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_2() const;
    
        typedef pu_cpp_int< 8 > err_csum_phdr_offset_gt_pkt_size_3_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_3;
        void err_csum_phdr_offset_gt_pkt_size_3 (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_3() const;
    
        typedef pu_cpp_int< 8 > err_csum_phdr_offset_gt_pkt_size_4_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_4;
        void err_csum_phdr_offset_gt_pkt_size_4 (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_4() const;
    
        typedef pu_cpp_int< 8 > err_csum_loc_gt_pkt_size_0_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_0;
        void err_csum_loc_gt_pkt_size_0 (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_0() const;
    
        typedef pu_cpp_int< 8 > err_csum_loc_gt_pkt_size_1_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_1;
        void err_csum_loc_gt_pkt_size_1 (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_1() const;
    
        typedef pu_cpp_int< 8 > err_csum_loc_gt_pkt_size_2_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_2;
        void err_csum_loc_gt_pkt_size_2 (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_2() const;
    
        typedef pu_cpp_int< 8 > err_csum_loc_gt_pkt_size_3_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_3;
        void err_csum_loc_gt_pkt_size_3 (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_3() const;
    
        typedef pu_cpp_int< 8 > err_csum_loc_gt_pkt_size_4_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_4;
        void err_csum_loc_gt_pkt_size_4 (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_4() const;
    
        typedef pu_cpp_int< 8 > err_csum_start_gt_end_0_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_0;
        void err_csum_start_gt_end_0 (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_0() const;
    
        typedef pu_cpp_int< 8 > err_csum_start_gt_end_1_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_1;
        void err_csum_start_gt_end_1 (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_1() const;
    
        typedef pu_cpp_int< 8 > err_csum_start_gt_end_2_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_2;
        void err_csum_start_gt_end_2 (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_2() const;
    
        typedef pu_cpp_int< 8 > err_csum_start_gt_end_3_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_3;
        void err_csum_start_gt_end_3 (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_3() const;
    
        typedef pu_cpp_int< 8 > err_csum_start_gt_end_4_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_4;
        void err_csum_start_gt_end_4 (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_4() const;
    
        typedef pu_cpp_int< 8 > err_crc_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_offset_gt_pkt_size;
        void err_crc_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 8 > err_crc_loc_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_loc_gt_pkt_size;
        void err_crc_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 8 > err_crc_mask_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_mask_offset_gt_pkt_size;
        void err_crc_mask_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_mask_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 8 > err_crc_start_gt_end_cpp_int_t;
        cpp_int int_var__err_crc_start_gt_end;
        void err_crc_start_gt_end (const cpp_int  & l__val);
        cpp_int err_crc_start_gt_end() const;
    
        typedef pu_cpp_int< 8 > err_pkt_eop_early_cpp_int_t;
        cpp_int int_var__err_pkt_eop_early;
        void err_pkt_eop_early (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early() const;
    
        typedef pu_cpp_int< 8 > err_ptr_ff_overflow_cpp_int_t;
        cpp_int int_var__err_ptr_ff_overflow;
        void err_ptr_ff_overflow (const cpp_int  & l__val);
        cpp_int err_ptr_ff_overflow() const;
    
        typedef pu_cpp_int< 8 > err_csum_ff_overflow_cpp_int_t;
        cpp_int int_var__err_csum_ff_overflow;
        void err_csum_ff_overflow (const cpp_int  & l__val);
        cpp_int err_csum_ff_overflow() const;
    
        typedef pu_cpp_int< 8 > err_pktout_ff_overflow_cpp_int_t;
        cpp_int int_var__err_pktout_ff_overflow;
        void err_pktout_ff_overflow (const cpp_int  & l__val);
        cpp_int err_pktout_ff_overflow() const;
    
        typedef pu_cpp_int< 8 > err_ptr_from_cfg_overflow_cpp_int_t;
        cpp_int int_var__err_ptr_from_cfg_overflow;
        void err_ptr_from_cfg_overflow (const cpp_int  & l__val);
        cpp_int err_ptr_from_cfg_overflow() const;
    
}; // cap_dprstats_csr_SAT_dpr_err_t
    
class cap_dprstats_csr_CNT_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_drop_t(string name = "cap_dprstats_csr_CNT_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 38 > pkt_drop_cpp_int_t;
        cpp_int int_var__pkt_drop;
        void pkt_drop (const cpp_int  & l__val);
        cpp_int pkt_drop() const;
    
        typedef pu_cpp_int< 44 > byte_drop_cpp_int_t;
        cpp_int int_var__byte_drop;
        void byte_drop (const cpp_int  & l__val);
        cpp_int byte_drop() const;
    
}; // cap_dprstats_csr_CNT_drop_t
    
class cap_dprstats_csr_CNT_interface_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_CNT_interface_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_CNT_interface_t(string name = "cap_dprstats_csr_CNT_interface_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_CNT_interface_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > dpp_phv_sop_cpp_int_t;
        cpp_int int_var__dpp_phv_sop;
        void dpp_phv_sop (const cpp_int  & l__val);
        cpp_int dpp_phv_sop() const;
    
        typedef pu_cpp_int< 32 > dpp_phv_eop_cpp_int_t;
        cpp_int int_var__dpp_phv_eop;
        void dpp_phv_eop (const cpp_int  & l__val);
        cpp_int dpp_phv_eop() const;
    
        typedef pu_cpp_int< 32 > dpp_ohi_sop_cpp_int_t;
        cpp_int int_var__dpp_ohi_sop;
        void dpp_ohi_sop (const cpp_int  & l__val);
        cpp_int dpp_ohi_sop() const;
    
        typedef pu_cpp_int< 32 > dpp_ohi_eop_cpp_int_t;
        cpp_int int_var__dpp_ohi_eop;
        void dpp_ohi_eop (const cpp_int  & l__val);
        cpp_int dpp_ohi_eop() const;
    
        typedef pu_cpp_int< 32 > dpp_csum_crc_vld_cpp_int_t;
        cpp_int int_var__dpp_csum_crc_vld;
        void dpp_csum_crc_vld (const cpp_int  & l__val);
        cpp_int dpp_csum_crc_vld() const;
    
        typedef pu_cpp_int< 32 > dpp_frame_vld_cpp_int_t;
        cpp_int int_var__dpp_frame_vld;
        void dpp_frame_vld (const cpp_int  & l__val);
        cpp_int dpp_frame_vld() const;
    
        typedef pu_cpp_int< 32 > pa_pkt_sop_cpp_int_t;
        cpp_int int_var__pa_pkt_sop;
        void pa_pkt_sop (const cpp_int  & l__val);
        cpp_int pa_pkt_sop() const;
    
        typedef pu_cpp_int< 32 > pa_pkt_eop_cpp_int_t;
        cpp_int int_var__pa_pkt_eop;
        void pa_pkt_eop (const cpp_int  & l__val);
        cpp_int pa_pkt_eop() const;
    
        typedef pu_cpp_int< 32 > pb_pkt_sop_cpp_int_t;
        cpp_int int_var__pb_pkt_sop;
        void pb_pkt_sop (const cpp_int  & l__val);
        cpp_int pb_pkt_sop() const;
    
        typedef pu_cpp_int< 32 > pb_pkt_eop_cpp_int_t;
        cpp_int int_var__pb_pkt_eop;
        void pb_pkt_eop (const cpp_int  & l__val);
        cpp_int pb_pkt_eop() const;
    
        typedef pu_cpp_int< 32 > pkt_drop_cpp_int_t;
        cpp_int int_var__pkt_drop;
        void pkt_drop (const cpp_int  & l__val);
        cpp_int pkt_drop() const;
    
}; // cap_dprstats_csr_CNT_interface_t
    
class cap_dprstats_csr_sta_flop_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sta_flop_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sta_flop_fifo_t(string name = "cap_dprstats_csr_sta_flop_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sta_flop_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_srdy_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc_srdy;
        void data_mux_force_bypass_crc_srdy (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc_srdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_drdy_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc_drdy;
        void data_mux_force_bypass_crc_drdy (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc_drdy() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_srdy_cpp_int_t;
        cpp_int int_var__dpr_crc_info_srdy;
        void dpr_crc_info_srdy (const cpp_int  & l__val);
        cpp_int dpr_crc_info_srdy() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_drdy_cpp_int_t;
        cpp_int int_var__dpr_crc_info_drdy;
        void dpr_crc_info_drdy (const cpp_int  & l__val);
        cpp_int dpr_crc_info_drdy() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_srdy_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info_srdy;
        void dpr_crc_update_info_srdy (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info_srdy() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_drdy_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info_drdy;
        void dpr_crc_update_info_drdy (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info_drdy() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_srdy_cpp_int_t;
        cpp_int int_var__dpr_csum_info_srdy;
        void dpr_csum_info_srdy (const cpp_int  & l__val);
        cpp_int dpr_csum_info_srdy() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_drdy_cpp_int_t;
        cpp_int int_var__dpr_csum_info_drdy;
        void dpr_csum_info_drdy (const cpp_int  & l__val);
        cpp_int dpr_csum_info_drdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_srdy_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum_srdy;
        void data_mux_force_bypass_csum_srdy (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum_srdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_drdy_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum_drdy;
        void data_mux_force_bypass_csum_drdy (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum_drdy() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_srdy_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info_srdy;
        void dpr_csum_update_info_srdy (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info_srdy() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_drdy_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info_drdy;
        void dpr_csum_update_info_drdy (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info_drdy() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_srdy_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info_srdy;
        void ptr_early_pkt_eop_info_srdy (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info_srdy() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_drdy_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info_drdy;
        void ptr_early_pkt_eop_info_drdy (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info_drdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_srdy_cpp_int_t;
        cpp_int int_var__data_mux_eop_err_srdy;
        void data_mux_eop_err_srdy (const cpp_int  & l__val);
        cpp_int data_mux_eop_err_srdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_drdy_cpp_int_t;
        cpp_int int_var__data_mux_eop_err_drdy;
        void data_mux_eop_err_drdy (const cpp_int  & l__val);
        cpp_int data_mux_eop_err_drdy() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_srdy_cpp_int_t;
        cpp_int int_var__pktin_eop_err_srdy;
        void pktin_eop_err_srdy (const cpp_int  & l__val);
        cpp_int pktin_eop_err_srdy() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_drdy_cpp_int_t;
        cpp_int int_var__pktin_eop_err_drdy;
        void pktin_eop_err_drdy (const cpp_int  & l__val);
        cpp_int pktin_eop_err_drdy() const;
    
        typedef pu_cpp_int< 1 > csum_err_srdy_cpp_int_t;
        cpp_int int_var__csum_err_srdy;
        void csum_err_srdy (const cpp_int  & l__val);
        cpp_int csum_err_srdy() const;
    
        typedef pu_cpp_int< 1 > csum_err_drdy_cpp_int_t;
        cpp_int int_var__csum_err_drdy;
        void csum_err_drdy (const cpp_int  & l__val);
        cpp_int csum_err_drdy() const;
    
        typedef pu_cpp_int< 1 > crc_err_srdy_cpp_int_t;
        cpp_int int_var__crc_err_srdy;
        void crc_err_srdy (const cpp_int  & l__val);
        cpp_int crc_err_srdy() const;
    
        typedef pu_cpp_int< 1 > crc_err_drdy_cpp_int_t;
        cpp_int int_var__crc_err_drdy;
        void crc_err_drdy (const cpp_int  & l__val);
        cpp_int crc_err_drdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_srdy_cpp_int_t;
        cpp_int int_var__data_mux_drop_srdy;
        void data_mux_drop_srdy (const cpp_int  & l__val);
        cpp_int data_mux_drop_srdy() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_drdy_cpp_int_t;
        cpp_int int_var__data_mux_drop_drdy;
        void data_mux_drop_drdy (const cpp_int  & l__val);
        cpp_int data_mux_drop_drdy() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_srdy_cpp_int_t;
        cpp_int int_var__phv_pkt_data_srdy;
        void phv_pkt_data_srdy (const cpp_int  & l__val);
        cpp_int phv_pkt_data_srdy() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_drdy_cpp_int_t;
        cpp_int int_var__phv_pkt_data_drdy;
        void phv_pkt_data_drdy (const cpp_int  & l__val);
        cpp_int phv_pkt_data_drdy() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_srdy_cpp_int_t;
        cpp_int int_var__pktout_len_cell_srdy;
        void pktout_len_cell_srdy (const cpp_int  & l__val);
        cpp_int pktout_len_cell_srdy() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_drdy_cpp_int_t;
        cpp_int int_var__pktout_len_cell_drdy;
        void pktout_len_cell_drdy (const cpp_int  & l__val);
        cpp_int pktout_len_cell_drdy() const;
    
        typedef pu_cpp_int< 1 > padding_size_srdy_cpp_int_t;
        cpp_int int_var__padding_size_srdy;
        void padding_size_srdy (const cpp_int  & l__val);
        cpp_int padding_size_srdy() const;
    
        typedef pu_cpp_int< 1 > padding_size_drdy_cpp_int_t;
        cpp_int int_var__padding_size_drdy;
        void padding_size_drdy (const cpp_int  & l__val);
        cpp_int padding_size_drdy() const;
    
        typedef pu_cpp_int< 1 > pktin_size_srdy_cpp_int_t;
        cpp_int int_var__pktin_size_srdy;
        void pktin_size_srdy (const cpp_int  & l__val);
        cpp_int pktin_size_srdy() const;
    
        typedef pu_cpp_int< 1 > pktin_size_drdy_cpp_int_t;
        cpp_int int_var__pktin_size_drdy;
        void pktin_size_drdy (const cpp_int  & l__val);
        cpp_int pktin_size_drdy() const;
    
        typedef pu_cpp_int< 1 > pktin_err_srdy_cpp_int_t;
        cpp_int int_var__pktin_err_srdy;
        void pktin_err_srdy (const cpp_int  & l__val);
        cpp_int pktin_err_srdy() const;
    
        typedef pu_cpp_int< 1 > pktin_err_drdy_cpp_int_t;
        cpp_int int_var__pktin_err_drdy;
        void pktin_err_drdy (const cpp_int  & l__val);
        cpp_int pktin_err_drdy() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_srdy_cpp_int_t;
        cpp_int int_var__phv_no_data_srdy;
        void phv_no_data_srdy (const cpp_int  & l__val);
        cpp_int phv_no_data_srdy() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_drdy_cpp_int_t;
        cpp_int int_var__phv_no_data_drdy;
        void phv_no_data_drdy (const cpp_int  & l__val);
        cpp_int phv_no_data_drdy() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_srdy_cpp_int_t;
        cpp_int int_var__ptr_lookahaed_srdy;
        void ptr_lookahaed_srdy (const cpp_int  & l__val);
        cpp_int ptr_lookahaed_srdy() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_drdy_cpp_int_t;
        cpp_int int_var__ptr_lookahaed_drdy;
        void ptr_lookahaed_drdy (const cpp_int  & l__val);
        cpp_int ptr_lookahaed_drdy() const;
    
        typedef pu_cpp_int< 1 > eop_vld_srdy_cpp_int_t;
        cpp_int int_var__eop_vld_srdy;
        void eop_vld_srdy (const cpp_int  & l__val);
        cpp_int eop_vld_srdy() const;
    
        typedef pu_cpp_int< 1 > eop_vld_drdy_cpp_int_t;
        cpp_int int_var__eop_vld_drdy;
        void eop_vld_drdy (const cpp_int  & l__val);
        cpp_int eop_vld_drdy() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_srdy_cpp_int_t;
        cpp_int int_var__csum_cal_vld_srdy;
        void csum_cal_vld_srdy (const cpp_int  & l__val);
        cpp_int csum_cal_vld_srdy() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_drdy_cpp_int_t;
        cpp_int int_var__csum_cal_vld_drdy;
        void csum_cal_vld_drdy (const cpp_int  & l__val);
        cpp_int csum_cal_vld_drdy() const;
    
}; // cap_dprstats_csr_sta_flop_fifo_t
    
class cap_dprstats_csr_sta_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sta_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sta_fifo_t(string name = "cap_dprstats_csr_sta_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sta_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > pktin_ff_srdy_cpp_int_t;
        cpp_int int_var__pktin_ff_srdy;
        void pktin_ff_srdy (const cpp_int  & l__val);
        cpp_int pktin_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > pktin_ff_drdy_cpp_int_t;
        cpp_int int_var__pktin_ff_drdy;
        void pktin_ff_drdy (const cpp_int  & l__val);
        cpp_int pktin_ff_drdy() const;
    
        typedef pu_cpp_int< 1 > ptr_ff_srdy_cpp_int_t;
        cpp_int int_var__ptr_ff_srdy;
        void ptr_ff_srdy (const cpp_int  & l__val);
        cpp_int ptr_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > ptr_ff_drdy_cpp_int_t;
        cpp_int int_var__ptr_ff_drdy;
        void ptr_ff_drdy (const cpp_int  & l__val);
        cpp_int ptr_ff_drdy() const;
    
        typedef pu_cpp_int< 1 > csum_ff_srdy_cpp_int_t;
        cpp_int int_var__csum_ff_srdy;
        void csum_ff_srdy (const cpp_int  & l__val);
        cpp_int csum_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > csum_ff_drdy_cpp_int_t;
        cpp_int int_var__csum_ff_drdy;
        void csum_ff_drdy (const cpp_int  & l__val);
        cpp_int csum_ff_drdy() const;
    
        typedef pu_cpp_int< 1 > pktout_ff_srdy_cpp_int_t;
        cpp_int int_var__pktout_ff_srdy;
        void pktout_ff_srdy (const cpp_int  & l__val);
        cpp_int pktout_ff_srdy() const;
    
        typedef pu_cpp_int< 1 > pktout_ff_drdy_cpp_int_t;
        cpp_int int_var__pktout_ff_drdy;
        void pktout_ff_drdy (const cpp_int  & l__val);
        cpp_int pktout_ff_drdy() const;
    
}; // cap_dprstats_csr_sta_fifo_t
    
class cap_dprstats_csr_sta_credit_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sta_credit_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sta_credit_t(string name = "cap_dprstats_csr_sta_credit_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sta_credit_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > egress_cpp_int_t;
        cpp_int int_var__egress;
        void egress (const cpp_int  & l__val);
        cpp_int egress() const;
    
        typedef pu_cpp_int< 5 > pktout_cpp_int_t;
        cpp_int int_var__pktout;
        void pktout (const cpp_int  & l__val);
        cpp_int pktout() const;
    
}; // cap_dprstats_csr_sta_credit_t
    
class cap_dprstats_csr_sta_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sta_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sta_spare_t(string name = "cap_dprstats_csr_sta_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sta_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sta_spare_t
    
class cap_dprstats_csr_sym_pktin_info_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_pktin_info_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_pktin_info_capture_t(string name = "cap_dprstats_csr_sym_pktin_info_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_pktin_info_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pktin_ff_rptr_start_cpp_int_t;
        cpp_int int_var__pktin_ff_rptr_start;
        void pktin_ff_rptr_start (const cpp_int  & l__val);
        cpp_int pktin_ff_rptr_start() const;
    
        typedef pu_cpp_int< 10 > pktin_ff_rptr_end_cpp_int_t;
        cpp_int int_var__pktin_ff_rptr_end;
        void pktin_ff_rptr_end (const cpp_int  & l__val);
        cpp_int pktin_ff_rptr_end() const;
    
}; // cap_dprstats_csr_sym_pktin_info_capture_t
    
class cap_dprstats_csr_sym_ohi_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_ohi_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_ohi_capture_t(string name = "cap_dprstats_csr_sym_ohi_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_ohi_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_ohi_capture_t
    
class cap_dprstats_csr_sym_phv5_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_phv5_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_phv5_capture_t(string name = "cap_dprstats_csr_sym_phv5_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_phv5_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_phv5_capture_t
    
class cap_dprstats_csr_sym_phv4_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_phv4_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_phv4_capture_t(string name = "cap_dprstats_csr_sym_phv4_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_phv4_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_phv4_capture_t
    
class cap_dprstats_csr_sym_phv3_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_phv3_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_phv3_capture_t(string name = "cap_dprstats_csr_sym_phv3_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_phv3_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_phv3_capture_t
    
class cap_dprstats_csr_sym_phv2_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_phv2_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_phv2_capture_t(string name = "cap_dprstats_csr_sym_phv2_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_phv2_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_phv2_capture_t
    
class cap_dprstats_csr_sym_phv1_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_phv1_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_phv1_capture_t(string name = "cap_dprstats_csr_sym_phv1_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_phv1_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_phv1_capture_t
    
class cap_dprstats_csr_sym_phv0_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_sym_phv0_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_sym_phv0_capture_t(string name = "cap_dprstats_csr_sym_phv0_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_sym_phv0_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_sym_phv0_capture_t
    
class cap_dprstats_csr_cfg_spare_stats_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_cfg_spare_stats_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_cfg_spare_stats_t(string name = "cap_dprstats_csr_cfg_spare_stats_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_cfg_spare_stats_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_cfg_spare_stats_t
    
class cap_dprstats_csr_cfg_capture_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_cfg_capture_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_cfg_capture_t(string name = "cap_dprstats_csr_cfg_capture_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_cfg_capture_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dprstats_csr_cfg_capture_t
    
class cap_dprstats_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprstats_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprstats_csr_t(string name = "cap_dprstats_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprstats_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dprstats_csr_cfg_capture_t cfg_capture;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprstats_csr_cfg_spare_stats_t, 8> cfg_spare_stats;
        #else 
        cap_dprstats_csr_cfg_spare_stats_t cfg_spare_stats[8];
        #endif
        int get_depth_cfg_spare_stats() { return 8; }
    
        cap_dprstats_csr_sym_phv0_capture_t sym_phv0_capture;
    
        cap_dprstats_csr_sym_phv1_capture_t sym_phv1_capture;
    
        cap_dprstats_csr_sym_phv2_capture_t sym_phv2_capture;
    
        cap_dprstats_csr_sym_phv3_capture_t sym_phv3_capture;
    
        cap_dprstats_csr_sym_phv4_capture_t sym_phv4_capture;
    
        cap_dprstats_csr_sym_phv5_capture_t sym_phv5_capture;
    
        cap_dprstats_csr_sym_ohi_capture_t sym_ohi_capture;
    
        cap_dprstats_csr_sym_pktin_info_capture_t sym_pktin_info_capture;
    
        cap_dprstats_csr_sta_spare_t sta_spare;
    
        cap_dprstats_csr_sta_credit_t sta_credit;
    
        cap_dprstats_csr_sta_fifo_t sta_fifo;
    
        cap_dprstats_csr_sta_flop_fifo_t sta_flop_fifo;
    
        cap_dprstats_csr_CNT_interface_t CNT_interface;
    
        cap_dprstats_csr_CNT_drop_t CNT_drop;
    
        cap_dprstats_csr_SAT_dpr_err_t SAT_dpr_err;
    
        cap_dprstats_csr_SAT_dpr_ff_err_t SAT_dpr_ff_err;
    
        cap_dprstats_csr_SAT_dpr_flop_ff_ovfl_err_t SAT_dpr_flop_ff_ovfl_err;
    
        cap_dprstats_csr_SAT_dpr_flop_ff_undflow_err_t SAT_dpr_flop_ff_undflow_err;
    
        cap_dprstats_csr_SAT_dpr_credit_err_t SAT_dpr_credit_err;
    
        cap_dprstats_csr_SAT_dpr_spare_err_t SAT_dpr_spare_err;
    
        cap_dprstats_csr_CNT_ecc_err_t CNT_ecc_err;
    
        cap_dprstats_csr_CNT_dpr_phv_t CNT_dpr_phv;
    
        cap_dprstats_csr_CNT_dpr_ohi_t CNT_dpr_ohi;
    
        cap_dprstats_csr_CNT_dpr_pktin_t CNT_dpr_pktin;
    
        cap_dprstats_csr_CNT_dpr_pktout_t CNT_dpr_pktout;
    
        cap_dprstats_csr_CNT_dpr_phv_drop_t CNT_dpr_phv_drop;
    
        cap_dprstats_csr_CNT_dpr_phv_no_data_t CNT_dpr_phv_no_data;
    
        cap_dprstats_csr_CNT_dpr_phv_drop_no_data_t CNT_dpr_phv_drop_no_data;
    
        cap_dprstats_csr_CNT_dpr_phv_drop_no_data_drop_t CNT_dpr_phv_drop_no_data_drop;
    
        cap_dprstats_csr_CNT_dpr_padded_t CNT_dpr_padded;
    
        cap_dprstats_csr_CNT_dpr_pktout_phv_drop_t CNT_dpr_pktout_phv_drop;
    
        cap_dprstats_csr_CNT_dpr_crc_rw_t CNT_dpr_crc_rw;
    
        cap_dprstats_csr_CNT_dpr_csum_rw_0_t CNT_dpr_csum_rw_0;
    
        cap_dprstats_csr_CNT_dpr_csum_rw_1_t CNT_dpr_csum_rw_1;
    
        cap_dprstats_csr_CNT_dpr_csum_rw_2_t CNT_dpr_csum_rw_2;
    
        cap_dprstats_csr_CNT_dpr_csum_rw_3_t CNT_dpr_csum_rw_3;
    
        cap_dprstats_csr_CNT_dpr_csum_rw_4_t CNT_dpr_csum_rw_4;
    
        cap_dprstats_csr_CNT_dpr_spare_0_t CNT_dpr_spare_0;
    
        cap_dprstats_csr_CNT_dpr_spare_1_t CNT_dpr_spare_1;
    
        cap_dprstats_csr_CNT_dpr_spare_2_t CNT_dpr_spare_2;
    
        cap_dprstats_csr_CNT_dpr_spare_3_t CNT_dpr_spare_3;
    
        cap_dprstats_csr_CNT_dpr_spare_4_t CNT_dpr_spare_4;
    
        cap_dprstats_csr_CNT_dpr_spare_5_t CNT_dpr_spare_5;
    
        cap_dprstats_csr_CNT_dpr_spare_6_t CNT_dpr_spare_6;
    
        cap_dprstats_csr_CNT_dpr_spare_7_t CNT_dpr_spare_7;
    
        cap_dprstats_csr_CNT_dpr_spare_8_t CNT_dpr_spare_8;
    
        cap_dprstats_csr_CNT_dpr_spare_9_t CNT_dpr_spare_9;
    
        cap_dprstats_csr_CNT_dpr_spare_10_t CNT_dpr_spare_10;
    
        cap_dprstats_csr_CNT_dpr_spare_11_t CNT_dpr_spare_11;
    
        cap_dprstats_csr_CNT_dpr_spare_12_t CNT_dpr_spare_12;
    
        cap_dprstats_csr_CNT_dpr_spare_13_t CNT_dpr_spare_13;
    
        cap_dprstats_csr_CNT_dpr_spare_14_t CNT_dpr_spare_14;
    
        cap_dprstats_csr_CNT_dpr_spare_15_t CNT_dpr_spare_15;
    
        cap_dprstats_csr_CNT_dpr_spare_byte_0_t CNT_dpr_spare_byte_0;
    
        cap_dprstats_csr_CNT_dpr_spare_byte_1_t CNT_dpr_spare_byte_1;
    
        cap_dprstats_csr_CNT_dpr_spare_byte_2_t CNT_dpr_spare_byte_2;
    
        cap_dprstats_csr_CNT_dpr_spare_byte_3_t CNT_dpr_spare_byte_3;
    
}; // cap_dprstats_csr_t
    
#endif // CAP_DPRSTATS_CSR_H
        