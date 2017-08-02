/*
 * capri_tm_rw.cc
 * Vasanth Kumar (Pensando Systems)
 */

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include <base.h>
#include <defines.h>
#include <capri_tm_rw.hpp>

#ifndef HAL_GTEST
#include <cap_blk_reg_model.h>
#include <cap_top_csr.h>
#include <cap_pbc_csr.h>
#endif

#define TM_PG_REG_GET(_c, _base_reg, _port, _pg) {   \
    {                                                \
        switch (_port) {                             \
        case 0:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_0_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_0_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_0_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_0_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_0_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_0_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_0_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_0_pg_7;  \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 1:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_1_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_1_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_1_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_1_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_1_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_1_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_1_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_1_pg_7;  \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 2:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_2_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_2_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_2_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_2_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_2_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_2_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_2_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_2_pg_7;  \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 3:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_3_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_3_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_3_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_3_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_3_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_3_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_3_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_3_pg_7;  \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 4:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_4_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_4_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_4_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_4_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_4_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_4_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_4_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_4_pg_7;  \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 5:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_5_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_5_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_5_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_5_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_5_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_5_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_5_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_5_pg_7;  \
                break;                               \
            case 8:                                  \
                _base_reg = &_c.cfg_account_5_pg_8;  \
                break;                               \
            case 9:                                  \
                _base_reg = &_c.cfg_account_5_pg_9;  \
                break;                               \
            case 10:                                 \
                _base_reg = &_c.cfg_account_5_pg_10; \
                break;                               \
            case 11:                                 \
                _base_reg = &_c.cfg_account_5_pg_11; \
                break;                               \
            case 12:                                 \
                _base_reg = &_c.cfg_account_5_pg_12; \
                break;                               \
            case 13:                                 \
                _base_reg = &_c.cfg_account_5_pg_13; \
                break;                               \
            case 14:                                 \
                _base_reg = &_c.cfg_account_5_pg_14; \
                break;                               \
            case 15:                                 \
                _base_reg = &_c.cfg_account_5_pg_15; \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 6:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_6_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_6_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_6_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_6_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_6_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_6_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_6_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_6_pg_7;  \
                break;                               \
            case 8:                                  \
                _base_reg = &_c.cfg_account_6_pg_8;  \
                break;                               \
            case 9:                                  \
                _base_reg = &_c.cfg_account_6_pg_9;  \
                break;                               \
            case 10:                                 \
                _base_reg = &_c.cfg_account_6_pg_10; \
                break;                               \
            case 11:                                 \
                _base_reg = &_c.cfg_account_6_pg_11; \
                break;                               \
            case 12:                                 \
                _base_reg = &_c.cfg_account_6_pg_12; \
                break;                               \
            case 13:                                 \
                _base_reg = &_c.cfg_account_6_pg_13; \
                break;                               \
            case 14:                                 \
                _base_reg = &_c.cfg_account_6_pg_14; \
                break;                               \
            case 15:                                 \
                _base_reg = &_c.cfg_account_6_pg_15; \
                break;                               \
            case 16:                                 \
                _base_reg = &_c.cfg_account_6_pg_16; \
                break;                               \
            case 17:                                 \
                _base_reg = &_c.cfg_account_6_pg_17; \
                break;                               \
            case 18:                                 \
                _base_reg = &_c.cfg_account_6_pg_18; \
                break;                               \
            case 19:                                 \
                _base_reg = &_c.cfg_account_6_pg_19; \
                break;                               \
            case 20:                                 \
                _base_reg = &_c.cfg_account_6_pg_20; \
                break;                               \
            case 21:                                 \
                _base_reg = &_c.cfg_account_6_pg_21; \
                break;                               \
            case 22:                                 \
                _base_reg = &_c.cfg_account_6_pg_22; \
                break;                               \
            case 23:                                 \
                _base_reg = &_c.cfg_account_6_pg_23; \
                break;                               \
            case 24:                                 \
                _base_reg = &_c.cfg_account_6_pg_24; \
                break;                               \
            case 25:                                 \
                _base_reg = &_c.cfg_account_6_pg_25; \
                break;                               \
            case 26:                                 \
                _base_reg = &_c.cfg_account_6_pg_26; \
                break;                               \
            case 27:                                 \
                _base_reg = &_c.cfg_account_6_pg_27; \
                break;                               \
            case 28:                                 \
                _base_reg = &_c.cfg_account_6_pg_28; \
                break;                               \
            case 29:                                 \
                _base_reg = &_c.cfg_account_6_pg_29; \
                break;                               \
            case 30:                                 \
                _base_reg = &_c.cfg_account_6_pg_30; \
                break;                               \
            case 31:                                 \
                _base_reg = &_c.cfg_account_6_pg_31; \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        case 7:                                      \
            switch (_pg) {                           \
            case 0:                                  \
                _base_reg = &_c.cfg_account_7_pg_0;  \
                break;                               \
            case 1:                                  \
                _base_reg = &_c.cfg_account_7_pg_1;  \
                break;                               \
            case 2:                                  \
                _base_reg = &_c.cfg_account_7_pg_2;  \
                break;                               \
            case 3:                                  \
                _base_reg = &_c.cfg_account_7_pg_3;  \
                break;                               \
            case 4:                                  \
                _base_reg = &_c.cfg_account_7_pg_4;  \
                break;                               \
            case 5:                                  \
                _base_reg = &_c.cfg_account_7_pg_5;  \
                break;                               \
            case 6:                                  \
                _base_reg = &_c.cfg_account_7_pg_6;  \
                break;                               \
            case 7:                                  \
                _base_reg = &_c.cfg_account_7_pg_7;  \
                break;                               \
            case 8:                                  \
                _base_reg = &_c.cfg_account_7_pg_8;  \
                break;                               \
            case 9:                                  \
                _base_reg = &_c.cfg_account_7_pg_9;  \
                break;                               \
            case 10:                                 \
                _base_reg = &_c.cfg_account_7_pg_10; \
                break;                               \
            case 11:                                 \
                _base_reg = &_c.cfg_account_7_pg_11; \
                break;                               \
            case 12:                                 \
                _base_reg = &_c.cfg_account_7_pg_12; \
                break;                               \
            case 13:                                 \
                _base_reg = &_c.cfg_account_7_pg_13; \
                break;                               \
            case 14:                                 \
                _base_reg = &_c.cfg_account_7_pg_14; \
                break;                               \
            case 15:                                 \
                _base_reg = &_c.cfg_account_7_pg_15; \
                break;                               \
            case 16:                                 \
                _base_reg = &_c.cfg_account_7_pg_16; \
                break;                               \
            case 17:                                 \
                _base_reg = &_c.cfg_account_7_pg_17; \
                break;                               \
            case 18:                                 \
                _base_reg = &_c.cfg_account_7_pg_18; \
                break;                               \
            case 19:                                 \
                _base_reg = &_c.cfg_account_7_pg_19; \
                break;                               \
            case 20:                                 \
                _base_reg = &_c.cfg_account_7_pg_20; \
                break;                               \
            case 21:                                 \
                _base_reg = &_c.cfg_account_7_pg_21; \
                break;                               \
            case 22:                                 \
                _base_reg = &_c.cfg_account_7_pg_22; \
                break;                               \
            case 23:                                 \
                _base_reg = &_c.cfg_account_7_pg_23; \
                break;                               \
            case 24:                                 \
                _base_reg = &_c.cfg_account_7_pg_24; \
                break;                               \
            case 25:                                 \
                _base_reg = &_c.cfg_account_7_pg_25; \
                break;                               \
            case 26:                                 \
                _base_reg = &_c.cfg_account_7_pg_26; \
                break;                               \
            case 27:                                 \
                _base_reg = &_c.cfg_account_7_pg_27; \
                break;                               \
            case 28:                                 \
                _base_reg = &_c.cfg_account_7_pg_28; \
                break;                               \
            case 29:                                 \
                _base_reg = &_c.cfg_account_7_pg_29; \
                break;                               \
            case 30:                                 \
                _base_reg = &_c.cfg_account_7_pg_30; \
                break;                               \
            case 31:                                 \
                _base_reg = &_c.cfg_account_7_pg_31; \
                break;                               \
            default:                                 \
                return HAL_RET_ERR;                  \
            }                                        \
            break;                                   \
        }                                            \
    }                                                \
}

