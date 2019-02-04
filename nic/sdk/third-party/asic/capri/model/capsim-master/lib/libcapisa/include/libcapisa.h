
#ifndef _LIBCAPISA_H_
#define _LIBCAPISA_H_

/*
 * Initialize the ISA library.
 * Returns:
 *      0       success
 *      -1      error
 */
int libcapisa_init(void);

/*
 * Disassemble a Capri opcode.
 */
std::string libcap_dasm(uint64_t pc, uint64_t opcode);

#endif
