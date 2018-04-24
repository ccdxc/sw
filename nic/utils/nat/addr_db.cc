// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/utils/nat/addr_db.hpp"
#include "sdk/slab.hpp"

using sdk::lib::ht;
using sdk::lib::slab;

namespace hal {
namespace utils {
namespace nat {

#define NAT_MAX_ADDR 1024  //TODO - right value?

static void *
addr_entry_key_func_get (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((addr_entry_t *)entry)->key);
}

static uint32_t
addr_entry_key_hash_func_compute (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(
        key, sizeof(addr_entry_key_t)) % ht_size;
}

static bool
addr_entry_key_func_compare (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(addr_entry_key_t)))
        return true;
    return false;
}

ht *addr_db_;
hal_ret_t
addr_db_init (uint32_t db_size)
{
    addr_db_ =
        sdk::lib::ht::factory(db_size << 1,
                              addr_entry_key_func_get,
                              addr_entry_key_hash_func_compute,
                              addr_entry_key_func_compare);
    HAL_ASSERT_RETURN((addr_db_ != NULL), HAL_RET_ERR);
    return HAL_RET_OK;
}

static inline ht *
addr_db (void)
{
    return addr_db_;
}

static inline addr_entry_t *
addr_entry_db_lookup (addr_entry_key_t *key)
{
    return (addr_entry_t *)addr_db()->lookup(key);
}

static inline hal_ret_t
addr_entry_db_insert (addr_entry_t *entry)
{
    sdk_ret_t ret;

    ret = addr_db()->insert(entry, &entry->db_node);
    if (ret == sdk::SDK_RET_OK)
        return HAL_RET_OK;
    else
        return HAL_RET_ENTRY_EXISTS;

    return HAL_RET_ERR;
}

static inline addr_entry_t *
addr_entry_db_remove (addr_entry_key_t *key)
{
    return ((addr_entry_t *)(addr_db()->remove(key)));
}

slab  *addr_entry_slab_;
static inline slab *
addr_entry_slab()
{
    return addr_entry_slab_;
}

addr_entry_t *
addr_entry_alloc (void)
{
    addr_entry_t *entry;
    entry = (addr_entry_t *)addr_entry_slab()->alloc();
    return entry;
}

void
addr_entry_free (addr_entry_t *entry)
{
    // TODO: kalyan, shouldn't we be doing delay delete here ??
    addr_entry_slab()->free(entry);
}

static inline void
addr_entry_fill (addr_entry_t *entry, addr_entry_key_t *key,
                 ip_addr_t tgt_ip_addr)
{
    memcpy(&entry->key, key, sizeof(addr_entry_key_t));
    entry->tgt_ip_addr = tgt_ip_addr;
}

hal_ret_t
addr_entry_add (addr_entry_key_t *key, ip_addr_t tgt_ip_addr)
{
    addr_entry_t *entry;

    if (addr_entry_db_lookup(key) != NULL) {
        return HAL_RET_ENTRY_EXISTS;
    }

    if ((entry = addr_entry_alloc()) == NULL) {
        return HAL_RET_OOM;
    }

    addr_entry_fill(entry, key, tgt_ip_addr);

    return addr_entry_db_insert(entry);
}

void
addr_entry_del (addr_entry_key_t *key)
{
    addr_entry_t *entry;

    entry = addr_entry_db_remove(key);
    addr_entry_free(entry);
}

addr_entry_t *
addr_entry_get (addr_entry_key_t *key)
{
    return addr_entry_db_lookup(key);
}

} // namespace nat
} // namespace plugins
} // namespace hal
