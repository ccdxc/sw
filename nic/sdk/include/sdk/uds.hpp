//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// SDK file descriptor header file
//------------------------------------------------------------------------------
#ifndef __SDK_UDS_HPP__
#define __SDK_UDS_HPP__

#include <sys/socket.h>

int
uds_send (int sock, int fd, char *iov_data, int iov_len)
{
    struct msghdr msghdr = {0};
    struct iovec io = {0};
    struct cmsghdr *cmsg;
    char   buffer[CMSG_SPACE(sizeof(int))];

    io.iov_base = iov_data;
    io.iov_len = iov_len;

    msghdr.msg_name = NULL;
    msghdr.msg_namelen = 0;
    msghdr.msg_iov = &io;
    msghdr.msg_iovlen = 1;
    msghdr.msg_flags = 0;
    if (fd >= 0) {
        msghdr.msg_control = buffer;
        msghdr.msg_controllen = CMSG_SPACE(sizeof(int));

        cmsg = CMSG_FIRSTHDR(&msghdr);
        cmsg->cmsg_len = CMSG_LEN(sizeof(int));
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;

        *((int *)CMSG_DATA(cmsg)) = fd;
    }
    return (sendmsg(sock, &msghdr, 0) >= 0 ? 0 : -1);
}

int
uds_recv (int sock, int *fd, void *iov_data, int iov_len)
{
    struct msghdr msghdr = {0};
    struct iovec io = {0};
    struct cmsghdr *cmsg;
    char   buffer[CMSG_SPACE(sizeof(int))];
    int    bytes_read;

    io.iov_base = iov_data;
    io.iov_len = iov_len;

    msghdr.msg_name = NULL;
    msghdr.msg_namelen = 0;
    msghdr.msg_iov = &io;
    msghdr.msg_iovlen = 1;
    msghdr.msg_flags = 0;
    msghdr.msg_control = buffer;
    msghdr.msg_controllen = CMSG_SPACE(sizeof(int));

    cmsg = CMSG_FIRSTHDR(&msghdr);
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    // initialize to -1, to identify invalid fds
    *((int *)CMSG_DATA(cmsg)) = -1;

    if ((bytes_read = recvmsg(sock, &msghdr, 0)) < 0) {
	    return -1;
    }

    if (fd) {
        *fd = *((int *)CMSG_DATA(cmsg));
    }
    return bytes_read;
}

#endif    // __SDK_UDS_HPP__
