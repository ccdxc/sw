/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <string.h>
#include <ctype.h>
#include "osal_stdtypes.h"
#include "osal_random.h"
#include "osal_assert.h"
#include "pnso_api.h"
#include "pnso_test.h"
#include "pnso_pbuf.h"

#define pnso_srand osal_srand
#define pnso_rand osal_rand

uint32_t roundup_block_count(uint32_t len, uint32_t block_size)
{
	if (!block_size)
		block_size = 1;

	return (len + block_size - 1) / block_size;
}

uint32_t roundup_len(uint32_t len, uint32_t block_size)
{
	return roundup_block_count(len, block_size) * block_size;
}

/* fill buflist with a repeat of the given data */
void test_fill_buflist(struct pnso_buffer_list *buflist,
		       const uint8_t *data, uint32_t data_len)
{
	size_t i, j, data_i;
	uint8_t *dst;

	data_i = 0;
	for (i = 0; i < buflist->count; i++) {
		if (!buflist->buffers[i].len) {
			continue;
		}

		dst = (uint8_t *) buflist->buffers[i].buf;
		for (j = 0; j < buflist->buffers[i].len; j++) {
			dst[j] = (uint8_t) data[data_i % data_len];
			data_i++;
		}
	}
}

static uint8_t g_random_data[TEST_MAX_RANDOM_LEN];

static void test_fill_random_flat_buf(uint8_t *dst, uint32_t len)
{
	size_t i;
	uint32_t rnum = 0;

	for (i = 0; i < len; i++) {
		if ((i % 4) == 0)
			rnum = pnso_rand();
		dst[i] = ((uint8_t*)(&rnum))[i % 4];
	}
}

/* Fill buflist with random values */
pnso_error_t test_fill_random(struct pnso_buffer_list *buflist, uint32_t seed, uint32_t random_len)
{
	size_t i;

	pnso_srand(seed);

	if (random_len) {
		if (random_len > sizeof(g_random_data))
			random_len = sizeof(g_random_data);
		test_fill_random_flat_buf(g_random_data, random_len);
		test_fill_buflist(buflist, g_random_data, random_len);
		return PNSO_OK;
	}

	for (i = 0; i < buflist->count; i++) {
		test_fill_random_flat_buf((uint8_t *) buflist->buffers[i].buf,
					  buflist->buffers[i].len);
	}

	return PNSO_OK;
}

/* Convert binary to decimal ASCII, returning length */
uint32_t safe_itoa(char *dst, uint32_t dst_len, uint64_t val)
{
	uint32_t ret, len = 0;
	uint64_t tmp;

	if (!dst || !dst_len) {
		return 0;
	}
	if (val == 0) {
		dst[0] = '0';
		dst[1] = '\0';
		return 1;
	}

	/* First calculate length */
	tmp = val;
	while (tmp) {
		len++;
		tmp /= 10;
	}

	if (len >= dst_len) {
		*dst = '\0';
		return 0;
	}

	/* Convert to string */
	ret = len;
	tmp = val;
	dst[len] = '\0';
	while (len) {
		dst[--len] = '0' + (tmp % 10);
		tmp /= 10;
	}

	return ret;
}

#define SAFE_C2HEX(x) ((x) < 10 ? (x) + '0' : (x) - 10 + 'a')

/* convert binary string to hex, returning number of src bytes consumed */
uint32_t safe_bintohex(char *dst, uint32_t dst_len,
		       const uint8_t *src, uint32_t src_len)
{
	uint32_t src_offset, dst_offset;
	uint8_t c;

	if (dst == NULL || dst_len == 0)
		return 0;
	dst[0] = '\0';
	if (src == NULL || src_len == 0)
		return 0;

	dst_offset = 0;
	for (src_offset = 0; src_offset < src_len && dst_offset+1 < dst_len;
	     src_offset++) {
		c = src[src_offset];
		dst[dst_offset++] = SAFE_C2HEX(c >> 4);
		dst[dst_offset++] = SAFE_C2HEX(c & 0x0f);
	}

	dst[dst_offset] = '\0';
	return src_offset;
}

uint32_t safe_strcpy_tolower(char *dst, const char *src, uint32_t max_len)
{
	uint32_t len;

	if (dst == NULL)
		return 0;

	if (src == NULL) {
		*dst = '\0';
		return 0;
	}

	for (len = 0; src[len] != '\0' && len < (max_len -1); len++) {
		dst[len] = tolower(src[len]);
	}
	dst[len] = '\0';

	return len;
}

uint32_t safe_strcpy(char *dst, const char *src, uint32_t max_len)
{
	uint32_t len;

	if (dst == NULL)
		return 0;

	if (src == NULL) {
		*dst = '\0';
		return 0;
	}

	for (len = 0; src[len] != '\0' && len < (max_len -1); len++) {
		dst[len] = src[len];
	}
	dst[len] = '\0';

	return len;
}

/* String compare which allows NULL inputs */
/* Treat empty string and NULL string as equivalent */
int safe_strcmp(const char *str1, const char *str2)
{
        char empty_str[1] = "";

	if (!str1) {
		str1 = empty_str;
	}
	if (!str2) {
		str2 = empty_str;
	}

	return strcmp(str1, str2);
}

/* String compare which allows NULL inputs */
/* Treat empty string and NULL string as equivalent */
int safe_strncmp(const char *str1, const char *str2, uint32_t len)
{
        char empty_str[1] = "";

	if (!str1) {
		str1 = empty_str;
	}
	if (!str2) {
		str2 = empty_str;
	}

	return strncmp(str1, str2, len);
}

static inline unsigned int char2val(char c)
{
	if (c >= '0' && c <= '9')
		return (unsigned int) c - '0';
	if (c >= 'a' && c <= 'z')
		return (unsigned int) (10 + c - 'a');
	if (c >= 'A' && c <= 'Z')
		return (unsigned int) (10 + c - 'A');
	return 0;
}

static unsigned long long str_to_dec(const char *val)
{
	unsigned long long ret = 0;

	while (*val) {
		if (isdigit(*val))
			ret = 10*ret + char2val(*val);
		else
			break;
		val++;
	}
	return ret;
}

static unsigned long long str_to_hex(const char *val)
{
	unsigned long long ret = 0;

	while (*val) {
		if (isxdigit(*val))
			ret = 16*ret + char2val(*val);
		else
			break;
		val++;
	}
	return ret;
}
	 
unsigned long long safe_strtoll(const char *val)
{
	if (!val || !isdigit(*val)) {
		return -1;
	}

	if (val[0] == '0' &&
	    (val[1] == 'x' || val[1] == 'X')) {
		return str_to_hex(val+2);
	}

	return str_to_dec(val);
}
