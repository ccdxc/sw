#ifndef NTLS_H
#define NTLS_H

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include <linux/if_alg.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/sendfile.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <openssl/modes.h>
#include <openssl/aes.h>


/* Opaque OpenSSL structures to fetch keys */
#define u64 uint64_t
#define u32 uint32_t
#define u8 uint8_t

typedef struct {
  u64 hi, lo;
} u128;

typedef struct {
  /* Following 6 names follow names in GCM specification */
  union {
    u64 u[2];
    u32 d[4];
    u8 c[16];
    size_t t[16 / sizeof(size_t)];
  } Yi, EKi, EK0, len, Xi, H;
  /*
   * Relative position of Xi, H and pre-computed Htable is used in some
   * assembler modules, i.e. don't change the order!
   */
#if TABLE_BITS==8
  u128 Htable[256];
#else
  u128 Htable[16];
  void (*gmult) (u64 Xi[2], const u128 Htable[16]);
  void (*ghash) (u64 Xi[2], const u128 Htable[16], const u8 *inp,
                 size_t len);
#endif
  unsigned int mres, ares;
  block128_f block;
  void *key;
} gcm128_context_alias;

typedef struct {
  union {
    double align;
    AES_KEY ks;
  } ks;                       /* AES key schedule to use */
  int key_set;                /* Set if key initialised */
  int iv_set;                 /* Set if an iv is set */
  gcm128_context_alias gcm;
  unsigned char *iv;          /* Temporary IV store */
  int ivlen;                  /* IV length */
  int taglen;
  int iv_gen;                 /* It is OK to generate IVs */
  int tls_aad_len;            /* TLS AAD length */
  ctr128_f ctr;
} EVP_AES_GCM_CTX;


/* AF_ALG defines not in linux headers */
#ifndef AF_ALG
#define AF_ALG 38
#endif
#ifndef SOL_ALG
#define SOL_ALG 279
#endif

#ifndef ALG_SET_AEAD_ASSOCLEN
#define ALG_SET_AEAD_ASSOCLEN 4
#endif
#ifndef ALG_SET_AEAD_AUTHSIZE
#define ALG_SET_AEAD_AUTHSIZE 5
#endif
#ifndef ALG_SET_PUBKEY
#define ALG_SET_PUBKEY 6
#endif

#define PF_NTLS                         12
#define AF_NTLS                         PF_NTLS

/* getsockopt() optnames */
#define NTLS_SET_IV_RECV                1
#define NTLS_SET_KEY_RECV               2
#define NTLS_SET_SALT_RECV              3
#define NTLS_SET_IV_SEND                4
#define NTLS_SET_KEY_SEND               5
#define NTLS_SET_SALT_SEND              6
#define NTLS_SET_MTU                    7
#define NTLS_UNATTACH                   8

/* setsockopt() optnames */
#define NTLS_GET_IV_RECV                11
#define NTLS_GET_KEY_RECV               12
#define NTLS_GET_SALT_RECV              13
#define NTLS_GET_IV_SEND                14
#define NTLS_GET_KEY_SEND               15
#define NTLS_GET_SALT_SEND              16
#define NTLS_GET_MTU                    17

/* Supported ciphers */
#define NTLS_CIPHER_AES_GCM_128         51

#define NTLS_VERSION_LATEST             0
#define NTLS_VERSION_1_2                1

/* Constants */
#define NTLS_AES_GCM_128_IV_SIZE        ((size_t)8)
#define NTLS_AES_GCM_128_KEY_SIZE       ((size_t)16)
#define NTLS_AES_GCM_128_SALT_SIZE      ((size_t)4)

/* Maximum data size carried in a TLS/DTLS record */
#define NTLS_MAX_PAYLOAD_SIZE           ((size_t)1 << 14)

struct sockaddr_ntls {
  __u16   sa_cipher;
  __u16   sa_socket;
  __u16   sa_version;
};


int ntls_attach(SSL *ssl, int transport_fd)
{
#if 1
  return 0;
#else
  int ret;

  int tfmfd = socket(AF_NTLS, SOCK_STREAM, 0);
  if (tfmfd == -1) {
    perror("socket error:");
    exit(-1);
  }

  struct sockaddr_ntls sa;


  sa.sa_cipher =  NTLS_CIPHER_AES_GCM_128;
  sa.sa_version = NTLS_VERSION_1_2;
  sa.sa_socket = transport_fd;

  if (bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    perror("AF_ALG: bind failed");
    close(tfmfd);
    exit(-1);
  }


  EVP_CIPHER_CTX * writeCtx = ssl->enc_write_ctx;
  EVP_CIPHER_CTX * readCtx = ssl->enc_read_ctx;

  EVP_AES_GCM_CTX* gcmWrite = (EVP_AES_GCM_CTX*)(writeCtx->cipher_data);
  EVP_AES_GCM_CTX* gcmRead = (EVP_AES_GCM_CTX*)(readCtx->cipher_data);

  unsigned char* writeKey = (unsigned char*)(gcmWrite->gcm.key);
  unsigned char* readKey = (unsigned char*)(gcmRead->gcm.key);

  unsigned char* writeIV = gcmWrite->iv;
  unsigned char* readIV = gcmRead->iv;

  unsigned char* readSeqNum = ssl->s3->read_sequence;
  unsigned char* writeSeqNum = ssl->s3->write_sequence;

  if (setsockopt(tfmfd, AF_NTLS, NTLS_SET_KEY_SEND, writeKey, 16)) {
    perror("AF_NTLS: set write key failed\n");
    exit(-1);
  }

  if (setsockopt(tfmfd, AF_NTLS, NTLS_SET_SALT_SEND, writeIV, 4)) {
    perror("AF_NTLS: set write salt failed\n");
    exit(-1);
  }

  if (setsockopt(tfmfd, AF_NTLS, NTLS_SET_IV_SEND, writeSeqNum, 8)) {
    perror("AF_NTLS: set write iv failed\n");
    exit(-1);
  }

  if (setsockopt(tfmfd, AF_NTLS, NTLS_SET_KEY_RECV, readKey, 16)) {
    perror("AF_NTLS: set read key failed\n");
    exit(-1);
  }

  if (setsockopt(tfmfd, AF_NTLS, NTLS_SET_SALT_RECV, readIV, 4)) {
    perror("AF_NTLS: set read salt failed\n");
    exit(-1);
  }

  if (setsockopt(tfmfd, AF_NTLS, NTLS_SET_IV_RECV, readSeqNum, 8)) {
    perror("AF_NTLS: set read iv failed\n");
    exit(-1);
  }
#endif
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

#endif /* #ifndef NTLS_H */
