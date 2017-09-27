// C interface to host_mem.
// Since there can only be one host mem, the object is made global.
#include <stdint.h>
#include <errno.h>
#include "nic/utils/host_mem/host_mem.hpp"

static std::unique_ptr<utils::HostMem> g_host_mem;

static int _init_host_mem() {
  if (g_host_mem.get() != nullptr)
    return 0;
  g_host_mem.reset(utils::HostMem::New());
  if (g_host_mem.get() == nullptr)
    return -ENOENT;
  return 0;
}

static void *_alloc_host_mem(size_t size, uint32_t align=1) {
  if (g_host_mem.get() != nullptr)
    return g_host_mem->Alloc(size, align);
  return nullptr;
}

static void _free_host_mem(void *ptr) {
  if (g_host_mem.get() != nullptr)
    g_host_mem->Free(ptr);
}

static uint64_t _host_mem_v2p(void *ptr) {
  if (g_host_mem.get() != nullptr)
    return g_host_mem->VirtToPhys(ptr);
  return 0;
}

static void *_host_mem_p2v(uint64_t phys) {
  if (g_host_mem.get() != nullptr)
    return g_host_mem->PhysToVirt(phys);
  return 0;
}

static void _delete_host_mem() {
  g_host_mem.reset(nullptr);
}

extern "C" {

int init_host_mem() {
  return _init_host_mem();
}

void *alloc_host_mem(size_t size) {
  return _alloc_host_mem(size);
}

void *alloc_page_aligned_host_mem(size_t size) {
  return _alloc_host_mem(size, 4096);
}

void free_host_mem(void *ptr) {
  _free_host_mem(ptr);
}

uint64_t host_mem_v2p(void *ptr) {
  return _host_mem_v2p(ptr);
}

void *host_mem_p2v(uint64_t phys) {
  return _host_mem_p2v(phys);
}

void delete_host_mem() {
  _delete_host_mem();
}

}  // extern 'C'
