//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <iostream>
#include <iomanip>
#include <gtest/gtest.h>
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"

namespace test {

void
dump_packet (std::vector<uint8_t> data) {
    int count = 0;
    std::cout << "Packet length : " << data.size() << "\n";
    for (std::vector<uint8_t>::const_iterator i = data.begin(); i != data.end();
         ++i) {
        if (count != 0 && count % 16 == 0)
            std::cout << "\n";
        // std::cout << *i << ' ';
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)*i
                  << " ";
        count++;
    }
    std::cout << "\n";
}

void
send_packet (const uint8_t *tx_pkt, uint32_t tx_pkt_len, uint32_t tx_port,
             const uint8_t *exp_rx_pkt, uint32_t exp_rx_pkt_len,
             uint32_t exp_rx_port)
{
#ifdef SIM
    std::vector<uint8_t> ipkt;
    std::vector<uint8_t> opkt;
    std::vector<uint8_t> epkt;
    uint32_t port = 0;
    uint32_t cos = 0;
    uint32_t i = 0;
    uint32_t tcscale = 1;
    int tcid = 0;
    int tcid_filter = 0;

    if (getenv("TCSCALE")) {
        tcscale = atoi(getenv("TCSCALE"));
    }
    if (getenv("TCID")) {
        tcid_filter = atoi(getenv("TCID"));
    }

    tcid++;
    if (tcid_filter != 0 && tcid != tcid_filter) {
        std::cout << "TCID not matching, skipping the send packet\n"
                  << std::endl;
        return;
    }

    ipkt.resize(tx_pkt_len);
    memcpy(ipkt.data(), tx_pkt, tx_pkt_len);
    epkt.resize(exp_rx_pkt_len);
    memcpy(epkt.data(), exp_rx_pkt, exp_rx_pkt_len);
    std::cout << "Testing Host to Switch" << std::endl;
    for (i = 0; i < tcscale; i++) {
        testcase_begin(tcid, i + 1);
        step_network_pkt(ipkt, tx_port);
        if (!getenv("SKIP_VERIFY")) {
            get_next_pkt(opkt, port, cos);
            dump_packet(opkt);
            EXPECT_TRUE(opkt == epkt);
            EXPECT_TRUE(port == exp_rx_port);
        }
        testcase_end(tcid, i + 1);
    }
#endif
}

}    // namespace test
