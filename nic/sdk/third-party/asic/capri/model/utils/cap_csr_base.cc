#include "cap_csr_base.h"
#include "sknobs.h"
#include "pknobs_reader.h"
#include <boost/algorithm/string/case_conv.hpp>
#ifdef _CSV_INCLUDED_


#ifdef PEN_CSR_ZERO_TIME_ENABLE
#define PEN_CSR_HDL_MAX_BITS 4096
extern "C" int sv2c_uvm_hdl_check_path(const char* path);
extern "C" int sv2c_uvm_hdl_deposit(const char * path, svLogicVecVal * data);
extern "C" int sv2c_uvm_hdl_read(const char * path, svLogicVecVal * data);
cpp_int_helper hdl_cpp_helper;
bool csr_zerotime_hdl_write(const char* path, cpp_int & data, unsigned bits) {
    int ret_val = 0;

    svScope new_scope;
    svScope old_scope = svGetScope();
    if (sknobs_exists((char *)"stub_build")) {
       new_scope = svGetScopeFromName("\\LIBVERIF.puvm_cpp_reg_shim_pkg ");
    } else {
       new_scope = svGetScopeFromName("puvm_cpp_reg_shim_pkg");
    }
    svSetScope(new_scope);


    svLogicVecVal sv_data[PEN_CSR_HDL_MAX_BITS/32]; 
    hdl_cpp_helper.convert_cpp_int_to_svlogic(sv_data, data, bits);
    if(sv2c_uvm_hdl_check_path(path)) {
        ret_val = sv2c_uvm_hdl_deposit(path, sv_data);
        if(ret_val == 0) {
            PLOG_MSG("desposit failed for path:" << path << endl);
        }
    } else {
        PLOG_MSG("uvm_hdl_check_path: " << path << " doesn't exist!" << endl);
    }
    svSetScope(old_scope);
    return ret_val;
}


bool csr_zerotime_hdl_read(const char * path, cpp_int & data, unsigned bits) {
    int ret_val = 0;
    svScope new_scope;
    svScope old_scope = svGetScope();
    if (sknobs_exists((char *)"stub_build")) {
       new_scope = svGetScopeFromName("\\LIBVERIF.puvm_cpp_reg_shim_pkg ");
    } else {
       new_scope = svGetScopeFromName("puvm_cpp_reg_shim_pkg");
    }
    svSetScope(new_scope);

    svLogicVecVal sv_data[PEN_CSR_HDL_MAX_BITS/32]; 
    if(sv2c_uvm_hdl_check_path(path)) {
        ret_val = sv2c_uvm_hdl_read(path, sv_data);
        if(ret_val) {
            hdl_cpp_helper.convert_svlogic_to_cpp_int(data, sv_data, bits);
        }
    } 
    svSetScope(old_scope);
    return ret_val;
}
#endif 
void SLEEP(int N) {
    svScope new_scope;
    svScope old_scope = svGetScope();
    if (sknobs_exists((char *)"stub_build")) {
       new_scope = svGetScopeFromName("\\LIBVERIF.puvm_cpp_reg_shim_pkg ");
    } else {
       new_scope = svGetScopeFromName("puvm_cpp_reg_shim_pkg");
    }
    svSetScope(new_scope);
    wait_time(N * 1000);
    svSetScope(old_scope);
}




#endif

u_int32_t cap_csr_base::int__field_last_lfc_count=0;

cap_csr_base::cap_csr_base(string _name, cap_csr_base * _parent) : pen_csr_base(_name, _parent) {
    int__field_init_done = false;
    int__access_no_zero_time = false;
    int__secure = false;
    int__set_attributes_done = false;
#ifndef CSR_NO_INST_PATH        
    base__int__csr_id = 0;
#endif
}


cap_csr_base::~cap_csr_base() { }

int get_idx_from_field_name(string field_name) {

    size_t first_idx_pos = field_name.find_last_of('[');
    size_t last_idx_pos = field_name.find_last_of(']');
    if( (first_idx_pos == string::npos) || (last_idx_pos == string::npos)) {
        PLOG_ERR("Unexpected: field_name doesn't have [foo] "<< field_name << endl);
    }

    string idx_str = field_name.substr ( first_idx_pos+1 , last_idx_pos - first_idx_pos);
    return stoi(idx_str);
}



