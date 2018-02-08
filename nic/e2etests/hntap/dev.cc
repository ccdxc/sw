#include <iostream>

#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
#include <net/ethernet.h>
#include <net/route.h>
#include "nic/e2etests/lib/helpers.hpp"
#include "nic/e2etests/hntap/dev.hpp"

static int
hntap_route_add (int sockfd, const char *dest_addr, const char *gateway_addr) {

  struct rtentry     route;
  struct sockaddr_in *addr;

  int err = 0;
  memset(&route, 0, sizeof(route));
  addr = (struct sockaddr_in*) &route.rt_gateway;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(gateway_addr);
  addr = (struct sockaddr_in*) &route.rt_dst;
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = inet_addr(dest_addr);

  route.rt_flags = RTF_HOST;

  if ((err = ioctl(sockfd, SIOCADDRT, &route)) < 0) {
      perror("Route add failed");
      return -1;
  }
  return 1;
}

dev_handle_t* hntap_create_tunnel_device (tap_endpoint_t type,
             const char *dev, const char *dev_ip, const char *dev_ipmask,
             const char *route_dest, const char *route_gw)
{
  struct ifreq ifr;
  int      fd, err, sock;
  const char *tapdev = "/dev/net/tun";
  dev_handle_t *handle = NULL;

  if ((fd = open(tapdev, O_RDWR)) < 0 ) {
      TLOG("Failed to open Tap device %s\n", strerror(errno));
      abort();
      goto out;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  /* create the device */
  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
      TLOG("Failed to set Tap device property %s\n", strerror(errno));
      close(fd);
      abort();
      goto out;
  }

  sock = socket(PF_PACKET,SOCK_DGRAM,0);
  if (sock < 0) {
      TLOG("Failed to open socket %s\n", strerror(errno));
      close(fd);
      abort();
      goto out;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_UP | IFF_RUNNING | IFF_PROMISC;
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  /* Set the device UP */
  if ((err = ioctl(sock, SIOCSIFFLAGS, (void *) &ifr)) < 0 ) {
    TLOG("Failed to bring up Tap device %s\n", strerror(errno));
    close(sock);
    close(fd);
    abort();
    goto out;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr((const char *)dev_ip);
  memcpy(&ifr.ifr_addr, &sa, sizeof(struct sockaddr));

  if ((err = ioctl(sock, SIOCSIFADDR, &ifr)) < 0) {
    perror("IP address config failed");
    close(sock);
    close(fd);
    abort();
    goto out;
  }

  /*
 *    * Add the netmask.
 *       */
  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr((const char *)dev_ipmask);
  memcpy(&ifr.ifr_addr, &sa, sizeof(struct sockaddr));

  if ((err = ioctl(sock, SIOCSIFNETMASK, &ifr)) < 0) {
    perror("IP address mask config failed");
    close(sock);
    close(fd);
    abort();
    goto out;
  }

  /*
 *    * Add a route to the host/nw dest reachable thru this tap device.
 *       */
  if ((err = hntap_route_add(sock, route_dest, route_gw)) < 0) {
      perror("IP Route add failed");
      close(sock);
      close(fd);
      abort();
      goto out;
  }

  handle = (dev_handle_t*)malloc(sizeof(dev_handle_t));
  handle->sock = sock;
  handle->fd = fd;
  handle->tap_ep = type;
  handle->type = HNTAP_TUN;

out:
  return handle;
}

dev_handle_t* hntap_create_tap_device (tap_endpoint_t type,
        const char *dev, const char *dev_ip, const char *dev_ipmask)
{
  struct ifreq ifr;
  int      fd, err, sock;
  const char *tapdev = "/dev/net/tun";
  dev_handle_t *handle = NULL;

  if ((fd = open(tapdev, O_RDWR)) < 0 ) {
    abort();
    goto out;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  /* create the device */
  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
    close(fd);
    perror("2\n");
    goto out;
  }

  sock = socket(PF_PACKET,SOCK_DGRAM,0);
  if (sock < 0) {
    close(fd);
    perror("3\n");
    goto out;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_UP | IFF_RUNNING | IFF_PROMISC;
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  /* Set the device UP */
  if ((err = ioctl(sock, SIOCSIFFLAGS, (void *) &ifr)) < 0 ) {
    close(sock);
    close(fd);
    abort();
    goto out;
  }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr((const char *)dev_ip);
  memcpy(&ifr.ifr_addr, &sa, sizeof(struct sockaddr));

  if ((err = ioctl(sock, SIOCSIFADDR, &ifr)) < 0) {
    perror("IP address config failed");
    close(sock);
    close(fd);
    abort();
    goto out;
  }

  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr((const char *)dev_ipmask);
  memcpy(&ifr.ifr_addr, &sa, sizeof(struct sockaddr));

  if ((err = ioctl(sock, SIOCSIFNETMASK, &ifr)) < 0) {
    perror("IP address mask config failed");
    close(sock);
    close(fd);
    abort();
    goto out;
  }

  handle = (dev_handle_t*)malloc(sizeof(dev_handle_t));
  handle->sock = sock;
  handle->fd = fd;
  handle->tap_ep = type;
  handle->type = HNTAP_TAP;

out:
  return handle;
}
