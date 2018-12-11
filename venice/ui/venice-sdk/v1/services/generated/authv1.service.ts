import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IAuthAuthenticationPolicy,IApiStatus,IAuthRoleBindingList,IAuthRoleBinding,IAuthRoleList,IAuthRole,IAuthUserList,IAuthUser,IAuthPasswordChangeRequest,IAuthPasswordResetRequest,IAuthAutoMsgAuthenticationPolicyWatchHelper,IAuthAutoMsgRoleBindingWatchHelper,IAuthAutoMsgRoleWatchHelper,IAuthAutoMsgUserWatchHelper } from '../../models/generated/auth';

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
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AuthenticationPolicy object */
  public AddAuthenticationPolicy(body: IAuthAuthenticationPolicy, stagingID: string = ""):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AuthenticationPolicy object */
  public UpdateAuthenticationPolicy(body: IAuthAuthenticationPolicy, stagingID: string = ""):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  public LdapBindCheck(body: IAuthAuthenticationPolicy, stagingID: string = ""):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/LdapBindCheck';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'LdapBindCheck') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  public LdapConnectionCheck(body: IAuthAuthenticationPolicy, stagingID: string = ""):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/LdapConnectionCheck';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'LdapConnectionCheck') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List RoleBinding objects */
  public ListRoleBinding_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListRoleBinding_1') as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AddRoleBinding_1(body: IAuthRoleBinding, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public GetRoleBinding_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public DeleteRoleBinding_1(O_Name, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public UpdateRoleBinding_1(O_Name, body: IAuthRoleBinding, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public ListRole_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListRole_1') as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AddRole_1(body: IAuthRole, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public GetRole_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public DeleteRole_1(O_Name, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public UpdateRole_1(O_Name, body: IAuthRole, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List RoleBinding objects */
  public ListRoleBinding(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListRoleBinding') as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AddRoleBinding(body: IAuthRoleBinding, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public GetRoleBinding(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public DeleteRoleBinding(O_Name, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public UpdateRoleBinding(O_Name, body: IAuthRoleBinding, stagingID: string = ""):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public ListRole(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListRole') as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AddRole(body: IAuthRole, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public GetRole(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public DeleteRole(O_Name, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public UpdateRole(O_Name, body: IAuthRole, stagingID: string = ""):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public ListUser(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListUser') as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AddUser(body: IAuthUser, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public GetUser(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public DeleteUser(O_Name, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public UpdateUser(O_Name, body: IAuthUser, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  public PasswordChange(O_Name, body: IAuthPasswordChangeRequest, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}/PasswordChange';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'PasswordChange') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  public PasswordReset(O_Name, body: IAuthPasswordResetRequest, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}/PasswordReset';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'PasswordReset') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public ListUser_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListUser_1') as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AddUser_1(body: IAuthUser, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public GetUser_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public DeleteUser_1(O_Name, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public UpdateUser_1(O_Name, body: IAuthUser, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  public PasswordChange_1(O_Name, body: IAuthPasswordChangeRequest, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}/PasswordChange';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'PasswordChange_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  public PasswordReset_1(O_Name, body: IAuthPasswordResetRequest, stagingID: string = ""):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}/PasswordReset';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'PasswordReset_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AuthenticationPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchAuthenticationPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgAuthenticationPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/authn-policy';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAuthenticationPolicy') as Observable<{body: IAuthAutoMsgAuthenticationPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch RoleBinding objects. Supports WebSockets or HTTP long poll */
  public WatchRoleBinding_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/role-bindings';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchRoleBinding_1') as Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Role objects. Supports WebSockets or HTTP long poll */
  public WatchRole_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/roles';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchRole_1') as Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch RoleBinding objects. Supports WebSockets or HTTP long poll */
  public WatchRoleBinding(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchRoleBinding') as Observable<{body: IAuthAutoMsgRoleBindingWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Role objects. Supports WebSockets or HTTP long poll */
  public WatchRole(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchRole') as Observable<{body: IAuthAutoMsgRoleWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch User objects. Supports WebSockets or HTTP long poll */
  public WatchUser(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchUser') as Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch User objects. Supports WebSockets or HTTP long poll */
  public WatchUser_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/watch/users';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchUser_1') as Observable<{body: IAuthAutoMsgUserWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}