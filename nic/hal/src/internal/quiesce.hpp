#ifndef __QUIESCE_HPP__
#define __QUIESCE_HPP__

namespace hal {

hal_ret_t quiesce_start(void);

hal_ret_t quiesce_stop(void);
} /* hal */

#endif  /* __QUIESCE_HPP__ */
