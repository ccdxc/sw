// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

/* This file contains data structures and APIs needed to operate on P4 table.
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

#ifndef __P4PD_API_H__
#define __P4PD_API_H__

#include <stdint.h>
#include "nic/include/base.h"
#include "nic/hal/pd/hal_pd_error.hpp"

#define P4TBL_NAME_MAX_LEN                           80               // TODO: this is generated in multiple files today !!!

#define P4PD_TCAM_DC_BIT                             0
#define P4PD_TCAM_DC_UINT8                           0xFF
#define P4PD_TCAM_DC_UINT16                          0xFFFF
#define P4PD_TCAM_DC_UINT32                          0xFFFFFFFF
#define P4PD_TABLE_MAX_CONCURRENCY                   (4)

typedef enum p4pd_table_type_ {
    P4_TBL_TYPE_HASH = 0,
    P4_TBL_TYPE_HASHTCAM,
    P4_TBL_TYPE_TCAM,
    P4_TBL_TYPE_INDEX,
    P4_TBL_TYPE_MPU,
    P4_TBL_TYPE_INVALID
} p4pd_table_type_en;

typedef enum p4pd_table_location_ {
    P4_TBL_LOCATION_PIPE = 0,
    P4_TBL_LOCATION_HBM,
    P4_TBL_LOCATION_INVALID
} p4pd_table_location_en;

typedef enum p4pd_table_dir_{
    P4_GRESS_INGRESS = 0,
    P4_GRESS_EGRESS,
    P4_GRESS_INVALID
} p4pd_table_dir_en;

typedef struct p4pd_table_mem_layout_ {
    uint16_t    entry_width;    /* In units of memory words.. 16b  in case of PIPE tables */
                                /* In units of bytes in case of HBM table */
    uint16_t    entry_width_bits;
    uint32_t    start_index;
    uint32_t    end_index;
    uint16_t    top_left_x;
    uint16_t    top_left_y;
    uint8_t     top_left_block;
    uint16_t    btm_right_x;
    uint16_t    btm_right_y;
    uint8_t     btm_right_block;
    uint8_t     num_buckets;
} p4pd_table_mem_layout_t;


typedef struct p4pd_table_properties_ {
    uint8_t                 tableid; /* ID used by table APIs */
    uint8_t                 stage_tableid; /* ID used for programming table config in P4 pipe */
    char                    *tablename;
    p4pd_table_type_en      table_type; /* tcam/hash/hash-tcam/index */
    p4pd_table_location_en  table_location; /* hbm/pipe */
    p4pd_table_dir_en       gress; /* Ingress / Egress */
    uint8_t                 hash_type; /* When hash table, this indicate 
                                        * polynomial supported on asic
                                        * Encoded value 0 - 3 representing
                                        * different hash functions.
                                        */
    bool                    has_oflow_table;
    bool                    is_oflow_table;
    uint16_t                key_struct_size; /* SW Key struct size in bytes */
    uint16_t                actiondata_struct_size; /* SW action data struct size in bytes */
    uint32_t                tabledepth; /* Number of entries */
    uint8_t                 stage; /* Stage # in pipeline */
    uint8_t                 oflow_table_id; /* if has_otcam is True */
    uint8_t                 oflow_parent_table_id; /* if otcam pointing to parent */
    p4pd_table_mem_layout_t sram_layout;
    p4pd_table_mem_layout_t tcam_layout; /* Will be not used in case of hash / index table. */
    p4pd_table_mem_layout_t hbm_layout; /* Only if HBM table.. */
    uint8_t                 table_thread_count; /* Number of table execution threads. Min 1 */
    uint8_t                 thread_table_id[P4PD_TABLE_MAX_CONCURRENCY];
} p4pd_table_properties_t;


/* P4PD Layer is initialized by invoking this function. It is expected
 * for the user of P4PD layer to invoke this API once before using any other
 * exposed by P4PD layer of code.
 *
 * All p4pd APIs will fail if this API is not invoked at the time of
 * initialization.
 */
