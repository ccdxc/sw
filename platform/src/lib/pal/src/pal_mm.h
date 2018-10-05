/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <fcntl.h>
#include <unistd.h>

#ifndef _PAL_MM_H_
#define _PAL_MM_H_

#define MAJORVERSION 1
#define MINORVERSION 0

#define MAGIC "0PALMEM0"
#define MAGICSIZE 8
#define MAXARENAS 32
#define MAXREGIONS 1024 
#define MAXREGIONNAME 64 
#define MAXUUID 32 
#define MINALLOC 4096
#define MEMSTART 0xC0000000

/* Keep the order as is. */
typedef enum uuid_e {
    NICMGR	= 	0,
    HAL  	=	1,
    PCIEMGR	=	2  
} uuid_t;

#define GET(region_addr) (*(u_int64_t*)region_addr)
#define SIZEMASK (~((u_int64_t)(0xFFF)))
#define FUTUREMASK (~0xFE)

#define OCCUPIEDFLAG		0x80
#define ALLOCFLAG		0x40
#define EXCLFLAG		0x20
#define COHERENTFLAG		0x08
#define CACHEABLEFLAG		0x04
#define ALLOC32BITFLAG		0x02

#define ISCOHERENT(region_addr) ((GET(region_addr) & ((u_int64_t)COHERENTFLAG)) != 0)
#define SETCOHERENT(region_addr) (GET(region_addr) |= (u_int64_t)COHERENTFLAG)

#define ISCACHEABLE(region_addr) ((GET(region_addr) & ((u_int64_t)CACHEABLEFLAG)) != 0)
#define SETCACHEABLE(region_addr) (GET(region_addr) |= (u_int64_t)CACHEABLEFLAG)

#define ISOCCUPIED(region_addr) ((GET(region_addr) & ((u_int64_t)OCCUPIEDFLAG)) != 0)
#define SETOCCUPIED(region_addr) (GET(region_addr) |= (u_int64_t)OCCUPIEDFLAG)
#define CLEAROCCUPIED(region_addr) (GET(region_addr) &= ~((u_int64_t)OCCUPIEDFLAG))

#define ISALLOCATED(region_addr) ((GET(region_addr) & ((u_int64_t)ALLOCFLAG)) != 0)
#define SETALLOCATED(region_addr) (GET(region_addr) |= (u_int64_t)ALLOCFLAG)
#define SETFREE(region_addr) (GET(region_addr) &= ~((u_int64_t)ALLOCFLAG))

#define ISEXCL(region_addr) ((GET(region_addr) & ((u_int64_t)EXCLFLAG)) != 0)
#define SETEXCL(region_addr) (GET(region_addr) |= (u_int64_t)EXCLFLAG)

#define ISSHAREABLE(region_addr) ((GET(region_addr) & ((u_int64_t)EXCLFLAG)) == 0)
#define SETSHAREABLE(region_addr) (GET(region_addr) &= ~((u_int64_t)EXCLFLAG))

#define GETREGIONSIZE(region_addr) (GET(region_addr) & (u_int64_t)SIZEMASK)
#define SETREGIONSIZE(region_addr, size) GET(region_addr) &= ~SIZEMASK; GET(region_addr) |= size;

/* Memory region header */
typedef struct pal_mem_region_s {
    /* "flags" represents the following - 
    	u_int64_t size : 56;
    	u_int64_t occupied : 1;
    	u_int64_t alloc : 1;
    	u_int64_t permission : 2;
    	u_int64_t coherent : 1;
    	u_int64_t cacheable : 1;
    	u_int64_t future : 2;

	representing flags as a single u_int64_t 
        to not have dependency on compiler's bit packing.
    */
    u_int64_t flags;
    char region_name[MAXREGIONNAME];

    /* UUID == 0 => Any process has access to the region */
    char UUID[MAXUUID];
    u_int16_t next_idx;
    u_int16_t prev_idx;
    u_int16_t arena_idx;
    u_int64_t start_addr;

    /* Adding a pad here to align to 128bytes.
     * It can be used in future. */
    char pad[10];
} pal_mem_region_t;

/* Memory arena - Coherent/non-Coherent */
typedef struct pal_mem_arena_s {
    u_int64_t start;
    u_int64_t size;
    u_int64_t flags;
    u_int64_t free_space;
    u_int64_t top;
    u_int16_t allocated_region_idx;
    u_int16_t free_region_idx;
} pal_mem_arena_t;

/* Metadata Must be 4K aligned */
typedef struct pal_mem_metadata_s {
    char magic[MAGICSIZE]; 
    u_int16_t major_ver;
    u_int16_t minor_ver;
    pal_mem_arena_t arenas[MAXARENAS];
    pal_mem_region_t regions[MAXREGIONS];
} pal_mem_metadata_t;

typedef enum pal_ret_e {
    PAL_FAIL = 0,
    PAL_SUCCESS = 1
} pal_ret_t;

void pal_mm_init(char *application_name);

u_int64_t pal_mem_alloc_int(char *region_name, uint32_t size, u_int32_t alloc_flags); 

void pal_mem_free_int(char *region_name); 

pal_ret_t pal_pa_access_allowed(u_int64_t pa, u_int64_t sz);

void *pal_mem_map_region_int(char *region_name);

void pal_mem_unmap_region_int(char *region_name);

u_int64_t pal_mem_region_pa_int(char *region_name);

#endif
