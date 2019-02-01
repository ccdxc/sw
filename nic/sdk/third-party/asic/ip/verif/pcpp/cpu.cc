#include "cpu.h"
#include "sknobs.h"

atomic<cpu*> cpu::_cpu_if { nullptr };
std::mutex cpu::m_;

cpu *cpu::access(void) {

  if(_cpu_if == nullptr) {
#ifdef _COSIM_      
     lock_guard<mutex> lock(m_);
#endif     
     if(_cpu_if == nullptr) {
        _cpu_if = new cpu;
     }
  }
  return _cpu_if;
}

cpu::cpu() {
    access_type = front_door_e;
    force_global_flags = 0;
    //Read sknobs to set plog Level
    if (sknobs_exists((char *)"cpu_access")) {
        string access_type_str = sknobs_get_string((char *)"cpu_access", (char *)"");
        if(!access_type_str.compare("air_slave")) {
            access_type = air_slave_e;
        } else if(!access_type_str.compare("back_door")) {
            access_type = back_door_e;
        } else {
            access_type = front_door_e;
        }
    } else {
        access_type = front_door_e;
    }

}

void cpu::set_access_type(cpu_access_type_e _access_type) {
    PLOG_MSG("setting access_type to " << _access_type << endl);
    access_type = _access_type;
}
void cpu::set_global_flags(unsigned _val) { force_global_flags = _val; }
cpu_access_type_e cpu::get_access_type() { return access_type; }

uint32_t cpu::read(uint32_t chip, uint64_t addr, bool no_zero_time, uint32_t flags) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    
    uint32_t ret_data;
    cpu_access_type_e l_access_type = access_type;
    if( (l_access_type == air_slave_e) || ( (l_access_type == back_door_e) && no_zero_time)) {
        if(cpu_csr_node_read(chip, addr, ret_data, flags | force_global_flags)) return ret_data;
        else l_access_type = front_door_e;
    }
   map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());
   if (iter == cpu_if_map.end() || iter->second == NULL) {
     PLOG_MSG ( "ERROR::cur_if_ptr not set!" << endl);
     return 0xdeadbeef;
   } else {
       return iter->second->read(chip, addr, no_zero_time ? front_door_e : l_access_type, flags | force_global_flags);
   }
}
void cpu::remove_if(string cpu_if_name) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    
      map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(cpu_if_name);
      if (iter == cpu_if_map.end() || iter->second == NULL) {
          PLOG_ERR ( "remove_if:" << cpu_if_name << " not there!" << endl);
      } else {
          // TODO: do we need to call delete?
          // assumption: some one will delete iter outside
          cpu_if_map.erase(iter);
      }
  }

void cpu::write(uint32_t chip, uint64_t addr, uint32_t data, bool no_zero_time, uint32_t flags) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    

    cpu_access_type_e l_access_type = access_type;
    if( (l_access_type == air_slave_e) || ( (l_access_type == back_door_e) && no_zero_time)) {
        if(cpu_csr_node_write(chip, addr, data, flags | force_global_flags)) return;
        else l_access_type = front_door_e;
    }
   map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());

   if (iter == cpu_if_map.end() || iter->second == NULL) {
       PLOG_MSG ( "ERROR::cur_if_ptr not set!" << endl);
   } else {
       iter->second->write(chip, addr, data, no_zero_time ? front_door_e : l_access_type, flags | force_global_flags);
   }
}

void cpu::block_write(uint32_t chip, uint64_t addr, int size, const vector<uint32_t>& data, bool no_zero_time, uint32_t flags) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    

    cpu_access_type_e l_access_type = access_type;
    if( (l_access_type == air_slave_e) || ( (l_access_type == back_door_e) && no_zero_time)) {
        if(cpu_csr_node_block_write(chip, addr, size, data, flags | force_global_flags)) return;
        else l_access_type = front_door_e;
    }
   map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());

   if (iter == cpu_if_map.end() || iter->second == NULL) {
     PLOG_MSG ( "ERROR::cur_if_ptr not set!" << endl);
     return;
   } else {
       iter->second->block_write(chip, addr, size, data, no_zero_time ? front_door_e : l_access_type, flags | force_global_flags);
   }
}

