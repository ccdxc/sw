#include <stdio.h>
#include <stdint.h>
#include "malloc_stub.h"
#include "mom.h"
#include <string.h>

static mom_ret_t 
get_moveable_metadata(mom_metadata_t** meta) {

    if(pal_mem_read(get_moveable_memory_start(),
                     (uint8_t*) *meta,
                     sizeof(mom_metadata_t))) {
        return MOM_SUCCESS;
    }

    printf("\nUnable to get moveable metadata.");
    return MOM_FAILURE;
}

static mom_ret_t 
mom_make_entry_at(mom_region_address_t* mom_addr,
              uint32_t index) {
    
    uint64_t address = get_moveable_memory_start() + 
               sizeof(mom_metadata_t) +
               sizeof(mom_region_address_t) * index;

    if(pal_mem_write(address,
                    (uint8_t*) mom_addr,
                    sizeof(mom_region_address_t))) {
        return MOM_SUCCESS;
    }

    return MOM_FAILURE;
}

static mom_ret_t
mom_read_entry_at(mom_region_address_t** mom_addr,
                  uint32_t index) {

        uint64_t address = get_moveable_memory_start() +
                           sizeof(mom_metadata_t) +
                           sizeof(mom_region_address_t) * index;

        if(pal_mem_read(address,
                        (uint8_t*)*mom_addr,
                        sizeof(mom_region_address_t))) {
                return MOM_SUCCESS;
        }

        return MOM_FAILURE;
}

/* Tag entry manipulation */
mom_ret_t 
mom_make_entry(char *name,
           uint64_t address,
           uint32_t size,
           uint32_t entry_count = MAXENTRIES) {

    mom_metadata_t *meta = (mom_metadata_t*) malloc(sizeof(mom_metadata_t));
    mom_region_address_t *addr = (mom_region_address_t*)
                                  malloc(sizeof(mom_region_address_t));
    uint64_t index = MAXREGIONS;
    uint64_t alloced = -1;
    mom_ret_t ret = MOM_SUCCESS;

    if(get_moveable_metadata(&meta) == MOM_FAILURE) {
        ret = MOM_FAILURE;
        goto exit;
    }

    if((mom_search_region(name, &addr, &index) == MOM_SUCCESS) &&
       (addr->valid == true)){
        printf("\nRegion already exists, and valid");
        ret = MOM_FAILURE;
        goto exit;
    } 

    alloced = meta->region_alloced;
        
    if(index == MAXREGIONS) {
        memset(addr, 0, sizeof(mom_region_address_t));
        strcpy(addr->name, name);
        meta->region_alloced++;
    }

    if(entry_count != MAXENTRIES &&
       size >= entry_count) {
        addr->entry_length = size/entry_count;
        printf("\nENTRY LENGTH = %u", addr->entry_length);
    }

    addr->valid = true;
    addr->address = address;
    addr->size = size;
    addr->entry_count = entry_count;

    if(index == MAXREGIONS) {
        mom_make_entry_at(addr, alloced);
    } else {
        mom_make_entry_at(addr, index);
    }

    meta->end_address = address + size;
    if(pal_mem_write(get_moveable_memory_start(),
             (uint8_t*) meta,
             sizeof(mom_metadata_t)));

exit:
    free(addr);
    free(meta);
    return ret;
}

mom_ret_t 
mom_search_region(char *name,
                  mom_region_address_t **addr,
                  uint64_t *index) {

    uint64_t alloced = -1;
    uint64_t i = 0;
    *index = MAXREGIONS;
    mom_metadata_t *meta = (mom_metadata_t*) malloc(sizeof(mom_metadata_t));
    mom_ret_t ret = MOM_FAILURE;

    if(name == NULL || addr == NULL) {
        ret = MOM_FAILURE;
	printf("\nName or Address is NULL.");
        goto exit;    
    }

    if(get_moveable_metadata(&meta) == MOM_FAILURE) {
        ret = MOM_FAILURE;
	printf("\nUnable to get moveable metadata.");
        goto exit;
    }

    alloced = meta->region_alloced;

    for(i = 0; i < alloced; i++) {
        memset(*addr, 0, sizeof(mom_region_address_t));

        if(mom_read_entry_at(addr, i) == MOM_SUCCESS) {
           if(strcmp((*addr)->name, name) == 0) {
               *index = i;
               ret = MOM_SUCCESS;
               goto exit;
           }
        }
	
	printf("\nUnable to find regions.");
    }

    /* If control reached here, it indicates failure to find region */
    memset(*addr, 0, sizeof(mom_region_address_t));

exit:
    free(meta);    
    return ret;
}

