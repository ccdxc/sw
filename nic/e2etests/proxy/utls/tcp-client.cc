#include "nic/e2etests/proxy/ntls.hpp"
#include <netinet/tcp.h>

int bytes_recv;
int port;
char* test_data;
bool from_localhost;
bool continuous_stream = false;

pthread_t server_thread;

void *main_server(void*);
int main_tcp_client(void);

int main(int argv, char* argc[]) {

  int opt = 0;

  setlinebuf(stdout);
  setlinebuf(stderr);

  if (argv != 7 && argv != 8) {
    TLOG( "usage: ./tcp-client -p <tcp-port> -d <test_data_file> -m from-host|from-net [-c]\n");
    exit(-1);
  }

  while ((opt = getopt(argv, argc, "p:d:m:c")) != -1) {
    switch (opt) {
    case 'p':
        port = atoi(optarg);
	TLOG( "port=%d\n", port);
	break;
    case 'd':
        test_data = optarg;
	break;
    case 'm':
        if (!strncmp(optarg, "from-host", 10)) {
            from_localhost = true;
        } else if (!strncmp(optarg, "from-net", 10)) {
	    from_localhost = false;
	} else {
            TLOG( "usage: ./tcp-client -p <tcp-port> -d <test_data_file> -m from-host|from-net \n");
	    exit(-1);
	}
        break;
    case 'c':
        continuous_stream = true;
        break;
    case '?':
    default:
        TLOG( "usage: ./tcp-client -p <tcp-port> -d <test_data_file> -m from-host|from-net \n");
	exit(-1);
        break;
    }

  }

  TLOG("Connecting to port %i, test-data file %s, %s\n", port, test_data,
	  from_localhost ? "from host" : "from network");

  main_tcp_client();
  return 0;
}

int create_socket() {
  int sockfd;
  struct sockaddr_in dest_addr;
  struct sockaddr_in src_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&(src_addr), '\0', sizeof(src_addr));
  src_addr.sin_family=AF_INET;
  src_addr.sin_port=htons(0xbaba);

  if (from_localhost) {
      inet_pton(AF_INET, "64.1.0.4", &src_addr.sin_addr.s_addr);
  } else {
      inet_pton(AF_INET, "64.0.0.2", &src_addr.sin_addr.s_addr);
  }


  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

  optval = 8;
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_SYNCNT, &optval, sizeof(optval)) != 0) {
      TLOG("can't bind port - %s", strerror(errno));
      exit(-1);
  }

  if ( bind(sockfd, (const struct sockaddr*)&src_addr, sizeof(src_addr)) != 0 ) {
      TLOG("can't bind port - %s", strerror(errno));
      exit(-1);
  }

  memset(&(dest_addr), '\0', sizeof(dest_addr));
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(port);

  if (from_localhost) {
      inet_pton(AF_INET, "64.0.0.1", &dest_addr.sin_addr.s_addr);
  } else {
      inet_pton(AF_INET, "64.1.0.3", &dest_addr.sin_addr.s_addr);
  }

  if ( connect(sockfd, (struct sockaddr *) &dest_addr,
               sizeof(struct sockaddr_in)) == -1 ) {
      TLOG("Client: Connect failed: %s", strerror(errno));
      exit(-1);
  }

  return sockfd;
}

void test_tcp(int transport_fd)
{
  clock_t start, end;
  double cpu_time_used;

  int filefd;
  int bytes;
  int totalbytes = 0;
  bytes_recv = 0;
  char buf[16384];

  int res = 0;
  int total_recv = 0;
  struct timeval tv;
  int do_nb_recv = 0;

  tv.tv_sec = 300;
  tv.tv_usec = 0;
  res = setsockopt(transport_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval));
  if (res < 0) {
      TLOG("Error setting timeout for recv() %s, will do non-blocking\n", strerror(errno));
      do_nb_recv = 1;
  }

  start = clock();

  filefd = open(test_data, O_RDONLY);
  totalbytes = 0;

  res = 0;
  total_recv = 0;

  do {
    memset(buf, 0, sizeof(buf));
    bytes = read(filefd, buf, sizeof(buf));
    totalbytes += bytes;
    if (bytes > 0) {
      send(transport_fd, buf, bytes, 0);
      TLOG("Client: Sent bytes so far %i - %s\n", totalbytes, buf);
    } else {
      break;
    }

    do {
        memset(buf, 0, sizeof(buf));
        res = recv(transport_fd, buf, sizeof(buf), do_nb_recv ? MSG_DONTWAIT : 0);
	if (res < 0) {
	    if (do_nb_recv && (errno == EWOULDBLOCK || errno == EAGAIN)) {
	        sleep(5);
	    } else {
                TLOG("Client: TCP Read error: %i (%s)\n", res, strerror(errno));
		exit(-1);
		break;
	    }
	}
    } while (do_nb_recv--);

    //res = recv(transport_fd, buf, sizeof(buf), 0);
    total_recv += res;
    if (res < 0) {
      TLOG("Client: TCP Read error: %i\n", res);
    } else {
      TLOG("Client: Received tcp test data: %i %i, %s\n", res, total_recv, buf);
    }
	
  } while(bytes > 0);

  close(filefd);


  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  TLOG("Client: TCP talk time: %.02f\n", cpu_time_used);
}

int main_tcp_client() 
{
  int transport_fd = 0;


  transport_fd = create_socket();

  TLOG("Client: Connected ! - transport fd %d\n", transport_fd);
  do {
    sleep(1);
  } while(0);

  // Start tests
  do {
    test_tcp(transport_fd);
    sleep(1);
  } while (continuous_stream);

  close(transport_fd);

  return(0);
}


