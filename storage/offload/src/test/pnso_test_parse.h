/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_PARSE_H__
#define __PNSO_TEST_PARSE_H__

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	NODE_ROOT,
	NODE_SVC_CHAIN,
	NODE_SVC,
	NODE_CRYPTO_KEY,
	NODE_TESTCASE,
	NODE_VALIDATION,
	NODE_CP_HDR,
	NODE_CP_HDR_MAPPING,
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

#define FOR_EACH_NODE(list) for (node = (list).head; node; node = node->siblings.tail)
#define FOR_EACH_NODE_SAFE(list) for (node = (list).head; \
		node && ((next_node = node->siblings.tail) != node); \
		node = next_node)
#define NODE_FIND_ID(list, id)  for (node = (list).head; node; \
				     node = node->siblings.tail) { \
					if (node->idx == id) break; }

#define TEST_CRYPTO_MAX_KEY_LEN 128

struct test_crypto_key {
	struct test_node node;
	uint32_t key1_len;
	uint32_t key2_len;
	uint8_t key1[TEST_CRYPTO_MAX_KEY_LEN];
	uint8_t key2[TEST_CRYPTO_MAX_KEY_LEN];
};

#define TEST_MAX_PATTERN_LEN 80
#define TEST_MAX_PATH_LEN 128

enum {
	FILE_FORMAT_RANDOM,
	FILE_FORMAT_FILE,
	FILE_FORMAT_PATTERN,
};

struct test_input_desc {
	uint16_t format; /* FILE_FORMAT_* */
	uint32_t offset;
	uint32_t len;
	uint32_t min_block_size; /* TODO */
	uint32_t max_block_size; /* TODO */
	uint32_t block_count; /* TODO */
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
};

struct test_svc_chain {
	struct test_node node;
	struct test_input_desc input;
	uint32_t num_services;
	//struct pnso_service svcs[PNSO_SVC_TYPE_MAX];
	struct test_node_list svcs;

	/* runtime data */
	struct test_validation *rt_retcode_validation;
};

static inline struct pnso_service *get_cur_svc(struct test_node *node)
{
	assert(node->type == NODE_SVC);

	return &((struct test_svc *) node)->svc;
}

#define MAX_SVC_CHAINS_PER_TESTCASE 64

enum {
	VALIDATION_UNKNOWN,
	VALIDATION_DATA_EQUAL,
	VALIDATION_SIZE_EQUAL,
	VALIDATION_RETCODE_EQUAL,

	/* Must be last */
	VALIDATION_TYPE_MAX
};

struct test_validation {
	struct test_node node;
	uint16_t type; /* VALIDATION_* */
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

struct test_testcase {
	struct test_node node;
	uint32_t repeat;
	uint16_t batch_depth;
	uint32_t svc_chain_count;
	uint32_t svc_chains[MAX_SVC_CHAINS_PER_TESTCASE];
	struct test_node_list validations;
};

#define MAX_FILE_PREFIX_LEN 32

struct test_desc {
	struct test_node node;
	struct pnso_init_params init_params;
	char output_file_prefix[MAX_FILE_PREFIX_LEN];
	struct test_node_list svc_chains;
	struct test_node_list crypto_keys;
	struct test_node_list cp_hdrs;
	struct test_node_list cp_hdr_map;
	struct test_node_list tests;
};


typedef pnso_error_t (*test_create_fn)(struct test_desc *root,
				       struct test_node **pparent, void *opaque);
typedef pnso_error_t (*test_set_fn)(struct test_desc *root,
				    struct test_node *parent, const char *val);

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

/* Insert to tail of list */
static inline void test_node_insert(struct test_node_list *list, struct test_node *node)
{
	if (!list->tail) {
		list->tail = node;
		list->head = node;
		return;
	}

	/* Replace the tail */
	node->siblings.head = list->tail;
	list->tail->siblings.tail = node;
	list->tail = node;
}



void test_free_desc(struct test_desc *desc);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PNSO_TEST_PARSE_H_ */
