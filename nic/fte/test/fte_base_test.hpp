#pragma once

#include <gtest/gtest.h>
#include "nic/include/base.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/nic.pb.h"
#include "gen/proto/telemetry.pb.h"
#include "nic/hal/plugins/cfg/nw/nic.hpp"
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
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"

#define FTE_ID 0
#define PKTBUF_LEN  2000
using namespace std;

using device::DeviceResponseMsg;
using device::DeviceRequest;

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
    static hal_handle_t add_uplink(uint32_t port_num);
    static hal_handle_t add_enic(hal_handle_t l2segh, uint32_t useg_vlan, uint64_t mac, hal_handle_t uplink);
    static hal_handle_t add_endpoint(hal_handle_t l2segh, hal_handle_t intfh,
                                     uint32_t ip, uint64_t mac, uint16_t useg_vlan,
                                     bool enable_e2e=false, bool set_uplink=false, hal_handle_t uplink=HAL_HANDLE_INVALID);
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
            uint16_t                      proto;
            uint16_t                      dport_low;
            uint16_t                      dport_high;
            nwsec::ALGName                alg;
            uint32_t                      idle_timeout;
            bool                          has_alg_opts;
            hal::alg_opts                 alg_opt;
        } app;
        telemetry::MonitorAction mon_action;
        bool collect;
    };
	
    static uint32_t myrandom(uint32_t i) { return std::rand()%i;}


    static void timeit(const std::string &msg, int count, std::function<void()> fn)
    {
        cout << msg << " " << count << " " << std::flush;

        std::clock_t start = clock();
        fn();
        int ticks = clock()-start;

        cout << " (" << 1000.0*ticks/CLOCKS_PER_SEC << " ms) ";
        if (count) {
            cout << count*CLOCKS_PER_SEC/ticks << "/sec";
        }
        cout << "\n";
    }
        

    static hal_handle_t add_nwsec_policy(hal_handle_t vrfh, std::vector<v4_rule_t> &rules);

    static hal_handle_t add_flowmon_policy(hal_handle_t vrfh, std::vector<v4_rule_t> &rules);

    static hal_handle_t add_nat_pool(hal_handle_t vrfh,
                                     uint32_t v4_addr, uint8_t prefix_len);

    static hal_handle_t add_nat_mapping(hal_handle_t vrfh, uint32_t v4_addr,
                                        hal_handle_t poolh, uint32_t *mapped_ip);

    static hal_ret_t inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len, bool copied_pkt_arg);
    static hal_ret_t inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr,std::vector<uint8_t *> &pkts, size_t pkt_len,
                                bool copied_pkt_arg);

    static hal_ret_t inject_eth_pkt(const fte::lifqid_t &lifq,
                                    hal_handle_t src_ifh, hal_handle_t src_l2segh,
                                    Tins::EthernetII &eth, bool add_padding=false);
    static hal_ret_t inject_eth_pkt(const fte::lifqid_t &lifq,
                                    hal_handle_t src_ifh, hal_handle_t src_l2segh,
                                    std::vector<Tins::EthernetII> &pkts, bool add_padding=false);
    static hal_ret_t inject_ipv4_pkt(const fte::lifqid_t &lifq,
                                     hal_handle_t dep, hal_handle_t sep,
                                     Tins::PDU &l4pdu, bool add_padding=false);
    static void set_logging_disable(bool val) { ipc_logging_disable_ = val; }
 
    static void run_service(hal_handle_t ep_h, std::string cmd);

    static string prefix_cmd(hal_handle_t ep_h);

    static void process_e2e_packets(void);

    static void gen_rules(uint32_t num_rules, uint32_t num_tenants,
                         vector<v4_rule_t *> &rules);

    static void send_packets (hal_handle_t srceph, hal_handle_t dsteph, 
                         uint32_t srcip, uint32_t dstip);

