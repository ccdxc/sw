/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <cinttypes>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/pal/include/pal.h"
#include "platform/evutils/include/evutils.h"
#include "platform/misc/include/misc.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pcieport/include/pcieport.h"
#include "lib/catalog/catalog.hpp"
#include "platform/fru/fru.hpp"

#include "pciemgrd_impl.hpp"

static pciemgrenv_t pciemgrenv;

pciemgrenv_t *
pciemgrenv_get(void)
{
    return &pciemgrenv;
}

static void
reboot_the_system(void)
{
    int r = system("/platform/tools/pcie_hostdn.sh");
    if (r) pciesys_logerror("failed to reboot %d\n", r);
}

static void
port_evhandler(pcieport_event_t *ev, void *arg)
{
    pciemgrenv_t *pme = pciemgrenv_get();

    switch (ev->type) {
    case PCIEPORT_EVENT_LINKUP: {
        pciesys_loginfo("port%d: linkup gen%dx%d\n",
                        ev->port, ev->linkup.gen, ev->linkup.width);
        break;
    }
    case PCIEPORT_EVENT_LINKDN: {
        pciesys_loginfo("port%d: linkdn\n", ev->port);
        break;
    }
    case PCIEPORT_EVENT_HOSTUP: {
        pciesys_loginfo("port%d: hostup gen%dx%d\n",
                        ev->port, ev->hostup.gen, ev->hostup.width);
        pciehw_event_hostup(ev->port, ev->hostup.gen, ev->hostup.width);
#ifdef IRIS
        update_pcie_port_status(ev->port, PCIEMGR_UP,
                                ev->hostup.gen, ev->hostup.width);
#endif
        break;
    }
    case PCIEPORT_EVENT_HOSTDN: {
        if (pme->reboot_on_hostdn) {
            reboot_the_system();
        }
        pciesys_loginfo("port%d: hostdn\n", ev->port);
        pciehw_event_hostdn(ev->port);
#ifdef IRIS
        update_pcie_port_status(ev->port, PCIEMGR_DOWN);
#endif
        break;
    }
    case PCIEPORT_EVENT_BUSCHG: {
        const u_int8_t secbus = ev->buschg.secbus;
        pciesys_loginfo("port%d: buschg 0x%02x\n", ev->port, secbus);
        pciehw_event_buschg(ev->port, secbus);
        break;
    }
    case PCIEPORT_EVENT_FAULT: {
        pciesys_logerror("port%d: fault %s\n", ev->port, ev->fault.reason);
#ifdef IRIS
        update_pcie_port_status(ev->port,
                                PCIEMGR_FAULT, 0, 0, ev->fault.reason);
#endif
        break;
    }
    default:
        /* Some event we don't need to handle. */
        pciesys_loginfo("port%d: event %d ignored\n", ev->port, ev->type);
        break;
    }
}

int
open_hostports(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciemgr_params_t *p = &pme->params;
    int r, port;

    /*
     * Open and configure all the ports we are going to manage.
     */
    r = 0;
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            if ((r = pcieport_open(port, p->initmode)) < 0) {
                pciesys_logerror("pcieport_open %d failed: %d\n", port, r);
                goto error_out;
            }
            if ((r = pcieport_hostconfig(port, p)) < 0) {
                pciesys_logerror("pcieport_hostconfig %d failed\n", port);
                goto close_error_out;
            }
#ifdef IRIS
            /* initialize delphi port status object */
            update_pcie_port_status(port, PCIEMGR_DOWN);
#endif
        }
    }

    if ((r = pcieport_register_event_handler(port_evhandler, NULL)) < 0) {
        goto close_error_out;
    }
    return r;

 close_error_out:
    pcieport_close(port);
 error_out:
    for (port--; port >= 0; port--) {
        pcieport_close(port);
    }
    return r;
}

void
close_hostports(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    int port;

    /* close all the ports we opened */
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pcieport_close(port);
        }
    }
}