int cap_csr_base::get_chip_id() const {
    int ret_val = 0;
    if(get_parent() != 0) {
        cap_csr_base * l_parent = dynamic_cast<cap_csr_base *>(get_parent());
        ret_val = l_parent->get_chip_id();
    } else {
        PLOG_ERR("cap_csr_base: parent is null and its not block or system. Can't get chip_id" << endl);
            return 0;
    }
    return ret_val;
}
#ifndef CSR_NO_INST_PATH        
unsigned cap_csr_base::get_csr_id() const {
    return base__int__csr_id;
}
void cap_csr_base::set_csr_id(unsigned _id) {
    base__int__csr_id = _id;
}
#else 
unsigned cap_csr_base::get_csr_id() const {
    return 0;
}
void cap_csr_base::set_csr_id(unsigned _id) { }
#endif

void cap_csr_base::write() {
    PLOG_ERR("cap_csr_base:: write not to be used" << endl);
}
void cap_csr_base::read() {
    PLOG_ERR("cap_csr_base:: read not to be used" << endl);
}
void cap_csr_base::write_hw(const cpp_int &, int block_write) {
    PLOG_ERR("cap_csr_base:: write_hw not to be used" << endl);
}
cpp_int cap_csr_base::read_hw(int block_read) {
    cpp_int read_val;
    PLOG_ERR("cap_csr_base:: read_hw not to be used" << endl);
    return read_val;
}
void cap_csr_base::read_compare(int block_read) {
    PLOG_ERR("cap_csr_base:: read_compare not to be used" << endl);
}

void cap_csr_base::set_attributes_null(string _name, uint64_t _offset) {
    set_attributes(0, _name, _offset);
}
void cap_csr_base::set_field_init_done(bool _flag, bool call_hier) {
    int__field_init_done = _flag; 
    if(call_hier) {
        for(auto itr : get_children(1)) {
            cap_csr_base * l_itr = dynamic_cast< cap_csr_base *>(itr);
            l_itr->set_field_init_done(_flag, call_hier);
        }
    }
}

void cap_csr_base::set_attributes(cap_csr_base * _parent, string _name, uint64_t _offset)
{
    if(get_csr_type() == CSR_TYPE_NONE) { set_csr_type(CSR_TYPE_REGISTER); }
    set_name(_name);
    set_offset(_offset);
    int__csr_end_addr = (get_width() != 0) ? (get_offset() + ((get_width() +7)/8) -1) : get_offset();
    set_parent(_parent);
    init();
    set_field_init_done(true);
    if(_parent) { _parent->update_block_boundaries(this); }
    int__set_attributes_done = true;
}

bool cap_csr_base::search_field_in_db(string _name) {
    for(auto it_field : get_fields()) {
        if(0 == it_field.compare(_name)) return true;
    }
    return false;
}

void cap_memory_base::write() {
    PLOG_ERR("cap_memory_base:: write not to be used" << endl);
}
void cap_memory_base::read() {
    PLOG_ERR("cap_memory_base:: read not to be used" << endl);
}
cap_memory_base::cap_memory_base(string _name, cap_csr_base * _parent):
    cap_csr_base(_name, _parent) {
        set_csr_type(CSR_TYPE_MEMORY);
}
cap_memory_base::~cap_memory_base() { }


void cap_decoder_base::write() {
    PLOG_ERR("cap_decoder_base:: write not to be used" << endl);
}
void cap_decoder_base::read() {
    PLOG_ERR("cap_decoder_base:: read not to be used" << endl);
}
cap_decoder_base::cap_decoder_base(string _name, cap_csr_base * _parent):
    cap_csr_base(_name, _parent) {
        set_csr_type(CSR_TYPE_DECODER);
}
cap_decoder_base::~cap_decoder_base() { }

void cap_block_base::write() {
    PLOG_ERR("cap_block_base:: write not to be used" << endl);
}
void cap_block_base::read() {
    PLOG_ERR("cap_block_base:: read not to be used" << endl);
}
cap_block_base::cap_block_base(string _name, cap_csr_base * _parent):
    cap_csr_base(_name, _parent) {

        block__chip_id = 0;
        set_name(_name);
        set_csr_type(CSR_TYPE_BLOCK);
        //set_attributes(_parent, _name, 0);
    }
cap_block_base::~cap_block_base() { }
int cap_block_base::get_chip_id() const {
    return block__chip_id;
}
void cap_block_base::set_chip_id(int _chip_id) {
    block__chip_id = _chip_id;
}
void cap_block_base::set_byte_size(int _byte_size) {
    block__byte_size = _byte_size;
}
int cap_block_base::get_byte_size() const {
    return block__byte_size;
}


vector<string> cap_csr_base::get_fields(int level) const {
    vector<string> ret_vec;
    //if(field_order_vector.find(get_type_name()) != field_order_vector.end()) {
    //    ret_vec = field_order_vector[get_type_name()];
    //}
    return ret_vec;
}