#define TM_PG_REG_SET_FIELD(_c, _port, _pg, _reg_name, _value) { \
    {                                                            \
        switch (_port) {                                         \
        case 0:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_0_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_0_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_0_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_0_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_0_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_0_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_0_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_0_pg_7._reg_name(_value);         \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 1:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_1_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_1_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_1_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_1_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_1_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_1_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_1_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_1_pg_7._reg_name(_value);         \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 2:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_2_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_2_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_2_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_2_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_2_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_2_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_2_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_2_pg_7._reg_name(_value);         \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 3:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_3_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_3_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_3_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_3_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_3_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_3_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_3_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_3_pg_7._reg_name(_value);         \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 4:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_4_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_4_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_4_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_4_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_4_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_4_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_4_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_4_pg_7._reg_name(_value);         \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 5:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_5_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_5_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_5_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_5_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_5_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_5_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_5_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_5_pg_7._reg_name(_value);         \
                break;                                           \
            case 8:                                              \
                _c.cfg_account_5_pg_8._reg_name(_value);         \
                break;                                           \
            case 9:                                              \
                _c.cfg_account_5_pg_9._reg_name(_value);         \
                break;                                           \
            case 10:                                             \
                _c.cfg_account_5_pg_10._reg_name(_value);        \
                break;                                           \
            case 11:                                             \
                _c.cfg_account_5_pg_11._reg_name(_value);        \
                break;                                           \
            case 12:                                             \
                _c.cfg_account_5_pg_12._reg_name(_value);        \
                break;                                           \
            case 13:                                             \
                _c.cfg_account_5_pg_13._reg_name(_value);        \
                break;                                           \
            case 14:                                             \
                _c.cfg_account_5_pg_14._reg_name(_value);        \
                break;                                           \
            case 15:                                             \
                _c.cfg_account_5_pg_15._reg_name(_value);        \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 6:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_6_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_6_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_6_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_6_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_6_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_6_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_6_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_6_pg_7._reg_name(_value);         \
                break;                                           \
            case 8:                                              \
                _c.cfg_account_6_pg_8._reg_name(_value);         \
                break;                                           \
            case 9:                                              \
                _c.cfg_account_6_pg_9._reg_name(_value);         \
                break;                                           \
            case 10:                                             \
                _c.cfg_account_6_pg_10._reg_name(_value);        \
                break;                                           \
            case 11:                                             \
                _c.cfg_account_6_pg_11._reg_name(_value);        \
                break;                                           \
            case 12:                                             \
                _c.cfg_account_6_pg_12._reg_name(_value);        \
                break;                                           \
            case 13:                                             \
                _c.cfg_account_6_pg_13._reg_name(_value);        \
                break;                                           \
            case 14:                                             \
                _c.cfg_account_6_pg_14._reg_name(_value);        \
                break;                                           \
            case 15:                                             \
                _c.cfg_account_6_pg_15._reg_name(_value);        \
                break;                                           \
            case 16:                                             \
                _c.cfg_account_6_pg_16._reg_name(_value);        \
                break;                                           \
            case 17:                                             \
                _c.cfg_account_6_pg_17._reg_name(_value);        \
                break;                                           \
            case 18:                                             \
                _c.cfg_account_6_pg_18._reg_name(_value);        \
                break;                                           \
            case 19:                                             \
                _c.cfg_account_6_pg_19._reg_name(_value);        \
                break;                                           \
            case 20:                                             \
                _c.cfg_account_6_pg_20._reg_name(_value);        \
                break;                                           \
            case 21:                                             \
                _c.cfg_account_6_pg_21._reg_name(_value);        \
                break;                                           \
            case 22:                                             \
                _c.cfg_account_6_pg_22._reg_name(_value);        \
                break;                                           \
            case 23:                                             \
                _c.cfg_account_6_pg_23._reg_name(_value);        \
                break;                                           \
            case 24:                                             \
                _c.cfg_account_6_pg_24._reg_name(_value);        \
                break;                                           \
            case 25:                                             \
                _c.cfg_account_6_pg_25._reg_name(_value);        \
                break;                                           \
            case 26:                                             \
                _c.cfg_account_6_pg_26._reg_name(_value);        \
                break;                                           \
            case 27:                                             \
                _c.cfg_account_6_pg_27._reg_name(_value);        \
                break;                                           \
            case 28:                                             \
                _c.cfg_account_6_pg_28._reg_name(_value);        \
                break;                                           \
            case 29:                                             \
                _c.cfg_account_6_pg_29._reg_name(_value);        \
                break;                                           \
            case 30:                                             \
                _c.cfg_account_6_pg_30._reg_name(_value);        \
                break;                                           \
            case 31:                                             \
                _c.cfg_account_6_pg_31._reg_name(_value);        \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        case 7:                                                  \
            switch (_pg) {                                       \
            case 0:                                              \
                _c.cfg_account_7_pg_0._reg_name(_value);         \
                break;                                           \
            case 1:                                              \
                _c.cfg_account_7_pg_1._reg_name(_value);         \
                break;                                           \
            case 2:                                              \
                _c.cfg_account_7_pg_2._reg_name(_value);         \
                break;                                           \
            case 3:                                              \
                _c.cfg_account_7_pg_3._reg_name(_value);         \
                break;                                           \
            case 4:                                              \
                _c.cfg_account_7_pg_4._reg_name(_value);         \
                break;                                           \
            case 5:                                              \
                _c.cfg_account_7_pg_5._reg_name(_value);         \
                break;                                           \
            case 6:                                              \
                _c.cfg_account_7_pg_6._reg_name(_value);         \
                break;                                           \
            case 7:                                              \
                _c.cfg_account_7_pg_7._reg_name(_value);         \
                break;                                           \
            case 8:                                              \
                _c.cfg_account_7_pg_8._reg_name(_value);         \
                break;                                           \
            case 9:                                              \
                _c.cfg_account_7_pg_9._reg_name(_value);         \
                break;                                           \
            case 10:                                             \
                _c.cfg_account_7_pg_10._reg_name(_value);        \
                break;                                           \
            case 11:                                             \
                _c.cfg_account_7_pg_11._reg_name(_value);        \
                break;                                           \
            case 12:                                             \
                _c.cfg_account_7_pg_12._reg_name(_value);        \
                break;                                           \
            case 13:                                             \
                _c.cfg_account_7_pg_13._reg_name(_value);        \
                break;                                           \
            case 14:                                             \
                _c.cfg_account_7_pg_14._reg_name(_value);        \
                break;                                           \
            case 15:                                             \
                _c.cfg_account_7_pg_15._reg_name(_value);        \
                break;                                           \
            case 16:                                             \
                _c.cfg_account_7_pg_16._reg_name(_value);        \
                break;                                           \
            case 17:                                             \
                _c.cfg_account_7_pg_17._reg_name(_value);        \
                break;                                           \
            case 18:                                             \
                _c.cfg_account_7_pg_18._reg_name(_value);        \
                break;                                           \
            case 19:                                             \
                _c.cfg_account_7_pg_19._reg_name(_value);        \
                break;                                           \
            case 20:                                             \
                _c.cfg_account_7_pg_20._reg_name(_value);        \
                break;                                           \
            case 21:                                             \
                _c.cfg_account_7_pg_21._reg_name(_value);        \
                break;                                           \
            case 22:                                             \
                _c.cfg_account_7_pg_22._reg_name(_value);        \
                break;                                           \
            case 23:                                             \
                _c.cfg_account_7_pg_23._reg_name(_value);        \
                break;                                           \
            case 24:                                             \
                _c.cfg_account_7_pg_24._reg_name(_value);        \
                break;                                           \
            case 25:                                             \
                _c.cfg_account_7_pg_25._reg_name(_value);        \
                break;                                           \
            case 26:                                             \
                _c.cfg_account_7_pg_26._reg_name(_value);        \
                break;                                           \
            case 27:                                             \
                _c.cfg_account_7_pg_27._reg_name(_value);        \
                break;                                           \
            case 28:                                             \
                _c.cfg_account_7_pg_28._reg_name(_value);        \
                break;                                           \
            case 29:                                             \
                _c.cfg_account_7_pg_29._reg_name(_value);        \
                break;                                           \
            case 30:                                             \
                _c.cfg_account_7_pg_30._reg_name(_value);        \
                break;                                           \
            case 31:                                             \
                _c.cfg_account_7_pg_31._reg_name(_value);        \
                break;                                           \
            default:                                             \
                return HAL_RET_ERR;                              \
            }                                                    \
            break;                                               \
        }                                                        \
    }                                                            \
}

