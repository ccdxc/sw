/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __KERNEL__
#include <ctype.h>
#endif
#include "osal.h"

#include "yaml.h"
#include "pnso_wafl.h"
#include "pnso_test.h"
#include "pnso_test_parse.h"


struct {
	pnso_submit_req_fn_t submit;
	pnso_output_fn_t status_output;
	pnso_alloc_fn_t alloc;
	pnso_dealloc_fn_t dealloc;
	pnso_realloc_fn_t realloc;
} g_hooks;

void *pnso_test_alloc(size_t sz)
{
	return osal_alloc(sz);
}

void *pnso_test_alloc_aligned(size_t alignment, size_t sz)
{
	return osal_aligned_alloc(alignment, sz);
}

void pnso_test_free(void *ptr)
{
	return osal_free(ptr);
}

#if 0
#undef TEST_NODE
#define TEST_NODE(name) #name
static const char *node_type_to_name_list[NODE_MAX] = {
	TEST_NODE_LIST
};
#endif

#undef VALIDATION_TYPE
#define VALIDATION_TYPE(name) #name
static const char *validation_type_to_name_list[VALIDATION_TYPE_MAX] = {
	VALIDATION_TYPE_LIST
};

/* global defaults */
static struct test_desc default_desc = {
	.node = { NODE_ROOT, 0, NULL, { NULL, NULL } },
	.init_params = {
		.per_core_qdepth = 16,
		.core_count = 1,
		.block_size = 4096 },
	.output_file_prefix = "sim_",
	.output_file_suffix = ".bin",
	.cpu_mask = 1,
	.flags = 0
};

/* testcase defaults */
static struct test_testcase default_testcase = {
	.node = { NODE_TESTCASE, 0, NULL, { NULL, NULL } },
	.repeat = 1,
	.batch_depth = 0,
	.batch_concurrency = 0,
	.sync_mode = SYNC_MODE_AUTO,
	.svc_chain_count = 0,
	.req_validations = { NULL, NULL },
	.batch_validations = { NULL, NULL },
};

/* service defaults */
static struct pnso_service default_svcs[PNSO_SVC_TYPE_MAX] = {
	{ .svc_type = PNSO_SVC_TYPE_NONE },
	{ .svc_type = PNSO_SVC_TYPE_ENCRYPT, .u.crypto_desc = {
		.algo_type = PNSO_CRYPTO_TYPE_XTS,
		.key_desc_idx = 1, .iv_addr = 0 } },
	{ .svc_type = PNSO_SVC_TYPE_DECRYPT, .u.crypto_desc = {
		.algo_type = PNSO_CRYPTO_TYPE_XTS,
		.key_desc_idx = 1, .iv_addr = 0 } },
	{ .svc_type = PNSO_SVC_TYPE_COMPRESS, .u.cp_desc = {
		.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A,
		.hdr_fmt_idx = 1,
		.hdr_algo = 1,
		.threshold_len = 0,
		.flags = PNSO_CP_DFLAG_ZERO_PAD | PNSO_CP_DFLAG_INSERT_HEADER } },
	{ .svc_type = PNSO_SVC_TYPE_DECOMPRESS, .u.dc_desc = {
		.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A,
		.hdr_fmt_idx = 1,
		.flags = PNSO_DC_DFLAG_HEADER_PRESENT } },
	{ .svc_type = PNSO_SVC_TYPE_HASH, .u.hash_desc = {
		.algo_type = PNSO_HASH_TYPE_SHA2_512,
		.flags = 0 /*PNSO_HASH_DFLAG_PER_BLOCK*/ } },
	{ .svc_type = PNSO_SVC_TYPE_CHKSUM, .u.chksum_desc = {
		.algo_type = PNSO_CHKSUM_TYPE_MADLER32,
		.flags = 0 /*PNSO_CHKSUM_DFLAG_PER_BLOCK*/ } },
	{ .svc_type = PNSO_SVC_TYPE_DECOMPACT, .u.decompact_desc = {
		.vvbn = 1 } }
};

static struct test_node *default_nodes_by_type[NODE_MAX] = {
	&default_desc.node, /* NODE_ROOT */
	NULL, /* NODE_ALIAS */
	NULL, /* NODE_SVC_CHAIN */
	NULL, /* NODE_SVC */
	NULL, /* NODE_CRYPTO_KEY */
	&default_testcase.node, /* NODE_TESTCASE */
	NULL, /* NODE_VALIDATION */
	NULL, /* NODE_CP_HDR */
	NULL, /* NODE_CP_HDR_MAPPING */
	NULL, /* NODE_FILE */
};

static void test_init_parser(void);

void pnso_test_init_fns(pnso_submit_req_fn_t submit,
			pnso_output_fn_t status_output,
			pnso_alloc_fn_t alloc,
			pnso_dealloc_fn_t dealloc,
			pnso_realloc_fn_t realloc)
{
	g_hooks.submit = submit;
	g_hooks.status_output = status_output;
	g_hooks.alloc = alloc;
	g_hooks.dealloc = dealloc;
	g_hooks.realloc = realloc;

	//yaml_init_allocators(alloc, dealloc, realloc);

	test_init_parser();
}

static void free_crypto_key(struct test_crypto_key *key)
{
	TEST_FREE(key);
}

static void free_cp_hdr(struct test_cp_header *hdr)
{
	TEST_FREE(hdr);
}

static void free_cp_hdr_mapping(struct test_cp_hdr_mapping *mapping)
{
	TEST_FREE(mapping);
}

static void free_alias(struct test_alias *alias)
{
	if (alias->val.data) {
		TEST_FREE(alias->val.data);
	}
	TEST_FREE(alias);
}

static void free_svc(struct test_svc *svc)
{
	if (svc->svc.svc_type == PNSO_SVC_TYPE_ENCRYPT ||
	    svc->svc.svc_type == PNSO_SVC_TYPE_DECRYPT) {
		if (svc->u.crypto.iv_data) {
			TEST_FREE(svc->u.crypto.iv_data);
		}
	}
	TEST_FREE(svc);
}

static void free_svc_chain(struct test_svc_chain *chain)
{
	struct test_node *node, *next_node;

	FOR_EACH_NODE_SAFE(chain->svcs) {
		free_svc((struct test_svc *) node);
	}
	if (chain->input.pattern.data) {
		TEST_FREE(chain->input.pattern.data);
	}
	TEST_FREE(chain);
}

static void free_validation(struct test_validation *validation)
{
	if (validation->pattern.data) {
		TEST_FREE(validation->pattern.data);
	}

	TEST_FREE(validation);
}

static void free_testcase(struct test_testcase *testcase)
{
	struct test_node *node, *next_node;
	FOR_EACH_NODE_SAFE(testcase->req_validations) {
		free_validation((struct test_validation *) node);
	}
	FOR_EACH_NODE_SAFE(testcase->batch_validations) {
		free_validation((struct test_validation *) node);
	}
	TEST_FREE(testcase);
}

void test_free_desc(struct test_desc *desc)
{
	struct test_node *node, *next_node;

	if (!desc) {
		return;
	}

	FOR_EACH_NODE_SAFE(desc->aliases) {
		free_alias((struct test_alias *) node);
	}
	FOR_EACH_NODE_SAFE(desc->svc_chains) {
		free_svc_chain((struct test_svc_chain *) node);
	}
	FOR_EACH_NODE_SAFE(desc->crypto_keys) {
		free_crypto_key((struct test_crypto_key *) node);
	}
	FOR_EACH_NODE_SAFE(desc->cp_hdrs) {
		free_cp_hdr((struct test_cp_header *) node);
	}
	FOR_EACH_NODE_SAFE(desc->cp_hdr_map) {
		free_cp_hdr_mapping((struct test_cp_hdr_mapping *) node);
	}
	FOR_EACH_NODE_SAFE(desc->tests) {
		free_testcase((struct test_testcase *) node);
	}

	TEST_FREE(desc);
}

#if 0
static const char *yaml_event_type_to_string(int event_type)
{
	static const char *type_strs[12] = {
		"NONE", "STREAM_START", "STREAM_END", "DOCUMENT_START",
		"DOCUMENT_END", "ALIAS", "SCALAR", "SEQUENCE_START",
		"SEQUENCE_END", "MAPPING_START", "MAPPING_END", "INVALID" };

	if (event_type < 0 || event_type > 11) {
		event_type = 11;
	}
	return type_strs[event_type];
}
#endif

