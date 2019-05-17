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
#include <time.h>
#include <unistd.h>

#include "platform/pal/include/pal_int.h"
#include "platform/pal/include/pal_reg.h"

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))

/* Number of repetitions in the main loop */
#define N_LOOPS_DEFAULT	5

/*
 * struct options - user-supplied options
 * verbose:		Print more information
 * setup:		Set up the registers, otherwise let the kernel do it
 * prepause:		Pause to see messages before performing actions
 * n_loops:		Number of times to get an interrupt
 * exit_in_between:	Exit after calling pal_int_start() and before calling
 *			pal_int_end()
 */
struct options {
	bool		verbose;
	bool		setup;
	bool		prepause;
	unsigned long	n_loops;
	bool		exit_in_between;
};

/*
 * This is the value to which we will set an interval time to catch
 * unexpect hangs quickly.
 */
static const struct itimerval timeout = {
	.it_value = {
		.tv_sec = 25,
		.tv_usec = 0,
	}
};

enum reg_type {
	REG_TYPE_UNKNOWN,
	REG_TYPE_CSR,
	REG_TYPE_GRP,
	REG_TYPE_CSRINTR,
};

/*
 * Since this is a test program, we define these here instead of the
 * device tree. These are addresses of the base of the CSR. Note that
 * only CSR registers have the int_test_set register so, even if we only
 * own a higher, non-CSR register, we have to trigger in a CSR register.
 */
#define PP_INT_PP_INTREG		0x070202e0lu	/* PCIEMAC */
#define PCIEMAC_INTREG			PP_INT_PP_INTREG
#define PCIEMAC_TRIGGER			(1 << 17)
#define PCIEMAC_REG_TYPE		REG_TYPE_CSR

#define MX0_INT_MAC_INTREG		0x01d82080lu	/* LINKMAC0 */
#define MX1_INT_MAC_INTREG		0x01e82080lu	/* LINKMAC1 */

#define LINKMAC0_INTREG			MX0_INT_MAC_INTREG
#define LINKMAC0_TRIGGER		(1 << 7)
#define LINKMAC0_REG_TYPE		REG_TYPE_CSR

#define LINKMAC1_INTREG			MX1_INT_MAC_INTREG
#define LINKMAC1_TRIGGER		(1 << 7)
#define LINKMAC1_REG_TYPE		REG_TYPE_CSR


#define MS_STA_CHIP_PADDR	0x6a000008	/* Version register */
#define MS_STA_CHIP_VERSION	0x7ffe		/* Expected value */

