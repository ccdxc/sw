//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Utilities to interface with Linux OS
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_linux_util.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <cstring>
#include <bits/sockaddr.h>
#include <asm/types.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <ctime>
#include <thread>

namespace pds_ms {

// Utility function to get MAC address for interface from Linux
static bool
get_linux_intf_mac_addr (const std::string& if_name, mac_addr_t& if_mac)
{
    struct ifaddrs *ifaddr = NULL;
    struct ifaddrs *ifa = NULL;
    bool ret = false;

    if (getifaddrs(&ifaddr) == -1) {
        PDS_TRACE_ERR ("Failed to get MAC address for %s", if_name.c_str());
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        PDS_TRACE_VERBOSE("Looping Linux interfaces - current %s", ifa->ifa_name);

        if (if_name == ifa->ifa_name) {
            SDK_ASSERT(ifa->ifa_addr);
            SDK_ASSERT(ifa->ifa_addr->sa_family == AF_PACKET);

            struct sockaddr_ll *sock_addr = (struct sockaddr_ll*)ifa->ifa_addr;
            SDK_ASSERT(sock_addr->sll_halen == ETH_ADDR_LEN);
            memcpy (if_mac, sock_addr->sll_addr, ETH_ADDR_LEN);
            PDS_TRACE_DEBUG("Found %s - get MAC %s", ifa->ifa_name,
                             macaddr2str(if_mac));
            ret = true;
            goto exit;
        }
    }
exit:
    freeifaddrs(ifaddr);
    return ret;
}

bool
get_linux_intf_params (const char* ifname,
                       uint32_t*   lnx_ifindex,
                       mac_addr_t& mac)
{
    *lnx_ifindex = if_nametoindex(ifname);
    if (*lnx_ifindex == 0) return false;
    return get_linux_intf_mac_addr(ifname, mac);
}


struct ipaddr_req_nlmsg_t {
// Allocate NLMsg space for 2 RT attributes and an extra 16 byte padding
#define NETLINK_SEND_BUF_LEN \
     (NLMSG_ALIGN(NLMSG_LENGTH(sizeof(struct ifaddrmsg))) + \
      (RTA_LENGTH(sizeof(in6_addr)))*2 + 16)
    char buf_[NETLINK_SEND_BUF_LEN];
};

static ipaddr_req_nlmsg_t
make_ipaddr_req_nlmsg (uint32_t pid, uint32_t lnx_ifindex, const in_ipx_addr_t& ip,
                       uint32_t prefix_len, uint64_t seq, bool del)
{
    /* Netlink IOV structure reference
       msg_iov=[{iov_base={{len=48, type=RTM_NEWADDR,
                            flags=NLM_F_REQUEST|NLM_F_ACK|NLM_F_EXCL|NLM_F_CREATE,
                            seq=<>, pid=0},
                           {ifa_family=AF_INET, ifa_prefixlen=24, ifa_flags=0,
                            ifa_scope=RT_SCOPE_UNIVERSE,
                            ifa_index=if_nametoindex("<>")},
                           [{{nla_len=8, nla_type=IFA_LOCAL}, 35.1.1.1},
                            {{nla_len=8, nla_type=IFA_BROADCAST}, 35.1.1.255},
                            {{nla_len=8, nla_type=IFA_ADDRESS}, 35.1.1.1}]},
                            iov_len=48}]
    */
    ipaddr_req_nlmsg_t req = {0};
    auto nlhdr = (nlmsghdr*) req.buf_;

    nlhdr->nlmsg_len = NLMSG_ALIGN(NLMSG_LENGTH(sizeof(struct ifaddrmsg)));
    PDS_TRACE_VERBOSE("NLMsg starting at %p len %d", nlhdr, nlhdr->nlmsg_len);
    nlhdr->nlmsg_pid = pid;
    nlhdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
    nlhdr->nlmsg_seq = seq;
    if (del) {
        nlhdr->nlmsg_type = RTM_DELADDR;
    } else {
        nlhdr->nlmsg_type = RTM_NEWADDR;
    }

    auto ifaddr = (ifaddrmsg*) NLMSG_DATA(nlhdr);
    PDS_TRACE_VERBOSE("Ifaddr  starting at %p", ifaddr);
    ifaddr->ifa_family = AF_INET;
    ifaddr->ifa_prefixlen = prefix_len;
    ifaddr->ifa_index = lnx_ifindex;
    ifaddr->ifa_flags = 0;
    ifaddr->ifa_scope = 0;

    auto attr = (rtattr*) (((char*)req.buf_) + nlhdr->nlmsg_len);
    attr->rta_type = IFA_LOCAL;
    int ip_len;
    if (ip.af == IP_AF_IPV4) {
        nlhdr->nlmsg_len += RTA_LENGTH(sizeof(in_addr));
        attr->rta_len = RTA_LENGTH(sizeof(in_addr));
        ip_len = sizeof(in_addr);
    } else {
        nlhdr->nlmsg_len += RTA_LENGTH(sizeof(in6_addr));
        attr->rta_len = RTA_LENGTH(sizeof(in6_addr));
        ip_len = sizeof(in6_addr);
    }
    PDS_TRACE_VERBOSE("Rtattr starting at %p len %d total len %d",
                      attr, attr->rta_len, nlhdr->nlmsg_len);
    memcpy(RTA_DATA(attr), &ip.addr, ip_len);

    if (prefix_len == 31) {
        PDS_TRACE_DEBUG("Override interface Broadcast address in Linux");
        // Special Case - Override the subnet Broadcast IP, set it same as our IP
        // Reason - /31 subnet has just 2 IP addresses and is P2P.
        // By default, Linux allocates the highest IP in the subnet as the
        // broadcast IP which causes the default route pointing to the
        // peer IP to fail in Linux.
        attr = (rtattr*) (((char*)req.buf_) + nlhdr->nlmsg_len);
        attr->rta_type = IFA_BROADCAST;
        if (ip.af == IP_AF_IPV4) {
            nlhdr->nlmsg_len += RTA_LENGTH(sizeof(in_addr));
            attr->rta_len = RTA_LENGTH(sizeof(in_addr));
            ip_len = sizeof(in_addr);
        } else {
            nlhdr->nlmsg_len += RTA_LENGTH(sizeof(in6_addr));
            attr->rta_len = RTA_LENGTH(sizeof(in6_addr));
            ip_len = sizeof(in6_addr);
        }
        PDS_TRACE_VERBOSE("Second Rtattr starting at %p len %d total len %d",
                          attr, attr->rta_len, nlhdr->nlmsg_len);
        memcpy(RTA_DATA(attr), &ip.addr, ip_len);
    }
    return req;
}

static void
send_nlmsg (int fd, nlmsghdr* nlh)
{
    struct iovec iov;
    iov.iov_base = (void*) nlh;
    iov.iov_len = nlh->nlmsg_len;

    // Default dest is Kernel
    sockaddr_nl     dest = {0};
    dest.nl_family =  AF_NETLINK;

    struct msghdr   msg = {0};
    msg.msg_name = (void*) &dest;
    msg.msg_namelen = sizeof(dest);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    auto ret = sendmsg (fd, &msg, 0);
    if (ret == -1) {
        throw Error(std::string("socket send failed: ")
                    .append(std::to_string(errno)));
    }
    PDS_TRACE_DEBUG("Successfully sent Netlink msg %ld", ret);
}

// Automatically close fd before returning from an exception or at the end
class fd_guard_t {
public:
    fd_guard_t () {};
    fd_guard_t (int fd) : fd_(fd) {};
    fd_guard_t (fd_guard_t&& fdg) noexcept {
         fd_ = fdg.fd_; fdg.fd_ = -1;
    }
    ~fd_guard_t(void) {
        if (fd_ == -1) return;
        close(fd_);
    }
    // No copying
    fd_guard_t (const fd_guard_t& fdg) = delete;
    fd_guard_t& operator=(const fd_guard_t& fdg) = delete;

