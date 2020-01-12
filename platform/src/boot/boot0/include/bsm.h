
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __BSM_H__
#define __BSM_H__

/*
 * Boot State Machine
 * Bits:
 *      10      wdt         0=disarmed, 1=armed
 *      9:8     fwid        BSM_FWID_xxx
 *      7:6     attempt     0..3 boot attempt
 *      5:4     track       BSM_FWID_xxx
 *      3:2     stage       BSM_STG_xxx
 *      1       running     0=bsm idle, 1=bsm running
 *      0       autoboot    0=init/manual, 1=autobooting
 *
 * When autobooting, the first-stage boot code selects a boot 'track', which
 * defines a sequence of images that will attempt to boot, in order.  For
 * example, for the MAINA track, the images main be MAINA, MAINB, GOLD, GOLD,
 * meaning that the MAINA image set will attempt to boot first.  If that fails
 * then the MAINB image set will attempt to boot, and so on.
 *
 * For each attempt at booting, 'fwid' will identify the actual image set that
 * is to be loaded.
 *
 * Upon failure the 'attempt' is incremented and the next fwid appropriate
 * for the track is tried.  After the max number of attempts (4), the boot
 * will halt.
 *
 * The 'stage' field tracks the current boot stage:
 *   STG_BOOT0, STG_UBOOT, STG_KERNEL
 *
 * The 'running' field is 1 if the BSM is still running (trying to boot), and
 * restarts while running == 1 will result in the next image option being
 * loaded.  At the end of a successful boot, running will be set to 0.
 *
 * On entry to the kernel, if u-boot cannot determine if the kernel is
 * BSM-savvy or not, then it will set stage = STG_KERNEL and running = 0.  A
 * kernel that understands the BSM can set running = 1 to contiue the BSM.
 *
 * Upon successful autoboot, the BSM stage will be ST_KERNEL, and running will
 * be zero
 *
 * If fwid != track then the attempt to boot the desired firmware image set
 * failed, and what is running is an alternative.
 *
 * If boot is interrupted and manually overridden, 'autoboot' is zeroed, and
 * the stage field is not used.
 */

/*
 * MS Nonresettable Register used to hold state.
 * Bits [10:0] of non-resettable register 0
 */
#define BSM_STATE_REG               MS_(MSR_CFG_NONRESETTABLE)
#define BSM_STATE_REG_LSB           0

/*
 * WDT to use for high level timeouts
 */
#define BSM_WDT                     3

/*
 * Autoboot Stages
 */
#define BSM_STG_BOOT0               0
#define BSM_STG_UBOOT               1
#define BSM_STG_KERNEL              2

/*
 * fwid and track ids
 */
#define BSM_FWID_MAINA              0
#define BSM_FWID_MAINB              1
#define BSM_FWID_GOLD               2
#define BSM_FWID_DIAG               3

/*
 * BSM fields [10:0]
 */
#define BSM_MASK                    0x7ff
#define BSM_WDT_LSB                 10
#define BSM_WDT_MASK                0x1
#define BSM_FWID_LSB                8
#define BSM_FWID_MASK               0x3
#define BSM_ATTEMPT_LSB             6
#define BSM_ATTEMPT_MASK            0x3
#define BSM_TRACK_LSB               4
#define BSM_TRACK_MASK              0x3
#define BSM_STAGE_LSB               2
#define BSM_STAGE_MASK              0x3
#define BSM_RUNNING_LSB             1
#define BSM_RUNNING_MASK            0x1
#define BSM_AUTOBOOT_LSB            0
#define BSM_AUTOBOOT_MASK           0x1

#define _BSM_GET(r, x) \
    (((r) >> BSM_##x##_LSB) & BSM_##x##_MASK)
#define _BSM_SET(v, x) \
    (((v) & BSM_##x##_MASK) << BSM_##x##_LSB)
#define _BSM_MODIFY(r, v, x) \
    (((r) & ~BSM_##x##_SET(BSM_##x##_MASK)) | BSM_##x##_SET(v))

#define BSM_WDT_GET(r)              _BSM_GET(r, WDT)
#define BSM_WDT_SET(v)              _BSM_SET(v, WDT)
#define BSM_WDT_MODIFY(r, v)        _BSM_MODIFY(r, v, WDT)
#define BSM_FWID_GET(r)             _BSM_GET(r, FWID)
#define BSM_FWID_SET(v)             _BSM_SET(v, FWID)
#define BSM_FWID_MODIFY(r, v)       _BSM_MODIFY(r, v, FWID)
#define BSM_ATTEMPT_GET(r)          _BSM_GET(r, ATTEMPT)
#define BSM_ATTEMPT_SET(v)          _BSM_SET(v, ATTEMPT)
#define BSM_ATTEMPT_MODIFY(r, v)    _BSM_MODIFY(r, v, ATTEMPT)
#define BSM_TRACK_GET(r)            _BSM_GET(r, TRACK)
#define BSM_TRACK_SET(v)            _BSM_SET(v, TRACK)
#define BSM_TRACK_MODIFY(r, v)      _BSM_MODIFY(r, v, TRACK)
#define BSM_STAGE_GET(r)            _BSM_GET(r, STAGE)
#define BSM_STAGE_SET(v)            _BSM_SET(v, STAGE)
#define BSM_STAGE_MODIFY(r, v)      _BSM_MODIFY(r, v, STAGE)
#define BSM_RUNNING_GET(r)          _BSM_GET(r, RUNNING)
#define BSM_RUNNING_SET(v)          _BSM_SET(v, RUNNING)
#define BSM_RUNNING_MODIFY(r, v)    _BSM_MODIFY(r, v, RUNNING)
#define BSM_AUTOBOOT_GET(r)         _BSM_GET(r, AUTOBOOT)
#define BSM_AUTOBOOT_SET(v)         _BSM_SET(v, AUTOBOOT)
#define BSM_AUTOBOOT_MODIFY(r, v)   _BSM_MODIFY(r, v, AUTOBOOT)

#ifndef __ASSEMBLY__

typedef struct {
    int wdt;
    int fwid;
    int attempt;
    int track;
    int stage;
    int running;
    int autoboot;
} bsm_t;

static inline void bsm_load(bsm_t *p)
{
    uint32_t r = bsm_readreg(BSM_STATE_REG);

    r >>= BSM_STATE_REG_LSB;

    p->wdt      = BSM_WDT_GET(r);
    p->fwid     = BSM_FWID_GET(r);
    p->attempt  = BSM_ATTEMPT_GET(r);
    p->track    = BSM_TRACK_GET(r);
    p->stage    = BSM_STAGE_GET(r);
    p->running  = BSM_RUNNING_GET(r);
    p->autoboot = BSM_AUTOBOOT_GET(r);
}

static inline void bsm_save(const bsm_t *p)
{
    uint32_t r, n;

    n = BSM_WDT_SET(p->wdt) |
        BSM_FWID_SET(p->fwid) |
        BSM_ATTEMPT_SET(p->attempt) |
        BSM_TRACK_SET(p->track) |
        BSM_STAGE_SET(p->stage) |
        BSM_RUNNING_SET(p->running) |
        BSM_AUTOBOOT_SET(p->autoboot);

    r = bsm_readreg(BSM_STATE_REG);
    r &= ~(BSM_MASK << BSM_STATE_REG_LSB);
    r |= n << BSM_STATE_REG_LSB;
    bsm_writereg(BSM_STATE_REG, r);
}

#endif

#endif
