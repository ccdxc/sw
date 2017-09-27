



#ifndef __P4PD_H__
#define __P4PD_H__


#include <string.h>
#include "nic/hal/pd/hal_pd_error.hpp"

/*
 * p4pd.h
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains data structures and APIs needed to operate on each 
 * P4 Table. 
 *
 * For every P4table, 
 *   1. A "C" structure to build match key is provided. This "C" structure 
 *      aka software key (swkey) is used by asic-library to work with p4-tables.
 *      This table key structure is not same as what is represented in hardware.
 *      Hardware representation is kept transparent to application or users of
 *      P4-table by providing necessary function to build hardware key from 
 *      swkey.
 *
 *   2. A list of action identifiers associated with table. This list is 
 *      provided as enumertion values. Action ID has to be used in order to 
 *      associate action with match-key.
 *
 *   3. A union of structures where each structure is built using action 
 *      parameters associated with an action. Each structure corresponds to
 *      one of the many actions associated with the match table.
 *
 *   4. In order to keep application layer agnostic of hardware representation 
 *      folllowing APIs are provided.
 *      5.a  <table_name>_hwkey_query() which returns hwkey length, 
 *           hwactiondata length associated with the key. These lengths
 *           can be used to allocate memory before using p4 table operation
 *           APIs. All table operation APIs expect caller of the APIs to
 *           provide necessary memory for table operation API arguments.
 *      5.b  <table_name>_hwkey_build() which converts table key from software
 *           representation to hardware representation.
 *
 *   5. In order to install an entry into P4 table or read an entry from p4-table
 *      following APIs are provided
 *      <table_name>_entry_write()
 *      <table_name>_entry_read()
 */

/*
 * This file groups data structure and APIs to operate on p4 table on each
 * P4-table basis.
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */




/* Software Key Structure for p4-table 'D_mpu_only' */
/* P4-table 'D_mpu_only' Mpu/Keyless table.*/


typedef struct __attribute__((__packed__)) D_mpu_only_swkey {
} D_mpu_only_swkey_t;



typedef enum D_mpu_only_actions_enum {
    D_MPU_ONLY_MPU_ONLY_ACTION_ID = 0,
    D_MPU_ONLY_MAX_ID = 1
} D_mpu_only_actions_en;

typedef struct __attribute__((__packed__)) __D_mpu_only_actiondata {
    uint8_t actionid;
    union {
    } D_mpu_only_action_u;
} D_mpu_only_actiondata;


/* Software Key Structure for p4-table 'C_idx' */
/* P4-table 'C_idx' is index table */


typedef enum C_idx_actions_enum {
    C_IDX_UPDATE_TTL_ID = 0,
    C_IDX_MAX_ID = 1
} C_idx_actions_en;

typedef struct __attribute__((__packed__)) __C_idx_actiondata {
    uint8_t actionid;
    union {
        struct __attribute__((__packed__)) __C_idx_update_ttl {
            uint8_t ttl_val;
        } C_idx_update_ttl;
    } C_idx_action_u;
} C_idx_actiondata;


/* Software Key Structure for p4-table 'A_hash' */
/* P4-table 'A_hash' is hash table */


typedef struct __attribute__((__packed__)) A_hash_swkey {
    uint8_t ethernet_dstAddr[6];
    uint16_t udp_dstPort;
} A_hash_swkey_t;



typedef enum A_hash_actions_enum {
    A_HASH_NOP_ID = 0,
    A_HASH_MAX_ID = 1
} A_hash_actions_en;

typedef struct __attribute__((__packed__)) __A_hash_actiondata {
    uint8_t actionid;
    union {
    } A_hash_action_u;
} A_hash_actiondata;


/* Software Key Structure for p4-table 'B_tcam' */
/* P4-table 'B_tcam' ternary table.*/


typedef struct __attribute__((__packed__)) B_tcam_swkey {
    uint8_t ipv4_valid;
    uint8_t ipv6_valid;
    uint8_t ipv6_dstAddr[16];
    uint32_t ipv4_dstAddr;
} B_tcam_swkey_t;


typedef struct __attribute__((__packed__)) B_tcam_swkey_mask {
    uint8_t ipv4_valid_mask;
    uint8_t ipv6_valid_mask;
    uint8_t ipv6_dstAddr_mask[16];
    uint32_t ipv4_dstAddr_mask;
} B_tcam_swkey_mask_t;

typedef enum B_tcam_actions_enum {
    B_TCAM_UPDATE_IPV4_ID = 0,
    B_TCAM_UPDATE_IPV6_ID = 1,
    B_TCAM_MAX_ID = 2
} B_tcam_actions_en;

typedef struct __attribute__((__packed__)) __B_tcam_actiondata {
    uint8_t actionid;
    union {
    } B_tcam_action_u;
} B_tcam_actiondata;



