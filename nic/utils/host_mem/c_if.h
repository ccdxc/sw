// C interface to host_mem.
// Since there can only be one host mem, the object is made global.
#ifndef _HOST_MEM_C_IF_H_
#define _HOST_MEM_C_IF_H_

#include <stdint.h>
#include <sys/types.h>

extern "C" {

int init_host_mem();
void *alloc_host_mem(size_t size);
void *alloc_page_aligned_host_mem(size_t size);
void *alloc_spec_aligned_host_mem(size_t size, size_t spec_align_size);
void free_host_mem(void *ptr);
uint64_t host_mem_v2p(void *ptr);
void *host_mem_p2v(uint64_t phys);
void delete_host_mem();

}  // extern 'C'

#endif  // _HOST_MEM_C_IF_H_
