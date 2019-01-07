/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include "platform/pal/include/pal.h"
#include <assert.h>

#define MAXAPPNAME 64
#define MAXREGIONNAME 64
#define OCCUPIEDFLAG    0x80
#define ALLOCFLAG       0x40
#define EXCLFLAG        0x20
#define COHERENTFLAG    0x08
#define CACHEABLEFLAG   0x04

#define RANDOMDATA "01234567"
#define RANDOMDATASIZE 8 //bytes

struct random_struct
{
    u_int16_t b;
    u_int32_t c;
    u_int64_t d;
    u_int8_t a;
}__attribute__((packed));

void
pal_mem_general()
{
    u_int8_t uint8     = 0xAB;
    u_int16_t uint16   = 0xBEBA;
    u_int32_t uint32   = 0xDEADBEEF;
    u_int64_t uint64   = 0xBEEFBABADEADBEBE;
    u_int64_t uint128[2] = {0x1234567812345678, 0xBABABEBEDEADBEEF};

    u_int8_t uint8_check = 0;
    u_int16_t uint16_check = 0;
    u_int32_t uint32_check = 0;
    u_int64_t uint64_check = 0;
    u_int64_t uint128_check[2] = {0, 0};

    struct random_struct r = {.a = 0xAB, .b = 0xBEBE, .c = 0xDEADBEEF, .d = 0xCECEDADABABABEBE};
    struct random_struct r_check;

    u_int64_t pa = 0xc0000000;

    printf("Writing 1 byte = %x\n", uint8);
    pal_mem_wr(pa, (void*)&uint8, sizeof(uint8), 1);
    pal_mem_rd(pa, (void*)&uint8_check, sizeof(uint8_check), 1);
    printf("Read 1 byte = %x\n", uint8_check);
    assert(uint8 == uint8_check);

    printf("Writing 2 bytes = %x\n", uint16);
    pal_mem_wr(pa, (void*)&uint16, sizeof(uint16), 1);
    pal_mem_rd(pa, (void*)&uint16_check, sizeof(uint16_check), 1);
    printf("Read 2 bytes = %x\n", uint16_check);
    assert(uint16 == uint16_check);

    printf("Writing 4 bytes = %x\n", uint32);
    pal_mem_wr(pa, (void*)&uint32, sizeof(uint32), 1);
    pal_mem_rd(pa, (void*)&uint32_check, sizeof(uint32_check), 1);
    printf("Read 4 bytes = %x\n", uint32_check);
    assert(uint32 == uint32_check);

    printf("Writing 8 bytes = %lx\n", uint64);
    pal_mem_wr(pa, (void*)&uint64, sizeof(uint64), 1);
    pal_mem_rd(pa, (void*)&uint64_check, sizeof(uint64_check), 1);
    printf("Read 8 bytes = %lx\n", uint64_check);
    assert(uint64 == uint64_check);
    
    printf("Writing 16 bytes = %lx %lx\n", uint128[0], uint128[1]);
    pal_mem_wr(pa, (void*)uint128, sizeof(uint128), 1);
    pal_mem_rd(pa, (void*)&uint128_check, sizeof(uint128_check), 1);
    printf("Read 16 bytes = %lx %lx\n", uint128_check[0], uint128_check[1]);
    assert(uint128[0] == uint128_check[0]);
    assert(uint128[1] == uint128_check[1]);

    printf("Writing random struct of size : %ld bytes\n", sizeof(r));
    pal_mem_wr(pa, (void*)&r, sizeof(r), 1);
    pal_mem_rd(pa, (void*)&r_check, sizeof(r_check), 1);
    printf("Read random struct = %x %x %x %lx\n", r_check.a, r_check.b, r_check.c, r_check.d);
    assert(r.a == r_check.a);
    assert(r.b == r_check.b);
    assert(r.c == r_check.c);
    assert(r.d == r_check.d);

    printf("All pal_mem_* APIs tested successfuly.\n");
}

