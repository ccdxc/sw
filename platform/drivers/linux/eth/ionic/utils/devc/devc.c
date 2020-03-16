#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <sys/mman.h>
#include <errno.h>

/* Supply these for ionic_if.h */
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int16_t __le16;
typedef u_int32_t u32;
typedef u_int32_t __le32;
typedef u_int64_t u64;
typedef u_int64_t __le64;
typedef u_int64_t dma_addr_t;
typedef u_int64_t phys_addr_t;
typedef u_int64_t cpumask_t;
#define BIT(n)  (1 << (n))
#define BIT_ULL(n)  (1ULL << (n))
typedef u8 bool;
#define false 0
#define true 1
#define __iomem
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#include "ionic_if.h"

#define IONIC_DEV_CMD_DONE              0x00000001

union ionic_dev_cmd_regs *cmd_regs;
union ionic_dev_info_regs *info_regs;

char *progname;

static void usage(char **argv)
{
	printf("%s <pci> nop\n", progname);
	printf("%s <pci> reset\n", progname);
	printf("%s <pci> identify\n", progname);
	printf("%s <pci> link <down|up>\n", progname);
	printf("%s <pci> lif_init <index>\n", progname);
	printf("%s <pci> vf <id> set|get spoof|trust|state|mac|vlan|rate [val]\n",
	       progname);
	exit(1);
}

static int go(union ionic_dev_cmd *cmd, union ionic_dev_cmd_comp *comp)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
		cmd_regs->cmd.words[i] = cmd->words[i];

	cmd_regs->done = 0;
	cmd_regs->doorbell = 1;

	sleep(1);

	if (!(cmd_regs->done & IONIC_DEV_CMD_DONE)) {
		fprintf(stderr, "Done bit not set after 1 second.\n");
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(comp->words); i++)
		comp->words[i] = cmd_regs->comp.words[i];

	return 1;
}

static void dump_regs()
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd_regs->cmd.words); i++)
		printf("cmd[%02d] = 0x%08x\n", i, cmd_regs->cmd.words[i]);

	for (i = 0; i < ARRAY_SIZE(cmd_regs->comp.words); i++)
		printf("comp[%02d] = 0x%08x\n", i, cmd_regs->comp.words[i]);
}

static void nop(int argc, char **argv)
{
	union ionic_dev_cmd cmd = {
		.nop.opcode = IONIC_CMD_NOP,
	};
	union ionic_dev_cmd_comp comp;

	if (argc != 2)
		usage(argv);

	if (go(&cmd, &comp))
		printf("status: %d\n", comp.nop.status);
}

static void reset(int argc, char **argv)
{
	union ionic_dev_cmd cmd = {
		.reset.opcode = IONIC_CMD_RESET,
	};
	union ionic_dev_cmd_comp comp;

	if (argc != 2)
		usage(argv);

	if (go(&cmd, &comp))
		printf("status: %d\n", comp.reset.status);
}

static void identify(int argc, char **argv)
{
	union ionic_dev_cmd cmd = {
		.identify.opcode = IONIC_CMD_IDENTIFY,
		.identify.ver = IONIC_IDENTITY_VERSION_1,
	};
	union ionic_dev_cmd_comp comp;
	union ionic_dev_identity *dev_ident;

	if (argc != 2)
		usage(argv);

	if (!go(&cmd, &comp))
		return;

	printf("status: %d\n", comp.identify.status);
	printf("ver: %d\n", comp.identify.ver);
	printf("\n");

	printf("signature:        0x%x\n", info_regs->signature);
	printf("asic_type:        0x%x\n", info_regs->asic_type);
	printf("asic_rev:         0x%x\n", info_regs->asic_rev);
	printf("serial_num:       %s\n",   info_regs->serial_num);
	printf("fw_version:       %s\n",   info_regs->fw_version);
	printf("fw_status:        0x%x\n", info_regs->fw_status);
	printf("fw_heartbeat:     0x%x\n", info_regs->fw_heartbeat);
	printf("\n");

	dev_ident = (union ionic_dev_identity *)cmd_regs->data;
	printf("version:          %d\n", dev_ident->version);
	printf("type:             %d\n", dev_ident->type);
	printf("nports:           %d\n", dev_ident->nports);
	printf("nlifs:            %d\n", dev_ident->nlifs);
	printf("nintrs:           %d\n", dev_ident->nintrs);
	printf("eth_eq_count:     %d\n", dev_ident->eq_count);
	printf("ndbpgs_per_lif:   %d\n", dev_ident->ndbpgs_per_lif);
	printf("intr_coal_mult:   %d\n", dev_ident->intr_coal_mult);
	printf("intr_coal_div:    %d\n", dev_ident->intr_coal_div);
}

