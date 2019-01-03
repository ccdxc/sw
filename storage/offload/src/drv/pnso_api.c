/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_sys.h"
#include "osal_setup.h"

#include "pnso_api.h"
#include "pnso_crypto.h"
#include "pnso_cpdc_cmn.h"

/*
 * NOTE/TODO:
 *	This file is taken from simulator components temporarily
 *	and needs to addressed/tailored eventually.
 *
 */

#define PNSO_MAX_CP_HEADER_FORMATS	8

/* index is the pnso_algo, value is the hdr_algo */
uint32_t g_algo_map[PNSO_COMPRESSION_TYPE_MAX] = { 0, 0 };

struct cp_header_format g_cp_hdr_formats[PNSO_MAX_CP_HEADER_FORMATS];

static void
tlv_to_buf(uint8_t *dst, uint32_t len, uint64_t val)
{
	switch (len) {
	case 1:
		*dst = (uint8_t) val;
		break;
	case 2:
		*(uint16_t *)dst = (uint16_t) val;
		break;
	case 4:
		*(uint32_t *)dst = (uint32_t) val;
		break;
	case 8:
		*(uint64_t *)dst = val;
		break;
	default:
		/* TODO */
		break;
	}
}

struct cp_header_format *
lookup_hdr_format(uint32_t hdr_fmt_idx, bool alloc)
{
	size_t i;
	struct cp_header_format *fmt;
	struct cp_header_format *vacant_fmt = NULL;

	if (!hdr_fmt_idx)
		return NULL;

	/* First try to find entry at the given index */
	if (hdr_fmt_idx <= PNSO_MAX_CP_HEADER_FORMATS) {
		fmt = &g_cp_hdr_formats[hdr_fmt_idx - 1];
		if (fmt->fmt_idx == hdr_fmt_idx)
			return fmt;

		if (fmt->fmt_idx == 0)
			vacant_fmt = fmt;
	}

	/* Second, search all entries */
	for (i = 0; i < PNSO_MAX_CP_HEADER_FORMATS; i++) {
		fmt = &g_cp_hdr_formats[i];
		if (fmt->fmt_idx == hdr_fmt_idx)
			return fmt;

		if (fmt->fmt_idx == 0 && !vacant_fmt)
			vacant_fmt = fmt;
	}

	if (alloc)
		return vacant_fmt;

	return NULL;
}

static void
set_algo_mapping(enum pnso_compression_type pnso_algo, uint32_t hdr_algo)
{
	g_algo_map[pnso_algo] = hdr_algo;
}

pnso_error_t
pnso_register_compression_header_format(
		struct pnso_compression_header_format *cp_hdr_fmt,
		uint16_t hdr_fmt_idx)
{
	size_t i, total_hdr_len;
	struct cp_header_format *format;

	/* Basic validation */
	if (!hdr_fmt_idx)
		return PNSO_ERR_CPDC_HDR_IDX_INVALID;

	if (!cp_hdr_fmt || cp_hdr_fmt->num_fields > PNSO_MAX_HEADER_FIELDS)
		return EINVAL;

	/* Find a suitable table entry */
	format = lookup_hdr_format(hdr_fmt_idx, true);
	if (!format)
		return ENOMEM;

	/* Fill the entry */
	format->fmt_idx = hdr_fmt_idx;
	format->fmt = *cp_hdr_fmt;

	/* Find the total header length */
	total_hdr_len = 0;
	for (i = 0; i < cp_hdr_fmt->num_fields; i++) {
		uint32_t tmp = cp_hdr_fmt->fields[i].offset +
			       cp_hdr_fmt->fields[i].length;
		if (tmp > total_hdr_len)
			total_hdr_len = tmp;
	}

	/* Allocate static header */
	if (format->static_hdr && (total_hdr_len > format->total_hdr_sz)) {
		/* Avoid memory leak of previously allocated format */
		osal_free(format->static_hdr);
		format->static_hdr = NULL;
		format->total_hdr_sz = 0;
	}

	if (total_hdr_len && !format->static_hdr) {
		format->static_hdr = osal_alloc(total_hdr_len);
		if (!format->static_hdr)
			return ENOMEM;
	}
	format->total_hdr_sz = total_hdr_len;

	/* Populate static header and store aggregate flags */
	format->type_mask = 0;
	memset(format->static_hdr, 0, total_hdr_len);
	for (i = 0; i < cp_hdr_fmt->num_fields; i++) {
		struct pnso_header_field *tlv = &cp_hdr_fmt->fields[i];

		tlv_to_buf(format->static_hdr+tlv->offset, tlv->length,
			       tlv->value);
		format->type_mask |= 1 << tlv->type;
	}

	return PNSO_OK;
}
OSAL_EXPORT_SYMBOL(pnso_register_compression_header_format);

/* Assumes mapping is 1:1 */
pnso_error_t
pnso_add_compression_algo_mapping(enum pnso_compression_type pnso_algo,
		uint32_t header_algo)
{
	if (pnso_algo >= PNSO_COMPRESSION_TYPE_MAX)
		return EINVAL;

	set_algo_mapping(pnso_algo, header_algo);

	return PNSO_OK;
}
OSAL_EXPORT_SYMBOL(pnso_add_compression_algo_mapping);

pnso_error_t pnso_set_key_desc_idx(const void *key1,
				   const void *key2,
				   uint32_t key_size,
				   uint32_t key_idx)
{
	return crypto_key_index_update(key1, key2, key_size, key_idx);
}
OSAL_EXPORT_SYMBOL(pnso_set_key_desc_idx);