#define TM_PG_MTU_SET(_c, _port, _pg, _mtu) 
#define TM_PG_MTU_REG_GET(_c, _base_reg, _port, _pg) 

static bool is_valid_tm_uplink_port(uint32_t port) {
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_NCSI));
}

#if 0
static bool is_valid_tm_port(uint32_t port) {
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_NCSI));
}

hal_ret_t capri_tm_pg_params_update(uint32_t port,
                                    uint32_t pg,
                                    tm_pg_params_t *pg_params)
{
    /* Do some sanity checks for port and pg */
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_register_base *pg_csr = NULL;
    cap_register_base *mtu_csr = NULL;

    TM_PG_REG_SET_FIELD(pbc_csr, port, pg, reserved_min, pg_params->reserved_min);
    TM_PG_REG_SET_FIELD(pbc_csr, port, pg, xon_threshold, pg_params->xon_threshold);
    TM_PG_REG_SET_FIELD(pbc_csr, port, pg, headroom, pg_params->headroom);
    TM_PG_REG_SET_FIELD(pbc_csr, port, pg, low_limit, pg_params->low_limit);
    TM_PG_REG_SET_FIELD(pbc_csr, port, pg, alpha, pg_params->alpha);
    TM_PG_REG_GET(pbc_csr, pg_csr, port, pg);

    /* Update the MTU Table */

    TM_PG_MTU_SET(pbc_csr, port, pg, pg_params->mtu);
    TM_PG_MTU_REG_GET(pbc_csr, mtu_csr, port, pg);

    pg_csr->write();
    mtu_csr->write();
#endif
    return HAL_RET_OK;
}

