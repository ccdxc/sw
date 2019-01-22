/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <sys/time.h>

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "platform/pal/include/pal_int.h"
#include "platform/pal/include/pal_reg.h"

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))

/* Number of repetitions in the main loop */
#define N_LOOPS_DEFAULT	5

/*
 * This is the value to which we will set an interval time to catch
 * unexpect hangs quickly.
 */
static const struct itimerval timeout = {
	.it_value = {
		.tv_sec = 0,
		.tv_usec = 1000000 / 2,
	}
};

/*
 * Since this is a test program, we define these here instead of the
 * device tree. These are addresses of the base of the CSR.
 */
#define PP_PORT_C_INT_C_MAC_INTREG	0x070111b0lu	/* GIC15 */
#define PXB_INT_ITR_ECC_INTREG		0x0719a590lu	/* GIC17 */
#define MC0_INT_MC_INTREG		0x6a180020lu	/* GIC18 */

#define MS_STA_CHIP_PADDR	0x6a000008	/* Version register */
#define MS_STA_CHIP_VERSION	0x7ffe		/* Expected value */

enum int_source {
	GIC15,
	GIC17,
	GIC18,
};

/* *Physical* address. The compiler can't tell us this because it only
 * deals with virtual addresses */
typedef uint64_t paddr_t;

/* Information about what we write to device registers to set them up */
struct reg_setup {
	paddr_t		paddr;
	uint32_t	value;
	size_t		size;
};

struct read_reg {
	paddr_t		paddr;
};

/* Things to pass to the signal handler */
struct siginfo {
	struct pal_int	*pal_int;
	bool		verbose;
	enum int_source	source;
};

/*
 * Normal CSRs
 *
 * Register names follow the hardware register names
 *
 * @enable_set:		Every one bit in values written enables the
 *			corresponding interrupt
 * @enable_set:		Every one bit in values written disables the
 *			corresponding interrupt
 * @intreg:		One for each interrupt that is being asserted
 * @test:		Test register. Write a one for a particular bit
 *			asserts the corresponding interrupt
 */
struct csr {
	uint32_t	intreg;
	uint32_t	test_set;
	uint32_t	enable_set;
	uint32_t	enable_clear;
};

/*
 * Root CSR
 * @intreg:		The bit corresponding to an interrupt source is one
 *			if that interrupt is asserted.
 * @enable_rw_reg:	Write a one to enable an interrupt, zero to disable it
 * @rw_reg:		One for each asserted interrupt
 */
struct root_csr {
	uint32_t	intreg;
	uint32_t	enable_rw_reg;
	uint32_t	rw_reg;
};

/*
 * Per GIC information, indexed by enum int_source
 * name:		Name of the device driver
 * intreg_paddr:	Physical address of the register to set for testing
 * test_trigger:	Value to write to intreg_paddr, as mapped into
 *			virtual memory
 * intreg_off
 */
struct per_intr_info {
	const char 		*name;
	const char 		*intreg_name;
	paddr_t			intreg_paddr;
	uint32_t		test_trigger;
	uint32_t		intreg_off;
	const struct reg_setup	*reg_setup;
	size_t			n_reg_setup;
	const struct read_reg	*read_reg;
	size_t			n_read_reg;
};

struct reg_name {
	paddr_t		paddr;
	const char	*name;
};

static struct siginfo siginfo;

