/* Copyright (c) 2011-2014 PLUMgrid, http://plumgrid.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#ifndef __LKL__LINUX_BPF_H__
#define __LKL__LINUX_BPF_H__

#include <lkl/linux/types.h>
#include <lkl/linux/bpf_common.h>

/* Extended instruction set based on top of classic BPF */

/* instruction classes */
#define LKL_BPF_ALU64	0x07	/* alu mode in double word width */

/* ld/ldx fields */
#define LKL_BPF_DW		0x18	/* double word */
#define LKL_BPF_XADD	0xc0	/* exclusive add */

/* alu/jmp fields */
#define LKL_BPF_MOV		0xb0	/* mov reg to reg */
#define LKL_BPF_ARSH	0xc0	/* sign extending arithmetic shift right */

/* change endianness of a register */
#define LKL_BPF_END		0xd0	/* flags for endianness conversion: */
#define LKL_BPF_TO_LE	0x00	/* convert to little-endian */
#define LKL_BPF_TO_BE	0x08	/* convert to big-endian */
#define LKL_BPF_FROM_LE	LKL_BPF_TO_LE
#define LKL_BPF_FROM_BE	LKL_BPF_TO_BE

#define LKL_BPF_JNE		0x50	/* jump != */
#define LKL_BPF_JSGT	0x60	/* SGT is signed '>', GT in x86 */
#define LKL_BPF_JSGE	0x70	/* SGE is signed '>=', GE in x86 */
#define LKL_BPF_CALL	0x80	/* function call */
#define LKL_BPF_EXIT	0x90	/* function return */

/* Register numbers */
enum {
	LKL_BPF_REG_0 = 0,
	LKL_BPF_REG_1,
	LKL_BPF_REG_2,
	LKL_BPF_REG_3,
	LKL_BPF_REG_4,
	LKL_BPF_REG_5,
	LKL_BPF_REG_6,
	LKL_BPF_REG_7,
	LKL_BPF_REG_8,
	LKL_BPF_REG_9,
	LKL_BPF_REG_10,
	__LKL__MAX_BPF_REG,
};

/* BPF has 10 general purpose 64-bit registers and stack frame. */
#define LKL_MAX_BPF_REG	__LKL__MAX_BPF_REG

struct lkl_bpf_insn {
	__lkl__u8	code;		/* opcode */
	__lkl__u8	dst_reg:4;	/* dest register */
	__lkl__u8	src_reg:4;	/* source register */
	__lkl__s16	off;		/* signed offset */
	__lkl__s32	imm;		/* signed immediate constant */
};

/* Key of an a LKL_BPF_MAP_TYPE_LPM_TRIE entry */
struct lkl_bpf_lpm_trie_key {
	__lkl__u32	prefixlen;	/* up to 32 for LKL_AF_INET, 128 for LKL_AF_INET6 */
	__lkl__u8	data[0];	/* Arbitrary size */
};

/* BPF syscall commands, see bpf(2) man-page for details. */
enum lkl_bpf_cmd {
	LKL_BPF_MAP_CREATE,
	LKL_BPF_MAP_LOOKUP_ELEM,
	LKL_BPF_MAP_UPDATE_ELEM,
	LKL_BPF_MAP_DELETE_ELEM,
	LKL_BPF_MAP_GET_NEXT_KEY,
	LKL_BPF_PROG_LOAD,
	LKL_BPF_OBJ_PIN,
	LKL_BPF_OBJ_GET,
	LKL_BPF_PROG_ATTACH,
	LKL_BPF_PROG_DETACH,
};

enum lkl_bpf_map_type {
	LKL_BPF_MAP_TYPE_UNSPEC,
	LKL_BPF_MAP_TYPE_HASH,
	LKL_BPF_MAP_TYPE_ARRAY,
	LKL_BPF_MAP_TYPE_PROG_ARRAY,
	LKL_BPF_MAP_TYPE_PERF_EVENT_ARRAY,
	LKL_BPF_MAP_TYPE_PERCPU_HASH,
	LKL_BPF_MAP_TYPE_PERCPU_ARRAY,
	LKL_BPF_MAP_TYPE_STACK_TRACE,
	LKL_BPF_MAP_TYPE_CGROUP_ARRAY,
	LKL_BPF_MAP_TYPE_LRU_HASH,
	LKL_BPF_MAP_TYPE_LRU_PERCPU_HASH,
	LKL_BPF_MAP_TYPE_LPM_TRIE,
};

