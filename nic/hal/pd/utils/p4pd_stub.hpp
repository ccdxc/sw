#ifndef __STUB_HPP__
#define __STUB_HPP__

#include <base.h>

#define p4pd_entry_write    p4pd_stub_entry_write
#define p4pd_entry_read     p4pd_stub_entry_read
#define p4pd_entry_create   p4pd_stub_entry_create

static inline int p4pd_stub_entry_write(uint32_t tableid, uint32_t index, uint8_t *hwkey,
                                        uint8_t *hwkey_y, void *actiondata)
{
    return 0;
}

static inline int p4pd_stub_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                                       void *swkey_mask, void *actiondata)
{
    return 0;
}

static inline int p4pd_stub_entry_create(uint32_t tableid, void *swkey, void *swkey_mask,
                                         void *actiondata, uint8_t *hwentry)
{
    return 0;
}

#endif
