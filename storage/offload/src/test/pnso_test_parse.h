/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_PARSE_H__
#define __PNSO_TEST_PARSE_H__

#include "osal_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	NODE_ROOT,
	NODE_ALIAS,
	NODE_SVC_CHAIN,
	NODE_SVC,
	NODE_CRYPTO_KEY,
	NODE_TESTCASE,
	NODE_VALIDATION,
	NODE_CP_HDR,
	NODE_CP_HDR_MAPPING,
	NODE_FILE,

	/* Must be last */
	NODE_MAX
} node_type_t;

struct test_node_list {
	struct test_node *head; /* prev */
	struct test_node *tail; /* next */
};

struct test_node {
	node_type_t type;
	uint32_t idx;
	struct test_node *parent;
	struct test_node_list siblings;
};

#define TEST_TABLE_BUCKET_COUNT 101
struct test_node_table {
	struct test_node_list buckets[TEST_TABLE_BUCKET_COUNT];
};

#define FOR_EACH_NODE(list) for (node = (list).head; node; node = node->siblings.tail)
#define FOR_EACH_NODE_SAFE(list) for (node = (list).head; \
		node && ((next_node = node->siblings.tail) != node); \
		node = next_node)
#define NODE_FIND_ID(list, id)  for (node = (list).head; node; \
				     node = node->siblings.tail) { \
					if (node->idx == id) break; }
#define FOR_EACH_TABLE_BUCKET(table) \
	for (bucket = 0, list = &(table).buckets[bucket]; \
	     bucket < TEST_TABLE_BUCKET_COUNT; \
	     list = &(table).buckets[++bucket])

#define TEST_CRYPTO_MAX_KEY_LEN 128

struct test_crypto_key {
	struct test_node node;
	uint32_t key1_len;
	uint32_t key2_len;
	uint8_t key1[TEST_CRYPTO_MAX_KEY_LEN];
	uint8_t key2[TEST_CRYPTO_MAX_KEY_LEN];
};

#define TEST_MAX_NAME_LEN 64
#define TEST_MAX_PATTERN_LEN 80
#define TEST_MAX_PATH_LEN 128
#define TEST_MAX_FILE_PREFIX_LEN 32
#define TEST_MAX_FULL_PATH_LEN (TEST_MAX_PATH_LEN+(2*TEST_MAX_FILE_PREFIX_LEN))
#define TEST_MAX_BATCH_DEPTH 512

enum {
	FILE_FORMAT_RANDOM,
	FILE_FORMAT_FILE,
	FILE_FORMAT_PATTERN,
};

struct test_input_desc {
	uint16_t format; /* FILE_FORMAT_* */
	uint32_t offset;
	uint32_t len;
	uint32_t min_block_size;
	uint32_t max_block_size;
	uint32_t block_count;
	uint32_t random_seed;
	char pattern[TEST_MAX_PATTERN_LEN];
	char pathname[TEST_MAX_PATH_LEN];
};

struct test_svc {
	struct test_node node;
	uint32_t output_flags;
	char output_path[TEST_MAX_PATH_LEN];
	struct pnso_service svc;
	union {
		struct {
			uint16_t threshold_delta;
		} cpdc;
		struct {
			uint32_t iv_data_len;
			uint8_t *iv_data;
		} crypto;
	} u;

	/* runtime data */
	uint32_t padded_len;
};

struct test_svc_chain {
	struct test_node node;
	char name[TEST_MAX_NAME_LEN];
	struct test_input_desc input;
	uint32_t num_services;
	//struct pnso_service svcs[PNSO_SVC_TYPE_MAX];
	struct test_node_list svcs;

	/* runtime data */
	struct test_validation *rt_retcode_validation;
};

static inline struct pnso_service *get_cur_svc(struct test_node *node)
{
	PNSO_ASSERT(node->type == NODE_SVC);

	return &((struct test_svc *) node)->svc;
}

#define MAX_SVC_CHAINS_PER_TESTCASE 64

enum {
	VALIDATION_UNKNOWN,
	VALIDATION_DATA_COMPARE,
	VALIDATION_SIZE_COMPARE,
	VALIDATION_RETCODE_COMPARE,

	/* Must be last */
	VALIDATION_TYPE_MAX
};

