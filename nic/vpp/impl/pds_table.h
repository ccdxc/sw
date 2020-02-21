//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
// Hardware table access interface

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int pds_table_read(uint32_t table_id, uint32_t key, void *action);
int pds_table_write(uint32_t table_id, uint32_t key, void *action);

#ifdef __cplusplus
}
#endif

