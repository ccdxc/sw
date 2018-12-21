#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <time.h>

#include "platform/pal/include/pal.h"

#define MAXAPPNAME 64
#define MAXREGIONNAME 64
#define OCCUPIEDFLAG    0x80
#define ALLOCFLAG       0x40
#define EXCLFLAG        0x20
#define COHERENTFLAG    0x08
#define CACHEABLEFLAG   0x04
#define RANDOMMAX 20


void print_help() {
    printf("palparallel -n <application-name> -o <number of operations>\n");
    printf("add -s flag if you want a real app-like simulation");
}

int main(int argc, char* argv[]) {
    char *application_name = NULL;
    char region_name[MAXREGIONNAME];
    int c = 0;
    u_int64_t flags = 0;
    u_int8_t operations = 0;
    u_int8_t op_cnt = 0;
    u_int8_t sim_mode = 0;

    int r = 0;
    char *region_names[] = {"Ten", "Thousand", "Thundering", "Typhoons", "Blistering", "Barnacles"};
    int region_count = 6;
    

    while((c = getopt(argc, argv, "n:o:s")) != -1) {
        switch(c) {
	    case 'n':
			application_name = optarg; break;
            case 'o':
                        operations = atoi(optarg); break;
	    case 's':
			sim_mode = 1; break;
	    default: 
			print_help();
	} 
    }

    if(application_name == NULL) {
	print_help();
        return 0;
    }

    if(operations == 0 || operations > 255) {
	/* Select 10 operations if no operations is supplied */
        printf("\nSelecting 10 operations.");
	operations = 10;
    }

    pal_init(application_name);

    if (sim_mode == 1) {
	u_int64_t pa[6];
 	u_int8_t i = 0;
        u_int64_t flags = 0;

	for (i = 0; i < region_count; i++) {
	    sprintf(region_name, "%s-%s", application_name, region_names[i]);
            pa[i] = pal_mem_alloc(region_name, 4096, flags);
            printf("\nALLOCED %s at %lx", region_name, pa[i]);
	}

        printf("\n\nALLOCED all required regions");

        pal_mem_wr(pa[0], "REGION0WRITING00000023", 23, MATTR_UNCACHED); 
        pal_mem_wr(pa[1], "REGION1WRITING00000023", 23, MATTR_UNCACHED); 
        pal_mem_wr(pa[2], "REGION2WRITING00000023", 23, MATTR_UNCACHED); 
        pal_mem_wr(pa[3], "REGION3WRITING00000023", 23, MATTR_UNCACHED); 
        pal_mem_wr(pa[4], "REGION4WRITING00000023", 23, MATTR_UNCACHED); 
        pal_mem_wr(pa[5], "REGION5WRITING00000023", 23, MATTR_UNCACHED); 

	sprintf(region_name, "%s-%s", application_name, region_names[2]);
        pal_mem_free(region_name);
        pal_mem_wr(pa[0], "BARUNHALDERBARUNHALDER", 23, MATTR_UNCACHED); 
	sprintf(region_name, "%s-%s", application_name, region_names[0]);
        pal_mem_free(region_name);
	sprintf(region_name, "%s-%s", application_name, region_names[1]);
        pal_mem_free(region_name);
        pal_mem_wr(pa[3], "BARUNHALDERBARUNHALDER", 23, MATTR_UNCACHED); 

        pal_print_metadata();

        return 0;
    }

    srand(time(NULL));

    while(op_cnt != operations) {
        /* 6 operations - ALLOC, FREE, MAP, UNMAP, WRITE, READ */
	int option = rand() % 3;

        switch(option) {
	    case 0: // ALLOC
                        r = rand() % region_count;
			flags = 0;
			flags |= EXCLFLAG;
			flags |= COHERENTFLAG;
		 	sprintf(region_name, "%s-%s", application_name, region_names[r]);
			pal_mem_alloc(region_name, 0xABC00, flags);
	
			break; 
	    case 1: // FREE 
			r = rand() % region_count;
			sprintf(region_name, "%s-%s", application_name, region_names[r]);
			pal_mem_free(region_name);
			break; 
	    case 2: // PRINT 
			pal_print_application_regions(application_name);
			break; 
	    case 3: // UNMAP 
			break; 
	    case 4: // WRITE 
			break; 
	    case 5: // READ 
			break; 
        }

	op_cnt++;
	r = rand() % 3;
	sleep(r);
    } 

    pal_print_metadata();

    return 0;
}
