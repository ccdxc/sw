#ifndef PEN_CSR_BASE_H
#define PEN_CSR_BASE_H

#ifndef PEN_CSR_LARGE_ARRAY_THRESHOLD
#define PEN_CSR_LARGE_ARRAY_THRESHOLD (64*1024)
#endif 

#if _BullseyeCoverage
    #define BullseyeCoverageSaveOff _Pragma("BullseyeCoverage save off")
    #define BullseyeCoverageRestore _Pragma("BullseyeCoverage restore")
#else
    #define BullseyeCoverageSaveOff
    #define BullseyeCoverageRestore
#endif

#include <stdint.h>
#include <iostream>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "cpp_int_helper.h"
#include <assert.h>
#include "LogMsg.h"
#include "cpu.h"

using namespace std;
#ifndef _SWIG_WRP_
using namespace boost::multiprecision;
using boost::multiprecision::cpp_int;

template<unsigned BITS>
using pu_cpp_int = number < cpp_int_backend<BITS, BITS, unsigned_magnitude, unchecked, void> >;
#endif

#define PU_ASSERT(ASSERT_COND)  assert(ASSERT_COND)

#ifdef _CSV_INCLUDED_
void SLEEP(int N);
#else
    #define SLEEP(N) usleep(N);
#endif

// forward declaration
class pen_csr_base;

class pen_csr_callback {
    public:

        pen_csr_callback() {}
        virtual ~pen_csr_callback() {}
        virtual void pre_csr_mem_zerotime_hdl_write(pen_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 
        virtual void post_csr_mem_zerotime_hdl_write(pen_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 
        virtual void pre_csr_mem_zerotime_hdl_read(pen_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 
        virtual void post_csr_mem_zerotime_hdl_read(pen_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 

        
};


class pen_csr_base {

    public:
        enum csr_type_t {
            CSR_TYPE_NONE = 0,
            CSR_TYPE_REGISTER,
            CSR_TYPE_MEM_ENTRY,
            CSR_TYPE_MEMORY,
            CSR_TYPE_DECODER,
            CSR_TYPE_BLOCK
        };

    protected:
        pen_csr_base * base__parent;
#ifndef CSR_NO_BASE_NAME        
        string base__name;
#endif        
//#ifndef CSR_NO_TYPE_NAME
//        string type__name;
//#endif
        uint64_t base__int__offset;
#ifndef CSR_NO_CSR_TYPE        
        csr_type_t base__int__csr_type;
#endif        
#ifndef CSR_NO_CHILDREN_REG        
        vector<pen_csr_base *> int__children;
#endif        
        uint64_t int__csr_end_addr;
#ifndef CSR_NO_CALLBACK        
        vector< std::shared_ptr<pen_csr_callback> > callback_vec;  
#endif        
#ifndef CSR_NO_RESET_VAL
        cpp_int int_var__reset_value; 
#endif        

    protected:
    public:


    pen_csr_base(string _name, pen_csr_base * _parent = nullptr);
    virtual ~pen_csr_base();
    virtual string get_name() const;
    //virtual string get_type_name() const;
    virtual pen_csr_base * get_parent() const;
    virtual void set_name(string _name);
    virtual void set_parent(pen_csr_base * _base);
    virtual void register_child(pen_csr_base * _child, bool add_to_parent);
    virtual void update_block_boundaries(pen_csr_base *);
    virtual string get_hier_path() const;

    virtual int get_width() const;
    virtual void init();
    virtual void show();
    virtual void all(const cpp_int & l__val);
    virtual cpp_int all() const;

    virtual int get_byte_size() const;
    virtual void set_offset(uint64_t _offset);
    virtual uint64_t get_offset() const;

    virtual void set_reset_val(cpp_int _val);
    virtual cpp_int get_reset_val() const;
    virtual csr_type_t get_csr_type() const;
    virtual void set_csr_type(csr_type_t _type);
    virtual unsigned get_csr_id() const;
    virtual void set_csr_id(unsigned _id);

    virtual vector<pen_csr_base *> get_children(int level=-1) const;
    virtual vector<pen_csr_base *> get_children_prefix(string pre, int level=-1) const;
    virtual vector<pen_csr_base *> get_children_string(string str, int level=-1) const;
    virtual vector<pen_csr_base *> get_children_hier_name(string str, int level=-1) const;
    virtual uint64_t get_csr_end_addr() const { return int__csr_end_addr;}

#ifndef CSR_NO_CALLBACK        
    void append_callback(std::shared_ptr<pen_csr_callback> ptr) { callback_vec.push_back(ptr); }
    vector< std::shared_ptr<pen_csr_callback> > & get_callbacks() { return callback_vec; }
#else
    void append_callback(std::shared_ptr<pen_csr_callback> ptr) { }
    vector< std::shared_ptr<pen_csr_callback> > & get_callbacks() { 
        static vector< std::shared_ptr<pen_csr_callback> > tmp; return tmp;  
    }
#endif


    // heler function to make APIs work with chip specific stuff
    virtual void write();
    virtual void read();
    virtual void write_hw(const cpp_int &, int block_write=0);
    virtual void block_write() { PLOG_ERR("pen_csr_base :: block_write "<< endl);  }
    virtual void block_read() { PLOG_ERR("pen_csr_base:: block_read "<< endl);  }
    virtual unsigned field_diff(string value_s, bool report_error) {
        PLOG_ERR("pen_csr_base:: field_diff"<< endl); 
        return 0;
    }
    virtual cpp_int read_hw(int block_read=0);
    virtual void read_compare(int block_read=0);
    virtual bool search_field_in_db(string _name);
    virtual unsigned diff(pen_csr_base * rhs, bool report_error=true);
    virtual void set_access_no_zero_time(bool _val) {}
    virtual void load_from_cfg(bool do_write = true, bool load_fields = true);
    virtual pen_csr_base * search_csr_by_name(string _name, bool ingore_error = false);
    virtual pen_csr_base * search_csr_by_addr(uint64_t _addr);
    virtual vector<string> get_fields(int level=1) const;
    virtual bool get_field_val(string field_name, cpp_int & val, int level=1);
    virtual bool set_field_val(string field_name, cpp_int & val, int level=1);
    void field_show();
};


class pen_decoder_base : public pen_csr_base {
    public:
        pen_decoder_base(string _name, pen_csr_base * _parent = 0);
        virtual ~pen_decoder_base();
};


#endif // PEN_CSR_BASE_H
