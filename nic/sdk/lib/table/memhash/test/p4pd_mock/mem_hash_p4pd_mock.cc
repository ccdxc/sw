//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "lib/p4/p4_api.hpp"
#include "mem_hash_p4pd_mock.hpp"
#ifdef IRIS
#include "gen/p4gen/p4/include/p4pd.h"
#endif

#ifdef APOLLO
#include "gen/p4gen/apollo/include/p4pd.h"
#endif

#ifdef GFT
#include "gen/p4gen/gft/include/p4pd.h"
#endif

typedef struct mem_hash_mock_table_size_ {
    const char *name; // Name
    uint32_t ksize; // Keysize
    uint32_t dsize; // Datasize
    uint32_t asize; // Datasize
    uint32_t tsize; // Tablesize
} mem_hash_mock_table_size_t;

mem_hash_mock_table_size_t mem_hash_mock_table_sizes[] = {
    [ MEM_HASH_P4TBL_ID_NONE ] = {  
        // MEM_HASH_P4TBL_ID_NONE
        .name = "NONE",
        .ksize = 0,
        .dsize = 0,
        .asize = 0,
        .tsize = 0,
    },
    [ MEM_HASH_P4TBL_ID_H5 ] = {
        // MEM_HASH_P4TBL_ID_H5
        .name = "MEM_HASH_P4TBL_ID_H5",
        .ksize = sizeof(mem_hash_h5_key_t), 
        .dsize = sizeof(mem_hash_h5_actiondata_t),
        .asize = sizeof(mem_hash_h5_appdata_t),
        .tsize = 16*1024*1024,
    },
    [ MEM_HASH_P4TBL_ID_H5_OHASH ] = {
        // MEM_HASH_P4TBL_ID_H5_OHASH
        .name = "MEM_HASH_P4TBL_ID_H5_OHASH",
        .ksize = sizeof(mem_hash_h5_key_t), 
        .dsize = sizeof(mem_hash_h5_actiondata_t),
        .asize = sizeof(mem_hash_h5_appdata_t),
        .tsize = 2*1024*1024,
    },
    [ MEM_HASH_P4TBL_ID_H10 ] = {
        // MEM_HASH_P4TBL_ID_H10
        .name = "MEM_HASH_P4TBL_ID_H10",
        .ksize = sizeof(mem_hash_h10_key_t), 
        .dsize = sizeof(mem_hash_h10_actiondata_t),
        .asize = sizeof(mem_hash_h10_appdata_t),
        .tsize = 256*1024,
    },
    [ MEM_HASH_P4TBL_ID_H10_OHASH ] = {
        // MEM_HASH_P4TBL_ID_H10_OHASH
        .name = "MEM_HASH_P4TBL_ID_H10_OHASH",
        .ksize = sizeof(mem_hash_h10_key_t), 
        .dsize = sizeof(mem_hash_h10_actiondata_t),
        .asize = sizeof(mem_hash_h10_appdata_t),
        .tsize = 64*1024,
    },
    [ MEM_HASH_P4TBL_ID_MAX ] = {  
        // MEM_HASH_P4TBL_ID_MAX
        .name = "NONE",
        .ksize = 0,
        .dsize = 0,
        .asize = 0,
        .tsize = 0,
    },

};

typedef struct mem_hash_mock_table_s {
    void    *klist;
    void    *dlist;
} mem_hash_mock_table_t;

static mem_hash_mock_table_t mocktables[MEM_HASH_P4TBL_ID_MAX];

typedef int p4pd_error_t;

static void
get_key_data_sizes_(uint32_t table_id, uint32_t *ksize, uint32_t *dsize)
{
    assert(table_id && table_id < MEM_HASH_P4TBL_ID_MAX);
    if (ksize) {
        *ksize = mem_hash_mock_table_sizes[table_id].ksize;
    }
    if (dsize) {
        *dsize = mem_hash_mock_table_sizes[table_id].dsize;
    }
    return;
}

