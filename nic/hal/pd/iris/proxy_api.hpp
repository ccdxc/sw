/*
 * p4pd_api.hpp
 * Mahesh Shirshyad (Pensando Systems)
 */
#ifndef __PROXY_API_H__
#define __PROXY_API_H__

#include <stdint.h>

#ifndef P4PD_CLI
#include <hal_pd_error.hpp>
#else
typedef int p4pd_error_t;
#endif

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

//#include <base.h>

#define P4PD_TCAM_DC_BIT                             1
#define P4PD_TCAM_DC_UINT8                           0xFF
#define P4PD_TCAM_DC_UINT16                          0xFFFF
#define P4PD_TCAM_DC_UINT32                          0xFFFFFFFF

#define P4PD_HBM_TLS_CB_START_ADDR                   0xaaaa

/* P4PD Layer is initialized by invoking this function. It is expected
 * for the user of P4PD layer to invoke this API once before using any other
 * exposed by P4PD layer of code.
 *
 * All p4pd APIs will fail if this API is not invoked at the time of
 * initialization.
 */
p4pd_error_t proxypd_init ();


/*
 * Invoking this function will cleanup all p4pd internal maintained structures
 * and free up heap memory.
 *
 * HAL can invoke this API for clean shutdown of HAL.
 */
void proxypd_cleanup();


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
p4pd_error_t proxypd_entry_write(uint32_t tableid,
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
 *                                 Data bits read from hardware are returned as
 *                                 action data structure matching structure 
 *                                 generated per p4 table. Refer to p4pd.h for 
 *                                 structure details
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
//p4pd_error_t tlscb_tbl_entry_read(uint32_t   tableid,
//                                  uint32_t   index,
//                                  void       *actiondata);



#endif
