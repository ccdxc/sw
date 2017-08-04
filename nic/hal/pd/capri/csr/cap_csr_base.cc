#include "cap_csr_base.h"
#include "sknobs.h"
#include "pknobs_reader.h"
#ifdef _CSV_INCLUDED_

void SLEEP(int N) {
    svScope new_scope;
    svScope old_scope = svGetScope();
    if (sknobs_exists((char *)"stub_build")) {
       new_scope = svGetScopeFromName("\\LIBRTL.puvm_cpp_reg_shim_pkg ");
    } else {
       new_scope = svGetScopeFromName("puvm_cpp_reg_shim_pkg");
    }
    svSetScope(new_scope);
    wait_time(N * 1000);
    svSetScope(old_scope);
}

#endif

map< string, vector<string> > cap_csr_base::field_order_vector;
map< string, cap_csr_base::set_function_map_t > cap_csr_base::int_func_map__set;
map< string, cap_csr_base::get_function_map_t > cap_csr_base::int_func_map__get;
map< string, cap_csr_base::set_array_function_map_t > cap_csr_base::int_func_map__set_array;
map< string, cap_csr_base::get_array_function_map_t > cap_csr_base::int_func_map__get_array;

cap_csr_base::cap_csr_base(string _name, cap_csr_base * _parent) {
    base__name = _name;
    type__name = _name;
    base__parent = _parent;
    base__int__offset = 0;
    base__int__csr_id = "def";
    int_var__reset_value = 0;
    int__csr_end_addr = 0;
    int__field_init_done = false;
}


cap_csr_base::~cap_csr_base() { }

string cap_csr_base::get_name() const {
    return base__name;
}

string cap_csr_base::get_type_name() const {
    return type__name;
}

static int get_idx_from_field_name(string field_name) {

    size_t first_idx_pos = field_name.find_last_of('[');
    size_t last_idx_pos = field_name.find_last_of(']');
    if( (first_idx_pos != string::npos) || (last_idx_pos != string::npos)) {
        PLOG_ERR("Unexpected: field_name doesn't have [foo] "<< field_name << endl);
    }

    string idx_str = field_name.substr ( first_idx_pos , last_idx_pos - first_idx_pos);
    int idx = stoi(idx_str);
}


cap_csr_base * cap_csr_base::get_parent() const {
    return base__parent;
}
void cap_csr_base::set_name(string _name) {
    base__name = _name;
}
void cap_csr_base::set_type_name(string _name) {
    type__name = _name;
}
void cap_csr_base::set_parent(cap_csr_base * _base) {
    if(base__parent && (_base != base__parent)) {
        PLOG_WARN("parent is already non-null, unexpected behavior" << endl);
    }

    cap_csr_base::int_func_map__get[get_type_name()]["all"] = &cap_csr_base::all;
    cap_csr_base::int_func_map__set[get_type_name()]["all"] = &cap_csr_base::all;

    if( (_base != 0) && (base__parent == 0)) {
        base__parent = _base;
        register_child(this,1);
    }
    base__parent = _base;
}

void cap_csr_base::register_child(cap_csr_base * _child, bool add_to_parent) {
    if(_child ==0) return;
    if(add_to_parent) {
        get_parent()->register_child(_child, 0);
    } else {
        int__children.push_back(_child);
    }
}

void cap_csr_base::update_block_boundaries(cap_csr_base * _child) {
    if(_child->get_csr_end_addr() >= int__csr_end_addr) {
        int__csr_end_addr = _child->get_csr_end_addr();
    }
}


string cap_csr_base::get_hier_path() const {
    string ret_val = "";
    if(get_parent() != 0) {
        ret_val = get_parent()->get_hier_path() + "." + get_name();
    } else {
        ret_val = get_name();
    }
    return ret_val;
}

int cap_csr_base::get_width() const {
    return 0;
}
void cap_csr_base::init() {
    return;
}
void cap_csr_base::show() {
    bool found;
    get_function_map_t l_get_func_map;
    if(int_func_map__get.find(get_type_name()) != int_func_map__get.end()) {
        l_get_func_map = int_func_map__get[get_type_name()];
    }

    get_array_function_map_t l_get_array_func_map;
    if(int_func_map__get_array.find(get_type_name()) != int_func_map__get_array.end()) {
        l_get_array_func_map = int_func_map__get_array[get_type_name()];
    }


    vector<string> l_field_names;
    if(field_order_vector.find(get_type_name()) != field_order_vector.end()) {
        l_field_names = field_order_vector[get_type_name()];
    }


    for(auto per_field : l_field_names) {
        found = false;
        for(auto get_it : l_get_func_map) {
            if(0 == per_field.compare(get_it.first)) {
                get_function_type_t get_func = get_it.second;
                found = true;
                PLOG_MSG(get_it.first << ": 0x" << (this->*get_func)() << endl);
                break;

            }
        }

        if(!found) {
            for(auto get_it : l_get_array_func_map) {
                if(0 == per_field.compare(get_it.first)) {
                    get_array_function_type_t get_func = get_it.second;
                    found = true;
                    int idx = get_idx_from_field_name(get_it.first);
                    PLOG_MSG(get_it.first << ": 0x" << (this->*get_func)(idx) << endl);
                    break;

                }
            }


        }

    }
}
void cap_csr_base::all(const cpp_int & l__val) {
    return;
}
cpp_int cap_csr_base::all() const {
    return cpp_int('0');
}

