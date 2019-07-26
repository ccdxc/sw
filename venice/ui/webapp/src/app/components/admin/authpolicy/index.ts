
import { AuthAuthenticationPolicy, ApiStatus, AuthLdap, AuthRadius, IAuthAuthenticationPolicy, AuthAuthenticators_authenticator_order, IApiStatus, AuthLdapServerStatus_result } from '@sdk/v1/models/generated/auth';

export interface LDAPCheckResponse {
    type: LDAPCheckType;
    authpolicy: AuthAuthenticationPolicy;
}

export enum LDAPCheckType {
    BIND = 'Bind',
    CONNECTION = 'Connect'
}

export interface CheckResponseError {
    type: LDAPCheckType;
    errors: any[];
}

export interface LdapSave {
    createData?: AuthLdap;
    onSuccess: Function;
}

export interface RadiusSave {
    createData?: AuthRadius;
    onSuccess: Function;
}