enum {
	COMPARE_TYPE_EQ, /* default */
	COMPARE_TYPE_NE,
	COMPARE_TYPE_LT,
	COMPARE_TYPE_LE,
	COMPARE_TYPE_GT,
	COMPARE_TYPE_GE,

	/* Must be last */
	COMPARE_TYPE_MAX
};

struct test_validation {
	struct test_node node;
	char name[TEST_MAX_NAME_LEN];
	uint16_t type; /* VALIDATION_* */
	uint16_t cmp_type; /* COMPARE_TYPE_* */
	char file1[TEST_MAX_PATH_LEN];
	char file2[TEST_MAX_PATH_LEN];
	uint32_t offset;
	uint32_t len;
	uint32_t svc_chain_idx;
	pnso_error_t retcode;
	uint32_t svc_count;
	pnso_error_t svc_retcodes[PNSO_SVC_TYPE_MAX];

	/* runtime stats */
	uint32_t rt_success_count;
	uint32_t rt_failure_count;
};

struct test_cp_header {
	struct test_node node;
	struct pnso_compression_header_format fmt;
};

struct test_cp_hdr_mapping {
	struct test_node node;
	uint32_t pnso_algo;
	uint32_t hdr_algo;
};

enum {
	SYNC_MODE_SYNC,
	SYNC_MODE_ASYNC,
	SYNC_MODE_POLL,

	/* Must be last */
	SYNC_MODE_MAX
};

struct test_testcase {
	struct test_node node;
	char name[TEST_MAX_NAME_LEN];
	uint32_t repeat;
	uint16_t batch_depth;
	uint16_t sync_mode;
	uint32_t svc_chain_count;
	uint32_t svc_chains[MAX_SVC_CHAINS_PER_TESTCASE];
	struct test_node_list validations;
};

struct test_node_file {
	struct test_node node;
	uint64_t checksum;
	uint64_t padded_checksum;
	uint32_t file_size;
	uint32_t padded_size;
	char filename[TEST_MAX_FULL_PATH_LEN+1];
};

#define TEST_ALIAS_MAX_NAME_LEN 32
#define TEST_ALIAS_MAX_VAL_LEN 64
struct test_alias {
	struct test_node node;
	char name[TEST_ALIAS_MAX_NAME_LEN];
	char val[TEST_ALIAS_MAX_VAL_LEN];
};

struct test_desc {
	struct test_node node;
	struct pnso_init_params init_params;
	char output_file_prefix[TEST_MAX_FILE_PREFIX_LEN];
	char output_file_suffix[TEST_MAX_FILE_PREFIX_LEN];
	bool delete_output_files;
	struct test_node_list aliases;
	struct test_node_list svc_chains;
	struct test_node_list crypto_keys;
	struct test_node_list cp_hdrs;
	struct test_node_list cp_hdr_map;
	struct test_node_list tests;
	struct test_node_table output_file_table;
};


typedef pnso_error_t (*test_create_fn)(struct test_desc *root,
				       struct test_node **pparent, void *opaque);
typedef pnso_error_t (*test_set_fn)(struct test_desc *root,
				    struct test_node *parent,
				    const char *val);

struct test_yaml_node_desc {
	const char *parent_name;
	const char *name;

	/* Called when node is first started */
	test_create_fn start_fn;

	/* Called for parameters within node */
	test_set_fn set_fn;

	/* Passed to function */
	void *opaque;
};

struct test_node *test_node_alloc(size_t size, node_type_t type);
void test_node_insert(struct test_node_list *list, struct test_node *node);

typedef int (*test_node_cmp_fn)(struct test_node *n1, struct test_node *n2);
struct test_node *test_node_lookup(struct test_node_list *list,
		struct test_node *node, test_node_cmp_fn fn);

struct test_node *test_node_table_lookup(struct test_node_table *table,
		struct test_node *node, test_node_cmp_fn fn);
void test_node_table_insert(struct test_node_table *table,
			    struct test_node *node);
void test_node_table_free_entries(struct test_node_table *table);

pnso_error_t parse_hex(const char *src, uint8_t *dst, uint32_t *dst_len);
static inline uint32_t str_hash(const char *str)
{
	uint32_t ret = 0;
	while (*str) {
		ret *= 31;
		ret ^= *str;
		str++;
	}
	return ret;
}

void test_free_desc(struct test_desc *desc);



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PNSO_TEST_PARSE_H_ */
