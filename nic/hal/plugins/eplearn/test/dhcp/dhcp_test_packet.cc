#include <arpa/inet.h>
#include <gtest/gtest.h>
// clang-format off
#include "nic/hal/plugins/eplearn/dhcp/dhcp_packet.hpp"
// clang-format on

#include <tins/tins.h>

using namespace Tins;
using namespace hal::network;
class dhcp_test : public ::testing::Test {
 protected:
  dhcp_test() {
  }

  virtual ~dhcp_test() {}

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
          dhcp_lib_init();
  }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}
};

TEST_F(dhcp_test, dhcp_init) {
    EthernetII eth = EthernetII() / IP() / UDP() / DHCP();

    // Retrieve a pointer to the stored TCP PDU
    DHCP *dhcp = eth.find_pdu<DHCP>();
    TCP *tcp = eth.find_pdu<TCP>();
    UDP *udp = eth.find_pdu<UDP>();

    ASSERT_TRUE(dhcp != NULL);
    ASSERT_TRUE(tcp == NULL);

    udp->sport(68);
    udp->dport(67);

    dhcp->type(DHCP::Flags::DISCOVER);
    ASSERT_TRUE(dhcp->type() == DHCP::Flags::DISCOVER);

    std::vector<uint8_t> buffer = eth.serialize();

    EthernetII eth_new(&buffer[0], buffer.size());

    udp = eth_new.find_pdu<UDP>();
    ASSERT_TRUE(udp != NULL);
    const RawPDU *raw = udp->find_pdu<RawPDU>();

    std::vector<uint8_t> dhcp_buffer = raw->payload();
    DHCP dhcp_new = DHCP(&dhcp_buffer[0], dhcp_buffer.size());
    ASSERT_TRUE(dhcp_new.type() == DHCP::Flags::DISCOVER);
}

EthernetII *get_dhcp_packet() {
    EthernetII *eth = new EthernetII();
    IP *ip = new IP();
    UDP *udp = new UDP();
    DHCP *dhcp = new DHCP();

    udp->inner_pdu(dhcp);
    ip->inner_pdu(udp);
    eth->inner_pdu(ip);

    return eth;
}

