#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

/* Supply these for ionic_if.h */
typedef uint8_t u8;
typedef uint16_t u16, __le16;
typedef uint32_t u32, __le32;
typedef uint64_t u64, __le64;
typedef uint64_t dma_addr_t;
#define BIT(n)		(1 << (n))
#define BIT_ULL(n)	(1ULL << (n))

typedef u8 bool;
#define false 0
#define true 1

#include "ionic_if.h"

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

void dump_adminq(char *path)
{
	char *fpath;
	FILE *file, *file_sg;
	struct admin_cmd cmd;
	int i, j;

	int head = get_val(path, "head");
	int tail = get_val(path, "tail");
	int num_descs = get_val(path, "num_descs");

	char *ionic_opcode_to_str(enum cmd_opcode opcode)
	{
		switch (opcode) {
		case IONIC_CMD_NOP:
			return "IONIC_CMD_NOP";
		case IONIC_CMD_INIT:
			return "IONIC_CMD_INIT";
		case IONIC_CMD_RESET:
			return "IONIC_CMD_RESET";
		case IONIC_CMD_IDENTIFY:
			return "IONIC_CMD_IDENTIFY";
		case IONIC_CMD_GETATTR:
			return "IONIC_CMD_GETATTR";
		case IONIC_CMD_SETATTR:
			return "IONIC_CMD_SETATTR";
		case IONIC_CMD_PORT_IDENTIFY:
			return "IONIC_CMD_PORT_IDENTIFY";
		case IONIC_CMD_PORT_INIT:
			return "IONIC_CMD_PORT_INIT";
		case IONIC_CMD_PORT_RESET:
			return "IONIC_CMD_PORT_RESET";
		case IONIC_CMD_PORT_GETATTR:
			return "IONIC_CMD_PORT_GETATTR";
		case IONIC_CMD_PORT_SETATTR:
			return "IONIC_CMD_PORT_SETATTR";
		case IONIC_CMD_LIF_INIT:
			return "IONIC_CMD_LIF_INIT";
		case IONIC_CMD_LIF_RESET:
			return "IONIC_CMD_LIF_RESET";
		case IONIC_CMD_LIF_IDENTIFY:
			return "IONIC_CMD_LIF_IDENTIFY";
		case IONIC_CMD_LIF_SETATTR:
			return "IONIC_CMD_LIF_SETATTR";
		case IONIC_CMD_LIF_GETATTR:
			return "IONIC_CMD_LIF_GETATTR";
		case IONIC_CMD_RX_MODE_SET:
			return "IONIC_CMD_RX_MODE_SET";
		case IONIC_CMD_RX_FILTER_ADD:
			return "IONIC_CMD_RX_FILTER_ADD";
		case IONIC_CMD_RX_FILTER_DEL:
			return "IONIC_CMD_RX_FILTER_DEL";
		case IONIC_CMD_Q_IDENTIFY:
			return "IONIC_CMD_Q_IDENTIFY";
		case IONIC_CMD_Q_INIT:
			return "IONIC_CMD_Q_INIT";
		case IONIC_CMD_Q_CONTROL:
			return "IONIC_CMD_Q_CONTROL";
		case IONIC_CMD_RDMA_RESET_LIF:
			return "IONIC_CMD_RDMA_RESET_LIF";
		case IONIC_CMD_RDMA_CREATE_EQ:
			return "IONIC_CMD_RDMA_CREATE_EQ";
		case IONIC_CMD_RDMA_CREATE_CQ:
			return "IONIC_CMD_RDMA_CREATE_CQ";
		case IONIC_CMD_RDMA_CREATE_ADMINQ:
			return "IONIC_CMD_RDMA_CREATE_ADMINQ";
		case IONIC_CMD_FW_DOWNLOAD:
			return "IONIC_CMD_FW_DOWNLOAD";
		case IONIC_CMD_FW_CONTROL:
			return "IONIC_CMD_FW_CONTROL";
		case IONIC_CMD_VF_GETATTR:
			return "IONIC_CMD_VF_GETATTR";
		case IONIC_CMD_VF_SETATTR:
			return "IONIC_CMD_VF_SETATTR";
		default:
			return "DEVCMD_UNKNOWN";
		}
	}

	fpath = make_path(path, "desc_blob");
	file = fopen(fpath, "rb");
	if (!file) {
		perror(fpath);
		exit(1);
	}

	for (i = 0; i < num_descs; i++) {
		fread(&cmd, sizeof(cmd), 1, file);

		printf("%s", heads_or_tails(i, head, tail));
		printf("[%04x]", i);
		printf(" op %02d %-15s", cmd.opcode, ionic_opcode_to_str(cmd.opcode));

		printf("\n");
	}

	fclose(file);
}

