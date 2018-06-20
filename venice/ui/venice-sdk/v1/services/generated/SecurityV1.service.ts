import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { SecurityAppList,ApiStatus,SecurityApp,SecurityAppUserList,SecurityAppUser,SecurityAppUserGrpList,SecurityAppUserGrp,SecurityCertificateList,SecurityCertificate,SecuritySecurityGroupList,SecuritySecurityGroup,SecuritySgpolicyList,SecuritySgpolicy,SecurityTrafficEncryptionPolicy } from '../../models/generated/security';

@Injectable()
export class SecurityV1Service extends AbstractService {
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

  /** Retreives a list of App objects */
  public ListApp():Observable<{body: SecurityAppList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/apps';
    return this.invokeAJAXGetCall(url, 'ListApp') as Observable<{body: SecurityAppList | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the App object */
  public GetApp(O_Name):Observable<{body: SecurityApp | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetApp') as Observable<{body: SecurityApp | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of AppUser objects */
  public ListAppUser():Observable<{body: SecurityAppUserList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAppUser') as Observable<{body: SecurityAppUserList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new AppUser object */
  public AddAppUser(body: SecurityAppUser):Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAppUser') as Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of AppUserGrp objects */
  public ListAppUserGrp():Observable<{body: SecurityAppUserGrpList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAppUserGrp') as Observable<{body: SecurityAppUserGrpList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new AppUserGrp object */
  public AddAppUserGrp(body: SecurityAppUserGrp):Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAppUserGrp') as Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the AppUserGrp object */
  public GetAppUserGrp(O_Name):Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAppUserGrp') as Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the AppUserGrp object */
  public DeleteAppUserGrp(O_Name):Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAppUserGrp') as Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}>;
  }
  
  public UpdateAppUserGrp(O_Name, body: SecurityAppUserGrp):Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAppUserGrp') as Observable<{body: SecurityAppUserGrp | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the AppUser object */
  public GetAppUser(O_Name):Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAppUser') as Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the AppUser object */
  public DeleteAppUser(O_Name):Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAppUser') as Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}>;
  }
  
  public UpdateAppUser(O_Name, body: SecurityAppUser):Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/app-users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAppUser') as Observable<{body: SecurityAppUser | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Certificate objects */
  public ListCertificate():Observable<{body: SecurityCertificateList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListCertificate') as Observable<{body: SecurityCertificateList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Certificate object */
  public AddCertificate(body: SecurityCertificate):Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddCertificate') as Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Certificate object */
  public GetCertificate(O_Name):Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetCertificate') as Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Certificate object */
  public DeleteCertificate(O_Name):Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate') as Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}>;
  }
  
  public UpdateCertificate(O_Name, body: SecurityCertificate):Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateCertificate') as Observable<{body: SecurityCertificate | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of SecurityGroup objects */
  public ListSecurityGroup():Observable<{body: SecuritySecurityGroupList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListSecurityGroup') as Observable<{body: SecuritySecurityGroupList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new SecurityGroup object */
  public AddSecurityGroup(body: SecuritySecurityGroup):Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSecurityGroup') as Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the SecurityGroup object */
  public GetSecurityGroup(O_Name):Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSecurityGroup') as Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the SecurityGroup object */
  public DeleteSecurityGroup(O_Name):Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup') as Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}>;
  }
  
  public UpdateSecurityGroup(O_Name, body: SecuritySecurityGroup):Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSecurityGroup') as Observable<{body: SecuritySecurityGroup | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Sgpolicy objects */
  public ListSgpolicy():Observable<{body: SecuritySgpolicyList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/sgpolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListSgpolicy') as Observable<{body: SecuritySgpolicyList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Sgpolicy object */
  public AddSgpolicy(body: SecuritySgpolicy):Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/sgpolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSgpolicy') as Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Sgpolicy object */
  public GetSgpolicy(O_Name):Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/sgpolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSgpolicy') as Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Sgpolicy object */
  public DeleteSgpolicy(O_Name):Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/sgpolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSgpolicy') as Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateSgpolicy(O_Name, body: SecuritySgpolicy):Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/sgpolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSgpolicy') as Observable<{body: SecuritySgpolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy(body: SecurityTrafficEncryptionPolicy):Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTrafficEncryptionPolicy') as Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy(O_Name):Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetTrafficEncryptionPolicy') as Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy(O_Name):Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy') as Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateTrafficEncryptionPolicy(O_Name, body: SecurityTrafficEncryptionPolicy):Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/security/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTrafficEncryptionPolicy') as Observable<{body: SecurityTrafficEncryptionPolicy | ApiStatus, statusCode: number}>;
  }
  
}