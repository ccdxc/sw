import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IAuthAuthenticationPolicy,IApiStatus,AuthAuthenticationPolicy,IAuthRoleBindingList,IAuthRoleBinding,AuthRoleBinding,IAuthRoleList,IAuthRole,AuthRole,IAuthUserList,IAuthUser,AuthUser } from '../../models/generated/auth';

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
  public GetAuthenticationPolicy(queryParam: any = null):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    return this.invokeAJAXGetCall(url, queryParam, 'GetAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AuthenticationPolicy object */
  public AddAuthenticationPolicy(body: AuthAuthenticationPolicy):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AuthenticationPolicy object */
  public UpdateAuthenticationPolicy(body: AuthAuthenticationPolicy):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List RoleBinding objects */
  public ListRoleBinding_1(queryParam: any = null):Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    return this.invokeAJAXGetCall(url, queryParam, 'ListRoleBinding_1') as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AddRoleBinding_1(body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public GetRoleBinding_1(O_Name, queryParam: any = null):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public DeleteRoleBinding_1(O_Name):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public UpdateRoleBinding_1(O_Name, body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateRoleBinding_1') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public ListRole_1(queryParam: any = null):Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    return this.invokeAJAXGetCall(url, queryParam, 'ListRole_1') as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AddRole_1(body: AuthRole):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public GetRole_1(O_Name, queryParam: any = null):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public DeleteRole_1(O_Name):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public UpdateRole_1(O_Name, body: AuthRole):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateRole_1') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List RoleBinding objects */
  public ListRoleBinding(queryParam: any = null):Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListRoleBinding') as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AddRoleBinding(body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public GetRoleBinding(O_Name, queryParam: any = null):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public DeleteRoleBinding(O_Name):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public UpdateRoleBinding(O_Name, body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public ListRole(queryParam: any = null):Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListRole') as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AddRole(body: AuthRole):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public GetRole(O_Name, queryParam: any = null):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public DeleteRole(O_Name):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public UpdateRole(O_Name, body: AuthRole):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public ListUser(queryParam: any = null):Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListUser') as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AddUser(body: AuthUser):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public GetUser(O_Name, queryParam: any = null):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public DeleteUser(O_Name):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public UpdateUser(O_Name, body: AuthUser):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public ListUser_1(queryParam: any = null):Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    return this.invokeAJAXGetCall(url, queryParam, 'ListUser_1') as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AddUser_1(body: AuthUser):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public GetUser_1(O_Name, queryParam: any = null):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public DeleteUser_1(O_Name):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public UpdateUser_1(O_Name, body: AuthUser):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateUser_1') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
}