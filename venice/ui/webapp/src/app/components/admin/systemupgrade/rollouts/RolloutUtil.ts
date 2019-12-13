import { EnumRolloutOptions } from '.';
import { IRolloutRollout, RolloutRollout, RolloutRolloutSpec, RolloutRolloutStatus_state, ILabelsSelector, LabelsSelector, LabelsRequirement, ILabelsRequirement  } from '@sdk/v1/models/generated/rollout';
import { Utility } from '@app/common/Utility';

export class RolloutUtil {
  // Make sure the follow parameter values match above index.ts-EnumRolloutOptions.keys
  public static ROLLOUTTYPE_NAPLES_ONLY = 'naplesonly';
  public static ROLLOUTTYPE_VENICE_ONLY = 'veniceonly';
  public static ROLLOUTTYPE_BOTH_NAPLES_VENICE = 'both';

  public static ROLLOUT_METADATA_JSON = 'metadata.json';
  public static ROLLOUT_BUNDLE_TAR = 'bundle.tar';

  /**
   * In English (from Suresh):
   * if dscs-only == true then venice is skipped.
   * if dsc-must-match-constraint is false  then
   *   ALL DSCS are upgraded
   * else
   *    Only DSCS matching constraints are upgraded
   *
   *  =====
   * DSC only :
   *    "dscs-only": true,
   *    if (dsc-must-match-constraint== true) {
   *         Naples listed in [order-constraints] will be updated
   *    } else {
   *        update all naples
   *    }
   *
   * Both Venice and DSC :
   *   "dscs-only": false,
   *    if (dsc-must-match-constraint== true) {
   *         Naples listed in [order-constraints] will be updated
   *    } else {
   *        update all naples
   *    }
   *
   * Venice-only :
   *   "dscs-only": false,
   *   "order-constraints": [],
   *   "dsc-must-match-constraint": true,
   *
   *
   *
   *
   */
  public static getRolloutNaplesVeniceType(rollout: RolloutRollout): string {
    if (rollout.spec['dscs-only']) {
      return RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY;
    } else {
      // Figure out if it is   RolloutUtil.ROLLOUTTYPE_VENICE_ONLY or RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE
      return (rollout.spec['dsc-must-match-constraint'] === true && rollout.spec['order-constraints'] && rollout.spec['order-constraints'].length === 0) ? RolloutUtil.ROLLOUTTYPE_VENICE_ONLY : RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE;
    }
  }

  public static formatRolloutNaplesCriteria(rollout: RolloutRollout): string[] {
    if (!rollout) {
      return null;
    }
    const list = Utility.convertLabelSelectorsToStringList(rollout.spec['order-constraints']);
    return list;
  }

  public static isRolloutPending(rollout: RolloutRollout) {
    if (rollout.status.state === RolloutRolloutStatus_state.progressing || rollout.status.state === RolloutRolloutStatus_state.scheduled
      || rollout.status.state === RolloutRolloutStatus_state['precheck-in-progress']
      || rollout.status.state === RolloutRolloutStatus_state['scheduled-for-retry']
      || rollout.status.state === RolloutRolloutStatus_state['suspend-in-progress']) {
      return true;
    }
    return false;
  }

  public static isRolloutInactive(rollout: RolloutRollout) {
    const state = rollout.status.state;
    if ([RolloutRolloutStatus_state.failure, RolloutRolloutStatus_state.success, RolloutRolloutStatus_state.suspended, RolloutRolloutStatus_state['deadline-exceeded']].includes(state)) {
      return true;
    }
    return false;
  }
}
