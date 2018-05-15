#include <stdio.h>
#include <stdint.h>

#define FILELOCK "~/test_mom_lock"
#define MAXREGIONS 0xFFFF
#define MAXENTRIES 0xFFFF

typedef struct mom_region_address_s {
    char name[63];
    bool valid;
    uint64_t address;

    /* Total size of the region */
    uint64_t size; 

    /* Entry specific detail */
    uint32_t entry_length;
    uint32_t entry_count;
} mom_region_address_t;

typedef struct mom_metadata_s {
    unsigned int start_address;
    unsigned int region_count;
    unsigned int region_alloced;
    unsigned int end_address;
    
    /* Must be last */
    mom_region_address_t regions[0];    
} mom_metadata_t;

typedef enum {
    MOM_SUCCESS = 0,
    MOM_FAILURE
} mom_ret_t;

/* Tag entry manipulation */
mom_ret_t mom_make_entry(char *name, uint64_t address, uint32_t size);
mom_ret_t mom_search_region(char *name, mom_region_address_t **, uint64_t *index);

/* Public APIs */
mom_ret_t mom_init(uint32_t region_count);
uint64_t alloc_region(char *name, uint32_t size, uint32_t entry_count = MAXENTRIES);
void free_region(char *name);
mom_ret_t get_memory_map(mom_region_address_t **map);
mom_ret_t write_memory_map(mom_region_address_t *map);
void print_mom_memory_map();
mom_ret_t mom_write_entry_to_region(char *region_name, int entry_index, uint8_t* data);
mom_ret_t mom_read_entry_from_region(char *region_name, int entry_index, uint8_t **data);
mom_ret_t
mom_move_region(mom_region_address_t *from_region,
                mom_region_address_t *to_region,
                int (*transform_fn)(uint8_t*, uint8_t**));