int
mem_hash_mock_init ()
{
    uint32_t table_size = 0;
    uint32_t tid = 0;
    uint32_t ksize = 0;
    uint32_t dsize = 0;

    for (tid = 1; tid < MEM_HASH_P4TBL_ID_MAX; tid++) {
        get_key_data_sizes_(tid, &ksize, &dsize);
        table_size = mem_hash_mock_table_sizes[tid].tsize;
        if (table_size == 0) {
            continue;
        }
        
        mocktables[tid].klist = calloc(table_size, ksize);
        mocktables[tid].dlist = calloc(table_size, dsize);

        assert(mocktables[tid].klist);
        assert(mocktables[tid].dlist);
    }
    return 0;
}

int
mem_hash_mock_cleanup ()
{
    uint32_t tid = 0;

    for (tid = 1; tid < MEM_HASH_P4TBL_ID_MAX; tid++) {
        free(mocktables[tid].klist);
        mocktables[tid].klist = NULL;

        free(mocktables[tid].dlist);
        mocktables[tid].dlist = NULL;
    }
    return 0;
}

uint32_t
mem_hash_mock_get_valid_count (uint32_t table_id)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < mem_hash_mock_table_sizes[table_id].tsize; i++) {
        switch (table_id) {
        case MEM_HASH_P4TBL_ID_H10:
        case MEM_HASH_P4TBL_ID_H10_OHASH:
        {
            mem_hash_h10_actiondata_t *dlist = (mem_hash_h10_actiondata_t *)(mocktables[table_id].dlist);
            count = count + dlist[i].action_u.info.entry_valid;
            break;
        }
        case MEM_HASH_P4TBL_ID_H5:
        case MEM_HASH_P4TBL_ID_H5_OHASH:
        {
            mem_hash_h5_actiondata_t *dlist = (mem_hash_h5_actiondata_t *)(mocktables[table_id].dlist);
            if (dlist[i].action_u.info.entry_valid) { 
                assert(dlist[i].action_u.info.entry_valid == 1);
            }
            count = count + dlist[i].action_u.info.entry_valid;
            break;
        }
        default:
            assert(0);
        }
    }
    return count;
}

int 
p4pd_entry_write (unsigned int table_id, unsigned int index, unsigned char *hwkey,
                  unsigned char *hwkey_y, void *actiondata)
{
    uint32_t ksize = 0;
    uint32_t dsize = 0;
    get_key_data_sizes_(table_id, &ksize, &dsize);
    
    assert(index < mem_hash_mock_table_sizes[table_id].tsize);
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_key_t *klist = (mem_hash_h10_key_t *)(mocktables[table_id].klist);
        mem_hash_h10_actiondata_t *dlist = (mem_hash_h10_actiondata_t *)(mocktables[table_id].dlist);
        memcpy(&klist[index], hwkey, ksize);
        memcpy(&dlist[index], actiondata, dsize);
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_key_t *klist = (mem_hash_h5_key_t *)(mocktables[table_id].klist);
        mem_hash_h5_actiondata_t *dlist = (mem_hash_h5_actiondata_t *)(mocktables[table_id].dlist);
        memcpy(&klist[index], hwkey, ksize);
        memcpy(&dlist[index], actiondata, dsize);
        break;
    }
    default:
        assert(0);
    }
    
    return 0;
}

int 
p4pd_entry_read(uint32_t table_id, uint32_t index, void *swkey,
                void *swkey_mask, void *actiondata)
{
    uint32_t ksize = 0;
    uint32_t dsize = 0;
    get_key_data_sizes_(table_id, &ksize, &dsize);

    assert(index < mem_hash_mock_table_sizes[table_id].tsize);
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_key_t *klist = (mem_hash_h10_key_t *)(mocktables[table_id].klist);
        mem_hash_h10_actiondata_t *dlist = (mem_hash_h10_actiondata_t *)(mocktables[table_id].dlist);
        memcpy(swkey, &klist[index], ksize);
        memcpy(actiondata, &dlist[index], dsize);
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_key_t *klist = (mem_hash_h5_key_t *)(mocktables[table_id].klist);
        mem_hash_h5_actiondata_t *dlist = (mem_hash_h5_actiondata_t *)(mocktables[table_id].dlist);
        memcpy(swkey, &klist[index], ksize);
        memcpy(actiondata, &dlist[index], dsize);
        break;
    }
    default:
        assert(0);
    }

    return 0;
}

p4pd_error_t
p4pd_hwkey_hwmask_build(uint32_t   tableid,
                        void       *swkey,
                        void       *swkey_mask,
                        uint8_t    *hw_key,
                        uint8_t    *hw_key_y)
{
    uint32_t ksize = 0;
    get_key_data_sizes_(tableid, &ksize, NULL);
    memcpy(hw_key, swkey, ksize);
    return 0;
}