static bool is_hex_prefix(const char *str)
{
	return (str && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'));
}

static void dump_svc(struct test_svc *svc)
{
	PNSO_LOG("      Service type %u\n", svc->svc.svc_type);

	switch (svc->svc.svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		PNSO_LOG("        algo_type %u\n",
			 svc->svc.u.crypto_desc.algo_type);
		PNSO_LOG("        key_desc_idx %u\n",
			 svc->svc.u.crypto_desc.key_desc_idx);
		PNSO_LOG("        iv_addr 0x%llx\n",
			(unsigned long long) svc->svc.u.crypto_desc.iv_addr);
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		PNSO_LOG("        algo_type %u\n",
			 svc->svc.u.cp_desc.algo_type);
		PNSO_LOG("        flags 0x%x\n",
			 svc->svc.u.cp_desc.flags);
		PNSO_LOG("        threshold %u\n",
			 svc->svc.u.cp_desc.threshold_len);
		PNSO_LOG("        threshold_delta %u\n",
			 svc->u.cpdc.threshold_delta);
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		PNSO_LOG("        algo_type %u\n",
			 svc->svc.u.dc_desc.algo_type);
		PNSO_LOG("        flags 0x%x\n",
			 svc->svc.u.dc_desc.flags);
		break;
	case PNSO_SVC_TYPE_HASH:
		PNSO_LOG("        algo_type %u\n",
			 svc->svc.u.hash_desc.algo_type);
		PNSO_LOG("        flags 0x%x\n",
			 svc->svc.u.hash_desc.flags);
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		PNSO_LOG("        algo_type %u\n",
			 svc->svc.u.chksum_desc.algo_type);
		PNSO_LOG("        flags 0x%x\n",
			 svc->svc.u.chksum_desc.flags);
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
		PNSO_LOG("        vvbn 0x%llx\n",
			(unsigned long long) svc->svc.u.decompact_desc.vvbn);
		break;
	default:
		break;
	}
}

static void dump_svc_chain(struct test_svc_chain *svc_chain)
{
	struct test_node *node;

	PNSO_LOG("  ID %d\n", svc_chain->node.idx);
	if (svc_chain->name[0]) {
		PNSO_LOG("    Name %s\n", svc_chain->name);
	}

	PNSO_LOG("    Input format %u\n", svc_chain->input.format);
	if (svc_chain->input.offset) {
		PNSO_LOG("      offset %u\n", svc_chain->input.offset);
	}
	if (svc_chain->input.len_range.type) {
		PNSO_LOG("      length %u\n", svc_chain->input.len_range.start);
	}
	if (svc_chain->input.pattern.data) {
		PNSO_LOG("      pattern %s\n", svc_chain->input.pattern.data);
	}
	if (svc_chain->input.pathname[0]) {
		PNSO_LOG("      filename %s\n", svc_chain->input.pathname);
	}
	if (svc_chain->input.block_count) {
		PNSO_LOG("      block count %u\n", svc_chain->input.block_count);
	}
	if (svc_chain->input.min_block_size) {
		PNSO_LOG("      min block size %u\n", svc_chain->input.min_block_size);
	}
	if (svc_chain->input.max_block_size) {
		PNSO_LOG("      max block size %u\n", svc_chain->input.max_block_size);
	}

	PNSO_LOG("    Num services %u\n", svc_chain->num_services);

	FOR_EACH_NODE(svc_chain->svcs) {
		dump_svc((struct test_svc *) node);
	}
}

static void dump_validation(struct test_validation *validation)
{
	PNSO_LOG("      ID %d\n", validation->node.idx);
	PNSO_LOG("        Type %d\n", validation->type);
	if (validation->svc_chain_idx) {
		PNSO_LOG("        Svc_chain %u\n", validation->svc_chain_idx);
	}
	if (validation->file1[0]) {
		PNSO_LOG("        File1 %s\n", validation->file1);
	}
	if (validation->file2[0]) {
		PNSO_LOG("        File2 %s\n", validation->file2);
	}
	if (validation->pattern.data) {
		PNSO_LOG("        Pattern %s\n", validation->pattern.data);
	}
	if (validation->offset) {
		PNSO_LOG("        Offset %u\n", validation->offset);
	}
	if (validation->len) {
		PNSO_LOG("        Length %u\n", validation->len);
	}
	if (validation->type == VALIDATION_RETCODE_COMPARE) {
		PNSO_LOG("        Retcode %d\n", validation->retcode);
		if (validation->svc_count) {
			uint32_t i, len;
			char retcode_str[128] = "";

			len = 0;
			for (i = 0; i < validation->svc_count; i++) {
				len += safe_itoa(retcode_str+len,
						 sizeof(retcode_str)-len,
						 validation->svc_retcodes[i]);
			}
			retcode_str[len] = '\0';
			PNSO_LOG("        SVC retcodes %s\n", retcode_str);
		}
	}
}

static void dump_testcase(struct test_testcase *testcase)
{
	struct test_node *node;
	uint32_t i, len = 0;
	char svc_chain_str[128] = "";

	PNSO_LOG("  ID %d\n", testcase->node.idx);
	if (testcase->name[0]) {
		PNSO_LOG("    Name %s\n", testcase->name);
	}
	PNSO_LOG("    Repeat %llu\n", (unsigned long long) testcase->repeat);
	PNSO_LOG("    Batch depth %u\n", testcase->batch_depth);
	PNSO_LOG("    Batch concurrency %u\n", testcase->batch_concurrency);
	PNSO_LOG("    Sync mode %u\n", testcase->sync_mode);
	for (i = 0; i < testcase->svc_chain_count; i++) {
		len += safe_itoa(svc_chain_str+len, sizeof(svc_chain_str), testcase->svc_chains[i]);
		if (len < sizeof(svc_chain_str) - 1) {
			svc_chain_str[len++] = ',';
			svc_chain_str[len] = '\0';
		}
	}
	PNSO_LOG("    Service chains %s\n", svc_chain_str);

	PNSO_LOG("    Validations:\n");
	FOR_EACH_NODE(testcase->req_validations) {
		dump_validation((struct test_validation *) node);
	}
	FOR_EACH_NODE(testcase->batch_validations) {
		dump_validation((struct test_validation *) node);
	}
}

void test_dump_desc(struct test_desc *desc)
{
	struct test_node *node;

	PNSO_LOG("\nTest Descriptor:\n");

	if (desc->aliases.head) {
		PNSO_LOG("Aliases:\n");
		FOR_EACH_NODE(desc->aliases) {
			struct test_alias *alias = (struct test_alias *) node;
			PNSO_LOG("  %s=%s\n", alias->name, (const char *) alias->val.data);
		}
	}

	PNSO_LOG("Global params:\n");
	PNSO_LOG("  per_core_qdepth %u\n", desc->init_params.per_core_qdepth);
	PNSO_LOG("  block_size %u\n", desc->init_params.block_size);
	PNSO_LOG("  cpu_mask 0x%llx\n", (unsigned long long) desc->cpu_mask);
	PNSO_LOG("  output_file_prefix %s\n", desc->output_file_prefix);
	PNSO_LOG("  output_file_suffix %s\n", desc->output_file_suffix);

	PNSO_LOG("Service chains:\n");
	FOR_EACH_NODE(desc->svc_chains) {
		dump_svc_chain((struct test_svc_chain *)node);
	}

	PNSO_LOG("Testcases:\n");
	FOR_EACH_NODE(desc->tests) {
		dump_testcase((struct test_testcase *)node);
	}
}

#define PNSO_TEST_MAX_STACK_DEPTH 15

static void dump_yaml_event(yaml_event_t *event, int indent_level)
{
	char indent[PNSO_TEST_MAX_STACK_DEPTH + 1];

	if (indent_level >= sizeof(indent))
		indent_level = sizeof(indent) - 1;
	else if (indent_level <= 0)
		indent_level = 1;

	memset(indent, ' ', indent_level);
	indent[indent_level] = '\0';

	switch (event->type) {
	case YAML_NO_EVENT:
		PNSO_LOG_TRACE("%sYAML NO event\n", indent);
		break;
	case YAML_STREAM_START_EVENT:
		PNSO_LOG_TRACE("%sYAML STREAM_START event\n", indent);
		break;
	case YAML_STREAM_END_EVENT:
		PNSO_LOG_TRACE("%sYAML STREAM_END event\n", indent);
		break;
	case YAML_DOCUMENT_START_EVENT:
		PNSO_LOG_TRACE("%sYAML DOCUMENT_START event\n", indent);
		break;
	case YAML_DOCUMENT_END_EVENT:
		PNSO_LOG_TRACE("%sYAML DOCUMENT_END event\n", indent);
		break;
	case YAML_ALIAS_EVENT:
		PNSO_LOG_TRACE("%sYAML ALIAS event, anchor %s\n", indent,
			      event->data.alias.anchor);
		break;
	case YAML_SCALAR_EVENT:
		PNSO_LOG_TRACE("%sYAML SCALAR event, anchor %s, tag %s, value %s\n",
			      indent,
			      event->data.scalar.anchor,
			      event->data.scalar.tag,
			      event->data.scalar.value);
		break;
	case YAML_SEQUENCE_START_EVENT:
		PNSO_LOG_TRACE("%sYAML SEQUENCE_START event, anchor %s, tag %s\n",
			      indent,
			      event->data.sequence_start.anchor,
			      event->data.sequence_start.tag);
		break;
	case YAML_SEQUENCE_END_EVENT:
		PNSO_LOG_TRACE("%sYAML SEQUENCE_END event\n", indent);
		break;
	case YAML_MAPPING_START_EVENT:
		PNSO_LOG_TRACE("%sYAML MAPPING_START event, anchor %s, tag %s\n",
			      indent,
			      event->data.mapping_start.anchor,
			      event->data.mapping_start.tag);
		break;
	case YAML_MAPPING_END_EVENT:
		PNSO_LOG_TRACE("%sYAML MAPPING_END event\n", indent);
		break;
	default:
		PNSO_LOG_ERROR("%sYAML UNKNOWN event\n", indent);
		break;
	}
}

static const char *parse_csv_next(const char *str, size_t *len)
{
	static const char delimiters[] = " \t\r\n,;()";
	size_t offset;

	if (!str || !(*str))
		return NULL;

	/* Skip past delimiters */
	offset = strspn(str, delimiters);
	str += offset;
	offset = strcspn(str, delimiters);

	*len = offset;
	return offset ? str : NULL;
}


#define PNSO_SVC_TYPE_CPDC_MASK ((1 << PNSO_SVC_TYPE_COMPRESS) | \
				 (1 << PNSO_SVC_TYPE_DECOMPRESS))
#define PNSO_SVC_TYPE_CRYPTO_MASK ((1 << PNSO_SVC_TYPE_ENCRYPT) | \
				   (1 << PNSO_SVC_TYPE_DECRYPT))
#define PNSO_SVC_TYPE_HASHCHKSUM_MASK ((1 << PNSO_SVC_TYPE_HASH) | \
				       (1 << PNSO_SVC_TYPE_CHKSUM))
#define PNSO_SVC_TYPE_ALL_MASK 0xffff

struct svc_param_desc {
	const char *name;
	uint32_t name_len;
	uint16_t svc_type_mask;
	uint32_t value;
};

