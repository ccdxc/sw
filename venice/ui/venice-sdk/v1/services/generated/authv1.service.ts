import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { IAuthAuthenticationPolicy,AuthAuthenticationPolicy,IApiStatus,ApiStatus,AuthTokenSecretRequest,IAuthTokenSecretRequest,IAuthRoleBindingList,AuthRoleBindingList,IAuthRoleBinding,AuthRoleBinding,IAuthRoleList,AuthRoleList,IAuthRole,AuthRole,IAuthUserPreference,AuthUserPreference,IAuthUserList,AuthUserList,IAuthUser,AuthUser,AuthSubjectAccessReviewRequest,IAuthSubjectAccessReviewRequest,AuthPasswordChangeRequest,IAuthPasswordChangeRequest,AuthPasswordResetRequest,IAuthPasswordResetRequest,IAuthAutoMsgAuthenticationPolicyWatchHelper,AuthAutoMsgAuthenticationPolicyWatchHelper,IAuthAutoMsgRoleBindingWatchHelper,AuthAutoMsgRoleBindingWatchHelper,IAuthAutoMsgRoleWatchHelper,AuthAutoMsgRoleWatchHelper,IAuthAutoMsgUserPreferenceWatchHelper,AuthAutoMsgUserPreferenceWatchHelper,IAuthAutoMsgUserWatchHelper,AuthAutoMsgUserWatchHelper } from '../../models/generated/auth';

