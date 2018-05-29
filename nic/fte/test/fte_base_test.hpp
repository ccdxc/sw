#pragma once

#include <gtest/gtest.h>
#include "nic/include/base.h"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/include/fte_ctx.hpp"
#include <tins/tins.h>
#include <map>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/plugins/sfw/sfw_pkt_utils.hpp"
#include "nic/e2etests/lib/packet.hpp"

#define FTE_ID 0
#define PKTBUF_LEN  2000
using namespace std;

typedef struct dev_handle_ {
    int                  sock;
    int                  fd;
    int                  other_hdl;
    hal_handle_t         ep;
    hal_handle_t         ep_pair;
} dev_handle_t;

typedef struct ep_info_ {
    uint32_t   ip;
    uint64_t   mac;
    uint32_t   vlan;
} ep_info_t;

class fte_base_test : public hal_base_test {
public:
    static hal_handle_t add_vrf();
    static hal_handle_t add_network(hal_handle_t vrfh, uint32_t v4_addr,
                                    uint8_t prefix_len, uint64_t rmac);
    static hal_handle_t add_l2segment(hal_handle_t nwh, uint16_t vlan_id);
    static hal_handle_t add_uplink(uint8_t port_num);
    static hal_handle_t add_endpoint(hal_handle_t l2segh, hal_handle_t intfh,
                                     uint32_t ip, uint64_t mac, uint16_t useg_vlan,
                                     bool enable_e2e=false);
    static hal_handle_t add_route(hal_handle_t vrfh,
                                  uint32_t v4_addr, uint8_t prefix_len,
                                  hal_handle_t eph);
    struct v4_rule_t {
        nwsec::SecurityAction action;
        struct {
            uint32_t addr;
            uint8_t plen;
        } from, to;
        struct {
            uint16_t proto;
            uint16_t dport_low;
            uint16_t dport_high;
            nwsec::ALGName alg;
        } app;
    };

    static hal_handle_t add_nwsec_policy(hal_handle_t vrfh, std::vector<v4_rule_t> &rules);

    static hal_handle_t add_nat_pool(hal_handle_t vrfh,
                                     uint32_t v4_addr, uint8_t prefix_len);

    static hal_handle_t add_nat_mapping(hal_handle_t vrfh, uint32_t v4_addr,
                                        hal_handle_t poolh, uint32_t *mapped_ip);

    static hal_ret_t inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len);
    static hal_ret_t inject_eth_pkt(const fte::lifqid_t &lifq,
                                    hal_handle_t src_ifh, hal_handle_t src_l2segh,
                                    Tins::EthernetII &eth);
    static hal_ret_t inject_ipv4_pkt(const fte::lifqid_t &lifq,
                                     hal_handle_t dep, hal_handle_t sep, Tins::PDU &l4pdu);
    static void set_logging_disable(bool val) { ipc_logging_disable_ = val; }
 
    static void run_service(hal_handle_t ep_h, std::string cmd);

    static void process_e2e_packets(void);

