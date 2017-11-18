#include <vector>
#include "dol/test/storage/xts.hpp"

#define kDefaultBufSize 4096

namespace xts {

/*
 * We need to initialize every field in struct bcos g++ does not support non-trivial designated initializers
 * Otherway is to use json file but we cannot use native enums and need a separate schema
 */

// TODO: Commented out test cases are currently failing - most likely a model issue. Need to verify with new model

std::vector<TestCtx> xts_tests = {
    // key_size, op1, stage_in_hbm, op2, num_sectors, num_mds, num_aols
    { AES128_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 1, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 1, 1, 1 },
    { AES128_KEY_SIZE, T10_ONLY, true, INVALID, 1, 1, 1 },
    { AES128_KEY_SIZE, T10_ONLY, false, INVALID, 1, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 1, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 1, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 1, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 1, 1, 1 },
    // num_aols = 2
    { AES128_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 1, 1, 2 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 1, 1, 2 },
    { AES128_KEY_SIZE, T10_ONLY, true, INVALID, 1, 1, 2 },
    { AES128_KEY_SIZE, T10_ONLY, false, INVALID, 1, 1, 2 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 1, 1, 2 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 1, 1, 2 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 1, 1, 2 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 1, 1, 2 },
    // num_mds = 2, num_aols = 4
    { AES128_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 1, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 1, 2, 4 },
    { AES128_KEY_SIZE, T10_ONLY, true, INVALID, 1, 2, 4 },
    { AES128_KEY_SIZE, T10_ONLY, false, INVALID, 1, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 1, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 1, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 1, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 1, 2, 4 },
    // num_sectors = 2
    { AES128_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 2, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 2, 1, 1 },
    { AES128_KEY_SIZE, T10_ONLY, true, INVALID, 2, 1, 1 },
    { AES128_KEY_SIZE, T10_ONLY, false, INVALID, 2, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 2, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 2, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 2, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 2, 1, 1 },
    // num_sectors = 4, num_mds = 2, num_aols = 4
    { AES128_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 4, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 4, 2, 4 },
    { AES128_KEY_SIZE, T10_ONLY, true, INVALID, 4, 2, 4 },
    { AES128_KEY_SIZE, T10_ONLY, false, INVALID, 4, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 4, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 4, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 4, 2, 4 },
    { AES128_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 4, 2, 4 },

    //Misc cases
    { AES128_KEY_SIZE, T10_ONLY, false, INVALID, 4, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, kDefaultBufSize/SECTOR_SIZE, 1, 1 },
    { AES128_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, kDefaultBufSize/SECTOR_SIZE, 1, 1 },

    // Key_size 256 bits
    { AES256_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 1, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 1, 1, 1 },
    { AES256_KEY_SIZE, T10_ONLY, true, INVALID, 1, 1, 1 },
    { AES256_KEY_SIZE, T10_ONLY, false, INVALID, 1, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 1, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 1, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 1, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 1, 1, 1 },
    // num_aols = 2
    { AES256_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 1, 1, 2 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 1, 1, 2 },
    { AES256_KEY_SIZE, T10_ONLY, true, INVALID, 1, 1, 2 },
    { AES256_KEY_SIZE, T10_ONLY, false, INVALID, 1, 1, 2 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 1, 1, 2 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 1, 1, 2 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 1, 1, 2 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 1, 1, 2 },
    // num_mds = 2, num_aols = 4
    { AES256_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 1, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 1, 2, 4 },
    { AES256_KEY_SIZE, T10_ONLY, true, INVALID, 1, 2, 4 },
    { AES256_KEY_SIZE, T10_ONLY, false, INVALID, 1, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 1, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 1, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 1, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 1, 2, 4 },
    // num_sectors = 2
    { AES256_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 2, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 2, 1, 1 },
    { AES256_KEY_SIZE, T10_ONLY, true, INVALID, 2, 1, 1 },
    { AES256_KEY_SIZE, T10_ONLY, false, INVALID, 2, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 2, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 2, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 2, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 2, 1, 1 },
    // num_sectors = 4, num_mds = 2, num_aols = 4
    { AES256_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, 4, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, 4, 2, 4 },
    { AES256_KEY_SIZE, T10_ONLY, true, INVALID, 4, 2, 4 },
    { AES256_KEY_SIZE, T10_ONLY, false, INVALID, 4, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, INVALID, 4, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, INVALID, 4, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, true, AES_DECR_N_T10, 4, 2, 4 },
    { AES256_KEY_SIZE, AES_ENCR_N_T10, false, AES_DECR_N_T10, 4, 2, 4 },

    //Misc cases
    { AES256_KEY_SIZE, T10_ONLY, false, INVALID, 4, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, true, AES_DECR_ONLY, kDefaultBufSize/SECTOR_SIZE, 1, 1 },
    { AES256_KEY_SIZE, AES_ENCR_ONLY, false, AES_DECR_ONLY, kDefaultBufSize/SECTOR_SIZE, 1, 1 },

};


}
