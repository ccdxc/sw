
#define WAFL_WPBH_MAGIC 0x12345678
#define WAFL_WPBH_VERSION 1

typedef struct wafl_packed_blk_header_s {
	uint32_t wpbh_magic;
	uint32_t wpbh_version:4;
	uint32_t wpbh_spares:28;
	uint32_t wpbh_btid; /* Buftreeid of all chunks */
	uint32_t wpbh_num_objs; /* Number of objects */
	uint32_t wpbh_bytes_stored; /* Number of bytes stored as packing */
	uint32_t wpbh_spares1;
} wafl_packed_blk_header_t;

typedef struct wafl_packed_data_info_s {
	uint64_t wpd_vvbn:48; /* Lost-write fbn of chunk */
	uint64_t wpd_flags:16; /* Reconstruct flags for data */
	uint16_t wpd_off; /* Offset in 4K page */
	uint16_t wpd_len; /* Length of data */
} __attribute__ ((__packed__)) wafl_packed_data_info_t;

typedef struct wafl_packed_blk_s {
	wafl_packed_blk_header_t wpb_hdr;
	wafl_packed_data_info_t wpb_data_info[0];
} wafl_packed_blk_t;

typedef enum {
	/* These flags go ondisk in packing header */
	WAFL_PACKED_DATA_ENCODED	= 0x00000001,
	WAFL_PACKED_DATA_LZO_PRO	= 0x00000002,
} wafl_packed_data_flags_t;