vector<uint32_t> cpu::block_read(uint32_t chip, uint64_t addr, int size, bool no_zero_time, uint32_t flags) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    
    vector<uint32_t> read_vector;

    cpu_access_type_e l_access_type = access_type;
    if( (l_access_type == air_slave_e) || ( (l_access_type == back_door_e) && no_zero_time)) {
        if(cpu_csr_node_block_read(chip, addr, size, read_vector, flags | force_global_flags)) return read_vector;
        else l_access_type = front_door_e;
    }

    map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());

    if (iter == cpu_if_map.end() || iter->second == NULL) {
     PLOG_MSG ("ERROR::cur_if_ptr not set!" << endl);
     for(int i=0; i < size; i++) read_vector.push_back(0xdeadbeef);
    } else {
        read_vector = iter->second->block_read(chip, addr, size, no_zero_time ? front_door_e : l_access_type, flags | force_global_flags);
    }
    return read_vector;
}


bool cpu::burst_write(uint32_t chip, uint64_t addr, unsigned int len, const unsigned char * data, bool no_zero_time, uint32_t flags, bool reverse_byte_order) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    

    cpu_access_type_e l_access_type = access_type;
    if( (l_access_type == air_slave_e) || ( (l_access_type == back_door_e) && no_zero_time)) {
      if(cpu_csr_node_burst_write(chip, addr, len, data, flags | force_global_flags, reverse_byte_order)) return true;
      else l_access_type = front_door_e;
    }
   map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());

   if (iter == cpu_if_map.end() || iter->second == NULL) {
     PLOG_MSG ("ERROR::cur_if_ptr not set!" << endl);
     return 1;
   }
   return iter->second->burst_write(chip, addr, len, data, no_zero_time ? front_door_e : l_access_type, flags | force_global_flags, reverse_byte_order);

}

bool cpu::burst_read(uint32_t chip, uint64_t addr, unsigned int len, unsigned char * data, bool no_zero_time, uint32_t flags, bool reverse_byte_order) {
#ifdef _COSIM_      
    lock_guard<mutex> lock(m_);
#endif    

    cpu_access_type_e l_access_type = access_type;
    if( (l_access_type == air_slave_e) || ( (l_access_type == back_door_e) && no_zero_time)) {
      if(cpu_csr_node_burst_read(chip, addr, len, data, flags | force_global_flags, reverse_byte_order)) return true;
      else l_access_type = front_door_e;
    }

   map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());

   if (iter == cpu_if_map.end() || iter->second == NULL) {
     PLOG_MSG ("ERROR::cur_if_ptr not set!" << endl);
     for(unsigned i=0; i < len; i++) { data[i] = 0x55; }
     return 1;
   }
   return iter->second->burst_read(chip, addr, len, data, no_zero_time ? front_door_e : l_access_type, flags | force_global_flags, reverse_byte_order);

}

/*
int cpu::call_uvm_hdl_deposit(uint32_t chip, char * path, vector<uint32_t> & value) {
   map<string, cpu_bus_base *>::iterator iter = cpu_if_map.find(get_cur_if_name());

   if (iter->second == NULL) {
     PLOG_MSG << "ERROR::cur_if_ptr not set!" << endl;
     return 0;
   }
   return iter->second->call_uvm_hdl_deposit(chip, path, value);
}
*/

#ifdef _CSV_INCLUDED_
#ifdef PEN_CSR_AIRSLAVE_ENABLE
extern "C" void c2sv_cpu_csr_node_register(char * node_path, uint64_t start_byte_addr, uint64_t csr_size, uint32_t chip_id) {
    bool node_exists = cpu::access()->check_csr_node_info_exists(start_byte_addr, csr_size, chip_id);
    if(!node_exists) {
        std::shared_ptr<cpu_csr_node_info_base> info (new cpu_csr_node_info(node_path, start_byte_addr, csr_size, chip_id));
        PLOG_MSG("Adding cpu_csr_node for " << node_path << " start_addr : 0x" << hex << start_byte_addr << " size : 0x" << csr_size << " id : " << dec << chip_id << endl);
        cpu::access()->add_cpu_csr_node_info(info);
    } else {
        PLOG_MSG("ignored path already found :" << node_path << hex << " for addr : 0x" << start_byte_addr << " size 0x" << csr_size << endl<< dec);
    }
}
#endif
#endif