void dump_admincq(char *path)
{
	FILE *file;
	struct admin_comp comp;
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
		       comp.status, comp.comp_index,
		       comp.color & IONIC_COMP_COLOR_MASK ? 1 : 0);

		printf("\n");
	}

	fclose(file);
}

void dump_notifycq(char *path)
{
	FILE *file;
	struct notifyq_event ev;
	int tail = get_val(path, "tail");
	int num_descs = get_val(path, "num_descs");
	int i;

	char *ev_opcode(u8 opcode)
	{
		switch (opcode) {
		case 0:
			return "none";
		case IONIC_EVENT_LINK_CHANGE:
			return "IONIC_EVENT_LINK_CHANGE";
		case IONIC_EVENT_RESET:
			return "IONIC_EVENT_RESET";
		case IONIC_EVENT_HEARTBEAT:
			return "IONIC_EVENT_HEARTBEAT";
		case IONIC_EVENT_LOG:
			return "IONIC_EVENT_LOG";
		case IONIC_EVENT_XCVR:
			return "IONIC_EVENT_XCVR";
		default:
			return "unknown";
		}
	}

	path = make_path(path, "desc_blob");
	file = fopen(path, "rb");
	if (!file) {
		perror(path);
		exit(1);
	}

	for (i = 0; i < num_descs; i++) {
		fread(&ev, sizeof(ev), 1, file);

		printf("%s", heads_or_tails(i, -1, tail));
		printf("[%04x]", i);

		printf(" eid %lu ecode %d %s", ev.eid, ev.ecode, ev_opcode(ev.ecode));

		printf("\n");
	}

	fclose(file);
}