enum int_source {
	PCIEMAC,
	LINKMAC0,
	LINKMAC1,
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
	struct options	*opts;
	bool		between_start_and_end;
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
 * Group interrupt registers
 * @intreg:		The bit corresponding to an interrupt source is one
 *			if that interrupt is asserted.
 * @enable_rw_reg:	Write a one to enable an interrupt, zero to disable it
 * @rw_reg:		One for each asserted interrupt
 */
struct group {
	uint32_t	intreg;
	uint32_t	enable_rw_reg;
	uint32_t	rw_reg;
};

struct csr_intr {
	uint32_t	intr;
};

/*
 * Per GIC information, indexed by enum int_source
 * name:		Name of the device driver
 * subname:		Some devices are broken into subdevices. This is the
 *			subdevice name.
 * intreg_paddr:	Physical address of the register we are using to
 * 			trigger an interrupt. This must be a CSR register
 * 			block
 * test_trigger:	Value to write to to the int_test_set of the register
 *			block at intreg_paddr to trigger an interrupt
 * intreg_off:		Value to write to the intreg register of the register
 *			block at intreg_paddr to reset the interrupt latch
 * reg_clear:		Pointer to values to write to clear the register being
 * 			used to their reset state
 * n_ret_clear:		Number of values in reg_clear
 * reg_setup:		Register values used to set up for the test
 * n_reg_setup:		Number of values in reg_setup
 * read_reg:		Registers to read and print
 * n_read_reg:		Number of registers specified in read_reg
 */
struct per_intr_info {
	const char 		*name;
	const char		*subname;
	paddr_t			intreg_paddr;
	uint32_t		test_trigger;
	uint32_t		intreg_off;
	const struct reg_setup	*reg_clear;
	size_t			n_reg_clear;
	const struct reg_setup	*reg_setup;
	size_t			n_reg_setup;
	const struct read_reg	*read_reg;
	size_t			n_read_reg;
};

struct reg_name {
	paddr_t		paddr;
	const char	*name;
};

static bool test_asserted;
static struct siginfo siginfo;
static const struct reg_name reg_names[] = {
	{0x01d82060, "mx0_intr"},
	{0x01d82070, "mx0_int_groups_intreg"},
	{0x01d82074, "mx0_int_groups_int_enable_rw_reg"},
	{0x01d82078, "mx0_int_groups_int_rw_reg"},
	{0x01d82080, "mx0_int_mac_intreg"},
	{0x01d82084, "mx0_int_mac_int_test_set"},
	{0x01d82088, "mx0_int_mac_int_enable_set"},
	{0x01d8208c, "mx0_int_mac_int_enable_clear"},
	{0x01e82060, "mx1_intr"},
	{0x01e82070, "mx1_int_groups_intreg"},
	{0x01e82074, "mx1_int_groups_int_enable_rw_reg"},
	{0x01e82078, "mx1_int_groups_int_rw_reg"},
	{0x01e82080, "mx1_int_mac_intreg"},
	{0x01e82084, "mx1_int_mac_int_test_set"},
	{0x01e82088, "mx1_int_mac_int_enable_set"},
	{0x01e8208c, "mx1_int_mac_int_enable_clear"},
	{0x0701119c, "pp_port_c_intr"},
	{0x070111a0, "pp_port_c_int_groups_intreg"},
	{0x070111a4, "pp_port_c_int_groups_int_enable_rw_reg"},
	{0x070111a8, "pp_port_c_int_groups_int_enable_rw_reg"},
	{0x070111b0, "pp_port_c_int_c_mac_intreg"},
	{0x070111b4, "pp_port_c_int_c_mac_int_test_set"},
	{0x070111b8, "pp_port_c_int_c_mac_int_enable_set"},
	{0x070111bc, "pp_port_c_int_c_mac_int_enable_clear"},
	{0x070202c8, "pp_intr"},
	{0x070202cc, "<unknown>"},
	{0x070202d0, "pp_int_groups_intreg"},
	{0x070202d4, "pp_int_groups_int_enable_rw_reg"},
	{0x070202e0, "pp_int_pp_intreg"},
	{0x070202e4, "pp_int_pp_int_test_set"},
	{0x070202e8, "pp_int_pp_int_enable_set"},
	{0x070202ec, "pp_int_pp_int_enable_clear"},
	{0x0719a578, "cap0.pxb.pxb.csr_intr"},
	{0x0719a580, "cap0.pxb.pxb.int_groups.intreg"},
	{0x0719a584, "cap0.pxb.pxb.int_groups.int_enable_rw_reg"},
	{0x0719a590, "cap0.pxb.pxb.int_itr_ecc.intreg"},
	{0x0719a590, "pxb_int_intr_ecc_intreg"},
	{0x0719a594, "pxb_int_intr_ecc_int_test_set"},
	{0x0719a598, "cap0.pxb.pxb.int_itr_ecc.int_enable_set"},
	{0x0719a5a0, "cap0.pxb.pxb.int_tgt_ecc.intreg"},
	{0x0719a5a8, "cap0.pxb.pxb.int_tgt_ecc.int_enable_set"},
	{0x0719a5b0, "cap0.pxb.pxb.int_err.intreg"},
	{0x0719a5b8, "cap0.pxb.pxb.int_err.int_enable_set"},
 	{0x6a001040, "ms_intr"},
	{0x6a001050, "ms_int_groups_intreg"},
	{0x6a001054, "ms_int_groups_int_enable_rw_reg"},
	{0x6a0011c0, "ms_int_gic14_intreg"},
	{0x6a0011c4, "ms_int_gic14_int_test_set"},
	{0x6a0011c8, "ms_int_gic14_int_enable_set"},
	{0x6a0011cc, "ms_int_gic14_int_enable_clear"},
	{0x6a0011f0, "ms_int_gic17_intreg"},
	{0x6a0011f4, "ms_int_gic17_int_test_set"},
	{0x6a0011f8, "ms_int_gic17_int_enable_set"},
	{0x6a0011fc, "ms_int_gic17_int_enable_clear"},
	{0x6a100048, "cap0.mc.mc0.mch.intr"},
	{0x6a100050, "cap0.mc.mc0.mch.int_groups.intreg"},
	{0x6a100054, "cap0.mc.mc0.mch.int_groups.int_enable_rw_reg"},
	{0x6a100058, "cap0.mc.mc0.mch.int_groups.int_rw_reg"},
	{0x6a100060, "cap0.mc.mc0.mch.int_mc.intreg"},
	{0x6a100068, "cap0.mc.mc0.mch.int_mc.int_enable_set"},
	{0x6a180000, "mc0_mc_cfg"},
	{0x6a180004, "mc0_intr"},
	{0x6a180010, "mc0_int_groups_intreg"},
	{0x6a180014, "cap0.mc.mc0.int_groups.int_enable_rw_reg"},
	{0x6a180018, "mc0_int_groups_int_rw_reg"},
	{0x6a180020, "cap0.mc.mc0.int_mc.intreg"},
	{0x6a180024, "mc0_int_mc_int_test_set"},
	{0x6a180028, "cap0.mc.mc0.int_mc.int_enable_set"},
};

static const struct reg_setup reg_clear_pciemac[] = {
	{0x070111bc, 0x7ffff /* pp_port_c_int_c_mac_int_enable_clear */},
	{0x070111b0, 0x7ffff /* pp_port_c_int_c_mac_intreg */},
	{0x070111a0, 0x0 /* pp_port_c_int_groups_intreg */},
	{0x0701119c, 0x0 /* pp_port_c_intr */},

	{0x070202ec, 0x7ffffff /* pp_int_pp_int_enable_clear */},
	{0x070202e0, 0x7ffffff /* pp_int_pp_intreg */},
	{0x070202d0, 0x0 /* pp_int_groups_intreg */},

	{0x6a0011f0, 0x1 /* ms_int_gic17_intreg */},
	{0x6a0011fc, 0x1 /* ms_int_gic17_int_enable_clear */},
	{0x6a001050, 0x0 /* ms_int_groups_intreg */},
};

/* Values to be written to configure for testing */
static const struct reg_setup reg_setup_pciemac[] = {
	{0x070202e8, /* pp_int_pp_int_enable_set */ 1 << 17},
	{0x070202d4, /* pp_int_groups_int_enable_rw_reg */ 0x1},
};

static const struct read_reg read_regs_pciemac[] = {
	{0x070111b8 /* pp_port_c_int_c_mac_int_enable_set */},
	{0x070111b0 /* pp_port_c_int_c_mac_intreg */},

	{0x070111a4 /* pp_port_c_int_groups_int_enable_rw_reg */},
	{0x070111a0 /* pp_port_c_int_groups_intreg */},

	{0x0701119c /* pp_port_c_intr */},

	{0x070202e8 /* pp_int_pp_int_enable_set */},
	{0x070202e0 /* pp_int_pp_intreg */},

	{0x070202d4 /* pp_int_groups_intreg */},
	{0x070202d0 /* pp_int_groups_int_enable_rw_reg */},

	{0x070202c8 /* pp_intr */},

	{0x6a0011f8 /* ms_int_gic17_int_enable_set */},
	{0x6a0011f0 /* ms_int_gic17_intreg */},

	{0x6a001054 /* ms_int_groups_int_enable_rw_reg */},
	{0x6a001050 /* ms_int_groups_intreg */},

 	{0x6a001040 /* ms_intr */},
};

/*
 * Clear and set up from the leaf register to mx0_intr/mx1_intr. We don't
 * do anything with those registers because they are owned by the kernel
 */
static const struct reg_setup reg_clear_linkmac[] = {
	{0x01d8208c, 0x3fffffff /* mx0_int_mac_int_enable_clear */},
	{0x01d82080, 0x3fffffff /* mx0_int_mac_intreg */},
	{0x01d82074, 0x0 /* mx0_int_groups_int_enable_rw_reg */},

	{0x01e8208c, 0x3fffffff /* mx1_int_mac_int_enable_clear */},
	{0x01e82080, 0x3fffffff /* mx1_int_mac_intreg */},
	{0x01e82074, 0x0 /* mx1_int_groups_int_enable_rw_reg */},
};

static const struct reg_setup reg_setup_linkmac[] = {
	{0x01d82088, 0x80 /* mx0_int_mac_int_enable_set */},
	{0x01d82074, 0x1 /* mx0_int_groups_int_enable_rw_reg */},

	{0x01e82088, 0x80 /* mx1_int_mac_int_enable_set */},
	{0x01e82074, 0x1 /* mx1_int_groups_int_enable_rw_reg */},
};

static const struct read_reg read_regs_linkmac0[] = {
	/* May be wrong */
	{0x01d82088}, /* mx0_int_mac_int_enable_set =0x100 */
	/* expect umac_CFG3_intr_interrupt set */
	{0x01d82080}, /* mx0_int_mac_intreg */

	{0x01d82074}, /* mx0_int_groups_int_enable_rw_reg =0x1 */
	/* expect int_mac_interrupt set */
	{0x01d82070}, /* mx0_int_groups_intreg */

	{0x01d82060}, /* mx0_intr =0x2 */
	{0x01e82060}, /* mx1_intr */
};

static const struct read_reg read_regs_linkmac1[] = {
	/* May be wrong */
	{0x01e82088}, /* mx1_int_mac_int_enable_set =0x100 */
	/* expect umac_CFG3_intr_interrupt set */
	{0x01e82080}, /* mx1_int_mac_intreg */

	{0x01e82074}, /* mx1_int_groups_int_enable_rw_reg =0x1 */
	/* expect int_mac_interrupt set */
	{0x01e82070}, /* mx1_int_groups_intreg */

	{0x01e82060}, /* mx1_intr =0x2 */
	{0x01d82060}, /* mx0_intr */
};

static struct per_intr_info per_intr_info[] = {
	[PCIEMAC] = {
		.name = "pciemac",
		.subname = "pciemac",

		.intreg_paddr = PCIEMAC_INTREG,
		.test_trigger = PCIEMAC_TRIGGER,
		.intreg_off = PCIEMAC_TRIGGER,

		.reg_clear = reg_clear_pciemac,
		.n_reg_clear = ARRAY_SIZE(reg_clear_pciemac),
		.reg_setup = reg_setup_pciemac,
		.n_reg_setup = ARRAY_SIZE(reg_setup_pciemac),
		.read_reg = read_regs_pciemac,
		.n_read_reg = ARRAY_SIZE(read_regs_pciemac),
	},
	[LINKMAC0] = {
		.name = "linkmac", 
		.subname = "linkmac0",
		.intreg_paddr = LINKMAC0_INTREG,
		.test_trigger = LINKMAC0_TRIGGER,
		.intreg_off = LINKMAC0_TRIGGER,

		.reg_clear = reg_clear_linkmac,
		.n_reg_clear = ARRAY_SIZE(reg_clear_linkmac),
		.reg_setup = reg_setup_linkmac,
		.n_reg_setup = ARRAY_SIZE(reg_setup_linkmac),
		.read_reg = read_regs_linkmac0,
		.n_read_reg = ARRAY_SIZE(read_regs_linkmac0),
	},
	[LINKMAC1] = {
		.name = "linkmac", 
		.subname = "linkmac1",
		.intreg_paddr = LINKMAC1_INTREG,
		.test_trigger = LINKMAC1_TRIGGER,
		.intreg_off = LINKMAC1_TRIGGER,

		.reg_clear = reg_clear_linkmac,
		.n_reg_clear = ARRAY_SIZE(reg_clear_linkmac),
		.reg_setup = reg_setup_linkmac,
		.n_reg_setup = ARRAY_SIZE(reg_setup_linkmac),
		.read_reg = read_regs_linkmac1,
		.n_read_reg = ARRAY_SIZE(read_regs_linkmac1),
	},
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

/*
 * Write memory barrier-wait until all write operations begun before this
 * instruction have completed
 */
static void __attribute__((used)) wmb(void)
{
	asm volatile (
		"	dsb	st\n"
		: :
	);
}

/*
 * Read memory barrier-wait until all read operations begun before this
 * instruction have completed
 */
static void __attribute__((used)) rmb(void)
{
	asm volatile (
		"	dsb	ld\n"
		: :
	);
}

static void wr32(uint32_t value, paddr_t paddr)
{
	pal_reg_wr32(value, paddr);
	wmb();
}

static int32_t rd32(paddr_t paddr)
{
	rmb();
	return pal_reg_rd32(paddr);
}

/*
 * Pause for the given amount of time if the options say to do so
 */
static void prepause(unsigned int seconds, struct options *opts)
{
	if (opts->prepause)
		sleep(seconds);
}

void reenable(struct pal_int *pal_int, struct options *opts)
{
	int rc;

	if (opts->verbose) {
		printf("calling pal_int_end\n");
		prepause(1, opts);
	}
	rc = pal_int_end(pal_int);
	if (rc == -1) 
		error_exit("pal_int_end failed\n");
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
	printf("%#*lx: %#*x %s\n", 2 + 2 * (int)sizeof(paddr), paddr,
		2 + 2 * (int)sizeof(value), value, reg_name(paddr));
}

static void print_reg(struct pal_int *pal_int, paddr_t paddr,
	struct options *opts)
{
	int32_t value;