static mom_ret_t
mom_update_region(char *region_name,
                  mom_region_address_t *to_region) {
    mom_region_address_t *addr = (mom_region_address_t *) malloc(sizeof(mom_region_address_t));
    uint64_t index = 0xFFFF;
    mom_ret_t ret = MOM_FAILURE;

    if(mom_search_region(region_name, &addr, &index) == MOM_SUCCESS) {
	printf("\nFOUND AT [%lu] to_region->addr = %lx", index, to_region->address);
        addr->address = to_region->address;
        addr->size = to_region->size;
        addr->entry_length = to_region->entry_length;
        addr->entry_count = addr->size / addr->entry_length;
        mom_make_entry_at(addr, index);
    }

    free(addr);
    return ret;
}

/* Public APIs */
mom_ret_t 
mom_init(uint32_t region_count) {
    mom_metadata_t meta;
    uint8_t *regions = (uint8_t *) malloc(sizeof(mom_region_address_t) * region_count);
    uint64_t region_size = sizeof(mom_region_address_t) * region_count;

    memset(&meta, 0, sizeof(meta));
    memset(regions, 0, region_size);

    meta.start_address = get_moveable_memory_start() + sizeof(meta);
    meta.region_count = region_count;
    meta.region_alloced = 0;

    if(!pal_mem_write(get_moveable_memory_start(),
                         (uint8_t*) &meta,
                  sizeof(meta))) {
        return MOM_FAILURE;
    }

    if(!pal_mem_write(meta.start_address,
                      regions,
                  region_size)) {
        return MOM_FAILURE;
    }

    return MOM_SUCCESS;     
}

uint64_t
alloc_region(char *name, uint32_t size, uint32_t entry_count) {
    uint64_t address = moveable_memory_sbrk(size);

    if(size%2 != 0 || entry_count%2 != 0) {
	printf("\nSize and/or entry_count are not even.");
	return 0;
    }

    if(mom_make_entry(name, address, size, entry_count) == MOM_SUCCESS) {
        return address;
    } else {
        printf("\nAlloc region failure.");
        return 0;
    }
}

void
free_region(char *name) {
    mom_region_address_t *addr = (mom_region_address_t *) malloc(sizeof(mom_region_address_t));
    uint64_t index = 0xFFFF;

    if(mom_search_region(name, &addr, &index) == MOM_SUCCESS) {
        addr->valid = false;
        mom_make_entry_at(addr, index);        
    } else {
        printf("\nCannot free region. Looks like %s region does not exist.", name);
    }
}

void 
print_mom_memory_map() {
    mom_region_address_t *addr = (mom_region_address_t*) malloc(sizeof(mom_region_address_t));
    mom_metadata_t *meta = (mom_metadata_t*) malloc(sizeof(mom_metadata_t));
    uint64_t alloced = MAXREGIONS;
    uint64_t i = 0;

    if(get_moveable_metadata(&meta) == MOM_FAILURE) {
        goto end;
        }

        alloced = meta->region_alloced;

    for(i = 0; i < alloced; i++) {
        memset(addr, 0, sizeof(mom_region_address_t));

        if(mom_read_entry_at(&addr, i) == MOM_SUCCESS) {
            printf("\nVALID = %d ADDRESS : 0x%x \tNAME : %s",
                addr->valid,
                (unsigned int) addr->address,
                addr->name);
        }    
    }

end:
    free(meta);
    free(addr);
    return;
}


/* Sideeffect : Memory allocated for MAP. The calling function must free map after NULL check.*/
mom_ret_t
get_memory_map(mom_region_address_t **map) {
        mom_region_address_t *addr = NULL; 
        mom_metadata_t *meta = (mom_metadata_t*) malloc(sizeof(mom_metadata_t));
        uint64_t region_count = -1;
        uint64_t i = 0;
    mom_ret_t ret = MOM_FAILURE;
    uint64_t size = -1;

    if(get_moveable_metadata(&meta) == MOM_FAILURE) {
        goto end;
    }

    region_count = meta->region_count;
    size = sizeof(mom_region_address_t) * region_count;

    addr = (mom_region_address_t *) malloc(size);

    size = (uint64_t)addr+size - 10;
        for(i = 0; i < region_count; i++) {
        mom_region_address_t *entry = addr; 

        entry += i;
                memset(entry, 0, sizeof(mom_region_address_t));
                if(mom_read_entry_at(&entry, i) != MOM_SUCCESS) {
                        printf("\nEntry read failed.");
            ret = MOM_FAILURE;
            goto end;
                }
        }

    ret = MOM_SUCCESS;

end:
        free(meta);
    *map = addr;
        return ret;
}

