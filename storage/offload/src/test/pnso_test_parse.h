/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_PARSE_H__
#define __PNSO_TEST_PARSE_H__

#include "osal_assert.h"
#include "pnso_test_desc.h"

#ifdef __cplusplus
extern "C" {
#endif

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

pnso_error_t pnso_test_stats_to_yaml(const struct test_testcase *testcase,
		uint64_t *stats, const char **stats_names, uint32_t stat_count,
		bool output_validations);

void test_free_desc(struct test_desc *desc);



#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PNSO_TEST_PARSE_H_ */