int cap_csr_base::get_byte_size() const {
    int my_width = get_width();
    if(my_width < 32) my_width = 32;
    return pow(2,ceil(log2(my_width)))/8;
}

void cap_csr_base::set_offset(uint64_t _offset) {
    base__int__offset = _offset;
}

uint64_t cap_csr_base::get_offset() const {
    uint64_t ret_val = 0;
    if(get_parent() != 0) {
        ret_val = get_parent()->get_offset() + base__int__offset;
    } else {
        ret_val = base__int__offset;
    }
    return ret_val;
}

int cap_csr_base::get_chip_id() const {
    int ret_val = 0;
    if(get_parent() != 0) {
        ret_val = get_parent()->get_chip_id();
    } else {
        PLOG_ERR("cap_csr_base: parent is null and its not block or system. Can't get chip_id" << endl);
            return 0;
    }
    return ret_val;
}

void cap_csr_base::write() {
    PLOG_ERR("cap_csr_base:: write not to be used" << endl);
}
void cap_csr_base::read() {
    PLOG_ERR("cap_csr_base:: read not to be used" << endl);
}
void cap_csr_base::set_attributes(cap_csr_base * _parent, string _name, uint64_t _offset)
{
    if(get_csr_type() == CSR_TYPE_NONE) { set_csr_type(CSR_TYPE_REGISTER); }
    set_name(_name);
    set_offset(_offset);
    int__csr_end_addr = get_offset() + ((get_width() +7)/8);
    set_parent(_parent);
    init();
    set_field_init_done(true);
    if(_parent) { _parent->update_block_boundaries(this); }
}

void cap_csr_base::set_reset_val(cpp_int _val) {
    int_var__reset_value = _val;
}
cpp_int cap_csr_base::get_reset_val() const {
    return int_var__reset_value;
}

cap_csr_base::csr_type_t cap_csr_base::get_csr_type() const {
    return base__int__csr_type;
}
void cap_csr_base::set_csr_type(csr_type_t _type) {
    base__int__csr_type = _type;
}

bool cap_csr_base::search_field_in_db(string _name) {
    bool found = false;
    for(auto it : cap_csr_base::field_order_vector) {
        if(it.first.compare(get_type_name()) == 0) {
            for(auto  it_field : it.second) {
                if(0 == it_field.compare(_name)) return true;
            }
        }
    }

    return false;
}

void cap_csr_base::register_set_func(string _name, cap_csr_base::set_function_type_t func_ptr) {
    PLOG_MSG("registering " << get_hier_path() << "type : " << get_type_name() << " func: " << _name << endl);
    if(!search_field_in_db(_name)) {
        cap_csr_base::int_func_map__set[get_type_name()][_name] = func_ptr;
        cap_csr_base::field_order_vector[get_type_name()].push_back(_name);
    }
}
void cap_csr_base::register_get_func(string _name, cap_csr_base::get_function_type_t func_ptr) {
    if(!search_field_in_db(_name)) {
        cap_csr_base::int_func_map__get[get_type_name()][_name] = func_ptr;
        cap_csr_base::field_order_vector[get_type_name()].push_back(_name);
    }
}

