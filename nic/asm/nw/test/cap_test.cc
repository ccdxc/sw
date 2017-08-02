#include "cap_test.hpp"
#include "capisa.h"
#include <arpa/inet.h>
#include "dtls.h"
// #include "egress.h"

static u_int8_t dvec_wb[CAPISA_DVEC_SZ];
static uint64_t table_addr;
static CapMPUAddrSpace table_addrspace;
extern char OPCODES_start[];
extern char OPCODES_end[];


// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
CapTest::CapTest(std::string test_name, uint32_t num_cases)
{
    num_cases_ = num_cases;

    dumpopts = 1;
    dumphex = 0;
}

// ----------------------------------------------------------------------------
// Destructor 
// ----------------------------------------------------------------------------
CapTest::~CapTest()
{
}

#define RSMAP_GEN(a, b) { a, b },
static std::map<CapMPURunState, std::string> rs_map = {
    CAPMPU_RUNSTATE_GENERATOR(RSMAP_GEN)
};

#define FAULTMAP_GEN(a, b) { a, b },
static std::map<CapMPUFaultCode, std::string> fault_map = {
    CAPMPU_FAULT_GENERATOR(FAULTMAP_GEN)
};

void
CapTest::run()
{
    // struct tx_stats_k k;

    memset(ctlfile_kvec, 0, CAPISA_KVEC_SZ);
    memset(ctlfile_dvec, 0, CAPISA_DVEC_SZ); 
    memset(ctlfile_phv, 0, CAPISA_PHV_SZ);
    memset(ctlfile_gpr, 0, 8 * 8);
    ctlfile_cflags = 0;
    ctlfile_table_addr = 0;
    maxinstr = 0;
    modelicache = 0;
    
    ctlfile_kvec[3] = 5;
    ctlfile_kvec[4] = 0xdc;
    ctlfile_kvec[6] = 0xaa;

    ctlfile_dvec[1] = 0x64;
    ctlfile_dvec[3] = 0x65;
    ctlfile_dvec[5] = 0x66;
    ctlfile_dvec[7] = 0xff;
    ctlfile_dvec[9] = 0x3f;
    ctlfile_dvec[10] = 0xf1;
    ctlfile_dvec[11] = 0x3;
    ctlfile_dvec[12] = 0xff;
    ctlfile_dvec[13] = 0x20;
    ctlfile_dvec[14] = 0xff;
    ctlfile_dvec[15] = 0xe0;

    ctlfile_gpr[5] = 0xdead0000;

    loadaddr = runaddr = 0x40000;

    if (isa_loadmem(OPCODES_start, OPCODES_end - OPCODES_start) < 0) {
        printf("Failed to load memory\n");
        return;
    }

    std::string path = "../../../obj/tx_stats.bin";
    if (load_program(path, loadaddr, 0, symtab) < 0) {
        printf("Failed to load the program\n");
        return;
    }

    printf("Finished loading opcodes and binary\n");
    
    CapMPUSim sim;
    printf("Input kvec: \n");
    for (int i = 0; i < CAPISA_KVEC_SZ; i++) {
        if (!(i % 32)) printf("\n");
        printf("%2x ", ctlfile_kvec[i]);
    }
    printf("\n");
    printf("Input dvec: \n");
    for (int i = 0; i < CAPISA_DVEC_SZ; i++) {
        if (!(i % 32)) printf("\n");
        printf("%2x ", ctlfile_dvec[i]);
    }
    printf("\n");
    printf("Input phv: \n");
    for (int i = 0; i < CAPISA_PHV_SZ; i++) {
        if (!(i % 32)) printf("\n");
        printf("%2x ", ctlfile_phv[i]);
    }
    printf("\n");

    sim.set_initial_kvec(ctlfile_kvec, CAPISA_KVEC_SZ);
    sim.set_initial_dvec(ctlfile_dvec, CAPISA_DVEC_SZ);
    sim.set_initial_phv(ctlfile_phv, CAPISA_PHV_SZ);

    printf("Input gpr: \n");
    for (int i = 1; i < 8; i++) {
        printf("%lx ", ctlfile_gpr[i]);
        sim.set_initial_gpr(i, ctlfile_gpr[i]);
    }
    printf("\n");
    printf("Input cflags: \n");
    printf("%x ", ctlfile_cflags);
    printf("\n");
    for (int i = 1; i < 8; i++) {
        sim.set_initial_cflag(i, (ctlfile_cflags >> i) & 0x1);
    }
    printf("Run Label: %s\n", runlabel.c_str());

    printf("Run addr:\n");
    printf("%lx ", runaddr);
    printf("\n");
    sim.set_initial_pc(runaddr);

    memcpy(dvec_wb, ctlfile_dvec, CAPISA_DVEC_SZ);
    table_addr = ctlfile_table_addr;
    table_addrspace = ctlfile_table_addrspace;
    printf("table_addr: %lx\n", table_addr);
    printf("table_addrspace: %d\n", table_addrspace);
    printf("modelicache: %d\n", modelicache);
    printf("dumpopts: %d\n", dumpopts);
    printf("dumphex: %d\n", dumphex);

    sim.reset();
    progmem_reset(modelicache);

    puts("------------------------------------------------------------------");
    puts("INPUT STATE");
    puts("------------------------------------------------------------------");
    show_input_state(sim);

    puts("------------------------------------------------------------------");
    puts("PROGRAM EXECUTION");
    puts("------------------------------------------------------------------");
    CapMPURunState rs = sim.run(maxinstr);
    if (rs == CAPMPU_RUNSTATE_STEPPED) {
        printf("# Instruction count exceeded\n");
    } else {
        printf("# %s\n", rs_map[rs].c_str());
        if (rs == CAPMPU_RUNSTATE_FAULTED) {
            printf("# %s\n", fault_map[sim.faultcode()].c_str());
        }
        int count = sim.read_instr_count();
        printf("# Executed %d instructions; %.1fMpps @ 800MHz\n",
                count, 800.0 / (double)count);
        if (modelicache) {
            puts("------------------------------------------------------------------");
            puts("CACHE STATE");
            puts("------------------------------------------------------------------");
            progmem_dumpstats();
        }
    }

    puts("------------------------------------------------------------------");
    puts("OUTPUT STATE");
    puts("------------------------------------------------------------------");
    show_output_state(sim);
}