void cpu::add_cpu_csr_node_info(std::shared_ptr<cpu_csr_node_info_base> node_ptr) {
    cpu_csr_info_array.push_back(node_ptr);
}

bool cpu::check_csr_node_info_exists(uint64_t addr, uint64_t csr_size, uint32_t chip_id) {
    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip_id) {
            if( (addr >= i->start_byte_addr) && ((addr + csr_size) <= (i->start_byte_addr + i->csr_size) ) ) {
                return 1;
            }
        }
    }
    return 0;
}

bool cpu::cpu_csr_node_block_read(uint32_t chip, uint64_t addr, uint32_t size, vector<uint32_t> & data, uint32_t flags) {
    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip) {
            if( (addr >= i->start_byte_addr) && (addr <= (i->start_byte_addr + i->csr_size) ) ) {
                data = i->block_read(addr, size, flags | force_global_flags);
                return 1;
            }
        }
    }
    return 0;
}



bool cpu::cpu_csr_node_block_write(uint32_t chip, uint64_t addr, uint32_t size, const vector<uint32_t>& data, uint32_t flags) {
    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip) {
            if( (addr >= i->start_byte_addr) && (addr <= (i->start_byte_addr + i->csr_size) ) ) {
                i->block_write(addr, size, data, flags | force_global_flags);
                return 1;
            }
        }
    }
    return 0;
}


bool cpu::cpu_csr_node_burst_read(uint32_t chip, uint64_t addr, unsigned int len, unsigned char * data, uint32_t flags, bool reverse_byte_order) {
    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip) {
            if( (addr >= i->start_byte_addr) && (addr <= (i->start_byte_addr + i->csr_size) ) ) {
              if (len % 4) {
                //multiple of 4 bytes access to csr
                PLOG_ERR("cpu::cpu_csr_node_burst_read: len is not multiple of 4 bytes: len=" << len << endl);
              } else {
                vector<uint32_t>  data_vec;
                data_vec = i->block_read(addr, len/4, flags | force_global_flags);
                for(unsigned j=0; j < len; j++) {
                  if (reverse_byte_order) {
                    data[len-j-1] = (data_vec[j/4] >> ((3-j%4)*8));
                  } else {
                    data[j]       = (data_vec[j/4] >> ((3-j%4)*8));
                  }
                }
                return true;
              }
            }
        }
    }
    return false;
}


bool cpu::cpu_csr_node_burst_write(uint32_t chip, uint64_t addr, unsigned int len, const unsigned char * data, uint32_t flags, bool reverse_byte_order) {

    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip) {
            if( (addr >= i->start_byte_addr) && (addr <= (i->start_byte_addr + i->csr_size) ) ) {
              if (len % 4) {
                //multiple of 4 bytes access to csr
                PLOG_ERR("cpu::cpu_csr_node_burst_read: len is not multiple of 4 bytes: len=" << len << endl);
              } else {
                vector<uint32_t>  data_vec;
                data_vec.resize(len/4);
                uint32_t temp;
                for(unsigned j=0; j < len; j++) {
                  if (j%4) temp=0;
                  temp = (temp << 8) | data[j];
                  if (j%3) data_vec[j/4] = temp;
                }
                i->block_write(addr, len/4, data_vec, flags | force_global_flags);
                return true;
            }
          }
        }
    }
    return false;
}


bool cpu::cpu_csr_node_write(uint32_t chip, uint64_t addr, uint32_t data, uint32_t flags) {
    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip) {
            if( (addr >= i->start_byte_addr) && (addr <= (i->start_byte_addr + i->csr_size) ) ) {
                i->write(addr, data, flags | force_global_flags);
                return 1;
            }
        }
    }
    return 0;
}

bool cpu::cpu_csr_node_read(uint32_t chip, uint64_t addr, uint32_t & data, uint32_t flags) {
    for(auto i : cpu_csr_info_array) {
        if(i->chip_id == chip) {
            if( (addr >= i->start_byte_addr) && (addr <= (i->start_byte_addr + i->csr_size) ) ) {
                data = i->read(addr, flags | force_global_flags);
                return 1;
            }
        }
    }
    return 0;
}