	value = rd32(paddr);
	prepause(1, opts);

	print_rw_info(paddr, value);
}

/* Print the cascading of an interrupt through registers */
static void print_regs(struct pal_int *pal_int, enum int_source source,
	struct options *opts)
{
	unsigned i;

	for (i = 0; i < per_intr_info[source].n_read_reg; i++) {
		paddr_t paddr;

		paddr = per_intr_info[source].read_reg[i].paddr;
		print_reg(pal_int, paddr, opts);
	}
}

static void write_regs(struct pal_int *pal_int,
	const struct reg_setup *regs, size_t n, struct options *opts)
{
	unsigned i;

	if (!opts->setup)
		return;

	for (i = 0; i < n; i++) {
		paddr_t paddr;
		uint32_t value;
//print_reg(pal_int, 0x1d82060, opts);
		paddr = regs[i].paddr;
		value = regs[i].value;
		if (opts->verbose) {
			print_rw_info(paddr, value);
			prepause(1, opts);
		}
		wr32(paddr, value);
	}
//print_reg(pal_int, 0x1d82060, opts);
}

static void clear_regs(struct pal_int *pal_int,
	const struct reg_setup *regs, size_t n, struct options *opts)
{
	if (!opts->setup)
		return;
	printf("Clearing registers:\n");
	write_regs(pal_int, regs, n, opts);
}

static void write_setup_regs(struct pal_int *pal_int,
	const struct reg_setup *reg_setup, size_t n, struct options *opts)
{
	if (!opts->setup) {
		printf("Not writing setup registers\n");
		return;
	}
	printf("Writing setup registers:\n");
	write_regs(pal_int, reg_setup, n, opts);
}

static __attribute__((unused)) void assert_test(struct pal_int *pal_int,
	enum int_source source, struct options *opts)
{
	uint32_t trigger_value;
	uint64_t paddr;
	ptrdiff_t delta;