int
CapTest::is_we_dirty(const uint8_t *we, int sz)
{
    for (int i = 0; i < sz; i++) {
        if (we[i]) {
            return 1;
        }
    }
    return 0;
}

void 
CapTest::dump_kvec(const uint8_t *buf, const uint8_t *we,int opts)
{
    do_dump_kdpvec(SYM_K, buf, we, CAPISA_KVEC_SZ, opts);
}

void
CapTest::dump_dvec(const uint8_t *buf, const uint8_t *we, int opts)
{
    do_dump_kdpvec(SYM_D, buf, we, CAPISA_DVEC_SZ, opts);
}

void
CapTest::dump_phv(const uint8_t *buf, const uint8_t *we, int opts)
{
    do_dump_kdpvec(SYM_P, buf, we, CAPISA_PHV_SZ, opts);
}

void
CapTest::do_dump_kdpvec(enum kdpid id, const uint8_t *buf,
        const uint8_t *we, int sz, int opts)
{
    printf("%s: Trying to dump: %d\n", __FUNCTION__, id);
    static const char c[] = "kdp";
    struct sym_info inf;
    sym *sym = sym_lookup_kdpid(id, &inf);
    if (sym == NULL) {
        printf("Not able to find symbol info\n");
        return;
    }
    std::vector<struct sym_abs_info> tab;
    sym_gen_abs_info(sym, tab);
    dump_vec(c[id], tab, buf, we, sz, opts);
}

uint64_t
CapTest::htonll(uint64_t n)
{
    return ((uint64_t)htonl(n & 0xffffffff) << 32) | htonl(n >> 32);
}

