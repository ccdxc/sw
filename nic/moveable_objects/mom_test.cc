#include "mom.h"
#include "malloc_stub.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ALL 0 
#define HPTC 1
#define LPTC 2
#define RUN_TEST(test_name) printf("\nTEST"); total++; if((test_name)() == SUCCESS) { success++; } else {failure++;}  

typedef enum TestResult_e {
	SUCCESS = 0,
	FAILURE
} TestResult;

static void
test_setup() {
	memory_init();
	mom_init(0x20);
}

static void
test_teardown() {
	//memory_free();
	print_mom_memory_map();
}

static TestResult 
alloc_test() {
	char data[10] = "region-1";
        uint64_t *rand = (uint64_t*) malloc(32);
	rand[0] = 0xFF;
	rand[1] = 0xCC;

	test_setup();

	alloc_region(data, 256, 8);
	mom_write_entry_to_region(data, 0, (uint8_t*) rand); 
	memset(rand, 0, 32);

	mom_read_entry_from_region(data, 0, (uint8_t**)&rand); 

	printf("The value read is %lx", rand[0]);

	free_region(data);
	test_teardown();
	return SUCCESS;
}

static TestResult
free_test() {
	char data[10] = "region-1";
	test_setup();

	free_region(data);
	test_teardown();
	return SUCCESS;
}

static TestResult
map_read_write_test() {
	char data[10] = "region-1";
	char data2[10] = "region-2";
	mom_region_address_t *map = NULL;

	test_setup();
	alloc_region(data, 0xDEAD, 0xBEBE);
	print_mom_memory_map();

	if (get_memory_map(&map) == MOM_SUCCESS) {
		memcpy((&map[0])->name, data2, 10);
		write_memory_map(map);
	}

        test_teardown();
        return SUCCESS;
}

static TestResult
move_test() {
        char data[10] = "region-1";
        uint64_t *rand = (uint64_t*) malloc(32);
        rand[0] = 0xAA;
        rand[1] = 0xBB;
	mom_region_address_t to_address;
	mom_region_address_t* from_address = (mom_region_address_t*) malloc(sizeof(mom_region_address_t));
	uint64_t index;

	strcpy(to_address.name, "region-3");
	to_address.address = 0xDEAD;
	

        test_setup();

        alloc_region(data, 256, 8);
        mom_write_entry_to_region(data, 0, (uint8_t*) rand);
        memset(rand, 0, 32);
        mom_read_entry_from_region(data, 0, (uint8_t**)&rand);
	mom_search_region(data, &from_address, &index);

	mom_move_region(from_address, &to_address, NULL);

        printf("The value read is %lx", rand[0]);

        free_region(data);
        test_teardown();
        return SUCCESS;
}

int transform_fn(uint8_t *from, uint8_t **to) {
	char randomData[32] = "DEADBEEFDEADBEEF";
	memcpy(*to, randomData, 32);
	return 1;
}

static TestResult
move_test_huge_null_fn() {
        char data[10] = "region-1";
        uint64_t *rand = (uint64_t*) malloc(32);
        rand[0] = 0xCAAAAAAFAAAAAAAA;
        rand[1] = 0xDBBBBBBEBBBBBBBB;
        mom_region_address_t to_address;
        mom_region_address_t* from_address = (mom_region_address_t*) malloc(sizeof(mom_region_address_t));
        uint64_t index;

        strcpy(to_address.name, "region-3");
        to_address.address = 0x1DEA0;

        test_setup();

        alloc_region(data, 256, 8);
	for(int i = 0; i < 8; i++) {
        	mom_write_entry_to_region(data, i, (uint8_t*) rand);
	}

        mom_search_region(data, &from_address, &index);
	to_address.entry_length = from_address->entry_length;
        mom_move_region(from_address, &to_address, NULL);

        //free_region(data);
        test_teardown();
        return SUCCESS;
}

static TestResult
move_test_huge() {
        char data[10] = "region-1";
        uint64_t *rand = (uint64_t*) malloc(32);
        rand[0] = 0xCAAAAAAFAAAAAAAA;
        rand[1] = 0xDBBBBBBEBBBBBBBB;
        mom_region_address_t to_address;
        mom_region_address_t* from_address = (mom_region_address_t*) malloc(sizeof(mom_region_address_t));
        uint64_t index;

        strcpy(to_address.name, "region-3");
        to_address.address = 0xDD0;


        test_setup();

        alloc_region(data, 256, 8);
        for(int i = 0; i < 8; i++) {
                mom_write_entry_to_region(data, i, (uint8_t*) rand);
        }

        mom_search_region(data, &from_address, &index);
	to_address.entry_length = from_address->entry_length;
        mom_move_region(from_address, &to_address, transform_fn);

        free_region(data);
        test_teardown();
        return SUCCESS;
}

static void
run_tests(int level) {
	int total = 0;
	int success = 0;
	int failure = total;

	printf("\nRunning MOM tests...\n");

	if(level == ALL || level == HPTC) {
		RUN_TEST(move_test_huge);
	} 

	if (level == ALL || level == LPTC) {
		RUN_TEST(move_test_huge_null_fn);
		RUN_TEST(move_test)
		RUN_TEST(alloc_test)
		RUN_TEST(free_test)
		RUN_TEST(map_read_write_test)
	}

	printf("\nAll tests run. SUCCESS = %d, FAILED = %d, SKIPPED = %d\n\n",
		success, failure, total - success - failure);
}

int main() {
	printf("\n");

	run_tests(HPTC);
	return 0;
}