typedef struct p4pd_cfg_s {
    const char     *table_map_cfg_file;    // fully resolved path to the table map file
    const char     *p4pd_pgm_name;         // program name (iris/gft/...)
    const char     *cfg_path;              // HAL config path
} p4pd_cfg_t;
p4pd_error_t p4pd_init(p4pd_cfg_t *p4pd_cfg);

/*
 * Invoking this function will cleanup all p4pd internal maintained structures
 * and free up heap memory.
 *
 * HAL can invoke this API for clean shutdown of HAL.
 */
void p4pd_cleanup();

/* P4PD API that uses tableID to return table properties that HAL
 * layer can use to construct, initialize P4 tables in local memory.
 *
 * Arguments:
 *
 *  IN  : uint32_t          tableid    : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : p4pd_table_ctx_t *table_ctx  : Returns a structure of data
 *                                       that contains table properties.
 * Return Value:
 *  P4PD_SUCCESS                       : Table properties copied into tbl_ctx
 *                                       Memory for tbl_ctx is provided by
 *                                       API caller.
 *
 *  P4PD_FAIL                          : If tableid is not valid

 */
p4pd_error_t p4pd_table_properties_get(uint32_t                 tableid,
                                       p4pd_table_properties_t *tbl_ctx);



/* P4PD wrapper API that uses tableID to perform p4table operatons on 
 * any p4 table
 */

/* Query key details for p4-table
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwkey_len          : Hardware key length in bits
 *                                       Returned value is ZERO if
 *                                       tableid identifies Index table.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length in bits.
 *                                       Returned value is ZERO if
 *                                       tableid identifies Ternary/TCAM
 *                                       table.
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */
void p4pd_hwentry_query(uint32_t tableid, 
                        uint32_t *hwkey_len, 
                        uint32_t *hwkeymask_len, 
                        uint32_t *hwactiondata_len);


/* Build hardware key function for p4-table. The returned hw key
 * is byte stream that cannot be interpreted in meaningful way.
 * This byte stream of key should be used to install/write key
 * into hardware table using p4pd_entry_write() API
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid      : Table Id that identifies
 *                                P4 table. This id is obtained
 *                                from p4pd_table_id_enum.
 *  IN  : void *swkey           : Software key to be converted to  hardware key
 *                                Can be NULL if tableid identifies 
 *                                table type as Index table.
 *                                A software key structure is generated for every
 *                                p4-table. Refer to p4pd.h for structure details.
 *                                Such Per p4 table key data structure should
 *                                provided as void* swkey.
 *  IN  : void *swkey_mask      : Software keymask to be applied in case of
 *                                ternary match. Can be NULL if tableid identifies 
 *                                table type as Index table or Exact match table.
 *                                A software keymask structure is generated for every
 *                                p4-table. Refer to p4pd.h for structure details.
 *                                Such Per p4 table keymask data structure should
 *                                provided as void* swkey_mask.
 *  OUT : uint8_t *hw_key       : hardware key returned as byte stream
 *  OUT : uint8_t *hw_key_mask  : hardware key mask returned as byte stream
 *                                Will be NULL if tableid identifies 
 *                                table type as Index table or Exact match.
 *                                table.
 * 
 * Return Value: 
 *  p4pd_error_t                : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_hwkey_hwmask_build(uint32_t   tableid,
                                 void       *swkey, 
                                 void       *swkey_mask, 
                                 uint8_t    *hw_key, 
                                 uint8_t    *hw_key_mask);

/* 
 * Build index value that pipeline uses to lookup 
 * p4-table (index based lookup tables). The returned index
 * is where the table entry should be installed into hardware
 * table using p4pd_entry_write() API
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid      : Table Id that identifies
 *                                P4 table. This id is obtained
 *                                from p4pd_table_id_enum.
 * 
 *  IN  : void *swkey           : Software key structure containing all p4-fields
 *                                that form table index.
 * 
 * Return Value 
 *  uint64_t                   : hw_index
 */
uint64_t
p4pd_index_to_hwindex_map(uint32_t   tableid,
                          void       *swkey);