void
CapTest::vec_read(mpz_t res, const uint8_t *buf, int sz,
        const struct sym_info *inf)
{
    mpz_set_ui(res, 0);
    for (int b = 0; b < inf->len; b++) {
        int byt = sz - ((inf->offs + b) >> 3) - 1;
        int bii = (inf->offs + b) & 0x7;
        int bv = (buf[byt] >> bii) & 0x1;
        if (bv) {
            mpz_setbit(res, b);
        }
    }
    if (inf->attr & SYM_ATTR_LITTLE) {
        uint64_t ival = mpz_get_ui(res);
        switch (inf->len) {
        case 16: ival = htons(ival); break;
        case 32: ival = htonl(ival); break;
        case 64: ival = htonll(ival); break;
        default: throw std::runtime_error("vec_read: invalid swap size");
        }
        mpz_set_ui(res, ival);
    }
    if (inf->attr & SYM_ATTR_SIGNED) {
        if (mpz_tstbit(res, inf->len - 1)) {
            mpz_t adj;

            mpz_init_set_ui(adj, 1);
            mpz_mul_2exp(adj, adj, inf->len);
            mpz_sub(res, res, adj);
            mpz_clear(adj);
        }
    }
}

int
CapTest::vec_write(uint8_t *buf, uint8_t *we, int sz,
        const struct sym_info *inf, mpz_t val)
{
    mpz_t min_z, max_z, range;

    mpz_inits(min_z, max_z, range, NULL);

    mpz_set_ui(range, 1);
    mpz_mul_2exp(range, range, inf->len);

    if (inf->attr & SYM_ATTR_SIGNED) {
        mpz_set_ui(max_z, 1);
        mpz_mul_2exp(max_z, max_z, inf->len - 1);
        mpz_sub(min_z, max_z, range);
        mpz_sub_ui(max_z, max_z, 1);
    } else {
        mpz_set_ui(min_z, 0);
        mpz_sub_ui(max_z, range, 1);
    }
    if (mpz_cmp(val, min_z) < 0 || mpz_cmp(val, max_z) > 0) {
        return -1;
    }
    mpz_clears(min_z, max_z, range, NULL);

    uint64_t ival;
    if (inf->attr & SYM_ATTR_LITTLE) {
        if (inf->attr & SYM_ATTR_SIGNED) {
            ival = mpz_get_si(val);
        } else {
            ival = mpz_get_ui(val);
        }
        switch (inf->len) {
        case 16: ival = htons(ival); break;
        case 32: ival = htonl(ival); break;
        case 64: ival = htonll(ival); break;
        default:
            // fprintf(stderr, "lineno: %d\n", ctl_lineno);
            fprintf(stderr, "line ....\n");
            throw std::runtime_error("vec_write: invalid swap size");
        }
        mpz_set_ui(val, ival);
    }
    for (int b = 0; b < inf->len; b++) {
        int sbv = mpz_tstbit(val, b);
        int di = sz - ((inf->offs + b) >> 3) - 1;
        int db = (inf->offs + b) & 0x7;
        buf[di] &= ~(1 << db);
        buf[di] |= sbv << db;
        we[di] |= (1 << db);
    }
    return 0;
}