void cap_csr_base::set_csr_inst_path(unsigned _id, string _path, unsigned call_set_csr_id_hier) {
#ifndef CSR_NO_INST_PATH        
    base__csr_inst_path_map[_id] = _path;
    if(call_set_csr_id_hier) {
        for(auto itr: get_children()) {
            itr->set_csr_id(_id);
        }
    }
#endif    
}

string cap_csr_base::get_csr_inst_path(unsigned _id) {
#ifndef CSR_NO_INST_PATH        
    string path = "";
    if(get_parent() != 0) {
        cap_csr_base * l_parent = dynamic_cast<cap_csr_base *>(get_parent());
        path = l_parent->get_csr_inst_path(_id);
    }
    if(base__csr_inst_path_map.find(_id) != base__csr_inst_path_map.end()) {
        if(path.compare("")) { return path + "." + base__csr_inst_path_map[_id]; }
        else {return base__csr_inst_path_map[_id]; } 
    }
    else return path; 
#else
    return "";
#endif    
}

string cap_register_base::get_csr_name() {
#ifndef CSR_NO_INST_PATH        
    if(base__csr_inst_path_map.find(get_csr_id()) != base__csr_inst_path_map.end()) {
        return  base__csr_inst_path_map[get_csr_id()]; 
    } 
#endif    
    if(get_parent() != 0) {
        if(get_parent()->get_csr_type() == CSR_TYPE_MEMORY) {
            string t_name = get_name();
            if (t_name.find("entry") != string::npos) {
                t_name.replace( t_name.find("entry") ,std::string("entry").length(), "");
            }
            return t_name; 
        } else {
            return "csr_internal_" + get_name() + "_reg";
        }
    }
    return "";
}




vector<unsigned int> cap_csr_base::convert_cpp_int_to_vector_32(const cpp_int & _data, uint64_t width) {
    vector<unsigned int> write_vector;
    export_bits(_data, std::back_inserter(write_vector), 32);
    reverse(write_vector.begin(), write_vector.end());
    int words = (width+31)/32;
    write_vector.resize(words);
    for(int ii = write_vector.size(); ii < words; ii++) {
        write_vector[ii] = 0;
    }
    PU_ASSERT(write_vector.size() <= unsigned(words));
    return write_vector;

}

vector<unsigned int> cap_csr_base::convert_cpp_int_to_vector_8(const cpp_int & _data, uint64_t width) {
    vector<unsigned int> write_byte_vector;
    export_bits(_data, std::back_inserter(write_byte_vector), 8);
    reverse(write_byte_vector.begin(), write_byte_vector.end());
    int num_bytes = (width+7)/8;
    write_byte_vector.resize(num_bytes);
    for(int ii = write_byte_vector.size(); ii < num_bytes; ii++) {
        write_byte_vector[ii] = 0;
    }
    PU_ASSERT(write_byte_vector.size() <= unsigned(num_bytes));
    return write_byte_vector;
}


cpp_int cap_csr_base::convert_vector_32_to_cpp_int(vector<unsigned int> read_vector) {
    cpp_int read_val;
    reverse(read_vector.begin(), read_vector.end());
    import_bits(read_val, read_vector.begin(), read_vector.end());
    return read_val;
}



vector<unsigned int> cap_register_base::get_write_vec() {
    return convert_cpp_int_to_vector_32(all(), get_width());
}


bool cap_register_base::write_all_fields_zero_time() {
    bool zerotime_success=false;
#ifdef PEN_CSR_ZERO_TIME_ENABLE
    string inst_path = get_csr_inst_path(get_csr_id());
    unsigned width = PEN_CSR_HDL_MAX_BITS;

    if(get_parent()->get_csr_type() == CSR_TYPE_MEMORY) {
        cpp_int temp_cpp = all();
        string path = string(inst_path + "." + get_csr_name());
        if (callback_vec.size() == 0) {
          // assume this means the callback is with the parent
          
        }
        for(auto callback_ptr: ((callback_vec.size() == 0) ? get_parent()->get_callbacks() : callback_vec)) { callback_ptr->pre_csr_mem_zerotime_hdl_write(this, path, temp_cpp, width); }
        zerotime_success = csr_zerotime_hdl_write( path.c_str(), temp_cpp, width);
        for(auto callback_ptr: ((callback_vec.size() == 0) ? get_parent()->get_callbacks() : callback_vec)) { callback_ptr->post_csr_mem_zerotime_hdl_write(this, path, temp_cpp, width); }
    } else {

        string cur_name = get_name();
        std::replace( cur_name.begin(), cur_name.end(), '[', '_');
        std::replace( cur_name.begin(), cur_name.end(), ']', '_');
        if (cur_name.compare(cur_name.length()-1,1,"_") == 0) {
            cur_name.erase (cur_name.length()-1, 1);
        }

        for(auto orig_field_name : get_fields(1)) {
            string field_name = orig_field_name;
            if(field_name.compare("all") ==0) continue;
            std::replace( field_name.begin(), field_name.end(), '[', '_');
            std::replace( field_name.begin(), field_name.end(), ']', '_');
            if (field_name.compare(field_name.length()-1,1,"_") == 0) {
                field_name.erase (field_name.length()-1, 1);
            }
            
            cpp_int val;
            get_field_val(orig_field_name.c_str(),val, 1);
            zerotime_success = csr_zerotime_hdl_write( string(inst_path + ".csr_internal_field_" + cur_name + "_" + field_name).c_str(), val , width);
            if(!zerotime_success) return zerotime_success;
        }

    }
#endif    

    return zerotime_success;

}