enum lkl_bpf_prog_type {
	LKL_BPF_PROG_TYPE_UNSPEC,
	LKL_BPF_PROG_TYPE_SOCKET_FILTER,
	LKL_BPF_PROG_TYPE_KPROBE,
	LKL_BPF_PROG_TYPE_SCHED_CLS,
	LKL_BPF_PROG_TYPE_SCHED_ACT,
	LKL_BPF_PROG_TYPE_TRACEPOINT,
	LKL_BPF_PROG_TYPE_XDP,
	LKL_BPF_PROG_TYPE_PERF_EVENT,
	LKL_BPF_PROG_TYPE_CGROUP_SKB,
	LKL_BPF_PROG_TYPE_CGROUP_SOCK,
	LKL_BPF_PROG_TYPE_LWT_IN,
	LKL_BPF_PROG_TYPE_LWT_OUT,
	LKL_BPF_PROG_TYPE_LWT_XMIT,
};

enum lkl_bpf_attach_type {
	LKL_BPF_CGROUP_INET_INGRESS,
	LKL_BPF_CGROUP_INET_EGRESS,
	LKL_BPF_CGROUP_INET_SOCK_CREATE,
	__LKL__MAX_BPF_ATTACH_TYPE
};

#define LKL_MAX_BPF_ATTACH_TYPE __LKL__MAX_BPF_ATTACH_TYPE

/* If LKL_BPF_F_ALLOW_OVERRIDE flag is used in LKL_BPF_PROG_ATTACH command
 * to the given target_fd cgroup the descendent cgroup will be able to
 * override effective bpf program that was inherited from this cgroup
 */
#define LKL_BPF_F_ALLOW_OVERRIDE	(1U << 0)

#define LKL_BPF_PSEUDO_MAP_FD	1

/* flags for LKL_BPF_MAP_UPDATE_ELEM command */
#define LKL_BPF_ANY		0 /* create new element or update existing */
#define LKL_BPF_NOEXIST	1 /* create new element if it didn't exist */
#define LKL_BPF_EXIST	2 /* update existing element */

#define LKL_BPF_F_NO_PREALLOC	(1U << 0)
/* Instead of having one common LRU list in the
 * BPF_MAP_TYPE_LRU_[PERCPU_]HASH map, use a percpu LRU list
 * which can scale and perform better.
 * Note, the LRU nodes (including free nodes) cannot be moved
 * across different LRU lists.
 */
#define LKL_BPF_F_NO_COMMON_LRU	(1U << 1)

union lkl_bpf_attr {
	struct { /* anonymous struct used by LKL_BPF_MAP_CREATE command */
		__lkl__u32	map_type;	/* one of enum lkl_bpf_map_type */
		__lkl__u32	key_size;	/* size of key in bytes */
		__lkl__u32	value_size;	/* size of value in bytes */
		__lkl__u32	max_entries;	/* max number of entries in a map */
		__lkl__u32	map_flags;	/* prealloc or not */
	};

	struct { /* anonymous struct used by BPF_MAP_*_ELEM commands */
		__lkl__u32		map_fd;
		__lkl__aligned_u64	key;
		union {
			__lkl__aligned_u64 value;
			__lkl__aligned_u64 next_key;
		};
		__lkl__u64		flags;
	};

	struct { /* anonymous struct used by LKL_BPF_PROG_LOAD command */
		__lkl__u32		prog_type;	/* one of enum lkl_bpf_prog_type */
		__lkl__u32		insn_cnt;
		__lkl__aligned_u64	insns;
		__lkl__aligned_u64	license;
		__lkl__u32		log_level;	/* verbosity level of verifier */
		__lkl__u32		log_size;	/* size of user buffer */
		__lkl__aligned_u64	log_buf;	/* user supplied buffer */
		__lkl__u32		kern_version;	/* checked when prog_type=kprobe */
	};

	struct { /* anonymous struct used by BPF_OBJ_* commands */
		__lkl__aligned_u64	pathname;
		__lkl__u32		bpf_fd;
	};

	struct { /* anonymous struct used by LKL_BPF_PROG_ATTACH/DETACH commands */
		__lkl__u32		target_fd;	/* container object to attach to */
		__lkl__u32		attach_bpf_fd;	/* eBPF program to attach */
		__lkl__u32		attach_type;
		__lkl__u32		attach_flags;
	};
} __attribute__((aligned(8)));