static const struct reg_name reg_names[] = {
	{0x0701119c, "pp_port_c_intr"},
	{0x070111a0, "pp_port_c_int_groups_intreg"},
	{0x070111a4, "pp_port_c_int_groups_int_enable_rw_reg"},
	{0x070111b0, "pp_port_c_int_c_mac_intreg"},
	{0x070111b0,"pp_port_c_int_c_mac_intreg"},
	{0x070111b4,"pp_port_c_int_c_mac_int_test_set"},
	{0x070111b8, "pp_port_c_int_c_mac_int_enable_set"},
	{0x070202c8, "pp_intr"},
	{0x070202d0, "pp_int_groups_intreg"},
	{0x070202d4, "pp_int_groups_int_enable_rw_reg"},
	{0x070202e0, "pp_int_pp_intreg"},
	{0x070202e8, "pp_int_pp_int_enable_set"},
	{0x0719a578, "cap0.pxb.pxb.csr_intr"},
	{0x0719a580, "cap0.pxb.pxb.int_groups.intreg"},
	{0x0719a584, "cap0.pxb.pxb.int_groups.int_enable_rw_reg"},
	{0x0719a590, "cap0.pxb.pxb.int_itr_ecc.intreg"},
	{0x0719a590,"pxb_int_intr_ecc_intreg"},
	{0x0719a594,"pxb_int_intr_ecc_int_test_set"},
	{0x0719a598, "cap0.pxb.pxb.int_itr_ecc.int_enable_set"},
	{0x0719a5a0, "cap0.pxb.pxb.int_tgt_ecc.intreg"},
	{0x0719a5a8, "cap0.pxb.pxb.int_tgt_ecc.int_enable_set"},
	{0x0719a5b0, "cap0.pxb.pxb.int_err.intreg"},
	{0x0719a5b8, "cap0.pxb.pxb.int_err.int_enable_set"},
 	{0x6a001040, "ms_intr"},
	{0x6a001050, "ms_int_groups_intreg"},
	{0x6a001054, "ms_int_groups_int_enable_rw_reg"},
	{0x6a0011f0, "ms_int_gic17_intreg"},
	{0x6a0011f8, "ms_int_gic17_int_enable_set"},
	{0x6a100048, "cap0.mc.mc0.mch.intr"},
	{0x6a100050, "cap0.mc.mc0.mch.int_groups.intreg"},
	{0x6a100054, "cap0.mc.mc0.mch.int_groups.int_enable_rw_reg"},
	{0x6a100058, "cap0.mc.mc0.mch.int_groups.int_rw_reg"},
	{0x6a100060, "cap0.mc.mc0.mch.int_mc.intreg"},
	{0x6a100068, "cap0.mc.mc0.mch.int_mc.int_enable_set"},
	{0x6a180000, "cap0.mc.mc0.intr"},
	{0x6a180010, "mc0_int_groups_intreg"},
	{0x6a180014, "cap0.mc.mc0.int_groups.int_enable_rw_reg"},
	{0x6a180018, "mc0_int_groups_int_rw_reg"},
	{0x6a180020, "cap0.mc.mc0.int_mc.intreg"},
	{0x6a180024, "mc0_int_mc_int_test_set"},
	{0x6a180028, "cap0.mc.mc0.int_mc.int_enable_set"},
};

/* Values to be written to configure for testing */
static const struct reg_setup reg_setup_gic15[] = {
	{0x0701119c, /* pp_port_c_intr */ 0x3},
	{0x070111b8, /* pp_port_c_int_c_mac_int_enable_set */ 0x7ffff},
	{0x070111a4, /* pp_port_c_int_groups_int_enable_rw_reg */ 0x3},

	{0x070202c8, /* pp_intr */ 0x3},
	{0x070202e8, /* pp_int_pp_int_enable_set */ 0x7ffffff},
	{0x070202d4, /* pp_int_groups_int_enable_rw_reg */ 0x3},

 	{0x6a001040, /* ms_intr */ 0x3},
	{0x6a0011f8, /* ms_int_gic17_int_enable_set */ 0x1},
	{0x6a001054, /* ms_int_groups_int_enable_rw_reg */ 0xffffffff},

	{0x070111a4, /* pp_port_c_int_groups_int_enable_rw_reg */ 0x3},
};

static const struct read_reg read_regs_gic15[] = {
	{0x070111b0 /* pp_port_c_int_c_mac_intreg */},
	{0x070111a0 /* pp_port_c_int_groups_intreg */},
	{0x0701119c /* pp_port_c_intr */},
	{0x070202e0 /* pp_int_pp_intreg */},
	{0x070202d0 /* pp_int_groups_intreg */},
	{0x070202c8 /* pp_intr */},
	{0x6a0011f0 /* ms_int_gic17_intreg */},
	{0x6a001050 /* ms_int_groups_intreg */},
 	{0x6a001040 /* ms_intr */},
};

static const struct reg_setup reg_setup_gic17[] = {
	// Suresh listed more things that must be done.
	// Is this necessary?
	{0x719a578, /* cap0.pxb.pxb.csr_intr */ 0x3},

	{0x719a598, /* cap0.pxb.pxb.int_itr_ecc.int_enable_set */ 0x3ffff},
	{0x719a5a8, /* cap0.pxb.pxb.int_tgt_ecc.int_enable_set */ 0xfffffff},
	{0x719a5b8, /* cap0.pxb.pxb.int_err.int_enable_set */ 0xfffff},

	{0x719a584, /* cap0.pxb.pxb.int_groups.int_enable_rw_reg */ 0x7},
};

