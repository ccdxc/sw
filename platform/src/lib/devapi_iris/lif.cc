//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "lif.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "vrf.hpp"
#include "enic.hpp"
#include "uplink.hpp"
#include "l2seg.hpp"
#include "filter.hpp"
#include "devapi_iris_types.hpp"
#include "devapi_mem.hpp"
#include "devapi_iris.hpp"
#include "hal_grpc.hpp"

namespace iris {

typedef std::map<uint32_t, devapi_lif*> lif_map_t;

devapi_lif *devapi_lif::internal_mgmt_ethlif = NULL;
lif_map_t devapi_lif::lif_db_;

devapi_lif *
devapi_lif::factory(lif_info_t *info, devapi_iris *dapi)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_lif *lif = NULL;
    devapi_l2seg    *native_l2seg = NULL;
    devapi_vrf      *vrf = NULL;
    devapi_enic     *enic = NULL;

    api_trace("lif create");
    NIC_LOG_DEBUG("lif_id: {}", info->lif_id);

    if (lif_db_.find(info->lif_id) != lif_db_.end()) {
        NIC_LOG_WARN("Dliflicate Create of lif with id: {}",
                     info->lif_id);
        return NULL;
    }

    mem = (devapi_lif *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_LIF,
                                  sizeof(devapi_lif));
    if (!mem) {
        NIC_LOG_ERR("lif create: Unable to allocate memory.");
        goto end;
    }

    lif = new (mem) devapi_lif();
    ret = lif->init_(info);
    if (ret != SDK_RET_OK) {
        lif->~devapi_lif();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_LIF, mem);
        lif = NULL;
        goto end;
    }
    ret = lif->lif_halcreate();
    if (ret != SDK_RET_OK) {
        NIC_LOG_DEBUG("Returning 1");
        goto end;
    }
    NIC_LOG_DEBUG("After hal create ...");

#if 0
    if (lif->is_intmgmtmnic()) {
        NIC_LOG_DEBUG("lif-{}:Setting Internal management lif",
                      info->lif_id);
        devapi_lif::set_intmgmt_lif(lif);
    }
#endif

    NIC_LOG_DEBUG("Adding lif to db for id: {}", lif->get_id());
    // Store in DB for disruptive upgrade
    lif_db_[lif->get_id()] = lif;

    if (lif->get_uplink() == NULL) {
        // HW: Admin devapi_lif.
        // DOL: HAL_LIF_ID_NICMGR_MIN lifs created in DevMgr constructor
        if (!lif->is_intmgmt()) {
            // All LIFs with uplink NULL except internal mgmt mnic and host mgmt.
            NIC_LOG_DEBUG("lif-{}: devapi_uplink is NULL", lif->get_id());
            goto end;
        }
    }

    // Skipping for Internal mgmt mnic
    if (lif->get_uplink()) {
        lif->get_uplink()->inc_num_lifs();
    }

    if (lif->is_intmgmt()) {
        // For Internal Mgmt mnic, create vrf and native l2seg.
        if (dapi->num_int_mgmt_mnics() == 0) {

            NIC_LOG_DEBUG("lif-{}:Setting Internal management lif, type: {}",
                          info->lif_id, info->type);
            devapi_lif::set_intmgmt_lif(lif);
            // Create Internal Mgmt Vrf
            vrf = devapi_vrf::factory(types::VRF_TYPE_INTERNAL_MANAGEMENT,
                                      NULL);
            lif->set_vrf(vrf);
            // Create native l2seg to hal
            native_l2seg = devapi_l2seg::factory(lif->get_vrf(),
                                                 NATIVE_VLAN_ID);
            if (!native_l2seg) {
                NIC_LOG_ERR("Failed to create native l2seg.");
                ret = SDK_RET_ERR;
                goto end;
            }
            lif->set_nativel2seg(native_l2seg);
        }
        dapi->inc_num_int_mgmt_mnics();
    } else {
#if 0
        // Skip native vlan for SWM lif
        if (lif->get_uplink()->get_num_lifs() == 1 &&
            info->type != sdk::platform::LIF_TYPE_SWM)
#endif
        if (lif->get_uplink()->get_num_lifs() == 1) {

            NIC_LOG_INFO("First lif id: {},on uplink {}, vrf: {}",
                         lif->get_id(),
                         lif->get_uplink()->get_id(),
                         lif->get_uplink()->get_vrf()->get_id());

            // Create native l2seg to hal
            native_l2seg = devapi_l2seg::lookup(lif->get_uplink()->get_vrf(),
                                                NATIVE_VLAN_ID);
            if (!native_l2seg) {
                native_l2seg = devapi_l2seg::factory(lif->get_uplink()->get_vrf(),
                                                     NATIVE_VLAN_ID);
                if (!native_l2seg) {
                    NIC_LOG_ERR("Failed to create native l2seg.");
                    ret = SDK_RET_ERR;
                    goto end;
                }

                // Update uplink structure with native l2seg
                lif->get_uplink()->set_native_l2seg(native_l2seg);

                // Update native_l2seg on uplink to hal
                ret = lif->get_uplink()->update_hal_native_l2seg(native_l2seg->get_id());
                if (ret != SDK_RET_OK) {
                    NIC_LOG_ERR("Failed to update uplink with native l2seg. "
                                "ret: {}", ret);
                    ret = SDK_RET_ERR;
                    goto end;
                }

                // Update l2seg's mbrifs on uplink
                // native_l2seg->Adddevapi_uplink(lif->get_uplink());
            } else {
                NIC_LOG_INFO("Native l2seg: {}", native_l2seg->get_id());
            }
        }
    }

    // Create devapi_enic
    enic = devapi_enic::factory(lif);
    if (!enic) {
        NIC_LOG_ERR("Failed to create enic.");
        ret = SDK_RET_ERR;
        goto end;
    }
    lif->set_enic(enic);

    lif->add_vlan(NATIVE_VLAN_ID);

    // No need as we are not supporting filters
    // to hal in smart
#if 0
    if (!lif->is_classicfwd()) {
        // If its promiscuos. send (devapi_lif, *, *) filter to HAL
        if (info->receive_promiscuous) {
            lif->create_macvlan_filter(0, 0);
        }
    }
#endif

end:
    if (ret != SDK_RET_OK) {
        lif->~devapi_lif();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_LIF, mem);
        lif = NULL;
    }
    return lif;
}

