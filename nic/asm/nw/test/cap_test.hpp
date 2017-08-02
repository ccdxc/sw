#ifndef __CAP_TEST_HPP__
#define __CAP_TEST_HPP__

#include <string>
#include <base.h>
#include "capmpubase.h"
#include "capmpusim.h"
#include "libasmsym.h"
#include "libmpuobj.h"
#include "gmp.h"

enum {
    NO_SHOW_WE,
    SHOW_WE
};

#define DUMPOPT_WEONLY      0x1         // only show fields written    
#define DUMPOPT_BITFIELDS   0x2         // show field bitrange

class CapTest {

private:
	uint32_t num_cases_;
	
	// Used for every test case
	uint8_t ctlfile_dvec[CAPISA_DVEC_SZ];
	uint8_t ctlfile_dvec_we[CAPISA_DVEC_SZ];
	uint8_t ctlfile_kvec[CAPISA_KVEC_SZ];
	uint8_t ctlfile_kvec_we[CAPISA_KVEC_SZ];
	uint8_t ctlfile_phv[CAPISA_PHV_SZ];
	uint8_t ctlfile_phv_we[CAPISA_PHV_SZ];
	uint64_t ctlfile_gpr[8];
	uint8_t ctlfile_cflags;
	uint64_t ctlfile_table_addr;
	CapMPUAddrSpace ctlfile_table_addrspace;

	// CapMPUSim sim;
	MpuSymbolTable symtab;
	uint64_t loadaddr;
	uint64_t runaddr;
	std::string runlabel;
	int modelicache;
	int dumpopts;
	int dumphex;
	int dumpsyms;
	int maxinstr;

	int is_we_dirty(const uint8_t *we, int sz);
	void we_combine(uint8_t *sum, const uint8_t *src, int sz);

	void show_input_state(CapMPUSim& sim);
	void show_output_state(CapMPUSim& sim);

public:


	CapTest(std::string test_name, uint32_t num_cases = 1);
	~CapTest();

	
    void run();	
	static uint64_t htonll(uint64_t n);
	static void dump_kvec(const uint8_t *buf, const uint8_t *we, int opts);
	static void dump_dvec(const uint8_t *buf, const uint8_t *we, int opts);
	static void dump_phv(const uint8_t *buf, const uint8_t *we, int opts);
	static void show_buf(char c, const uint8_t *buf, const u_int8_t *we, int len, int show_we);
	static void do_dump_kdpvec(enum kdpid id, const uint8_t *buf,
                        const uint8_t *we, int sz, int opts);
	static void dump_vec(char c, std::vector<struct sym_abs_info>& tab,
		          const uint8_t *buf, const uint8_t *we, int sz, int opts);
	static void vec_read(mpz_t res, const uint8_t *buf, int sz,
                  const struct sym_info *inf);
	static int vec_write(uint8_t *buf, uint8_t *we, int sz,
                  const struct sym_info *inf, mpz_t val);

};

#endif // __CAP_TEST_HPP__
