#include <arpa/inet.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "gen/proto/oper.pb.h"
#include "gen/proto/types.pb.h"
#include "lib/operd/operd.hpp"

#define MAX_LINES 5000
#define MAX_FILES 5

static void
print_flow (int fd, char *data, uint64_t data_length)
{
    pds::FlowLog flow;
    
    bool result = flow.ParseFromArray(data, data_length);
    assert(result == true);
    
    switch (flow.logtype()) {
    case pds::FLOW_LOG_TYPE_OPEN:
        dprintf(fd, "Add, ");
        break;
    case pds::FLOW_LOG_TYPE_CLOSE:
        dprintf(fd, "Delete, ");
        break;
    default:
        assert(0);
    }
    switch (flow.flowaction()) {
    case types::SECURITY_RULE_ACTION_ALLOW:
        dprintf(fd, "allow, ");
        break;
    case types::SECURITY_RULE_ACTION_DENY:
        dprintf(fd, "deny, ");
        break;
    default:
        assert(0);
    }
    
    if (flow.has_key()) {
        if (flow.key().has_ipflowkey()) { 
            struct in_addr ip_addr;
            struct in6_addr ip6_addr;
            char buffer[INET6_ADDRSTRLEN];
            
            dprintf(fd, "ip, ");
            if (flow.key().ipflowkey().srcip().af() == types::IP_AF_INET) {
                ip_addr.s_addr = htonl(flow.key().ipflowkey().srcip().v4addr());
                inet_ntop(AF_INET, &ip_addr, buffer, INET6_ADDRSTRLEN);
            } else {
                memcpy(ip6_addr.s6_addr,
                       flow.key().ipflowkey().srcip().v6addr().data(),
                       16);
                inet_ntop(AF_INET6, &ip6_addr, buffer, INET6_ADDRSTRLEN);
            }
            dprintf(fd, "source: %s:%" PRIu32 ", ", buffer,
                   flow.key().ipflowkey().l4info().tcpudpinfo().srcport());

            
            if (flow.key().ipflowkey().dstip().af() == types::IP_AF_INET) {
                ip_addr.s_addr = htonl(flow.key().ipflowkey().dstip().v4addr());
                inet_ntop(AF_INET, &ip_addr, buffer, INET6_ADDRSTRLEN);
            } else {
                memcpy(ip6_addr.s6_addr,
                       flow.key().ipflowkey().dstip().v6addr().data(),
                       16);
                inet_ntop(AF_INET6, &ip6_addr, buffer, INET6_ADDRSTRLEN);
            }
            dprintf(fd, "destination: %s:%" PRIu32 ", ", buffer,
                   flow.key().ipflowkey().l4info().tcpudpinfo().dstport());
            
            dprintf(fd, "proto: %" PRIu32, flow.key().ipflowkey().ipprotocol());
        } else if (flow.key().has_macflowkey()) {
            dprintf(fd, "l2");
        }
    }
    fflush(stdout);
}

int
main(int argc, const char *argv[])
{
    char timebuf[256];
    struct tm *ltm;
    int lines = 0;
    int fd = -1;
    std::string filename;
    
    if (argc != 2) {
        printf("Usage: %s output_file\n", argv[0]);
        exit(-1);
    }

    filename = argv[1];

    while (true) {
        uint64_t next_size;
        int64_t timestamp;
        ssize_t n;
        char *buffer;

        // rotate the files if necessary
        if (fd == -1 || lines > MAX_LINES) {
            if (fd != -1) {
                close(fd);
                fd = -1;
            }
            for (int i = MAX_FILES - 1; i >= 0; i--) {
                std::string oldname;
                std::string newname;
                if (i == 0)
                    oldname = filename;
                else
                    oldname = filename + "." + std::to_string(i);
                newname = filename + "." + std::to_string(i + 1);
                rename(oldname.c_str(), newname.c_str());
            }
            fd = open(filename.c_str(), O_RDWR | O_CREAT, 0666);
            if (fd == -1) {
                fprintf(stderr, "Failed to open %s\n", filename.c_str());
                exit(-1);
            }
        }

        n = read(STDIN_FILENO, &next_size, sizeof(next_size));
        if (n != sizeof(next_size)) {
            fprintf(stderr, "Unexpectect next size\n");
            exit(-1);
        }

        n = read(STDIN_FILENO, &timestamp, sizeof(timestamp));
        if (n != sizeof(timestamp)) {
            fprintf(stderr, "Unexpectect timestamp size\n");
            exit(-1);
        }

        buffer = (char *)malloc(next_size);
        n = read(STDIN_FILENO, buffer, next_size);
        if (n < 0 || static_cast<uint64_t>(n) != next_size) {
            fprintf(stderr, "read size mismatch\n");
            exit(-1);
        }

        ltm = localtime(&timestamp);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", ltm);
        
        n = dprintf(fd, "%s ", timebuf);
        print_flow(fd, buffer, next_size);
        dprintf(fd, "\n");
        lines += 1;
        
        free(buffer);
    }
}
