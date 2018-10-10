#include "common.h"

static int gl_test_duration;

int
add_new_packet (int index, const uint8_t *pkt, uint32_t len)
{
    glinfo.pktinfo[index].len = len;
    memcpy(glinfo.pktinfo[index].pkt, pkt, len);
    // Copy the correct SourceMAC.
    memcpy(glinfo.pktinfo[index].pkt + ETH_ALEN, 
           glinfo.srcmac.ether_addr_octet, ETH_ALEN);
    //memcpy(glinfo.pktinfo[index].pkt, glinfo.dstmac, ETH_ALEN);
    glinfo.num_pkts++;
    return 0;
}

int
read_data_file ()
{
    char                errbuff[PCAP_ERRBUF_SIZE];
    struct pcap_pkthdr  *header;
    const uint8_t       *packet;   
    int                 pkt_index = 0;
    pcap_t              *handler = NULL;
    
    handler = pcap_open_offline(glopts.pcapfile, errbuff);

    while (pcap_next_ex(handler, &header, &packet) >= 0) {
        LOG_DEBUG("Packet # %i", pkt_index);
        LOG_DEBUG("Packet size: %d bytes", header->len);
        add_new_packet(pkt_index, packet, header->len);
        pkt_index++;
    }
}

int
init_destination_address ()
{
    uint8_t dest[ETH_ALEN] = {0x08, 0x00, 0x27, 0xbf, 0x95, 0xfd};

    bzero(&glinfo.address, sizeof(glinfo.address));
    glinfo.address.sll_ifindex = glinfo.ifindex;
    glinfo.address.sll_halen = ETH_ALEN;
    memcpy(glinfo.address.sll_addr, dest, ETH_ALEN);
}

int
prepare_message ()
{
    init_destination_address();
    return 0;
}

int
init_sender_iovecs ()
{
    int i = 0;
    int pktindex = 0;

    if (glinfo.num_pkts < MAX_NUM_PKTS) {
        // If we have less than 1K packets, fill the remaining iovecs,
        // with same packets.
        LOG_DEBUG("NumPkts:%d < MaxPkts:%d", glinfo.num_pkts, MAX_NUM_PKTS);
        for (i = glinfo.num_pkts; i < MAX_NUM_PKTS; i++) {
            glinfo.pktinfo[i].len = glinfo.pktinfo[pktindex].len;
            memcpy(glinfo.pktinfo[i].pkt, glinfo.pktinfo[pktindex].pkt,
                   MAX_PKT_SIZE);
            pktindex = (pktindex + 1) % glinfo.num_pkts;
        }
        glinfo.num_pkts = MAX_NUM_PKTS;
        LOG_DEBUG("Updating NumPkts to %d", glinfo.num_pkts);
    }

    for (i = 0; i < MAX_NUM_PKTS; i++) {
        glinfo.iovecs[i].iov_len = glinfo.pktinfo[i].len;
    }

    return 0;
}

int
is_timeout ()
{
    clock_gettime(CLOCK_REALTIME, &glinfo.end_tspec);
    time_t total_time = glinfo.end_tspec.tv_sec - glinfo.beg_tspec.tv_sec;
    if (total_time > gl_test_duration) {
        return TRUE;
    }

    return FALSE;
}

int
start_sender ()
{
    read_data_file();
    prepare_message();
    init_sender_iovecs();

    while (1) {
        for (int i = 0; i < MAX_NUM_PKTS / NUM_PKTS_PER_SEND; i++) {
            int count = sendmmsg(glinfo.fd, 
                                 glinfo.mmsgs + i * NUM_PKTS_PER_SEND,
                                 NUM_PKTS_PER_SEND, 0);
            if (count < 0) {
                perror("sendmmsg");
                exit(1);
            }
            LOG_DEBUG("%d Packets sent successfully.", NUM_PKTS_PER_SEND);
        }
        
        glinfo.total_pkts += MAX_NUM_PKTS;
        glinfo.total_bytes += (MAX_NUM_PKTS * glinfo.pktinfo[0].len); 

        if (is_timeout()) {
            return 0;
        }
    }

    return 0;
}

int
print_raw_buffer (uint8_t *buf, uint32_t len)
{
    int i = 0;

    for (i = 0; i < len; i++) {
        if (i && i % 4 == 0) {
            printf(" ");
        }
        if (i && i % 16 == 0) {
            printf("\n");
        }
        printf("%02x", buf[i]);
    }
    printf("\n");
    return 0;
}

int
start_receiver ()
{
    int count = 0;
    //uint8_t buffer[4096];

    while (1) {
        //bzero(buffer, sizeof(buffer));
        count = recvmmsg(glinfo.fd, &glinfo.mmsgs[0], MAX_NUM_PKTS, MSG_DONTWAIT, NULL);
        //count = recvmmsg(glinfo.fd, &glinfo.mmsgs[0], 32, 0, NULL);
        //count = recv(glinfo.fd, buffer, 1500, 0);
        //count = recvfrom(glinfo.fd, buffer, 1500, 0, NULL, NULL);
        if (count < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                goto check_timeout;
            }
            perror("recvmmsg");
            exit(1);
        }

        LOG_DEBUG("Received %d packets.", count);
        //print_raw_buffer(buffer, count);
        for (int i = 0; i < count; i++) {
            LOG_DEBUG(" - Packet#%d: Length:%d", i, glinfo.mmsgs[i].msg_len);
            glinfo.total_bytes += glinfo.mmsgs[i].msg_len;

            glinfo.mmsgs[i].msg_hdr.msg_flags = 0;
            glinfo.mmsgs[i].msg_len = 0;
        }

        glinfo.total_pkts += count;

check_timeout:
        if (is_timeout()) {
            return 0;
        }
    }

    LOG_INFO("Packets sent successfully.");
    return 0;
}

int
print_stats ()
{
    time_t total_time = glinfo.end_tspec.tv_sec - glinfo.beg_tspec.tv_sec;
    
    int pps = glinfo.total_pkts / total_time;
    long long bw = glinfo.total_bytes / total_time * 8;

    printf("Runtime         : %ld seconds\n", total_time);
    printf("Total Packets   : %ld @ %d Kpps\n",
           glinfo.total_pkts, pps / 1024);
    printf("Total Bytes     : %lld @ %lld Mbps\n",
           glinfo.total_bytes, bw/1024/1024);
}

int
start_tests ()
{
    clock_gettime(CLOCK_REALTIME, &glinfo.beg_tspec);

    if (IS_RUNMODE_SENDER()) {
        start_sender();
    } else if (IS_RUNMODE_RECEIVER()) {
        start_receiver();
    } else {
        LOG_ERROR("Sender or Receiver not specified.");
    }

    print_stats();

    return 0;
}


int
main (int argc, char *argv[])
{
    int                 ret = 0;
    
    gl_test_duration = atoi(getenv("TEST_DURATION"));

    parse_args(argc, argv);
    
    init_glinfo();

    start_tests();

    return 0;
}