/* Install entry into P4-table.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  IN  : uint8_t *hwkey         : Hardware key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *hwkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure is generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_entry_write(uint32_t tableid,
                              uint32_t index,
                              uint8_t *hwkey, 
                              uint8_t *hwkey_mask,
                              void    *actiondata);

/* Read P4 table hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  OUT : void    *swkey         : Hardware key data read from hardware table is 
 *                                 converted to software key. A software key
 *                                 structure is generated for every p4-table.
 *                                 Refer to p4pd.h for structure details.
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  OUT : void    *swkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  OUT : void    *actiondata    : Action data associated with the key.
 *                                 Data bits read from hardware are returned as
 *                                 action data structure matching structure 
 *                                 generated per p4 table. Refer to p4pd.h for 
 *                                 structure details
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_entry_read(uint32_t   tableid,
                             uint32_t   index,
                             void       *swkey, 
                             void       *swkey_mask,
                             void       *actiondata);



/* Return Log string of decoded P4 table hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint8_t  hwentry       : Table entry bytes as read from device/hardware.
 *  IN  : uint8_t  hwentry_y     : TCAM Table entry trit_y bytes as read from 
 *                                 device/hardware. In non TCAM case, NULL.
 *  IN  : uint16_t hwentry_len   : Table entry length in bits.
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field 
 *                                 name and value.
 * Return Value: 
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_table_entry_decoded_string_get(uint32_t   tableid,
                                                 uint32_t   index,
                                                 uint8_t*   hwentry,
                                                 /* Valid only in case of TCAM;
                                                  * Otherwise can be NULL) 
                                                  */
                                                 uint8_t*   hwentry_y,
                                                 uint16_t   hwentry_len,
                                                 char*      buffer,
                                                 uint16_t   buf_len);


/* Return Log string of decoded P4 table structure (key, actiondata structures).
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : (void*)  swkey         : Table key structure.
 *  IN  : (void*)  swkey_mask    : Table key mask structure.
 *                                 In non TCAM case, NULL.
 *  IN  : (void*)  actiondata    : Table actiondata
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field 
 *                                 name and value.
 * Return Value: 
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_table_ds_decoded_string_get(uint32_t   tableid,
                                              uint32_t   index,
                                              void*      sw_key,
                                              /* Valid only in case of TCAM;
                                               * Otherwise can be NULL) 
                                               */
                                              void*      sw_key_mask,
                                              void*      action_data,
                                              char*      buffer,
                                              uint16_t   buf_len);


/*============================  P4PLUS PD RXDMA TABLE APIs ===================*/

/* P4PD Layer is initialized by invoking this function. It is expected
 * for the user of P4PD layer to invoke this API once before using any other
 * exposed by P4PD layer of code.
 *
 * All p4pd APIs will fail if this API is not invoked at the time of
 * initialization.
 */
p4pd_error_t p4pluspd_rxdma_init(p4pd_cfg_t *cfg);

/*
 * Invoking this function will cleanup all p4pd internal maintained structures
 * and free up heap memory.
 *
 * HAL can invoke this API for clean shutdown of HAL.
 */
void p4pluspd_rxdma_cleanup();

p4pd_error_t p4pluspd_rxdma_table_properties_get(uint32_t                 tableid,
                                                 p4pd_table_properties_t *tbl_ctx);
/* Query key details for p4-table
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwkey_len          : Hardware key length
 *                                       Returned value is ZERO if
 *                                       tableid identifies Index table.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length.
 *                                       Returned value is ZERO if
 *                                       tableid identifies Ternary/TCAM
 *                                       table.
 *  OUT : uint32_t *hwactiondata_len   : Action data length.
 *
 * Return Value:
 *  None
 */

void
p4pd_common_rxdma_actions_hwentry_query(uint32_t tableid,
                            uint32_t *hwkey_len,
                            uint32_t *hwkeymask_len,
                            uint32_t *hwactiondata_len);
