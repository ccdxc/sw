#ifndef HNTAP_DEV_H
#define HNTAP_DEV_H
#include <map>
#include "nic/e2etests/lib/packet.hpp"

enum tap_endpoint_t {
    TAP_ENDPOINT_NONE,
    TAP_ENDPOINT_HOST,
    TAP_ENDPOINT_NET,
    TAP_ENDPOINT_MAX
};

enum dev_type_t {
    HNTAP_TAP,
    HNTAP_TUN
};

enum pkt_direction_t {
    PKT_DIRECTION_FROM_DEV,
    PKT_DIRECTION_TO_DEV,
};

typedef int (*pkt_pre_process_func)(char *pktbuf, uint32_t len);
typedef int (*pkt_process_nat_cb)(char *pktbuf, uint32_t len, pkt_direction_t direction);

typedef struct dev_handle_t {
#define MAX_TAP_APP_PORTS 1
#define TX_RX_QUEUE_SIZE 16
    dev_type_t           type;
    tap_endpoint_t       tap_ep;
    int                  sock;
    int                  fd;
    bool                 needs_vlan_tag; //Add vlan tag for Tunneled packets only.
    int                  lif_id;
    uint32_t             port;
    const char           name[50];
    const char           ip[50];
    const char           ip_mask[50];
    const char           route_dest[50];
    const char           route_gw[50];
    pkt_pre_process_func pre_process;
    pkt_process_nat_cb   nat_cb;
    uint32_t             tap_ports[MAX_TAP_APP_PORTS];
    uint32_t             seqnum[MAX_TAP_APP_PORTS];
    struct tcp_header_t  flowtcp[MAX_TAP_APP_PORTS];
    uint8_t             *recv_buf[TX_RX_QUEUE_SIZE];

} dev_handle_t;


void add_dev_handle_tap_pair(dev_handle_t *dev, dev_handle_t *other_dev);

dev_handle_t* hntap_create_tunnel_device(tap_endpoint_t type,
        const char *dev, const char *dev_ip, const char *dev_ipmask,
        const char *route_dest, const char *route_gw);

dev_handle_t* hntap_create_tunnel_devicev6(tap_endpoint_t type,
        const char *dev, const char *dev_ip,
        const char *route_dest, const char *route_gw);

dev_handle_t* hntap_create_tap_device(tap_endpoint_t type,
        const char *dev, const char*mac_addr = NULL,
        const char *dev_ip = NULL, const char *dev_ipmask = NULL);

void hntap_work_loop(dev_handle_t *dev_handles[], uint32_t max_handles, bool send_recv_parallel = false);

static inline const char* hntap_type(tap_endpoint_t type) {
    switch(type) {
        case TAP_ENDPOINT_HOST:
            return "Host Interface";
        case TAP_ENDPOINT_NET:
            return "Network Interface";
        default:
            abort();
    }
    return nullptr;
}

#endif