p4pd_error_t
p4pd_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    uint32_t ksize = 0;
    uint32_t dsize = 0;
    get_key_data_sizes_(table_id, &ksize, &dsize);

    mem_hash_mock_table_size_t  *sizeinfo;
    assert(table_id && table_id < MEM_HASH_P4TBL_ID_MAX);

    memset(props, 0, sizeof(p4pd_table_properties_t));
    sizeinfo = &mem_hash_mock_table_sizes[table_id];

    props->tablename = (char *) "MemHashMainTable";
    props->key_struct_size = sizeinfo->ksize;
    props->actiondata_struct_size = sizeinfo->dsize;
    props->hash_type = 0;
    props->tabledepth = sizeinfo->tsize;
    
    if (table_id == MEM_HASH_P4TBL_ID_H5) {
        props->has_oflow_table = 1;
        props->oflow_table_id = MEM_HASH_P4TBL_ID_H5_OHASH;
    } else if (table_id == MEM_HASH_P4TBL_ID_H10) {
        props->has_oflow_table = 1;
        props->oflow_table_id = MEM_HASH_P4TBL_ID_H10_OHASH;
    }

    return 0;
}

// ===================== NEW FUNCTIONS =======================
#if 0
p4pd_error_t
p4pd_mem_hash_entry_set_action_id(uint32_t table_id,
                                  void *data,
                                  uint32_t action_id)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
        ((mem_hash_h10_actiondata_t *)data)->action_id = action_id;
        break;
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
        ((mem_hash_h5_actiondata_t *)data)->action_id = action_id;
        break;
    default:
        assert(0);
    }
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_entry_valid(uint32_t table_id,
                                    void *data,
                                    uint8_t entry_valid)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
        ((mem_hash_h10_actiondata_t *)data)->action_u.info.entry_valid = entry_valid;
        break;
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
        ((mem_hash_h5_actiondata_t *)data)->action_u.info.entry_valid = entry_valid;
        break;
    default:
        assert(0);
    }
    return 0;
}

uint8_t
p4pd_mem_hash_entry_get_entry_valid(uint32_t table_id,
                                    void *data)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
        return ((mem_hash_h10_actiondata_t *)data)->action_u.info.entry_valid;
        break;
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
        return ((mem_hash_h5_actiondata_t *)data)->action_u.info.entry_valid;
        break;
    default:
        assert(0);
    }
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_hint(uint32_t table_id,
                             void *data,
                             uint32_t hint,
                             uint32_t slot)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t *dst = (mem_hash_h10_actiondata_t *)data;
        if (slot == 1) { dst->action_u.info.hint1 = hint; } 
        else if (slot == 2) { dst->action_u.info.hint2 = hint; } 
        else if (slot == 3) { dst->action_u.info.hint3 = hint; }
        else if (slot == 4) { dst->action_u.info.hint4 = hint; }
        else if (slot == 5) { dst->action_u.info.hint5 = hint; }
        else if (slot == 6) { dst->action_u.info.hint6 = hint; }
        else if (slot == 7) { dst->action_u.info.hint7 = hint; }
        else if (slot == 8) { dst->action_u.info.hint8 = hint; }
        else if (slot == 9) { dst->action_u.info.hint9 = hint; }
        else if (slot == 10) { dst->action_u.info.hint10 = hint; }
        else { assert(0); }
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t *dst = (mem_hash_h5_actiondata_t *)data;
        if (slot == 1) { dst->action_u.info.hint1 = hint; }
        else if (slot == 2) { dst->action_u.info.hint2 = hint; }
        else if (slot == 3) { dst->action_u.info.hint3 = hint; }
        else if (slot == 4) { dst->action_u.info.hint4 = hint; }
        else if (slot == 5) { dst->action_u.info.hint5 = hint; }
        else { assert(0); }
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_hash5(uint32_t table_id,
                            void *data,
                             uint32_t hash10)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t   *dst = (mem_hash_h10_actiondata_t *)data;
        dst->action_u.info.hash10 = hash10;
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        break;
    }

    default:
        assert(0);
    }
    return 0;
}