/* Install entry into P4-table.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  IN  : uint8_t *hwkey         : Hardware key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *hwkey_y       : Key match trit bit mask used in ternary matching.
 *                                 This value is obtained by using
 *                                 p4pd_hwkey_hwmask_build().
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 *
 * Return Value:
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_common_rxdma_actions_entry_write(uint32_t tableid,
                          uint32_t index,
                          uint8_t *hwkey,
                          uint8_t *hwkey_y,
                          void    *actiondata);



/* Read P4 table hardware entry.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  OUT : void    *swkey         : Hardware key data read from hardware table is
 *                                 converted to software key. A software key
 *                                 structure is generated for every p4-table.
 *                                 Refer to p4pd.h for structure details.
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  OUT : void    *swkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  OUT : void    *actiondata    : Action data associated with the key.
 *                                 Data bits read from hardware are returned
 *                                 action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *
 * Return Value:
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_common_rxdma_actions_entry_read(uint32_t   tableid,
                         uint32_t   index,
                         void       *swkey,
                         void       *swkey_mask,
                         void       *actiondata);


/* Return Log string of decoded P4 table structure (key, actiondata structures).
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : (void*)  swkey         : Table key structure.
 *  IN  : (void*)  swkey_mask    : Table key mask structure.
 *                                 In non TCAM case, NULL.
 *  IN  : (void*)  actiondata    : Table actiondata
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field
 *                                 name and value.
 * Return Value:
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_common_rxdma_actions_table_ds_decoded_string_get(uint32_t   tableid,
                                                      uint32_t index,
                                          void*      sw_key,
                                          /* Valid only in case of TCAM;
                                           * Otherwise can be NULL)
                                           */
                                          void*      sw_key_mask,
                                          void*      action_data,
                                          char*      buffer,
                                          uint16_t   buf_len);

/*============================  P4PLUS PD RXDMA TABLE APIs ===================*/



/*============================  P4PLUS PD TXDMA TABLE APIs ===================*/

/* P4PD Layer is initialized by invoking this function. It is expected
 * for the user of P4PD layer to invoke this API once before using any other
 * exposed by P4PD layer of code.
 *
 * All p4pd APIs will fail if this API is not invoked at the time of
 * initialization.
 */
p4pd_error_t p4pluspd_txdma_init(p4pd_cfg_t *cfg);

/*
 * Invoking this function will cleanup all p4pd internal maintained structures
 * and free up heap memory.
 *
 * HAL can invoke this API for clean shutdown of HAL.
 */
void p4pluspd_txdma_cleanup();

p4pd_error_t p4pluspd_txdma_table_properties_get(uint32_t                 tableid,
                                                  p4pd_table_properties_t *tbl_ctx);
/* Query key details for p4-table
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwkey_len          : Hardware key length
 *                                       Returned value is ZERO if
 *                                       tableid identifies Index table.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length.
 *                                       Returned value is ZERO if
 *                                       tableid identifies Ternary/TCAM
 *                                       table.
 *  OUT : uint32_t *hwactiondata_len   : Action data length.
 *
 * Return Value:
 *  None
 */

void
p4pd_common_txdma_actions_hwentry_query(uint32_t tableid,
                            uint32_t *hwkey_len,
                            uint32_t *hwkeymask_len,
                            uint32_t *hwactiondata_len);
/* Install entry into P4-table.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  IN  : uint8_t *hwkey         : Hardware key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *hwkey_y       : Key match trit bit mask used in ternary matching.
 *                                 This value is obtained by using
 *                                 p4pd_hwkey_hwmask_build().
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 *
 * Return Value:
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_common_txdma_actions_entry_write(uint32_t tableid,
                          uint32_t index,
                          uint8_t *hwkey,
                          uint8_t *hwkey_y,
                          void    *actiondata);



/* Read P4 table hardware entry.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  OUT : void    *swkey         : Hardware key data read from hardware table is
 *                                 converted to software key. A software key
 *                                 structure is generated for every p4-table.
 *                                 Refer to p4pd.h for structure details.
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  OUT : void    *swkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  OUT : void    *actiondata    : Action data associated with the key.
 *                                 Data bits read from hardware are returned
 *                                 action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *
 * Return Value:
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_common_txdma_actions_entry_read(uint32_t   tableid,
                         uint32_t   index,
                         void       *swkey,
                         void       *swkey_mask,
                         void       *actiondata);

/* Return Log string of decoded P4 table structure (key, actiondata structures).
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : (void*)  swkey         : Table key structure.
 *  IN  : (void*)  swkey_mask    : Table key mask structure.
 *                                 In non TCAM case, NULL.
 *  IN  : (void*)  actiondata    : Table actiondata
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field
 *                                 name and value.
 * Return Value:
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_common_txdma_actions_table_ds_decoded_string_get(uint32_t   tableid,
                                                      uint32_t   index,
                                          void*      sw_key,
                                          /* Valid only in case of TCAM;
                                           * Otherwise can be NULL)
                                           */
                                          void*      sw_key_mask,
                                          void*      action_data,
                                          char*      buffer,
                                          uint16_t   buf_len);

