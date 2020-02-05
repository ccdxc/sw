//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines upgrade enums
///
//----------------------------------------------------------------------------

#ifndef __UPGRADE_UPGRADE__HPP__
#define __UPGRADE_UPGRADE__HPP__

#include "nic/sdk/include/sdk/types.hpp"

/// \defgroup UPG Upgrade Manager
/// @{


/// \brief upgrade stages
typedef enum upg_stage_e {
    UPG_STAGE_NONE = 0,
    UPG_STAGE_START,      ///< start a new upgrade
    UPG_STAGE_PREPARE,    ///< prepare for an upgrade
    UPG_STAGE_ABORT,      ///< abort the on-going upgrade
    UPG_STAGE_VERIFY,     ///< verify the upgrade is successful
    UPG_STAGE_ROLLBACK,   ///< rollback to the previous version
    UPG_STAGE_SWITCH,     ///< switch to the new version
    UPG_STAGE_FINISH,     ///< last stage of the upgrade.
                          ///< indicates that upgrade is done
} upg_stage_t;

/// \brief upgrade modes
typedef enum upg_mode_e {
    UPG_MODE_NONE = 0,
    UPG_MODE_DISRUPTIVE,        ///< disruptive upgrade
    UPG_MODE_NON_DISRUPTIVE     ///< non disruptive single instance upgrade
} upg_mode_t;

/// \brief upgrade operational table state actions
typedef enum upg_oper_state_action_e {
    UPG_OPER_STATE_ACTION_NONE = 0,
    UPG_OPER_STATE_ACTION_COPY,       ///< copy operational state tables to new location in memory
                                      ///< with proper translation
    UPG_OPER_STATE_ACTION_FLUSH,      ///< flush the oprational state tables
    UPG_OPER_STATE_ACTION_PRESERVE,   ///< keep the operational state tables in the current location
                                      ///< with no changes.
    UPG_OPER_STATE_ACTION_REPLACE     ///< replace the existing operational state table
} upg_oper_state_action_t;

/// @}

#endif    // __UPGRADE_UPGRADE__HPP__
