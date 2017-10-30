/* dhcp.h

   Protocol structures... */

/*
 * Copyright (c) 2011-2012,2014 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 2004-2009 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1995-2003 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   https://www.isc.org/
 *
 */

#ifndef DHCP_INTERNAL_H
#define DHCP_INTERNAL_H
#include <isc/result.h>
#include <isc/resultclass.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <time.h>

#define log_error HAL_TRACE_ERR
#define log_info HAL_TRACE_INFO
#define log_debug HAL_TRACE_DEBUG
#define log_fatal HAL_TRACE_ERR

struct hash_table;
#include "nic/third-party/isc-dhcp/includes/omapip/hash.h"
typedef struct hash_table group_hash_t;
typedef struct hash_table universe_hash_t;
typedef struct hash_table option_name_hash_t;
typedef struct hash_table option_code_hash_t;
#include "nic/third-party/isc-dhcp/includes/omapip/alloc.h"
#include "nic/third-party/isc-dhcp/includes/tree.h"

#define DHCP_UDP_OVERHEAD \
    (20 + /* IP header */ \
     8)   /* UDP header */
#define DHCP_SNAME_LEN 64
#define DHCP_FILE_LEN 128
#define DHCP_FIXED_NON_UDP 236
#define DHCP_FIXED_LEN (DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
/* Everything but options. */
#define BOOTP_MIN_LEN 300

#define DHCP_MTU_MAX 1500
#define DHCP_MTU_MIN            576

#define DHCP_MAX_OPTION_LEN (DHCP_MTU_MAX - DHCP_FIXED_LEN)
#define DHCP_MIN_OPTION_LEN     (DHCP_MTU_MIN - DHCP_FIXED_LEN)

#define HARDWARE_ADDR_LEN 20
#if !defined (BYTE_NAME_HASH_SIZE)
# define BYTE_NAME_HASH_SIZE	401	/* Default would be ridiculous. */
#endif
#if !defined (BYTE_CODE_HASH_SIZE)
# define BYTE_CODE_HASH_SIZE	254	/* Default would be ridiculous. */
#endif

#define dmalloc_reuse(x, y, l, z)

struct hash_table;
typedef struct hash_table group_hash_t;
typedef struct hash_table universe_hash_t;
typedef struct hash_table option_name_hash_t;
typedef struct hash_table option_code_hash_t;
typedef struct hash_table dns_zone_hash_t;
typedef struct hash_table lease_ip_hash_t;
typedef struct hash_table lease_id_hash_t;
typedef struct hash_table host_hash_t;
typedef struct hash_table class_hash_t;

struct option_cache {
    int refcnt;
    struct option_cache *next;
    struct expression *expression;
    struct option *option;
    struct data_string data;

#define OPTION_HAD_NULLS 0x00000001
    u_int32_t flags;
};

int packet_allocate (struct packet **, const char *, int);
int packet_reference (struct packet **,
		      struct packet *, const char *, int);
int packet_dereference (struct packet **, const char *, int);
int option_state_allocate (struct option_state **, const char *, int);
int option_state_reference (struct option_state **,
			    struct option_state *, const char *, int);
int option_state_dereference (struct option_state **,
			      const char *, int);
int parse_option_buffer(struct option_state *, const unsigned char *, unsigned,
                        struct universe *);
struct universe *find_option_universe(struct option *, const char *);
int parse_encapsulated_suboptions(struct option_state *, struct option *,
                                  const unsigned char *, unsigned,
                                  struct universe *, const char *);


int data_string_new(struct data_string *, const char *, unsigned int,
		    const char *, int);
void data_string_copy(struct data_string *, const struct data_string *,
		      const char *, int);
void data_string_forget (struct data_string *, const char *, int);
void data_string_truncate (struct data_string *, int);
int option_cache_allocate (struct option_cache **, const char *, int);
int option_cache_reference (struct option_cache **,
			    struct option_cache *, const char *, int);