/*============================  P4PLUS PD TXDMA TABLE APIs ===================*/


/*======================== P4PD GLOBAL/COMMON TABLE C/R/W routines =============*/
/*      These routines call the appropriate C/R/W APIs across p4pd, rxdma,   *
 *      txdma, etc APIs based on the table index passed to these APIs        */

/* Query key details for p4-table
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwkey_len          : Hardware key length
 *                                       Returned value is ZERO if
 *                                       tableid identifies Index table.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length.
 *                                       Returned value is ZERO if
 *                                       tableid identifies Ternary/TCAM
 *                                       table.
 *  OUT : uint32_t *hwactiondata_len   : Action data length.
 *
 * Return Value:
 *  None
 */





/*======================== P4PD GLOBAL/COMMON TABLE C/R/W routines =============*/
/*      These routines call the appropriate C/R/W APIs across p4pd, rxdma,   *
 *      txdma, etc APIs based on the table index passed to these APIs        */

/* Query key details for p4-table
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwkey_len          : Hardware key length
 *                                       Returned value is ZERO if
 *                                       tableid identifies Index table.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length.
 *                                       Returned value is ZERO if
 *                                       tableid identifies Ternary/TCAM
 *                                       table.
 *  OUT : uint32_t *hwactiondata_len   : Action data length.
 *
 * Return Value:
 *  None
 */

void
p4pd_global_hwentry_query(uint32_t tableid,
                          uint32_t *hwkey_len,
                          uint32_t *hwkeymask_len,
                          uint32_t *hwactiondata_len);

/* Install entry into P4-table.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  IN  : uint8_t *hwkey         : Hardware key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *hwkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure is generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_global_entry_write(uint32_t tableid,
                              uint32_t index,
                              uint8_t *hwkey, 
                              uint8_t *hwkey_mask,
                              void    *actiondata);

/* Read P4 table hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  OUT : void    *swkey         : Hardware key data read from hardware table is
 *                                 converted to software key. A software key
 *                                 structure is generated for every p4-table.
 *                                 Refer to p4pd.h for structure details.
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  OUT : void    *swkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  OUT : void    *actiondata    : Action data associated with the key.
 *                                 Data bits read from hardware are returned as
 *                                 action data structure matching structure 
 *                                 generated per p4 table. Refer to p4pd.h for 
 *                                 structure details
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_global_entry_read(uint32_t   tableid,
                             uint32_t   index,
                             void       *swkey, 
                             void       *swkey_mask,
                             void       *actiondata);



/* Return Log string of decoded P4 table structure (key, actiondata structures).
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : (void*)  swkey         : Table key structure.
 *  IN  : (void*)  swkey_mask    : Table key mask structure.
 *                                 In non TCAM case, NULL.
 *  IN  : (void*)  actiondata    : Table actiondata
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field
 *                                 name and value.
 * Return Value:
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_global_table_ds_decoded_string_get(uint32_t   tableid,
                                        uint32_t   index,
                                        void*      sw_key,
                                        /* Valid only in case of TCAM;
                                         * Otherwise can be NULL)
                                         */
                                        void*      sw_key_mask,
                                        void*      action_data,
                                        char*      buffer,
                                        uint16_t   buf_len);

p4pd_error_t
p4pd_global_table_properties_get(uint32_t tableid, void *tbl_ctx);

#endif    // __P4PD_API_H__
