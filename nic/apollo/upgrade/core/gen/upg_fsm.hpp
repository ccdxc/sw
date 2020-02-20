#ifndef __FSM_AUTO_GEN__
#define __FSM_AUTO_GEN__

#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"

namespace upg {

    stage_transition_t stage_transitions[] = {
    stage_transition_t(STAGE_ID_VALIDATION, SVC_RSP_OK,   STAGE_ID_START),
    stage_transition_t(STAGE_ID_VALIDATION, SVC_RSP_FAIL, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_VALIDATION, SVC_RSP_CRIT, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_VALIDATION, SVC_RSP_NONE, STAGE_ID_EXIT),

    stage_transition_t(STAGE_ID_START, SVC_RSP_OK,   STAGE_ID_PREPARE),
    stage_transition_t(STAGE_ID_START, SVC_RSP_FAIL, STAGE_ID_ABORT),
    stage_transition_t(STAGE_ID_START, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_START, SVC_RSP_NONE, STAGE_ID_ABORT),

    stage_transition_t(STAGE_ID_PREPARE, SVC_RSP_OK,   STAGE_ID_BACKUP),
    stage_transition_t(STAGE_ID_PREPARE, SVC_RSP_FAIL, STAGE_ID_ABORT),
    stage_transition_t(STAGE_ID_PREPARE, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_PREPARE, SVC_RSP_NONE, STAGE_ID_ABORT),

    stage_transition_t(STAGE_ID_BACKUP, SVC_RSP_OK,   STAGE_ID_UPGRADE),
    stage_transition_t(STAGE_ID_BACKUP, SVC_RSP_FAIL, STAGE_ID_ROLLBACK),
    stage_transition_t(STAGE_ID_BACKUP, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_BACKUP, SVC_RSP_NONE, STAGE_ID_ROLLBACK),

    stage_transition_t(STAGE_ID_UPGRADE, SVC_RSP_OK,   STAGE_ID_VERIFY),
    stage_transition_t(STAGE_ID_UPGRADE, SVC_RSP_FAIL, STAGE_ID_ROLLBACK),
    stage_transition_t(STAGE_ID_UPGRADE, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_UPGRADE, SVC_RSP_NONE, STAGE_ID_ROLLBACK),

    stage_transition_t(STAGE_ID_VERIFY, SVC_RSP_OK,   STAGE_ID_FINISH),
    stage_transition_t(STAGE_ID_VERIFY, SVC_RSP_FAIL, STAGE_ID_ROLLBACK),
    stage_transition_t(STAGE_ID_VERIFY, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_VERIFY, SVC_RSP_NONE, STAGE_ID_ROLLBACK),

    stage_transition_t(STAGE_ID_FINISH, SVC_RSP_OK,   STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_FINISH, SVC_RSP_FAIL, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_FINISH, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_FINISH, SVC_RSP_NONE, STAGE_ID_EXIT),

    stage_transition_t(STAGE_ID_ABORT, SVC_RSP_OK,   STAGE_ID_FINISH),
    stage_transition_t(STAGE_ID_ABORT, SVC_RSP_FAIL, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_ABORT, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_ABORT, SVC_RSP_NONE, STAGE_ID_CRITICAL),

    stage_transition_t(STAGE_ID_ROLLBACK, SVC_RSP_OK,   STAGE_ID_FINISH),
    stage_transition_t(STAGE_ID_ROLLBACK, SVC_RSP_FAIL, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_ROLLBACK, SVC_RSP_CRIT, STAGE_ID_CRITICAL),
    stage_transition_t(STAGE_ID_ROLLBACK, SVC_RSP_NONE, STAGE_ID_CRITICAL),

    stage_transition_t(STAGE_ID_CRITICAL, SVC_RSP_OK,   STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_CRITICAL, SVC_RSP_FAIL, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_CRITICAL, SVC_RSP_CRIT, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_CRITICAL, SVC_RSP_NONE, STAGE_ID_EXIT),

    stage_transition_t(STAGE_ID_EXIT, SVC_RSP_OK,   STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_EXIT, SVC_RSP_FAIL, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_EXIT, SVC_RSP_CRIT, STAGE_ID_EXIT),
    stage_transition_t(STAGE_ID_EXIT, SVC_RSP_NONE, STAGE_ID_EXIT)
};

svc_t svc[] = {
    svc_t("sysmgr"),
    svc_t("pdsagent"),
    svc_t("vpp")
};

event_sequence_t event_sequence = PARALLEL;

idl_stage_t idl_stages_cfg[] = {
    idl_stage_t("validation",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("start",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("prepare",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("backup",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("upgrade",
                "100",
                "pdsageny:vpp:sysmgr",
                "serial",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("verify",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("finish",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("abort",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("rollback",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("critical",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh"),
    idl_stage_t("exit",
                "",
                "",
                "",
                "pre_a.sh:pre_b.sh",
                "post_a.sh:post:b.sh")
};
}
#endif
