#include <vector>

#ifndef MOCKPAL_H
#define MOCKPAL_H
using namespace std;
typedef struct region_s { 
    char region_name[64];
    uint64_t start_address;
    uint64_t entry_size;
    uint64_t size;
} region_t;

typedef enum pal_ret_e {
    PAL_SUCCESS,
    PAL_FAILURE
} pal_ret_t;

uint64_t pal_moveable_sbrk(int32_t size);

uint64_t setup_memory();

pal_ret_t pal_mem_move(uint64_t from, uint64_t to, uint32_t size);

pal_ret_t pal_mem_write(uint64_t address, uint8_t *buffer, uint32_t size);

pal_ret_t pal_mem_read(uint64_t address, uint8_t *buffer, uint32_t size);

pal_ret_t pal_mem_write_region(uint64_t region_address, const char *region_name, uint64_t start_address, uint64_t entry_size, uint64_t size); 

pal_ret_t pal_mem_read_region(uint64_t region_address, region_t *region); 

pal_ret_t bottom_up_move_entry(uint64_t from, uint64_t to, uint32_t entry_size, uint32_t entry_count); 

uint64_t pal_alloc_memory(const char *name, uint32_t size); 

uint64_t pal_get_region(const char *region_name);

vector<region_t> pal_get_map(); 
#endif