bool cap_register_base::read_all_fields_zero_time() {
    bool zerotime_success=false;
#ifdef PEN_CSR_ZERO_TIME_ENABLE
    string inst_path = get_csr_inst_path(get_csr_id());
    unsigned width = PEN_CSR_HDL_MAX_BITS;

    if(get_parent()->get_csr_type() == CSR_TYPE_MEMORY) {
        cpp_int temp_cpp;
        string path = string(inst_path + "." + get_csr_name());
        for(auto callback_ptr: ((callback_vec.size() == 0) ? get_parent()->get_callbacks() : callback_vec)) { callback_ptr->pre_csr_mem_zerotime_hdl_read(this, path, temp_cpp, width); }
        zerotime_success = csr_zerotime_hdl_read(path.c_str(), temp_cpp, width);
        for(auto callback_ptr: ((callback_vec.size() == 0) ? get_parent()->get_callbacks() : callback_vec)) { callback_ptr->post_csr_mem_zerotime_hdl_read(this, path, temp_cpp, width); }
        if(zerotime_success) {  all(temp_cpp); }
    } else {

        string cur_name = get_name();
        std::replace( cur_name.begin(), cur_name.end(), '[', '_');
        std::replace( cur_name.begin(), cur_name.end(), ']', '_');
        if (cur_name.compare(cur_name.length()-1,1,"_") == 0) {
            cur_name.erase (cur_name.length()-1, 1);
        }

        for(auto orig_field_name : get_fields()) {
            string field_name = orig_field_name;
            if(field_name.compare("all") ==0) continue;
            std::replace( field_name.begin(), field_name.end(), '[', '_');
            std::replace( field_name.begin(), field_name.end(), ']', '_');
            if (field_name.compare(field_name.length()-1,1,"_") == 0) {
                field_name.erase (field_name.length()-1, 1);
            }

            cpp_int val;
            zerotime_success = csr_zerotime_hdl_read( string(inst_path + ".csr_internal_field_" + cur_name + "_" + field_name).c_str(), val , width);
            if(!zerotime_success) {
                return zerotime_success;
            } else {
                set_field_val(orig_field_name.c_str(), val,1);
            }

        }

    }
#endif    

    return zerotime_success;

}


void cap_register_base::write() {
    bool zerotime_success =false;

#ifdef PEN_CSR_ZERO_TIME_ENABLE

    if( (cpu::access()->get_access_type() == back_door_e) && (get_access_no_zero_time() == false) ) {
        zerotime_success = write_all_fields_zero_time();
        if(zerotime_success) {
            PLOG_MSG("csr write zerotime: " << get_hier_path() << " val: 0x" << hex << all() << endl << dec);
        }
	else {
            PLOG_MSG("csr write zerotime failed: " << get_hier_path() << " val: 0x" << hex << all() << endl << dec);
	}
    }
#endif    

    if(!zerotime_success) {
       int block_access = sknobs_get_value((char *)"cap_csr_base/block_access", 0);
       if (block_access == 0) {
          write_hw(all(), 0);
       } else {
          write_hw(all(), 1);
       }
    }
}
void cap_register_base::read() {
    bool zerotime_success =false;
#ifdef PEN_CSR_ZERO_TIME_ENABLE
    if( (cpu::access()->get_access_type() == back_door_e) && (get_access_no_zero_time() == false) ) {
        zerotime_success = read_all_fields_zero_time();
        if(zerotime_success) {
            PLOG_MSG("csr read zerotime: " << get_hier_path() << " val: 0x" << hex << all() << endl << dec);
        }
    }
#endif    

    if(!zerotime_success && update_shadow) {
       int block_access = sknobs_get_value((char *)"cap_csr_base/block_access", 0);
       if (block_access == 0) {
          all(read_hw(0));
       } else {
          all(read_hw(1));
       }
    }
}



void cap_register_base::block_write() {
    write_hw(all(), 1);
}

void cap_register_base::block_byte_write() {
    write_hw(all(), 2);
}