static const struct read_reg read_regs_gic17[] = {
	{0x0719a598 /* cap0.pxb.pxb.int_itr_ecc.int_enable_set */},
	{0x0719a590 /* cap0.pxb.pxb.int_itr_ecc.intreg */},

	{0x0719a5a8 /* cap0.pxb.pxb.int_tgt_ecc.int_enable_set */},
	{0x0719a5a0 /* cap0.pxb.pxb.int_tgt_ecc.intreg */},

	{0x0719a5b8 /* cap0.pxb.pxb.int_err.int_enable_set */},
	{0x0719a5b0 /* cap0.pxb.pxb.int_err.intreg */},

	{0x0719a584 /* cap0.pxb.pxb.int_groups.int_enable_rw_reg */},
	{0x0719a580 /* cap0.pxb.pxb.int_groups.intreg */},

	{0x6a001054 /* ms_int_groups_int_enable_rw_reg */},
	{0x6a001050 /* ms_int_groups_intreg */},
};

static const struct reg_setup reg_setup_gic18[] = {
	 /* Enable MC interrupts for GIC 18 */
	{0x6a180004, /* cap0.mc.mc0.intr */ 0x3},
	{0x6a180028, /* cap0.mc.mc0.int_mc.int_enable_set */ 0x1},
	{0x6a180014, /* cap0.mc.mc0.int_groups.int_enable_rw_reg */ 0x1},

	{0x6a100048, /* cap0.mc.mc0.mch.intr */ 0x3},
	{0x6a100068, /* cap0.mc.mc0.mch.int_mc.int_enable_set */ 0x3},
	{0x6a100054, /* cap0.mc.mc0.mch.int_groups.int_enable_rw_reg */ 0x1},
};

static const struct read_reg read_regs_gic18[] = {
	{0x6a180000 /* cap0.mc.mc0.intr */},
	{0x6a180020 /* cap0.mc.mc0.int_mc.intreg */},
	{0x6a180010 /* mc0_int_groups_intreg */},
	{0x6a180018 /* mc0_int_groups_int_rw_reg */},

	{0x6a100048 /* cap0.mc.mc0.mch.intr */},
	{0x6a100060 /* cap0.mc.mc0.mch.int_mc.intreg */},
	{0x6a100050 /* cap0.mc.mc0.mch.int_groups.intreg */},
	{0x6a100058 /* cap0.mc.mc0.mch.int_groups.int_rw_reg */},
};


static struct per_intr_info per_intr_info[] = {
	[GIC15] = {"uio_pengic15", "pp_port_c_int_c_mac_test_set",
		PP_PORT_C_INT_C_MAC_INTREG, 0x80, 0x80,
		reg_setup_gic15, ARRAY_SIZE(reg_setup_gic15),
		read_regs_gic15, ARRAY_SIZE(read_regs_gic15)},
	[GIC17] = {"uio_pengic17", "pxb_int_itr_ecc_test_set",
		PXB_INT_ITR_ECC_INTREG, 0xffff, 0xffff,
		reg_setup_gic17, ARRAY_SIZE(reg_setup_gic17),
		read_regs_gic17, ARRAY_SIZE(read_regs_gic17)},
	[GIC18] = {"uio_pengic18", "mc0_int_mc_test_set",
		MC0_INT_MC_INTREG, 0x1, 0xf,
		reg_setup_gic18, ARRAY_SIZE(reg_setup_gic18),
		read_regs_gic18, ARRAY_SIZE(read_regs_gic18)},
};

static void error_exit(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));
static void error_exit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

static const char *reg_name(paddr_t paddr)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(reg_names); i++) {
		if (paddr == reg_names[i].paddr)
			return reg_names[i].name;
	}

	return "<unknown>";
}

static __attribute((unused)) void print_mapping(struct pal_int *pal_int)
{
	unsigned i;

	for (i = 0; i < pal_int->n_mappings; i++) {
		struct mapping_info *mi;
		paddr_t paddr;
		void *vaddr;
		size_t devsize;

		mi = &pal_int->mapping_info[i];
		paddr = mi->paddr;
		vaddr = mi->vaddr;
		devsize = mi->devsize;
		printf("Mapping #%u: physical 0x%lx virtual %p size 0x%zx\n",
			i + 1u, paddr, vaddr, devsize);
	}
}

/* Print read/write information */
static void print_rw_info(paddr_t paddr, int32_t value)
{
		printf("%#*lx: %#*x %s\n", 2 + 2 * (int)sizeof(paddr),
			paddr, 2 + 2 * (int)sizeof(value), value,
			reg_name(paddr));
}

