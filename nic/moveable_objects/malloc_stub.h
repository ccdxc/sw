#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/* Internal APIs */
void memory_init();
void memory_free();

/* Primary APIs */
uint64_t moveable_memory_sbrk(int size); 
uint64_t get_memory_base();
uint64_t get_memory_size();
uint64_t get_moveable_memory_start();

/* PAL APIs - Stub */
int pal_mem_read(uint64_t address, uint8_t* data, uint32_t size);
int pal_mem_write(uint64_t address, uint8_t* data, uint32_t size);
