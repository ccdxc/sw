#include "nic/include/fte_db.hpp"

namespace fte {

alg_entry_t *alloc_and_insert_alg_entry(alg_entry_t *entry);
void alg_completion_hdlr (fte::ctx_t& ctx, bool status);

}

namespace hal {
namespace net {

// Big-Endian util
inline uint32_t
__be_pack_uint32(const uint8_t *buf, uint8_t idx)
{
    int shift = 24;
    uint32_t val = 0;

    do {
       val = val | (buf[idx++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

inline uint16_t
__be_pack_uint16(const uint8_t *buf, uint8_t idx)
{
    int shift = 8;
    uint32_t val = 0;

    do {
       val = val | (buf[idx++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

//Little Endian util
inline uint32_t
__le_pack_uint32(const uint8_t *buf, uint8_t idx)
{
    int shift = 0;
    uint32_t val = 0;

    do {
       val = val | (buf[idx++]<<shift);
       shift += 8;
    } while (shift <= 24);

    return val;
}

inline uint32_t
__pack_uint32(const uint8_t *buf, uint8_t idx, uint8_t format=0)
{
    if (format == 1) {
        return (__le_pack_uint32(buf, idx));
    } else {
        return (__be_pack_uint32(buf, idx));
    }
}

inline uint16_t
__le_pack_uint16(const uint8_t *buf, uint8_t idx)
{
    int shift = 0;
    uint32_t val = 0;

    do {
       val = val | (buf[idx++]<<shift);
       shift += 8;
    } while (shift <= 8);

    return val;
}

inline uint16_t
__pack_uint16(const uint8_t *buf, uint8_t idx, uint8_t format=0)
{
    if (format == 1) {
        return (__le_pack_uint16(buf, idx));
    } else {
        return (__be_pack_uint16(buf, idx));
    }
}


void insert_rpc_entry(fte::ctx_t& ctx, fte::RPCMap *map);

}
}
