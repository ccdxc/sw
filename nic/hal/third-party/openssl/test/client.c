#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/engine.h>

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int port=56789;
int transport_fd = 0;
#define IP 
int create_socket() {
  int sockfd;
  struct sockaddr_in dest_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&(dest_addr), '\0', sizeof(dest_addr));
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(port);

  //inet_pton(AF_INET, "192.168.69.136", &dest_addr.sin_addr.s_addr);
  inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr.s_addr);

  if ( connect(sockfd, (struct sockaddr *) &dest_addr,
               sizeof(struct sockaddr_in)) == -1 ) {
    perror("Connect: ");
    exit(-1);
  }

  return sockfd;
}

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

#define MAXSIZE 10240

void onevent(SSL* ssl, BIO* rbio, BIO* wbio) {
    char buf[MAXSIZE];
    size_t rbytes = BIO_ctrl_pending(rbio); 
    size_t wbytes = BIO_ctrl_pending(wbio); 
    size_t bytes_read;
    
    // write to socket
    int r = SSL_is_init_finished(ssl);
    if(r == 0) {
        rbytes = BIO_read(wbio, buf, sizeof(buf));
        printf("Received bytes from SSL network bio: %d\n", rbytes);
        send(transport_fd, buf, rbytes, 0);
    }
}

void init_pse_engine() 
{
    char* eng_path;
 
    ENGINE_load_dynamic();
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    ENGINE* pse_engine = ENGINE_by_id("dynamic");
    if(pse_engine == NULL) {
        printf("Failed to load dynamic engine\n");
        exit(0);
    }
    
    eng_path =  "/home/vagrant/ws/spjain13/src/github.com/pensando/sw/nic/conf/openssl/engine/libpse.so"; 
    printf("Loading pensando engine from path: %s\n", eng_path);
    
    if(!ENGINE_ctrl_cmd_string(pse_engine, "SO_PATH", eng_path, 0)) {
        printf("SO_PATH failed!!\n");
        exit(0);
    }

    if(!ENGINE_ctrl_cmd_string(pse_engine, "ID", "pse", 0)) {
        printf("ID failed!!\n");
        exit(0);
    }
    
    if(!ENGINE_ctrl_cmd_string(pse_engine, "LOAD", NULL, 0)) {
        printf("ENGINE LOAD_ADD failed, err: %s\n", 
            ERR_error_string(ERR_get_error(), NULL));
        exit(0);
    }
    int ret = ENGINE_init(pse_engine);
    printf("Successfully loaded OpenSSL Engine: %s init result: %d\n",
                            ENGINE_get_name(pse_engine), ret);
    ENGINE_set_default_EC(pse_engine);
    return;
}
int main()
{
    SSL_CTX *ctx;
    SSL *ssl;
    BIO *wbio = NULL;
    BIO *rbio  = NULL;

    BIO *ibio = NULL;
    BIO *nbio = NULL;

    //OPENSSL_init_crypto(OPENSSL_INIT_ENGINE_DYNAMIC, NULL);
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    SSL_load_error_strings();/* load all error messages */
  

    init_pse_engine();
#if 0
    ENGINE_load_dynamic();
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    ENGINE* pse_engine = ENGINE_by_id("pse");
    if(pse_engine == NULL) {
        printf("Failed to load dynamic engines\n");
        exit(1);
    }

    int ret = ENGINE_init(pse_engine);
    printf("Eng: %s init result: %d\n", ENGINE_get_name(pse_engine), ret);
    
    //ENGINE_set_default_EC(pse_engine);
#endif
    if ( (ctx = SSL_CTX_new(SSLv23_client_method())) == NULL)
    //if ( (ctx = SSL_CTX_new(TLS_client_method())) == NULL)
        printf("Unable to create a new SSL context structure.\n");

    //SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    
    // Force gcm(aes) mode
    SSL_CTX_set_cipher_list(ctx, "ECDHE-ECDSA-AES128-GCM-SHA256");
    //SSL_CTX_set_cipher_list(ctx, "ECDHE-RSA-AES128-GCM-SHA256");

    ssl = SSL_new(ctx);

    SSL_CTX_set_info_callback(ctx, dummy_ssl_info_callback); 
    SSL_CTX_set_msg_callback(ctx, dummy_ssl_msg_callback);
    
    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());
    if(!rbio || !wbio) {
        printf("Error: could not allocate bio");
        exit(-1);
    }
    BIO_set_mem_eof_return(rbio, -1);
    BIO_set_mem_eof_return(wbio, -1);
    
    BIO_new_bio_pair(&ibio, 0, &nbio, 0);    
    SSL_set_bio(ssl, ibio, ibio);
    //SSL_set_bio(ssl, rbio, wbio);
    SSL_set_connect_state(ssl);

    transport_fd = create_socket();

    //SSL_set_fd(ssl, transport_fd);
    /*
    if ( SSL_connect(ssl) != 1 ) {
        printf("Error: Could not build a SSL session\n");
        exit(-1);
    }
    */
    printf("Starting handshake \n");
    int r = 0;
    char buf[MAXSIZE];
    do {
        r = SSL_do_handshake(ssl);
        printf("handshake return: %d, err: %d\n", r, SSL_get_error(ssl, r)); 
        onevent(ssl, nbio, nbio);
        while(1) {
            // receive
            size_t bytes = recv(transport_fd, buf, 1024, 0);
            printf("Received from tcp: %d bytes\n", bytes);
            if(bytes <= 1024) {
                BIO_write(nbio, buf, bytes);
                break;
            }
        } 
    } while(r< 0);
}