protected:
    fte_base_test() {}

    virtual ~fte_base_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() { }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        ipc_logging_disable_ = false;
    }

    static void hntap_create_tap_device (const char *dev, dev_handle_t *handle)
    {
        struct      ifreq ifr;
        int         fd, err, sock;
        const char *tapdev = "/dev/net/tun";

        if ((fd = open(tapdev, O_RDWR)) < 0 ) {
            abort();
            return;
        }

        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);

        /* create the device */
        if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
            close(fd);
            perror("2\n");
            return;
        }

        sock = socket(AF_INET,SOCK_DGRAM,0);
        if (sock < 0) {
            close(fd);
            perror("3\n");
            return;
        }

        memset(&ifr, 0, sizeof(ifr));
        ifr.ifr_flags = IFF_UP | IFF_RUNNING | IFF_PROMISC;
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
        /* Set the device UP */
        if ((err = ioctl(sock, SIOCSIFFLAGS, (void *) &ifr)) < 0 ) {
            close(sock);
            close(fd);
            abort();
            return;
        }

        handle->sock = sock;
        handle->fd = fd;
    }  
    
    static void SetUpE2ETestCase() {
        string ep = "EP";
        string cmd, prefix_cmd;
        ofstream f;

        f.open("cmd.sh");
        std::map<hal_handle_t, ep_info_t>::iterator it;
        uint32_t i = 0;
        for (it=eps.begin(); it!=eps.end(); it++) {
             ep_info_t  ep = it->second;
             dev_handle_t handle;
             string EP = "EP" + to_string(i+1);
             hntap_create_tap_device(EP.c_str(), &handle);
             handle.other_hdl = (i+1)%eps.size();
             handle.ep = it->first;

             // Set up EP Pair. Pair each EP to its next neighbor
             // Last one points to first
             if (i) handles[i-1].ep_pair = it->first;
             handles.push_back(handle);
             if ((i+1) == eps.size()) handles[i].ep_pair = (eps.begin())->first;

             cmd = "ip netns add " +  EP;
             f << cmd << "\n";
             cmd = "ip link set dev " + EP + " netns " + EP;
             f << cmd << "\n";
             prefix_cmd = "ip netns exec " + EP;
             cmd = prefix_cmd + " ifconfig " + EP + " up";
             f << cmd << "\n";
             std::stringstream mac, ip;
             mac << std::hex << ep.mac;
             ip << std::hex << ep.ip;
             if (ep.vlan) {
                 string vlan = to_string(ep.vlan);
                 string vlan_intf = EP + "." + vlan;
                 cmd = prefix_cmd + " ip link add link " + EP + " name " + vlan_intf + " type vlan id " + vlan;
                 f << cmd << "\n";
                 cmd = prefix_cmd + " ifconfig " + vlan_intf + " up";
                 f << cmd << "\n";
                 cmd = prefix_cmd + " ifconfig " + vlan_intf + " hw ether " + mac.str();
                 f << cmd << "\n";
                 cmd = prefix_cmd + " ifconfig " + vlan_intf + " 0x" + ip.str();
                 f << cmd << "\n";
             } else {
                 cmd = cmd = prefix_cmd + " ip link set " + EP + " name " + EP;
                 f << cmd << "\n";
                 cmd = prefix_cmd + " ifconfig " + EP + " hw ether " + mac.str();
                 f << cmd << "\n";
                 cmd = prefix_cmd + " ifconfig " + EP + " 0x" + ip.str();
                 f << cmd << "\n";                          
             }
             i++;
        }
        for (uint32_t i=0; i<handles.size(); i++) {
            ep_info_t  ep = eps[handles[i].ep_pair];
            string EP = "EP" + to_string(i+1);
            std::stringstream mac, ip;
            mac << std::hex << ep.mac;
            ip << std::hex << ep.ip;
            prefix_cmd = "ip netns exec ";
            cmd = prefix_cmd + EP + " arp -s 0x" + ip.str() + " " + mac.str();
            f << cmd << "\n";
        }
        f.close();
        std::system("chmod +x cmd.sh && ./cmd.sh && rm cmd.sh");  
    }

    static void CleanUpE2ETestCase(void) {
        string ep = "EP";
        string cmd;
        ofstream f;

        f.open("cmd.sh");
        std::map<hal_handle_t, ep_info_t>::iterator it;
        for (uint32_t i=0; i<eps.size(); i++) {
            string EP = "EP" + to_string(i+1);
            cmd = "ip netns del " + EP;
            f << cmd << "\n";
        }
        f.close();
        std::system("chmod +x cmd.sh && ./cmd.sh && rm cmd.sh");
    } 

    static fte::ctx_t ctx_;

private:
    static uint32_t vrf_id_, l2seg_id_, intf_id_, nwsec_id_, nh_id_, pool_id_;
    static bool ipc_logging_disable_;
    static std::map<hal_handle_t, ep_info_t> eps;
    static std::vector<dev_handle_t> handles;
};

#define CHECK_ALLOW_TCP(dep, sep, dst_port, src_port, msg) {                   \
        hal_ret_t ret;                                                         \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                         \
        tcp.flags(Tins::TCP::SYN);                                             \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, tcp);             \
        EXPECT_EQ(ret, HAL_RET_OK)<< msg;                                      \
        EXPECT_FALSE(ctx_.drop())<< msg;                                       \
        EXPECT_NE(ctx_.session(), nullptr)<< msg;                              \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;                       \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;                       \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;             \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;             \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sipv4(),            \
                  ctx_.key().sip.v4_addr);                                     \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dipv4(),            \
                  ctx_.key().dip.v4_addr);                                     \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sport()), \
                  ctx_.key().sport);                                           \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dport()), \
                  ctx_.key().dport);                                           \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->ipprot(),            \
                  ctx_.key().proto);                                           \
    }

#define CHECK_DENY_TCP(dep, sep, dst_port, src_port, msg) {             \
        hal_ret_t ret;                                                  \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                  \
        tcp.flags(Tins::TCP::SYN);                                      \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep,tcp);       \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                              \
        EXPECT_TRUE(ctx_.drop()) << msg;                                \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                      \
        EXPECT_NE(ctx_.session()->iflow, nullptr) << msg;               \
        EXPECT_NE(ctx_.session()->rflow, nullptr) << msg;               \
        EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.drop) << msg;      \
        EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.drop) << msg;      \
    }

#define CHECK_ALLOW_UDP(dep, sep, dst_port, src_port, msg)          \
    {                                                               \
        hal_ret_t ret;                                              \
        Tins::UDP pdu = Tins::UDP(dst_port, src_port);              \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);  \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                          \
        EXPECT_FALSE(ctx_.drop()) << msg;                           \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                  \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;            \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;            \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;  \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;  \
    }

#define CHECK_DENY_UDP(dep, sep, dst_port, src_port, msg)                      \
    {                                                                          \
        hal_ret_t ret;                                                         \
        Tins::UDP pdu = Tins::UDP(dst_port, src_port);                         \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);             \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                                     \
        EXPECT_TRUE(ctx_.drop()) << msg;                                       \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                             \
        EXPECT_NE(ctx_.session()->iflow, nullptr) << msg;                      \
        EXPECT_NE(ctx_.session()->rflow, nullptr) << msg;                      \
        EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.drop) << msg;             \
        EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.drop) << msg;             \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->sipv4(),            \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).sip.v4_addr);         \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->dipv4(),            \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).dip.v4_addr);         \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->sport()), \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).sport);                \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->dport()), \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).dport);                \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->ipprot(),            \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).proto);                \
    }
