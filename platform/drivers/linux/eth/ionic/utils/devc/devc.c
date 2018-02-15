#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <sys/mman.h>
#include <errno.h>

/* Supply these for ionic_dev.h */
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef u_int64_t dma_addr_t;
#define BIT(n)  (1 << (n))
typedef u8 bool;
#define false 0
#define true 1
#define __iomem
#define BUILD_BUG_ON(c)

#include "../../ionic_dev.h"

#define DEV_CMD_DONE                    0x00000001

struct dev_cmd_regs *regs;
struct dev_cmd_db *db;

static void usage(char **argv)
{
	printf("%s nop\n", argv[0]);
	printf("%s reset\n", argv[0]);
	printf("%s identify\n", argv[0]);
	printf("%s lif_init <index>\n", argv[0]);
	exit(1);
}

static int go(union dev_cmd *cmd, union dev_cmd_comp *comp)
{
	unsigned int i;

	for (i = 0; i < 16; i++)
		regs->cmd.words[i] = cmd->words[i];

	regs->done = 0;
	db->v = 1;

	sleep(1);

	if (!(regs->done & DEV_CMD_DONE)) {
		fprintf(stderr, "Done bit not set after 1 second.\n");
		return 0;
	}

	for (i = 0; i < 4; i++)
		comp->words[i] = regs->comp.words[i];

	return 1;
}

static void nop(int argc, char **argv)
{
	union dev_cmd cmd = {
		.nop.opcode = CMD_OPCODE_NOP,
	};
	union dev_cmd_comp comp;

	if (argc != 2)
		usage(argv);

	if (go(&cmd, &comp)) {
		printf("status: %d\n", comp.nop.status);
	}
}

static void reset(int argc, char **argv)
{
	union dev_cmd cmd = {
		.reset.opcode = CMD_OPCODE_RESET,
	};
	union dev_cmd_comp comp;

	if (argc != 2)
		usage(argv);

	if (go(&cmd, &comp)) {
		printf("status: %d\n", comp.reset.status);
	}
}

static void identify(int argc, char **argv)
{
	union dev_cmd cmd = {
		.identify.opcode = CMD_OPCODE_IDENTIFY,
		.identify.ver = IDENTITY_VERSION_1,
		.identify.addr = 0,  // fix model to take no host addr?
	};
	union dev_cmd_comp comp;

	if (argc != 2)
		usage(argv);

	if (go(&cmd, &comp)) {
		printf("status: %d\n", comp.identify.status);
		printf("ver: %d\n", comp.identify.ver);
		// TODO add dump of identify info from second half of dev cmd page
	}
}

static void lif_init(int argc, char **argv)
{
	union dev_cmd cmd = {
		.lif_init.opcode = CMD_OPCODE_LIF_INIT,
	};
	union dev_cmd_comp comp;

	if (argc != 3)
		usage(argv);

	cmd.lif_init.index = strtol(argv[2], NULL, 10);

	if (go(&cmd, &comp)) {
		printf("status: %d\n", comp.identify.status);
	}
}

int main(int argc, char **argv)
{
	int fd;
	char *path = "/dev/mem";
	char *cmd;
	off_t bar_addr = 0xfe600000;
	size_t bar_size = 2 * 4096;

	if (argc < 2)
		usage(argv);

	fd = open(path, O_RDWR | O_SYNC);
	if (fd == -1) {
		perror(path);
		exit(1);
	} 

	regs = mmap(0, bar_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bar_addr);
	if (regs == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	if (regs->signature != DEV_CMD_SIGNATURE) {
		fprintf(stderr, "Signature didn't match.  Expected 0x%08x, got 0x%08x\n",
			DEV_CMD_SIGNATURE, regs->signature);
	}

	db = (void *)regs + 0x1000;

	cmd = argv[1];

	if (strcmp(cmd, "nop") == 0)
		nop(argc, argv);
	if (strcmp(cmd, "reset") == 0)
		reset(argc, argv);
	if (strcmp(cmd, "identify") == 0)
		identify(argc, argv);
	if (strcmp(cmd, "lif_init") == 0)
		lif_init(argc, argv);

	munmap(regs, bar_size);
	close(fd);

	return 0;
}
