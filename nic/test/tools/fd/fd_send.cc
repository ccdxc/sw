//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains core helper functions
///
//----------------------------------------------------------------------------

#include <string>
#include <cstdio>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include "nic/sdk/include/sdk/uds.hpp"

#define FD_RECV_SOCKET_PATH             "/tmp/fd_recv_sock"

void
fd_send_test (void)
{
    char iov_data = '*'; 
    int sock_fd, send_fd = STDOUT_FILENO;
    struct sockaddr_un sock_addr;

    // Initialize unix socket
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Failed to open unix domain socket for fd send\n");
        return;
    }

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, FD_RECV_SOCKET_PATH);

    if (connect(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        printf("Failed to connect unix domain socket for fd send\n");
        return;
    }

    if (uds_send(sock_fd, send_fd, &iov_data, 1)) {
        printf("Send fd failed\n");
        return;
    }

    printf("Sent fd: %d\n", send_fd);

    return;
}

int main()
{
    fd_send_test();
    return 0;
}