hal_ret_t capri_tm_pg_cos_map_update(uint32_t port,
                                     uint32_t ncos_maps,
                                     tm_pg_cos_map_t *pg_cos_map) 
{
    /* Do some sanity checks for port, pg and cos values */
    cpp_int reg_val;
    tm_pg_cos_map_t *m;

    for (i = 0; i < ncos_maps; i++) {
        m = &pg_cos_map[i]; 

        reg_val |= (m->pg & (npgs-1)) << (m->cos * std::log2(ntcs));
    }

    cos_csr->write();
    return HAL_RET_OK;
}

hal_ret_t capri_tm_scheduler_map_update(uint32_t port,
                                        tm_scheduler_map_t *scheduler_map) 
{
    return HAL_RET_OK;
}
#endif

/* Program the lif value on an uplink port */
hal_ret_t capri_tm_uplink_lif_set(uint32_t port,
                                  uint32_t lif)
{
    if (!is_valid_tm_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM port", port);
        return HAL_RET_ERR;
    }

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    switch(port) {
        case TM_PORT_UPLINK_0:
            pbc_csr.cfg_src_port_to_lif_map.entry_0(lif);
            break;
        case TM_PORT_UPLINK_1:
            pbc_csr.cfg_src_port_to_lif_map.entry_1(lif);
            break;
        case TM_PORT_UPLINK_2:
            pbc_csr.cfg_src_port_to_lif_map.entry_2(lif);
            break;
        case TM_PORT_UPLINK_3:
            pbc_csr.cfg_src_port_to_lif_map.entry_3(lif);
            break;
        case TM_PORT_UPLINK_4:
            pbc_csr.cfg_src_port_to_lif_map.entry_4(lif);
            break;
        case TM_PORT_UPLINK_5:
            pbc_csr.cfg_src_port_to_lif_map.entry_5(lif);
            break;
        case TM_PORT_UPLINK_6:
            pbc_csr.cfg_src_port_to_lif_map.entry_6(lif);
            break;
        case TM_PORT_UPLINK_7:
            pbc_csr.cfg_src_port_to_lif_map.entry_7(lif);
            break;
        case TM_PORT_NCSI:
            pbc_csr.cfg_src_port_to_lif_map.entry_8(lif);
            break;
        default:
            return HAL_RET_ERR;
    }

    pbc_csr.cfg_src_port_to_lif_map.write();

    return HAL_RET_OK;
}
