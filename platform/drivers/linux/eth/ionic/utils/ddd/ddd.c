#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/* Supply these for ionic_if.h */
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef u_int64_t dma_addr_t;
#define __packed __attribute__((packed))
#define BIT(n)  (1 << (n))

typedef u8 bool;
#define false 0
#define true 1

#include "../../ionic_if.h"

char *make_path(char *q, char *obj)
{
	static char path[256];
	char *base = "/sys/kernel/debug/ionic";

	sprintf(path, "%s/%s/%s", base, q, obj);
	return path;
}

int get_val(char *q, char *obj)
{
	char *path = make_path(q, obj);
	FILE *file;
	int val;

	file = fopen(path, "r");
	if (!file) {
		perror(path);
		exit(1);
	}

	fscanf(file, "%d", &val);
	fclose(file);

	return val;
}

char *heads_or_tails(int i, int head, int tail)
{
	if (i == head && i == tail)
		return "ht->";
	if (i == head)
		return "h-->";
	if (i == tail)
		return "t-->";
	return "    ";
}

void dump_rxq(char *path)
{
	FILE *file;
	struct rxq_desc desc;
	int i;

	int head = get_val(path, "head");
	int tail = get_val(path, "tail");
	int num_descs = get_val(path, "num_descs");

	path = make_path(path, "desc_blob");
	file = fopen(path, "rb");
	if (!file) {
		perror(path);
		exit(1);
	}

	for (i = 0; i < num_descs; i++) {
		fread(&desc, sizeof(desc), 1, file);

		printf("%s", heads_or_tails(i, head, tail));
		printf("[%04x]", i);
		printf(" addr 0x%lx len %d",
		       (u64)desc.addr, desc.len);

		printf("\n");
	}

	fclose(file);
}

void dump_rxcq(char *path)
{
	FILE *file;
	struct rxq_comp comp;
	int tail = get_val(path, "tail");
	int num_descs = get_val(path, "num_descs");
	int i;

	path = make_path(path, "desc_blob");
	file = fopen(path, "rb");
	if (!file) {
		perror(path);
		exit(1);
	}

	for (i = 0; i < num_descs; i++) {
		fread(&comp, sizeof(comp), 1, file);

		printf("%s", heads_or_tails(i, -1, tail));
		printf("[%04x]", i);
		printf(" status %d comp_index 0x%x color %d",
		       comp.status, comp.comp_index, comp.color);
		printf(" csum/flags 0x%04x/[%s%s%s%s%s%s]", comp.csum,
		       comp.csum_tcp_ok ?  " TCP_OK"  : "",
		       comp.csum_tcp_bad ? " TCP_BAD" : "",
		       comp.csum_udp_ok ?  " UDP_OK"  : "",
		       comp.csum_udp_bad ? " UDP_BAD" : "",
		       comp.csum_ip_ok ?   " IP_OK"   : "",
		       comp.csum_ip_bad ?  " IP_BAD"  : "");
		if (comp.rss_type)
			printf(" rss_hash/type 0x%08x/%d", comp.rss_hash,
			       comp.rss_type);
		if (comp.V)
			printf(" vlan_tci 0x%04x", comp.vlan_tci);
		printf("\n");
	}

	fclose(file);
}

void dump_txcq(char *path)
{
	FILE *file;
	struct txq_comp comp;
	int tail = get_val(path, "tail");
	int num_descs = get_val(path, "num_descs");
	int i;

	path = make_path(path, "desc_blob");
	file = fopen(path, "rb");
	if (!file) {
		perror(path);
		exit(1);
	}

	for (i = 0; i < num_descs; i++) {
		fread(&comp, sizeof(comp), 1, file);

		printf("%s", heads_or_tails(i, -1, tail));
		printf("[%04x]", i);
		printf(" status %d comp_index 0x%x color %d",
		       comp.status, comp.comp_index, comp.color);
		printf("\n");
	}

	fclose(file);
}

void dump_txq(char *path)
{
	char *fpath;
	FILE *file, *file_sg;
	struct txq_desc desc;
	struct txq_sg_desc sg_desc;
	int i, j;

	int head = get_val(path, "head");
	int tail = get_val(path, "tail");
	int num_descs = get_val(path, "num_descs");

	fpath = make_path(path, "desc_blob");
	file = fopen(fpath, "rb");
	if (!file) {
		perror(fpath);
		exit(1);
	}

	fpath = make_path(path, "sg_desc_blob");
	file_sg = fopen(fpath, "rb");
	if (!file_sg) {
		perror(fpath);
		exit(1);
	}

	char *txq_opcode(u8 opcode)
	{
		switch (opcode) {
		case TXQ_DESC_OPCODE_CALC_NO_CSUM:
			return "CALC_NO_CSUM";
		case TXQ_DESC_OPCODE_CALC_CSUM:
			return "CALC_CSUM";
		case TXQ_DESC_OPCODE_CALC_CRC32_CSUM:
			return "CALC_CRC32_CSUM";
		case TXQ_DESC_OPCODE_TSO:
			return "TSO";
		case TXQ_DESC_OPCODE_NOP:
			return "NOP";
		default:
			return "???";
		}
	}

	for (i = 0; i < num_descs; i++) {
		fread(&desc, sizeof(desc), 1, file);
		fread(&sg_desc, sizeof(sg_desc), 1, file_sg);

		printf("%s", heads_or_tails(i, head, tail));
		printf("[%04x]", i);
		printf(" %-15s flags %s%s%s addr 0x%lx len %d",
		       txq_opcode(desc.opcode), desc.V ? "V" : "-",
		       desc.C ? "C" : "-", desc.O ? "O" : "-",
		       (u64)desc.addr, desc.len);

		switch (desc.opcode) {
		case TXQ_DESC_OPCODE_CALC_CSUM:
		case TXQ_DESC_OPCODE_CALC_CRC32_CSUM:
			printf(" hdr_len %d csum_offset %d", desc.hdr_len,
			       desc.csum_offset);
			break;
		case TXQ_DESC_OPCODE_TSO:
			printf(" hdr_len %d mss %d", desc.hdr_len,
			       desc.mss);
			break;
		}

		if (desc.vlan_tci)
			printf(" vlan 0x%04x", desc.vlan_tci);

		printf("\n");

		for (j = 0; j < desc.num_sg_elems; j++) {
			printf("                                     addr 0x%lx len %d\n",
			       (u64)sg_desc.elems[j].addr,
			       sg_desc.elems[j].len);
		}

	}

	fclose(file);
	fclose(file_sg);
}

int main (int argc, char **argv)
{
	char *path, *bdf, *lif, *q, *qcq;
	bool cq, tx;

	void usage(void)
	{
		printf("e.g. %s 0000:03:00.0/lif0/tx0/q\n", argv[0]);
		exit(1);
	}

	if (argc != 2)
		usage();

	path = strdup(argv[1]);

	bdf = strtok(argv[1], "/");
	lif = strtok(NULL, "/");
	q = strtok(NULL, "/");
	qcq = strtok(NULL, "/");

	tx = (q[0] == 't');
	cq = (qcq[0] == 'c');

	if (tx && cq)
		dump_txcq(path);
	if (tx && !cq)
		dump_txq(path);
	if (!tx && cq)
		dump_rxcq(path);
	if (!tx && !cq)
		dump_rxq(path);

	return 0;
}
