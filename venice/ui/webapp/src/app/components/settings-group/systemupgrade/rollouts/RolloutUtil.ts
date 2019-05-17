import {EnumRolloutOptions} from '.';
import { IRolloutRollout, RolloutRollout, RolloutRolloutSpec } from '@sdk/v1/models/generated/rollout';

export class RolloutUtil {
 // Make sure the follow parameter values match above index.ts-EnumRolloutOptions.keys
 public static ROLLOUTTYPE_NAPLES_ONLY = 'naplesonly';
 public static ROLLOUTTYPE_VENICE_ONLY = 'veniceonly';
 public static ROLLOUTTYPE_BOTH_NAPLES_VENICE = 'both';
 public static ROLLOUT_METADATA_JSON = 'metadata.json';
 public static ROLLOUT_BUNDLE_TAR = 'bundle.tar';

 public static getRolloutNaplesVeniceType(newRollout: RolloutRollout): string {
    if (newRollout.spec['smartnics-only']) {
      return RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY;
    } else {
      // Figure out if it is   RolloutUtil.ROLLOUTTYPE_NODE_ONLY or RolloutUtil.ROLLOUTTYPE_BOTH_NIC_NODE
      return  newRollout.spec['smartnic-must-match-constraint'] ? RolloutUtil.ROLLOUTTYPE_VENICE_ONLY : RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE;
    }
  }
}