/* BPF helper function descriptions:
 *
 * void *bpf_map_lookup_elem(&map, &key)
 *     Return: Map value or NULL
 *
 * int bpf_map_update_elem(&map, &key, &value, flags)
 *     Return: 0 on success or negative error
 *
 * int bpf_map_delete_elem(&map, &key)
 *     Return: 0 on success or negative error
 *
 * int bpf_probe_read(void *dst, int size, void *src)
 *     Return: 0 on success or negative error
 *
 * lkl_u64 bpf_ktime_get_ns(void)
 *     Return: current ktime
 *
 * int bpf_trace_printk(const char *fmt, int fmt_size, ...)
 *     Return: length of buffer written or negative error
 *
 * lkl_u32 bpf_prandom_u32(void)
 *     Return: random value
 *
 * lkl_u32 bpf_raw_smp_processor_id(void)
 *     Return: SMP processor ID
 *
 * int bpf_skb_store_bytes(skb, offset, from, len, flags)
 *     store bytes into packet
 *     @skb: pointer to skb
 *     @offset: offset within packet from skb->mac_header
 *     @from: pointer where to copy bytes from
 *     @len: number of bytes to store into packet
 *     @flags: bit 0 - if true, recompute skb->csum
 *             other bits - reserved
 *     Return: 0 on success or negative error
 *
 * int bpf_l3_csum_replace(skb, offset, from, to, flags)
 *     recompute IP checksum
 *     @skb: pointer to skb
 *     @offset: offset within packet where IP checksum is located
 *     @from: old value of header field
 *     @to: new value of header field
 *     @flags: bits 0-3 - size of header field
 *             other bits - reserved
 *     Return: 0 on success or negative error
 *
 * int bpf_l4_csum_replace(skb, offset, from, to, flags)
 *     recompute TCP/UDP checksum
 *     @skb: pointer to skb
 *     @offset: offset within packet where TCP/UDP checksum is located
 *     @from: old value of header field
 *     @to: new value of header field
 *     @flags: bits 0-3 - size of header field
 *             bit 4 - is pseudo header
 *             other bits - reserved
 *     Return: 0 on success or negative error
 *
 * int bpf_tail_call(ctx, prog_array_map, index)
 *     jump into another BPF program
 *     @ctx: context pointer passed to next program
 *     @prog_array_map: pointer to map which type is LKL_BPF_MAP_TYPE_PROG_ARRAY
 *     @index: index inside array that selects specific program to run
 *     Return: 0 on success or negative error
 *
 * int bpf_clone_redirect(skb, ifindex, flags)
 *     redirect to another netdev
 *     @skb: pointer to skb
 *     @ifindex: ifindex of the net device
 *     @flags: bit 0 - if set, redirect to ingress instead of egress
 *             other bits - reserved
 *     Return: 0 on success or negative error
 *
 * lkl_u64 bpf_get_current_pid_tgid(void)
 *     Return: current->tgid << 32 | current->pid
 *
 * lkl_u64 bpf_get_current_uid_gid(void)
 *     Return: current_gid << 32 | current_uid
 *
 * int bpf_get_current_comm(char *buf, int size_of_buf)
 *     stores current->comm into buf
 *     Return: 0 on success or negative error
 *
 * lkl_u32 bpf_get_cgroup_classid(skb)
 *     retrieve a proc's classid
 *     @skb: pointer to skb
 *     Return: classid if != 0
 *
 * int bpf_skb_vlan_push(skb, vlan_proto, vlan_tci)
 *     Return: 0 on success or negative error
 *
 * int bpf_skb_vlan_pop(skb)
 *     Return: 0 on success or negative error
 *
 * int bpf_skb_get_tunnel_key(skb, key, size, flags)
 * int bpf_skb_set_tunnel_key(skb, key, size, flags)
 *     retrieve or populate tunnel metadata
 *     @skb: pointer to skb
 *     @key: pointer to 'struct lkl_bpf_tunnel_key'
 *     @size: size of 'struct lkl_bpf_tunnel_key'
 *     @flags: room for future extensions
 *     Return: 0 on success or negative error
 *
 * lkl_u64 bpf_perf_event_read(&map, index)
 *     Return: Number events read or error code
 *
 * int bpf_redirect(ifindex, flags)
 *     redirect to another netdev
 *     @ifindex: ifindex of the net device
 *     @flags: bit 0 - if set, redirect to ingress instead of egress
 *             other bits - reserved
 *     Return: TC_ACT_REDIRECT
 *
 * lkl_u32 bpf_get_route_realm(skb)
 *     retrieve a dst's tclassid
 *     @skb: pointer to skb
 *     Return: realm if != 0
 *
 * int bpf_perf_event_output(ctx, map, index, data, size)
 *     output perf raw sample
 *     @ctx: struct pt_regs*
 *     @map: pointer to perf_event_array map
 *     @index: index of event in the map
 *     @data: data on stack to be output as raw data
 *     @size: size of data
 *     Return: 0 on success or negative error
 *
 * int bpf_get_stackid(ctx, map, flags)
 *     walk user or kernel stack and return id
 *     @ctx: struct pt_regs*
 *     @map: pointer to stack_trace map
 *     @flags: bits 0-7 - numer of stack frames to skip
 *             bit 8 - collect user stack instead of kernel
 *             bit 9 - compare stacks by hash only
 *             bit 10 - if two different stacks hash into the same stackid
 *                      discard old
 *             other bits - reserved
 *     Return: >= 0 stackid on success or negative error
 *
 * lkl_s64 bpf_csum_diff(from, from_size, to, to_size, seed)
 *     calculate csum diff
 *     @from: raw from buffer
 *     @from_size: length of from buffer
 *     @to: raw to buffer
 *     @to_size: length of to buffer
 *     @seed: optional seed
 *     Return: csum result or negative error code
 *
 * int bpf_skb_get_tunnel_opt(skb, opt, size)
 *     retrieve tunnel options metadata
 *     @skb: pointer to skb
 *     @opt: pointer to raw tunnel option data
 *     @size: size of @opt
 *     Return: option size
 *
 * int bpf_skb_set_tunnel_opt(skb, opt, size)
 *     populate tunnel options metadata
 *     @skb: pointer to skb
 *     @opt: pointer to raw tunnel option data
 *     @size: size of @opt
 *     Return: 0 on success or negative error
 *
 * int bpf_skb_change_proto(skb, proto, flags)
 *     Change protocol of the skb. Currently supported is v4 -> v6,
 *     v6 -> v4 transitions. The helper will also resize the skb. eBPF
 *     program is expected to fill the new headers via skb_store_bytes
 *     and lX_csum_replace.
 *     @skb: pointer to skb
 *     @proto: new skb->protocol type
 *     @flags: reserved
 *     Return: 0 on success or negative error
 *
 * int bpf_skb_change_type(skb, type)
 *     Change packet type of skb.
 *     @skb: pointer to skb
 *     @type: new skb->pkt_type type
 *     Return: 0 on success or negative error
 *
 * int bpf_skb_under_cgroup(skb, map, index)
 *     Check cgroup2 membership of skb
 *     @skb: pointer to skb
 *     @map: pointer to bpf_map in LKL_BPF_MAP_TYPE_CGROUP_ARRAY type
 *     @index: index of the cgroup in the bpf_map
 *     Return:
 *       == 0 skb failed the cgroup2 descendant test
 *       == 1 skb succeeded the cgroup2 descendant test
 *        < 0 error
 *
 * lkl_u32 bpf_get_hash_recalc(skb)
 *     Retrieve and possibly recalculate skb->hash.
 *     @skb: pointer to skb
 *     Return: hash
 *
 * lkl_u64 bpf_get_current_task(void)
 *     Returns current task_struct
 *     Return: current
 *
 * int bpf_probe_write_user(void *dst, void *src, int len)
 *     safely attempt to write to a location
 *     @dst: destination address in userspace
 *     @src: source address on stack
 *     @len: number of bytes to copy
 *     Return: 0 on success or negative error
 *
 * int bpf_current_task_under_cgroup(map, index)
 *     Check cgroup2 membership of current task
 *     @map: pointer to bpf_map in LKL_BPF_MAP_TYPE_CGROUP_ARRAY type
 *     @index: index of the cgroup in the bpf_map
 *     Return:
 *       == 0 current failed the cgroup2 descendant test
 *       == 1 current succeeded the cgroup2 descendant test
 *        < 0 error
 *
 * int bpf_skb_change_tail(skb, len, flags)
 *     The helper will resize the skb to the given new size, to be used f.e.
 *     with control messages.
 *     @skb: pointer to skb
 *     @len: new skb length
 *     @flags: reserved
 *     Return: 0 on success or negative error
 *
 * int bpf_skb_pull_data(skb, len)
 *     The helper will pull in non-linear data in case the skb is non-linear
 *     and not all of len are part of the linear section. Only needed for
 *     read/write with direct packet access.
 *     @skb: pointer to skb
 *     @len: len to make read/writeable
 *     Return: 0 on success or negative error
 *
 * lkl_s64 bpf_csum_update(skb, csum)
 *     Adds csum into skb->csum in case of CHECKSUM_COMPLETE.
 *     @skb: pointer to skb
 *     @csum: csum to add
 *     Return: csum on success or negative error
 *
 * void bpf_set_hash_invalid(skb)
 *     Invalidate current skb->hash.
 *     @skb: pointer to skb
 *
 * int bpf_get_numa_node_id()
 *     Return: Id of current NUMA node.
 *
 * int bpf_skb_change_head()
 *     Grows headroom of skb and adjusts MAC header offset accordingly.
 *     Will extends/reallocae as required automatically.
 *     May change skb data pointer and will thus invalidate any check
 *     performed for direct packet access.
 *     @skb: pointer to skb
 *     @len: length of header to be pushed in front
 *     @flags: Flags (unused for now)
 *     Return: 0 on success or negative error
 *
 * int bpf_xdp_adjust_head(xdp_md, delta)
 *     Adjust the xdp_md.data by delta
 *     @xdp_md: pointer to xdp_md
 *     @delta: An positive/negative integer to be added to xdp_md.data
 *     Return: 0 on success or negative on error
 *
 * int bpf_probe_read_str(void *dst, int size, const void *unsafe_ptr)
 *     Copy a NUL terminated string from unsafe address. In case the string
 *     length is smaller than size, the target is not padded with further NUL
 *     bytes. In case the string length is larger than size, just count-1
 *     bytes are copied and the last byte is set to NUL.
 *     @dst: destination address
 *     @size: maximum number of bytes to copy, including the trailing NUL
 *     @unsafe_ptr: unsafe address
 *     Return:
 *       > 0 length of the string including the trailing NUL on success
 *       < 0 error
 */