static void write_setup_regs(struct pal_int *pal_int,
	const struct reg_setup *reg_setup, size_t n, bool verbose)
{
	unsigned i;

	printf("Writing setup registers:\n");
	for (i = 0; i < n; i++) {
		paddr_t paddr;
		uint32_t value;

		paddr = reg_setup[i].paddr;
		value = reg_setup[i].value;
		if (verbose)
			print_rw_info(paddr, value);
		pal_reg_wr32(paddr, value);
	}
}

static __attribute__((unused)) void assert_test(struct pal_int *pal_int,
	enum int_source source, bool verbose)
{
	uint32_t trigger_value;
	uint64_t paddr;

	if (verbose)
		printf("Trigger write:\n");
	paddr = pal_int->csr_paddr + offsetof(struct csr, test_set);
	trigger_value = per_intr_info[source].test_trigger;
	if (verbose)
		print_rw_info(paddr, trigger_value);
	pal_reg_wr32(paddr, trigger_value);
}

static void deassert_test(struct pal_int *pal_int, enum int_source source,
	bool verbose)
{
	uint32_t untrigger_value;
	uint32_t intreg_off;
	paddr_t paddr;

	if (verbose)
		printf("Trigger clear:\n");
	paddr = pal_int->csr_paddr + offsetof(struct csr, test_set);
	/* Write zero to test register */
	untrigger_value = 0x0;
	if (verbose)
		print_rw_info(paddr, untrigger_value);
	pal_reg_wr32(paddr, untrigger_value);

	/* Turn off the interrupt */
	paddr = pal_int->csr_paddr + offsetof(struct csr, intreg);
	intreg_off = per_intr_info[source].intreg_off;
	if (verbose)
		printf("...and write 0x%x to corresponding intreg at 0x%lx\n",
			intreg_off, paddr);
	pal_reg_wr32(paddr, intreg_off);
}

static void setup_device_registers(struct pal_int *pal_int,
	enum int_source source, bool verbose)
{
#if 0
	struct csr *csr;
	paddr_t paddr;
	const char *name;
	int32_t value;
#endif

#if 0
	printf("# Clear test register\n");
	paddr = per_intr_info[source].intreg_paddr;
	csr = pal_int_paddr_to_vaddr(pal_int, paddr);
	if (csr == NULL)
		error_exit("%s: no mapping for paddr 0x%lx\n", __func__, paddr);
	name = per_intr_info[source].intreg_name;
	value = 0;
	if (verbose)
		printf("%#*lx: %#*x %s->test_set\n", 2 + 2 * (int)sizeof(paddr),
			paddr + offsetof(struct csr, test_set),
			2 + 2 * (int)sizeof(value), value, name);
	pal_int_write32(pal_int, 0x0, &csr->test_set);
#else
	deassert_test(pal_int, source, verbose);
#endif

	switch (source) {
	case GIC15:
		printf("# Write %zu GIC15-oriented registers\n",
			ARRAY_SIZE(reg_setup_gic15));
		write_setup_regs(pal_int, per_intr_info[source].reg_setup,
			per_intr_info[source].n_reg_setup, verbose);
		printf("# Write %zu GIC17-oriented registers\n", (size_t)0);
		printf("# Write %zu GIC18-oriented registers\n", (size_t)0);
		break;

	case GIC17:
		printf("# Write %zu GIC15-oriented registers\n", (size_t)0);
		printf("# Write %zu GIC17-oriented registers\n",
			ARRAY_SIZE(reg_setup_gic17));
		write_setup_regs(pal_int, per_intr_info[source].reg_setup,
			per_intr_info[source].n_reg_setup, verbose);
		printf("# Write %zu GIC18-oriented registers\n", (size_t)0);
		break;

	case GIC18:
		printf("# Write %zu GIC15-oriented registers\n", (size_t)0);
		printf("# Write %zu GIC17-oriented registers\n", (size_t)0);
		printf("# Write %zu GIC18-oriented registers\n",
			ARRAY_SIZE(reg_setup_gic18));
		write_setup_regs(pal_int, per_intr_info[source].reg_setup,
			per_intr_info[source].n_reg_setup, verbose);
		break;

	default:
		error_exit("Unknown source: %d\n", source);
		break;
	}
}

static void print_reg(struct pal_int *pal_int, paddr_t paddr)
{
	int32_t value;

	value = pal_reg_rd32(paddr);
	print_rw_info(paddr, value);
}