void dump_rxq(char *path)
{
	char *fpath;
	FILE *file, *file_sg;
	struct rxq_desc desc;
	struct rxq_sg_desc sg_desc;
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

	for (i = 0; i < num_descs; i++) {
		fread(&desc, sizeof(desc), 1, file);
		fread(&sg_desc, sizeof(sg_desc), 1, file_sg);

		printf("%s", heads_or_tails(i, head, tail));
		printf("[%04x]", i);
		printf(" addr 0x%lx len %d",
		       (u64)desc.addr, (u16)desc.len);

		printf("\n");

		for (j = 0; j < IONIC_RX_MAX_SG_ELEMS; j++) {
			printf("           addr 0x%lx len %d\n",
			       (u64)sg_desc.elems[j].addr,
			       sg_desc.elems[j].len);
		}
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

		printf(" status %d comp_index 0x%x color %d num_sg %d",
		       comp.status, comp.comp_index,
			   comp.pkt_type_color & IONIC_COMP_COLOR_MASK ? 1 : 0,
			   comp.num_sg_elems);
		printf(" len %4d", comp.len);

		printf(" csum %d", comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_CALC);
		if (comp.csum_flags)
			printf(" csum/flags 0x%04x/[%s%s%s%s%s%s]",
				comp.csum,
				comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_TCP_OK ? " TCP_OK"  : "",
				comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_TCP_BAD ? " TCP_BAD" : "",
				comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_UDP_OK ? " UDP_OK"  : "",
				comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_UDP_BAD ? " UDP_BAD" : "",
				comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_IP_OK ? " IP_OK"   : "",
				comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_IP_BAD ? " IP_BAD"  : "");

		printf(" rss_hash 0x%08x pkt_type %d",
				comp.rss_hash,
				comp.pkt_type_color & IONIC_RXQ_COMP_PKT_TYPE_MASK);

		printf(" vlan %d", comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_VLAN ? 1 : 0);
		if (comp.csum_flags & IONIC_RXQ_COMP_CSUM_F_VLAN)
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
	int num_sges = 0;

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
		case IONIC_TXQ_DESC_OPCODE_CSUM_NONE:
			return "CSUM_NONE";
		case IONIC_TXQ_DESC_OPCODE_CSUM_PARTIAL:
			return "CSUM_PARTIAL";
		case IONIC_TXQ_DESC_OPCODE_CSUM_HW:
			return "CSUM_HW";
		case IONIC_TXQ_DESC_OPCODE_TSO:
			return "TSO";
		default:
			return "???";
		}
	}

	for (i = 0; i < num_descs; i++) {
		fread(&desc, sizeof(desc), 1, file);
		fread(&sg_desc, sizeof(sg_desc), 1, file_sg);

		printf("%s", heads_or_tails(i, head, tail));
		printf("[%04x]", i);

		switch ((desc.cmd >> IONIC_TXQ_DESC_OPCODE_SHIFT) & IONIC_TXQ_DESC_OPCODE_MASK) {
		case IONIC_TXQ_DESC_OPCODE_CSUM_PARTIAL:
			printf(" %-15s flags %s%s-- addr 0x%llx len %d",
			       txq_opcode((desc.cmd >> IONIC_TXQ_DESC_OPCODE_SHIFT) & IONIC_TXQ_DESC_OPCODE_MASK),
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_VLAN) ? "V" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_ENCAP) ? "O" : "-",
			       ((u64)desc.cmd >> IONIC_TXQ_DESC_ADDR_SHIFT) & IONIC_TXQ_DESC_ADDR_MASK,
			       desc.len);
			printf(" csum_start %d csum_offset %d", desc.csum_start, desc.csum_offset);
			break;
		case IONIC_TXQ_DESC_OPCODE_CSUM_HW:
			printf(" %-15s flags %s%s%s%s-- addr 0x%llx len %d",
			       txq_opcode((desc.cmd >> IONIC_TXQ_DESC_OPCODE_SHIFT) & IONIC_TXQ_DESC_OPCODE_MASK),
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_VLAN) ? "V" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_ENCAP) ? "O" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_CSUM_L3) ? "3" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_CSUM_L4) ? "4" : "-",
			       ((u64)desc.cmd >> IONIC_TXQ_DESC_ADDR_SHIFT) & IONIC_TXQ_DESC_ADDR_MASK,
			       desc.len);
			break;
		case IONIC_TXQ_DESC_OPCODE_TSO:
			printf(" %-15s flags %s%s%s%s addr 0x%llx len %d",
			       txq_opcode((desc.cmd >> IONIC_TXQ_DESC_OPCODE_SHIFT) & IONIC_TXQ_DESC_OPCODE_MASK),
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_VLAN) ? "V" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_ENCAP) ? "O" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_TSO_SOT) ? "S" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_TSO_EOT) ? "E" : "-",
			       ((u64)desc.cmd >> IONIC_TXQ_DESC_ADDR_SHIFT) & IONIC_TXQ_DESC_ADDR_MASK,
			       desc.len);
			printf(" hdr_len %d mss %d", desc.hdr_len, desc.mss);
			break;
		default:
			printf(" %-15s flags %s%s-- addr 0x%llx len %d",
			       txq_opcode((desc.cmd >> IONIC_TXQ_DESC_OPCODE_SHIFT) & IONIC_TXQ_DESC_OPCODE_MASK),
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_VLAN) ? "V" : "-",
			       (desc.cmd & IONIC_TXQ_DESC_FLAG_ENCAP) ? "O" : "-",
			       ((u64)desc.cmd >> IONIC_TXQ_DESC_ADDR_SHIFT) & IONIC_TXQ_DESC_ADDR_MASK,
			       desc.len);
			break;
		}

		if (desc.cmd & IONIC_TXQ_DESC_FLAG_VLAN)
			printf(" vlan 0x%04x", desc.vlan_tci);

		printf("\n");

		num_sges = (desc.cmd >> IONIC_TXQ_DESC_NSGE_SHIFT) & IONIC_TXQ_DESC_NSGE_MASK;
		for (j = 0; j < num_sges; j++) {
			printf("                                       addr 0x%lx len %d\n",
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
	bool cq;

	void usage(void)
	{
		printf("e.g. %s 0000:03:00.0/lif0/L0-tx0/q\n", argv[0]);
		exit(1);
	}

	if (argc != 2)
		usage();

	path = strdup(argv[1]);

	bdf = strtok(argv[1], "/");
	lif = strtok(NULL, "/");
	q = strtok(NULL, "/");
	qcq = strtok(NULL, "/");

	cq = (qcq[0] == 'c');

	switch (q[3]) {
	case 't':
		if (cq)
			dump_txcq(path);
		else
			dump_txq(path);
		break;
	case 'r':
		if (cq)
			dump_rxcq(path);
		if (!cq)
			dump_rxq(path);
		break;
	case 'a':
		if (cq)
			dump_admincq(path);
		else
			dump_adminq(path);
		break;
	case 'n':
		if (cq)
			dump_notifycq(path);
		break;
	default:
		usage();
		break;
	}

	return 0;
}