void
CapTest::dump_vec(char c, std::vector<struct sym_abs_info>& tab,
        const uint8_t *buf, const uint8_t *we, int sz, int opts)
{
    mpz_t val, vwe, ztmp;
    int want_bits = ((opts & DUMPOPT_BITFIELDS) != 0);

    printf("%c = {\n", c);
    mpz_inits(val, vwe, ztmp, NULL);
    for (auto si : tab) {
        struct sym_info weinf = si.info;
        weinf.attr = 0; /* no monkey business on write-enable */
        vec_read(vwe, we, sz, &weinf);
        if ((opts & DUMPOPT_WEONLY) && mpz_sgn(vwe) == 0) {
            continue;
        }
        vec_read(val, buf, sz, &si.info);
        printf("    %s = ", si.name.c_str());
        switch (SYM_SYNTAX_FROM_ATTR(si.info.attr)) {
        case SYM_SYNTAX_DEC:
            gmp_printf("%Zd;", val);
            break;

        case SYM_SYNTAX_HEX:
            gmp_printf("%#Zx;", val);
            break;

        case SYM_SYNTAX_BIN: {
            mpz_t nbv;
            mpz_init_set(nbv, val);
            if (mpz_sgn(nbv) < 0) {
                mpz_abs(nbv, nbv);
                printf("-0b");
            } else {
                printf("0b");
            }
            char *sbuf = (char *)malloc(mpz_sizeinbase(nbv, 2) + 2);
            printf("%s;", mpz_get_str(sbuf, 2, nbv));
            mpz_clear(nbv);
            free(sbuf);
            break;
        }
        case SYM_SYNTAX_IPV4: {
            char sbuf[20];
            struct in_addr addr;
            addr.s_addr = htonl(mpz_get_ui(val));
            printf("%s;", inet_ntop(AF_INET, &addr, sbuf, sizeof (sbuf)));
            break;
        }
        case SYM_SYNTAX_IPV6: {
            char sbuf[80];
            void *abuf;
            struct in6_addr addr = { 0 };
            size_t count;
            abuf = mpz_export(NULL, &count, 1, 1, 1, 0, val);
            if (count > sizeof (addr.s6_addr)) {
                throw std::runtime_error("dump_vec: IPV6 value too big");
            }
            memcpy((char *)addr.s6_addr + sizeof (addr.s6_addr) - count,
                        abuf, count);
            free(abuf);
            printf("%s;", inet_ntop(AF_INET6, &addr, sbuf, sizeof (sbuf)));
            break;
        }
        case SYM_SYNTAX_MAC: {
            uint64_t mac = mpz_get_ui(val);
            for (int i = 40; i >= 0; i -= 8) {
                printf("%s%02lx", (i < 40) ? ":" : "", (mac >> i) & 0xff);
            }
            putchar(';');
            break;
        }
        default:
            throw std::runtime_error("dump_vec: invalid syn");
            break;
        }

        /*
         * Check the write-enable mask for this field.  All 1s means the
         * entire field was written.
         *      not all ones if ((vwe & (vwe + 1)) != 0)
         */
        mpz_add_ui(ztmp, vwe, 1);
        mpz_and(ztmp, ztmp, vwe);
        int want_mask = (mpz_sgn(ztmp) != 0);
        int want_trailer = want_mask || want_bits;
        if (want_trailer) {
            printf(" //");
            if (want_bits) {
                printf(" [%d:%d]",
                        si.info.offs + si.info.len - 1, si.info.offs);
            }
            if (want_mask) {
                gmp_printf(" & %#Zx", vwe);
            }
        }
        putchar('\n');
    }
    printf("};\n");
    mpz_clears(val, vwe, ztmp, NULL);
}

void
CapTest::show_buf(char c, const uint8_t *buf, const u_int8_t *we, int len, int show_we)
{
    int w = 8 * len - 1;
    for (int i = 0; i < len; i += 32, buf += 32) {
        int emit = 1;
        if (we != NULL) {
            emit = 0;
            for (int j = 0; j < 32; j++) {
                emit |= we[j];
            }
        }
        if (emit) {
            int hi = w - i * 8;
            int lo = hi - 255;
            printf("%c[%4d:%4d]:", c, hi, lo);
            for (int j = 0; j < 32; j++) {
                if (we == NULL || we[j]) {
                    printf("%02x", buf[j]);
                } else {
                    printf("--");
                }
            }
            putchar('\n');
            if (show_we == SHOW_WE && we != NULL) {
                printf("             ");
                for (int j = 0; j < 32; j++) {
                    if (we[j]) {
                        printf("%02x", we[j]);
                    } else {
                        printf("--");
                    }
                }
                putchar('\n');
            }
        }
        if (we) {
            we += 32;
        }
    }
}
void
CapTest::show_input_state(CapMPUSim& sim)
{
    if (is_we_dirty(ctlfile_kvec_we, CAPISA_KVEC_SZ)) {
        dump_kvec(ctlfile_kvec, ctlfile_kvec_we, dumpopts);
        if (dumphex) {
            show_buf('k', ctlfile_kvec, ctlfile_kvec_we,
                    sizeof (ctlfile_kvec), NO_SHOW_WE);
        }
    }

    if (is_we_dirty(ctlfile_dvec_we, CAPISA_DVEC_SZ)) {
        dump_dvec(ctlfile_dvec, ctlfile_dvec_we, dumpopts);
        if (dumphex) {
            show_buf('d', ctlfile_dvec, ctlfile_dvec_we,
                    sizeof (ctlfile_dvec), NO_SHOW_WE);
        }
    }

    if (is_we_dirty(ctlfile_phv_we, CAPISA_PHV_SZ)) {
        dump_phv(ctlfile_phv, ctlfile_phv_we, dumpopts);
        if (dumphex) {
            if (dumphex) {
                show_buf('p', ctlfile_phv,
                        ctlfile_phv_we, sizeof (ctlfile_phv), SHOW_WE);
            }
        }
    }
    puts("GPRs:");
    for (int i = 0; i < 8; i++) {
        printf("r%d = 0x%016lx\n", i, sim.read_gpr(i));
    }
    printf("CFLAGS:\n");
    for (int i = 0; i < 8; i++) {
        printf("c%d = %d\n", i, sim.read_cflag(i));
    }
    printf("pc = 0x%016lx\n", sim.read_pc());
}