void cap_register_base::block_read() {
    if (update_shadow) {
      all(read_hw(1));
    }
}

void cap_register_base::block_byte_read() {
    if (update_shadow) {
      all(read_hw(2));
    }
}


void cap_register_base::write_hw(const cpp_int & write_val, int block_write) {
    uint64_t width = get_width();
    int words = (width+31)/32;
    auto write_vector = convert_cpp_int_to_vector_32(write_val, width);
    auto write_byte_vector = convert_cpp_int_to_vector_8(write_val, width);

    PU_ASSERT(write_vector.size() <= unsigned(words));

    uint64_t offset;
    uint64_t byte_size;
    offset = get_offset();
    byte_size = write_byte_vector.size();
    PLOG("csr", "write_hw: name: " << get_hier_path() << " addr: 0x" << hex << offset << " data: 0x" << write_val << dec << " byte_size:" << byte_size << endl);
    //PLOG_MSG("write_hw: name: " << get_hier_path() << " addr: 0x" << hex << offset << " data: 0x" << write_val << dec << " byte_size:" << byte_size << endl);

    bool cpu_no_zero_time = get_access_no_zero_time();
#ifdef PEN_CSR_ZERO_TIME_ENABLE
    cpu_no_zero_time = true;
#endif    
    if(block_write) {
       if (block_write == 2) {
          PLOG("csr","block byte_write: name: " << get_hier_path() << " addr: 0x" << hex << offset << "data: 0x" << write_val << dec << " size:" << write_byte_vector.size() << endl);
          cpu::access()->block_write(get_chip_id(), offset, write_byte_vector.size(), write_byte_vector, cpu_no_zero_time, get_access_secure() ? secure_byte_acc_e : none_byte_acc_e );
       } else {
          PLOG("csr","block write: name: " << get_hier_path() << " addr: 0x" << hex << offset << "data: 0x" << write_val << dec << endl);
          //PLOG_MSG("block write: name: " << get_hier_path() << " addr: 0x" << hex << offset << "data: 0x" << write_val << dec << endl);
          cpu::access()->block_write(get_chip_id(), offset, write_vector.size(), write_vector, cpu_no_zero_time, get_access_secure() ? secure_acc_e : none_acc_e );
       }
    } else {
        for(unsigned int ii = 0; ii < write_vector.size(); ii++) {
            PLOG("csr", "write byte: name : " << get_hier_path() << " addr 0x" << hex << offset + (ii*4) << " data: 0x" << write_vector[ii] << dec << endl);
            cpu::access()->write(get_chip_id(), offset + (ii*4), write_vector[ ii], cpu_no_zero_time, get_access_secure() ? secure_acc_e : none_acc_e );
        }
    }
}

cpp_int cap_register_base::read_hw(int block_read) {
    uint64_t offset, width;
    int chip_id;
    offset = get_offset();
    width = get_width();
    chip_id = get_chip_id();
    vector<unsigned int> read_vector;
    int words = (width+31)/32;
    int num_bytes = (width+7)/8;
    if (block_read == 2) {
       read_vector.resize(num_bytes);
    }
    else {
       read_vector.resize(words);
    }
    PLOG("csr", "read_hw name : " << get_hier_path() << " addr 0x" << hex << offset << dec << endl);
    bool cpu_no_zero_time = get_access_no_zero_time();
#ifdef PEN_CSR_ZERO_TIME_ENABLE
    cpu_no_zero_time = true;
#endif    

    if(block_read) {
       if (block_read == 2) {
          read_vector = cpu::access()->block_read(get_chip_id(), offset, read_vector.size(), cpu_no_zero_time, get_access_secure() ? secure_byte_acc_e : none_byte_acc_e );
          PLOG("csr","block byte read: name: " << get_hier_path() << " addr: 0x" << hex << offset << dec << endl);
       } else {
          read_vector = cpu::access()->block_read(get_chip_id(), offset, read_vector.size(), cpu_no_zero_time, get_access_secure() ? secure_acc_e : none_acc_e );
          PLOG("csr","block read: name: " << get_hier_path() << " addr: 0x" << hex << offset << dec << endl);
          //PLOG_MSG("block read: name: " << get_hier_path() << " addr: 0x" << hex << offset << dec << endl);
       }
    } else {
        for(int ii = 0; ii < words; ii++) {
            PLOG("csr", "read byte: name : " << get_hier_path() << " addr 0x" << hex << offset + (ii*4)  << dec << endl);
            read_vector[ii] = cpu::access()->read(chip_id, offset + (ii*4), cpu_no_zero_time, get_access_secure() ? secure_acc_e : none_acc_e );
        }
    }

    cpp_int ret_val = convert_vector_32_to_cpp_int(read_vector);
    return ret_val;
}