void * dmalloc (unsigned, const char *, int);
//void  dmalloc_reuse (void *, const char*, int, int);
void dfree (void *, const char *, int);
#if defined (DEBUG_MEMORY_LEAKAGE) || defined (DEBUG_MALLOC_POOL) || \
		defined (DEBUG_MEMORY_LEAKAGE_ON_EXIT)
void dmalloc_reuse (void *, const char *, int, int);
int option_chain_head_allocate (struct option_chain_head **,
				const char *, int);
#endif
int option_chain_head_reference (struct option_chain_head **,
				 struct option_chain_head *,
				 const char *, int);
int option_chain_head_dereference (struct option_chain_head **,
				   const char *, int);
int option_cache_reference(struct option_cache **ptr, struct option_cache *src,
                           const char *file, int line);
int option_cache_dereference (struct option_cache **,
			      const char *, int);

struct option *new_option (const char *, const char *, int);
int option_reference(struct option **dest, struct option *src,
		     const char * file, int line);
int option_dereference(struct option **dest, const char *file, int line);

struct option_cache *lookup_option (struct universe *,
				    struct option_state *, unsigned);
struct option_cache *lookup_hashed_option (struct universe *,
					   struct option_state *,
					   unsigned);

void expression_dereference (struct expression **, const char *, int);
void save_hashed_option(struct universe *, struct option_state *,
			struct option_cache *, isc_boolean_t appendp);
void delete_option (struct universe *, struct option_state *, int);
void delete_hashed_option (struct universe *,
			   struct option_state *, int);
void hashed_option_space_foreach (struct packet *, struct lease *,
				  struct client_state *,
				  struct option_state *,
				  struct option_state *,
				  struct binding_scope **,
				  struct universe *, void *,
				  void (*) (struct option_cache *,
					    struct packet *,
					    struct lease *,
					    struct client_state *,
					    struct option_state *,
					    struct option_state *,
					    struct binding_scope **,
					    struct universe *, void *));
int hashed_option_state_dereference (struct universe *,
				     struct option_state *,
				     const char *, int);
int hashed_option_space_encapsulate (struct data_string *,
				     struct packet *, struct lease *,
				     struct client_state *,
				     struct option_state *,
				     struct option_state *,
				     struct binding_scope **,
				     struct universe *);
u_int32_t getUChar (const unsigned char *);
void putUChar (unsigned char *, u_int32_t);
int buffer_allocate(struct buffer **ptr, unsigned len, const char *file,
                    int line);
int buffer_reference(struct buffer **ptr, struct buffer *bp, const char *file,
                     int line);
int buffer_dereference(struct buffer **ptr, const char *file, int line);
HASH_FUNCTIONS_DECL (option_code, const unsigned *, struct option,
		     option_code_hash_t)
extern int universe_count, universe_max;
extern struct universe **universes;
extern struct universe dhcp_universe;
extern char *default_option_format;


void add_enumeration (struct enumeration *);
struct enumeration *find_enumeration (const char *, int);
struct enumeration_value *find_enumeration_value (const char *, int,
						  unsigned *,
						  const char *);
pair foo_new_pair (const char *, int);
void free_pair (pair, const char *, int);
int dns_host_entry_allocate (struct dns_host_entry **,
			     const char *, const char *, int);
int dns_host_entry_reference (struct dns_host_entry **,
			      struct dns_host_entry *,
			      const char *, int);
int dns_host_entry_dereference (struct dns_host_entry **,
				const char *, int);
void free_expression (struct expression *, const char *, int);

ssize_t decode_ethernet_header(unsigned char *buf, unsigned bufix,
                               struct hardware *from);

ssize_t decode_udp_ip_header(struct interface_info *interface,
                             unsigned char *buf, unsigned bufix,
                             struct sockaddr_in *from, unsigned buflen,
                             unsigned *rbuflen, int csum_ready);

int parse_options(struct packet *);

void initialize_common_option_spaces(void);


#endif /* DHCP_H */