p4pd_error_t
p4pd_mem_hash_entry_set_hash(uint32_t table_id,
                             void *data,
                             uint32_t hash,
                             uint32_t slot)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t   *dst = (mem_hash_h10_actiondata_t *)data;
        if (slot == 1) { dst->action_u.info.hash1 = hash; }
        else if (slot == 2) { dst->action_u.info.hash2 = hash; }
        else if (slot == 3) { dst->action_u.info.hash3 = hash; }
        else if (slot == 4) { dst->action_u.info.hash4 = hash; }
        else if (slot == 5) { dst->action_u.info.hash5 = hash; }
        else if (slot == 6) { dst->action_u.info.hash6 = hash; }
        else if (slot == 7) { dst->action_u.info.hash7 = hash; }
        else if (slot == 8) { dst->action_u.info.hash8 = hash; }
        else if (slot == 9) { dst->action_u.info.hash9 = hash; }
        else if (slot == 10) { dst->action_u.info.hash10 = hash; }
        else { assert(0); }
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t   *dst = (mem_hash_h5_actiondata_t *)data;
        if (slot == 1) { dst->action_u.info.hash1 = hash; }
        else if (slot == 2) { dst->action_u.info.hash2 = hash; }
        else if (slot == 3) { dst->action_u.info.hash3 = hash; }
        else if (slot == 4) { dst->action_u.info.hash4 = hash; }
        else if (slot == 5) { dst->action_u.info.hash5 = hash; }
        else { assert(0); }
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_more_hints(uint32_t table_id,
                                   void *data,
                                   uint8_t more_hints)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t *dst = (mem_hash_h10_actiondata_t *)data;
        dst->action_u.info.more_hints = more_hints;
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t *dst = (mem_hash_h5_actiondata_t *)data;
        dst->action_u.info.more_hints = more_hints;
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

uint32_t
p4pd_mem_hash_entry_get_more_hints(uint32_t table_id,
                                   void *data)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t *dst = (mem_hash_h10_actiondata_t *)data;
        return dst->action_u.info.more_hints;
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t *dst = (mem_hash_h5_actiondata_t *)data;
        return dst->action_u.info.more_hints;
        break;
    }
    default:
        assert(0);
    }
    return 0;
}



p4pd_error_t
p4pd_mem_hash_entry_set_more_hashs(uint32_t table_id,
                                   void *data,
                                   uint32_t more_hashs)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t *dst = (mem_hash_h10_actiondata_t *)data;
        dst->action_u.info.more_hashs = more_hashs;
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t *dst = (mem_hash_h5_actiondata_t *)data;
        dst->action_u.info.more_hashs = more_hashs;
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

uint8_t
p4pd_mem_hash_entry_get_more_hashs(uint32_t table_id,
                                   void *data)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t *dst = (mem_hash_h10_actiondata_t *)data;
        return dst->action_u.info.more_hashs;
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t *dst = (mem_hash_h5_actiondata_t *)data;
        return dst->action_u.info.more_hashs;
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

uint32_t
p4pd_mem_hash_entry_get_hash(uint32_t table_id,
                             void *data,
                             uint32_t slot)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t   *dst = (mem_hash_h10_actiondata_t *)data;
        if (slot == 1) { return dst->action_u.info.hash1; }
        else if (slot == 2) { return dst->action_u.info.hash2; }
        else if (slot == 3) { return dst->action_u.info.hash3; }
        else if (slot == 4) { return dst->action_u.info.hash4; }
        else if (slot == 5) { return dst->action_u.info.hash5; }
        else if (slot == 6) { return dst->action_u.info.hash6; }
        else if (slot == 7) { return dst->action_u.info.hash7; }
        else if (slot == 8) { return dst->action_u.info.hash8; }
        else if (slot == 9) { return dst->action_u.info.hash9; }
        else if (slot == 10) { return dst->action_u.info.hash10; }
        else { assert(0); }
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t   *dst = (mem_hash_h5_actiondata_t *)data;
        if (slot == 1) { return dst->action_u.info.hash1; }
        else if (slot == 2) { return dst->action_u.info.hash2; }
        else if (slot == 3) { return dst->action_u.info.hash3; }
        else if (slot == 4) { return dst->action_u.info.hash4; }
        else if (slot == 5) { return dst->action_u.info.hash5; }
        else { assert(0); }
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