static void
intr_poll(void *arg)
{
    pciemgrenv_t *pme = pciemgrenv_get();

    // poll for port events
    if (pme->poll_port) {
        for (int port = 0; port < PCIEPORT_NPORTS; port++) {
            if (pme->enabled_ports & (1 << port)) {
                pcieport_poll(port);
            }
        }
    }

    // poll for device events
    if (pme->poll_dev) {
        pciehdev_poll();
    }
}

static void
notify_intr(void *arg)
{
    pciehw_notify_intr(0);
}

static void
indirect_intr(void *arg)
{
    pciehw_indirect_intr(0);
}

int
intr_init(void)
{
    pciemgrenv_t *pme = pciemgrenv_get();
    int need_poll_timer = 0;

    if (pme->poll_port) {
        need_poll_timer = 1;
    } else {
        // XXX need to get port intr connected
        need_poll_timer = 1;
    }
    if (pme->poll_dev) {
        pciehw_notify_poll_init();
        pciehw_indirect_poll_init();
        need_poll_timer = 1;
    } else {
#ifdef __aarch64__
        uint64_t msgaddr;
        uint32_t msgdata;

        static struct pal_int notifyint;
        pal_int_open_msi(&notifyint, &msgaddr, &msgdata);
        evutil_add_pal_int(EV_DEFAULT_ &notifyint, notify_intr, NULL);
        pciesys_logdebug("intr_init notify "
                         "msgaddr 0x%" PRIx64 " msgdata 0x%" PRIx32 "\n",
                         msgaddr, msgdata);
        pciehw_notify_intr_init(0, msgaddr, msgdata);

        static struct pal_int indirectint;
        pal_int_open_msi(&indirectint, &msgaddr, &msgdata);
        evutil_add_pal_int(EV_DEFAULT_ &indirectint, indirect_intr, NULL);
        pciesys_logdebug("intr_init indirect "
                         "msgaddr 0x%" PRIx64 " msgdata 0x%" PRIx32 "\n",
                         msgaddr, msgdata);
        pciehw_indirect_intr_init(0, msgaddr, msgdata);
#else
        if (0) notify_intr(NULL);
        if (0) indirect_intr(NULL);
#endif
    }
    if (need_poll_timer) {
        static evutil_timer timer = {0};
        evutil_timer_start(EV_DEFAULT_ &timer, intr_poll, NULL, 0.01, 0.01);
    }
    return 0;
}

static u_int64_t
getenv_override_ull(const char *label, const char *name, const u_int64_t def)
{
    const char *env = getenv(name);
    if (env) {
        u_int64_t val = strtoull(env, NULL, 0);
        pciesys_loginfo("%s: $%s override %" PRIu64 " (0x%" PRIx64 ")\n",
                        label, name, val, val);
        return val;
    }
    return def;
}

static u_int64_t
pciemgrd_param_ull(const char *name, const u_int64_t def)
{
    return getenv_override_ull("pciemgrd", name, def);
}

#ifdef VPD_HAS_SYSINFO
#include "platform/capri/csrint/csr_init.hpp"

typedef struct pciemgrd_sysinfo_s {
    unsigned int cclk_mhz;
    unsigned int vin_mv;
    unsigned int vdd_mv;
    unsigned int vcpu_mv;
    unsigned int pin_uw;
} pciemgrd_sysinfo_t;

static int
pciemgrd_get_sysinfo(pciemgrd_sysinfo_t *sysinfo)
{
    memset(sysinfo, 0, sizeof(*sysinfo));

    // XXX fix cap_top_s... crash
    // sdk::platform::capri::csr_init();
    //sysinfo->cclk = cap_top_sbus_get_core_freq(0, 0);

    system_voltage_t voltages = { 0 };
    read_voltages(&voltages);

    sysinfo->vin_mv = voltages.vin;
    sysinfo->vdd_mv = voltages.vout1;
    sysinfo->vcpu_mv = voltages.vout2;

    system_power_t powers = { 0 };
    read_powers(&powers);

    sysinfo->pin_uw = powers.pin;

    return 0;
}
#endif

