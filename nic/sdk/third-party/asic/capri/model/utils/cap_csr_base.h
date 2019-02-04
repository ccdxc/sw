#ifndef CAP_CSR_BASE_H
#define CAP_CSR_BASE_H

#include "pen_csr_base.h"
#ifndef CAP_CSR_LARGE_ARRAY_THRESHOLD
#define CAP_CSR_LARGE_ARRAY_THRESHOLD  PEN_CSR_LARGE_ARRAY_THRESHOLD
#endif 


using namespace std;
using namespace boost::multiprecision;
using boost::multiprecision::cpp_int;


// forward declaration
class cap_csr_base;

class cap_csr_callback : public pen_csr_callback {
    public:

        cap_csr_callback() : pen_csr_callback() {}
        virtual ~cap_csr_callback() {}
        virtual void pre_csr_mem_zerotime_hdl_write(cap_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 
        virtual void post_csr_mem_zerotime_hdl_write(cap_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 
        virtual void pre_csr_mem_zerotime_hdl_read(cap_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 
        virtual void post_csr_mem_zerotime_hdl_read(cap_csr_base * ptr, string & path, cpp_int & data, unsigned & bits) {} 

        
};


class cap_csr_base : public pen_csr_base {

    public:


    protected:
        bool int__field_init_done;
        bool int__access_no_zero_time;
        bool int__secure;
        bool int__set_attributes_done;
#ifndef CSR_NO_INST_PATH        
        unsigned base__int__csr_id; 
        map<unsigned , string> base__csr_inst_path_map;
#endif        

    public:

	     static bool update_shadow; // When the read does not return a valid value (as in the case of model) don't update shadow
        static u_int32_t int__field_last_lfc_count;

    cap_csr_base(string _name, cap_csr_base * _parent = nullptr);
    std::string get_cfg_name();
    virtual ~cap_csr_base();
    virtual unsigned field_diff(string value_s, bool report_error);
    virtual void field_show();

    virtual int get_chip_id() const;
    virtual unsigned get_csr_id() const;
    virtual void set_csr_id(unsigned _id);
    virtual void write();
    virtual void read();
    virtual void write_hw(const cpp_int &, int block_write=0);
    virtual cpp_int read_hw(int block_read=0);
    virtual void read_compare(int block_read=0);

    vector<unsigned int> convert_cpp_int_to_vector_32(const cpp_int & _data, uint64_t width);
    vector<unsigned int> convert_cpp_int_to_vector_8(const cpp_int & _data, uint64_t width);
    cpp_int convert_vector_32_to_cpp_int(vector<unsigned int> read_vector);

    virtual void set_attributes(cap_csr_base * _parent, string _name, uint64_t _offset);
    virtual void set_attributes_null(string _name, uint64_t _offset);
    virtual string get_csr_inst_path(unsigned _id) ;
    virtual void set_csr_inst_path(unsigned _id, string _path, unsigned call_set_csr_id_hier=1);

    virtual vector<string> get_fields(int level=1) const;
    virtual void load_from_cfg(bool do_write = true, bool load_fields = true);
    virtual u_int32_t load_from_cfg_rtn_status(bool do_write = true, bool load_fields = true);
    virtual pen_csr_base * search_csr_by_name(string _name, bool ingore_error = false);
    virtual pen_csr_base * search_csr_by_addr(uint64_t _addr);
    virtual void write_using_addr(uint64_t _addr, u_int32_t data, bool update = true);
    virtual u_int32_t read_using_addr(uint64_t _addr, bool update = true);

    virtual bool get_field_init_done() const { return int__field_init_done; }
    virtual void set_field_init_done(bool _flag, bool call_hier=false);

    virtual void block_write() { PLOG_ERR("cap_csr_base:: block_write "<< endl);  }
    virtual void block_read() { PLOG_ERR("cap_csr_base:: block_read "<< endl);  }
    virtual unsigned diff(pen_csr_base *, bool report_error=true);
    virtual void set_access_no_zero_time(bool _val) { int__access_no_zero_time = _val; }
    virtual bool get_access_no_zero_time() const;
    virtual void set_access_secure(bool _val) { int__secure = _val; }
    virtual bool get_access_secure();

#ifdef _COSIM_
    string get_field_str_val(string tgt_field_name); 
    virtual bool set_field_str_val(string field_name, string val, int level=1);
#endif    

    virtual bool get_field_val(const char * field_name, cpp_int & val,int level=1);
    virtual bool set_field_val(const char * field_name, cpp_int & val,int level=1);
    virtual bool get_field_val(string field_name, cpp_int & val, int level=1);
    virtual bool set_field_val(string field_name, cpp_int & val, int level=1);
    virtual bool search_field_in_db(string _name);

};


class cap_register_base : public cap_csr_base {
    private:
    public:
        cap_register_base(string _name, cap_csr_base * _parent = nullptr);
        virtual ~cap_register_base();
        virtual void write();
        virtual void read();
        vector<unsigned int> get_write_vec();
        virtual void write_hw(const cpp_int &, int block_write=0);
        virtual cpp_int read_hw(int block_read=0);
        virtual void read_compare(int block_read=0);


        virtual void block_write();
        virtual void block_read();
        virtual void block_byte_write() ;
        virtual void block_byte_read() ;

        virtual string get_csr_name();
        virtual bool write_all_fields_zero_time();
        virtual bool read_all_fields_zero_time();
};

class cap_memory_base : public cap_csr_base {
    public:
        cap_memory_base(string _name, cap_csr_base * _parent = nullptr);
        virtual ~cap_memory_base();
        virtual void write();
        virtual void read();
};

class cap_decoder_base : public cap_csr_base {
    public:
        cap_decoder_base(string _name, cap_csr_base * _parent = nullptr);
        virtual ~cap_decoder_base();
        virtual void write();
        virtual void read();
};

class cap_block_base : public cap_csr_base {

    protected:
    int block__chip_id;
    int block__byte_size;


    //vector<cap_csr_base *> block__children;
    public:
        cap_block_base(string _name, cap_csr_base * _parent = nullptr);
        virtual ~cap_block_base();
        virtual int get_chip_id() const;
        virtual void set_chip_id(int _chip_id);
        virtual void write();
        virtual void read();
        virtual void set_byte_size(int _byte_size);
        virtual int  get_byte_size() const;
        //virtual void register_child(cap_csr_base * _child, bool add_to_parent);
        //virtual vector<cap_csr_base *> get_children();
};

template<class T, unsigned MAX_DEPTH>
class cap_csr_large_array_wrapper: public cap_csr_base {
    public:
        map< int, std::shared_ptr<T> > entry_map;

        cap_csr_large_array_wrapper() :
            cap_csr_base("wrp", 0) {
            }

        void init() {
            T & t0 = (*this)[0];
            T & tmax = (*this)[MAX_DEPTH-1];
            PLOG("csr", t0.get_hier_path() << " created " << endl);
            PLOG("csr", tmax.get_hier_path() << " created " << endl);
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
            } else if(unsigned(idx) < MAX_DEPTH) {
                std::shared_ptr<T> tmp(new T);
                tmp->set_attributes(this, get_name() + "[" + to_string(idx) + "]", base__int__offset + (tmp->get_byte_size()*idx));
                entry_map[idx] = tmp;
            } else {
                PLOG_ERR("Requested more than MAX_DEPTH " << MAX_DEPTH << " HIER : " << get_hier_path() << endl);
                return *entry_map[0].get();
            }

            return *entry_map[idx].get();
        }

};

  int get_idx_from_field_name(string field_name);
  vector<string> cap_util_split_by_str(string str, char delimiter);

#endif // CAP_CSR_BASE_H
