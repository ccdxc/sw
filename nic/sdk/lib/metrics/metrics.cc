// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "htable.hpp"
#include "kvstore.hpp"
#include "metrics.hpp"
#include "lib/pal/pal.hpp"
#include "shm.hpp"

namespace sdk {
namespace metrics {

static const int SDK_METRICS_DEFAULT_TBL_SIZE = 1000;

typedef enum metrics_counter_type_ {
    METRICS_COUNTER_VALUE64   = 1, // A 64bit value
    METRICS_COUNTER_POINTER64 = 2, // A pointer to a 64bit value
    METRICS_COUNTER_RSVD64    = 3, // A reserved counter (to be ignored)
} metrics_counter_type_t;

typedef struct metrics_counter_ {
    const char *name; // Name of the metric. e.g. InboundPackets
    metrics_counter_type_t type;
} metrics_counter_t;

struct counter_ {
    std::string name;
    metrics_counter_type_t type;
};

typedef struct metrics_table_ {
    std::string name;
    TableMgrUptr tbl;
    metrics_type_t type;
    std::vector<struct counter_> counters;
} metrics_table_t;

typedef struct internal_key_ {
    key_t key;
    unsigned int counter;
} __attribute__((packed)) internal_key_t;

typedef struct buffer_ {
    int length;
    char data[];
} __attribute__((packed)) buffer_t;

typedef struct serialized_spec_ {
    unsigned int name_length;
    metrics_type_t type;
    unsigned int counter_count;
    char         name[];
} serialized_spec_t;

typedef struct serialized_counter_spec_ {
    unsigned int           name_length;
    metrics_counter_type_t type;
    char                   name[];
} serialized_counter_spec_t;

static ShmPtr
get_shm (void)
{
    static ShmPtr shm = nullptr;

    if (shm == nullptr) {
        shm = std::make_shared<Shm>();
        error err = shm->MemMap(SDK_METRICS_SHM_NAME, SDK_METRICS_SHM_SIZE);
        assert(err == error::OK());
    }

    return shm;
}

static TableMgrUptr
get_meta_table (void)
{ 
    TableMgrUptr tbl = nullptr;

    if (tbl == nullptr) {
        tbl = get_shm()->Kvstore()->Table("__metrics_meta__");
        if (tbl == nullptr) {
            tbl = get_shm()->Kvstore()->CreateTable(
                "__metrics_meta__", SDK_METRICS_DEFAULT_TBL_SIZE);
            assert(tbl != nullptr);
        }
    }

    return tbl;
}

static void
save_schema_ (metrics_table_t *tbl, schema_t *schema)
{
    const char *cntr_name;
    serialized_spec_t *srlzd;
    int16_t srlzd_len;
    int i;

    // todo
    // check if it already exists and crash if mismatch
    
    for (i = 0; (cntr_name = schema->counters[i]), cntr_name != NULL; i++) {
        std::string key = tbl->name + ":" + std::to_string(i);
        serialized_counter_spec_t *srlzd_counter;
        int16_t srlzd_counter_len =  sizeof(serialized_counter_spec_t *) +
            strlen(cntr_name) + 1;
        metrics_counter_type_t type;

        if (tbl->type == SW) {
            type = METRICS_COUNTER_VALUE64; 
        } else {
            if (cntr_name[0] == '_') {
                type = METRICS_COUNTER_RSVD64;
            } else {
                type = METRICS_COUNTER_POINTER64;
            }
        }

        srlzd_counter = (serialized_counter_spec_t *)malloc(srlzd_counter_len);
        srlzd_counter->name_length = strlen(cntr_name);
        memcpy(srlzd_counter->name, cntr_name,
               srlzd_counter->name_length + 1); 
        srlzd_counter->type = type;

        error err = get_meta_table()->Publish(
            key.c_str(), key.size(), (char *)srlzd_counter, srlzd_counter_len);
        assert(err == error::OK());

        free(srlzd_counter);

        tbl->counters.push_back({
            name: cntr_name,
            type: type,
            });
    }

    srlzd_len = sizeof(srlzd) + strlen(schema->name) + 1;
    srlzd = (serialized_spec_t *)malloc(srlzd_len);
    srlzd->name_length = strlen(schema->name);
    srlzd->type = tbl->type;
    memcpy(srlzd->name, schema->name, srlzd->name_length + 1);
    srlzd->counter_count = i;

    error err = get_meta_table()->Publish(
        tbl->name.c_str(), tbl->name.size(), (char *)srlzd, srlzd_len);
    assert(err == error::OK());

    free(srlzd);
}

static metrics_table_t *
load_table_ (const char *name)
{
    metrics_table_t *tbl;
    serialized_spec_t *srlzd;

    tbl = new metrics_table_t();
    tbl->name = name;
    
    srlzd = (serialized_spec_t *)get_meta_table()->Find(
        tbl->name.c_str(), tbl->name.size());
    assert(srlzd != NULL);
    tbl->type = srlzd->type;

    for (unsigned int i = 0; i < srlzd->counter_count; i++) {
        serialized_counter_spec_t *srlzd_counter;
        std::string key = tbl->name + ":" + std::to_string(i);

        srlzd_counter = (serialized_counter_spec_t *)get_meta_table()->Find(
            key.c_str(), key.size());
        assert(srlzd_counter != NULL);
        
        tbl->counters.push_back({
            name: srlzd_counter->name,
            type: srlzd_counter->type,
            });
    }

    return tbl;
}

void *
create (schema_t *schema)
{
    metrics_table_t *tbl;
    TableMgrUptr tbmgr;

  
    tbmgr = get_shm()->Kvstore()->Table(schema->name);
    if (tbmgr == nullptr) {
        tbl = new metrics_table_t();
        tbl->tbl = get_shm()->Kvstore()->CreateTable(
            schema->name, SDK_METRICS_DEFAULT_TBL_SIZE);
        assert(tbl->tbl != nullptr);
        tbl->name = schema->name;
        tbl->type = schema->type;
        save_schema_(tbl, schema);
    } else {
        tbl = load_table_(schema->name);
        tbl->tbl = std::move(tbmgr);
    }

    return tbl;
}

void
row_address(void *handler, key_t key, void *address) {
    metrics_table_t *tbl = (metrics_table_t *)handler;
    internal_key_t ikey = {
        .key = key,
        .counter = 0,
    };

    assert(tbl->type == HBM);

    error err = tbl->tbl->Publish((char *)&ikey, sizeof(ikey), (char *)&address,
                                  sizeof(address));

    assert(err == error::OK());
}

void
metrics_update (void *handler, key_t key, unsigned int counter, uint64_t value)
{
    metrics_table_t *tbl = (metrics_table_t *)handler;
    internal_key_t ikey = {
        .key = key,
        .counter = counter,
    };
    error err = tbl->tbl->Publish((char *)&ikey, sizeof(ikey), (char *)&value,
                                  sizeof(value));
    assert(err == error::OK());
}

void *
metrics_open (const char *name)
{
    metrics_table_t *tbl = load_table_(name);
    
    tbl->tbl = get_shm()->Kvstore()->Table(name);
    if (tbl->tbl == nullptr) {
        delete tbl;
        return NULL;
    }

    return tbl;
}

static uint64_t
metrics_read_value (void *handler, key_t key, unsigned int counter)
{
    metrics_table_t *tbl;
    uint64_t *value;
    internal_key_t ikey = {
        .key = key,
        .counter = counter,
    };

    tbl = (metrics_table_t *)handler;

    assert(tbl != NULL && counter >= 0 && counter < tbl->counters.size());

    value = (uint64_t *)tbl->tbl->Find((char *)&ikey, sizeof(ikey));
    if (value == NULL) {
        return 0;
    }

    return *value;
}

static metrics_counters_t
metrics_read_values (void  *handler, key_t key)
{
    metrics_table_t *tbl;
    metrics_counters_t counters;

    tbl = (metrics_table_t *)handler;

    for (unsigned int i = 0; i < tbl->counters.size(); i++) {
        metrics_counter_pair_t pair;

        pair.first = tbl->counters[i].name;
        pair.second = metrics_read_value(handler, key, i);
        counters.push_back(pair);
    }
    
    return counters;
}

static uint64_t
read_value (uint64_t base, unsigned int offset)
{
    uint64_t value;
    int rc;
    
    rc = sdk::lib::pal_reg_read(base + (offset * sizeof(value)),
                                (uint32_t *)&value, 2);
    assert(rc == sdk::lib::PAL_RET_OK);

    return value;
}

static metrics_counters_t
metrics_read_pointers (void  *handler, key_t key)
{
    metrics_table_t *tbl;
    metrics_counters_t counters;
    uint64_t *base;
    internal_key_t ikey = {
        .key = key,
        .counter = 0,
    };

    tbl = (metrics_table_t *)handler;

    base = (uint64_t *)tbl->tbl->Find((char *)&ikey, sizeof(ikey));
    if (base == NULL) {
        return counters;
    }
    
    for (unsigned int i = 0; i < tbl->counters.size(); i++) {
        if (tbl->counters[i].type != METRICS_COUNTER_RSVD64) {
            metrics_counter_pair_t pair;

            pair.first = tbl->counters[i].name;
            pair.second = read_value(*base, i);
            counters.push_back(pair);
        }
    }
    
    return counters;
}

metrics_counters_t
metrics_read (void  *handler, key_t key)
{
    metrics_table_t *tbl;

    tbl = (metrics_table_t *)handler;

    if (tbl->type == SW) {
        return metrics_read_values(handler, key);
    } else {
        return metrics_read_pointers(handler, key);
    }
}

} // namespace sdk
} // namespace metrics
