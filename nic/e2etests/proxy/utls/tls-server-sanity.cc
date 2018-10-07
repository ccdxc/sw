#include <unistd.h>
#include <sys/time.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

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

int bytes_recv = 0, bytes_sent = 0;
int port = 0;
char *cipher = NULL;
char *certfile = NULL;
char *keyfile = NULL;
char *clientCAfile = NULL;
char *payload_file = NULL;

pthread_t server_thread;

#define USAGE_STR   "usage: ./tls-server-sanity -p <listen-tcp-port> -c <cipher> -r <cert-file> -k <key-file> -a <ca-cert-file> -f <data-file> \n"

void *main_server(void*);

int main(int argv, char* argc[]) {

    int     opt = 0;

  setlinebuf(stdout);
  setlinebuf(stderr);

    while ((opt = getopt(argv, argc, "p:c:r:k:a:f:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                TLOG("port: %d\n", port);
                break;
            case 'c':
                cipher = optarg;
                TLOG("Cipher: [%s]\n", cipher);
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
    if (!port || !cipher || !certfile || !keyfile || !clientCAfile || !payload_file) {
        TLOG(USAGE_STR);
        exit(-1);
    }

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();/* load all error messages */

  int rc = pthread_create(&server_thread, NULL, main_server, NULL);
  if (rc) {
    TLOG("Error creating server %i\n", rc);
    exit(-1);
  }
  while (1) { 
	sleep(2);
  }

  return 0;
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

SSL_CTX* InitServerCTX(void)
{
  SSL_CTX *ctx;

  ctx = SSL_CTX_new(SSLv23_server_method());/* create new context from method */

  if ( ctx == NULL )
    {
      ERR_print_errors_fp(stderr);
      abort();
    }

   SSL_CTX_set_info_callback(ctx, dummy_ssl_info_callback);
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

void test_tls(SSL *ssl)
{
  FILE  *fp;
  int bytes;
  char buf[16384];
  char fbuf[16384];
  uint32_t bytes_read = 0;
  uint32_t file_size = 0;
  struct stat fst;

  if (stat(payload_file, &fst)) {
      TLOG("Failed to stat file %s: err: %d\n", payload_file, errno);
      exit(-1);
  }
  else {
      file_size = fst.st_size;
  }

  fp = fopen(payload_file, "r");
  if (!fp) {
      TLOG("Failed to open payload file: <%s>\n", payload_file);
      exit(-1);
  }
 
  TLOG("Starting data reception\n");
  do {

    bytes = SSL_read(ssl, buf, sizeof(buf));/* get request */
    TLOG("received bytes: %d\n", bytes);
    if ( bytes > 0 ) {
      bytes_recv += bytes;
      bytes_read = fread(fbuf, 1, bytes, fp);
      if (bytes_read != (uint32_t)bytes) {
          TLOG("Receive validation failed: length mismatch\n");
          exit(-1);
      }
      else {
          if (strncmp(buf, fbuf, bytes)) {
              TLOG("Receive validation failed: content mismatch\n");
              exit(-1);
          }
          else {
              if (bytes_recv == file_size)
                  TLOG("Received Data Validation successful\n");
                  return;
          }
      }
    }
    else {
      ERR_print_errors_fp(stderr);
      break;
    }
  } while (bytes > 0 );
  TLOG("Ending data reception\n");

  fclose(fp);

}

void Servlet(int client, SSL* ssl)/* Serve the connection -- threadable */
{

  int sd;

  TLOG("Start Accept\n");
  if ( SSL_accept(ssl) == -1 ) {
    ERR_print_errors_fp(stderr);
  } else {

    test_tls(ssl);

    //ntls_attach(ssl, client);

  }
  sd = SSL_get_fd(ssl);/* get socket connection */
  SSL_free(ssl);/* release SSL state */
  close(sd);/* close connection */
  exit(0);
}

void *main_server(void* unused)
{
  SSL_CTX *ctx;

  ctx = InitServerCTX();/* initialize SSL */
  LoadCertificates(ctx, certfile, keyfile);/* load certs */
  SSL_CTX_set_cipher_list(ctx, cipher);

  int server = OpenListener(port);/* create server socket */
  while (1)
    {
      struct sockaddr_in addr;
      unsigned int len = sizeof(addr);
      SSL *ssl;

      int client = accept(server, (struct sockaddr*) &addr, &len);/* accept connection as usual */

      ssl = SSL_new(ctx);         /* get new SSL state with context */
      SSL_set_fd(ssl, client);/* set connection socket to SSL state */
      Servlet(client, ssl);/* service connection */
    }
  close(server);/* close server socket */
  SSL_CTX_free(ctx);/* release context */

  return NULL;
}