	delta = offsetof(struct csr, test_set);
	paddr = per_intr_info[source].intreg_paddr + delta;
	trigger_value = per_intr_info[source].test_trigger;
	if (opts->verbose) {
		print_rw_info(paddr, trigger_value);
		prepause(1, opts);
	}
	test_asserted = true;
	wr32(paddr, trigger_value);
}

static void deassert_test(struct pal_int *pal_int, enum int_source source,
	struct options *opts)
{
	uint32_t untrigger_value;
	uint32_t intreg_off;
	paddr_t paddr;

	if (opts->verbose) {
		printf("Trigger clear:\n");
	}
	paddr = per_intr_info[source].intreg_paddr +
		offsetof(struct csr, test_set);
	/* Write zero to test register */
	untrigger_value = 0x0;
	if (opts->verbose) {
		print_rw_info(paddr, untrigger_value);
		prepause(1, opts);
	}
	wr32(paddr, untrigger_value);

	/* Turn off the interrupt */
	paddr = per_intr_info[source].intreg_paddr +
		offsetof(struct csr, intreg);
	intreg_off = per_intr_info[source].intreg_off;
	if (opts->verbose) {
		print_rw_info(paddr, intreg_off);
		prepause(1, opts);
	}
	wr32(paddr, intreg_off);
	test_asserted = false;
}

static void setup_device_registers(struct pal_int *pal_int,
	enum int_source source, struct options *opts)
{
	if (!opts->setup) {
		printf("Skipping register setup\n");
		return;
	}
	deassert_test(pal_int, source, opts);
	clear_regs(pal_int, per_intr_info[source].reg_clear,
		per_intr_info[source].n_reg_clear, opts);
	if (opts->verbose) {
		printf("Registers after clearing:\n");
		print_regs(pal_int, source, opts);
	}
	write_setup_regs(pal_int, per_intr_info[source].reg_setup,
		per_intr_info[source].n_reg_setup, opts);
}

static void sigalrm_handler(int sig)
{
	if (siginfo.verbose) {
		printf("Registers after trigger failed:\n");
		print_regs(siginfo.pal_int, siginfo.source, siginfo.opts);
	}

	if (test_asserted)
		deassert_test(siginfo.pal_int, siginfo.source, siginfo.opts);
	if (siginfo.between_start_and_end)
		reenable(siginfo.pal_int, siginfo.opts);
	print_regs(siginfo.pal_int, siginfo.source, siginfo.opts);
	error_exit("Interrupt didn't trigger in %ld.%06ld seconds\n",
		timeout.it_value.tv_sec, timeout.it_value.tv_usec);
}

static void exit_with_activity(struct pal_int *pal_int, enum int_source source,
	struct options *opts)
{
	/* Inject fake activity */
	printf("Injecting fake activity\n");
	fflush(stdout);
	prepause(1, opts);
	assert_test(pal_int, source, opts);
	printf("Exiting after asserting test and before pal_int_end()\n");
	exit(1);
}

/*
 * Loop reading interrupts and enabling the device.
 *
 */
static void main_loop(struct pal_int *pal_int, enum int_source source,
	struct options *opts)
{
	static struct itimerval cancel;
	struct sigaction act;
	unsigned i;
	int rc;

