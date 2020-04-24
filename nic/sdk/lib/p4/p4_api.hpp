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

#include "p4_utils.hpp"
#include "include/sdk/types.hpp"

#ifndef __P4_API_H__
#define __P4_API_H__

#define P4TBL_ID_MAX                                 100
#define P4TBL_NAME_MAX_LEN                           80
#define P4TBL_MAX_ACTIONS                            64
#define P4ACTION_NAME_MAX_LEN                        100

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

typedef enum p4pd_pipeline_ {
    P4_PIPELINE_INGRESS = 0,
    P4_PIPELINE_EGRESS,
    P4_PIPELINE_RXDMA,
    P4_PIPELINE_TXDMA,
    P4_PIPELINE_MAX
} p4pd_pipeline_t;

typedef enum p4pd_table_cache_ {
    P4_TBL_CACHE_NONE           = 0U,
    P4_TBL_CACHE_INGRESS        = 1U << 0,
    P4_TBL_CACHE_EGRESS         = 1U << 1,
    P4_TBL_CACHE_TXDMA          = 1U << 2,
    P4_TBL_CACHE_RXDMA          = 1U << 3,
    P4_TBL_CACHE_TXDMA_RXDMA    = P4_TBL_CACHE_TXDMA | P4_TBL_CACHE_RXDMA,
} p4pd_table_cache_t;

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


//------------------------------------------------------------------------------
// different modes of writing to ASIC
// 1. non-blocking - adds write operation to asicrw thread's work queue &
//                   returns
// 2. blocking     - adds write operation to asicrw thread's work queue & blocks
//                   until the operation is done by asicrw thread
// 3. write-thru   - non-blocking version that bypasses asicrw thread completely
//                   and writes in the caller thread's context
//------------------------------------------------------------------------------
typedef enum p4pd_table_write_mode_ {
    P4_TBL_WRITE_MODE_NON_BLOCKING = 0,
    P4_TBL_WRITE_MODE_BLOCKING     = 1,
    P4_TBL_WRITE_MODE_WRITE_THRU   = 2
} p4pd_table_write_mode_t;

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
    mem_addr_t              base_mem_pa; /* physical addres in memory */
    mem_addr_t              base_mem_va; /* virtual  address in memory */
    p4pd_table_cache_t      cache; /* Cache region info. Valid only for memory based tables */
    p4pd_pipeline_t         pipe; /* Pipeline this table belongs to */
    p4pd_table_write_mode_t wr_mode; /* Pipeline table write mode */
    bool                    read_thru_mode; /* Pipeline table read thru mode */
    uint8_t                 table_thread_count; /* Number of table execution threads. Min 1 */
    uint8_t                 thread_table_id[P4PD_TABLE_MAX_CONCURRENCY];
    uint64_t                token_refresh_rate; // token refresh rate
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
    const char     *p4pd_rxdma_pgm_name;   // rxdma program name
    const char     *p4pd_txdma_pgm_name;   // txdma program name
    const char     *cfg_path;              // HAL config path
} p4pd_cfg_t;
p4pd_error_t p4pd_init(p4pd_cfg_t *p4pd_cfg);

/* P4PD SRAM/TCAM/HBM confinguration
 * Updated by parsing the table map config file
 */
typedef struct p4pd_table_max_cfg_s {
    uint32_t tcam_ingress_depth;
    uint32_t tcam_egress_depth;
    uint32_t sram_ingress_depth;
    uint32_t sram_egress_depth;
} p4pd_table_max_cfg_t;

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
p4pd_error_t p4pd_table_properties_get(uint32_t tableid,
                                       p4pd_table_properties_t *tbl_ctx);

p4pd_error_t p4pd_table_properties_optimal_get(uint32_t tableid,
                                               p4pd_table_properties_t **tbl_ctx);