sdk_ret_t
devapi_lif::init_(lif_info_t *info)
{
    memcpy(&info_, info, sizeof(lif_info_t));
    vrf_ = NULL;
    enic_ = NULL;
    native_l2seg_ = NULL;

    return SDK_RET_OK;
}

devapi_lif *
devapi_lif::lookup(uint32_t lif_id)
{
    if (lif_db_.find(lif_id) != lif_db_.end()) {
        return lif_db_[lif_id];
    }
    return NULL;
}

sdk_ret_t
devapi_lif::reset(void)
{
    NIC_LOG_DEBUG("Resetting lif: {}", get_id());

    // Remove Filters
    remove_macfilters(false, true);
    remove_vlanfilters(true /* skip_native_vlan */);
    remove_macvlanfilters();

    // Reset Vlan Offload
    upd_vlanoff(
        false /* Vlan Strip */,
        false /* Vlan Insert */
    );

    // Reset Rx Modes
    upd_rxmode(
        false /* Broadcast */,
        false /* All Multicast */,
        false /* Promiscuous */
    );
    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::get_max_filters(uint32_t *ucast_filters,
                            uint32_t *mcast_filters)
{
    *ucast_filters = *mcast_filters = MAX_FILTERS;
    return SDK_RET_OK;
}

void
devapi_lif::destroy(devapi_lif *lif, devapi_iris *dapi)
{
    api_trace("lif delete");

    // Remove from DB
    lif_db_.erase(lif->get_id());

    lif->remove_macfilters(false, true);
    lif->remove_vlanfilters();
    lif->remove_macvlanfilters();

    if (lif->get_uplink()) {
        lif->get_uplink()->dec_num_lifs();
    }

    // if (lif->is_classicfwd())
    if (true) {
        // Delete enic
        if (lif->get_enic()) {
            devapi_enic::destroy(lif->get_enic());
        }

        if (lif->is_intmgmt()) {
            // if (lif->is_intmgmtmnic()) {
            if (dapi->num_int_mgmt_mnics() == 1) {
                // Delete L2seg
                devapi_l2seg::destroy(lif->get_nativel2seg());
                lif->set_nativel2seg(NULL);
                // Delete Vrf
                devapi_vrf::destroy(lif->get_vrf());
                lif->set_vrf(NULL);

                devapi_lif::set_intmgmt_lif(NULL);
            }
            dapi->dec_num_int_mgmt_mnics();
        } else {
            /*
             * SWM check is for l2seg for OOB to not get deleted when OOB lif
             * is being removed.
             * It will be deleted from swm.cc
             */
            if (lif->get_uplink() && (lif->get_uplink()->get_num_lifs() == 0) &&
                (!(lif->get_uplink()->get_native_l2seg() &&
                 lif->get_uplink()->get_native_l2seg()->is_single_wire_mgmt()))) {
                NIC_LOG_DEBUG("Last lif id: {}, hw_id: {} on uplink {}",
                              lif->get_id(),
                              lif->get_id(),
                              lif->get_uplink()->get_id());

                lif->get_uplink()->get_native_l2seg()->
                    del_uplink(lif->get_uplink());

                // Update native_l2seg on uplink to hal
                lif->get_uplink()->update_hal_native_l2seg(0);

                // Delete L2seg
                devapi_l2seg::destroy(lif->get_uplink()->get_native_l2seg());

                lif->get_uplink()->set_native_l2seg(NULL);
            }
        }
    } else {
        // Never reach here
        if (lif->get_isprom()) {
            lif->delete_macvlan_filter(0, 0);
        }
    }
    lif->~devapi_lif();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_LIF, lif);
}

devapi_lif::~devapi_lif()
{
    mac_table_.clear();
    vlan_table_.clear();
    mac_vlan_table_.clear();
    mac_vlan_filter_table_.clear();

    // Delete devapi_lif
    // devapi_lif::destroy(lif_);
    lif_haldelete();
}

/**
 * @re_add: Mac is already added without any registration. So re-adding.
 *          Happens for ALL_MC filter.
 */
sdk_ret_t
devapi_lif::add_mac(mac_t mac, bool re_add)
{
    sdk_ret_t ret = SDK_RET_OK;
    mac_vlan_t      mac_vlan;
    bool            skip_registration = false;

    api_trace("Adding Mac Filter");
    NIC_LOG_DEBUG("Mac filter: {}", macaddr2str(mac));

    // re_add
    // - True: Has to exist and should not be counted against max limit
    // - False: Should not exists and should be counted against max limit
    if ((re_add && mac_table_.find(mac) != mac_table_.end()) ||
        mac_table_.find(mac) == mac_table_.end()) {

        // Check if max limit reached
        if (!re_add && mac_table_.size() == info_.max_mac_filters) {
            NIC_LOG_ERR("Reached Max Mac filter limit of {} for lif: {}",
                          info_.max_mac_filters,
                          get_id());
            return sdk::SDK_RET_NO_RESOURCE;
        }

        /*
         * Classic:
         *      - Walk through Vlans and create (Mac,Vlan) filters
         * Smart:
         *      - Create Mac filter
         */
        // if (is_classicfwd())
        if (true) {
            if (is_multicast(mac) && is_recallmc()) {
                skip_registration = true;
            }
            // In micro-seg mode, host lifs will be prom. So no need to install
            // mac filters. For own mac we install as l2seg have to be creaetd.
            if (!is_multicast(mac) && hal->get_micro_seg_en() &&
                mac != MAC_TO_UINT64(info_.mac)) {
                NIC_LOG_DEBUG("Skipping ucast mac registration for "
                                "non-native mac in micro-seg");
                skip_registration = true;
            }

            if (!skip_registration) {
                // Register new mac across all existing vlans
                for (auto vlan_it = vlan_table_.cbegin(); vlan_it != vlan_table_.cend(); vlan_it++) {
                    // Check if (MacVlan) filter is already present
                    mac_vlan = std::make_tuple(mac, *vlan_it);
                    if (mac_vlan_table_.find(mac_vlan) == mac_vlan_table_.end()) {
                        // No (MacVlan) filter. Creating (Mac, Vlan)
                        ret = create_macvlan_filter(mac, *vlan_it);
                        if (ret != SDK_RET_OK) {
                            NIC_LOG_ERR("lif:{}: Adding mac:{} failed for vlan: {}. Cleaning up this mac.",
                                        get_id(), macaddr2str(mac), *vlan_it);
                            del_mac(mac, false /* update_db */, true /* add failure */);
                            goto end;
                        }
                    } else {
                        NIC_LOG_DEBUG("(Mac,Vlan) filter present. No-op");
                    }
                }
            }
        } else {
            // Smart mode: Not sending filters at all
            // create_mac_filter(mac);
        }

        if (!re_add) {
            // Store mac filter
            mac_table_.insert(mac);
        }
    } else {
        NIC_LOG_WARN("Mac already registered: {}", macaddr2str(mac));
    }

end:
    return ret;
}

