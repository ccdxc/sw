

#include <iostream>

#include "lib_driver.hpp"

int main(int argc, char *argv[])
{
	if (argc != 4) {
		std::cout << "Usage: " << argv[0] << " lif qtype qid" << std::endl;
		return (-1);
	}

	uint64_t lif = strtoul(argv[1], NULL, 0);
	uint32_t qtype = strtoul(argv[2], NULL, 0);
	uint32_t qid = strtoul(argv[3], NULL, 0);

	uint64_t qstate_addr = get_qstate_addr(lif, qtype, qid);
	//std::cout << "0x" << std::hex << qstate_addr << std::endl;
	std::cout << qstate_addr << std::endl;
	return (0);
}