static void
pciemgrd_sbus_lock(void)
{
    pal_lock_ret_t r = pal_wr_lock(SBUSLOCK);
    assert(r == LCK_SUCCESS);
}

static void
pciemgrd_sbus_unlock(void)
{
    pal_lock_ret_t r = pal_wr_unlock(SBUSLOCK);
    assert(r == LCK_SUCCESS);
}

static void
pciemgrd_sbus_locker_init(pciemgrenv_t *pme)
{
    static pciesys_sbus_locker_t sbus_locker = {
        .sbus_lock   = pciemgrd_sbus_lock,
        .sbus_unlock = pciemgrd_sbus_unlock,
    };

    pciesys_set_sbus_locker(&sbus_locker);
}

static void
pciemgrd_vpd_params(pciemgrenv_t *pme)
{
    pciemgr_params_t *params = &pme->params;

#define S(field, str) \
    strncpy0(params->field, str, sizeof(params->field))

#ifdef __aarch64__
    std::string s;

#define SFRU(KEY, field) \
    do { \
        if (sdk::platform::readFruKey(KEY, s) == 0) {   \
            S(field, s.c_str());\
        } \
    } while (0)

    SFRU(PRODUCTNAME_KEY, id);
    SFRU(PARTNUM_KEY, partnum);
    SFRU(SERIALNUMBER_KEY, serialnum);
    SFRU(MANUFACTURERDATE_KEY, mfgdate);
    SFRU(ENGCHANGELEVEL_KEY, engdate); // XXX HPE wants a date
#undef SFRU

    boost::property_tree::ptree ver;
    boost::property_tree::read_json("/nic/etc/VERSION.json", ver);
    S(fwvers, ver.get<std::string>("sw.version").c_str());

#ifdef VPD_HAS_SYSINFO
    char misc[PCIEMGR_STRSZ], *mp = misc;
    int misc_left = PCIEMGR_STRSZ;
#define MISC_APPEND(fmt, val) \
    do { \
        int n; \
        n = snprintf(mp, misc_left, "%s" fmt, mp == misc ? "" : ",", val); \
        if (n >= 0 && n < misc_left) { \
            misc_left -= n;            \
            mp += n;                   \
        }                              \
    } while (0)

    //
    // XXX
    // Many of these vary over time, but we're taking a snapshot
    // and encoding into VPD.  Maybe these are not right for VPD.
    // (Or maybe we periodically update VPD with current values?)
    // Leaving this out for now until we get more clarity on what
    // should be included in the "misc" section.
    // XXX
    //

    pciemgrd_sysinfo_t sysinfo;
    pciemgrd_get_sysinfo(&sysinfo);

    if (sysinfo.cclk_mhz) {
        MISC_APPEND("cclk=%uMHz", sysinfo.cclk_mhz);
    }
    if (sysinfo.vin_mv) {
        MISC_APPEND("vin=%.0fV", sysinfo.vin_mv / 1000.0);
    }
    if (sysinfo.vdd_mv) {
        MISC_APPEND("vdd=%umV", sysinfo.vdd_mv);
    }
    if (sysinfo.vcpu_mv) {
        MISC_APPEND("vcpu=%umV", sysinfo.vcpu_mv);
    }
    if (sysinfo.pin_uw) {
        MISC_APPEND("pin=%.0fW", sysinfo.pin_uw / 1000000.0);
    }
#undef MISC_APPEND

    S(misc, misc);
#endif

#else

    S(id, "SIM NAPLES");
    S(partnum, "SIM-123456-78");
    S(serialnum, "SIM12345678");
    S(mfgdate, "SIM-2013-08-29");
    S(engdate, "SIM-2013-08-29");
    S(pcarev, "SIM-PCA-X1");
    S(misc, "SIM 1000GHz,0.0001pW");
    S(fwvers, "SIM 1.2.3.4");
#endif
#undef S
}

