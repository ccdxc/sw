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

struct counter_ {
    std::string name;
    bool is_pointer;
};

typedef struct metrics_table_ {
    std::string                 name;
    TableMgrUptr                tbl;
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
    unsigned int counter_count;
    char name[];
} serialized_spec_t;

typedef struct serialized_counter_spec_ {
    unsigned int name_length;
    metrics_counter_type_t type;
    char name[];
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
save_schema_ (metrics_table_t *tbl, metrics_schema_t *schema)
{
    metrics_counter_t *cntr_props;
    serialized_spec_t *srlzd;
    int16_t srlzd_len;
    int i;

    // todo
    // check if it already exists and crash if mismatch
    
    for (i = 0; (cntr_props = &schema->counters[i]), cntr_props->name != NULL; i++) {
        std::string key = tbl->name + ":" + std::to_string(i);
        serialized_counter_spec_t *srlzd_counter;
        int16_t srlzd_counter_len =  sizeof(serialized_counter_spec_t *) +
            strlen(cntr_props->name) + 1;

        srlzd_counter = (serialized_counter_spec_t *)malloc(srlzd_counter_len);
        srlzd_counter->name_length = strlen(cntr_props->name);
        memcpy(srlzd_counter->name, cntr_props->name,
               srlzd_counter->name_length + 1); 
        srlzd_counter->type = cntr_props->type;

        error err = get_meta_table()->Publish(
            key.c_str(), key.size(), (char *)srlzd_counter, srlzd_counter_len);
        assert(err == error::OK());

        free(srlzd_counter);

        tbl->counters.push_back({
            name: cntr_props->name,
            is_pointer: cntr_props->type == METRICS_COUNTER_POINTER64,
            });
    }

    srlzd_len = sizeof(srlzd) + strlen(schema->name) + 1;
    srlzd = (serialized_spec_t *)malloc(srlzd_len);
    srlzd->name_length = strlen(schema->name);
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

    for (unsigned int i = 0; i < srlzd->counter_count; i++) {
        serialized_counter_spec_t *srlzd_counter;
        std::string key = tbl->name + ":" + std::to_string(i);

        srlzd_counter = (serialized_counter_spec_t *)get_meta_table()->Find(
            key.c_str(), key.size());
        assert(srlzd_counter != NULL);
        
        tbl->counters.push_back({
            name: srlzd_counter->name,
            is_pointer: srlzd_counter->type == METRICS_COUNTER_POINTER64,
            });
    }

    return tbl;
}

void *
metrics_register (metrics_schema_t *schema)
{
    metrics_table_t *tbl = new metrics_table_t();
   
    tbl->tbl = get_shm()->Kvstore()->Table(schema->name);
    if (tbl->tbl == nullptr) {
        tbl->tbl = get_shm()->Kvstore()->CreateTable(
            schema->name, SDK_METRICS_DEFAULT_TBL_SIZE);
        assert(tbl->tbl != nullptr);
        tbl->name = schema->name;
        save_schema_(tbl, schema);
    }

    return tbl;
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

metrics_counters_t
metrics_read (void  *handler, key_t key)
{
    metrics_table_t *tbl;
    metrics_counters_t counters;

    tbl = (metrics_table_t *)handler;

    for (unsigned int i = 0; i < tbl->counters.size(); i++) {
        metrics_counter_pair_t pair;

        pair.first = tbl->counters[i].name;
        pair.second = metrics_read(handler, key, i);
        counters.push_back(pair);
    }
    
    return counters;
}

uint64_t
metrics_read (void *handler, key_t key, unsigned int counter)
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

    if (tbl->counters[counter].is_pointer) {
        uint64_t data = 0;
        sdk::lib::pal_ret_t rc;
        rc = sdk::lib::pal_reg_read(*value, (uint32_t *)&data, 2);
        assert(rc == sdk::lib::PAL_RET_OK);
        return data;
    } else {
        return *value;
    }
}

} // namespace sdk
} // namespace metrics
