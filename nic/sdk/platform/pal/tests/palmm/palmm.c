/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"

#define GARBAGE "01234567"
#define GARBAGESZ 8

int main(int argc, char* argv[]) {
    u_int64_t pa = 0;
    char buf[64]; 
 
    if(argc == 1) {
    	pal_mem_wr(pa, "BARUNHALDERBARUNHALDER", 23, MATTR_UNCACHED); 
    	pal_mem_rd(pa, buf, 23, MATTR_UNCACHED);
    } else if(argc == 2) {
	pal_mem_wr(pa, argv[1], sizeof(argv[1]), MATTR_UNCACHED);
    	pal_mem_rd(pa, buf, sizeof(argv[1]), MATTR_UNCACHED);
    }

    pal_init("PALMMTESTAPP");

    printf("\nMEM_READ = %s\n", buf);

    pa = pal_mem_alloc("test-region", 1024 * 1024, 0x01);
    printf("\nWRITING into address %lx", pa);
    pal_mem_wr(pa, "HELLOWORLDHELLOWORLD", 21, MATTR_UNCACHED);

    pal_mem_alloc("test-region-2", 1024 * 1024, 0x01);
    pal_mem_free("test-region-2");
    pal_mem_free("test-region");
    pal_mem_alloc("test-region-3", 1024 * 1024, 0x01);
    pal_mem_alloc("test-region-4", 1024 * 1024, 0x01);

    pal_mem_alloc("test-region-5", 1024 * 1024, 0);
    pal_mem_alloc("test-region-6", 1024 * 1024, 0);
    pal_mem_free("test-region-6");
    pa = pal_mem_alloc("test-region-7", 1024 * 1024, 0);
    printf("\nRegion 7 allocated at address = %lx", pa);
    pal_mem_wr(pa, "PENSANDOSYSTEMS", 16, MATTR_UNCACHED);
    pal_mem_unmap_region("test-region-7");
    
    pal_mem_rd(pa, buf, 16, MATTR_UNCACHED);
    printf("\n++++++++++++++ The buffer read from PA[%lx] is %s +++++++++++++\n", pa, buf);
     
    pal_mem_map_region("test-region-7");
   
    pal_mem_free("test-region");
    
    return 0;
}