/*
 * These overrides are separated here and called by the main loop
 * *after* the logger initialization any setting overrides can be
 * included in the log.
 */
void
pciemgrd_params(pciemgrenv_t *pme)
{
    pme->reboot_on_hostdn = pciemgrd_param_ull("PCIE_REBOOT_ON_HOSTDN",
                                               pme->reboot_on_hostdn);
    pme->enabled_ports = pciemgrd_param_ull("PCIE_ENABLED_PORTS",
                                            pme->enabled_ports);
    pme->poll_port = pciemgrd_param_ull("PCIE_POLL_PORT", pme->poll_port);
    pme->poll_dev = pciemgrd_param_ull("PCIE_POLL_DEV", pme->poll_dev);
    pme->cpumask = pciemgrd_param_ull("PCIE_CPUMASK", pme->cpumask);
    pme->fifopri = pciemgrd_param_ull("PCIE_FIFOPRI", pme->fifopri);
    pme->mlockall = pciemgrd_param_ull("PCIE_MLOCKALL", pme->mlockall);

    pciemgr_params_t *params = &pme->params;
    params->single_pnd = pciemgrd_param_ull("PCIE_SINGLE_PND",
                                            params->single_pnd);

    pciemgrd_vpd_params(pme);

    if (pme->params.restart) {
        if (upgrade_state_restore() < 0) {
            pciesys_logerror("restore failed, forcing full init\n");
            pme->params.initmode = FORCE_INIT;
        }
        /*
         * If this upgrade restart was due to a rollback
         * we have consumed the state we saved for the rollback.
         * Mark this complete (removes any saved rollback state).
         */
        if (upgrade_rollback_in_progress()) {
            upgrade_rollback_complete();
        }
    }
}

void
pciemgrd_catalog_defaults(pciemgrenv_t *pme)
{
    /* on asic single port, on haps 2 ports enabled */
    pme->enabled_ports = pal_is_asic() ? 0x1 : 0x5;

    pciemgr_params_t *params = &pme->params;
    params->vendorid = PCI_VENDOR_ID_PENSANDO;
    params->subvendorid = params->vendorid;
    params->subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100_4GB;

#ifdef __aarch64__
    sdk::lib::catalog *catalog = sdk::lib::catalog::factory();
    if (catalog != NULL) {
        pme->enabled_ports  = catalog->pcie_hostport_mask();
        params->cap_gen     = catalog->pcie_gen();
        params->cap_width   = catalog->pcie_width();
        params->subdeviceid = catalog->pcie_subdeviceid();
        sdk::lib::catalog::destroy(catalog);
    } else {
        pciesys_logerror("no catalog\n");
    }
#endif /* __aarch64__ */
}

/*
 * Lock our memory.  We are not swapping on our system, but the system
 * might reclaim text pages if low on memory, slowing our reaction to
 * pcie events.
 */
static void
pciemgrd_mem_init(pciemgrenv_t *pme)
{
#ifdef __aarch64__
    if (pme->mlockall) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
            pciesys_logerror("mlockall failed: %s\n", strerror(errno));
        }

        /* Fault in our stack here by writing to each stack page. */
        volatile char big_stack[64 * 1024];
        const int pagesize = getpagesize();
        for (unsigned int i = 0; i < sizeof(big_stack); i += pagesize) {
            big_stack[i] = 0;
        }
    }
#endif
}

/*
 * Set the cpu affinity mask to restrict to cpu 0.
 * Set sched policy to SCHED_FIFO to select "real-time"
 * scheduling so pciemgr can respond to pcie transactions
 * in "pcie transaction timeout" time frames, typically 50ms.
 */
