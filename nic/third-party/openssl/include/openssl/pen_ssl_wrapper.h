#pragma once

#define TLS_method pen_TLS_method
#define TLS_client_method pen_TLS_client_method
#define TLS_server_method pen_TLS_server_method
#define SSL_CTX_new pen_SSL_CTX_new
#define ERR_load_SSL_strings pen_ERR_load_SSL_strings
#define OPENSSL_init_ssl pen_OPENSSL_init_ssl
#define OPENSSL_init_crypto pen_OPENSSL_init_crypto
#define ERR_load_BIO_strings pen_ERR_load_BIO_strings
#define SSL_CTX_set_options pen_SSL_CTX_set_options
#define SSL_CTX_set_cipher_list pen_SSL_CTX_set_cipher_list
#define SSL_CTX_set_info_callback pen_SSL_CTX_set_info_callback
#define SSL_CTX_set_msg_callback pen_SSL_CTX_set_msg_callback
#define SSL_state_string pen_SSL_state_string
#define SSL_state_string_long pen_SSL_state_string_long
#define SSL_new pen_SSL_new
#define BIO_new_bio_pair pen_BIO_new_bio_pair
#define SSL_set_bio pen_SSL_set_bio
#define SSL_set_connect_state pen_SSL_set_connect_state
#define SSL_do_handshake pen_SSL_do_handshake
#define SSL_get_error pen_SSL_get_error
#define ERR_error_string_n pen_ERR_error_string_n
#define BIO_ctrl_pending pen_BIO_ctrl_pending
#define BIO_read pen_BIO_read
#define BIO_write pen_BIO_write
#define BIO_test_flags pen_BIO_test_flags
#define SSL_in_init pen_SSL_in_init
#define SSL_free pen_SSL_free
#define BIO_free pen_BIO_free
#include "nic/third-party/openssl/include/openssl/ssl.h"
#include "nic/third-party/openssl/include/openssl/bio.h"
#include "nic/third-party/openssl/include/openssl/err.h"

#undef BIO_free
#undef SSL_free
#undef SSL_in_init
#undef BIO_test_flags
#undef BIO_read
#undef BIO_write
#undef BIO_ctrl_pending
#undef ERR_error_string_n
#undef SSL_get_error
#undef SSL_do_handshake
#undef SSL_set_connect_state
#undef SSL_set_bio
#undef BIO_new_bio_pair
#undef SSL_new
#undef SSL_state_string
#undef SSL_state_string_long
#undef SSL_CTX_set_msg_callback
#undef SSL_CTX_set_info_callback
#undef SSL_CTX_set_cipher_list
#undef SSL_CTX_set_options
#undef ERR_load_BIO_strings
#undef OPENSSL_init_crypto
#undef OPENSSL_init_ssl
#undef TLS_client_method
#undef TLS_server_method
#undef TLS_method
#undef SSL_CTX_new
#undef ERR_load_SSL_strings
