#include "nic/proxy-e2etest/ntls.hpp"

#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
      printf("\t"); \
      printf(msg); \
      printf(" - %s ", SSL_state_string(ssl)); \
      printf(" - %s ", SSL_state_string_long(ssl)); \
      printf("\n"); \
    }\
 }

// INFO CALLBACK
void dummy_ssl_info_callback(const SSL* ssl, int where, int ret) {
  if(ret == 0) {
    printf("ssl error occured.\n");
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
  printf("\tMessage callback with length: %zu\n", len);
}

int bytes_recv,bytes_sent;
int port;


pthread_t server_thread;

void *main_server(void*);

int main(int argv, char* argc[]) {

  if (argv != 2) {
    printf("usage: ./tls port\n");
    exit(-1);
  }
  port = atoi(argc[1]);
  printf("Serving port %i\n", port);

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();/* load all error messages */

  int rc = pthread_create(&server_thread, NULL, main_server, NULL);
  if (rc) {
    printf("Error creating server %i\n", rc);
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

  if ( bind(sd, (const struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
      perror("can't bind port");
      abort();
    }
  if ( listen(sd, 10) != 0 )
    {
      perror("Can't configure listening port");
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
   SSL_CTX_set_msg_callback(ctx, dummy_ssl_msg_callback);
  return ctx;
}

void LoadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile)
{
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
      fprintf(stderr, "Private key does not match the public certificate\n");
      abort();
    }
}

void test_tls(SSL *ssl)
{
  int bytes, send_bytes;
  char buf[16384];
  int i;
  do {

    bytes = SSL_read(ssl, buf, sizeof(buf));/* get request */
    if ( bytes > 0 ) {
      bytes_recv += bytes;
      printf("Bytes recv: %i\n", bytes_recv);
    }
    else {
      ERR_print_errors_fp(stderr);
      break;
    }
    for (i = 0; i < bytes; i++) {
      printf(" %02x", buf[i]);
    }
    printf("\n");
    send_bytes = SSL_write(ssl, buf, bytes);
    printf("Bytes sent: %i\n", send_bytes);
    bytes_sent += send_bytes; 
  } while (bytes > 0 );

}

void Servlet(int client, SSL* ssl)/* Serve the connection -- threadable */
{

  int sd;

  if ( SSL_accept(ssl) == -1 ) {
    ERR_print_errors_fp(stderr);
  } else {

    test_tls(ssl);

    //ntls_attach(ssl, client);

    while (1) {
      sleep(5);
    }
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
  LoadCertificates(ctx, "proxy-e2etest/ca.crt", "proxy-e2etest/ca.pem");/* load certs */
  SSL_CTX_set_cipher_list(ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");

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