void
CapTest::we_combine(uint8_t *sum, const uint8_t *src, int sz)
{
    for (int i = 0; i < sz; i++) {
        sum[i] |= src[i];
    }
}
void
CapTest::show_output_state(CapMPUSim& sim)
{
    uint8_t dvec[CAPISA_DVEC_SZ];
    uint8_t dvec_we[CAPISA_DVEC_SZ];
    uint8_t phv[CAPISA_PHV_SZ];
    uint8_t phv_we[CAPISA_PHV_SZ];

    sim.read_dvec(dvec, sizeof (dvec));
    sim.read_dvec_we(dvec_we, sizeof (dvec_we));
    if (is_we_dirty(dvec_we, sizeof (dvec_we))) {
        printf("Writeback d[]:\n");
        dump_dvec(dvec, dvec_we, dumpopts);
        if (dumphex) {
            show_buf('d', dvec, dvec_we, sizeof (dvec), NO_SHOW_WE);
        }
        printf("Combined d[]:\n");
        we_combine(dvec_we, ctlfile_dvec_we, sizeof (dvec_we));
        dump_dvec(dvec, dvec_we, dumpopts);
    }

    sim.read_phv(phv, sizeof (phv));
    sim.read_phv_we(phv_we, sizeof (phv_we));
    if (is_we_dirty(phv_we, sizeof (phv_we))) {
        printf("Output p[]:\n");
        dump_phv(phv, phv_we, dumpopts);
        if (dumphex) {
            show_buf('p', phv, phv_we, sizeof (phv), SHOW_WE);
        }
        printf("Combined p[]:\n");
        we_combine(phv_we, ctlfile_phv_we, sizeof (phv_we));
        dump_phv(phv, phv_we, dumpopts);
    }

    for (int i = 0; i < 8; i++) {
        printf("r%d = 0x%016lx\n", i, sim.read_gpr(i));
    }
    printf("CFLAGS:\n");
    for (int i = 0; i < 8; i++) {
        printf("c%d = %d\n", i, sim.read_cflag(i));
    }
    printf("pc = 0x%016lx\n", sim.read_pc());
}

void
sys_mpu_phv_write(const uint8_t *p, const uint8_t *we, int flush, void *arg)
{
    printf("# PHV write%s\n", flush ? " and flush" : "");
    CapTest::show_buf('p', p, we, CAPISA_PHV_SZ, SHOW_WE);
    CapTest::dump_phv(p, we, 1 /* dumpopts */);
}

#define ADDRSPACE_GEN(a, b) { a, b },
static std::map<CapMPUAddrSpace, std::string> addrspace_map = {
    CAPMPU_ADDRSPACE_GENERATOR(ADDRSPACE_GEN)
};
void
sys_mpu_tbl_write(const uint8_t *d, const uint8_t *we, void *arg)
{
    printf("# table writeback (%s[0x%lx])\n",
            addrspace_map[table_addrspace].c_str(), table_addr);
    CapTest::show_buf('d', d, we, CAPISA_DVEC_SZ, NO_SHOW_WE);
    CapTest::dump_dvec(d, we, 1 /*dumpopts*/);
}

void
sys_mpu_mem_write(uint64_t addr, const uint8_t *buf, int len, void *arg)
{
    printf("# memwr addr=0x%lx, len=%d, buf=[", addr, len);
    for (int i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("]\n");
}

void
sys_mpu_tbl_setaddr(uint64_t addr, CapMPUAddrSpace addrspace, void *arg)
{
    printf("# tbl_setaddr addr=0x%lx, addrspace=%s\n",
            addr, addrspace_map[addrspace].c_str());
    table_addr = addr;
    table_addrspace = addrspace;
}

void
errf(const char *fmt, ...)
{
#if 0
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s:%d: ", ctl_curfile, ctl_lineno);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
#endif
}
