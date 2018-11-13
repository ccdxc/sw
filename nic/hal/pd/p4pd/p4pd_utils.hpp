#ifndef __P4PD_UTILS_H__
#define __P4PD_UTILS_H__

#include <stdint.h>

typedef int p4pd_error_t;
#define P4PD_SUCCESS                                 0
#define P4PD_FAIL                                    -1

#define P4PD_MAX_ACTION_DATA_LEN (512)
#define P4PD_MAX_MATCHKEY_LEN    (512) /* When multiple flits can be
                                        * chained, then key max size
                                        * can be more than 512. For
                                        * assuming one flit.
                                        */
#define P4PD_ACTIONPC_BITS       (8)   /* For now assume 8bit actionPC.
                                        * 9bits, change it to 2 bytes.
                                        */
#define P4PD_TCAM_WORD_CHUNK_LEN (16)  /* Tcam word chunk */
#define P4PD_MAX_PHV_LEN         (6*1024) /* Used to build wide table entry */

extern void
p4pd_utils_copy_into_hwentry(uint8_t *dest,
                             uint16_t dest_start_bit,
                             uint8_t *src,
                             uint16_t src_start_bit,
                             uint16_t num_bits);

extern void
p4pd_utils_copy_single_bit(uint8_t *dest,
                           uint16_t dest_start_bit,
                           uint8_t *src,
                           uint16_t src_start_bit,
                           uint16_t num_bits);

extern void
p4pd_utils_copy_le_src_to_be_dest(uint8_t *dest,
                                  uint16_t dest_start_bit,
                                  uint8_t *src,
                                  uint16_t src_start_bit,
                                  uint16_t num_bits);

extern void
p4pd_utils_copy_be_src_to_be_dest(uint8_t *dest,
                                  uint16_t dest_start_bit,
                                  uint8_t *src,
                                  uint16_t src_start_bit,
                                  uint16_t num_bits);

extern void
p4pd_utils_copy_be_src_to_le_dest(uint8_t *dest,
                                  uint16_t dest_start_bit,
                                  uint8_t *src,
                                  uint16_t src_start_bit,
                                  uint16_t num_bits);

extern void
p4pd_utils_copy_be_adata_to_le_dest(uint8_t *dest,
                                    uint16_t dest_start_bit,
                                    uint8_t *src,
                                    uint16_t src_start_bit,
                                    uint16_t num_bits);

extern void
p4pd_utils_copy_byte_aligned_src_and_dest(uint8_t *dest,
                                          uint16_t dest_start_bit,
                                          uint8_t *src,
                                          uint16_t src_start_bit,
                                          uint16_t num_bits);
extern void
p4pd_utils_swizzle_bytes(uint8_t *hwentry, uint16_t hwentry_bit_len);

extern uint32_t
p4pd_utils_widekey_hash_table_entry_prepare(uint8_t *hwentry,
                                            uint8_t action_pc,
                                            uint16_t match_key_start_bit,
                                            uint8_t *hwkey,
                                            uint16_t keylen,
                                            uint16_t actiondata_len,
                                            uint8_t *packed_actiondata_before_matchkey,
                                            uint16_t actiondata_before_matchkey_len,
                                            uint8_t *packed_actiondata_after_matchkey,
                                            uint16_t actiondata_after_matchkey_len,
                                            uint16_t *axi_shift_bytes);

extern uint32_t
p4pd_utils_hash_table_entry_prepare(uint8_t *hwentry,
                                    uint8_t action_pc,
                                    uint16_t match_key_start_bit,
                                    uint8_t *hwkey,
                                    uint16_t keylen,
                                    uint16_t actiondata_len,
                                    uint8_t *packed_actiondata_before_matchkey,
                                    uint16_t actiondata_before_matchkey_len,
                                    uint8_t *packed_actiondata_after_matchkey,
                                    uint16_t actiondata_after_matchkey_len,
                                    uint16_t *axi_shift_bytes);


extern uint32_t
p4pd_utils_p4table_entry_prepare(uint8_t *hwentry,
                                 uint8_t action_pc,
                                 uint8_t *hwkey,
                                 uint16_t keylen,
                                 uint8_t *packed_actiondata,
                                 uint16_t actiondata_len);
#endif

