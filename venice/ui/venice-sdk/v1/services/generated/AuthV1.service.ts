import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IAuthRoleBindingList,IAuthRoleBinding,AuthRoleBinding,IAuthRoleList,IAuthRole,AuthRole,IAuthUserList,IAuthUser,AuthUser } from '../../models/generated/auth';

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

  /** List RoleBinding objects */
  public AutoListRoleBinding-1():Observable<{body: IAuthRoleBindingList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    return this.invokeAJAXGetCall(url, 'AutoListRoleBinding-1') as Observable<{body: IAuthRoleBindingList | Error, statusCode: number}>;
  }
  
  /** Create RoleBinding object */
  public AutoAddRoleBinding-1(body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddRoleBinding-1') as Observable<{body: IAuthRoleBinding | Error, statusCode: number}>;
  }
  
  /** Get RoleBinding object */
  public AutoGetRoleBinding-1(O_Name):Observable<{body: IAuthRoleBinding | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetRoleBinding-1') as Observable<{body: IAuthRoleBinding | Error, statusCode: number}>;
  }
  
  /** Delete RoleBinding object */
  public AutoDeleteRoleBinding-1(O_Name):Observable<{body: IAuthRoleBinding | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteRoleBinding-1') as Observable<{body: IAuthRoleBinding | Error, statusCode: number}>;
  }
  
  /** Update RoleBinding object */
  public AutoUpdateRoleBinding-1(O_Name, body: AuthRoleBinding):Observable<{body: IAuthRoleBinding | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/role-bindings/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateRoleBinding-1') as Observable<{body: IAuthRoleBinding | Error, statusCode: number}>;
  }
  
  /** List Role objects */
  public AutoListRole-1():Observable<{body: IAuthRoleList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    return this.invokeAJAXGetCall(url, 'AutoListRole-1') as Observable<{body: IAuthRoleList | Error, statusCode: number}>;
  }
  
  /** Create Role object */
  public AutoAddRole-1(body: AuthRole):Observable<{body: IAuthRole | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddRole-1') as Observable<{body: IAuthRole | Error, statusCode: number}>;
  }
  
  /** Get Role object */
  public AutoGetRole-1(O_Name):Observable<{body: IAuthRole | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetRole-1') as Observable<{body: IAuthRole | Error, statusCode: number}>;
  }
  
  /** Delete Role object */
  public AutoDeleteRole-1(O_Name):Observable<{body: IAuthRole | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteRole-1') as Observable<{body: IAuthRole | Error, statusCode: number}>;
  }
  
  /** Update Role object */
  public AutoUpdateRole-1(O_Name, body: AuthRole):Observable<{body: IAuthRole | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/roles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateRole-1') as Observable<{body: IAuthRole | Error, statusCode: number}>;
  }
  
  /** List User objects */
  public AutoListUser-1():Observable<{body: IAuthUserList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    return this.invokeAJAXGetCall(url, 'AutoListUser-1') as Observable<{body: IAuthUserList | Error, statusCode: number}>;
  }
  
  /** Create User object */
  public AutoAddUser-1(body: AuthUser):Observable<{body: IAuthUser | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddUser-1') as Observable<{body: IAuthUser | Error, statusCode: number}>;
  }
  
  /** Get User object */
  public AutoGetUser-1(O_Name):Observable<{body: IAuthUser | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetUser-1') as Observable<{body: IAuthUser | Error, statusCode: number}>;
  }
  
  /** Delete User object */
  public AutoDeleteUser-1(O_Name):Observable<{body: IAuthUser | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteUser-1') as Observable<{body: IAuthUser | Error, statusCode: number}>;
  }
  
  /** Update User object */
  public AutoUpdateUser-1(O_Name, body: AuthUser):Observable<{body: IAuthUser | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/users/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateUser-1') as Observable<{body: IAuthUser | Error, statusCode: number}>;
  }
  
}