void cap_register_base::read_compare(int block_read) {
  bool zerotime_success =false;

#ifdef PEN_CSR_ZERO_TIME_ENABLE
  if (update_shadow && (block_read == 0)) {
    cpp_int shadow_all = all();
    if( (cpu::access()->get_access_type() == back_door_e) && (get_access_no_zero_time() == false) ) {
        zerotime_success = read_all_fields_zero_time();
        if(zerotime_success) {
            PLOG_MSG("csr read zerotime: " << get_hier_path() << " val: 0x" << hex << all() << endl << dec);
            if (shadow_all != all()) {
              PLOG_ERR("read_compare : " << get_hier_path() << " exp: 0x" << hex << shadow_all << " actual: 0x" << all() << dec << endl);
            }
        }
    }
  }
#endif    
  if (!zerotime_success) {
    cpp_int read_val = read_hw(block_read);
    if(read_val != all()) {
        PLOG_ERR("read_compare : " << get_hier_path() << " exp: 0x" << hex << all() << " actual: 0x" << read_val << dec << endl);
    }
    if (update_shadow) {
      all(read_val);
    }
  }
}

unsigned cap_csr_base::diff(pen_csr_base * rhs, bool report_error) {
    unsigned errors = 0;
    if(all() != rhs->all()) {
        errors++;
        if(report_error) {
            PLOG_ERR("all: 0x" << hex << all() << " != 0x"<< rhs->all() << endl);
        //} else {
        //    PLOG_MSG("all: 0x" << hex << all() << " != 0x"<< rhs->all() << endl);
        }


        //if(get_type_name().compare(rhs->get_type_name()) ==0) {
            for(auto per_field : get_fields()) {
                if(per_field.compare("all") == 0) continue;
                cpp_int this_val, rhs_val;
                this->get_field_val(per_field.c_str(), this_val, 1);
                rhs->get_field_val(per_field.c_str(), rhs_val, 1);
                if(this_val != rhs_val) {
                    PLOG_MSG(per_field << ": 0x" << hex << this_val << " != 0x"<< rhs_val << endl);
                }
            }
        //}


        auto lhs_children = get_children(1);
        auto rhs_children = rhs->get_children(1);

        if(lhs_children.size() == rhs_children.size()) {
            for(unsigned ii = 0; ii < lhs_children.size(); ii++) {
                if(lhs_children[ii]->get_name().compare(rhs_children[ii]->get_name())) {
                    errors += lhs_children[ii]->diff(rhs_children[ii]);
                }
            }
        }
        
    }
    return errors;
}



unsigned cap_csr_base::field_diff(string value_s, bool report_error) {

    unsigned errors = 0;
    cpp_int val;
    cpp_int val_tmp;

    if (isdigit(value_s[0])) {
        val = cpp_int(value_s);
    } else {
        PLOG_ERR("passed value_s to field_diff is wrong all: 0x" << hex << val << dec << endl);
    }

    if(all() != val) {
        errors++;
        if(report_error) {
            PLOG_ERR("all: 0x" << hex << all() << " != 0x"<< val << endl);
        }
        val_tmp = all();
        all(val);
        PLOG_MSG("=========================================" << endl) ;
        field_show();
        PLOG_MSG("-----------------------------------------" << endl) ;
        all(val_tmp);
        field_show();
        PLOG_MSG("=========================================" << endl) ;
    }
    return errors;
}

void cap_csr_base::field_show() {
    string cur_path = get_hier_path();
    for(auto per_field : get_fields()) {
       if(per_field.compare("all") == 0) continue;
       cpp_int field_val;
       if(!get_field_val(per_field.c_str(), field_val, 1)) {
          PLOG_ERR("show: fail, field " << per_field << " get_field_val failed" << endl);
       }
       PLOG_MSG(cur_path << "." << hex << per_field << ": 0x" << field_val << endl << dec) ;
    }
    for(auto it_child: get_children(1)) {
       it_child->field_show();
    }


}


cap_register_base::cap_register_base(string _name, cap_csr_base * _parent):
    cap_csr_base(_name, _parent) {
        set_csr_type(CSR_TYPE_REGISTER);
}

bool cap_csr_base::get_access_no_zero_time() const {
    if(int__access_no_zero_time) return true;
    else if(get_parent()) {
        cap_csr_base * l_parent = dynamic_cast<cap_csr_base *>(get_parent());
        return l_parent->get_access_no_zero_time();
    }
    return false;
}

bool cap_csr_base::get_access_secure() { 
  if (sknobs_get_value((char *)"cap_csr_base/secure_access_override", 0)) {
    return 1;
  }
  else {
    return int__secure; 
  }
}



