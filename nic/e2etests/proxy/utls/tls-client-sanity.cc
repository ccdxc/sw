
#include <unistd.h>
#include <sys/time.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <openssl/modes.h>
#include <openssl/aes.h>


#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
      TLOG("\t"); \
      printf(msg); \
      printf(" - %s ", SSL_state_string(ssl)); \
      printf(" - %s ", SSL_state_string_long(ssl)); \
      printf("\n"); \
    }\
 }

/*
 * Log with timestamp.
 */
static inline void
TLOG(const char *fmt, ...) {
  char   timestr[24];
  struct timeval tv;
  va_list args;

  gettimeofday(&tv, NULL);
  strftime(timestr, 24, "%H:%M:%S", gmtime(&tv.tv_sec));
  printf("[%s.%03ld] ", timestr, tv.tv_usec / 1000);

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

// INFO CALLBACK
void dummy_ssl_info_callback(const SSL* ssl, int where, int ret) {
  if(ret == 0) {
    TLOG("Server: ssl error occured.\n");
    return;
  }
  WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  WHERE_INFO(ssl, where, SSL_CB_EXIT, "Server: EXIT");
  WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
  WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
  WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
  WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "Server: HANDSHAKE DONE");
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

int bytes_recv = 0, bytes_sent = 0;
struct in_addr dest_ip = { 0 };
int dest_port = 0;
char *cipher = NULL;
char *certfile = NULL;
char *keyfile = NULL;
char *clientCAfile = NULL;
char *payload_file = NULL;

pthread_t server_thread;

int main_tls_client(void);

#define USAGE_STR   "usage: tls-client-sanity -d <dest-ip> -p <dest-tcp-port> > -r <cert-file> -k <key-file> -a <ca-cert-file> -f <data-file> \n"

int main(int argv, char* argc[]) {

    int     opt = 0;

  setlinebuf(stdout);
  setlinebuf(stderr);

    while ((opt = getopt(argv, argc, "d:p:r:k:a:f:")) != -1) {
        switch (opt) {
            case 'd':
                dest_ip.s_addr = inet_addr(optarg);
                TLOG("dest ip: %s\n", inet_ntoa(dest_ip));
                break;
            case 'p':
                dest_port = atoi(optarg);
                TLOG("port: %d\n", dest_port);
                break;
            case 'r':
                certfile = optarg;
                TLOG("Cert : [%s]\n", certfile);
                break;
            case 'k':
                keyfile = optarg;
                TLOG("Key: [%s]\n", keyfile);
                break;
            case 'a':
                clientCAfile = optarg;
                TLOG("CertCA : [%s]\n", clientCAfile);
                break;
            case 'f':
                payload_file = optarg;
                TLOG("Payload File: [%s]\n", payload_file);
                break;
            case '?':
            default:
                TLOG(USAGE_STR);
                exit(-1);
                break;
      }
  }
    if (!dest_ip.s_addr || !dest_port || !certfile || !keyfile || !clientCAfile || !payload_file) {
        TLOG(USAGE_STR);
        exit(-1);
    }

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();/* load all error messages */

  TLOG("Connecting to %s:%i, test-data file %s\n", inet_ntoa(dest_ip), dest_port, payload_file);

  main_tls_client();

  return 0;
}

int create_socket()
{
  int sockfd;
  struct sockaddr_in dest_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

  optval = 8;
  if (setsockopt(sockfd, IPPROTO_TCP, TCP_SYNCNT, &optval, sizeof(optval)) != 0) {
      TLOG("can't bind port - %s", strerror(errno));
      exit(-1);
  }

  memset(&(dest_addr), '\0', sizeof(dest_addr));
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(dest_port);

  dest_addr.sin_addr.s_addr = dest_ip.s_addr;

  if ( connect(sockfd, (struct sockaddr *) &dest_addr,
               sizeof(struct sockaddr_in)) == -1 ) {
    TLOG("Client: Connect failed - %s\n", strerror(errno));
    exit(-1);
  }
  TLOG("Client: TCP Connected - %s", strerror(errno));
  return sockfd;
}


int OpenListener(int port)
{   int sd;
  struct sockaddr_in addr;

  sd = socket(PF_INET, SOCK_STREAM, 0);
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int optval = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

  if ( bind(sd, (const struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
      TLOG("can't bind port - %s", strerror(errno));
      abort();
    }
  if ( listen(sd, 10) != 0 )
    {
      TLOG("Server: Can't configure listening port - %s", strerror(errno));
      abort();
    }
  return sd;
}

SSL_CTX* InitClientCTX(void)
{
  SSL_CTX *ctx;

  ctx = SSL_CTX_new(TLS_client_method());

  if ( ctx == NULL )
    {
      ERR_print_errors_fp(stderr);
      abort();
    }

   SSL_CTX_set_info_callback(ctx, dummy_ssl_info_callback);
   SSL_CTX_set_msg_callback(ctx, dummy_ssl_msg_callback);
  return ctx;
}

void LoadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile)
{
  if(clientCAfile) {
    TLOG("adding CA file: %s\n", clientCAfile);
    if(!SSL_CTX_load_verify_locations(ctx, clientCAfile, NULL)) {
        ERR_print_errors_fp(stderr);
        abort();
    }

    SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(clientCAfile));
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, NULL);
   }
  /* set the local certificate from CertFile */
  if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
      ERR_print_errors_fp(stderr);
      abort();
    }
  /* set the private key from KeyFile (may be the same as CertFile) */
  if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
      ERR_print_errors_fp(stderr);
      abort();
    }
  /* verify private key */
  if ( !SSL_CTX_check_private_key(ctx) )
    {
      TLOG("Private key does not match the public certificate\n");
      abort();
    }
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



  start = clock();

  filefd = open(payload_file, O_RDONLY);
  if (filefd == -1) {
      TLOG("Failed to open file: %s\n", payload_file);
      exit(-1);
  }
  totalbytes = 0;


  do {
    bytes = read(filefd, buf, sizeof(buf));
    totalbytes += bytes;
    if (bytes > 0) {
      SSL_write(ssl, buf, bytes);
    } else {
      break;
    }
  } while(bytes > 0);
  TLOG("Sent bytes %i \n", totalbytes);

  //sleep(5);

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

  ctx = InitClientCTX();/* initialize SSL */
  LoadCertificates(ctx, certfile, keyfile);/* load certs */

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
