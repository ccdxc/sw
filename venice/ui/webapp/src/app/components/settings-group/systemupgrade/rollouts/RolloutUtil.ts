import {EnumRolloutOptions} from '.';
import { IRolloutRollout, RolloutRollout, RolloutRolloutSpec } from '@sdk/v1/models/generated/rollout';

export class RolloutUtil {
 // Make sure the follow parameter values match above index.ts-EnumRolloutOptions.keys
 public static ROLLOUTTYPE_NAPLES_ONLY = 'naplesonly';
 public static ROLLOUTTYPE_VENICE_ONLY = 'veniceonly';
 public static ROLLOUTTYPE_BOTH_NAPLES_VENICE = 'both';

 public static ROLLOUT_METADATA_JSON = 'metadata.json';
 public static ROLLOUT_BUNDLE_TAR = 'bundle.tar';

 /**
  * In English (from Suresh):
  * if smartnics-only == true then venice is skipped.
  * if smartnic-must-match-constraint is false  then
	*   ALL naples are upgraded
  * else
  *    Only naples matching constraints are upgraded
  *
  *  =====
  * Naples only :
  *    "smartnics-only": true,
  *    if (smartnic-must-match-constraint== true) {
  *         Naples listed in [order-constraints] will be updated
  *    } else {
  *        update all naples
  *    }
  *
  * Both Venice and Napple :
  *   "smartnics-only": false,
  *    if (smartnic-must-match-constraint== true) {
  *         Naples listed in [order-constraints] will be updated
  *    } else {
  *        update all naples
  *    }
  *
  * Venice-only :
  *   "smartnics-only": false,
  *   "order-constraints": [],
  *   "smartnic-must-match-constraint": true,
  *
  *
  *
  *
  */
 public static getRolloutNaplesVeniceType(rollout: RolloutRollout): string {
    if (rollout.spec['smartnics-only']) {
      return RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY;
    } else {
      // Figure out if it is   RolloutUtil.ROLLOUTTYPE_VENICE_ONLY or RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE
      return  (rollout.spec['smartnic-must-match-constraint'] === true && rollout.spec['order-constraints'] && rollout.spec['order-constraints'].length === 0 ) ? RolloutUtil.ROLLOUTTYPE_VENICE_ONLY : RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE;
    }
  }
}