#define __LKL__BPF_FUNC_MAPPER(FN)		\
	FN(unspec),			\
	FN(map_lookup_elem),		\
	FN(map_update_elem),		\
	FN(map_delete_elem),		\
	FN(probe_read),			\
	FN(ktime_get_ns),		\
	FN(trace_printk),		\
	FN(get_prandom_u32),		\
	FN(get_smp_processor_id),	\
	FN(skb_store_bytes),		\
	FN(l3_csum_replace),		\
	FN(l4_csum_replace),		\
	FN(tail_call),			\
	FN(clone_redirect),		\
	FN(get_current_pid_tgid),	\
	FN(get_current_uid_gid),	\
	FN(get_current_comm),		\
	FN(get_cgroup_classid),		\
	FN(skb_vlan_push),		\
	FN(skb_vlan_pop),		\
	FN(skb_get_tunnel_key),		\
	FN(skb_set_tunnel_key),		\
	FN(perf_event_read),		\
	FN(redirect),			\
	FN(get_route_realm),		\
	FN(perf_event_output),		\
	FN(skb_load_bytes),		\
	FN(get_stackid),		\
	FN(csum_diff),			\
	FN(skb_get_tunnel_opt),		\
	FN(skb_set_tunnel_opt),		\
	FN(skb_change_proto),		\
	FN(skb_change_type),		\
	FN(skb_under_cgroup),		\
	FN(get_hash_recalc),		\
	FN(get_current_task),		\
	FN(probe_write_user),		\
	FN(current_task_under_cgroup),	\
	FN(skb_change_tail),		\
	FN(skb_pull_data),		\
	FN(csum_update),		\
	FN(set_hash_invalid),		\
	FN(get_numa_node_id),		\
	FN(skb_change_head),		\
	FN(xdp_adjust_head),		\
	FN(probe_read_str),