protected:
    fte_base_test() {};

    virtual ~fte_base_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() { }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase(std::string c_file="hal.json", 
                sys::ForwardMode fwdmode=sys::FWD_MODE_MICROSEG, 
                sys::PolicyMode policymode=sys::POLICY_MODE_ENFORCE) {
        hal_ret_t                   ret;
        DeviceRequest               nic_req;
        DeviceResponseMsg           nic_rsp;

        hal_base_test::SetUpTestCase(false, c_file);
        sleep(1);

        hal::g_hal_state->set_fwd_mode(fwdmode);
        hal::g_hal_state->set_policy_mode(policymode);

        // Set device mode as Smart switch
        nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::device_create(&nic_req, &nic_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
 
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
             cmd = prefix_cmd + " ifconfig lo 127.0.0.1";
             f << cmd << "\n";
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
    static uint32_t vrf_id_, l2seg_id_, intf_id_, nwsec_id_, nh_id_, pool_id_, lif_id_;
    static uint64_t flowmon_rule_id_;
    static bool ipc_logging_disable_;
    static std::map<hal_handle_t, ep_info_t> eps;
    static std::vector<dev_handle_t> handles;
};

#define CHECK_COLLECT_ACTION(dep, sep, dst_port, src_port, msg) {                   \
        hal_ret_t ret;                                                         \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                         \
        tcp.flags(Tins::TCP::SYN);                                             \
        tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));                 \
        tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));                     \
        tcp.mss(1200);                                                         \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, tcp);             \
        EXPECT_EQ(ret, HAL_RET_OK)<< msg;                                      \
        EXPECT_NE(ctx_.session(), nullptr)<< msg;                              \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;                       \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;                       \
        EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.export_en, 1)<< msg;        \
        EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.export_id1, 2)<< msg;       \
        EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.export_id2, 0)<< msg;       \
        EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.export_id3, 0)<< msg;       \
        EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.export_id4, 0)<< msg;       \
    }

#define CHECK_MIRROR_ACTION(dep, sep, dst_port, src_port, msg) {                   \
        hal_ret_t ret;                                                         \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                         \
        tcp.flags(Tins::TCP::SYN);                                             \
        tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));                 \
        tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));                     \
        tcp.mss(1200);                                                         \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, tcp);             \
        EXPECT_EQ(ret, HAL_RET_OK)<< msg;                                      \
        EXPECT_NE(ctx_.session(), nullptr)<< msg;                              \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;                       \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;                       \
        EXPECT_EQ(ctx_.session()->iflow->config.ing_mirror_session, 2)<< msg;             \
    }

#define CHECK_ALLOW_TCP(dep, sep, dst_port, src_port, msg) {                   \
        hal_ret_t ret;                                                         \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                         \
        tcp.flags(Tins::TCP::SYN);                                             \
        tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));                 \
        tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));                     \
        tcp.mss(1200);                                                         \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, tcp);             \
        EXPECT_EQ(ret, HAL_RET_OK)<< msg;                                      \
        EXPECT_FALSE(ctx_.drop())<< msg;                                       \
        EXPECT_NE(ctx_.session(), nullptr)<< msg;                              \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;                       \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;                       \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;             \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;             \
    }

#define CHECK_DENY_TCP(dep, sep, dst_port, src_port, msg) {             \
        fte::fte_stats_t stats;                                         \
        hal_ret_t   ret;                                                \
        uint16_t    sfw_feature;                                        \
        stats = fte::fte_stats_get(FTE_ID);                             \
        sfw_feature = fte::feature_id("pensando.io/sfw:sfw");           \
        uint64_t sfw_drop = stats.feature_stats[sfw_feature].drop_pkts; \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                  \
        tcp.flags(Tins::TCP::SYN);                                      \
        tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));          \
        tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));              \
        tcp.mss(1200);                                                  \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep,tcp);       \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                              \
        EXPECT_TRUE(ctx_.drop()) << msg;                                \
        if (!ctx_.existing_session()) {                                 \
            stats = fte::fte_stats_get(FTE_ID);                         \
            EXPECT_EQ(stats.feature_stats[sfw_feature].drop_pkts, sfw_drop+1);\
        }                                                               \
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
        fte::fte_stats_t stats;                                                \
        hal_ret_t ret;                                                         \
        uint16_t    sfw_feature;                                               \
        stats = fte::fte_stats_get(FTE_ID);                                    \
        sfw_feature = fte::feature_id("pensando.io/sfw:sfw");                  \
        uint64_t sfw_drop = stats.feature_stats[sfw_feature].drop_pkts;        \
        Tins::UDP pdu = Tins::UDP(dst_port, src_port);                         \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);             \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                                     \
        EXPECT_TRUE(ctx_.drop()) << msg;                                       \
        if (!ctx_.existing_session()) {                                        \
            stats = fte::fte_stats_get(FTE_ID);                                \
            EXPECT_EQ(stats.feature_stats[sfw_feature].drop_pkts, sfw_drop+1); \
        }                                                                      \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                             \
        EXPECT_NE(ctx_.session()->iflow, nullptr) << msg;                      \
        EXPECT_NE(ctx_.session()->rflow, nullptr) << msg;                      \
        EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.drop) << msg;             \
        EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.drop) << msg;             \
    }