/* P4PD API that uses tableID to set the table write mode that ASIC
 * layer use when writing to P4 tables.
 *
 * Arguments:
 *
 *  IN  : uint32_t          tableid    : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
 *  IN  : p4pd_table_write_mode_t wr_mode  : Table write mode.
 *
 * Return Value:
 *  P4PD_SUCCESS                       : Table write mode is set successfully
 *                                       in p4tbls context that was initialized earlier.
 *
 *  P4PD_FAIL                          : If tableid is not valid

 */
p4pd_error_t p4pd_table_properties_set_write_mode(uint32_t tableid,
                                                  p4pd_table_write_mode_t wr_mode);


/* P4PD API that uses tableID to set the table read thru mode property that ASIC
 * layer can uses when reading P4 tables.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
 *  IN  : bool     read_thru_mode      : Table reade mode.
 *
 * Return Value:
 *  P4PD_SUCCESS                       : Table read thru mode is set successfully
 *                                       in p4tbls context that was initialized earlier.
 *
 *  P4PD_FAIL                          : If tableid is not valid
 *
 */
p4pd_error_t p4pd_table_properties_set_read_thru_mode(uint32_t tableid,
                                                      bool read_thru_mode);


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

p4pd_error_t p4pd_rxdma_hwkey_hwmask_build(uint32_t   tableid,
                                           void       *swkey,
                                           void       *swkey_mask,
                                           uint8_t    *hw_key,
                                           uint8_t    *hw_key_mask);

p4pd_error_t p4pd_txdma_hwkey_hwmask_build(uint32_t   tableid,
                                           void       *swkey,
                                           void       *swkey_mask,
                                           uint8_t    *hw_key,
                                           uint8_t    *hw_key_mask);

