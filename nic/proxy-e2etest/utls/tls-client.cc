#include "nic/proxy-e2etest/ntls.hpp"

int bytes_recv;
int port;
char* test_data;


pthread_t server_thread;

void *main_server(void*);
int main_tls_client(void);

int main(int argv, char* argc[]) {

  if (argv != 3) {
    printf("usage: ./tls port test_data_file\n");
    exit(-1);
  }
  port = atoi(argc[1]);
  printf("Connecting to port %i\n", port);
  test_data = argc[2];

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();/* load all error messages */

  main_tls_client();
  return 0;
}

int create_socket() {
  int sockfd;
  struct sockaddr_in dest_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&(dest_addr), '\0', sizeof(dest_addr));
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(port);

  inet_pton(AF_INET, "10.0.100.2", &dest_addr.sin_addr.s_addr);

  if ( connect(sockfd, (struct sockaddr *) &dest_addr,
               sizeof(struct sockaddr_in)) == -1 ) {
    perror("Connect: ");
    exit(-1);
  }

  return sockfd;
}

void test_tls(SSL *ssl)
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
      printf("Sent bytes so far %i\n", totalbytes);
    } else {
      break;
    }
    res = SSL_read(ssl, buf, 1);
    total_recv += res;
    if (res < 0) {
      printf("SSL Read error: %i\n", res);
    } else {
      printf("Received openssl test data: %i %i\n", res, total_recv);
    }
	
  } while(bytes > 0);

  close(filefd);


  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("OpenSSL talk time: %.02f\n", cpu_time_used);
}

int main_tls_client() 
{
  SSL_CTX *ctx;
  SSL *ssl;
  int transport_fd = 0;


  if ( (ctx = SSL_CTX_new(SSLv23_client_method())) == NULL)
    printf("Unable to create a new SSL context structure.\n");

  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
  // Force gcm(aes) mode
  SSL_CTX_set_cipher_list(ctx, "ECDH-ECDSA-AES128-GCM-SHA256");

  ssl = SSL_new(ctx);

  transport_fd = create_socket();

  SSL_set_fd(ssl, transport_fd);

  if ( SSL_connect(ssl) != 1 ) {
    printf("Error: Could not build a SSL session\n");
    exit(-1);
  }

  // Start tests
  test_tls(ssl);

  while(1) {
   sleep(5);
  }

  SSL_free(ssl);
  close(transport_fd);
  SSL_CTX_free(ctx);
  return(0);
}