uint32_t
p4pd_mem_hash_entry_get_hint(uint32_t table_id,
                             void *data,
                             uint32_t slot)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
    {
        mem_hash_h10_actiondata_t   *dst = (mem_hash_h10_actiondata_t *)data;
        if (slot == 1) { return dst->action_u.info.hint1; }
        else if (slot == 2) { return dst->action_u.info.hint2; }
        else if (slot == 3) { return dst->action_u.info.hint3; }
        else if (slot == 4) { return dst->action_u.info.hint4; }
        else if (slot == 5) { return dst->action_u.info.hint5; }
        else if (slot == 6) { return dst->action_u.info.hint6; }
        else if (slot == 7) { return dst->action_u.info.hint7; }
        else if (slot == 8) { return dst->action_u.info.hint8; }
        else if (slot == 9) { return dst->action_u.info.hint9; }
        else if (slot == 10) { return dst->action_u.info.hint10; }
        else { assert(0); }
        break;
    }
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
    {
        mem_hash_h5_actiondata_t   *dst = (mem_hash_h5_actiondata_t *)data;
        if (slot == 1) { return dst->action_u.info.hint1; }
        else if (slot == 2) { return dst->action_u.info.hint2; }
        else if (slot == 3) { return dst->action_u.info.hint3; }
        else if (slot == 4) { return dst->action_u.info.hint4; }
        else if (slot == 5) { return dst->action_u.info.hint5; }
        else { assert(0); }
        break;
    }
    default:
        assert(0);
    }
    return 0;
}

uint8_t
p4pd_mem_hash_entry_get_num_hints(uint32_t table_id)
{
    switch (table_id) {
    case MEM_HASH_P4TBL_ID_H10:
    case MEM_HASH_P4TBL_ID_H10_OHASH:
        return 10;
    case MEM_HASH_P4TBL_ID_H5:
    case MEM_HASH_P4TBL_ID_H5_OHASH:
        return 5;
    default:
        assert(0);
    }
    return 0;
}
#endif