    operator bool(void) {return fd_ != -1;}
    int get(void) {return fd_;}
    void reset(int new_fd) {
        if (fd_ != -1) {close (fd_);}
        fd_ = new_fd;
    }
private :
    int fd_ = -1;
};

static fd_guard_t
create_and_bind_nl_socket (void)
{
    fd_guard_t fdg(socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE));
    if (!fdg) {
        throw Error(std::string("socket create failed: ")
                    .append(std::to_string(errno)));
    }
    sockaddr_nl  src = {0};
    src.nl_family =  AF_NETLINK;

    if (bind(fdg.get(), (struct sockaddr*)&src, sizeof(src)) == -1) {
        throw Error(std::string("socket bind failed: ")
                    .append(std::to_string(errno)));
    }
    // Set timeout to ensure recv thread exits in case of any errors
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fdg.get(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    return fdg;
}


void
netlink_rcv_main (fd_guard_t fdg, uint32_t pid, uint64_t seq)
{
    static char netlink_rcv_buf_[1024];
    int fd = fdg.get();

    while (1) {
        auto rtn = recv(fd, netlink_rcv_buf_, sizeof(netlink_rcv_buf_), 0);

        if ((rtn < 0)) {
            if (errno == EAGAIN) {
                // Exit thread on timeout to avoid permanently waiting in case
                // the netlink request never went through
                PDS_TRACE_INFO("Netlink Recv Timeout");
                return;
            }
            // Some socket error - try again
            PDS_TRACE_ERR("Netlink Recv socket error %s %d, try again",
                          strerror(errno), errno);
            continue;
        }
        auto nlhdr = (struct nlmsghdr *) netlink_rcv_buf_;
        for (;NLMSG_OK(nlhdr, rtn); nlhdr = NLMSG_NEXT(nlhdr, rtn)) {
            if (nlhdr->nlmsg_type != NLMSG_ERROR) {
                PDS_TRACE_VERBOSE("Received a non-ack Netlink message");
                continue;
            }
            PDS_TRACE_INFO("Netlink Recv response len %ld", rtn);
            auto nle = (struct nlmsgerr *) NLMSG_DATA(nlhdr);
            nlhdr = &(nle->msg);
            if (!((nlhdr->nlmsg_seq == seq) && (nlhdr->nlmsg_pid == pid))) {
                PDS_TRACE_VERBOSE("Netlink Recv unknown msg pid %u seq %u exp pid %u seq %lu",
                                   nlhdr->nlmsg_pid, nlhdr->nlmsg_seq, pid, seq);
                continue;
            }
            if (nle->error == 0) {
                PDS_TRACE_DEBUG("Netlink Recv Status %s %d", strerror(nle->error), nle->error);
            } else {
                PDS_TRACE_ERR("Netlink Recv ERROR %s %d", strerror(nle->error), nle->error);
            }
            return;
        }
        // Haven't received the response we are looking for yet
        continue;
    }
}

void
config_linux_intf_ip (uint32_t lnx_ifindex, const in_ipx_addr_t& ip,
                           uint32_t prefix_len, bool del)
{
    try {
        auto fdg = create_and_bind_nl_socket();
        int fd = fdg.get();

        uint64_t seq = time(NULL);
        uint32_t pid = pthread_self() << 16 | getpid();

        // Form netlink request
        auto nlmsg = make_ipaddr_req_nlmsg(pid, lnx_ifindex, ip, prefix_len, seq, del);

        // Create a temp thread to receive Netlink response
        // Transfer socket guard ownership - Socket will be closed by the recv thread
        std::thread nl_rcv_thrd(netlink_rcv_main, std::move(fdg), pid, seq);
        nl_rcv_thrd.detach();

        // Send Netlink request
        send_nlmsg(fd, (nlmsghdr*)&nlmsg);

    } catch (Error& e) {
        PDS_TRACE_ERR("Netlink %s", e.what());
    }
}

void
config_linux_loopback_ip (const in_ipx_addr_t& ip, uint32_t prefix_len, bool del)
{
    // Assumption: IfIndex 1 is always loopback
    config_linux_intf_ip(1, ip, prefix_len, del);
}

} // End namespace
