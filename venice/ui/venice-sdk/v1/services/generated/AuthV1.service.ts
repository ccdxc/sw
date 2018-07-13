import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IAuthAuthenticationPolicy,IApiStatus,AuthAuthenticationPolicy,IAuthRoleBindingList,IAuthRoleBinding,AuthRoleBinding,IAuthRoleList,IAuthRole,AuthRole,IAuthUserList,IAuthUser,AuthUser } from '../../models/generated/auth';

@Injectable()
export class AuthV1Service extends AbstractService {
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

  /** Creates a new AuthenticationPolicy object */
  public AddAuthenticationPolicy(body: AuthAuthenticationPolicy):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the AuthenticationPolicy object */
  public GetAuthenticationPolicy(O_Name):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the AuthenticationPolicy object */
  public DeleteAuthenticationPolicy(O_Name):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateAuthenticationPolicy(O_Name, body: AuthAuthenticationPolicy):Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/authn-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAuthenticationPolicy') as Observable<{body: IAuthAuthenticationPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives a list of RoleBinding objects */
  public ListRoleBinding():Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListRoleBinding') as Observable<{body: IAuthRoleBindingList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Creates a new RoleBinding object */
  public AddRoleBinding(body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the RoleBinding object */
  public GetRoleBinding(O_Name):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the RoleBinding object */
  public DeleteRoleBinding(O_Name):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateRoleBinding(O_Name, body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/role-bindings/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateRoleBinding') as Observable<{body: IAuthRoleBinding | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives a list of Role objects */
  public ListRole():Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListRole') as Observable<{body: IAuthRoleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Creates a new Role object */
  public AddRole(body: AuthRole):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the Role object */
  public GetRole(O_Name):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the Role object */
  public DeleteRole(O_Name):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateRole(O_Name, body: AuthRole):Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/roles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateRole') as Observable<{body: IAuthRole | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives a list of User objects */
  public ListUser():Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListUser') as Observable<{body: IAuthUserList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Creates a new User object */
  public AddUser(body: AuthUser):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the User object */
  public GetUser(O_Name):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the User object */
  public DeleteUser(O_Name):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateUser(O_Name, body: AuthUser):Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateUser') as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }
  
}