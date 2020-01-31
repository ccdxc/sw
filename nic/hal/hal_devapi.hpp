//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_DEVAPI_HPP__
#define __HAL_DEVAPI_HPP__

namespace hal {

class devapi_hal {
public:
    devapi_hal() {
        hal_up_ = false;
    }
    ~devapi_hal() {}
    bool hal_up(void) { return hal_up_; }
    void set_hal_up(bool status) { hal_up_ = status; }
private:
    bool hal_up_;
};

extern devapi_hal g_devapi_hal;

}    // namespace hal

using hal::g_devapi_hal;

#endif    // __HAL_DEVAPI_HPP__
