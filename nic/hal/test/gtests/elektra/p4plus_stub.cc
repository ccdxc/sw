#include <inttypes.h>
#include "nic/hal/pd/hal_pd_error.hpp"

void p4pd_txdma_hwentry_query(uint32_t tableid, uint32_t *hwkey_len,
    uint32_t *hwkeymask_len, uint32_t *hwactiondata_len) {}

p4pd_error_t
p4pd_txdma_entry_write_with_datamask(uint32_t tableid,
    uint32_t index, uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata,
    void *actiondata_mask) {
    return 0;
}

p4pd_error_t
p4pd_txdma_entry_write(uint32_t tableid, uint32_t index,
    uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata) {
    return 0;
}

p4pd_error_t
p4pd_txdma_entry_read(uint32_t tableid, uint32_t index,
    void *swkey, void *swkey_mask, void *actiondata) {
    return 0;
}

p4pd_error_t
p4pd_txdma_table_entry_decoded_string_get(uint32_t tableid,
    uint32_t index, uint8_t* hwentry, uint8_t* hwentry_y,
    uint16_t hwentry_len, char* buffer, uint16_t buf_len) {
    return 0;
}

p4pd_error_t
p4pd_txdma_table_ds_decoded_string_get(uint32_t tableid,
    uint32_t index, void* sw_key, void* sw_key_mask, void* action_data,
    char* buffer, uint16_t buf_len) {
    return 0;
}

void p4pd_txdma_prep_p4tbl_sw_struct_sizes(void) {}
void p4pd_txdma_prep_p4tbl_names(void) {}
int p4pd_txdma_get_max_action_id(uint32_t tableid) { return 0; }
void p4pd_txdma_get_action_name(uint32_t tableid, int actionid,
                                char *action_name) {}

char p4pd_txdma_tbl_names[1][1];
uint16_t p4pd_txdma_tbl_swkey_size[1];
uint16_t p4pd_txdma_tbl_sw_action_data_size[1];

uint32_t p4pd_txdma_tableid_min_get() { return 0; }
uint32_t p4pd_txdma_tableid_max_get() { return 0; }


void p4pd_rxdma_hwentry_query(uint32_t tableid, uint32_t *hwkey_len,
    uint32_t *hwkeymask_len, uint32_t *hwactiondata_len) {}

p4pd_error_t
p4pd_rxdma_entry_write_with_datamask(uint32_t tableid,
    uint32_t index, uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata,
    void *actiondata_mask) {
    return 0;
}

p4pd_error_t
p4pd_rxdma_entry_write(uint32_t tableid, uint32_t index,
    uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata) {
    return 0;
}

p4pd_error_t
p4pd_rxdma_entry_read(uint32_t tableid, uint32_t index,
    void *swkey, void *swkey_mask, void *actiondata) {
    return 0;
}

p4pd_error_t
p4pd_rxdma_table_entry_decoded_string_get(uint32_t tableid,
    uint32_t index, uint8_t* hwentry, uint8_t* hwentry_y,
    uint16_t hwentry_len, char* buffer, uint16_t buf_len) {
    return 0;
}

p4pd_error_t
p4pd_rxdma_table_ds_decoded_string_get(uint32_t tableid,
    uint32_t index, void* sw_key, void* sw_key_mask, void* action_data,
    char* buffer, uint16_t buf_len) {
    return 0;
}

void p4pd_rxdma_prep_p4tbl_sw_struct_sizes(void) {}
void p4pd_rxdma_prep_p4tbl_names(void) {}
int p4pd_rxdma_get_max_action_id(uint32_t tableid) { return 0; }
void p4pd_rxdma_get_action_name(uint32_t tableid, int actionid,
                                char *action_name) {}

char p4pd_rxdma_tbl_names[1][1];
uint16_t p4pd_rxdma_tbl_swkey_size[1];
uint16_t p4pd_rxdma_tbl_sw_action_data_size[1];

uint32_t p4pd_rxdma_tableid_min_get() { return 0; }
uint32_t p4pd_rxdma_tableid_max_get() { return 0; }
