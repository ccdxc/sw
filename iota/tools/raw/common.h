#define _GNU_SOURCE // sendmmsg

#include <errno.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <pcap/pcap.h>
#include <time.h>

#define MAX_PKT_SIZE            9000
#define MAX_NUM_PKTS            4096
#define NUM_PKTS_PER_SEND       1 // 1024

#define TRUE    1
#define FALSE   0

typedef enum {
    LOG_LEVEL_NONE,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_VERBOSE,
} log_level_t;

typedef enum {
    RUNMODE_NONE,
    RUNMODE_SENDER,
    RUNMODE_RECEIVER,
} runmode_t;

typedef struct ether_addr   mac_addr_t;
typedef struct ether_header eth_hdr_t;
typedef struct iphdr        ip_hdr_t;

typedef struct global_options_s {
    uint8_t     *ifname;
    uint8_t     *pcapfile;
    log_level_t loglevel;
    runmode_t   runmode;

    uint16_t    etype;
    uint8_t     ip_proto;
    uint16_t    sport;
    uint16_t    dport;
    uint16_t    num_flows;
    uint32_t    sip;
} global_options_t;
extern global_options_t    glopts;

typedef struct pkt_info_s {
    uint8_t     pkt[MAX_PKT_SIZE];
    uint32_t    len;
} pkt_info_t;

typedef struct global_info_s {
    int                 fd;
    log_level_t         loglevel;
    pkt_info_t          pktinfo[MAX_NUM_PKTS];
    
    int                 ifindex;
    mac_addr_t          srcmac;
    uint32_t            num_pkts;
    struct sockaddr_ll  address;

    struct iovec        iovecs[MAX_NUM_PKTS];
    struct mmsghdr      mmsgs[MAX_NUM_PKTS];

    struct timespec     beg_tspec;
    struct timespec     end_tspec;
    
    long                total_pkts;
    long long           total_bytes;
} global_info_t;
extern global_info_t       glinfo;

#define LOG_COMMON(_level, _prefix, _msg, _args...) \
{\
    if (glinfo.loglevel >= _level) {\
        printf("%s ", _prefix);\
        printf(_msg, ##_args); \
        printf("\n"); \
    } \
}

#define LOG_ERROR(_msg, _args...)\
        LOG_COMMON(LOG_LEVEL_ERROR, "[ERRR]", _msg, ##_args)
#define LOG_INFO(_msg, _args...)\
        LOG_COMMON(LOG_LEVEL_INFO, "[INFO]", _msg, ##_args)
#define LOG_DEBUG(_msg, _args...)\
        LOG_COMMON(LOG_LEVEL_DEBUG, "[DEBG]", _msg, ##_args)
#define LOG_VERBOSE(_msg, _args...)\
        LOG_COMMON(LOG_LEVEL_VERBOSE, "[VERB]", _msg, ##_args)

#define maccpy(_dst, _src) \
{ \
    for (int i = 0; i < 6; i++) { \
        _dst[i] = _src[i]; \
    } \
}

#define IS_RUNMODE_SENDER()\
        (glopts.runmode == RUNMODE_SENDER)
#define IS_RUNMODE_RECEIVER()\
        (glopts.runmode == RUNMODE_RECEIVER)
#define IS_RUNMODE_NONE()\
        (glopts.runmode == RUNMODE_NONE)

int
parse_args (int argc, char *argv[]);

int
init_glinfo ();