/* integer value in 'imm' field of LKL_BPF_CALL instruction selects which helper
 * function eBPF program intends to call
 */
#define __LKL__BPF_ENUM_FN(x) BPF_FUNC_ ## x
enum lkl_bpf_func_id {
	__LKL__BPF_FUNC_MAPPER(__LKL__BPF_ENUM_FN)
	__LKL__BPF_FUNC_MAX_ID,
};
#undef __LKL__BPF_ENUM_FN

/* All flags used by eBPF helper functions, placed here. */

/* BPF_FUNC_skb_store_bytes flags. */
#define LKL_BPF_F_RECOMPUTE_CSUM		(1ULL << 0)
#define LKL_BPF_F_INVALIDATE_HASH		(1ULL << 1)

/* BPF_FUNC_l3_csum_replace and BPF_FUNC_l4_csum_replace flags.
 * First 4 bits are for passing the header field size.
 */
#define LKL_BPF_F_HDR_FIELD_MASK		0xfULL

/* BPF_FUNC_l4_csum_replace flags. */
#define LKL_BPF_F_PSEUDO_HDR		(1ULL << 4)
#define LKL_BPF_F_MARK_MANGLED_0		(1ULL << 5)
#define LKL_BPF_F_MARK_ENFORCE		(1ULL << 6)