/* Print the cascading of an interrupt through registers */
static void print_regs(struct pal_int *pal_int, enum int_source source)
{
	unsigned i;

	printf("Reading registers\n");
	for (i = 0; i < per_intr_info[source].n_read_reg; i++) {
		paddr_t paddr;

		paddr = per_intr_info[source].read_reg[i].paddr;
		print_reg(pal_int, paddr);
	}
}

static void sigalrm_handler(int sig)
{
	if (siginfo.verbose)
		print_regs(siginfo.pal_int, siginfo.source);
	error_exit("Interrupt didn't trigger in %ld.%06ld seconds\n",
		timeout.it_value.tv_sec, timeout.it_value.tv_usec);
}

/*
 * Loop reading interrupts and enabling the device.
 *
 */
static void main_loop(struct pal_int *pal_int, enum int_source source,
	bool verbose, unsigned long n_loops)
{
	static struct itimerval cancel;
	struct sigaction act;
	unsigned i;
	int rc;

	rc = pal_int_open(pal_int, per_intr_info[source].name);
	if (rc == -1)
		error_exit("pal_int_open(%s) failed: %m (%d)\n",
			per_intr_info[source].name, errno);

	if (verbose)
		print_mapping(pal_int);

	memset(&act, 0, sizeof(act));
	act.sa_handler = sigalrm_handler;
	rc = sigaction(SIGALRM, &act, NULL);
	if (rc == -1)
		error_exit("sigaction failed: %m\n");

	setup_device_registers(pal_int, source, verbose);

	if (verbose)
		print_regs(pal_int, source);
	
	/* Save registers for interrupt handler */
	siginfo.pal_int = pal_int;
	siginfo.verbose = verbose;
	siginfo.source = source;

	for (i = 0; i < n_loops; i++) {
		ssize_t rc;

		printf("# Trigger interrupt #%u\n", i + 1);

		/* Set or reset the timer */
		rc = setitimer(ITIMER_REAL, &timeout, NULL);
		if (rc != 0)
			error_exit("setitimer for timeout failed: %m\n");

		/* Trigger the interrupt */
		assert_test(pal_int, source, verbose);

		/* Wait for the interrupt to happen */
		rc = pal_int_start(pal_int);
		if (rc == -1)
			error_exit("pal_int_start failed\n");
		if (verbose)
			print_regs(pal_int, source);

		printf("Interrupt received, %zd interrupts\n", rc);

		/* Stop the timer */
		rc = setitimer(ITIMER_REAL, &cancel, NULL);
		if (rc != 0)
			error_exit("setitimer for timeout failed: %m\n");

		/* Turn off the interrupt source */
		deassert_test(pal_int, source, verbose);

		/* Re-enable interrupts */
		rc = pal_int_end(pal_int);
		if (rc == -1)
			error_exit("pal_int_end failed\n");
	}

	rc = pal_int_close(pal_int);
	if (rc != 0)
		error_exit("pal_int_close failed: %m (%d)\n", errno);
}

static void usage(const char *name)
{
	error_exit("usage: %s -v -n num [ gic15 | gic17 | gic18 ]\n"
		   "where:	-v	Verbose output\n"
		   "		-n num	Number of loops\n",
		name);
}

int main(int argc, char *argv[])
{
	struct pal_int pal_int;
	enum int_source source;
	char *which_gic;
	char *endptr;
	unsigned n_args;
	unsigned long n_loops;
	int opt;
	bool verbose;

	verbose = false;
	n_loops = 5;

	while ((opt = getopt(argc, argv, "n:v")) != -1) {
		switch (opt) {
		case 'v':
			verbose = 1;
			break;

		case 'n':
			n_loops = strtoul(optarg, &endptr, 0);
			if (*optarg == '\0' || *endptr != '\0') {
				fprintf(stderr, "Invalid number for -n\n");
				usage(argv[0]);
			}
			break;

		default:
			usage(argv[0]);
			break;
		}
	}

	n_args = argc - optind;
	if (n_args != 1)
		usage(argv[0]);

	which_gic = argv[optind];
	if (strcmp(which_gic, "gic15") == 0)
		source = GIC15;
	else if (strcmp(which_gic, "gic17") == 0)
		source = GIC17;
	else if (strcmp(which_gic, "gic18") == 0)
		source = GIC18;
	else
		usage(argv[0]);

	main_loop(&pal_int, source, verbose, n_loops);

	return 0;
}
