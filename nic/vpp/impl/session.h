//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_IMPL_SESSION_H__
#define __VPP_IMPL_SESSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct session_info_s {
    uint64_t timestamp;
    uint32_t iflow_tcp_seq_num;
    uint32_t iflow_tcp_ack_num;
    uint16_t iflow_tcp_win_size;
    uint32_t rflow_tcp_seq_num;
    uint32_t rflow_tcp_ack_num;
    uint16_t rflow_tcp_win_size;
} session_info_t;

void set_skip_session_program(int val);

int initialize_session(void);

int session_track_program(uint32_t ses_id, void *action);

void session_insert(uint32_t ses_id, void *ses_info);

void session_get_addr(uint32_t ses_id, uint8_t **ses_addr,
                      uint32_t *entry_size);

int pds_session_program(uint32_t ses_id, void *action);

uint64_t pds_session_get_timestamp(uint32_t session_id);

void pds_session_get_session_state(uint32_t session_id, uint8_t *iflow_state, 
                                   uint8_t *rflow_state);

bool pds_session_state_established(uint8_t state);

bool pds_session_active_on_vnic_get(uint16_t vnic_id, uint32_t *sess_count);

int session_track_program(uint32_t ses_id, void *action);

void pds_session_get_info(uint32_t session_id, session_info_t *session_info);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_IMPL_SESSION_H__
