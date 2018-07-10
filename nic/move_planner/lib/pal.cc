#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "pal.h"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

#define MAXMEMORY 65536 

using namespace std;
uint64_t base_address = 0;
uint64_t top = 0;
const char mmap_file[] = "./mmap.bin";

uint64_t pal_moveable_sbrk(int32_t size) {
    top += size;
    UPG_LOG_INFO("TOP RETURN {}", top - base_address);
    return top;
}

uint64_t pal_get_moveable_base() {
    return base_address;
}

pal_ret_t pal_mem_move(uint64_t from, uint64_t to, uint32_t size) {
    if(from != to) {
        UPG_LOG_INFO("pal_mem_move FROM {} TO {} SIZE {}", from, to, size);
        memmove((void*) to, (void*)from, (size_t) size);
    }

    return PAL_SUCCESS;
}

pal_ret_t pal_mem_write(uint64_t address, uint8_t *buffer, uint32_t size) {
    //cout << "\nPAL MEM WRITE :" << std::hex << address - base_address << " BUFFER : " << buffer << " SIZE : 0x" << std::hex << size;
    memcpy((char*)address, (char*)buffer, size);
    return PAL_SUCCESS;
}

pal_ret_t pal_mem_read(uint64_t address, uint8_t *buffer, uint32_t size) {
    //cout << "\nPAL MEM READ :" << std::hex << address - base_address << " BUFFER : " << buffer << " SIZE : 0x" << std::hex << size;
    memcpy((char*)buffer, (char*)address, size);
    return PAL_SUCCESS;
}

uint64_t setup_memory() {
    int fd = open(mmap_file, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);    
    ftruncate(fd, MAXMEMORY);
    base_address = (uint64_t) mmap(0, MAXMEMORY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    UPG_LOG_INFO("BASE ADDRESS : {}", base_address);
    //pal_mem_write(base_address, (uint8_t*)mmap_file, sizeof(mmap_file)); 
    //memmove((char*)base_address + 512, (char*)base_address, strlen(mmap_file));
    top = base_address;
    return base_address;
}

pal_ret_t pal_mem_write_region(uint64_t region_address, const char *region_name, uint64_t start_address, uint64_t entry_size, uint64_t size) {
    region_t region;
    memset(&region, 0, sizeof(region));
    strcpy(region.region_name, region_name);
    region.start_address = start_address;
    region.entry_size = entry_size;
    region.size = size;

    UPG_LOG_INFO("Inside pal mem_write_region REGION ADDRESS : {} BASE : {}",
                  region_address,
                  base_address);

    pal_mem_write(region_address, (uint8_t*) &region, sizeof(region_t));
    return PAL_SUCCESS;
}

pal_ret_t pal_mem_read_region(uint64_t region_address, region_t *region) {
    pal_mem_read(region_address, (uint8_t*)region, sizeof(region_t));
    return PAL_SUCCESS;
}

pal_ret_t bottom_up_move_entry(uint64_t from, uint64_t to, uint32_t entry_size, uint32_t entry_count) {
    int i = 0;
    for(i = entry_count - 1; i >= 0; i--) {
        pal_mem_move(from + i * entry_size,
                     to + i * entry_size,
                     entry_size);
    }

    return PAL_SUCCESS;
}

uint64_t pal_alloc_memory(const char *name, uint32_t size) {
    uint64_t return_address = pal_moveable_sbrk(size+ sizeof(region_t));
    UPG_LOG_INFO("RET : {}", return_address);

    pal_mem_write_region(return_address - (size) - sizeof(region_t),
                         name,
                         return_address - size,
                         32,
			 size);

    return return_address;
}

vector<region_t> pal_get_map() {
    region_t reg;
    vector<region_t> mem_map;
    uint64_t address = base_address; 
    uint64_t prev_size = 0;

    while(address < top) {
        pal_mem_read_region(address, &reg);
        mem_map.push_back(reg);
        prev_size = reg.size;
        address = address + sizeof(region_t) + prev_size;
        UPG_LOG_INFO("get_map NAME : {} SIZE = {}", reg.region_name, reg.size);
    }
    
    return mem_map; 
}

uint64_t pal_get_region(const char *region_name) {
    region_t reg;
    uint64_t address = base_address;
    uint64_t prev_size = 0;

    while(address < top) {
        pal_mem_read_region(address, &reg);

        UPG_LOG_INFO("REGION NAME : {}", reg.region_name);
        if(strcmp(region_name, reg.region_name) == 0) {
		return reg.start_address;
        }

        prev_size = reg.size;
        address = address + sizeof(region_t) + prev_size;
    }

    return 0;

}
