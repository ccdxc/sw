#ifndef __SDK_OPERD_DECODER_H__
#define __SDK_OPERD_DECODER_H__

#include <stddef.h>
#include <stdint.h>

extern "C" {

typedef enum  {
    OPERD_DECODER_PLAIN_TEXT = 0,
    OPERD_DECODER_ALERTS     = 1,
    OPERD_DECODER_VPP        = 51,
    OPERD_DECODER_MAX        = 255,
} operd_decoders;

typedef size_t (*decoder_fn)(uint8_t encoder, const char *data,
                             size_t data_length, char *output,
                             size_t output_size);

typedef int (*register_decoder_fn)(uint8_t encoder_id, decoder_fn fn);

typedef void (*decoder_lib_init_fn)(register_decoder_fn callback);

}

#endif
