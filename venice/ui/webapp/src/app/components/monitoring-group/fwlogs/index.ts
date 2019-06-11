import { SecuritySGPolicy } from '@sdk/v1/models/generated/security';
import { SecuritySGRule } from '@sdk/v1/models/generated/security';

export interface PolicyRuleTuple {
    policy: SecuritySGPolicy;
    rule: SecuritySGRule;
}