/* BPF_FUNC_clone_redirect and BPF_FUNC_redirect flags. */
#define LKL_BPF_F_INGRESS			(1ULL << 0)

/* BPF_FUNC_skb_set_tunnel_key and BPF_FUNC_skb_get_tunnel_key flags. */
#define LKL_BPF_F_TUNINFO_IPV6		(1ULL << 0)

/* BPF_FUNC_get_stackid flags. */
#define LKL_BPF_F_SKIP_FIELD_MASK		0xffULL
#define LKL_BPF_F_USER_STACK		(1ULL << 8)
#define LKL_BPF_F_FAST_STACK_CMP		(1ULL << 9)
#define LKL_BPF_F_REUSE_STACKID		(1ULL << 10)

/* BPF_FUNC_skb_set_tunnel_key flags. */
#define LKL_BPF_F_ZERO_CSUM_TX		(1ULL << 1)
#define LKL_BPF_F_DONT_FRAGMENT		(1ULL << 2)

/* BPF_FUNC_perf_event_output and BPF_FUNC_perf_event_read flags. */
#define LKL_BPF_F_INDEX_MASK		0xffffffffULL
#define LKL_BPF_F_CURRENT_CPU		LKL_BPF_F_INDEX_MASK
/* BPF_FUNC_perf_event_output for sk_buff input context. */
#define LKL_BPF_F_CTXLEN_MASK		(0xfffffULL << 32)

/* user accessible mirror of in-kernel sk_buff.
 * new fields can only be added to the end of this structure
 */
struct __lkl__sk_buff {
	__lkl__u32 len;
	__lkl__u32 pkt_type;
	__lkl__u32 mark;
	__lkl__u32 queue_mapping;
	__lkl__u32 protocol;
	__lkl__u32 vlan_present;
	__lkl__u32 vlan_tci;
	__lkl__u32 vlan_proto;
	__lkl__u32 priority;
	__lkl__u32 ingress_ifindex;
	__lkl__u32 ifindex;
	__lkl__u32 tc_index;
	__lkl__u32 cb[5];
	__lkl__u32 hash;
	__lkl__u32 tc_classid;
	__lkl__u32 data;
	__lkl__u32 data_end;
};

struct lkl_bpf_tunnel_key {
	__lkl__u32 tunnel_id;
	union {
		__lkl__u32 remote_ipv4;
		__lkl__u32 remote_ipv6[4];
	};
	__lkl__u8 tunnel_tos;
	__lkl__u8 tunnel_ttl;
	__lkl__u16 tunnel_ext;
	__lkl__u32 tunnel_label;
};

/* Generic BPF return codes which all BPF program types may support.
 * The values are binary compatible with their TC_ACT_* counter-part to
 * provide backwards compatibility with existing SCHED_CLS and SCHED_ACT
 * programs.
 *
 * XDP is handled seprately, see XDP_*.
 */
enum lkl_bpf_ret_code {
	LKL_BPF_OK = 0,
	/* 1 reserved */
	LKL_BPF_DROP = 2,
	/* 3-6 reserved */
	LKL_BPF_REDIRECT = 7,
	/* >127 are reserved for prog type specific return codes */
};

struct lkl_bpf_sock {
	__lkl__u32 bound_dev_if;
	__lkl__u32 family;
	__lkl__u32 type;
	__lkl__u32 protocol;
};

#define LKL_XDP_PACKET_HEADROOM 256

/* User return codes for XDP prog type.
 * A valid XDP program must return one of these defined values. All other
 * return codes are reserved for future use. Unknown return codes will result
 * in packet drop.
 */
enum lkl_xdp_action {
	LKL_XDP_ABORTED = 0,
	LKL_XDP_DROP,
	LKL_XDP_PASS,
	LKL_XDP_TX,
};

/* user accessible metadata for XDP packet hook
 * new fields must be added to the end of this structure
 */
struct lkl_xdp_md {
	__lkl__u32 data;
	__lkl__u32 data_end;
};

#endif /* __LKL__LINUX_BPF_H__ */
