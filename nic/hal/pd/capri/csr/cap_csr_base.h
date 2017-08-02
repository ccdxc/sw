#ifndef CAP_CSR_BASE_H
#define CAP_CSR_BASE_H

#include <stdint.h>
#include <iostream>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "cpp_int_helper.h"
#include <assert.h>
#include "LogMsg.h"
#include "cpu.h"

using namespace std;
using namespace boost::multiprecision;
using boost::multiprecision::cpp_int;

template<unsigned BITS>
using pu_cpp_int = number < cpp_int_backend<BITS, BITS, unsigned_magnitude, unchecked, void> >;

#define PU_ASSERT(ASSERT_COND)  assert(ASSERT_COND)
#ifdef _CSV_INCLUDED_
void SLEEP(int N);
#else
    #define SLEEP(N) ;
#endif




class cap_csr_base {

    public:
        enum csr_type_t {
            CSR_TYPE_NONE = 0,
            CSR_TYPE_REGISTER,
            CSR_TYPE_MEM_ENTRY,
            CSR_TYPE_MEMORY,
            CSR_TYPE_DECODER,
            CSR_TYPE_BLOCK
        };

        typedef void (cap_csr_base::*set_function_type_t)(const cpp_int &);
        typedef cpp_int (cap_csr_base::*get_function_type_t)(void) const;
        typedef std::map<string, cap_csr_base::set_function_type_t > set_function_map_t;
        typedef std::map<string, cap_csr_base::get_function_type_t > get_function_map_t;

        typedef void (cap_csr_base::*set_array_function_type_t)(const cpp_int &, int);
        typedef cpp_int (cap_csr_base::*get_array_function_type_t)(int) const;
        typedef std::map<string, cap_csr_base::set_array_function_type_t > set_array_function_map_t;
        typedef std::map<string, cap_csr_base::get_array_function_type_t > get_array_function_map_t;


    protected:
        cap_csr_base * base__parent;
        string base__name;
        string type__name;
        uint64_t base__int__offset;
        csr_type_t base__int__csr_type;
        string base__int__csr_id;
        vector<cap_csr_base *> int__children;
        uint64_t int__csr_end_addr;
        bool int__field_init_done;

    public:

        static map<string, vector < string > > field_order_vector;
        static map<string, set_function_map_t > int_func_map__set;
        static map<string, get_function_map_t > int_func_map__get;
        static map<string, set_array_function_map_t > int_func_map__set_array;
        static map<string, get_array_function_map_t > int_func_map__get_array;

        cpp_int int_var__reset_value;
	static bool update_shadow; // When the read does not return a valid value (as in the case of model) don't update shadow

    cap_csr_base(string _name, cap_csr_base * _parent = 0);
    virtual ~cap_csr_base();
    virtual string get_name() const;
    virtual string get_type_name() const;
    virtual cap_csr_base * get_parent() const;
    virtual void set_name(string _name);
    virtual void set_type_name(string _name);
    virtual void set_parent(cap_csr_base * _base);
    virtual string get_hier_path() const;

    virtual int get_width() const;
    virtual void init();
    virtual void show();
    virtual void all(const cpp_int & l__val);
    virtual cpp_int all() const;