static void port_state(int argc, char **argv)
{
	union ionic_dev_cmd cmd = {
		.port_setattr.opcode = IONIC_CMD_PORT_SETATTR,
		.port_setattr.attr = IONIC_PORT_ATTR_STATE,
	};
	union ionic_dev_cmd_comp comp;
	u8 state;

	if (argc != 3)
		usage(argv);

	if (!strcmp(argv[2], "up")) {
		cmd.port_setattr.state = 1;
	} else if (!strcmp(argv[2], "down")) {
		cmd.port_setattr.state = 0;
	} else {
		usage(argv);
	}

	if (go(&cmd, &comp))
		printf("status: %d\n", comp.identify.status);
}

static void lif_init(int argc, char **argv)
{
	union ionic_dev_cmd cmd = {
		.lif_init.opcode = IONIC_CMD_LIF_INIT,
	};
	union ionic_dev_cmd_comp comp;

	if (argc != 3)
		usage(argv);

	cmd.lif_init.index = strtol(argv[2], NULL, 10);

	if (go(&cmd, &comp))
		printf("status: %d\n", comp.identify.status);
}

static void vf_attr(int argc, char **argv)
{
	union ionic_dev_cmd cmd = { 0 };
	union ionic_dev_cmd_comp comp;
	unsigned int attr;
	unsigned int vf;
	unsigned int set = 0;

	/* devc vf <id> set/get spoof|trust|state|mac|vlan|rate [val]*/
	if (argc < 5)
		usage(argv);

	errno = 0;
	vf = strtol(argv[2], NULL, 10);
	if (errno)
		usage(argv);
	cmd.vf_setattr.vf_index = vf;

	if (!strcmp(argv[3], "set")) {
		set++;
		if (argc != 6)
			usage(argv);
	} else if (argc != 5) {
		usage(argv);
	}

	if (!strcmp(argv[4], "spoof")) {
		cmd.vf_setattr.attr = IONIC_VF_ATTR_SPOOFCHK;
		if (set)
			cmd.vf_setattr.spoofchk = strtol(argv[5], NULL, 10);
	} else if (!strcmp(argv[4], "trust")) {
		cmd.vf_setattr.attr = IONIC_VF_ATTR_TRUST;
		if (set)
			cmd.vf_setattr.trust = strtol(argv[5], NULL, 10);
	} else if (!strcmp(argv[4], "state")) {
		cmd.vf_setattr.attr = IONIC_VF_ATTR_LINKSTATE;
		if (set)
			cmd.vf_setattr.linkstate = strtol(argv[5], NULL, 10);
	} else if (!strcmp(argv[4], "vlan")) {
		cmd.vf_setattr.attr = IONIC_VF_ATTR_VLAN;
		if (set)
			cmd.vf_setattr.vlanid = strtol(argv[5], NULL, 10);
	} else if (!strcmp(argv[4], "rate")) {
		cmd.vf_setattr.attr = IONIC_VF_ATTR_RATE;
		if (set)
			cmd.vf_setattr.maxrate = strtol(argv[5], NULL, 10);
	} else if (!strcmp(argv[4], "mac")) {
		cmd.vf_setattr.attr = IONIC_VF_ATTR_MAC;
		if (set) {
			//val = strtol(argv[5], NULL, 10);
			printf("set mac not yet supported\n");
			exit(1);
		}
	} else {
		usage(argv);
	}

	if (set)
		cmd.vf_setattr.opcode = IONIC_CMD_VF_SETATTR;
	else
		cmd.vf_setattr.opcode = IONIC_CMD_VF_GETATTR;

	if (!go(&cmd, &comp))
		return;

	printf("status: %d\n", comp.identify.status);

	if (!set && !comp.identify.status) {
		switch (cmd.vf_setattr.attr) {
		case IONIC_VF_ATTR_MAC:
			printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
				comp.vf_getattr.macaddr[0],
				comp.vf_getattr.macaddr[1],
				comp.vf_getattr.macaddr[2],
				comp.vf_getattr.macaddr[3],
				comp.vf_getattr.macaddr[4],
				comp.vf_getattr.macaddr[5]);
			break;
		case IONIC_VF_ATTR_VLAN:
			printf("%d\n", comp.vf_getattr.vlanid);
			break;
		case IONIC_VF_ATTR_RATE:
			printf("%d\n", comp.vf_getattr.maxrate);
			break;
		default:
			printf("%d\n", comp.vf_getattr.trust);
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int fd;
	char *path = "/dev/mem";
	char *cmd;
	off_t bar_addr = 0;
	void *mapped;
	size_t map_size = 4096;
	char *errmsg;
	char buf[128];
	char cmdbuf[128];
	FILE *fp;
	char *pciaddr;

	progname = argv[0];

	if (argc < 3)
		usage(argv);

	/* get the 64-bit bar offset from setpci in two 32-bit chunks */
	pciaddr = argv[1];
	snprintf(cmdbuf, sizeof(cmdbuf),
		 "setpci -s %s base_address_0 base_address_1", pciaddr);
	fp = popen(cmdbuf, "r");
	if (!fp) {
		perror("popen for setpci");
		exit(1);
	}
	buf[0] = '0';
	buf[1] = 'x';
	if (fgets(&buf[2], sizeof(buf)-2, fp) == NULL) {
		printf("no data from setpci\n");
		exit(1);
	}
	errno = 0;
	bar_addr = strtoul(buf, NULL, 0);
	if (errno) {
		perror("strtoul");
		exit(1);
	}

	if (fgets(&buf[2], sizeof(buf)-2, fp) == NULL) {
		printf("no data from setpci\n");
		exit(1);
	}
	errno = 0;
	bar_addr |= (strtoul(buf, NULL, 0) << 32);
	if (errno) {
		perror("strtoul");
		exit(1);
	}

	bar_addr &= ~0xfULL;

	fclose(fp);

	/* skip over the pci addr and continue on */
	argv++;
	argc--;

	fd = open(path, O_RDWR | O_SYNC);
	if (fd == -1) {
		perror(path);
		exit(1);
	}

	mapped = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		      fd, bar_addr);
	if (mapped == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	info_regs = mapped + IONIC_BAR0_DEV_INFO_REGS_OFFSET;
	if (info_regs->signature != IONIC_DEV_INFO_SIGNATURE) {
		fprintf(stderr,
			"Signature mismatch. Expected 0x%08x, got 0x%08x\n",
			IONIC_DEV_INFO_SIGNATURE, info_regs->signature);
	}
	cmd_regs = mapped + IONIC_BAR0_DEV_CMD_REGS_OFFSET;

	cmd = argv[1];

	if (strcmp(cmd, "nop") == 0)
		nop(argc, argv);
	else if (strcmp(cmd, "reset") == 0)
		reset(argc, argv);
	else if (strcmp(cmd, "identify") == 0)
		identify(argc, argv);
	else if (strcmp(cmd, "link") == 0)
		port_state(argc, argv);
	else if (strcmp(cmd, "lif_init") == 0)
		lif_init(argc, argv);
	else if (strcmp(cmd, "vf") == 0)
		vf_attr(argc, argv);
	else if (strcmp(cmd, "dump") == 0)
		dump_regs();
	else
		usage(argv);

	munmap(mapped, map_size);
	close(fd);

	return 0;
}