p4pd_error_t p4pd_global_hwkey_hwmask_build(uint32_t   tableid,
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

/* Install entry into P4-table (using sw keys).
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
 *  IN  : uint8_t *swkey         : Software key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *swkey_mask    : Key mask bits mask used in ternary matching.
 *                                 This data structure is of same type as the Key.
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
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_entry_install(uint32_t tableid,
                                uint32_t index,
                                void    *swkey,
                                void    *swkey_mask,
                                void    *actiondata);

p4pd_error_t p4pd_rxdma_entry_install(uint32_t tableid,
                                      uint32_t index,
                                      void    *swkey,
                                      void    *swkey_mask,
                                      void    *actiondata);

p4pd_error_t p4pd_txdma_entry_install(uint32_t tableid,
                                      uint32_t index,
                                      void    *swkey,
                                      void    *swkey_mask,
                                      void    *actiondata);

p4pd_error_t p4pd_global_entry_install(uint32_t tableid,
                                       uint32_t index,
                                       void    *swkey,
                                       void    *swkey_mask,
                                       void    *actiondata);
/* Install entry into P4-table (using hw keys).
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

/* Install entry into P4-table with a actiondata mask
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
p4pd_error_t p4pd_entry_write_with_datamask(uint32_t tableid,
                                            uint32_t index,
                                            uint8_t *hwkey,
                                            uint8_t *hwkey_mask,
                                            void    *actiondata,
                                            void    *actiondata_mask);

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

extern void p4pd_prep_p4tbl_sw_struct_sizes(void);
extern void p4pd_prep_p4tbl_names(void);
extern int p4pd_get_max_action_id(uint32_t tableid);
extern void p4pd_get_action_name(uint32_t tableid, int actionid, char *action_name);
extern char p4pd_tbl_names[][P4TBL_NAME_MAX_LEN];
extern uint16_t p4pd_tbl_swkey_size[];
extern uint16_t p4pd_tbl_sw_action_data_size[];
extern uint32_t p4pd_tableid_min_get();
extern uint32_t p4pd_tableid_max_get();

/*============================  P4PLUS PD RXDMA TABLE APIs ===================*/

/* P4PD Layer is initialized by invoking this function. It is expected
 * for the user of P4PD layer to invoke this API once before using any other
 * exposed by P4PD layer of code.
 *
 * All p4pd APIs will fail if this API is not invoked at the time of
 * initialization.
 */
extern p4pd_error_t p4pluspd_rxdma_init(p4pd_cfg_t *cfg);

/*
 * Invoking this function will cleanup all p4pd internal maintained structures
 * and free up heap memory.
 *
 * HAL can invoke this API for clean shutdown of HAL.
 */
extern void p4pluspd_rxdma_cleanup();

extern p4pd_error_t p4pluspd_rxdma_table_properties_get(uint32_t tableid,
    p4pd_table_properties_t *tbl_ctx);

extern p4pd_error_t p4pluspd_rxdma_table_properties_optimal_get(uint32_t tableid,
    p4pd_table_properties_t **tbl_ctx);

extern void p4pd_rxdma_hbm_table_address_set(uint32_t tableid, mem_addr_t pa,
                                             mem_addr_t va,
                                             p4pd_table_cache_t cache);
extern void p4pd_rxdma_hwentry_query(uint32_t tableid, uint32_t *hwkey_len,
    uint32_t *hwkeymask_len, uint32_t *hwactiondata_len);
extern p4pd_error_t p4pd_rxdma_entry_write_with_datamask(uint32_t tableid,
    uint32_t index, uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata,
    void *actiondata_mask);
extern p4pd_error_t p4pd_rxdma_entry_write(uint32_t tableid, uint32_t index,
    uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata);
extern p4pd_error_t p4pd_rxdma_entry_read(uint32_t tableid, uint32_t index,
    void *swkey, void *swkey_mask, void *actiondata);
extern p4pd_error_t p4pd_rxdma_table_entry_decoded_string_get(uint32_t tableid,
    uint32_t index, uint8_t* hwentry, uint8_t* hwentry_y,
    uint16_t hwentry_len, char* buffer, uint16_t buf_len);
extern p4pd_error_t p4pd_rxdma_table_ds_decoded_string_get(uint32_t tableid,
    uint32_t index, void* sw_key, void* sw_key_mask, void* action_data,
    char* buffer, uint16_t buf_len);
extern void p4pd_rxdma_prep_p4tbl_sw_struct_sizes(void);
extern void p4pd_rxdma_prep_p4tbl_names(void);
extern int p4pd_rxdma_get_max_action_id(uint32_t tableid);
extern void p4pd_rxdma_get_action_name(uint32_t tableid, int actionid,
                                       char *action_name);
extern char p4pd_rxdma_tbl_names[][P4TBL_NAME_MAX_LEN];
extern uint16_t p4pd_rxdma_tbl_swkey_size[];
extern uint16_t p4pd_rxdma_tbl_sw_action_data_size[];

extern uint32_t p4pd_rxdma_tableid_min_get();
extern uint32_t p4pd_rxdma_tableid_max_get();
extern uint32_t p4pd_sram_table_depth_get(p4pd_pipeline_t pipeline);
extern uint32_t p4pd_tcam_table_depth_get(p4pd_pipeline_t pipeline);

/*============================  P4PLUS PD RXDMA TABLE APIs ===================*/


/*============================  P4PLUS PD TXDMA TABLE APIs ===================*/

/* P4PD Layer is initialized by invoking this function. It is expected
 * for the user of P4PD layer to invoke this API once before using any other
 * exposed by P4PD layer of code.
 *
 * All p4pd APIs will fail if this API is not invoked at the time of
 * initialization.
 */
extern p4pd_error_t p4pluspd_txdma_init(p4pd_cfg_t *cfg);

/*
 * Invoking this function will cleanup all p4pd internal maintained structures
 * and free up heap memory.
 *
 * HAL can invoke this API for clean shutdown of HAL.
 */
extern void p4pluspd_txdma_cleanup();

extern p4pd_error_t p4pluspd_txdma_table_properties_get(uint32_t tableid,
    p4pd_table_properties_t *tbl_ctx);
extern p4pd_error_t p4pluspd_txdma_table_properties_optimal_get(uint32_t tableid,
    p4pd_table_properties_t **tbl_ctx);

extern void p4pd_txdma_hbm_table_address_set(uint32_t tableid, mem_addr_t pa,
                                             mem_addr_t va,
                                             p4pd_table_cache_t cache);
extern void p4pd_txdma_hwentry_query(uint32_t tableid, uint32_t *hwkey_len,
    uint32_t *hwkeymask_len, uint32_t *hwactiondata_len);
extern p4pd_error_t p4pd_txdma_entry_write_with_datamask(uint32_t tableid,
    uint32_t index, uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata,
    void *actiondata_mask);
extern p4pd_error_t p4pd_txdma_entry_write(uint32_t tableid, uint32_t index,
    uint8_t *hwkey, uint8_t *hwkey_y, void *actiondata);
extern p4pd_error_t p4pd_txdma_entry_read(uint32_t tableid, uint32_t index,
    void *swkey, void *swkey_mask, void *actiondata);
extern p4pd_error_t p4pd_txdma_table_entry_decoded_string_get(uint32_t tableid,
    uint32_t index, uint8_t* hwentry, uint8_t* hwentry_y,
    uint16_t hwentry_len, char* buffer, uint16_t buf_len);
extern p4pd_error_t p4pd_txdma_table_ds_decoded_string_get(uint32_t tableid,
    uint32_t index, void* sw_key, void* sw_key_mask, void* action_data,
    char* buffer, uint16_t buf_len);
extern void p4pd_txdma_prep_p4tbl_sw_struct_sizes(void);
extern void p4pd_txdma_prep_p4tbl_names(void);
extern int p4pd_txdma_get_max_action_id(uint32_t tableid);
extern void p4pd_txdma_get_action_name(uint32_t tableid, int actionid,
                                       char *action_name);
extern char p4pd_txdma_tbl_names[][P4TBL_NAME_MAX_LEN];
extern uint16_t p4pd_txdma_tbl_swkey_size[];
extern uint16_t p4pd_txdma_tbl_sw_action_data_size[];

extern uint32_t p4pd_txdma_tableid_min_get();
extern uint32_t p4pd_txdma_tableid_max_get();

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

/* Install entry into P4-table, performs a read-modify-write based on
 * actiondata_mask
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
 *  IN  : void    *actiondata_mask : Action data mask associated with the action
 *                                 data.
 *
 * Return Value:
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
p4pd_global_entry_write_with_datamask(uint32_t tableid,
                                      uint32_t  index,
                                      uint8_t   *hwkey,
                                      uint8_t   *hwkey_mask,
                                      void      *actiondata,
                                      void      *actiondata_mask);

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

p4pd_error_t p4pd_global_table_properties_get(uint32_t tableid, void *tbl_ctx);

p4pd_error_t p4pd_global_table_properties_optimal_get(uint32_t tableid, void *tbl_ctx);

uint32_t
p4pd_global_actiondata_appdata_size_get(uint32_t tableid, uint8_t actionid);

p4pd_error_t
p4pd_global_actiondata_appdata_set(uint32_t tableid, uint8_t actionid,
                                   void *appdata, void *actiondata);

void
p4pd_global_hbm_table_address_set(uint32_t tableid, mem_addr_t pa,
                                  mem_addr_t va, p4pd_table_cache_t cache);

/*
 * Functions to read/modify Mem Hash entries.
 */
p4pd_error_t
p4pd_actiondata_hwfield_set(uint32_t   tableid,
                            uint8_t    actionid,
                            uint32_t   argument_slotid,
                            uint8_t    *argumentvalue,
                            void       *actiondata);

p4pd_error_t
p4pd_rxdma_actiondata_hwfield_set(uint32_t   tableid,
                                  uint8_t    actionid,
                                  uint32_t   argument_slotid,
                                  uint8_t    *argumentvalue,
                                  void       *actiondata);

p4pd_error_t
p4pd_txdma_actiondata_hwfield_set(uint32_t   tableid,
                                  uint8_t    actionid,
                                  uint32_t   argument_slotid,
                                  uint8_t    *argumentvalue,
                                  void       *actiondata);

p4pd_error_t
p4pd_global_actiondata_hwfield_set(uint32_t   tableid,
                                   uint8_t    actionid,
                                   uint32_t   argument_slotid,
                                   uint8_t    *argumentvalue,
                                   void       *actiondata);

p4pd_error_t
p4pd_actiondata_hwfield_get(uint32_t   tableid,
                            uint8_t    actionid,
                            uint32_t   argument_slotid,
                            uint8_t    *argumentvalue,
                            void       *actiondata);

p4pd_error_t
p4pd_rxdma_actiondata_hwfield_get(uint32_t   tableid,
                                  uint8_t    actionid,
                                  uint32_t   argument_slotid,
                                  uint8_t    *argumentvalue,
                                  void       *actiondata);

p4pd_error_t
p4pd_txdma_actiondata_hwfield_get(uint32_t   tableid,
                                  uint8_t    actionid,
                                  uint32_t   argument_slotid,
                                  uint8_t    *argumentvalue,
                                  void       *actiondata);

p4pd_error_t
p4pd_global_actiondata_hwfield_get(uint32_t   tableid,
                                   uint8_t    actionid,
                                   uint32_t   argument_slotid,
                                   uint8_t    *argumentvalue,
                                   void       *actiondata);

p4pd_error_t
p4pd_actiondata_appdata_get(uint32_t   tableid,
                            uint8_t    actionid,
                            void       *appdata,
                            void       *actiondata);

p4pd_error_t
p4pd_rxdma_actiondata_appdata_get(uint32_t   tableid,
                                  uint8_t    actionid,
                                  void       *appdata,
                                  void       *actiondata);

p4pd_error_t
p4pd_txdma_actiondata_appdata_get(uint32_t   tableid,
                                  uint8_t    actionid,
                                  void       *appdata,
                                  void       *actiondata);

p4pd_error_t
p4pd_global_actiondata_appdata_get(uint32_t   tableid,
                                   uint8_t    actionid,
                                   void       *appdata,
                                   void       *actiondata);

p4pd_error_t
p4pd_actiondata_appdata_set(uint32_t   tableid,
                            uint8_t    actionid,
                            void       *appdata,
                            void       *actiondata);

p4pd_error_t
p4pd_rxdma_actiondata_appdata_set(uint32_t   tableid,
                                  uint8_t    actionid,
                                  void       *appdata,
                                  void       *actiondata);

p4pd_error_t
p4pd_txdma_actiondata_appdata_set(uint32_t   tableid,
                                  uint8_t    actionid,
                                  void       *appdata,
                                  void       *actiondata);

p4pd_error_t
p4pd_rxdma_actiondata_appdata_set(uint32_t   tableid,
                                  uint8_t    actionid,
                                  void       *appdata,
                                  void       *actiondata);

p4pd_error_t
p4pd_txdma_actiondata_appdata_set(uint32_t   tableid,
                                  uint8_t    actionid,
                                  void       *appdata,
                                  void       *actiondata);

uint32_t
p4pd_actiondata_appdata_size_get(uint32_t   tableid,
                                 uint8_t    actionid);
uint32_t
p4pd_txdma_actiondata_appdata_size_get(uint32_t tableid,
                                       uint8_t  actionid);
uint32_t
p4pd_rxdma_actiondata_appdata_size_get(uint32_t tableid,
                                       uint8_t  actionid);
p4pd_error_t p4pd_table_adjust_offsets(void);

#endif    // __P4_API_H__