sdk_ret_t
devapi_lif::del_mac(mac_t mac, bool update_db, bool add_failure)
{
    mac_vlan_t mac_vlan_key, mac_key, vlan_key;
    bool skipped_registration = false;

    api_trace("Deleting Mac Filter");
    NIC_LOG_DEBUG("Deleting Mac filter: {}", macaddr2str(mac));

    mac_key = std::make_tuple(mac, 0);
    if (add_failure || mac_table_.find(mac) != mac_table_.end()) {
        // if (is_classicfwd())
        if (true) {
            if (is_multicast(mac)) {
                if (is_recallmc()) {
                    skipped_registration = true;
                }
            }

            if (!skipped_registration) {
                for (auto vlan_it = vlan_table_.cbegin(); vlan_it != vlan_table_.cend(); vlan_it++) {
                    vlan_key = std::make_tuple(0, *vlan_it);
                    mac_vlan_key = std::make_tuple(mac, *vlan_it);
                    if (vlan_table_.find(*vlan_it) != vlan_table_.end() &&
                        mac_vlan_table_.find(mac_vlan_key) == mac_vlan_table_.end()) {
                        NIC_LOG_DEBUG("Mac Delete: Mac, Vlan are present but (Mac,Vlan) is not. Remove (Mac,Vlan) entity");
                        // Mac, Vlan are present and (Mac,Vlan) is not
                        delete_macvlan_filter(mac, *vlan_it);
                    } else {
                        // Case:
                        //  Case 1: Vlan filter not present but (Mac,Vlan) is either present or not.
                        //  Case 2: Vlan filter is present along with (Mac,Vlan)
                        NIC_LOG_DEBUG("Mac Delete: No-op");
                    }
                }
            }
        } else {
            // Smart mode: Not sending filters at all
            // delete_mac_filter(mac);
        }

        if (update_db) {
            // Erase mac filter
            mac_table_.erase(mac);
        }
    } else {
        NIC_LOG_ERR("Mac not registered: {}", mac);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::add_vlan(vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    mac_vlan_t      mac_vlan;

    api_trace("Adding Vlan Filter");
    NIC_LOG_DEBUG("Adding Vlan filter: {}", vlan);
    if (!vlan) {
        vlan = 8192;
    }
    if (!is_classicfwd(vlan)) {
        // Not classic: Dont even store the vlan filter.
        NIC_LOG_DEBUG("Lif: {}, Vlan: {} is not classic. Skipping vlan add",
                      get_id(), vlan);
        return ret;
    }

    if (vlan_table_.find(vlan) == vlan_table_.end()) {
        // Check if max limit reached
        if (vlan_table_.size() == info_.max_vlan_filters) {
            NIC_LOG_ERR("Reached Max Vlan filter limit of {} for lif: {}",
                          info_.max_vlan_filters,
                          get_id());
            return sdk::SDK_RET_NO_RESOURCE;
        }

        /*
         * Classic:
         *      - Walk through Vlans and create (Mac,Vlan) filters
         * Smart:
         *      - Create Vlan filter
         */
        if (is_classicfwd(vlan)) {
            // Register new mac across all existing vlans
            for (auto it = mac_table_.cbegin(); it != mac_table_.cend(); it++) {
                if (is_multicast(*it) && is_recallmc()) {
                    // skip Mcast macs if ALL_MC is set
                    continue;
                }
                // Check if (MacVlan) filter is already present
                mac_vlan = std::make_tuple(*it, vlan);
                if (mac_vlan_table_.find(mac_vlan) == mac_vlan_table_.end()) {
                    // No (MacVlan) filter. Creating (Mac, Vlan)
                    ret = create_macvlan_filter(*it, vlan);
                    if (ret != SDK_RET_OK) {
                        NIC_LOG_ERR("lif:{}: Adding vlan:{} failed for mac: {}. Cleaning up this vlan.",
                                    get_id(), vlan, macaddr2str(*it));
                        del_vlan(vlan, false /* update_db */, true /* add failure */);
                        goto end;
                    }
                } else {
                    NIC_LOG_DEBUG("(Mac,Vlan) filter present. No-op");
                }
            }
        } else {
            // Wont even reach here
            // create_vlan_filter(vlan);
        }

        // Store vlan filter
        vlan_table_.insert(vlan);
    } else {
        NIC_LOG_WARN("Vlan already registered: {}", vlan);
    }
end:
    return ret;
}

sdk_ret_t
devapi_lif::del_vlan(vlan_t vlan, bool update_db, bool add_failure)
{
    mac_vlan_t mac_vlan_key;

    api_trace("Deleting Vlan Filter");
    NIC_LOG_DEBUG("Deleting Vlan filter: {}", vlan);
    if (!vlan) {
        NIC_LOG_DEBUG("Ignoring Delete of Vlan filter 0");
        return SDK_RET_OK;
    }

    if (!is_classicfwd(vlan)) {
        // Not classic: Dont even store the vlan filter.
        NIC_LOG_DEBUG("Lif: {}, Vlan: {} is not classic. Skipping vlan del",
                      get_id(), vlan);
        return SDK_RET_OK;
    }

    if (add_failure || vlan_table_.find(vlan) != vlan_table_.end()) {
        if (is_classicfwd(vlan)) {
            for (auto it = mac_table_.cbegin(); it != mac_table_.cend(); it++) {
                if (is_multicast(*it) && is_recallmc()) {
                    // skip Mcast macs if ALL_MC is set
                    continue;
                }
                mac_vlan_key = std::make_tuple(*it, vlan);
                if (mac_table_.find(*it) != mac_table_.end() &&
                    mac_vlan_table_.find(mac_vlan_key) == mac_vlan_table_.end()) {
                    NIC_LOG_DEBUG("Vlan Delete: Mac, Vlan are present but (Mac,Vlan) is not. Remove (Mac,Vlan) entity");
                    // Mac, Vlan are present and (Mac,Vlan) is not
                    delete_macvlan_filter(*it, vlan);
                } else {
                    // Case:
                    //  Case 1: Mac filter not present but (Mac,Vlan) is either present or not.
                    //  Case 2: Mac filter is present along with (Mac,Vlan)
                    NIC_LOG_DEBUG("Vlan Delete: No-op");
                }
            }
        } else {
            // Wont even reach here
            // delete_vlan_filter(vlan);
        }

        if (update_db) {
            // Erase vlan filter
            vlan_table_.erase(vlan);
        }
    } else {
        NIC_LOG_ERR("Vlan not registered: {}", vlan);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::add_macvlan(mac_t mac, vlan_t vlan)
{
    sdk_ret_t ret = SDK_RET_OK;
    mac_vlan_t      key(mac, vlan);

    api_trace("Adding (Mac,Vlan) Filter");
    NIC_LOG_DEBUG("Adding (Mac,Vlan) mac: {}, filter: {}", macaddr2str(mac), vlan);

    if (!vlan) {
        vlan = 8192;
    }

    if (!is_classicfwd(vlan)) {
        // Not classic: Dont even store the vlan filter.
        return ret;
    }

    if (mac_vlan_table_.find(key) == mac_vlan_table_.end()) {
        // Check if max limit reached
        if (mac_vlan_table_.size() == info_.max_mac_vlan_filters) {
            NIC_LOG_ERR("Reached Max Mac-Vlan filter limit of {} for lif: {}",
                          info_.max_mac_vlan_filters,
                          get_id());
            return sdk::SDK_RET_NO_RESOURCE;
        }
        if (is_classicfwd(vlan)) {
            // Check if mac filter and vlan filter is present
            if (mac_table_.find(mac) == mac_table_.end() ||
                vlan_table_.find(vlan) == vlan_table_.end()) {
                ret = create_macvlan_filter(mac, vlan);
                if (ret != SDK_RET_OK) {
                    NIC_LOG_ERR("lif:{}: Adding mac:{} vlan: {} failed Cleaning up this (mac,vlan).",
                                get_id(), macaddr2str(mac), vlan);
                    del_macvlan(mac, vlan, false /* update_db */, true /* add failure */);
                    goto end;
                }
            } else {
                NIC_LOG_DEBUG("Mac filter and Vlan filter preset. "
                                "No-op for (Mac,Vlan) filter");
            }
        } else {
            // Wont come here
            // create_macvlan_filter(mac, vlan);
        }

        // Store mac-vlan filter
        mac_vlan_table_.insert(key);
    } else {
        NIC_LOG_WARN("Mac-Vlan already registered: {}", mac);
    }
end:
    return ret;
}

sdk_ret_t
devapi_lif::del_macvlan(mac_t mac, vlan_t vlan, bool update_db, bool add_failure)
{
    mac_vlan_t mac_vlan_key;

    api_trace("Deleting (Mac,Vlan) Filter");
    NIC_LOG_DEBUG("Deleting (Mac,Vlan) mac: {}, filter: {}", macaddr2str(mac), vlan);

    if (!vlan) {
        vlan = 8192;
    }

    if (!is_classicfwd(vlan)) {
        // Not classic: Dont even store the vlan filter.
        return SDK_RET_OK;
    }

    mac_vlan_key = std::make_tuple(mac, vlan);
    if (add_failure || mac_vlan_table_.find(mac_vlan_key) != mac_vlan_table_.end()) {
        if (is_classicfwd(vlan)) {
            if (mac_table_.find(mac) == mac_table_.end() ||
                 vlan_table_.find(vlan) == vlan_table_.end()) {
                // One of Mac or Vlan is not present.
                // (Mac,Vlan) entity was created only by (Mac,Vlan) filter
                delete_macvlan_filter(mac, vlan);
            } else {
                // Mac filter and Vlan filter both exist
                NIC_LOG_DEBUG("Mac filter and Vlan filter present. "
                                "No-op for (Mac,Vlan) filter");
            }
        } else {
            // Wont come here
            // delete_macvlan_filter(mac, vlan);
        }
        if (update_db) {
            // Erase mac-vlan filter
            mac_vlan_table_.erase(mac_vlan_key);
        }
    } else {
        NIC_LOG_ERR("(Mac,Vlan) not registered: mac: {}, vlan: {}",
                      mac, vlan);
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::upd_rx_en(bool rx_en)
{
    sdk_ret_t status = SDK_RET_OK;

    if (rx_en == info_.rx_en) {
        NIC_LOG_WARN("Rx en: {}. No change in rx enable. Nop", rx_en);
        return SDK_RET_OK;
    }

    info_.rx_en = rx_en;
    status = lif_halupdate();

    return status;
}

sdk_ret_t
devapi_lif::upd_rxmode(bool broadcast, bool all_multicast, bool promiscuous)
{
    sdk_ret_t status = SDK_RET_OK;

    if (broadcast != info_.receive_broadcast ||
        all_multicast != info_.receive_all_multicast ||
        promiscuous != info_.receive_promiscuous) {

        status = update_recprom(promiscuous);
        if (status != SDK_RET_OK) {
            return status;
        }

        status = update_recallmc(all_multicast);
        if (status != SDK_RET_OK) {
            return status;
        }

        status = update_recbcast(broadcast);
        if (status != SDK_RET_OK) {
            return status;
        }

        status = lif_halupdate();

    } else {
        NIC_LOG_DEBUG("devapi_lif: {}. No rx mode change.", get_id());
    }

    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::update_recprom(bool receive_promiscuous)
{
    if (receive_promiscuous == info_.receive_promiscuous) {
        NIC_LOG_WARN("Prom flag: {}. No change in promiscuous flag. Nop",
                       receive_promiscuous);
        return SDK_RET_OK;
    }

    NIC_LOG_DEBUG("devapi_lif: {}. Prom. flag change {} -> {}",
                  get_id(), info_.receive_promiscuous,
                  receive_promiscuous);

    info_.receive_promiscuous = receive_promiscuous;

    // For smart, hal is not supporting filters
#if 0
    if (is_classicfwd()) {
    } else {
        if (receive_promiscuous) {
            create_macvlan_filter(0, 0);
        } else {
            delete_macvlan_filter(0, 0);
        }
    }
#endif

    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::update_recbcast(bool receive_broadcast)
{
    sdk_ret_t status = SDK_RET_OK;

    if (receive_broadcast == info_.receive_broadcast) {
        NIC_LOG_WARN("Bcast flag: {}. No change in broadcast flag. Nop",
                       receive_broadcast);
        return SDK_RET_OK;
    }

    NIC_LOG_DEBUG("devapi_lif: {}. Bcast. flag change {} -> {}",
                    get_id(), info_.receive_broadcast,
                    receive_broadcast);

    info_.receive_broadcast = receive_broadcast;

    // Update devapi_lif to Hal
    status = lif_halupdate();

    return status;
}

sdk_ret_t
devapi_lif::update_recallmc(bool receive_all_multicast)
{
    sdk_ret_t status = SDK_RET_OK;

    if (receive_all_multicast == info_.receive_all_multicast) {
        NIC_LOG_WARN("ALL_MC flag: {}. No change in all_multicast flag. Nop",
                       receive_all_multicast);
        return SDK_RET_OK;
    }

    NIC_LOG_DEBUG("devapi_lif: {}. ALL_MC flag change {} -> {}",
                  get_id(), info_.receive_all_multicast,
                  receive_all_multicast);

    /*
     * False -> True
     * - False: Call deprogram_mcfilters
     * True -> False
     * - False: Call program_mcfilters
     */

    if (receive_all_multicast) {
        // Enable ALL_MC: Remove all MC filters on this lif
        deprogram_mcfilters();
        info_.receive_all_multicast = receive_all_multicast;
    } else {
        // Disable ALL_MC: Add all MC filters on this lif
        info_.receive_all_multicast = receive_all_multicast;
        program_mcfilters();
    }

    // Update devapi_lif to Hal
    status = lif_halupdate();

    return status;
}

sdk_ret_t
devapi_lif::upd_vlanoff(bool vlan_strip, bool vlan_insert)
{
    sdk_ret_t status = SDK_RET_OK;

    if (vlan_strip != info_.vlan_strip_en ||
        vlan_insert != info_.vlan_insert_en) {

        status = update_vlanstrip(vlan_strip);
        if (status != SDK_RET_OK) {
            return status;
        }

        status = update_vlanins(vlan_insert);
        if (status != SDK_RET_OK) {
            return status;
        }

        status = lif_halupdate();

    } else {
        NIC_LOG_DEBUG("devapi_lif: {}. No vlan offload change.", get_id());
    }

    return status;
}

sdk_ret_t
devapi_lif::update_vlanstrip(bool vlan_strip_en)
{
    api_trace("Vlan Strip change");
    if (vlan_strip_en == info_.vlan_strip_en) {
        NIC_LOG_WARN("No change in Vlan strip. Nop",
                       vlan_strip_en);
        return SDK_RET_OK;
    }

    NIC_LOG_DEBUG("devapi_lif: {}. Vlan strip change {} -> {}",
                    get_id(), info_.vlan_strip_en,
                    vlan_strip_en);

    info_.vlan_strip_en = vlan_strip_en;

    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::update_vlanins(bool vlan_insert_en)
{
    api_trace("Vlan Insert change");
    if (vlan_insert_en == info_.vlan_insert_en) {
        NIC_LOG_WARN("Prom flag: {}. No change in broadcast flag. Nop",
                       vlan_insert_en);
        return SDK_RET_OK;
    }

    NIC_LOG_DEBUG("devapi_lif: {}. Vlan Insert flag change {} -> {}",
                  get_id(), info_.vlan_insert_en,
                  vlan_insert_en);

    info_.vlan_insert_en = vlan_insert_en;

    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::update_bcast_filters(lif_bcast_filter_t bcast_filter)
{
    sdk_ret_t status = SDK_RET_OK;
    
    NIC_LOG_DEBUG("Bcast filters update lif: {}: arp: {}, dhcp_client: {}, "
                  "dhcp_server: {}, netbios: {}",
                  get_id(), 
                  bcast_filter.arp, bcast_filter.dhcp_client, 
                  bcast_filter.dhcp_server, bcast_filter.netbios);
    if (bcast_filter.arp != info_.bcast_filter.arp ||
        bcast_filter.dhcp_client != info_.bcast_filter.dhcp_client ||
        bcast_filter.dhcp_server != info_.bcast_filter.dhcp_server ||
        bcast_filter.netbios != info_.bcast_filter.netbios) {
        info_.bcast_filter.arp = bcast_filter.arp;
        info_.bcast_filter.dhcp_client = bcast_filter.dhcp_client;
        info_.bcast_filter.dhcp_server = bcast_filter.dhcp_server;
        info_.bcast_filter.netbios = bcast_filter.netbios;

        status = lif_halupdate();
    } else {
        NIC_LOG_DEBUG("devapi_lif: {}. No bcast filters change.", get_id());
    }

    return status;
}

sdk_ret_t
devapi_lif::update_mcast_filters(lif_mcast_filter_t mcast_filter)
{
    sdk_ret_t status = SDK_RET_OK;

    NIC_LOG_DEBUG("Mcast filters Update lif: {}: ipv6_neigh_adv: {}, ipv6_router_adv: {}, "
                  "dhcpv6_relay: {}, dhcpv6_mcast: {}, ipv6_mld: {}, ipv6_neigh_sol: {}",
                  get_id(), 
                  mcast_filter.ipv6_neigh_adv, mcast_filter.ipv6_router_adv,
                  mcast_filter.dhcpv6_relay, mcast_filter.dhcpv6_mcast, 
                  mcast_filter.ipv6_mld, mcast_filter.ipv6_neigh_sol);
    
    if (mcast_filter.ipv6_neigh_adv != info_.mcast_filter.ipv6_neigh_adv ||
        mcast_filter.ipv6_router_adv != info_.mcast_filter.ipv6_router_adv ||
        mcast_filter.dhcpv6_relay != info_.mcast_filter.dhcpv6_relay ||
        mcast_filter.dhcpv6_mcast != info_.mcast_filter.dhcpv6_mcast ||
        mcast_filter.ipv6_mld != info_.mcast_filter.ipv6_mld ||
        mcast_filter.ipv6_neigh_sol != info_.mcast_filter.ipv6_neigh_sol) {
        info_.mcast_filter.ipv6_neigh_adv = mcast_filter.ipv6_neigh_adv;
        info_.mcast_filter.ipv6_router_adv = mcast_filter.ipv6_router_adv;
        info_.mcast_filter.dhcpv6_relay = mcast_filter.dhcpv6_relay;
        info_.mcast_filter.dhcpv6_mcast = mcast_filter.dhcpv6_mcast;
        info_.mcast_filter.ipv6_mld = mcast_filter.ipv6_mld;
        info_.mcast_filter.ipv6_neigh_sol = mcast_filter.ipv6_neigh_sol;

        status = lif_halupdate();
    } else {
        NIC_LOG_DEBUG("devapi_lif: {}. No mcast filters change.", get_id());
    }

    return status;
}

sdk_ret_t
devapi_lif::upd_name(std::string name)
{
    strcpy(info_.name, name.c_str());
    return lif_halupdate();
}

sdk_ret_t
devapi_lif::upd_state(lif_state_t state)
{
    NIC_LOG_DEBUG("devapi_lif: {} state: {} -> {}", info_.lif_id, info_.lif_state, state);
    info_.lif_state = state;
    return lif_halupdate();
}

sdk_ret_t
devapi_lif::upd_rdma_sniff(bool rdma_sniff)
{
    if (rdma_sniff == info_.rdma_sniff) {
        NIC_LOG_WARN("RDMA sniff: {}. No change. Nop",
                     rdma_sniff);
    }
    info_.rdma_sniff = rdma_sniff;
    return lif_halupdate();
}

sdk_ret_t
devapi_lif::create_macvlan_filter(mac_t mac, vlan_t vlan)
{

    sdk_ret_t     ret = SDK_RET_OK;
    mac_vlan_filter_t   key;
    filter_type_t       type;
    devapi_filter       *filter = NULL;

    if (!mac && !vlan) {
        type = kh::FILTER_LIF;
    } else {
        type = kh::FILTER_LIF_MAC_VLAN;
    }

    key = std::make_tuple(type, mac, vlan);
    filter = devapi_filter::factory(this, mac, vlan, type);
    if (filter == NULL) {
        NIC_LOG_DEBUG("create_macvlan_filter failed.");
        ret = SDK_RET_ERR;
        goto end;
    } else {
        mac_vlan_filter_table_[key] = filter;
    }
end:
    return ret;
}

sdk_ret_t
devapi_lif::delete_macvlan_filter(mac_t mac, vlan_t vlan)
{
    std::map<mac_vlan_filter_t, devapi_filter*>::iterator it;
    mac_vlan_filter_t key;
    filter_type_t type;

    if (!mac && !vlan) {
        type = kh::FILTER_LIF;
    } else {
        type = kh::FILTER_LIF_MAC_VLAN;
    }

    key = std::make_tuple(type, mac, vlan);
    it = mac_vlan_filter_table_.find(key);
    if (it != mac_vlan_filter_table_.end()) {
        devapi_filter *filter = it->second;
        mac_vlan_filter_table_.erase(it);
        devapi_filter::destroy(filter);
    }
    return SDK_RET_OK;
}

// Should be called only in case of smart.
sdk_ret_t
devapi_lif::create_mac_filter(mac_t mac)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC, mac, 0);

    mac_vlan_filter_table_[key] = devapi_filter::factory(this, mac, 0,
                                                        kh::FILTER_LIF_MAC);
    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::delete_mac_filter(mac_t mac)
{
    std::map<mac_vlan_filter_t, devapi_filter*>::iterator it;
    mac_vlan_filter_t key(kh::FILTER_LIF_MAC, mac, 0);

    it = mac_vlan_filter_table_.find(key);
    devapi_filter *filter = it->second;

    mac_vlan_filter_table_.erase(it);
    devapi_filter::destroy(filter);

    return SDK_RET_OK;
}

// Should be called only in case of smart.
sdk_ret_t
devapi_lif::create_vlan_filter(vlan_t vlan)
{
    mac_vlan_filter_t key(kh::FILTER_LIF_VLAN, 0, vlan);

    mac_vlan_filter_table_[key] = devapi_filter::factory(this, 0, vlan,
                                                        kh::FILTER_LIF_VLAN);
    return SDK_RET_OK;
}

sdk_ret_t
devapi_lif::delete_vlan_filter(vlan_t vlan)
{
    std::map<mac_vlan_filter_t, devapi_filter*>::iterator it;
    mac_vlan_filter_t key(kh::FILTER_LIF_VLAN, 0, vlan);

    it = mac_vlan_filter_table_.find(key);
    devapi_filter *filter = it->second;

    mac_vlan_filter_table_.erase(it);
    devapi_filter::destroy(filter);
    return SDK_RET_OK;
}

void
devapi_lif::program_mcfilters(void)
{
    mac_t mac;
    for (auto it = mac_table_.begin(); it != mac_table_.end();it++) {
        mac = *it;
        if (is_multicast(mac)) {
            NIC_LOG_DEBUG("Trigger multicast MAC:{} filter add", macaddr2str(mac));
            add_mac(mac, true);
        }
    }
}

void
devapi_lif::deprogram_mcfilters(void)
{
    mac_t mac;

    for (auto it = mac_table_.begin(); it != mac_table_.end();it++) {
        mac = *it;
        if (is_multicast(mac)) {
            NIC_LOG_DEBUG("Trigger multicast MAC:{} filter del", macaddr2str(mac));
            del_mac(mac, false);
        }
    }
}

void
devapi_lif::remove_macfilters(bool skip_native_mac, bool update_db)
{
    mac_t mac;

    NIC_LOG_DEBUG("lif-{}: Removing Mac Filters", get_id());
    for (auto it = mac_table_.begin(); it != mac_table_.end();) {
        mac = *it;
        if (skip_native_mac && mac == MAC_TO_UINT64(info_.mac)) {
            it++;
            continue;
        }
        del_mac(mac, false);
        if (update_db) {
            it = mac_table_.erase(it);
        } else {
            it++;
        }
    }
    NIC_LOG_DEBUG("# of Mac Filters: {}", mac_table_.size());
}

void
devapi_lif::remove_vlanfilters(bool skip_native_vlan)
{
    vlan_t vlan;

    NIC_LOG_DEBUG("lif-{}: Removing Vlan Filters", get_id());

    for (auto it = vlan_table_.begin(); it != vlan_table_.end();) {
        vlan = *it;
#if 0
        if (skip_native_vlan && (vlan == NATIVE_VLAN_ID &&
                                 info_.type != sdk::platform::LIF_TYPE_SWM))
#endif
        if (skip_native_vlan && (vlan == NATIVE_VLAN_ID)) {
            it++;
            continue;
        }
        del_vlan(vlan, false);
        it = vlan_table_.erase(it);
    }
    NIC_LOG_DEBUG("# of Vlan Filters: {}", vlan_table_.size());
}

void
devapi_lif::remove_macvlanfilters(void)
{
    mac_t mac;
    vlan_t vlan;

    NIC_LOG_DEBUG("lif-{}: Removing Mac-Vlan Filters", get_id());
    for (auto it = mac_vlan_table_.begin(); it != mac_vlan_table_.end();) {
        mac = std::get<0>(*it);
        vlan = std::get<1>(*it);
        del_macvlan(mac, vlan, false);
        it = mac_vlan_table_.erase(it);
    }
    NIC_LOG_DEBUG("# of Mac-Vlan Filters: {}", mac_vlan_table_.size());
}

sdk_ret_t
devapi_lif::lif_halcreate(void)
{
    sdk_ret_t            ret = SDK_RET_OK;
    grpc::ClientContext        context;
    grpc::Status               status;

    LifSpec              *req;
    LifResponse          rsp;
    LifRequestMsg        req_msg;
    LifResponseMsg       rsp_msg;
    LifQStateMapEntry    *lif_qstate_map_ent;
    lif_info_t           *lif_info = get_lifinfo();

    // Set default number of max filters if nothing is passed
    if (!lif_info->max_vlan_filters) {
        lif_info->max_vlan_filters = LIF_DEFAULT_MAX_VLAN_FILTERS;
    }

    if (!lif_info->max_mac_filters) {
        lif_info->max_mac_filters = LIF_DEFAULT_MAX_MAC_FILTERS;
    }

    if (!lif_info->max_mac_vlan_filters) {
        lif_info->max_mac_vlan_filters = LIF_DEFAULT_MAX_MAC_VLAN_FILTERS;
    }

    NIC_LOG_DEBUG("Creating devapi_lif: id: {}, name: {}, prom: {}, oob: {}, "
                  "int_mgmt_mnic: {}, host_mgmt_mnic: {}, rdma_en: {}, "
                  "admin_status: {}",
                  lif_info->lif_id,
                  lif_info->name,
                  lif_info->receive_promiscuous,
                  is_oobmnic(),
                  is_intmgmtmnic(),
                  is_hostmgmt(),
                  lif_info->enable_rdma, lif_info->lif_state);

    populate_req(req_msg, &req);

    // Populate qstate map
    for (uint32_t i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto & qinfo = lif_info->queue_info[i];
        NIC_LOG_DEBUG("Processing queue type: {}, size: {}", i, qinfo.size);
        if (qinfo.size < 1) continue;

        NIC_LOG_DEBUG("Queue type_num: {}, entries: {}, purpose: {}",
                      qinfo.type_num,
                      qinfo.entries, qinfo.purpose);

        lif_qstate_map_ent = req->add_lif_qstate_map();
        lif_qstate_map_ent->set_type_num(qinfo.type_num);
        lif_qstate_map_ent->set_size(qinfo.size);
        lif_qstate_map_ent->set_entries(qinfo.entries);
        lif_qstate_map_ent->set_purpose((intf::LifQPurpose)qinfo.purpose);
    }

    VERIFY_HAL();
    status = hal->lif_create(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Created lif id: {}", get_id());
        } else {
            NIC_LOG_ERR("Failed to create devapi_lif for hw_lif_id: {}. err: {}",
                        get_id(), rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to create devapi_lif for hw_lif_id: {}. err: {}:{}",
                     get_id(), status.error_code(), status.error_message());
    }

    // Store spec
    spec_.CopyFrom(*req);
end:
    return ret;
}

sdk_ret_t
devapi_lif::lif_halupdate(void)
{
    sdk_ret_t            ret = SDK_RET_OK;
    grpc::ClientContext  context;
    grpc::Status         status;
    LifSpec              *req;
    LifResponse          rsp;
    LifRequestMsg        req_msg;
    LifResponseMsg       rsp_msg;
    // lif_info_t           *lif_info = get_lifinfo();

    populate_req(req_msg, &req);

    VERIFY_HAL();
    status = hal->lif_update(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Updated devapi_lif id: {}", get_id());
        } else {
            NIC_LOG_ERR("Failed to update devapi_lif for id: {}. err: {}",
                          get_id(), rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update devapi_lif for id: {}. err: {}:{}",
                      get_id(), status.error_code(), status.error_message());
    }

    // Store spec
    spec_.CopyFrom(*req);
end:
    return ret;
}

sdk_ret_t
devapi_lif::lif_haldelete(void)
{
    sdk_ret_t               ret = SDK_RET_OK;
    grpc::ClientContext     context;
    grpc::Status            status;
    LifDeleteRequest        *req;
    LifDeleteResponse       rsp;
    LifDeleteRequestMsg     req_msg;
    LifDeleteResponseMsg    rsp_msg;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(get_id());
    VERIFY_HAL();
    status = hal->lif_delete(req_msg, rsp_msg);
    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("Deleted devapi_lif id: {}", get_id());
        } else {
            NIC_LOG_ERR("Failed to delete devapi_lif for id: {}. err: {}",
                          get_id(), rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to delete devapi_lif for id: {}. err: {}:{}",
                     get_id(), status.error_code(), status.error_message());
    }
end:
    return ret;
}

void
devapi_lif::populate_req(LifRequestMsg &req_msg,
                         LifSpec **req_ptr)
{
    lif_info_t   *lif_info = get_lifinfo();
    LifSpec      *req;

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_lif_id(get_id());
    req->set_name(lif_info->name);
    req->set_type((types::LifType)lif_info->type);
    req->set_hw_lif_id(lif_info->lif_id);
    req->mutable_pinned_uplink_if_key_handle()->
        set_interface_id(get_uplink() ? get_uplink()->get_id() : 0);
    req->mutable_packet_filter()->set_receive_broadcast(lif_info->receive_broadcast);
    req->mutable_packet_filter()->set_receive_all_multicast(lif_info->receive_all_multicast);
    req->mutable_packet_filter()->set_receive_promiscuous(lif_info->receive_promiscuous);
    req->set_vlan_strip_en(lif_info->vlan_strip_en);
    req->set_vlan_insert_en(lif_info->vlan_insert_en);
    req->set_is_management(is_oobmnic() ||
                           is_intmgmt());
    req->set_admin_status((::intf::IfStatus)lif_info->lif_state);
    req->set_enable_rdma(lif_info->enable_rdma);
    req->set_rdma_sniff_en(lif_info->rdma_sniff);
    // NCSI bcast & mcast filters
    req->mutable_bcast_pkt_filter()->set_arp(lif_info->bcast_filter.arp);
    req->mutable_bcast_pkt_filter()->set_dhcp_client(lif_info->bcast_filter.dhcp_client);
    req->mutable_bcast_pkt_filter()->set_dhcp_server(lif_info->bcast_filter.dhcp_server);
    req->mutable_bcast_pkt_filter()->set_netbios(lif_info->bcast_filter.netbios);
    req->mutable_mcast_pkt_filter()->set_ipv6_neigh_adv(lif_info->mcast_filter.ipv6_neigh_adv);
    req->mutable_mcast_pkt_filter()->set_ipv6_router_adv(lif_info->mcast_filter.ipv6_router_adv);
    req->mutable_mcast_pkt_filter()->set_dhcpv6_relay(lif_info->mcast_filter.dhcpv6_relay);
    req->mutable_mcast_pkt_filter()->set_dhcpv6_mcast(lif_info->mcast_filter.dhcpv6_mcast);
    req->mutable_mcast_pkt_filter()->set_ipv6_mld(lif_info->mcast_filter.ipv6_mld);
    req->mutable_mcast_pkt_filter()->set_ipv6_neigh_sol(lif_info->mcast_filter.ipv6_neigh_sol);

    if (lif_info->type == sdk::platform::LIF_TYPE_SWM) {
        req->mutable_swm_oob()->
            set_interface_id(devapi_uplink::get_oob_uplink()->get_id());
        req->set_rx_en(lif_info->rx_en);
    }

    if (lif_info->rx_limit_bytes) {
        req->mutable_rx_policer()->mutable_bps_policer()->
            set_bytes_per_sec(lif_info->rx_limit_bytes);
        req->mutable_rx_policer()->mutable_bps_policer()->
            set_burst_bytes(lif_info->rx_burst_bytes);
    }
    if (lif_info->tx_limit_bytes) {
        req->mutable_tx_policer()->mutable_bps_policer()->
            set_bytes_per_sec(lif_info->tx_limit_bytes);
        req->mutable_tx_policer()->mutable_bps_policer()->
            set_burst_bytes(lif_info->tx_burst_bytes);

    }

    *req_ptr = req;
}

sdk_ret_t 
devapi_lif::set_micro_seg_en(bool en)
{
    devapi_lif *lif= NULL;

    for (auto it = lif_db_.cbegin(); it != lif_db_.cend(); it++) {
        lif = (devapi_lif *)(it->second);
        if (lif->is_host()) {
            lif->remove_vlanfilters(true);
            lif->remove_macfilters(true, false);
        }
    }

    return SDK_RET_OK;
}

devapi_uplink *
devapi_lif::get_uplink(void)
{
    // return info_.pinned_uplink;
    return devapi_uplink::get_uplink(info_.pinned_uplink_port_num);
}

devapi_enic *
devapi_lif::get_enic(void)
{
    return enic_;
}

void
devapi_lif::set_enic(devapi_enic *enic)
{
    enic_ = enic;
}

uint32_t
devapi_lif::get_id(void)
{
    return info_.lif_id;
}

bool
devapi_lif::get_isprom(void)
{
    return info_.receive_promiscuous;
}

lif_info_t *
devapi_lif::get_lifinfo(void)
{
    return &info_;
}

devapi_vrf *
devapi_lif::get_vrf(void)
{
    if (is_intmgmt()) {
        if (internal_mgmt_ethlif->info_.lif_id == info_.lif_id) {
            return vrf_;
        } else {
            return internal_mgmt_ethlif->get_vrf();
        }
    } else {
        return get_uplink()->get_vrf();
    }
#if 0
    if (is_intmgmtmnic()) {
        return vrf_;
    } else if (is_hostmgmt()) {
        return internal_mgmt_ethlif->get_vrf();
    } else {
        return get_uplink()->get_vrf();
    }
#endif
}

devapi_l2seg *
devapi_lif::get_nativel2seg(void)
{
    if (is_intmgmt()) {
        if (internal_mgmt_ethlif->info_.lif_id == info_.lif_id) {
            return native_l2seg_;
        } else {
            return internal_mgmt_ethlif->get_nativel2seg();
        }
    } else {
        return get_uplink()->get_native_l2seg();
    }
#if 0
    if (is_intmgmtmnic()) {
        return native_l2seg_;
    } else if (is_hostmgmt()) {
        return internal_mgmt_ethlif->get_nativel2seg();
    } else {
        return get_uplink()->get_native_l2seg();
    }
#endif
}

bool
devapi_lif::is_oobmnic(void)
{
    return (info_.type ==
            sdk::platform::LIF_TYPE_MNIC_OOB_MGMT);
}

bool
devapi_lif::is_intmgmtmnic(void)
{
    return (info_.type ==
            sdk::platform::LIF_TYPE_MNIC_INTERNAL_MGMT);
}

bool
devapi_lif::is_inbmgmtmnic(void)
{
    return (info_.type ==
            sdk::platform::LIF_TYPE_MNIC_INBAND_MGMT);
}

bool
devapi_lif::is_mnic(void)
{
    return (is_oobmnic() ||
            is_intmgmtmnic() ||
            is_inbmgmtmnic());
}

bool
devapi_lif::is_host(void)
{
    return (info_.type == sdk::platform::LIF_TYPE_HOST);
}

bool
devapi_lif::is_hostmgmt(void)
{
    return (info_.type == sdk::platform::LIF_TYPE_HOST_MGMT);
}

bool
devapi_lif::is_intmgmt(void)
{
    return (is_intmgmtmnic() ||
            is_hostmgmt());
}

bool
devapi_lif::is_recallmc(void)
{
    return info_.receive_all_multicast;
}

bool
devapi_lif::is_classicfwd(vlan_t vlan)
{
    // Classic:
    // - All LIFs
    // Smart (Hostpin):
    // - All LIFs
    //   - Host Mgmt - All traffic
    //   - Host Data - Untag traffic
    //   - ARM Internal Mgmt - All traffic
    //   - ARM Internal Data - Untag traffic
    //   - ARM OOB - All traffic
    //   TODO: This is causing vlans to be created in GS case.
    // if (hal->get_fwd_mode() == sdk::platform::FWD_MODE_CLASSIC ||
    if (!hal->get_micro_seg_en() ||
        (is_hostmgmt() || is_intmgmtmnic() ||
         is_oobmnic() || vlan == NATIVE_VLAN_ID)) {
        return true;
    }
    return false;
}

bool
devapi_lif::is_swm(void)
{
    return (info_.type == sdk::platform::LIF_TYPE_SWM);
}

}    // namespce iris