#define HELP() {\
              printf("palmm <application_name> <operation> <options>\n"); \
              printf("operation:\n"); \
              printf("   p : print regions owned by application\n"); \
              printf("   a : allocate region\n"); \
              printf("   f : free region\n"); \
              printf("   m : map region\n"); \
              printf("   g : get physical address of region\n"); \
              printf("\noptions:\n"); \
              printf("   n : name of application\n"); \
              printf("   r : region name\n"); \
              printf("   d : data to be written\n"); \
              printf("   c : coherent region\n"); \
              printf("   e : exclusive region\n"); \
	      printf("   s : size (Bytes) of region to be alloced.\n"); \
              }

int main(int argc, char* argv[]) {
    char *application_name = NULL;
    char *region_name = NULL;
    int c = 0;
    u_int8_t print = 0;
    u_int8_t map = 0;
    u_int8_t alloc = 0;
    u_int8_t free = 0;
    u_int8_t exclusive = 0;
    u_int8_t coherent = 0;
    u_int64_t size = 0;
    u_int8_t write = 0;
    u_int8_t getpa = 0;
    char *data = NULL;

    while((c = getopt(argc, argv, "n:pmr:ecafs:wgd:")) != -1) {
        switch(c) {
	    case 'p':
			print = 1; break;
	    case 'a':
			alloc = 1; break;
	    case 'f':
			free = 1; break;
	    case 'm':
			map = 1; break;
	    case 'n':
			application_name = optarg; break;
            case 'd':
                        data = optarg; break;
	    case 'r':
			region_name = optarg; break;
            case 'g':
                        getpa = 1; break;
	    case 'c':
			coherent = 1; break;
	    case 'e':
			exclusive = 1; break;
            case 'w':
                        write = 1; break;
	    case 's':
			size = atoi(optarg); break;
	   default: 
			HELP();
	} 
    }

    if(application_name == NULL) {
	if(print != 1)  {
            printf("Running pal_mem general tests.\n");
	    pal_mem_general();
	    return 0;
        } else {
	    HELP();
	    return 0;
	}
    } else {
    	pal_init(application_name);
    }

    //pal_mem_trace_control(1);

    if(print == 1) {
        pal_print_application_regions(application_name);
    } else if(alloc == 1) {
        u_int32_t flags = 0;
	u_int64_t pa = 0;

        if(coherent == 1) {
	    flags |= COHERENTFLAG;
	}

        if(exclusive == 1) {
	    flags |= EXCLFLAG;
	}
	pa = pal_mem_alloc(region_name, size, flags);

	printf("[PALAPP : %s] Alloced region %s of size %ld with flags %x at %lx\n",
		application_name,
                region_name,
                size,
                flags,
                pa);
    } else if (free == 1) {
	pal_mem_free(region_name);
	printf("[PALAPP : %s] Free for region %s\n",
                 application_name,
                 region_name);
    } else if (map == 1) {
        void *va = NULL;
	va = pal_mem_map_region(region_name);

        if(va != NULL) {
	    printf("[PALAPP : %s] Region [%s] mapped to VA = %p\n",
                   application_name,
                   region_name,
                   va);
	} else {
	    printf("\n[PALAPP : %s] Failed to map Region [%s]\n",
                   application_name,
                   region_name);
	    return 1;
        }
    } else if (write == 1) {
        u_int32_t flags = 0;
        u_int64_t pa = 0;
        char *buf = (char*) malloc(size);
        u_int64_t written = 0;

        buf[0] = '\0';

	printf("[PALAPP : %s] Write data of size %ld\n",
	       application_name,
	       size);
       
        while(written < size) {
		strcat(buf, RANDOMDATA);
		written += RANDOMDATASIZE;
	}
	 
        if(coherent == 1) {
            flags |= COHERENTFLAG;
        }

        if(exclusive == 1) {
            flags |= EXCLFLAG;
        }

	// Allocate 16 pages 
        pa = pal_mem_alloc(region_name, 4096 * 16, flags);
	printf("\nGot address of %lx after allocation", pa);

	pal_mem_wr(pa, (u_int8_t*) buf, written, MATTR_UNCACHED);
    } else if (getpa == 1) {
	u_int64_t pa = pal_mem_region_pa(region_name);
	printf("\nPA for region %s is 0x%lx", region_name, pa);
    } else if (data != NULL && region_name != NULL) {
        u_int64_t pa = pal_mem_region_pa(region_name);
	pal_mem_wr(pa, data, strlen(data), MATTR_UNCACHED);
    }
    
    return 0;
}
