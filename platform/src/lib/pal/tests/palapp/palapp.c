/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

#include "platform/src/lib/pal/include/pal.h"

#define MAXAPPNAME 64
#define MAXREGIONNAME 64
#define OCCUPIEDFLAG    0x80
#define ALLOCFLAG       0x40
#define EXCLFLAG        0x20
#define COHERENTFLAG    0x08
#define CACHEABLEFLAG   0x04

#define RANDOMDATA "01234567"
#define RANDOMDATASIZE 8 //bytes

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
	    HELP();
	    return 0;
        } else {
	    pal_print_metadata();
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