p4pd_error_t
p4pd_actiondata_appdata_set(uint32_t   tableid,
                            uint8_t    actionid,
                            void       *appdata,
                            void       *actiondata)
{
    switch (tableid) {
        case MEM_HASH_P4TBL_ID_H5:
        case MEM_HASH_P4TBL_ID_H5_OHASH:
            switch (actionid) {
                case MEM_HASH_P4TBL_ID_H5_ACTION_ID_0:
                ((mem_hash_h5_actiondata_t*)actiondata)->action_u.info.d1 = ((mem_hash_h5_appdata_t*)appdata)->d1;
                ((mem_hash_h5_actiondata_t*)actiondata)->action_u.info.d2 = ((mem_hash_h5_appdata_t*)appdata)->d2;
                break;
                default:
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
        break;
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

p4pd_error_t
p4pd_actiondata_appdata_get(uint32_t   tableid,
                            uint8_t    actionid,
                            void       *appdata,
                            void       *actiondata)
{
    switch (tableid) {
        case MEM_HASH_P4TBL_ID_H5:
        case MEM_HASH_P4TBL_ID_H5_OHASH:
            switch (actionid) {
                case MEM_HASH_P4TBL_ID_H5_ACTION_ID_0:
                ((mem_hash_h5_appdata_t*)appdata)->d1 = ((mem_hash_h5_actiondata_t*)actiondata)->action_u.info.d1;
                ((mem_hash_h5_appdata_t*)appdata)->d2 = ((mem_hash_h5_actiondata_t*)actiondata)->action_u.info.d2;
                break;
                default:
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
        break;
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

uint32_t
p4pd_actiondata_appdata_size_get(uint32_t   tableid,
                                 uint8_t    actionid)
{
    switch (tableid) {
        case MEM_HASH_P4TBL_ID_H5:
        case MEM_HASH_P4TBL_ID_H5_OHASH:
            switch (actionid) {
                case MEM_HASH_P4TBL_ID_H5_ACTION_ID_0:
                return sizeof(mem_hash_h5_appdata_t);
                break;
                default:
                    assert(0);
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
        break;
        default:
            assert(0);
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    assert(0);
    return (0);
}

p4pd_error_t
p4pd_actiondata_hwfield_set(uint32_t   tableid,
                            uint8_t    actionid,
                            uint32_t   argument_slotid,
                            uint8_t    *argumentvalue,
                            void       *actiondata)
{
    switch (tableid) {
        case MEM_HASH_P4TBL_ID_H5:
        case MEM_HASH_P4TBL_ID_H5_OHASH:
            ((mem_hash_h5_actiondata_t*)actiondata)->action_id = actionid;
            switch (actionid) {
                case MEM_HASH_P4TBL_ID_H5_ACTION_ID_0:
                    {
                        if (argument_slotid >= 13) {
                            assert(0);
                            return (P4PD_FAIL);
                        }
                        uint32_t argument_offsets[] = {
                            offsetof(mem_hash_h5_info_t, entry_valid),
                            offsetof(mem_hash_h5_info_t, hash1),
                            offsetof(mem_hash_h5_info_t, hint1),
                            offsetof(mem_hash_h5_info_t, hash2),
                            offsetof(mem_hash_h5_info_t, hint2),
                            offsetof(mem_hash_h5_info_t, hash3),
                            offsetof(mem_hash_h5_info_t, hint3),
                            offsetof(mem_hash_h5_info_t, hash4),
                            offsetof(mem_hash_h5_info_t, hint4),
                            offsetof(mem_hash_h5_info_t, hash5),
                            offsetof(mem_hash_h5_info_t, hint5),
                            offsetof(mem_hash_h5_info_t, more_hashs),
                            offsetof(mem_hash_h5_info_t, more_hints),
                        };
                        uint32_t argument_byte_width[] = {
                            sizeof(uint8_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint8_t),
                            sizeof(uint32_t),
                        };

                        uint8_t* structbase = (uint8_t*)&(((mem_hash_h5_actiondata_t*)actiondata)->action_u);
                        memcpy((structbase + argument_offsets[argument_slotid]),
                                argumentvalue, argument_byte_width[argument_slotid]);
                    }
                break;
                default:
                    // Invalid action
                    assert(0);
                    return (P4PD_FAIL);
                break;
            }
            break;
        default:
            // Invalid tableid
            assert(0);
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

p4pd_error_t
p4pd_actiondata_hwfield_get(uint32_t   tableid,
                            uint8_t    actionid,
                            uint32_t   argument_slotid,
                            uint8_t    *argumentvalue,
                            void       *actiondata)
{
    switch (tableid) {
        case MEM_HASH_P4TBL_ID_H5:
        case MEM_HASH_P4TBL_ID_H5_OHASH:
            switch (actionid) {
                case MEM_HASH_P4TBL_ID_H5_ACTION_ID_0:
                    {
                        if (argument_slotid >= 13) {
                            assert(0);
                            return (P4PD_FAIL);
                        }
                        uint32_t argument_offsets[] = {
                            offsetof(mem_hash_h5_info_t, entry_valid),
                            offsetof(mem_hash_h5_info_t, hash1),
                            offsetof(mem_hash_h5_info_t, hint1),
                            offsetof(mem_hash_h5_info_t, hash2),
                            offsetof(mem_hash_h5_info_t, hint2),
                            offsetof(mem_hash_h5_info_t, hash3),
                            offsetof(mem_hash_h5_info_t, hint3),
                            offsetof(mem_hash_h5_info_t, hash4),
                            offsetof(mem_hash_h5_info_t, hint4),
                            offsetof(mem_hash_h5_info_t, hash5),
                            offsetof(mem_hash_h5_info_t, hint5),
                            offsetof(mem_hash_h5_info_t, more_hashs),
                            offsetof(mem_hash_h5_info_t, more_hints),
                        };
                        uint32_t argument_byte_width[] = {
                            sizeof(uint8_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint16_t),
                            sizeof(uint32_t),
                            sizeof(uint8_t),
                            sizeof(uint32_t),
                        };
                        uint8_t* structbase = (uint8_t*)&(((mem_hash_h5_actiondata_t*)actiondata)->action_u);
                        memcpy(argumentvalue, (structbase + argument_offsets[argument_slotid]),
                               argument_byte_width[argument_slotid]);
                    }
                break;
                default:
                    // Invalid action
                    assert(0);
                    return (P4PD_FAIL);
                break;
            }
            break;
        default:
            // Invalid tableid
            assert(0);
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}