static cpp_int get_val (string & value_s, string path="")
{
    cpp_int val;

    if (isdigit(value_s[0])) {
        val = cpp_int(value_s);
    } else {
        val = PKnobsReader::evalKnob(path);
    }

    return val;
}

std::string cap_csr_base::get_cfg_name()
{
    size_t index = 0;
    string name = get_hier_path();
    std::replace( name.begin(), name.end(), '.', '/');
    std::replace( name.begin(), name.end(), '[', '/');
    std::replace( name.begin(), name.end(), ']', '/');
    if (name.compare(name.length()-1,1,"/") == 0) {
        name.erase (name.length()-1, 1);
    }
    while(true) {
        index = name.find("//", index);
        if(index == std::string::npos) break;
        name.replace(index, 2, "/");
        index += 1;
    }
    if (name[name.length() - 1] == '/') {
        name.pop_back();
    }
    return name;
}

cap_register_base::~cap_register_base() { }


void cap_csr_base::load_from_cfg(bool do_write, bool load_fields) {
  bool write_needed = false;
  string name = get_cfg_name();
  string value_s = "";
  if (sknobs_exists((char*) name.c_str()) != 0){
    value_s = sknobs_get_string((char*) name.c_str(),(char*)"");
    cpp_int val = get_val(value_s, name);
    all(val);
    if (all() != get_reset_val())  {
      cap_csr_base::int__field_last_lfc_count++;
      //PLOG_MSG("load_from_cfg: writing reg " << name.c_str() << endl);
      if (do_write) { write_needed = true; }
    }
  }

  if(load_fields && sknobs_prefix_exists((name+"/").c_str())) {
      for(auto orig_field_name : get_fields()) {
          string field_name = orig_field_name;
          std::replace( field_name.begin(), field_name.end(), '.', '/');
          std::replace( field_name.begin(), field_name.end(), '[', '/');
          std::replace( field_name.begin(), field_name.end(), ']', '/');
          if (field_name.compare(field_name.length()-1,1,"/") == 0) {
              field_name.erase (field_name.length()-1, 1);
          }
          field_name = name + "/" + field_name;
          if (sknobs_exists((char*) field_name.c_str()) != 0) {
              value_s = sknobs_get_string((char*) field_name.c_str(),(char*)"");
              cpp_int val = get_val(value_s, field_name);
              set_field_val(orig_field_name.c_str(), val, 1);
              cap_csr_base::int__field_last_lfc_count++;
              if(do_write) write_needed = true;
          }
      }

  }

  if(write_needed) {
      write();
  }
  if (sknobs_prefix_exists((name+"/").c_str())) {
      for (auto i: get_children(1)) {
          i->load_from_cfg(do_write, load_fields);
      }
  }

}

u_int32_t cap_csr_base::load_from_cfg_rtn_status(bool do_write, bool load_fields) {
   cap_csr_base::int__field_last_lfc_count=0;
   load_from_cfg(do_write,load_fields);
   return cap_csr_base::int__field_last_lfc_count;
}

vector<string> cap_util_split_by_str(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}

pen_csr_base * cap_csr_base::search_csr_by_name(string _name, bool ignore_error) {

    auto pos = _name.find(get_hier_path());
    if(pos == 0) {
        _name.erase(pos, get_hier_path().length());
        if(_name.length()) {
            _name.erase( 0,1); // remove "." - separator
        } else {
            return this;
        }
    }
    auto str_vec = cap_util_split_by_str(_name, '.');
    auto child_vec = get_children(1);

    pen_csr_base * nearest_match = 0;

    if(_name.compare(get_hier_path()) ==0) return this;

    for(auto i: str_vec ) {
        bool match = true;
        match = false;
        for(auto j : child_vec) {
            if( i.compare(j->get_name()) == 0) {
                match = true;
                nearest_match = j;
                child_vec = nearest_match->get_children(1);
                break;
            }
        }
        if(!match && ignore_error == false) {
            PLOG_ERR("search_csr_by_name : no register found for " << _name << " comparing " << i << endl);
            return 0;
        }
    }

    if(!nearest_match && ignore_error == false) {
        PLOG_ERR("search_csr_by_name : default: no register found for " << _name << endl);
    }
    return nearest_match;

}

