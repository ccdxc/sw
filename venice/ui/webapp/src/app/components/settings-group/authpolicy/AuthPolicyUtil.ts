import { LDAPCheckResponse , LDAPCheckType, CheckResponseError } from '@app/components/settings-group/authpolicy/.';
import { AuthAuthenticationPolicy, ApiStatus, AuthLdap, IAuthAuthenticationPolicy, AuthAuthenticators_authenticator_order, IApiStatus, AuthLdapServerStatus_result } from '@sdk/v1/models/generated/auth';

export class AuthPolicyUtil {
    public static processLDAPCheckResponse(checkResponse: LDAPCheckResponse): CheckResponseError {
        const _checkResponseError: CheckResponseError = {
            type: null,
            errors: []
        };
        if (checkResponse) {
            _checkResponseError.type = checkResponse.type;
            checkResponse.authpolicy.status['ldap-servers'].forEach(server => {
                if ( server.result === AuthLdapServerStatus_result.Bind_Failure
                    || server.result === AuthLdapServerStatus_result.Connect_Failure  ) {
                    _checkResponseError.errors.push(server);
                }
            });
        }
        return _checkResponseError;
    }
}