	printf("Testing %s\n", per_intr_info[source].subname);

	if (opts->verbose) {
		printf("Opening \"%s\"\n", per_intr_info[source].name);
		prepause(1, opts);
	}

	rc = pal_int_open(pal_int, per_intr_info[source].name);
	if (rc == -1)
		error_exit("pal_int_open(%s) failed: %m (%d)\n",
			per_intr_info[source].name, errno);

	if (opts->verbose)
		print_mapping(pal_int);

	memset(&act, 0, sizeof(act));
	act.sa_handler = sigalrm_handler;
	rc = sigaction(SIGALRM, &act, NULL);
	if (rc == -1)
		error_exit("sigaction failed: %m\n");

	setup_device_registers(pal_int, source, opts);

#if 0
	if (opts->verbose)
		print_regs(pal_int, source, opts);
#endif
	
	/* Save options for interrupt handler */
	siginfo.pal_int = pal_int;
	siginfo.verbose = opts->verbose;
	siginfo.source = source;
	siginfo.opts = opts;

	for (i = 0; i < opts->n_loops; i++) {
		ssize_t rc;

		if (opts->verbose) {
			printf("Registers before trigger:\n");
			print_regs(pal_int, source, opts);
		}
		prepause(1, opts);
		printf("# Trigger interrupt #%u: ", i + 1);
		fflush(stdout);

		/* Set or reset the timer */
		rc = setitimer(ITIMER_REAL, &timeout, NULL);
		if (rc != 0) {
			deassert_test(pal_int, source, opts);
			error_exit("setitimer for timeout failed: %m\n");
		}

		/* Trigger the interrupt */
		assert_test(pal_int, source, opts);
		prepause(1, opts);
		if (opts->verbose) {
			printf("After trigger:\n");
			print_regs(pal_int, source, opts);
		}

		/* Wait for the interrupt to happen */
		siginfo.between_start_and_end = true;
		rc = pal_int_start(pal_int);
		if (rc == -1) {
			deassert_test(pal_int, source, opts);
			error_exit("pal_int_start failed\n");
		}
		if (opts->verbose) {
			printf("After call to pal_int_start:\n");
			print_regs(pal_int, source, opts);
		}

		printf("Interrupt received, %zd interrupts\n", rc);

		/* Stop the timer */
		rc = setitimer(ITIMER_REAL, &cancel, NULL);
		if (rc != 0)
			error_exit("setitimer for timeout failed: %m\n");

		/* Turn off the interrupt source */
		deassert_test(pal_int, source, opts);

		if (opts->exit_in_between)
			exit_with_activity(pal_int, source, opts);

		if (opts->exit_in_between) {
			/* Inject fake activity */
			printf("Injecting fake activity\n");
			fflush(stdout);
			prepause(1, opts);
			assert_test(pal_int, source, opts);
			printf("Exiting after asserting test and before pal_int_end()\n");
			exit(1);
		}
		/* Re-enable interrupts */
		reenable(pal_int, opts);
		siginfo.between_start_and_end = false;
	}