TEST_F(dhcp_test, dhcp_discover_parse_test) {
    EthernetII *eth = get_dhcp_packet();

    // Retrieve a pointer to the stored TCP PDU
    DHCP *dhcp = eth->find_pdu<DHCP>();

    dhcp->type(DHCP::Flags::DISCOVER);
    dhcp->xid(1234);
    dhcp->htype(1);
    dhcp->hlen(6);
    dhcp->secs(5);
    dhcp->ciaddr("1.1.1.1");
    dhcp->yiaddr("2.2.2.2");
    dhcp->siaddr("3.3.3.3");
    dhcp->giaddr("4.4.4.4");
    unsigned char chaddr[16] = "9999";
    HWAddress<6> hw_address(chaddr);
    dhcp->chaddr(hw_address);
    const char *sname = "SNAME";
    dhcp->sname((const uint8_t *)sname);
    const char *fname = "FNAME";
    dhcp->file((const uint8_t *)fname);

    /* Options */
    std::vector<uint8_t> client_identifier_data = {
        1, 1, 1, 1, 1, 1, 1,
    };
    // Create a DHCP::option
    DHCP::option clid_opt(DHCP::DHCP_CLIENT_IDENTIFIER,
                          client_identifier_data.begin(),
                          client_identifier_data.end());
    dhcp->add_option(clid_opt);

    struct sockaddr_in sa;
    inet_pton(AF_INET, "192.168.1.1", &(sa.sin_addr));
    std::vector<uint8_t> vec_addr;
    vec_addr.resize(sizeof(sa.sin_addr));

    std::memcpy(vec_addr.data(), &(sa.sin_addr), sizeof(sa.sin_addr));

    DHCP::option req_addr_opt(DHCP::DHCP_REQUESTED_ADDRESS, vec_addr.begin(),
                              vec_addr.end());
    dhcp->add_option(req_addr_opt);

    std::vector<uint8_t> param_request_data = {1, 3, 6, 42};

    DHCP::option param_req_list(DHCP::DHCP_PARAMETER_REQUEST_LIST,
                                param_request_data.begin(),
                                param_request_data.end());
    dhcp->add_option(param_req_list);

    std::vector<uint8_t> buffer = eth->serialize();

    struct packet *decoded_packet;
    struct dhcp_packet *pkt;

    hal_ret_t ret =
        parse_dhcp_packet(&buffer[0], buffer.size(), &decoded_packet);

    ASSERT_EQ(ret, HAL_RET_OK);
    pkt = decoded_packet->raw;
    ASSERT_NE(decoded_packet->packet_length, 0);
    ASSERT_EQ(pkt->op, DHCP::Flags::DISCOVER);
    ASSERT_EQ(pkt->xid, ntohl(1234));
    ASSERT_EQ(pkt->htype, 1);
    ASSERT_EQ(pkt->hops, 0);
    ASSERT_EQ(pkt->hlen, 6);
    ASSERT_EQ(pkt->secs, ntohs(5));
    struct in_addr ciaddr;
    inet_pton(AF_INET, "1.1.1.1", &(ciaddr.s_addr));
    ASSERT_EQ(pkt->ciaddr.s_addr, ciaddr.s_addr);
    struct in_addr yiaddr;
    inet_pton(AF_INET, "2.2.2.2", &(yiaddr.s_addr));
    ASSERT_EQ(pkt->yiaddr.s_addr, yiaddr.s_addr);
    struct in_addr siaddr;
    inet_pton(AF_INET, "3.3.3.3", &(siaddr.s_addr));
    ASSERT_EQ(pkt->siaddr.s_addr, siaddr.s_addr);
    struct in_addr giaddr;
    inet_pton(AF_INET, "4.4.4.4", &(giaddr.s_addr));
    ASSERT_EQ(pkt->giaddr.s_addr, giaddr.s_addr);

    ASSERT_EQ(memcmp(chaddr, pkt->chaddr, 16), 0);
    ASSERT_EQ(strcmp(sname, pkt->sname), 0);
    ASSERT_EQ(strcmp(fname, pkt->file), 0);
    ASSERT_EQ(pkt->options[4], 53);

    /* Parse options */
    ASSERT_TRUE(decoded_packet->options_valid);
    ASSERT_EQ(decoded_packet->options->universe_count, 1);
    ASSERT_TRUE(decoded_packet->options->universes != NULL);

    struct option_data data;
    ret = dhcp_lookup_option(decoded_packet, 53, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = dhcp_lookup_option(decoded_packet, 61, &data);
    ASSERT_EQ(ret, HAL_RET_OK);

    ASSERT_EQ(memcmp(data.data, &client_identifier_data[0],
                     client_identifier_data.size()),
              0);

    ret = dhcp_lookup_option(decoded_packet, 50, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(memcmp(data.data, &vec_addr[0], vec_addr.size()), 0);
    ret = dhcp_lookup_option(decoded_packet, 55, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(
        memcmp(data.data, &param_request_data[0], param_request_data.size()),
        0);
    free_dhcp_packet(&decoded_packet);
}

TEST_F(dhcp_test, dhcp_offer_parse_test) {
    EthernetII eth = EthernetII() / IP() / UDP() / DHCP();

    // Retrieve a pointer to the stored TCP PDU
    DHCP *dhcp = eth.find_pdu<DHCP>();

    dhcp->type(DHCP::Flags::OFFER);
    dhcp->xid(1234);
    dhcp->htype(1);
    dhcp->hlen(6);
    dhcp->secs(5);
    dhcp->ciaddr("1.1.1.1");
    dhcp->yiaddr("2.2.2.2");
    dhcp->siaddr("3.3.3.3");
    dhcp->giaddr("4.4.4.4");
    unsigned char chaddr[16] = "9999";
    HWAddress<6> hw_address(chaddr);
    dhcp->chaddr(hw_address);
    const char *sname = "SNAME";
    dhcp->sname((const uint8_t *)sname);
    const char *fname = "FNAME";
    dhcp->file((const uint8_t *)fname);

    /* Options */
    std::vector<uint8_t> client_identifier_data = {
        1, 1, 1, 1, 1, 1, 1,
    };
    // Create a DHCP::option

    struct sockaddr_in sa;
    inet_pton(AF_INET, "255.255.255.0", &(sa.sin_addr));
    std::vector<uint8_t> subnet_mask;
    subnet_mask.resize(sizeof(sa.sin_addr));

    std::memcpy(subnet_mask.data(), &(sa.sin_addr), sizeof(sa.sin_addr));

    DHCP::option subnet_mask_opt(DHCP::SUBNET_MASK, subnet_mask.begin(),
                                 subnet_mask.end());
    dhcp->add_option(subnet_mask_opt);

    std::vector<uint8_t> renewal_time = {1, 3, 6, 42};

    DHCP::option renewal_time_data(DHCP::DHCP_RENEWAL_TIME,
                                   renewal_time.begin(), renewal_time.end());

    dhcp->add_option(renewal_time_data);

    inet_pton(AF_INET, "192.168.0.1", &(sa.sin_addr));
    std::vector<uint8_t> server_identifier;
    server_identifier.resize(sizeof(sa.sin_addr));

    std::memcpy(server_identifier.data(), &(sa.sin_addr), sizeof(sa.sin_addr));

    DHCP::option server_identifer_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                                      server_identifier.begin(),
                                      server_identifier.end());
    dhcp->add_option(server_identifer_opt);

    std::vector<uint8_t> lease_time = {0, 0, 8, 5};

    DHCP::option lease_time_data(DHCP::DHCP_LEASE_TIME, lease_time.begin(),
                                 lease_time.end());

    dhcp->add_option(lease_time_data);

    std::vector<uint8_t> buffer = eth.serialize();

    struct packet *decoded_packet;

    hal_ret_t ret =
        parse_dhcp_packet(&buffer[0], buffer.size(), &decoded_packet);

    ASSERT_EQ(ret, HAL_RET_OK);
    /* Parse options */
    ASSERT_TRUE(decoded_packet->options_valid);
    ASSERT_EQ(decoded_packet->options->universe_count, 1);
    ASSERT_TRUE(decoded_packet->options->universes != NULL);

    struct option_data data;
    ret = dhcp_lookup_option(decoded_packet, 53, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ret = dhcp_lookup_option(decoded_packet, 1, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(memcmp(data.data, &subnet_mask[0], subnet_mask.size()), 0);
    ret = dhcp_lookup_option(decoded_packet, 58, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(memcmp(data.data, &renewal_time[0], renewal_time.size()), 0);
    ret = dhcp_lookup_option(decoded_packet, 51, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(memcmp(data.data, &lease_time[0], lease_time.size()), 0);
    ret = dhcp_lookup_option(decoded_packet, 54, &data);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(
        memcmp(data.data, &server_identifier[0], server_identifier.size()), 0);

    free_dhcp_packet(&decoded_packet);
}

