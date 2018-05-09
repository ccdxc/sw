#include "nic/e2etests/proxy/ntls.hpp"
#include <netinet/tcp.h>

int bytes_recv;
int port;
char* test_data;
bool from_localhost=true;

pthread_t server_thread;

void *main_server(void*);

int main_tls_client(void);
#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
      TLOG("\t"); \
      TLOG(msg); \
      TLOG(" - %s ", SSL_state_string(ssl)); \
      TLOG(" - %s ", SSL_state_string_long(ssl)); \
      TLOG("\n"); \
    }\
 } 

// INFO CALLBACK
void dummy_ssl_info_callback(const SSL* ssl, int where, int ret) {
  if(ret == 0) {
    TLOG("ssl error occured.\n");
    return;
  }
  WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  WHERE_INFO(ssl, where, SSL_CB_EXIT, "EXIT");
  WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
  WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
  WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
  WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}

// MSG CALLBACK
void dummy_ssl_msg_callback(
                            int writep
                            ,int version
                            ,int contentType
                            ,const void* buf
                            ,size_t len
                            ,SSL* ssl
                            ,void *arg
                            ) 
{
    TLOG("\tMessage callback with length: %zu\n", len);
}

static void 
usage() 
{
    TLOG("usage: ./tls -p port -d test_data_file -m from-host|from-net\n");
}

int main(int argc, char* argv[]) {
  int opt = 0;

  setlinebuf(stdout);
  setlinebuf(stderr);

  if (argc != 7 && argc != 8) {
      usage();
      exit(-1);
  }

  while ((opt = getopt(argc, argv, "p:d:m:c")) != -1) {
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
        usage();
        exit(-1);
	}
        break;
    case '?':
    default:
        usage();
     	exit(-1);
        break;
    }

  }

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();/* load all error messages */

  TLOG("Connecting to port %i, test-data file %s, %s\n", port, test_data,
	  from_localhost ? "from host" : "from network");

  main_tls_client();
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
    TLOG("Client: Connect failed - %s", strerror(errno));
    exit(-1);
  }
  TLOG("Client: TCP Connected - %s", strerror(errno));
  return sockfd;
}

void test_tls(SSL *ssl, int transport_fd)
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
    bytes = read(filefd, buf, sizeof(buf));
    totalbytes += bytes;
    if (bytes > 0) {
      SSL_write(ssl, buf, bytes);
      TLOG("Sent bytes so far %i - %s\n", totalbytes, buf);
    } else {
      break;
    }

    do {
        memset(buf, 0, sizeof(buf));
        res = SSL_read(ssl, buf, sizeof(buf));
        if(res < 0) {
            int ssl_err = SSL_get_error(ssl, res);
            TLOG("Client: TLS read error: %i, %i\n", res, ssl_err);
            if(ssl_err == SSL_ERROR_WANT_READ || ssl_err == SSL_ERROR_WANT_WRITE) {
                sleep(5);
            } else {
                exit(-1);
                break;
            }
        }
    } while (do_nb_recv--);

    total_recv += res;
    if (res < 0) {
        TLOG("SSL Read error: %i\n", res, SSL_get_error(ssl, res));
    } else {
        TLOG("Received openssl test data: %i %i, %s\n", res, total_recv, buf);
    }
	
  } while(bytes > 0);

  close(filefd);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  TLOG("OpenSSL talk time: %.02f\n", cpu_time_used);
}

int main_tls_client() 
{
  SSL_CTX *ctx;
  SSL *ssl;
  int transport_fd = 0;


  if ( (ctx = SSL_CTX_new(TLS_client_method())) == NULL)
    TLOG("Unable to create a new SSL context structure.\n");
  
  SSL_CTX_set_info_callback(ctx, dummy_ssl_info_callback); 
  SSL_CTX_set_msg_callback(ctx, dummy_ssl_msg_callback);

  ssl = SSL_new(ctx);
  SSL_set_connect_state(ssl);

  transport_fd = create_socket();
  TLOG("Client: TCP socket created, starting SSL...\n");

  SSL_set_fd(ssl, transport_fd);

  int ret = SSL_connect(ssl);
  if ( ret != 1 ) {
      TLOG("Error: Failure in SSL Connect: %d, err: %d\n", ret, SSL_get_error(ssl, ret));
      exit(-1);
  }

  TLOG("Client: Connected ! - transport fd %d\n", transport_fd);


  // Start tests
  test_tls(ssl, transport_fd);
  sleep(1);
  
  SSL_free(ssl);
  close(transport_fd);
  SSL_CTX_free(ctx);
  return(0);
}