@Injectable()
export class Authv1Service extends AbstractService {
  constructor(protected _http: HttpClient) {
    super(_http);
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  /** Get AuthenticationPolicy object */
  public GetAuthenticationPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    const opts = {
      eventID: 'GetAuthenticationPolicy',
      objType: 'AuthAuthenticationPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AuthenticationPolicy object */
  public AddAuthenticationPolicy(body: IAuthAuthenticationPolicy, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    const opts = {
      eventID: 'AddAuthenticationPolicy',
      objType: 'AuthAuthenticationPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthAuthenticationPolicy(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AuthenticationPolicy object */
  public UpdateAuthenticationPolicy(body: IAuthAuthenticationPolicy, stagingID: string = "", previousVal: IAuthAuthenticationPolicy = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    const opts = {
      eventID: 'UpdateAuthenticationPolicy',
      objType: 'AuthAuthenticationPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthAuthenticationPolicy(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Test LDAP bind operation */
  public LdapBindCheck(body: IAuthAuthenticationPolicy, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/LdapBindCheck';
    const opts = {
      eventID: 'LdapBindCheck',
      objType: 'AuthAuthenticationPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthAuthenticationPolicy(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Test LDAP connection */
  public LdapConnectionCheck(body: IAuthAuthenticationPolicy, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/LdapConnectionCheck';
    const opts = {
      eventID: 'LdapConnectionCheck',
      objType: 'AuthAuthenticationPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthAuthenticationPolicy(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Generate secret for token signing */
  public TokenSecretGenerate(body: IAuthTokenSecretRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/TokenSecretGenerate';
    const opts = {
      eventID: 'TokenSecretGenerate',
      objType: 'AuthAuthenticationPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthTokenSecretRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List RoleBinding objects */
  public ListRoleBinding_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    const opts = {
      eventID: 'ListRoleBinding_1',
      objType: 'AuthRoleBindingList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AddRoleBinding_1(body: IAuthRoleBinding, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    const opts = {
      eventID: 'AddRoleBinding_1',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRoleBinding(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public GetRoleBinding_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetRoleBinding_1',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public DeleteRoleBinding_1(O_Name, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteRoleBinding_1',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public UpdateRoleBinding_1(O_Name, body: IAuthRoleBinding, stagingID: string = "", previousVal: IAuthRoleBinding = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateRoleBinding_1',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRoleBinding(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public ListRole_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    const opts = {
      eventID: 'ListRole_1',
      objType: 'AuthRoleList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AddRole_1(body: IAuthRole, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    const opts = {
      eventID: 'AddRole_1',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRole(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public GetRole_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetRole_1',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public DeleteRole_1(O_Name, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteRole_1',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public UpdateRole_1(O_Name, body: IAuthRole, stagingID: string = "", previousVal: IAuthRole = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateRole_1',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRole(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List RoleBinding objects */
  public ListRoleBinding(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListRoleBinding',
      objType: 'AuthRoleBindingList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AddRoleBinding(body: IAuthRoleBinding, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddRoleBinding',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRoleBinding(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public GetRoleBinding(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetRoleBinding',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public DeleteRoleBinding(O_Name, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteRoleBinding',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public UpdateRoleBinding(O_Name, body: IAuthRoleBinding, stagingID: string = "", previousVal: IAuthRoleBinding = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateRoleBinding',
      objType: 'AuthRoleBinding',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRoleBinding(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public ListRole(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListRole',
      objType: 'AuthRoleList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AddRole(body: IAuthRole, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddRole',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRole(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public GetRole(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetRole',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public DeleteRole(O_Name, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteRole',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public UpdateRole(O_Name, body: IAuthRole, stagingID: string = "", previousVal: IAuthRole = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateRole',
      objType: 'AuthRole',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthRole(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get UserPreference object */
  public GetUserPreference(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/user-preferences/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetUserPreference',
      objType: 'AuthUserPreference',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update UserPreference object */
  public UpdateUserPreference(O_Name, body: IAuthUserPreference, stagingID: string = "", previousVal: IAuthUserPreference = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/user-preferences/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateUserPreference',
      objType: 'AuthUserPreference',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthUserPreference(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public ListUser(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListUser',
      objType: 'AuthUserList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AddUser(body: IAuthUser, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddUser',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthUser(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public GetUser(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetUser',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public DeleteUser(O_Name, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteUser',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public UpdateUser(O_Name, body: IAuthUser, stagingID: string = "", previousVal: IAuthUser = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateUser',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthUser(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Review authorization for user */
  public IsAuthorized(O_Name, body: IAuthSubjectAccessReviewRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}/IsAuthorized';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'IsAuthorized',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthSubjectAccessReviewRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Change user password */
  public PasswordChange(O_Name, body: IAuthPasswordChangeRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}/PasswordChange';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'PasswordChange',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthPasswordChangeRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Reset user password */
  public PasswordReset(O_Name, body: IAuthPasswordResetRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}/PasswordReset';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'PasswordReset',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthPasswordResetRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get UserPreference object */
  public GetUserPreference_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/user-preferences/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetUserPreference_1',
      objType: 'AuthUserPreference',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update UserPreference object */
  public UpdateUserPreference_1(O_Name, body: IAuthUserPreference, stagingID: string = "", previousVal: IAuthUserPreference = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/user-preferences/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateUserPreference_1',
      objType: 'AuthUserPreference',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthUserPreference(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthUserPreference | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public ListUser_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    const opts = {
      eventID: 'ListUser_1',
      objType: 'AuthUserList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AddUser_1(body: IAuthUser, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    const opts = {
      eventID: 'AddUser_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthUser(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public GetUser_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetUser_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public DeleteUser_1(O_Name, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteUser_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public UpdateUser_1(O_Name, body: IAuthUser, stagingID: string = "", previousVal: IAuthUser = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateUser_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthUser(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Review authorization for user */
  public IsAuthorized_1(O_Name, body: IAuthSubjectAccessReviewRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}/IsAuthorized';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'IsAuthorized_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthSubjectAccessReviewRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Change user password */
  public PasswordChange_1(O_Name, body: IAuthPasswordChangeRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}/PasswordChange';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'PasswordChange_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthPasswordChangeRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Reset user password */
  public PasswordReset_1(O_Name, body: IAuthPasswordResetRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}/PasswordReset';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'PasswordReset_1',
      objType: 'AuthUser',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new AuthPasswordResetRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AuthenticationPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchAuthenticationPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgAuthenticationPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/authn-policy';
    const opts = {
      eventID: 'WatchAuthenticationPolicy',
      objType: 'AuthAutoMsgAuthenticationPolicyWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgAuthenticationPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch RoleBinding objects. Supports WebSockets or HTTP long poll */
  public WatchRoleBinding_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/role-bindings';
    const opts = {
      eventID: 'WatchRoleBinding_1',
      objType: 'AuthAutoMsgRoleBindingWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Role objects. Supports WebSockets or HTTP long poll */
  public WatchRole_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/roles';
    const opts = {
      eventID: 'WatchRole_1',
      objType: 'AuthAutoMsgRoleWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch RoleBinding objects. Supports WebSockets or HTTP long poll */
  public WatchRoleBinding(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchRoleBinding',
      objType: 'AuthAutoMsgRoleBindingWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Role objects. Supports WebSockets or HTTP long poll */
  public WatchRole(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchRole',
      objType: 'AuthAutoMsgRoleWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch UserPreference objects. Supports WebSockets or HTTP long poll */
  public WatchUserPreference(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgUserPreferenceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/user-preferences';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchUserPreference',
      objType: 'AuthAutoMsgUserPreferenceWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgUserPreferenceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch User objects. Supports WebSockets or HTTP long poll */
  public WatchUser(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchUser',
      objType: 'AuthAutoMsgUserWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch UserPreference objects. Supports WebSockets or HTTP long poll */
  public WatchUserPreference_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgUserPreferenceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/user-preferences';
    const opts = {
      eventID: 'WatchUserPreference_1',
      objType: 'AuthAutoMsgUserPreferenceWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgUserPreferenceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch User objects. Supports WebSockets or HTTP long poll */
  public WatchUser_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/users';
    const opts = {
      eventID: 'WatchUser_1',
      objType: 'AuthAutoMsgUserWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}