void cap_csr_base::register_set_array_func(string _name, cap_csr_base::set_array_function_type_t func_ptr) {
    if(!search_field_in_db(_name)) {
        cap_csr_base::int_func_map__set_array[get_type_name()][_name] = func_ptr;
        cap_csr_base::field_order_vector[get_type_name()].push_back(_name);
    }
}
void cap_csr_base::register_get_array_func(string _name, cap_csr_base::get_array_function_type_t func_ptr) {
    if(!search_field_in_db(_name)) {
        cap_csr_base::int_func_map__get_array[get_type_name()][_name] = func_ptr;
        cap_csr_base::field_order_vector[get_type_name()].push_back(_name);
    }
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
/*
void cap_block_base::register_child(cap_csr_base * _child) {
    if( (_child != 0) && (_child == get_parent()))  {
        get_parent()->register_child(_child);
    } else {
        block__children.push_back(_child);
    }
}
*/
vector<cap_csr_base *> cap_csr_base::get_children(int level) const {
    vector<cap_csr_base *> ret_val;
    if( (level == -1) || (level > 0)) {
        for (auto i: int__children) {
            ret_val.push_back(i);
            vector<cap_csr_base *> tmp = i->get_children(level >0 ? level -1 : level);
            ret_val.insert(ret_val.end(),  tmp.begin(), tmp.end());
        }
    }
    return ret_val;
}



//vector<cap_csr_base *> cap_block_base::get_children() {
//    for
//    return block__children;
//}
//

void cap_block_base::set_csr_inst_path(string _id, string _path) {
    block__csr_inst_path_map[_id] = _path;
}

void cap_block_base::load_from_cfg(bool do_write, bool load_fields) {
  for (auto i: get_children()) {
      i->load_from_cfg(do_write, load_fields);
  }
}


string cap_block_base::get_csr_inst_path(string _id) {
    if(block__csr_inst_path_map.find(_id) != block__csr_inst_path_map.end())
        return block__csr_inst_path_map[_id];
    else return "";
}


string cap_csr_base::get_csr_inst_path(string _id) {
    if(get_parent() != 0) {
        return get_parent()->get_csr_inst_path(_id);
    } else {
        return "";
    }
}

string cap_csr_base::get_csr_id() const {
    return base__int__csr_id;
}
void cap_csr_base::set_csr_id(string _id) {
    base__int__csr_id = _id;
}


vector<unsigned int> cap_register_base::int__get_write_vec() {
    cpp_int write_val;
    uint64_t width;
    write_val = all();
    width = get_width();
    vector<unsigned int> write_vector;
    export_bits(write_val, std::back_inserter(write_vector), 32);
    reverse(write_vector.begin(), write_vector.end());
    int words = (width+31)/32;
    PU_ASSERT(write_vector.size() <= words);
    write_vector.resize(words);
    for(int ii = write_vector.size(); ii < words; ii++) {
        write_vector[ii] = 0;
    }
    return write_vector;
}


void cap_register_base::int__set_read_vec(vector<unsigned int> read_vector) {
    cpp_int read_val;
    uint64_t offset, width;
    offset = get_offset();
    width = get_width();
    reverse(read_vector.begin(), read_vector.end());
    import_bits(read_val, read_vector.begin(), read_vector.end());
    PLOG_MSG("read: name: " << get_hier_path() << " addr : 0x" << hex << offset << " data: 0x" << read_val << dec <<  endl);
    PU_ASSERT(read_vector.size() == (width + 31)/32);
    if (cap_csr_base::update_shadow)
        all(read_val);

}
void cap_register_base::write() {

    vector<unsigned int> write_vector = int__get_write_vec();
    uint64_t offset, width;
    offset = get_offset();
    PLOG_MSG("write: name: " << get_hier_path() << " addr: 0x" << hex << offset << "data: 0x" << all() << dec << endl);
    for(int ii = 0; ii < write_vector.size(); ii++) {
        cpu::access()->write(get_chip_id(), offset + (ii*4), write_vector[ ii], int__access_no_zero_time);
    }
}
void cap_register_base::read() {
    uint64_t offset, width;
    int chip_id;
    offset = get_offset();
    width = get_width();
    chip_id = get_chip_id();
    vector<unsigned int> read_vector;
    int words = (width+31)/32;
    read_vector.resize(words);
    for(int ii = 0; ii < words; ii++) {
        read_vector[ii] = cpu::access()->read(chip_id, offset + (ii*4), int__access_no_zero_time);
    }

    int__set_read_vec(read_vector);
}



void cap_register_base::block_write() {
    vector<unsigned int> write_vector = int__get_write_vec();
    uint64_t offset, width;
    offset = get_offset();
    PLOG_MSG("block write: name: " << get_hier_path() << " addr: 0x" << hex << offset << "data: 0x" << all() << dec << endl);
    cpu::access()->block_write(get_chip_id(), offset, write_vector.size(), write_vector, int__access_no_zero_time);
}


void cap_register_base::block_read() {
    uint64_t offset, width;
    offset = get_offset();
    width = get_width();
    vector<unsigned int> read_vector;
    int words = (width+31)/32;
    read_vector.resize(words);
    read_vector = cpu::access()->block_read(get_chip_id(), offset, read_vector.size(), int__access_no_zero_time);
    int__set_read_vec(read_vector);
}



cap_register_base::cap_register_base(string _name, cap_csr_base * _parent):
    cap_csr_base(_name, _parent) {
        set_csr_type(CSR_TYPE_REGISTER);
        int__access_no_zero_time = false;
}

static cpp_int get_val (string & value_s)
{
    cpp_int val;

    if (isdigit(value_s[0])) {
        val = cpp_int(value_s);
    } else {
        val = PKnobsReader::evalKnob(value_s);
    }
    return val;
}


cap_register_base::~cap_register_base() { }

void cap_csr_base::load_from_cfg(bool do_write, bool load_fields) {
  string name = get_hier_path();
  std::replace( name.begin(), name.end(), '.', '/');
  std::replace( name.begin(), name.end(), '[', '/');
  std::replace( name.begin(), name.end(), ']', '/');
  size_t index = 0;
  bool write_needed = false;
  while(true) {
      index = name.find("//", index);
      if(index == std::string::npos) break;
      name.replace(index, 2, "/");
      index += 1;
  }
  if (name[name.length() - 1] == '/') {
    name.pop_back();
  }
  string value_s = "";
  if (sknobs_exists((char*) name.c_str()) != 0){
    value_s = sknobs_get_string((char*) name.c_str(),(char*)"");
    cpp_int val = get_val(value_s);
    all(val);
    if (all() != get_reset_val() && do_write)  {
      //PLOG_MSG("load_from_cfg: writing reg " << name.c_str() << endl);
      write_needed = true;
    }
  }

  if(load_fields) {
      for(auto it : cap_csr_base::int_func_map__set) {
          if(!it.first.compare(get_type_name())) {
              for(auto jj : it.second) {
                  string field_name = jj.first;
                  std::replace( field_name.begin(), field_name.end(), '.', '/');
                  std::replace( field_name.begin(), field_name.end(), '[', '/');
                  std::replace( field_name.begin(), field_name.end(), ']', '/');
                  field_name = name + "/" + field_name;
                  if (sknobs_exists((char*) field_name.c_str()) != 0) {
                      value_s = sknobs_get_string((char*) field_name.c_str(),(char*)"");
                      cpp_int val = get_val(value_s);
                      set_function_type_t set_func = jj.second;
                      (this->*set_func)(val);
                      if(do_write) write_needed = true;
                  }
              }
              break;
          }
      }

      for(auto it : cap_csr_base::int_func_map__set_array) {
          if(!it.first.compare(get_type_name())) {

              for(auto jj : it.second) {
                  string field_name = jj.first;
                  string orig_field_name = field_name;

                  std::replace( field_name.begin(), field_name.end(), '.', '/');
                  std::replace( field_name.begin(), field_name.end(), '[', '/');
                  std::replace( field_name.begin(), field_name.end(), ']', '/');
                  field_name = name + "/" + field_name;
                  if (sknobs_exists((char*) field_name.c_str()) != 0) {
                      value_s = sknobs_get_string((char*) field_name.c_str(),(char*)"");
                      cpp_int val = get_val(value_s);
                      int idx = get_idx_from_field_name(orig_field_name);

                      set_array_function_type_t set_func = jj.second;
                      (this->*set_func)(val, idx);

                      if(do_write) write_needed = true;
                  }
              }
          }
      }
  }

  if(write_needed) {
      write();
  }


}

vector<string> split_by_str(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}

cap_csr_base * cap_csr_base::search_csr_by_name(string _name) {

    auto str_vec = split_by_str(_name, '.');
    auto child_vec = get_children(1);

    cap_csr_base * nearest_match = 0;

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
        if(!match) {
            PLOG_ERR("search_csr_by_name : no register found for " << _name << endl);
            return 0;
        }
    }

    if(!nearest_match) {
        PLOG_ERR("search_csr_by_name : default: no register found for " << _name << endl);
    }
    return nearest_match;

}

cap_csr_base * cap_csr_base::search_csr_by_addr(uint64_t _addr) {
  cap_csr_base * match_ptr = 0;
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
              if( (_addr < (i->get_csr_end_addr())) &&
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
void cap_csr_base::write_using_addr(uint64_t _addr, uint32_t data) {

    cap_csr_base * csr_ptr = search_csr_by_addr(_addr);
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
        csr_ptr->write();
    }
}


uint32_t cap_csr_base::read_using_addr(uint64_t _addr) {

    cap_csr_base * csr_ptr = search_csr_by_addr(_addr);
    if(!csr_ptr) {
        PLOG_ERR("read_using_addr : Invalid address! 0x" << hex << _addr << dec << endl);
        return 0xDEADBEEF;
    }

    uint64_t cur_offset = csr_ptr->get_offset();
    int word_no = (_addr - cur_offset) >> 2;
    if(word_no == 0) {
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

bool cap_csr_base::update_shadow = true;
