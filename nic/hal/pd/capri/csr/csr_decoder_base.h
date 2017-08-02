#ifndef CSR_DECODER_BASE_H
#define CSR_DECODER_BASE_H
#include <stdint.h>
#include <iostream>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "cpp_int_helper.h"
#include <assert.h>
   
using namespace std;
using namespace boost::multiprecision;
using boost::multiprecision::cpp_int;

template<unsigned BITS>
using pu_cpp_int = number < cpp_int_backend<BITS, BITS, unsigned_magnitude, unchecked, void> >;

#define PU_ASSERT(ASSERT_COND) assert(ASSERT_COND)

class csr_decoder_base {

    protected: 
        csr_decoder_base * parent;
        string name;
    public:

    csr_decoder_base(string _name, csr_decoder_base * _parent = 0) {
        name = _name;
        parent = _parent;
    }

    virtual ~csr_decoder_base() {}


    virtual string get_name() {
        return name;
    }
    csr_decoder_base * get_parent() {
        return parent;
    }

    virtual void set_name(string _name) {
        name = _name;
    }
    virtual void set_parent(csr_decoder_base * _base) {
        parent = _base;
    }


    string get_hier_path() {
        string ret_val = "";
        if(get_parent() != 0) {
            ret_val = get_parent()->get_hier_path() + "." + get_name();
        } else {
            ret_val = get_name();
        }
        return ret_val;
    }

};
#endif // CSR_DECODER_BASE_H