/* List of Table Ids assigned for every p4 table.
 * This table id is needed to operate on any P4-table
 * using APIs
 *   p4pd_hwentry_query()
 *   p4pd_hwkey_hwmask_build()
 *   p4pd_entry_write()
 *   p4pd_entry_read()
 */
typedef enum p4pd_table_ids_ {
    P4TBL_ID_HASH_MIN = 1,
    P4TBL_ID_HASH_MAX = 1,
    P4TBL_ID_HASH_OTCAM_MIN = 2,
    P4TBL_ID_HASH_OTCAM_MAX = 1,
    P4TBL_ID_TCAM_MIN = 2,
    P4TBL_ID_TCAM_MAX = 2,
    P4TBL_ID_INDEX_MIN = 3,
    P4TBL_ID_INDEX_MAX = 3,
    P4TBL_ID_MPU_MIN = 4,
    P4TBL_ID_MPU_MAX = 4,
    P4TBL_ID_TBLMIN = 1,
    P4TBL_ID_A_HASH = 1,
    P4TBL_ID_B_TCAM = 2,
    P4TBL_ID_C_IDX = 3,
    P4TBL_ID_D_MPU_ONLY = 4,
    P4TBL_ID_TBLMAX = 5
} p4pd_table_id;


#define P4TBL_NAME_MAX_LEN 80 /* p4 table name will be truncated to 80 characters */

extern char p4pd_tbl_names[P4TBL_ID_TBLMAX][P4TBL_NAME_MAX_LEN];
extern uint16_t p4pd_tbl_swkey_size[P4TBL_ID_TBLMAX];
extern uint16_t p4pd_tbl_sw_action_data_size[P4TBL_ID_TBLMAX];

inline void p4pd_prep_p4tbl_names()
{
    strncpy(p4pd_tbl_names[P4TBL_ID_A_HASH], "A_hash", strlen("A_hash"));
    strncpy(p4pd_tbl_names[P4TBL_ID_B_TCAM], "B_tcam", strlen("B_tcam"));
    strncpy(p4pd_tbl_names[P4TBL_ID_C_IDX], "C_idx", strlen("C_idx"));
    strncpy(p4pd_tbl_names[P4TBL_ID_D_MPU_ONLY], "D_mpu_only", strlen("D_mpu_only"));
}

inline void p4pd_prep_p4tbl_sw_struct_sizes()
{
    p4pd_tbl_swkey_size[P4TBL_ID_A_HASH] = sizeof(A_hash_swkey);
    p4pd_tbl_sw_action_data_size[P4TBL_ID_A_HASH]= sizeof(A_hash_actiondata);
    p4pd_tbl_swkey_size[P4TBL_ID_B_TCAM] = sizeof(B_tcam_swkey);
    p4pd_tbl_sw_action_data_size[P4TBL_ID_B_TCAM]= sizeof(B_tcam_actiondata);
    p4pd_tbl_sw_action_data_size[P4TBL_ID_C_IDX]= sizeof(C_idx_actiondata);
    p4pd_tbl_swkey_size[P4TBL_ID_D_MPU_ONLY] = sizeof(D_mpu_only_swkey);
    p4pd_tbl_sw_action_data_size[P4TBL_ID_D_MPU_ONLY]= sizeof(D_mpu_only_actiondata);
}


inline int p4pd_get_max_action_id(uint32_t tableid)
{
    switch(tableid) {
        case P4TBL_ID_A_HASH:
            return (A_HASH_MAX_ID);
        break;
        case P4TBL_ID_B_TCAM:
            return (B_TCAM_MAX_ID);
        break;
        case P4TBL_ID_C_IDX:
            return (C_IDX_MAX_ID);
        break;
        case P4TBL_ID_D_MPU_ONLY:
            return (D_MPU_ONLY_MAX_ID);
        break;
    }
    // Not found tableid case
    return (0);
}

inline void p4pd_get_action_name(uint32_t tableid, int actionid, char *action_name)
{
    switch(tableid) {
        case P4TBL_ID_A_HASH:
            switch(actionid) {
                case A_HASH_NOP_ID:
                    strcpy(action_name, "nop");
                    return;
                break;
            }
        break;
        case P4TBL_ID_B_TCAM:
            switch(actionid) {
                case B_TCAM_UPDATE_IPV4_ID:
                    strcpy(action_name, "update_ipv4");
                    return;
                break;
                case B_TCAM_UPDATE_IPV6_ID:
                    strcpy(action_name, "update_ipv6");
                    return;
                break;
            }
        break;
        case P4TBL_ID_C_IDX:
            switch(actionid) {
                case C_IDX_UPDATE_TTL_ID:
                    strcpy(action_name, "update_ttl");
                    return;
                break;
            }
        break;
        case P4TBL_ID_D_MPU_ONLY:
            switch(actionid) {
                case D_MPU_ONLY_MPU_ONLY_ACTION_ID:
                    strcpy(action_name, "mpu_only_action");
                    return;
                break;
            }
        break;
    }
    *action_name = '\0';
}

#endif 
