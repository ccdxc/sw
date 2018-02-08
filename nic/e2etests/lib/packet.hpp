#ifndef LIB_PACKET_H
#define LIB_PACKET_H

#define ETH_ADDR_LEN 6

struct ether_header_t
{
    uint8_t  dmac[ETH_ADDR_LEN];      /* destination eth addr */
    uint8_t  smac[ETH_ADDR_LEN];      /* source ether addr    */
    uint16_t etype;                   /* ether type */
} __attribute__ ((__packed__));

struct vlan_header_t
{
    uint8_t  dmac[ETH_ADDR_LEN];      /* destination eth addr */
    uint8_t  smac[ETH_ADDR_LEN];      /* source ether addr    */
    uint16_t tpid;                    /* Tag protocol id*/
    uint16_t vlan_tag;                /* dot1p +cfi + vlan-id */
    uint16_t etype;                   /* ether type */
} __attribute__ ((__packed__));

struct ipv4_header_t {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t    version:4;
    uint8_t    ihl:4;
#else
    uint8_t    ihl:4;
    uint8_t    version:4;
#endif
    uint8_t    tos;
    uint16_t   tot_len;
    uint16_t   id;
    uint16_t   frag_off;
    uint8_t    ttl;
    uint8_t    protocol;
    uint16_t   check;
    uint32_t   saddr;
    uint32_t   daddr;
    /*The options start here. */
}__attribute__ ((__packed__));

struct tcp_header_t {
    uint16_t  sport;
    uint16_t  dport;
    uint32_t  seq;
    uint32_t  ack_seq;
#if __BYTE_ORDER == __BIG_ENDIAN
    uint16_t   doff:4,
        res1:4,
        cwr:1,
        ece:1,
        urg:1,
        ack:1,
        psh:1,
        rst:1,
        syn:1,
        fin:1;
#else
    uint16_t   res1:4,
        doff:4,
        fin:1,
        syn:1,
        rst:1,
        psh:1,
        ack:1,
        urg:1,
        ece:1,
        cwr:1;
#endif
    uint16_t  window;
    uint16_t  check;
    uint16_t  urg_ptr;
}__attribute__ ((__packed__));



struct tcp_pseudo /*the tcp pseudo header*/
{
  uint32_t src_addr;
  uint32_t dst_addr;
  uint8_t zero;
  uint8_t proto;
  uint16_t length;
};


int dump_pkt(char *pkt, int len, uint32_t htnp_port = 0);
long checksum(unsigned short *addr, unsigned int count);
long get_tcp_checksum(struct ipv4_header_t * myip, struct tcp_header_t * mytcp);
uint16_t hntap_get_etype(struct ether_header_t *eth);

#endif