mom_ret_t
write_memory_map(mom_region_address_t *map) {
        mom_metadata_t *meta = (mom_metadata_t*) malloc(sizeof(mom_metadata_t));
        mom_ret_t ret = MOM_SUCCESS;
    uint64_t region_size = MAXREGIONS;

    if(get_moveable_metadata(&meta) == MOM_FAILURE) {
        ret = MOM_FAILURE;
        goto end;
    }

    if(!pal_mem_write(get_moveable_memory_start(),
                      (uint8_t*)meta,
                      sizeof(mom_metadata_t))) {
        ret = MOM_FAILURE;
        goto end;
    }

    region_size = meta->region_count * sizeof(mom_region_address_t);

    printf("\nREGION SIZE is : %lu [ADDR = %u]", region_size, meta->start_address);
    if(!pal_mem_write(meta->start_address,
                      (uint8_t*) map,
                      region_size)) {
            ret = MOM_FAILURE;
    }

end:
    free(meta);
        return ret;
}

mom_ret_t
mom_write_entry_to_region(char *region_name,
                          int entry_index,
                          uint8_t* data) {
    mom_region_address_t *addr = (mom_region_address_t*)
                                     malloc(sizeof(mom_region_address_t));
    uint64_t index = MAXREGIONS;
    mom_ret_t ret = MOM_FAILURE;

    printf("\nMaking entry into region - %s", region_name);
    if(mom_search_region(region_name, &addr, &index) == MOM_SUCCESS) {
        uint64_t mem_address = addr->address + addr->entry_length * entry_index;
	if(pal_mem_write(mem_address, (uint8_t*) data, addr->entry_length)) {
            ret = MOM_SUCCESS;    
	}
    }

    free(addr);
    return ret;
}

mom_ret_t
mom_read_entry_from_region(char *region_name,
                           int entry_index,
                           uint8_t **data) {
    mom_region_address_t *addr = (mom_region_address_t*)
                                     malloc(sizeof(mom_region_address_t));
    uint64_t index = MAXREGIONS;
    mom_ret_t ret = MOM_FAILURE;

    printf("\nReading entry from region - %s", region_name);
    if(mom_search_region(region_name, &addr, &index) == MOM_SUCCESS) {
        uint64_t mem_address = addr->address + addr->entry_length * entry_index;
        if(pal_mem_read(mem_address, (uint8_t*) *data, addr->entry_length)) {
	    printf("\nSetting return to success.");
            ret = MOM_SUCCESS;
        }
    }

    printf("\nRET = %d", ret);
    free(addr);
    return ret;
}

static mom_ret_t
mom_move_region_by_entry(uint64_t from_address,
			 uint64_t from_stride,
			 uint64_t entry_count,
			 uint64_t to_address,
			 uint64_t to_stride,
			 int (*transform_fn)(uint8_t*, uint8_t**)) {
    mom_ret_t ret = MOM_SUCCESS;

    uint8_t *entry_in = (uint8_t*) malloc(from_stride);
    uint8_t *entry_out = (uint8_t*) malloc(to_stride);
    uint64_t i = 0;
    ret = MOM_SUCCESS;

    for(i = 0; i < entry_count; i++) {
        memset(entry_in, 0, from_stride);
        memset(entry_out, 0, to_stride);

        if(pal_mem_read(from_address + from_stride * i,
                        entry_in,
			from_stride) != 1) {
            ret = MOM_FAILURE;
            break;
        }

        if(transform_fn != NULL &&
	   !transform_fn(entry_in, &entry_out)) {
            printf("\nMemory entry transform failed at index %lu", i);
            ret = MOM_FAILURE;
            break;
        }

        if(pal_mem_write(to_address + i * to_stride,
                         entry_out,
                         to_stride) != 1) {
            ret = MOM_FAILURE;
            break;
        }
    }

    free(entry_in);
    free(entry_out);

    return ret;
}

mom_ret_t
mom_move_region(mom_region_address_t *from_region,
		mom_region_address_t *to_region,
		int (*transform_fn)(uint8_t*, uint8_t**)) {
    mom_ret_t ret = MOM_FAILURE;

    if(from_region == NULL || to_region == NULL) {
        goto exit;
    }

    if(transform_fn == NULL && from_region->size < 10240) {
        uint8_t *entry_in_out = (uint8_t*) malloc(from_region->size);

        if(!pal_mem_read(from_region->address,
                         entry_in_out,
                         from_region->size)) {
	    printf("\nPAL mem read failed.");
	    free(entry_in_out);
 	    ret = MOM_FAILURE;
	    goto exit;	      
        }

        if(!pal_mem_write(to_region->address,
                          entry_in_out,
                          from_region->size)) {
	    printf("\nPAL mem write failed.");
            free(entry_in_out);
            ret = MOM_FAILURE;
            goto exit;
        }
    } else { 
	ret = mom_move_region_by_entry(from_region->address,
                                       from_region->entry_length,
                         	       from_region->entry_count,
                         	       to_region->address,
                         	       to_region->entry_length,
			               transform_fn); 
    }

    if(ret == MOM_SUCCESS) {
	ret = mom_update_region(from_region->name, to_region);
    }

exit:
    return ret;
}