	if (opts->verbose) {
		printf("closing UIO file descriptor\n");
		prepause(1, opts);
	}

	rc = pal_int_close(pal_int);
	if (rc != 0)
		error_exit("pal_int_close failed: %m (%d)\n", errno);
}

static void usage(const char *name)
{
	error_exit("usage: %s -vsx -n num [ pciemac | linkmac0 | linkmac1 ]\n"
		"where:	-n num	Number of loops\n"
		"	-s	Run register setup code\n"
	   	"	-v	Verbose output\n"
		"	-x	Exit the program after calling\n"
		"		pal_int_start() and pal_int_end()\n",
		name);
}

int main(int argc, char *argv[])
{
	struct pal_int pal_int;
	enum int_source source;
	char *which_gic;
	char *endptr;
	unsigned n_args;
	int opt;
	struct options opts;
	enum int_source i;

	opts.prepause = false;
	opts.setup = false;
	opts.verbose = false;
	opts.n_loops = 5;
	opts.exit_in_between = 0;

	while ((opt = getopt(argc, argv, "n:psvx")) != -1) {
		switch (opt) {
		case 'n':
			opts.n_loops = strtoul(optarg, &endptr, 0);
			if (*optarg == '\0' || *endptr != '\0') {
				fprintf(stderr, "Invalid number for -n\n");
				usage(argv[0]);
			}
			break;

		case 'p':
			opts.prepause = true;
			break;

		case 's':
			opts.setup = true;
			break;

		case 'v':
			opts.verbose = true;
			break;

		case 'x':
			opts.exit_in_between = true;
			break;

		default:
			fprintf(stderr, "Invalid option %c\n", opt);
			usage(argv[0]);
			break;
		}
	}

	n_args = argc - optind;
	if (n_args != 1) {
		fprintf(stderr, "Too many arguments\n");
		usage(argv[0]);
	}

	which_gic = argv[optind];

	for (i = 0; i < ARRAY_SIZE(per_intr_info); i++) {
		if (strcmp(which_gic, per_intr_info[i].subname) == 0)
			break;
	}

	if (i == ARRAY_SIZE(per_intr_info))
		error_exit("Unknown device: %s\n", which_gic);

	source = i;

	main_loop(&pal_int, source, &opts);

	return 0;
}