#define CP_DFLAG_DESC(name) \
	{ #name, sizeof(#name)-1, (1<<PNSO_SVC_TYPE_COMPRESS), PNSO_CP_DFLAG_##name }
#define DC_DFLAG_DESC(name) \
	{ #name, sizeof(#name)-1, (1<<PNSO_SVC_TYPE_DECOMPRESS), PNSO_DC_DFLAG_##name }
#define CPDC_DFLAG_DESC(name) \
	{ #name, sizeof(#name)-1, PNSO_SVC_TYPE_CPDC_MASK, PNSO_DC_DFLAG_##name }
#define HASH_DFLAG_DESC(name) \
	{ #name, sizeof(#name)-1, (1<<PNSO_SVC_TYPE_HASH), PNSO_HASH_DFLAG_##name }
#define CHKSUM_DFLAG_DESC(name) \
	{ #name, sizeof(#name)-1, (1<<PNSO_SVC_TYPE_CHKSUM), PNSO_CHKSUM_DFLAG_##name }
#define HASHCHKSUM_DFLAG_DESC(name) \
	{ #name, sizeof(#name)-1, PNSO_SVC_TYPE_HASHCHKSUM_MASK, PNSO_CHKSUM_DFLAG_##name }
#define HASH_ALGO_DESC(name)		\
	{ #name, sizeof(#name)-1, (1<<PNSO_SVC_TYPE_HASH), PNSO_HASH_TYPE_##name }
#define CHKSUM_ALGO_DESC(name)		\
	{ #name, sizeof(#name)-1, (1<<PNSO_SVC_TYPE_CHKSUM), PNSO_CHKSUM_TYPE_##name }
#define CPDC_ALGO_DESC(name)		\
	{ #name, sizeof(#name)-1, PNSO_SVC_TYPE_CPDC_MASK, PNSO_COMPRESSION_TYPE_##name }
#define CRYPTO_ALGO_DESC(name)		\
	{ #name, sizeof(#name)-1, PNSO_SVC_TYPE_CRYPTO_MASK, PNSO_CRYPTO_TYPE_##name }
#define OUTPUT_FLAG_DESC(name)		\
	{ #name, sizeof(#name)-1, 0, TEST_OUTPUT_FLAG_##name }
#define CP_HDR_FIELD_DESC(name) \
	{ #name, sizeof(#name)-1, 0, PNSO_HDR_FIELD_TYPE_##name }
#define CP_HDR_FIELD_DESC(name) \
	{ #name, sizeof(#name)-1, 0, PNSO_HDR_FIELD_TYPE_##name }
#define SYNC_MODE_DESC(name) \
	{ #name, sizeof(#name)-1, 0, SYNC_MODE_##name }
#define CMP_TYPE_DESC(name) \
	{ #name, sizeof(#name)-1, 0, COMPARE_TYPE_##name }
#define RANDOM_SEED_DESC(name) \
	{ #name, sizeof(#name)-1, 0, RANDOM_SEED_##name }
#define GLOBAL_FLAG_DESC(name) \
	{ #name, sizeof(#name)-1, 0, GLOBAL_FLAG_##name }
    

/* Keep alphabetized */
static struct svc_param_desc g_global_flags_map[] = {
	GLOBAL_FLAG_DESC(ABORT_ON_FAIL),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_dflag_map[] = {
	CP_DFLAG_DESC(BYPASS_ONFAIL),
	DC_DFLAG_DESC(HEADER_PRESENT),
	CP_DFLAG_DESC(INSERT_HEADER),
	CHKSUM_DFLAG_DESC(PER_BLOCK),
	HASH_DFLAG_DESC(PER_BLOCK),
	CP_DFLAG_DESC(ZERO_PAD),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_algo_map[] = {
	CHKSUM_ALGO_DESC(ADLER32),
	CHKSUM_ALGO_DESC(CRC32C),
	CRYPTO_ALGO_DESC(GCM),
	CPDC_ALGO_DESC(LZRW1A),
	CHKSUM_ALGO_DESC(MADLER32),
	CHKSUM_ALGO_DESC(MCRC64),
	HASH_ALGO_DESC(SHA2_256),
	HASH_ALGO_DESC(SHA2_512),
	CRYPTO_ALGO_DESC(XTS),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_output_flags_map[] = {
	OUTPUT_FLAG_DESC(APPEND),
	OUTPUT_FLAG_DESC(TINY),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_cp_hdr_field_map[] = {
	CP_HDR_FIELD_DESC(ALGO),
	CP_HDR_FIELD_DESC(INDATA_CHKSUM),
	CP_HDR_FIELD_DESC(OUTDATA_LENGTH),
	CP_HDR_FIELD_DESC(STATIC),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_sync_mode_map[] = {
	SYNC_MODE_DESC(ASYNC),
	SYNC_MODE_DESC(AUTO),
	SYNC_MODE_DESC(POLL),
	SYNC_MODE_DESC(SIM),
	SYNC_MODE_DESC(SYNC),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_cmp_type_map[] = {
	CMP_TYPE_DESC(EQ),
	CMP_TYPE_DESC(EZ),
	CMP_TYPE_DESC(GE),
	CMP_TYPE_DESC(GT),
	CMP_TYPE_DESC(GZ),
	CMP_TYPE_DESC(LE),
	CMP_TYPE_DESC(LT),
	CMP_TYPE_DESC(LZ),
	CMP_TYPE_DESC(NE),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_dyn_offset_map[] = {
	{ "eob", 3, 0, DYN_OFFSET_EOB },
	{ "eof", 3, 0, DYN_OFFSET_EOF },

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

/* Keep alphabetized */
static struct svc_param_desc g_random_seed_map[] = {
	RANDOM_SEED_DESC(REQ_ID),
	RANDOM_SEED_DESC(REQ_TIME),
	RANDOM_SEED_DESC(START_TIME),

	/* Must be last */
	{ NULL, 0, 0, 0 }
};

static void dump_param_map_names(struct svc_param_desc *param_map, uint16_t svc_type)
{
	struct svc_param_desc *entry;
	char str[256];
	size_t str_i = 0;
	size_t entry_i;

	/* Concatenate list of names */
	for (entry_i = 0; ; entry_i++) {
		entry = &param_map[entry_i];
		if (!entry->name) {
			break;
		}
		if (!entry->svc_type_mask ||
		    ((1<<svc_type) & entry->svc_type_mask)) {
			memcpy(str+str_i, entry->name, entry->name_len);
			str_i += entry->name_len;
			str[str_i++] = ',';
		}
	}
	str[str_i] = '\0';

	PNSO_LOG_DEBUG("  %s\n", str);
}

static int alias_cmp(struct test_node *node1, struct test_node *node2)
{
	return safe_strcmp(((struct test_alias *)node1)->name,
			   ((struct test_alias *)node2)->name);
}

static struct test_alias *lookup_alias_node(struct test_desc *desc, const char *name, uint32_t name_len)
{
	struct test_alias search_node;

	if (name_len == 0 || name_len >= TEST_ALIAS_MAX_NAME_LEN) {
		return NULL;
	}

	memset(&search_node, 0, sizeof(search_node));
	memcpy(search_node.name, name, name_len);
	search_node.name[name_len] = '\0';

	return (struct test_alias *) test_node_lookup(&desc->aliases,
						      &search_node.node,
						      alias_cmp);
}

static const char *lookup_alias_val(struct test_desc *desc, const char *name, uint32_t name_len)
{
	struct test_alias *found_alias;

	found_alias = lookup_alias_node(desc, name, name_len);
	if (!found_alias) {
		return NULL;
	}

	return (const char *) found_alias->val.data;
}

static struct svc_param_desc *lookup_svc_param(struct svc_param_desc *param_map,
					       const char *str, size_t len,
					       uint16_t svc_type)
{
	int cmp;
	struct svc_param_desc *param_desc;

	for (param_desc = param_map; param_desc->name != NULL; param_desc++) {
		cmp = strncasecmp(param_desc->name, str, len);
		if (cmp > 0) {
			/* Assumes param_map is alphabetized by name */
			goto not_found;
		}
		if (cmp == 0 && len == param_desc->name_len) {
			if (!param_desc->svc_type_mask ||
			    ((1<<svc_type) & param_desc->svc_type_mask)) {
				return param_desc;
			}
		}
	}

not_found:
	PNSO_LOG_DEBUG("Unknown parameter '%*s', should be one of:\n", (int) len, str);
	dump_param_map_names(param_map, svc_type);
	return NULL;
}

static pnso_error_t lookup_svc_param_value(struct svc_param_desc *param_map,
					   const char *str,
					   uint16_t svc_type,
					   uint32_t *ret)
{
	struct svc_param_desc *svc_param;

	if (isdigit(*str)) {
		*ret = (uint32_t) safe_strtoll(str);
		return PNSO_OK;
	}

	svc_param = lookup_svc_param(param_map, str, strlen(str), svc_type);
	if (!svc_param) {
		return EINVAL;
	}

	*ret = svc_param->value;
	return PNSO_OK;
}

static pnso_error_t lookup_svc_param_csv_flags(struct svc_param_desc *param_map,
			const char *str, uint16_t svc_type, uint32_t *flags)
{
	const char *search_str;
	size_t len;
	struct svc_param_desc *svc_param;
	uint32_t ret = 0;

	if (isdigit(*str)) {
		*flags = (uint32_t) safe_strtoll(str);
		return PNSO_OK;
	}

	search_str = str;
	while ((search_str = parse_csv_next(search_str, &len))) {
		svc_param = lookup_svc_param(param_map, search_str,
					     len, svc_type);
		if (svc_param) {
			ret |= svc_param->value;
		} else {
			return EINVAL;
		}
		search_str += len;
	}
	*flags = ret;
	return PNSO_OK;
}

#define ALIAS_SWAP(root, str) \
	if (str && *str == '$') { \
		const char *tmp = lookup_alias_val(root, str+1, strlen(str+1)); \
		if (!tmp) { \
			PNSO_LOG_ERROR("Unknown alias name '%s'\n", str+1); \
			return EINVAL; \
		} \
		str = tmp; \
	}

#define FUNC_SET_INT(fname, field, min, max) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	unsigned long long param; \
 \
	ALIAS_SWAP(root, val); \
	param = safe_strtoll(val); \
	PNSO_LOG_TRACE("Calling %s(%s)\n", #fname, val ? val : ""); \
	if (param >= (min) && param <= (max)) { \
		field = param; \
		return PNSO_OK; \
	} \
	PNSO_LOG_ERROR("Invalid value %s for %s\n", val, #fname); \
	return EINVAL; \
}

#define FUNC_SET_INT_EXT(fname, field, min, max, ext) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	unsigned long long param; \
 \
	ALIAS_SWAP(root, val); \
	param = safe_strtoll(val); \
	PNSO_LOG_TRACE("Calling %s(%s)\n", #fname, val ? val : ""); \
	if (param >= (min) && param <= (max)) { \
		field = param; \
		ext; \
		return PNSO_OK; \
	} \
	PNSO_LOG_ERROR("Invalid value %s for %s\n", val, #fname); \
	return EINVAL; \
}

#define FUNC_SET_RANGED_INT(fname, field, min, max) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	uint32_t start = 0, stop = 0, step = 1; \
	const char *search_str = val; \
	size_t len; \
	uint16_t range_type = RANGE_TYPE_NONE; \
 \
	ALIAS_SWAP(root, val); \
	PNSO_LOG_TRACE("Calling %s(%s)\n", #fname, val ? val : ""); \
	if (*val == 'r') { \
		len = 0; \
		if (safe_strncmp(val, "range(", 6) == 0) { \
			range_type = RANGE_TYPE_SEQ; \
			len = 6; \
		} else if (safe_strncmp(val, "random(", 7) == 0) {	\
			range_type = RANGE_TYPE_RAND; \
			len = 7; \
		} else { \
			PNSO_LOG_ERROR("Invalid value expression %s for %s\n", val, #fname); \
			return EINVAL; \
		} \
		if ((search_str = parse_csv_next(val+len, &len)) != NULL) { \
			start = safe_strtoll(search_str); \
			if ((search_str = parse_csv_next(search_str+len, &len)) != NULL) { \
				stop = safe_strtoll(search_str); \
				if ((search_str = parse_csv_next(search_str+len, &len)) != NULL) { \
					step = safe_strtoll(search_str); \
				} \
			} \
		} \
	} else { \
		range_type = RANGE_TYPE_SEQ; \
		start = safe_strtoll(val); \
		stop = start; \
	} \
	if (start >= (min) && stop <= (max) && start <= stop && \
	    step >= 1 && step <= (1 + stop - start)) { \
		(field)->type = range_type; \
		(field)->start = start; \
		(field)->stop = stop; \
		(field)->step = step; \
		(field)->count = (step + (stop - start)) / step; \
		return PNSO_OK; \
	} \
	PNSO_LOG_ERROR("Invalid value %s for %s, start %u stop %u step %u\n", \
		       val, #fname, start, stop, step); \
	return EINVAL; \
}

#define FUNC_SET_STRING(fname, field, maxlen) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	uint32_t len; \
 \
	ALIAS_SWAP(root, val); \
	len = strlen(val); \
	PNSO_LOG_TRACE("Calling %s(%s)\n", #fname, val ? val : ""); \
	if (len < maxlen) { \
		safe_strcpy((char*)(field), val, maxlen); \
		return PNSO_OK; \
	} \
	PNSO_LOG_ERROR("Invalid string length %u in %s, max %u\n", \
		       len, #fname, maxlen); \
	return EINVAL; \
}

#define FUNC_SET_BLOB(fname, field, maxlen) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	uint32_t len; \
	pnso_error_t err; \
 \
	ALIAS_SWAP(root, val); \
	PNSO_LOG_TRACE("Calling %s(%s)\n", #fname, val ? val : ""); \
	if ((field)->data) { \
		TEST_FREE((field)->data); \
		(field)->data = 0; \
		(field)->len = 0; \
	} \
	if (is_hex_prefix(val)) { \
		len = maxlen; \
		err = parse_hex(val, NULL, &len); \
		if (err) return EINVAL; \
		(field)->data = TEST_ALLOC(len + 1); \
		if (!(field)->data) { \
			PNSO_LOG_ERROR("Failed to alloc hex blob of len %u", len); \
			return EINVAL; \
		} \
		parse_hex(val, (field)->data, &len); \
		(field)->data[len] = 0; \
		(field)->len = len; \
	} else { \
		len = strlen(val); \
		if (len > maxlen) { \
			PNSO_LOG_ERROR("Invalid string length %u in %s, max %u\n", \
				       len, #fname, maxlen); \
			return EINVAL; \
		} \
		(field)->data = TEST_ALLOC(len + 1); \
		if (!(field)->data) { \
			PNSO_LOG_ERROR("Failed to alloc ascii blob of len %u", len); \
			return EINVAL; \
		} \
		(field)->len = len; \
		safe_strcpy((char*)(field)->data, val, len + 1); \
	} \
	return PNSO_OK; \
}

#define FUNC_SET_STATIC(fname, field, static_val) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	PNSO_LOG_TRACE("Calling %s\n", #fname); \
	field = static_val; \
	return PNSO_OK; \
}

#define FUNC_SET_PARAM_CSV_FLAGS(fname, field, param_map, svc_type) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	pnso_error_t rc; \
	uint32_t flags; \
	if (!val) { \
		return PNSO_OK; \
	} \
	ALIAS_SWAP(root, val); \
	rc = lookup_svc_param_csv_flags(param_map, val, svc_type, &flags); \
	if (rc == PNSO_OK) { \
		field = flags; \
	} \
	return rc; \
}

#define FUNC_SET_PARAM(fname, field, param_map, svc_type, min, max) \
static pnso_error_t fname(struct test_desc *root, struct test_node *parent, \
			  const char *val) \
{ \
	pnso_error_t rc; \
	uint32_t tmp; \
 \
	if (!val) { \
		return PNSO_OK; \
	} \
	ALIAS_SWAP(root, val); \
	rc = lookup_svc_param_value(param_map, val, svc_type, &tmp); \
	if (rc != PNSO_OK) { \
		PNSO_LOG_ERROR("Invalid parameter '%s' for %s\n", val, #fname); \
		return rc; \
	} \
	if (tmp < (min) || tmp > (max)) { \
		PNSO_LOG_ERROR("Invalid value for %s\n", #fname); \
		return EINVAL; \
	} \
	field = tmp; \
	return PNSO_OK; \
}


FUNC_SET_INT(test_set_per_core_qdepth, root->init_params.per_core_qdepth, 1, USHRT_MAX)
FUNC_SET_INT(test_set_block_size, root->init_params.block_size, 1, USHRT_MAX)
FUNC_SET_INT(test_set_cpu_mask, root->cpu_mask, 1, ULONG_MAX)
FUNC_SET_INT(test_set_limit_rate, root->limit_rate, 0, ULONG_MAX)
FUNC_SET_INT(test_set_status_interval, root->status_interval, 0, USHRT_MAX)
FUNC_SET_STRING(test_set_outfile_prefix, root->output_file_prefix,
		TEST_MAX_FILE_PREFIX_LEN)
FUNC_SET_STRING(test_set_outfile_suffix, root->output_file_suffix,
		TEST_MAX_FILE_PREFIX_LEN)
FUNC_SET_INT(test_set_store_output_files, root->store_output_files, 0, 1)

FUNC_SET_INT(test_set_idx, parent->idx, 1, UINT_MAX)

FUNC_SET_STRING(test_set_svc_chain_name, ((struct test_svc_chain *)parent)->name,
		TEST_MAX_NAME_LEN)
FUNC_SET_INT(test_set_svc_chain_batch_weight, ((struct test_svc_chain *)parent)->batch_weight, 1, TEST_MAX_BATCH_DEPTH)
FUNC_SET_PARAM(test_set_svc_chain_sync_mode, ((struct test_svc_chain *)parent)->sync_mode,
	       g_sync_mode_map, 0, 0, SYNC_MODE_MAX-1)
FUNC_SET_PARAM(test_set_input_random, ((struct test_svc_chain *)parent)->input.random_seed,
		g_random_seed_map, 0, 0, UINT_MAX)
FUNC_SET_RANGED_INT(test_set_input_random_len, &((struct test_svc_chain *)parent)->input.random_len_range, 1, TEST_MAX_RANDOM_LEN)
FUNC_SET_INT(test_set_input_offset, ((struct test_svc_chain *)parent)->input.offset, 0, UINT_MAX)
FUNC_SET_RANGED_INT(test_set_input_len, &((struct test_svc_chain *)parent)->input.len_range, 1, UINT_MAX)
FUNC_SET_INT(test_set_input_min_block, ((struct test_svc_chain *)parent)->input.min_block_size, 0, UINT_MAX)
FUNC_SET_INT(test_set_input_max_block, ((struct test_svc_chain *)parent)->input.max_block_size, 0, UINT_MAX)
FUNC_SET_INT(test_set_input_block_count, ((struct test_svc_chain *)parent)->input.block_count, 0, 1024)
FUNC_SET_STRING(test_set_input_file, ((struct test_svc_chain *)parent)->input.pathname, TEST_MAX_PATH_LEN)
FUNC_SET_STRING(test_set_input_output_file, ((struct test_svc_chain *)parent)->input.output_path, TEST_MAX_PATH_LEN)
FUNC_SET_BLOB(test_set_input_pattern, &((struct test_svc_chain *)parent)->input.pattern, TEST_MAX_PATTERN_LEN)
FUNC_SET_STRING(test_set_output_file, ((struct test_svc *)parent)->output_path, TEST_MAX_PATH_LEN)

FUNC_SET_INT(test_set_testcase_repeat, ((struct test_testcase *)parent)->repeat, 0, LLONG_MAX)
FUNC_SET_INT(test_set_testcase_turbo, ((struct test_testcase *)parent)->turbo, 0, 1)
FUNC_SET_INT(test_set_testcase_batch_depth, ((struct test_testcase *)parent)->batch_depth, 0, TEST_MAX_BATCH_DEPTH)
FUNC_SET_INT(test_set_testcase_batch_concurrency, ((struct test_testcase *)parent)->batch_concurrency, 0, TEST_MAX_BATCH_CONCURRENCY)
FUNC_SET_INT(test_set_testcase_retry_timeout, ((struct test_testcase *)parent)->retry_timeout, 0, UINT_MAX)
FUNC_SET_PARAM(test_set_testcase_sync_mode, ((struct test_testcase *)parent)->sync_mode,
	       g_sync_mode_map, 0, 0, SYNC_MODE_MAX-1)
FUNC_SET_STRING(test_set_testcase_name, ((struct test_testcase *)parent)->name,
		TEST_MAX_NAME_LEN)

FUNC_SET_STRING(test_set_validation_file1, ((struct test_validation *)parent)->file1,
		TEST_MAX_PATH_LEN)
FUNC_SET_STRING(test_set_validation_file2, ((struct test_validation *)parent)->file2,
		TEST_MAX_PATH_LEN)
FUNC_SET_BLOB(test_set_validation_pattern, &((struct test_validation *)parent)->pattern, TEST_MAX_PATTERN_LEN)
FUNC_SET_PARAM(test_set_compare_type, ((struct test_validation *)parent)->cmp_type,
	       g_cmp_type_map, 0, 0, COMPARE_TYPE_MAX-1)

FUNC_SET_PARAM(test_set_validation_data_offset, ((struct test_validation *)parent)->offset,
	       g_dyn_offset_map, 0, 0, UINT_MAX)
FUNC_SET_PARAM(test_set_validation_data_len, ((struct test_validation *)parent)->len,
	       g_dyn_offset_map, 0, 0, UINT_MAX)
FUNC_SET_INT(test_set_validation_svc_chain, ((struct test_validation *)parent)->svc_chain_idx, 0, UINT_MAX)
FUNC_SET_INT(test_set_validation_svc_idx, ((struct test_validation *)parent)->svc_idx, 0, UINT_MAX)
FUNC_SET_INT_EXT(test_set_validation_retcode, ((struct test_validation *)parent)->retcode, 0, UINT_MAX,
		 ((struct test_validation *)parent)->flags |= VALIDATION_FLAG_CHECK_RETCODE)
FUNC_SET_INT_EXT(test_set_validation_req_retcode, ((struct test_validation *)parent)->req_retcode, 0, UINT_MAX,
		 ((struct test_validation *)parent)->flags |= VALIDATION_FLAG_CHECK_REQ_RETCODE)

FUNC_SET_INT(test_set_cp_hdr_offset, ((struct test_cp_header *)parent)->fmt.fields[
	((struct test_cp_header *)parent)->fmt.num_fields - 1].offset, 0, UINT_MAX);
FUNC_SET_INT(test_set_cp_hdr_length, ((struct test_cp_header *)parent)->fmt.fields[
	((struct test_cp_header *)parent)->fmt.num_fields - 1].length, 0, UINT_MAX);
FUNC_SET_INT(test_set_cp_hdr_val, ((struct test_cp_header *)parent)->fmt.fields[
	((struct test_cp_header *)parent)->fmt.num_fields - 1].value, 0, UINT_MAX);

FUNC_SET_INT(test_set_cp_hdr_pnso_algo, ((struct test_cp_hdr_mapping *)parent)->pnso_algo,
	0, UINT_MAX);
FUNC_SET_INT(test_set_cp_hdr_algo, ((struct test_cp_hdr_mapping *)parent)->hdr_algo,
	0, UINT_MAX);

static pnso_error_t test_set_alias(struct test_desc *root,
				   struct test_node *parent,
				   const char *val)
{
	uint32_t name_len;
	uint32_t val_len;
	const char *alias_name;
	const char *alias_val;
	struct test_alias *alias;

	PNSO_LOG_TRACE("Calling test_set_alias(%s)\n", val);

	/* Find alias name */
	alias_name = val;
	name_len = 0;
	for (name_len = 0; name_len < TEST_ALIAS_MAX_NAME_LEN; name_len++) {
		if (val[name_len] == '\0' || val[name_len] == '=') {
			break;
		}
	}
	if (name_len == 0 || name_len >= TEST_ALIAS_MAX_NAME_LEN ||
	    val[name_len] != '=') {
		PNSO_LOG_ERROR("Invalid alias 'name=value' pair\n");
		return EINVAL;
	}

	/* Find alias value */
	alias_val = val + name_len + 1;
	val_len = strlen(alias_val);
	if (val_len == 0 || val_len >= TEST_ALIAS_MAX_VAL_LEN) {
		PNSO_LOG_ERROR("Invalid alias value length %u\n",
			       val_len);
		return EINVAL;
	}

	/* Lookup existing node */
	alias = lookup_alias_node(root, alias_name, name_len);
	if (!alias) {
		/* Create new node */
		alias = (struct test_alias *) test_node_alloc(sizeof(*alias),
							      NODE_ALIAS);
		if (!alias) {
			return ENOMEM;
		}
		memcpy(alias->name, alias_name, name_len);
		alias->name[name_len] = '\0';
		test_node_insert(&root->aliases, &alias->node);
	} else {
		/* Cleanup old value */
		if (alias->val.data) {
			TEST_FREE(alias->val.data);
			alias->val.data = NULL;
		}
		alias->val.len = 0;
	}

	/* Set new value string */
	alias->val.data = TEST_ALLOC(val_len + 1);
	if (!alias->val.data) {
		PNSO_LOG_ERROR("Failed to alloc alias val of len %u", val_len);
		return EINVAL;
	}
	memcpy(alias->val.data, alias_val, val_len);
	alias->val.data[val_len] = 0;
	alias->val.len = val_len;

	return PNSO_OK;
}

static pnso_error_t test_set_validation_svc_retcodes(struct test_desc *root,
						     struct test_node *parent,
						     const char *val)
{
	const char *search_str = val;
	struct test_validation *validation = (struct test_validation *) parent;
	pnso_error_t svc_retcode;
	size_t len;

	ALIAS_SWAP(root, val);
	if (!parent || validation->node.type != NODE_VALIDATION) {
		PNSO_LOG_ERROR("Invalid parent pointer in %s\n", __FUNCTION__);
		return EINVAL;
	}

	while ((search_str = parse_csv_next(search_str, &len))) {
		if (!isdigit(*search_str)) {
			return EINVAL;
		}
		svc_retcode = safe_strtoll(search_str);
		if (validation->svc_count >= PNSO_SVC_TYPE_MAX) {
			PNSO_LOG_ERROR("Cannot exceed %u svcs per validation\n",
				       PNSO_SVC_TYPE_MAX);
			return EINVAL;
		}
		validation->svc_retcodes[validation->svc_count++] = svc_retcode;
		search_str += len;
	}

	return PNSO_OK;
}

static pnso_error_t test_set_testcase_svc_chains(struct test_desc *root,
						 struct test_node *parent,
						 const char *val)
{
	const char *search_str = val;
	struct test_testcase *testcase = (struct test_testcase *) parent;
	unsigned long long chain_idx;
	size_t len;

	ALIAS_SWAP(root, val);
	if (!parent || testcase->node.type != NODE_TESTCASE) {
		PNSO_LOG_ERROR("Invalid parent pointer in set_testcase_svc_chains\n");
		return EINVAL;
	}

	while ((search_str = parse_csv_next(search_str, &len))) {
		if (!isdigit(*search_str)) {
			return EINVAL;
		}
		chain_idx = safe_strtoll(search_str);
		if (chain_idx < 1 || chain_idx > UINT_MAX) {
			return EINVAL;
		}
		if (testcase->svc_chain_count >= MAX_SVC_CHAINS_PER_TESTCASE) {
			PNSO_LOG_ERROR("Cannot exceed %d svc_chains per testcase\n",
				       MAX_SVC_CHAINS_PER_TESTCASE);
			return EINVAL;
		}
		testcase->svc_chains[testcase->svc_chain_count++] = (uint32_t) chain_idx;
		search_str += len;
	}

	return PNSO_OK;
}

static inline uint32_t xtoint(char c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'A' && c <= 'F') {
		return 10 + (c - 'A');
	}
	if (c >= 'a' && c <= 'f') {
		return 10 + (c - 'a');
	}
	return 0;
}

/*
 * Parse input data such as "03 a7 2b 80 ff or 0x03 0x07 0x2b"
 * If dst == NULL, just set the dst_len
 */
pnso_error_t parse_hex(const char *src, uint8_t *dst, uint32_t *dst_len)
{
	uint32_t max_len = *dst_len;
	*dst_len = 0;
	while (*src) {
		if (isspace(*src)) {
			src++;
			continue;
		}
		if (is_hex_prefix(src)) {
			src += 2;
			continue;
		}
		if (!isxdigit(*src) || !isxdigit(*(src+1))) {
			return EINVAL;
		}
		if (*dst_len >= max_len) {
			PNSO_LOG_ERROR("Cannot exceed dst buffer size %u in "
				       "parse_hex\n", max_len);
			return EINVAL;
		}
		if (dst) {
			*dst = (xtoint(*src) << 4) | xtoint(*(src+1));
			dst++;
		}
		(*dst_len)++;
		src += 2;
	}

	return PNSO_OK;
}

static pnso_error_t test_set_crypto_iv_data(struct test_desc *root,
					    struct test_node *parent,
					    const char *val)
{
	pnso_error_t err;
	struct test_svc *svc = (struct test_svc *) parent;
	uint32_t len;

	ALIAS_SWAP(root, val);

	/* Validation */
	if (parent->type != NODE_SVC) {
		return EINVAL;
	}
	if (svc->svc.svc_type != PNSO_SVC_TYPE_ENCRYPT &&
	    svc->svc.svc_type != PNSO_SVC_TYPE_DECRYPT) {
		return EINVAL;
	}

	/* Parse hex data */
	len = (strlen(val)/2) + 1;
	if (len < TEST_CRYPTO_IV_DATA_LEN)
		len = TEST_CRYPTO_IV_DATA_LEN;
	/* TODO: reduce alignment to 64 once aligned alloc works */
	svc->u.crypto.iv_data = (uint8_t*) TEST_ALLOC_ALIGNED(4096, len);
	if (!svc->u.crypto.iv_data) {
		PNSO_LOG_ERROR("Failed to allocate %u bytes for crypto iv_data\n", len);
		return ENOMEM;
	}
	memset(svc->u.crypto.iv_data, 0, len);
	err = parse_hex(val, svc->u.crypto.iv_data, &len);
	if (err != PNSO_OK) {
		PNSO_LOG_ERROR("Invalid hex data for crypto iv_data\n");
		return err;
	}
	svc->u.crypto.iv_data_len = len;

	return PNSO_OK;
}

static pnso_error_t set_key_data(const struct test_crypto_key *key,
				 const char *val,
				 uint8_t *dst,
				 uint32_t *len)
{
	if (key->node.type != NODE_CRYPTO_KEY) {
		return EINVAL;
	}

	*len = TEST_CRYPTO_MAX_KEY_LEN;
	return parse_hex(val, dst, len);
}

static pnso_error_t test_set_key1_data(struct test_desc *root,
				       struct test_node *parent,
				       const char *val)
{
	pnso_error_t err;
	struct test_crypto_key *key;
	uint32_t key_len;

	ALIAS_SWAP(root, val);

	key = (struct test_crypto_key *) parent;
	err = set_key_data(key, val, key->key1, &key_len);
	if (err != PNSO_OK) {
		return err;
	}
	key->key1_len = key_len;

	/* Copy key1 to key2, if key2 is not set */
	if (!key->key2_len) {
		memcpy(key->key2, key->key1, sizeof(key->key2));
		key->key2_len = key_len;
	}
	return PNSO_OK;
}

static pnso_error_t test_set_key2_data(struct test_desc *root,
				       struct test_node *parent,
				       const char *val)
{
	pnso_error_t err;
	struct test_crypto_key *key;
	uint32_t key_len;

	ALIAS_SWAP(root, val);

	key = (struct test_crypto_key *) parent;
	err = set_key_data(key, val, key->key2, &key_len);
	if (err != PNSO_OK) {
		return err;
	}
	key->key2_len = key_len;
	return PNSO_OK;
}

static pnso_error_t test_set_op_flags(struct test_desc *root,
			       struct test_node *parent, const char *val)
{
	pnso_error_t err;
	uint32_t flags = 0;
	struct pnso_service *svc = get_cur_svc(parent);

	ALIAS_SWAP(root, val);

	if (!svc || !val) {
		/* No-op */
		return PNSO_OK;
	}

	err = lookup_svc_param_csv_flags(g_dflag_map, val,
					 svc->svc_type, &flags);
	if (err != PNSO_OK) {
		return err;
	}

	/* Copy the flags to pnso_service */
	switch (svc->svc_type) {
	case PNSO_SVC_TYPE_NONE:
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		err = EINVAL;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc->u.cp_desc.flags = flags;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc->u.dc_desc.flags = flags;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc->u.hash_desc.flags = flags;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc->u.chksum_desc.flags = flags;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	default:
		err = EINVAL;
		break;
	}

	return err;
}

static pnso_error_t test_set_op_algo_type(struct test_desc *root,
				   struct test_node *parent, const char *val)
{
	pnso_error_t err = PNSO_OK;
	uint16_t algo_type = 0;
	struct svc_param_desc *svc_param;
	struct pnso_service *svc = get_cur_svc(parent);

	ALIAS_SWAP(root, val);

	if (!svc || !val) {
		/* No-op */
		return PNSO_OK;
	}

	if (isdigit(*val)) {
		algo_type = safe_strtoll(val);
	} else {
		/* Lookup algo_type by name */
		svc_param = lookup_svc_param(g_algo_map, val, strlen(val),
					     svc->svc_type);
		if (svc_param) {
			algo_type = svc_param->value;
		} else {
			PNSO_LOG_ERROR("Invalid algo_type '%s'\n", val);
			return EINVAL;
		}
	}

	/* Copy the algo_type to pnso_service */
	switch (svc->svc_type) {
	case PNSO_SVC_TYPE_NONE:
		err = EINVAL;
		break;
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		svc->u.crypto_desc.algo_type = algo_type;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc->u.cp_desc.algo_type = algo_type;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc->u.dc_desc.algo_type = algo_type;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc->u.hash_desc.algo_type = algo_type;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc->u.chksum_desc.algo_type = algo_type;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	default:
		err = EINVAL;
		break;
	}

	return err;
}

static pnso_error_t test_set_global_flags(struct test_desc *root,
				   struct test_node *parent, const char *val)
{
	pnso_error_t err;
	uint32_t flags = 0;

	ALIAS_SWAP(root, val);

	if (!val) {
		/* No-op */
		return PNSO_OK;
	}

	err = lookup_svc_param_csv_flags(g_global_flags_map,
					 val, 0, &flags);
	if (err == PNSO_OK) {
		/* Copy the flags to root */
		root->flags = flags;
	}

	return err;
}

static pnso_error_t test_set_output_flags(struct test_desc *root,
				   struct test_node *parent, const char *val)
{
	pnso_error_t err;
	uint32_t flags = 0;
	struct test_svc *svc_node = (struct test_svc *) parent;

	ALIAS_SWAP(root, val);

	if (!svc_node || !val) {
		/* No-op */
		return PNSO_OK;
	}
	OSAL_ASSERT(svc_node->node.type == NODE_SVC);

	err = lookup_svc_param_csv_flags(g_output_flags_map,
					 val, 0, &flags);
	if (err == PNSO_OK) {
		/* Copy the flags to pnso_service */
		svc_node->output_flags = flags;
	}

	return err;
}

static pnso_error_t test_set_cp_hdr_type(struct test_desc *root,
				   struct test_node *parent, const char *val)
{
	pnso_error_t err;
	uint32_t field_type = 0;
	struct test_cp_header *cp_hdr = (struct test_cp_header *) parent;

	ALIAS_SWAP(root, val);

	if (!cp_hdr || !val) {
		/* No-op */
		return PNSO_OK;
	}
	OSAL_ASSERT(parent->type == NODE_CP_HDR);

	err = lookup_svc_param_value(g_cp_hdr_field_map, val, 0, &field_type);
	if (err == PNSO_OK) {
		cp_hdr->fmt.fields[cp_hdr->fmt.num_fields - 1].type =
			field_type;
	}

	return err;
}

FUNC_SET_INT(test_set_output_len, ((struct test_svc *)parent)->output_len,
	     1, UINT_MAX);
FUNC_SET_INT(test_set_decompress_hdr_fmt_idx,
	     get_cur_svc(parent)->u.dc_desc.hdr_fmt_idx, 1, USHRT_MAX);
FUNC_SET_INT(test_set_compress_hdr_fmt_idx,
	     get_cur_svc(parent)->u.cp_desc.hdr_fmt_idx, 1, USHRT_MAX);
FUNC_SET_INT(test_set_compress_hdr_algo,
	     get_cur_svc(parent)->u.cp_desc.hdr_algo, 0, UINT_MAX);
FUNC_SET_INT(test_set_compress_threshold_len,
	     get_cur_svc(parent)->u.cp_desc.threshold_len, 0, UINT_MAX);
FUNC_SET_INT(test_set_compress_threshold_delta,
	     ((struct test_svc *)parent)->u.cpdc.threshold_delta, 0, USHRT_MAX);
FUNC_SET_INT(test_set_decompact_vvbn,
	     get_cur_svc(parent)->u.decompact_desc.vvbn, 0, (1ll<<48)-1);

static pnso_error_t test_set_crypto_key_idx(struct test_desc *root,
				     struct test_node *parent, const char *val)
{
	struct pnso_service *svc = get_cur_svc(parent);
	int key_id;

	ALIAS_SWAP(root, val);

	/* Validation */
	if (!svc || !val) {
		/* No-op */
		return PNSO_OK;
	}
	if (svc->svc_type != PNSO_SVC_TYPE_ENCRYPT &&
	    svc->svc_type != PNSO_SVC_TYPE_DECRYPT) {
		return EINVAL;
	}

	/* Read key idx */
	key_id = safe_strtoll(val);
	if (key_id < 0) {
		return EINVAL;
	}

	/* Set key idx */
	svc->u.crypto_desc.key_desc_idx = (uint32_t) key_id;

	return PNSO_OK;
}

struct test_node *test_node_alloc(size_t size, node_type_t type)
{
	struct test_node *node;
	struct test_node *default_node;

	if (size < sizeof(*node)) {
		PNSO_LOG_ERROR("Node size %zu too small\n", size);
		return NULL;
	}

	/* Alloc and initialize */
	node = (struct test_node *) TEST_ALLOC(size);
	if (!node) {
		PNSO_LOG_ERROR("Failed to allocate node, type %d, size %zu\n",
			       type, size);
		return NULL;
	}
	default_node = default_nodes_by_type[type];
	if (default_node == NULL) {
		memset(node, 0, size);
		node->type = type;
	} else {
		if (default_node->type != type) {
			PNSO_LOG_ERROR("Bad default node for type %u.\n",
				       type);
			TEST_FREE(node);
			return NULL;
		}
		memcpy(node, default_node, size);
	}

	return node;
}

void test_node_insert(struct test_node_list *list, struct test_node *node)
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

struct test_node *test_node_lookup(struct test_node_list *list,
				   struct test_node *search_node,
				   test_node_cmp_fn fn)
{
	struct test_node *node;

	FOR_EACH_NODE(*list) {
		if (fn(node, search_node) == 0) {
			return node;
		}
	}
	return NULL;
}

struct test_node *test_node_table_lookup(struct test_node_table *table,
					 struct test_node *search_node,
					 test_node_cmp_fn fn)
{
	struct test_node_list *list;

	list = &table->buckets[search_node->idx % TEST_TABLE_BUCKET_COUNT];
	return test_node_lookup(list, search_node, fn);
}

void test_node_table_insert(struct test_node_table *table, struct test_node *node)
{
	struct test_node_list *list;

	list = &table->buckets[node->idx % TEST_TABLE_BUCKET_COUNT];
	test_node_insert(list, node);
}

void test_node_table_free_entries(struct test_node_table *table)
{
	int bucket;
	struct test_node_list *list;
	struct test_node *node, *next_node;

	FOR_EACH_TABLE_BUCKET(*table) {
		FOR_EACH_NODE_SAFE(*list) {
			osal_free(node);
		}
	}
	memset(table, 0, sizeof(*table));
}


#define FUNC_CREATE_NODE(fname, struct_type, node_type, list)	\
static pnso_error_t fname(struct test_desc *root, \
			  struct test_node **parent, \
			  void *opaque) \
{ \
	struct_type *new_node; \
	if (!parent || !*parent) { \
		return PNSO_OK; \
	} \
	new_node = (struct_type *) test_node_alloc(sizeof(*new_node), node_type); \
	if (!new_node) { \
		return ENOMEM; \
	} \
	test_node_insert(&(list), (struct test_node *) new_node); \
	*parent = (struct test_node *) new_node; \
	return PNSO_OK; \
}

FUNC_CREATE_NODE(test_create_crypto_key, struct test_crypto_key,
		 NODE_CRYPTO_KEY, root->crypto_keys)
FUNC_CREATE_NODE(test_create_svc_chain, struct test_svc_chain,
		 NODE_SVC_CHAIN, root->svc_chains)
FUNC_CREATE_NODE(test_create_testcase, struct test_testcase,
		 NODE_TESTCASE, root->tests)
FUNC_CREATE_NODE(test_create_cp_hdr_format, struct test_cp_header,
		 NODE_CP_HDR, root->cp_hdrs)
FUNC_CREATE_NODE(test_create_cp_hdr_mapping, struct test_cp_hdr_mapping,
		 NODE_CP_HDR_MAPPING, root->cp_hdr_map)
#if 0
FUNC_CREATE_NODE(test_create_validation, struct test_validation,
		 NODE_VALIDATION, ((struct test_testcase *)(*parent))->validations)
#endif

#if 0
static pnso_error_t test_create_svc_chain(struct test_desc *root,
					  struct test_node **parent,
					  void *opaque)
{
	struct test_svc_chain *svc_chain;

	/* Alloc and initialize */
	svc_chain = (struct test_svc_chain *) test_node_alloc(sizeof(*svc_chain),
							 NODE_SVC_CHAIN);
	if (!svc_chain) {
		return ENOMEM;
	}

	/* Add to global svc_chain list */
	test_node_insert(&root->svc_chains, &svc_chain->node);
	*parent = &svc_chain->node;

	return PNSO_OK;
}
#endif

static void construct_validation_name(struct test_validation *validation)
{
	char *name = validation->name;

	switch (validation->type) {
	case VALIDATION_DATA_COMPARE:
		strcpy(name, "data");
		break;
	case VALIDATION_SIZE_COMPARE:
		strcpy(name, "size");
		break;
	case VALIDATION_RETCODE_COMPARE:
		strcpy(name, "retcode");
		break;
	case VALIDATION_DATA_LEN_COMPARE:
		strcpy(name, "data_len");
		break;
	default:
		strcpy(name, "unknown");
		break;
	}
}

static pnso_error_t test_create_validation(struct test_desc *root,
					   struct test_node **parent,
					   void *opaque)
{
	struct test_testcase *testcase;
	struct test_validation *validation;

	/* Validation */
	if (!parent || !*parent) {
		PNSO_LOG_ERROR("Missing parent pointer for validation creation.\n");
		return EINVAL;
	}
	if ((*parent)->type != NODE_TESTCASE) {
		PNSO_LOG_ERROR("Wrong parent type for validation creation.\n");
		return EINVAL;
	}
	testcase = (struct test_testcase *) (*parent);
	if ((uint64_t)(opaque) >= VALIDATION_TYPE_MAX) {
		PNSO_LOG_ERROR("Invalid validation type\n");
		return EINVAL;
	}

	validation = (struct test_validation *) test_node_alloc(sizeof(*validation),
							   NODE_VALIDATION);
	if (!validation) {
		return ENOMEM;
	}
	validation->type = (uint16_t)(uint64_t)opaque;
	construct_validation_name(validation);

	if (validation_is_per_req(validation->type)) {
		test_node_insert(&testcase->req_validations, &validation->node);
	} else {
		test_node_insert(&testcase->batch_validations, &validation->node);
	}
	validation->node.parent = *parent;
	*parent = &validation->node;

	return PNSO_OK;
}

static pnso_error_t test_create_op(struct test_desc *root,
				   struct test_node **parent,
				   void *opaque)
{
	struct test_svc_chain *svc_chain;
	struct test_svc *svc;

	/* Validation */
	if (!parent || !*parent) {
		PNSO_LOG_ERROR("Missing parent pointer for op creation.\n");
		return EINVAL;
	}
	if ((*parent)->type != NODE_SVC_CHAIN) {
		PNSO_LOG_ERROR("Wrong parent type for op creation.\n");
		return EINVAL;
	}
	svc_chain = (struct test_svc_chain *) (*parent);
	if (svc_chain->num_services >= PNSO_SVC_TYPE_MAX) {
		PNSO_LOG_ERROR("Cannot exceed max ops per svc_chain.\n");
		return EINVAL;
	}
	if ((uint64_t)(opaque) >= PNSO_SVC_TYPE_MAX) {
		PNSO_LOG_ERROR("Invalid svc_type for op creation.\n");
		return EINVAL;
	}

	/* Set it */
	svc = (struct test_svc *) test_node_alloc(sizeof(*svc), NODE_SVC);
	if (!svc) {
		return ENOMEM;
	}
	/* svc->svc.svc_type = (uint16_t)(uint64_t)opaque; */

	/* Set defaults */
	svc->svc = default_svcs[(uint64_t) opaque];

	test_node_insert(&svc_chain->svcs, &svc->node);
	svc->node.parent = *parent;
	svc_chain->num_services++;
	*parent = &svc->node;

	return PNSO_OK;
}

static pnso_error_t test_create_cp_hdr_field(struct test_desc *root,
					     struct test_node **parent,
					     void *opaque)
{
	struct test_cp_header *cp_hdr;

	/* Validation */
	if (!parent || !*parent) {
		PNSO_LOG_ERROR("Missing parent pointer for cp_hdr creation.\n");
		return EINVAL;
	}
	if ((*parent)->type != NODE_CP_HDR) {
		PNSO_LOG_ERROR("Wrong parent type for cp_hdr creation.\n");
		return EINVAL;
	}
	cp_hdr = (struct test_cp_header *) (*parent);
	if (cp_hdr->fmt.num_fields >= PNSO_MAX_HEADER_FIELDS) {
		PNSO_LOG_ERROR("Cannot exceed max fields per cp_hdr.\n");
		return EINVAL;
	}

	cp_hdr->fmt.num_fields++;

	return PNSO_OK;
}

#define ROOT_NODE_DESC(name) \
static struct test_yaml_node_desc name##_node_desc = { \
	NULL , #name, NULL, NULL, NULL, NULL, NULL, 0 };
#define CHILD_NODE_DESC(parent, name, create_fn, set_fn, opaque) \
static struct test_yaml_node_desc parent##_##name##_node_desc = { \
	& parent##_node_desc, #name, create_fn, set_fn, opaque, NULL, NULL, 1 };

#define ROOT_NODE_DESC_LIST \
ROOT_NODE_DESC(alias_group1) \
ROOT_NODE_DESC(alias_group2) \
ROOT_NODE_DESC(alias_group3) \
ROOT_NODE_DESC(alias_group4) \
ROOT_NODE_DESC(alias_group5) \
ROOT_NODE_DESC(global_params) \
ROOT_NODE_DESC(crypto_keys) \
ROOT_NODE_DESC(svc_chains) \
ROOT_NODE_DESC(tests) \
ROOT_NODE_DESC(cp_hdr_formats) \
ROOT_NODE_DESC(cp_hdr_mapping)

#define CHILD_NODE_DESC_LIST \
CHILD_NODE_DESC(alias_group1, alias, NULL, test_set_alias, NULL) \
CHILD_NODE_DESC(alias_group2, alias, NULL, test_set_alias, NULL) \
CHILD_NODE_DESC(alias_group3, alias, NULL, test_set_alias, NULL) \
CHILD_NODE_DESC(alias_group4, alias, NULL, test_set_alias, NULL) \
CHILD_NODE_DESC(alias_group5, alias, NULL, test_set_alias, NULL) \
\
CHILD_NODE_DESC(global_params, per_core_qdepth,    NULL, test_set_per_core_qdepth, NULL) \
CHILD_NODE_DESC(global_params, block_size,         NULL, test_set_block_size, NULL) \
CHILD_NODE_DESC(global_params, cpu_mask,           NULL, test_set_cpu_mask, NULL) \
CHILD_NODE_DESC(global_params, limit_rate,         NULL, test_set_limit_rate, NULL) \
CHILD_NODE_DESC(global_params, status_interval,    NULL, test_set_status_interval, NULL) \
CHILD_NODE_DESC(global_params, output_file_prefix, NULL, test_set_outfile_prefix, NULL) \
CHILD_NODE_DESC(global_params, output_file_suffix, NULL, test_set_outfile_suffix, NULL) \
CHILD_NODE_DESC(global_params, store_output_files, NULL, test_set_store_output_files, NULL) \
CHILD_NODE_DESC(global_params, flags,              NULL, test_set_global_flags, NULL) \
\
CHILD_NODE_DESC(crypto_keys, key, test_create_crypto_key, NULL, NULL) \
CHILD_NODE_DESC(crypto_keys_key, idx,  NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(crypto_keys_key, key1, NULL, test_set_key1_data, NULL) \
CHILD_NODE_DESC(crypto_keys_key, key2, NULL, test_set_key2_data, NULL) \
\
CHILD_NODE_DESC(svc_chains, svc_chain, test_create_svc_chain, NULL, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain, name,         NULL, test_set_svc_chain_name, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain, batch_weight, NULL, test_set_svc_chain_batch_weight, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain, mode,    NULL, test_set_svc_chain_sync_mode, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain, input,        NULL, NULL, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain, ops,          NULL, NULL, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_input, random,         NULL, test_set_input_random, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, random_len,     NULL, test_set_input_random_len, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, offset,         NULL, test_set_input_offset, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, len,            NULL, test_set_input_len, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, file,           NULL, test_set_input_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, output_file,    NULL, test_set_input_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, pattern,        NULL, test_set_input_pattern, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, min_block_size, NULL, test_set_input_min_block, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, max_block_size, NULL, test_set_input_max_block, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_input, block_count,    NULL, test_set_input_block_count, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops, compress,   test_create_op, NULL, (void*)PNSO_SVC_TYPE_COMPRESS) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops, decompress, test_create_op, NULL, (void*)PNSO_SVC_TYPE_DECOMPRESS) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops, hash,       test_create_op, NULL, (void*)PNSO_SVC_TYPE_HASH) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops, chksum,     test_create_op, NULL, (void*)PNSO_SVC_TYPE_CHKSUM) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops, encrypt,    test_create_op, NULL, (void*)PNSO_SVC_TYPE_ENCRYPT) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops, decrypt,    test_create_op, NULL, (void*)PNSO_SVC_TYPE_DECRYPT) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops, decompact,  test_create_op, NULL, (void*)PNSO_SVC_TYPE_DECOMPACT) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, flags,           NULL, test_set_op_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, algo_type,       NULL, test_set_op_algo_type, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, hdr_fmt_idx,     NULL, test_set_compress_hdr_fmt_idx, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, hdr_algo,        NULL, test_set_compress_hdr_algo, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, threshold,       NULL, test_set_compress_threshold_len, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, threshold_delta, NULL, test_set_compress_threshold_delta, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, output_file,     NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, output_flags,    NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_compress, output_len,      NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompress, flags,        NULL, test_set_op_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompress, algo_type,    NULL, test_set_op_algo_type, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompress, hdr_fmt_idx,  NULL, test_set_decompress_hdr_fmt_idx, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompress, output_file,  NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompress, output_flags, NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompress, output_len,   NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_encrypt, algo_type,    NULL, test_set_op_algo_type, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_encrypt, key_idx,      NULL, test_set_crypto_key_idx, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_encrypt, iv_data,      NULL, test_set_crypto_iv_data, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_encrypt, output_file,  NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_encrypt, output_flags, NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_encrypt, output_len,   NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decrypt, algo_type,    NULL, test_set_op_algo_type, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decrypt, key_idx,      NULL, test_set_crypto_key_idx, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decrypt, iv_data,      NULL, test_set_crypto_iv_data, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decrypt, output_file,  NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decrypt, output_flags, NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decrypt, output_len,   NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_hash, flags,        NULL, test_set_op_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_hash, algo_type,    NULL, test_set_op_algo_type, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_hash, output_file,  NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_hash, output_flags, NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_hash, output_len,   NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_chksum, flags,        NULL, test_set_op_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_chksum, algo_type,    NULL, test_set_op_algo_type, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_chksum, output_file,  NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_chksum, output_flags, NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_chksum, output_len,   NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompact, vvbn,         NULL, test_set_decompact_vvbn, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompact, output_file,  NULL, test_set_output_file, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompact, output_flags, NULL, test_set_output_flags, NULL) \
CHILD_NODE_DESC(svc_chains_svc_chain_ops_decompact, output_len, NULL, test_set_output_len, NULL) \
\
CHILD_NODE_DESC(tests, test,            test_create_testcase, NULL, NULL) \
CHILD_NODE_DESC(tests_test, idx,         NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test, name,        NULL, test_set_testcase_name, NULL) \
CHILD_NODE_DESC(tests_test, mode,        NULL, test_set_testcase_sync_mode, NULL) \
CHILD_NODE_DESC(tests_test, turbo,       NULL, test_set_testcase_turbo, NULL) \
CHILD_NODE_DESC(tests_test, repeat,      NULL, test_set_testcase_repeat, NULL) \
CHILD_NODE_DESC(tests_test, batch_depth, NULL, test_set_testcase_batch_depth, NULL) \
CHILD_NODE_DESC(tests_test, batch_concurrency, NULL, test_set_testcase_batch_concurrency, NULL) \
CHILD_NODE_DESC(tests_test, retry_timeout, NULL, test_set_testcase_retry_timeout, NULL) \
CHILD_NODE_DESC(tests_test, svc_chains,  NULL, test_set_testcase_svc_chains, NULL) \
CHILD_NODE_DESC(tests_test, validations, NULL, NULL, NULL) \
\
CHILD_NODE_DESC(tests_test_validations, data_compare,    test_create_validation, NULL, (void*)VALIDATION_DATA_COMPARE) \
CHILD_NODE_DESC(tests_test_validations, size_compare,    test_create_validation, NULL, (void*)VALIDATION_SIZE_COMPARE) \
CHILD_NODE_DESC(tests_test_validations, retcode_compare, test_create_validation, NULL, (void*)VALIDATION_RETCODE_COMPARE) \
CHILD_NODE_DESC(tests_test_validations, data_len_compare, test_create_validation, NULL, (void*)VALIDATION_DATA_LEN_COMPARE) \
CHILD_NODE_DESC(tests_test_validations, data_input_compare, test_create_validation, NULL, (void*)VALIDATION_DATA_INPUT_COMPARE) \
CHILD_NODE_DESC(tests_test_validations, data_output_compare, test_create_validation, NULL, (void*)VALIDATION_DATA_OUTPUT_COMPARE) \
\
CHILD_NODE_DESC(tests_test_validations_data_compare, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, type,         NULL, test_set_compare_type, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, svc_chain,    NULL, test_set_validation_svc_chain, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, file1,        NULL, test_set_validation_file1, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, file2,        NULL, test_set_validation_file2, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, pattern,      NULL, test_set_validation_pattern, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, offset,       NULL, test_set_validation_data_offset, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_compare, len,          NULL, test_set_validation_data_len, NULL) \
\
CHILD_NODE_DESC(tests_test_validations_size_compare, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_size_compare, type,         NULL, test_set_compare_type, NULL) \
CHILD_NODE_DESC(tests_test_validations_size_compare, svc_chain,    NULL, test_set_validation_svc_chain, NULL) \
CHILD_NODE_DESC(tests_test_validations_size_compare, file1,        NULL, test_set_validation_file1, NULL) \
CHILD_NODE_DESC(tests_test_validations_size_compare, file2,        NULL, test_set_validation_file2, NULL) \
CHILD_NODE_DESC(tests_test_validations_size_compare, val,          NULL, test_set_validation_data_len, NULL) \
\
CHILD_NODE_DESC(tests_test_validations_retcode_compare, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_retcode_compare, type,         NULL, test_set_compare_type, NULL) \
CHILD_NODE_DESC(tests_test_validations_retcode_compare, svc_chain,    NULL, test_set_validation_svc_chain, NULL) \
CHILD_NODE_DESC(tests_test_validations_retcode_compare, retcode,      NULL, test_set_validation_retcode, NULL) \
CHILD_NODE_DESC(tests_test_validations_retcode_compare, req_retcode,  NULL, test_set_validation_req_retcode, NULL) \
CHILD_NODE_DESC(tests_test_validations_retcode_compare, svc_retcodes, NULL, test_set_validation_svc_retcodes, NULL) \
\
CHILD_NODE_DESC(tests_test_validations_data_len_compare, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_len_compare, type,         NULL, test_set_compare_type, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_len_compare, svc_chain,    NULL, test_set_validation_svc_chain, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_len_compare, val,          NULL, test_set_validation_data_len, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_len_compare, svc_vals,     NULL, test_set_validation_svc_retcodes, NULL) \
\
CHILD_NODE_DESC(tests_test_validations_data_input_compare, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_input_compare, type,         NULL, test_set_compare_type, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_input_compare, svc_chain,    NULL, test_set_validation_svc_chain, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_input_compare, file1,        NULL, test_set_validation_file1, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_input_compare, pattern,      NULL, test_set_validation_pattern, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_input_compare, offset,       NULL, test_set_validation_data_offset, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_input_compare, len,          NULL, test_set_validation_data_len, NULL) \
\
CHILD_NODE_DESC(tests_test_validations_data_output_compare, idx,          NULL, test_set_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, type,         NULL, test_set_compare_type, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, svc_chain,    NULL, test_set_validation_svc_chain, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, svc_idx,      NULL, test_set_validation_svc_idx, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, file1,        NULL, test_set_validation_file1, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, pattern,      NULL, test_set_validation_pattern, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, offset,       NULL, test_set_validation_data_offset, NULL) \
CHILD_NODE_DESC(tests_test_validations_data_output_compare, len,          NULL, test_set_validation_data_len, NULL) \
\
CHILD_NODE_DESC(cp_hdr_formats, format,        test_create_cp_hdr_format, NULL, NULL) \
CHILD_NODE_DESC(cp_hdr_formats_format, idx,           NULL,  test_set_idx, NULL) \
CHILD_NODE_DESC(cp_hdr_formats_format, cp_hdr_fields, NULL, NULL, NULL) \
\
CHILD_NODE_DESC(cp_hdr_formats_format_cp_hdr_fields,  field, test_create_cp_hdr_field, NULL, NULL) \
CHILD_NODE_DESC(cp_hdr_formats_format_cp_hdr_fields_field, type,   NULL, test_set_cp_hdr_type, NULL) \
CHILD_NODE_DESC(cp_hdr_formats_format_cp_hdr_fields_field, offset, NULL, test_set_cp_hdr_offset, NULL) \
CHILD_NODE_DESC(cp_hdr_formats_format_cp_hdr_fields_field, len,    NULL, test_set_cp_hdr_length, NULL) \
CHILD_NODE_DESC(cp_hdr_formats_format_cp_hdr_fields_field, val,    NULL, test_set_cp_hdr_val, NULL) \
\
CHILD_NODE_DESC(cp_hdr_mapping, entry, test_create_cp_hdr_mapping, NULL, NULL) \
CHILD_NODE_DESC(cp_hdr_mapping_entry, pnso_algo, NULL, test_set_cp_hdr_pnso_algo, NULL) \
CHILD_NODE_DESC(cp_hdr_mapping_entry, hdr_algo,  NULL, test_set_cp_hdr_algo, NULL) \

/* Actual node definition */
ROOT_NODE_DESC_LIST
CHILD_NODE_DESC_LIST

#undef ROOT_NODE_DESC
#define ROOT_NODE_DESC(name) & name##_node_desc,
#undef CHILD_NODE_DESC
#define CHILD_NODE_DESC(parent, name, create_fn, set_fn, opaque) & parent##_##name##_node_desc,


/* Master list of root YAML node pointers */
static struct test_yaml_node_desc *root_node_descs[] = {
	ROOT_NODE_DESC_LIST

	/* Must be last */
	NULL
};

static struct test_yaml_node_desc *child_node_descs[] = {
	CHILD_NODE_DESC_LIST

	/* Must be last */
	NULL
};

static void test_init_parser(void)
{
	size_t i;
	struct test_yaml_node_desc *yaml_desc;

	/* Initialize child and sibling pointers at start time */

	/* Clear any previous init */
	for (i = 0; ; i++) {
		yaml_desc = root_node_descs[i];
		if (!yaml_desc)
			break;
		yaml_desc->siblings = NULL;
		yaml_desc->children = NULL;
	}
	for (i = 0; ; i++) {
		yaml_desc = child_node_descs[i];
		if (!yaml_desc)
			break;
		yaml_desc->siblings = NULL;
		yaml_desc->children = NULL;
	}

	/* Initialize children and siblings */
	for (i = 0; ; i++) {
		yaml_desc = child_node_descs[i];
		if (!yaml_desc)
			break;
		yaml_desc->siblings = yaml_desc->parent->children;
		yaml_desc->parent->children = yaml_desc;
	}
}

static void dump_yaml_desc_children(struct test_yaml_node_desc *parent, uint32_t indent_len)
{
	struct test_yaml_node_desc *yaml_desc;
	char indent[32];

	indent_len = indent_len < sizeof(indent) ? indent_len : sizeof(indent)-1;
	memset(indent, ' ', indent_len);
	indent[indent_len] = '\0';

	for (yaml_desc = parent->children; yaml_desc; yaml_desc = yaml_desc->siblings) {
		PNSO_LOG("%s%s\n", indent, yaml_desc->name);
		if (yaml_desc->children)
			dump_yaml_desc_children(yaml_desc, indent_len+2);
	}
}

void test_dump_yaml_desc_tree(void)
{
	size_t i;
	struct test_yaml_node_desc *yaml_desc;

	for (i = 0; ; i++) {
		yaml_desc = root_node_descs[i];
		if (!yaml_desc)
			break;
		PNSO_LOG("  %s\n", yaml_desc->name);
		if (yaml_desc->children)
			dump_yaml_desc_children(yaml_desc, 4);
	}
}

static struct test_yaml_node_desc *lookup_yaml_root_node_desc(const char *name)
{
	size_t i;
	struct test_yaml_node_desc *node_desc;

	for (i = 0; ; i++) {
		node_desc = root_node_descs[i];
		if (!node_desc)
			break;
		if (0 == safe_strcmp(node_desc->name, name)) {
			PNSO_LOG_TRACE("Lookup YAML root desc SUCCESS, root.%s\n",
				       name);
			return node_desc;
		}
	}
	PNSO_LOG_WARN("Lookup YAML node desc FAIL, root.%s\n",
		      name);

	return NULL;
}

static struct test_yaml_node_desc *lookup_yaml_node_desc(struct test_yaml_node_desc *parent,
							 const char *name)
{
	struct test_yaml_node_desc *node_desc;

	/* TODO: alphabetize node_descs and use binary search */

	if (!parent)
		return lookup_yaml_root_node_desc(name);

	for (node_desc = parent->children; node_desc; node_desc = node_desc->siblings) {
		if (0 == safe_strcmp(node_desc->name, name)) {
			/* Found */
			PNSO_LOG_TRACE("Lookup YAML node desc SUCCESS, %s.%s\n",
				       parent->name, name);
			return node_desc;
		}
	}

	PNSO_LOG_WARN("Lookup YAML node desc FAIL, %s.%s\n",
		      parent->name, name);

	return NULL;
}

static pnso_error_t test_create_node(struct test_desc *root,
				     struct test_node **parent,
				     struct test_yaml_node_desc *node_desc)
{
	if (!parent || !*parent) {
		/* No way to link new node, even if we wanted to */
		PNSO_LOG_DEBUG("Cannot create node of type %s, no parent.\n",
			       node_desc->name);
		return PNSO_OK;
	}

	return node_desc->start_fn(root, parent, node_desc->opaque);
}

static pnso_error_t parse_yaml_misc_event(yaml_parser_t *parser, yaml_event_t *event,
		struct test_desc *root, struct test_node *parent,
		struct test_yaml_node_desc *parent_yaml_desc,
		int *stream_done)
{
	pnso_error_t err = PNSO_OK;
	//struct test_node *new_node = NULL;

	switch (event->type) {
	case YAML_NO_EVENT:
	case YAML_STREAM_END_EVENT:
		*stream_done = 1;
		break;
	default:
		break;
	}

	return err;
}

static pnso_error_t parse_yaml_scalar1(yaml_parser_t *parser,
		yaml_event_t *event,
		struct test_desc *root,
		struct test_node **parent,
		struct test_yaml_node_desc **parent_yaml_desc)
{
	pnso_error_t err = PNSO_OK;
	struct test_yaml_node_desc *yaml_node_desc;

	/* Create a new node if necessary */
	yaml_node_desc = lookup_yaml_node_desc(*parent_yaml_desc,
					  (const char*)event->data.scalar.value);
	if (yaml_node_desc) {
		*parent_yaml_desc = yaml_node_desc;
		if (yaml_node_desc->start_fn) {
			err = test_create_node(root, parent, yaml_node_desc);
		}
	} else {
		*parent_yaml_desc = NULL;
		*parent = NULL;
	}

	return err;
}

static pnso_error_t parse_yaml_scalar2(yaml_parser_t *parser,
		yaml_event_t *event,
		struct test_desc *root,
		struct test_node *parent,
		struct test_yaml_node_desc *parent_yaml_desc)
{
	pnso_error_t err = PNSO_OK;

	/* Set parameter value if necessary */
	if (parent_yaml_desc && parent_yaml_desc->set_fn) {
		err = parent_yaml_desc->set_fn(root, parent,
					       (const char*)event->data.scalar.value);
	}

	return err;
}


static pnso_error_t parse_yaml_event_loop(yaml_parser_t *parser, yaml_event_type_t stop_event,
					  struct test_desc *root, struct test_node *parent,
					  struct test_yaml_node_desc *parent_yaml_desc,
					  int *stream_done, int stack_depth)
{
	pnso_error_t err = PNSO_OK;
	yaml_event_t events[2];
	yaml_event_t *prev_event = NULL, *event = NULL;
	struct test_yaml_node_desc *new_parent_yaml_desc = parent_yaml_desc;
	struct test_node *new_parent = parent;
	int event_idx = 0;
	int block_done = 0;

	if (stack_depth > PNSO_TEST_MAX_STACK_DEPTH) {
		PNSO_LOG_ERROR("Failed to parse YAML input, depth %d too high.\n",
			       stack_depth);
		err = EINVAL;
		goto done;
	}

	/* Loop through events until hitting some stop condition */
	while (!*stream_done && !block_done) {
		/* Get next event */
		event = &events[event_idx];
		if (!yaml_parser_parse(parser, event)) {
			*stream_done = 1;
			event = NULL;
			goto done;
		}
		event_idx = !event_idx;
		if (OSAL_LOG_ON(OSAL_LOG_LEVEL_DEBUG)) {
			dump_yaml_event(event, stack_depth);
		}

		/* Check for stop condition */
		if (event->type == stop_event) {
			block_done = 1;
		}

		/* Process event */
		switch (event->type) {
		case YAML_SCALAR_EVENT:
			if (prev_event && prev_event->type == YAML_SCALAR_EVENT) {
				/* New scalar contains value of the prev scalar */
				err = parse_yaml_scalar2(parser, event, root,
							 new_parent,
							 new_parent_yaml_desc);
				yaml_event_delete(prev_event);
				prev_event = NULL;
				yaml_event_delete(event);
				event = NULL;
			} else {
				new_parent = parent;
				new_parent_yaml_desc = parent_yaml_desc;
				err = parse_yaml_scalar1(parser, event, root,
							 &new_parent,
							 &new_parent_yaml_desc);
			}
			break;

		case YAML_MAPPING_START_EVENT:
			err = parse_yaml_event_loop(parser,
						    YAML_MAPPING_END_EVENT,
						    root, new_parent,
						    new_parent_yaml_desc,
						    stream_done,
						    stack_depth + 1);
			break;

		case YAML_SEQUENCE_START_EVENT:
			err = parse_yaml_event_loop(parser,
						    YAML_SEQUENCE_END_EVENT,
						    root, new_parent,
						    new_parent_yaml_desc,
						    stream_done,
						    stack_depth + 1);
			break;

		default:
			/* All other types of events fall here */
			err = parse_yaml_misc_event(parser, event,
						    root, parent,
						    parent_yaml_desc,
						    stream_done);
			break;
		}

		if (prev_event) {
			yaml_event_delete(prev_event);
		}
		prev_event = event;
		event = NULL;
		if (err) {
			break;
		}
	}

done:
	if (prev_event) {
		yaml_event_delete(prev_event);
	}
	if (event) {
		yaml_event_delete(event);
	}
	return err;
}

struct test_desc *pnso_test_desc_alloc(void)
{
	struct test_desc *desc;

	/* Allocate top-level test descriptor */
	desc = (struct test_desc *) TEST_ALLOC(sizeof(*desc));
	if (!desc) {
		return NULL;
	}
	memset(desc, 0, sizeof(*desc));

	/* Pre-populate global default values */
	*desc = default_desc;

	return desc;
}

void pnso_test_desc_free(struct test_desc *desc)
{
	test_free_desc(desc);
}

pnso_error_t pnso_test_parse_buf(const unsigned char *buf, size_t buf_len,
				 struct test_desc *desc)
{
	pnso_error_t err = PNSO_OK;
	yaml_parser_t parser;
	int stream_done;

	PNSO_LOG_TRACE("Parsing %zu byte buffer:\n%s\n", buf_len, buf);

	/* Initialize YAML parser */
	memset(&parser, 0, sizeof(parser));
	if (!yaml_parser_initialize(&parser)) {
		PNSO_LOG_ERROR("Failed to init YAML parser.\n");
		return ENOMEM;
	}
	yaml_parser_set_input_string(&parser, buf, buf_len);

	/* Extract fields and allocate private struct */
	stream_done = 0;
	err = parse_yaml_event_loop(&parser, YAML_STREAM_END_EVENT, desc,
				    &desc->node, NULL, &stream_done, 1);
	if (err) {
		goto error;
	}
	if (parser.error) {
		PNSO_LOG_ERROR("YAML parse error %u at line %zu: %s %s\n",
			       parser.error, parser.problem_mark.line,
			       parser.problem, parser.context);
		err = EINVAL;
		goto error;
	}
	yaml_parser_delete(&parser);

	return PNSO_OK;

error:
	PNSO_LOG_ERROR("Failed to parse YAML.\n");
	yaml_parser_delete(&parser);

	return err;
}

pnso_error_t pnso_test_parse_file(const char *fname, struct test_desc *desc)
{
	pnso_error_t err;
#ifndef __KERNEL__
	uint8_t *buf;
	uint32_t len = 0;

	buf = test_alloc_and_read_file(fname, 0, &len);
	if (!buf) {
		PNSO_LOG_ERROR("Cannot read YAML file %s\n", fname);
		return ENOMEM;
	}

	err = pnso_test_parse_buf(buf, len, desc);
	TEST_FREE(buf);
#else
	err = EINVAL;
#endif

	return err;
}

#define TEST_MAX_VALIDATION_STAT_LEN 256
static uint32_t validation_stats_to_yaml(const struct test_validation *validation, char *dst)
{
	uint32_t len = 0;
	uint32_t max_len = TEST_MAX_VALIDATION_STAT_LEN;

	if (validation->type  >= VALIDATION_TYPE_MAX)
		return 0;

	len += safe_strcpy(dst+len, "    { \"", max_len-len);
	len += safe_strcpy_tolower(dst+len, validation_type_to_name_list[validation->type], max_len-len);
	len += safe_strcpy(dst+len, "\": {\n", max_len-len);

	len += safe_strcpy(dst+len, "        \"idx\": ", max_len-len);
	len += safe_itoa(dst+len, max_len-len, validation->node.idx);
	len += safe_strcpy(dst+len, ",\n", max_len-len);

	len += safe_strcpy(dst+len, "        \"success\": ", max_len-len);
	len += safe_itoa(dst+len, max_len-len, validation->rt_success_count);
	len += safe_strcpy(dst+len, ",\n", max_len-len);

	len += safe_strcpy(dst+len, "        \"failure\": ", max_len-len);
	len += safe_itoa(dst+len, max_len-len, validation->rt_failure_count);

	if (validation->rt_reason[0] != '\0') {
		len += safe_strcpy(dst+len, ",\n        \"reason\": \"", max_len-len);
		len += safe_strcpy(dst+len, validation->rt_reason, max_len - len);
		len += safe_strcpy(dst+len, "\"", max_len-len);
	}

	len += safe_strcpy(dst+len, "\n    }},\n", max_len-len);

	return len;
}

#define TEST_MAX_STAT_NAME_LEN 32
pnso_error_t pnso_test_stats_to_yaml(const struct test_testcase *testcase,
		uint64_t *stats, const char **stats_names, uint32_t stat_count,
		bool output_validations, void *opaque)
{
	uint32_t i;
	uint32_t len = 0;
	uint32_t max_len = 64 + TEST_MAX_NAME_LEN + stat_count*(32 + TEST_MAX_STAT_NAME_LEN);
	char *dst;

	if (output_validations) {
		max_len += TEST_MAX_VALIDATION_STAT_LEN *
				test_count_nodes(&testcase->req_validations);
		max_len += TEST_MAX_VALIDATION_STAT_LEN *
				test_count_nodes(&testcase->batch_validations);
	}

	dst = TEST_ALLOC(max_len);
	if (!dst)
		return ENOMEM;

	len += safe_strcpy(dst+len, "{\"tests\": [{ \"test\": {\n  \"idx\": ", max_len-len);
	len += safe_itoa(dst+len, max_len-len, testcase->node.idx);
	len += safe_strcpy(dst+len, ",\n", max_len-len);
	if (testcase->name[0]) {
		len += safe_strcpy(dst+len, "  \"name\": \"", max_len-len);
		len += safe_strcpy(dst+len, testcase->name, max_len-len);
		len += safe_strcpy(dst+len, "\",\n", max_len-len);
	}
	len += safe_strcpy(dst+len, "  \"stats\": {\n", max_len-len);
	for (i = 0; i < stat_count; i++) {
		if (len >= max_len-1)
			goto nomem;
		len += safe_strcpy(dst+len, "    \"", max_len-len);
		len += safe_strcpy(dst+len, stats_names[i], TEST_MAX_STAT_NAME_LEN);
		len += safe_strcpy(dst+len, "\": ", max_len-len);
		len += safe_itoa(dst+len, max_len-len, stats[i]);
		if (i < stat_count-1) {
			len += safe_strcpy(dst+len, ",\n", max_len-len);
		} else {
			/* Last stat, no comma needed */
			len += safe_strcpy(dst+len, "\n", max_len-len);
		}
	}
	if (len >= max_len-1)
		goto nomem;

	if (output_validations &&
	    (testcase->req_validations.head ||
	     testcase->batch_validations.head)) {
		struct test_node *node;

		len += safe_strcpy(dst+len, "  },\n", max_len-len);
		len += safe_strcpy(dst+len, "  \"validations\": [\n", max_len-len);
		FOR_EACH_NODE(testcase->req_validations) {
			if (len+TEST_MAX_VALIDATION_STAT_LEN >= max_len-1)
				goto nomem;
			len += validation_stats_to_yaml((const struct test_validation *)(node), dst+len);
		}
		FOR_EACH_NODE(testcase->batch_validations) {
			if (len+TEST_MAX_VALIDATION_STAT_LEN >= max_len-1)
				goto nomem;
			len += validation_stats_to_yaml((const struct test_validation *)(node), dst+len);
		}
		/* remove last ",\n" */
		len -= 2;
		len += safe_strcpy(dst+len, "\n  ]\n", max_len-len);
	} else {
		len += safe_strcpy(dst+len, "  }\n", max_len-len);
	}

	if (len >= max_len-1)
		goto nomem;

	len += safe_strcpy(dst+len, "}}]}\n", max_len-len);

	if (len >= max_len-1)
		goto nomem;

	g_hooks.status_output(dst, opaque);

	TEST_FREE(dst);
	return PNSO_OK;

nomem:
	TEST_FREE(dst);
	return ENOMEM;
}

