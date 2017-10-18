/* 
 * ----------------------------------------------------------------------------
 *
 * dos_api.hpp
 *
 * DoS APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __DOS_API_HPP__
#define __DOS_API_HPP__


namespace hal {

using hal::dos_policy_t;

void dos_set_pd_dos(dos_policy_t *pi_nw, void *pd_nw);

} // namespace hal
#endif // __DOS_API_HPP__