static void
pciemgrd_sched_init(pciemgrenv_t *pme)
{
#ifdef __aarch64__
    if (pme->cpumask) {
        unsigned int cpumask = pme->cpumask;
        cpu_set_t cpuset;

        CPU_ZERO(&cpuset);
        for (int cpu = 0; cpumask; cpu++) {
            int cpubit = 1 << cpu;
            if (cpumask & cpubit) {
                CPU_SET(cpu, &cpuset);
                cpumask &= ~cpubit;
            }
        }
        if (sched_setaffinity(0, sizeof(cpuset), &cpuset) < 0) {
            pciesys_logerror("sched_setaffinity 0x%x: %s\n",
                             pme->cpumask, strerror(errno));
        }
    }

    if (pme->fifopri) {
        struct sched_param param;
        const int policy = SCHED_FIFO;

        memset(&param, 0, sizeof(param));
        param.sched_priority = pme->fifopri;
        if (sched_setscheduler(0, policy, &param) < 0) {
            pciesys_logerror("sched_setscheduler FIFO pri %d: %s\n",
                             pme->fifopri, strerror(errno));
        }
    }
#endif
}

// called after libev blocks, before any io,timer handlers
static void
evmon_begin(void *arg)
{
    u_int64_t *tstamp = (u_int64_t *)arg;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *tstamp = tv.tv_sec * 1000000 + tv.tv_usec;
}

// called before libev blocks, after any io,timer handlers
static void
evmon_end(void *arg)
{
    u_int64_t *tstamp = (u_int64_t *)arg;
    u_int64_t tnow, tdelta;
    struct timeval tv;

    // first time called is before evmon_begin
    if (*tstamp == 0) return;

    gettimeofday(&tv, NULL);
    tnow = tv.tv_sec * 1000000 + tv.tv_usec;

    tdelta = tnow - *tstamp;
    if (tdelta > 2000) {
        pciesys_logwarn("evmon %ldus delay\n", tdelta);
    }
}

//
// Monitor libev callbacks runtime.  If any callbacks take too long
// to run we'll log a msg about it.  We don't want slow callbacks
// doing blocking calls that might keep pciemgr from servicing
// pcie transactions and delivering completions before the
// pcie transaction timeout (typically 10ms-50ms).
//
static void
pciemgrd_evmon_init(pciemgrenv_t *pme)
{
    static evutil_check evmoncheck;
    static evutil_prepare evmonprep;
    static u_int64_t evmon_tstamp;

    evutil_add_check(EV_DEFAULT_ &evmoncheck, evmon_begin, &evmon_tstamp);
    evutil_add_prepare(EV_DEFAULT_ &evmonprep, evmon_end, &evmon_tstamp);
}

void
pciemgrd_sys_init(pciemgrenv_t *pme)
{
    pciemgrd_mem_init(pme);
    pciemgrd_sched_init(pme);
    pciemgrd_evmon_init(pme);
    pciemgrd_sbus_locker_init(pme);
}

void
pciemgrd_start()
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciemgr_params_t *params = &pme->params;
    int r;

    pme->interactive = 1;
    pme->reboot_on_hostdn = pal_is_asic() ? 1 : 0;
    pme->poll_port = 1;
    pme->poll_dev = 0;
    pme->fifopri = 50;

    params->strict_crs = 1;

    /*
     * For aarch64 we can inherit a system in which we get restarted on
     * a running system (mostly for testing).
     * For x86_64 we want to FORCE_INIT to reinitialize hw/shmem on startup.
     *
     * On "real" ARM systems the upstream port bridge
     * is in hw and our first virtual device is bus 0 at 00:00.0.
     *
     * For simulation we want the virtual upstream port bridge
     * at 00:00.0 so our first virtual device is bus 1 at 01:00.0.
     */
#ifdef __aarch64__
    params->initmode = INHERIT_OK;
    params->first_bus = 0;
#else
    params->initmode = FORCE_INIT;
    params->first_bus = 1;
    params->fake_bios_scan = 1;         /* simulate bios scan to set bdf's */
#endif

    if (upgrade_in_progress()) {
        params->restart = 1;
        params->initmode = INHERIT_OK;
    }

    /* Get the catalog defaults. */
    pciemgrd_catalog_defaults(pme);

    r = server_loop(pme);

    exit(r < 0 ? 1 : 0);
}