    virtual void set_offset(uint64_t _offset);
    virtual uint64_t get_offset() const;
    virtual int get_chip_id() const;
    virtual int get_byte_size() const;
    virtual void write();
    virtual void read();
    virtual void set_attributes(cap_csr_base * _parent, string _name, uint64_t _offset);
    virtual void set_reset_val(cpp_int _val);
    virtual cpp_int get_reset_val() const;
    virtual void register_child(cap_csr_base * _child, bool add_to_parent);
    virtual csr_type_t get_csr_type() const;
    virtual void set_csr_type(csr_type_t _type);
    virtual bool search_field_in_db(string _name);
    virtual void register_set_func(string _name, cap_csr_base::set_function_type_t func_ptr);
    virtual void register_get_func(string _name, cap_csr_base::get_function_type_t func_ptr);
    virtual void register_set_array_func(string _name, cap_csr_base::set_array_function_type_t func_ptr);
    virtual void register_get_array_func(string _name, cap_csr_base::get_array_function_type_t func_ptr);
    virtual string get_csr_inst_path(string _id) ;
    virtual string get_csr_id() const;
    virtual void set_csr_id(string _id);
    virtual vector<cap_csr_base *> get_children(int level=-1) const;
    virtual void load_from_cfg(bool do_write = true, bool load_fields = true);
    virtual cap_csr_base * search_csr_by_name(string _name);
    virtual cap_csr_base * search_csr_by_addr(uint64_t _addr);
    virtual void write_using_addr(uint64_t _addr, uint32_t data);
    virtual uint32_t read_using_addr(uint64_t _addr);

    virtual void update_block_boundaries(cap_csr_base *);
    virtual uint64_t get_csr_end_addr() const { return int__csr_end_addr;}
    virtual bool get_field_init_done() const { return int__field_init_done; }
    virtual void set_field_init_done(bool _flag) { int__field_init_done = _flag; }
};


class cap_register_base : public cap_csr_base {
    public:
        cap_register_base(string _name, cap_csr_base * _parent = 0);
        virtual ~cap_register_base();
        virtual void write();
        virtual int write_bd();
        virtual void read();
};

class cap_memory_base : public cap_csr_base {
    public:
        cap_memory_base(string _name, cap_csr_base * _parent = 0);
        virtual ~cap_memory_base();
        virtual void write();
        virtual void read();
};

class cap_decoder_base : public cap_csr_base {
    public:
        cap_decoder_base(string _name, cap_csr_base * _parent = 0);
        virtual ~cap_decoder_base();
        virtual void write();
        virtual void read();
};

class cap_block_base : public cap_csr_base {

    protected:
    int block__chip_id;
    int block__byte_size;


    //vector<cap_csr_base *> block__children;
    map<string , string> block__csr_inst_path_map;
    public:
        cap_block_base(string _name, cap_csr_base * _parent = 0);
        virtual ~cap_block_base();
        virtual int get_chip_id() const;
        virtual void set_chip_id(int _chip_id);
        virtual void write();
        virtual void read();
        virtual void set_byte_size(int _byte_size);
        virtual int  get_byte_size() const;
        //virtual void register_child(cap_csr_base * _child, bool add_to_parent);
        //virtual vector<cap_csr_base *> get_children();
        virtual void set_csr_inst_path(string _id, string _path);
        virtual string get_csr_inst_path(string _id) ;
        virtual void load_from_cfg(bool do_write = true, bool load_fields = true);
};

template<class T, unsigned MAX_DEPTH>
class cap_csr_large_array_wrapper: public cap_csr_base {
    public:
        map< int, shared_ptr<T> > entry_map;

        cap_csr_large_array_wrapper() :
            cap_csr_base("wrp", 0) {
            }

        void init() {
            T & t0 = (*this)[0];
            T & tmax = (*this)[MAX_DEPTH-1];
        }

        void show() {
            for(auto it : entry_map) {
                it.second->show();
            }
        }

        void set_attributes(cap_csr_base * _parent, string _name, uint64_t _offset) {
            cap_csr_base::set_attributes(_parent, _name, _offset);
        }

        T& operator[](int idx)       { 
            if(entry_map.find(idx) != entry_map.end()) {
                // already exists
            } else if(idx < MAX_DEPTH) {
                shared_ptr<T> tmp(new T);
                tmp->set_attributes(this, get_name() + "[" + to_string(idx) + "]", base__int__offset + (tmp->get_byte_size()*idx));
                entry_map[idx] = tmp;
            } else {
                PLOG_ERR("Requested more than MAX_DEPTH " << MAX_DEPTH << " HIER : " << get_hier_path() << endl);
                return *entry_map[0].get();
            }

            return *entry_map[idx].get(); 
        }
        
};


#endif // CAP_CSR_BASE_H
