#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "malloc_stub.h"

#define FILENAME "faux.hbm"

/* Internal APIs */
void memory_init() {
	FILE* fp = fopen(FILENAME, "ab+");

	if (fp == NULL) {
		printf("Couldn't open faux HBM for testing.");
	}

	fclose(fp);
}

void memory_free() {
	unlink(FILENAME);
}

static bool is_outside_boundry(uint32_t) {
	return false;
}

/* Primary APIs */
uint64_t moveable_memory_sbrk(int size) {
        uint64_t ret_address = 0;
	FILE *fp = fopen(FILENAME, "rb+");

        if (is_outside_boundry(size)) {
                printf("\nReturning null from memory sbrk");
        } else {
                char data[size];
                memset((void*)data, 0, size);
                fseek(fp, 0, SEEK_END);
                ret_address = ftell(fp);
		pal_mem_write(ret_address, (uint8_t*) data, size);
		rewind(fp);
		fclose(fp);
        }

        return ret_address;
}

uint64_t get_memory_base() {
	return 0;
}

uint64_t get_memory_size() {
        int size = 0;
	FILE *fp = fopen(FILENAME, "rb+");

        if(fp != NULL) {
                fseek(fp, 0, SEEK_END);
                size = ftell(fp);
                rewind(fp);
        }

	fclose(fp);
        return size;
}

uint64_t get_moveable_memory_start() {
	return 0;
}

/* PAL APIs - Stub */
int pal_mem_read(uint64_t address, uint8_t* data, uint32_t size) {
	FILE *fp = fopen(FILENAME, "rb+");

        if(fp != NULL && data != NULL) {
                fseek(fp, address, SEEK_SET);
                fread(data, 1, size, fp);
                fclose(fp);
		return 1;
        }
	
	printf("\nPAL MEM READ failed. address : %lx, data : %lx, size = %u",
		address, (unsigned long int) data, size);

        return 0;
}

int pal_mem_write(uint64_t address, uint8_t* data, uint32_t size) {
	FILE * fp = fopen(FILENAME, "rb+");
        printf("\nPAL MEM WRITE address : %lx, data : %lx, size = %u",
                address, (unsigned long int) data, size);

	printf("\nData content = %s", (char*) data);
        if(fp != NULL && data != NULL) {
                fseek(fp, address, SEEK_SET);
                if(fwrite(data, 1, size, fp) != size) {
			printf("\nFWRITE failed.");
			fclose(fp);
			return 0;
		}

                fclose(fp);
		return 1;
        }

        printf("\nPAL MEM WRITE failed. address : %lx, data : %lx, size = %u",
                address, (unsigned long int) data, size);

	return 0;
}
