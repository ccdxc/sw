//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifdef __cplusplus
extern "C" {
#endif

void set_skip_session_program(int val);

int initialize_session(void);

int session_program(uint32_t ses_id, void *action);

void session_insert(uint32_t ses_id, void *ses_info);

void session_get_addr(uint32_t ses_id, uint8_t **ses_addr,
                      uint32_t *entry_size);

uint64_t pds_session_get_timestamp(uint32_t session_id);

#ifdef __cplusplus
}
#endif