pen_csr_base * cap_csr_base::search_csr_by_addr(uint64_t _addr) {
  pen_csr_base * match_ptr = 0;
  auto child_vec = get_children(1);
  bool linear_search = false;


  // if all children are only blocks , apply linear search
  // as it is top most level where addresses are not sorted
  if(child_vec.size()) linear_search = true;
  for (auto i: child_vec) {
      if(i->get_csr_type() != CSR_TYPE_BLOCK) {
          linear_search = false; break;
      }
  }

  if(child_vec.size()) {
      if( (linear_search) ||
              ((_addr >= get_offset() ) &&
              (_addr <= get_csr_end_addr()))
        ) {

          // TODO: if linear_search flag is not set, apply binary_search here
          for (auto i: child_vec) {
              if(_addr > i->get_csr_end_addr()) continue;
              if( (_addr <= (i->get_csr_end_addr())) &&
                      (_addr >= (i->get_offset()) ))
              {
                  if( i->get_children(1).size()) {
                      match_ptr = i->search_csr_by_addr(_addr);
                      if(match_ptr != 0) { return match_ptr; }
                  } else {
                      return i;
                  }
              }
          }
      }
  }
  return match_ptr;
}
void cap_csr_base::write_using_addr(uint64_t _addr, u_int32_t data, bool update) {

    pen_csr_base * csr_ptr = search_csr_by_addr(_addr);
    if(!csr_ptr) {
        PLOG_ERR("write_using_addr : Invalid address! 0x" << hex << _addr << dec << endl);
        return;
    }

    uint64_t cur_offset = csr_ptr->get_offset();
    cpp_int write_val = csr_ptr->all();
    vector<unsigned int> write_vector;
    int total_words = (csr_ptr->get_width() +31)/32;
    export_bits(write_val, std::back_inserter(write_vector), 32);
    reverse(write_vector.begin(), write_vector.end());
    write_vector.resize(total_words);
    for(int ii = write_vector.size(); ii < total_words; ii++) {
        write_vector[ii] = 0;
    }

    int word_no = (_addr - cur_offset) >> 2;

    if(word_no >= total_words) {
        PLOG_ERR("write_using_addr : total words are " << total_words << " requested " << word_no << endl);
        csr_ptr->show();
        return;
    }

    write_vector[word_no] = data;
    reverse(write_vector.begin(), write_vector.end());
    import_bits(write_val, write_vector.begin(), write_vector.end());
    csr_ptr->all(write_val);

    if(word_no == (total_words-1)) {
      if (update) {
        csr_ptr->write();
      }
    }
}


u_int32_t cap_csr_base::read_using_addr(uint64_t _addr, bool update) {

    pen_csr_base * csr_ptr = search_csr_by_addr(_addr);
    if(!csr_ptr) {
        PLOG_ERR("read_using_addr : Invalid address! 0x" << hex << _addr << dec << endl);
        return 0xDEADBEEF;
    }

    uint64_t cur_offset = csr_ptr->get_offset();
    int word_no = (_addr - cur_offset) >> 2;
    if(word_no == 0 && update) {
        csr_ptr->read();
    }
    cpp_int val = csr_ptr->all();
    vector<unsigned int> read_vector;
    int total_words = (csr_ptr->get_width() +31)/32;
    export_bits(val, std::back_inserter(read_vector), 32);
    reverse(read_vector.begin(), read_vector.end());
    read_vector.resize(total_words);
    for(int ii = read_vector.size(); ii < total_words; ii++) {
        read_vector[ii] = 0;
    }


    if(word_no >= total_words) {
        PLOG_ERR("read_using_addr : total words are " << total_words << " requested " << word_no << endl);
        return 0xDEADBEEF;
    }

    return read_vector[word_no];

}

#ifdef _COSIM_
bool cap_csr_base::set_field_str_val(string tgt_field_name, string val, int level) {
    cpp_int tmp(val);
    return set_field_val(tgt_field_name, tmp, level);
}


string cap_csr_base::get_field_str_val(string tgt_field_name) {
    cpp_int tmp_val;
    bool ret = get_field_val(tgt_field_name, tmp_val, 1);
    if(ret) {
        stringstream ss;
        ss << hex << tmp_val;
        return string("0x" + ss.str());
    } else {
        return "0xDEADBEEF";
    }
    return "";

}
#endif

bool cap_csr_base::get_field_val(const char * tgt_field_name, cpp_int & val, int level) {

    bool field_found = false;        
    /*
    for(auto orig_field_name : get_fields()) {
        string field_name = orig_field_name;
        if(field_name.compare(tgt_field_name)==0) {
            return get_field_val(orig_field_name, val);
        }

    }
    */


    return field_found;

}

bool cap_csr_base::get_field_val(string field_name, cpp_int & val, int level) {
    return get_field_val(field_name.c_str(), val, level);
}
bool cap_csr_base::set_field_val(string field_name, cpp_int & val, int level) { 
    return set_field_val(field_name.c_str(), val, level);
}
bool cap_csr_base::set_field_val(const char * tgt_field_name, cpp_int & val, int level) {
    return false;
}




bool cap_csr_base::update_shadow = true;
