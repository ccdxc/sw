import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISecurityAppList,IApiStatus,ISecurityApp,ISecurityAppUserList,ISecurityAppUser,SecurityAppUser,ISecurityAppUserGrpList,ISecurityAppUserGrp,SecurityAppUserGrp,ISecurityCertificateList,ISecurityCertificate,SecurityCertificate,ISecuritySecurityGroupList,ISecuritySecurityGroup,SecuritySecurityGroup,ISecuritySgpolicyList,ISecuritySgpolicy,SecuritySgpolicy,ISecurityTrafficEncryptionPolicy,SecurityTrafficEncryptionPolicy,ISecurityAutoMsgAppUserWatchHelper,ISecurityAutoMsgAppUserGrpWatchHelper,ISecurityAutoMsgCertificateWatchHelper,ISecurityAutoMsgSecurityGroupWatchHelper,ISecurityAutoMsgSgpolicyWatchHelper,ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper } from '../../models/generated/security';

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
  public ListApp():Observable<{body: ISecurityAppList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    return this.invokeAJAXGetCall(url, 'ListApp') as Observable<{body: ISecurityAppList | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the App object */
  public GetApp(O_Name):Observable<{body: ISecurityApp | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetApp') as Observable<{body: ISecurityApp | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of AppUser objects */
  public ListAppUser():Observable<{body: ISecurityAppUserList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAppUser') as Observable<{body: ISecurityAppUserList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new AppUser object */
  public AddAppUser(body: SecurityAppUser):Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAppUser') as Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of AppUserGrp objects */
  public ListAppUserGrp():Observable<{body: ISecurityAppUserGrpList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAppUserGrp') as Observable<{body: ISecurityAppUserGrpList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new AppUserGrp object */
  public AddAppUserGrp(body: SecurityAppUserGrp):Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAppUserGrp') as Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the AppUserGrp object */
  public GetAppUserGrp(O_Name):Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAppUserGrp') as Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the AppUserGrp object */
  public DeleteAppUserGrp(O_Name):Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAppUserGrp') as Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}>;
  }
  
  public UpdateAppUserGrp(O_Name, body: SecurityAppUserGrp):Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAppUserGrp') as Observable<{body: ISecurityAppUserGrp | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the AppUser object */
  public GetAppUser(O_Name):Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAppUser') as Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the AppUser object */
  public DeleteAppUser(O_Name):Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAppUser') as Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}>;
  }
  
  public UpdateAppUser(O_Name, body: SecurityAppUser):Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/app-users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAppUser') as Observable<{body: ISecurityAppUser | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Certificate objects */
  public ListCertificate():Observable<{body: ISecurityCertificateList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListCertificate') as Observable<{body: ISecurityCertificateList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Certificate object */
  public AddCertificate(body: SecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddCertificate') as Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Certificate object */
  public GetCertificate(O_Name):Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetCertificate') as Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Certificate object */
  public DeleteCertificate(O_Name):Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate') as Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}>;
  }
  
  public UpdateCertificate(O_Name, body: SecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateCertificate') as Observable<{body: ISecurityCertificate | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of SecurityGroup objects */
  public ListSecurityGroup():Observable<{body: ISecuritySecurityGroupList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListSecurityGroup') as Observable<{body: ISecuritySecurityGroupList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new SecurityGroup object */
  public AddSecurityGroup(body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the SecurityGroup object */
  public GetSecurityGroup(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the SecurityGroup object */
  public DeleteSecurityGroup(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}>;
  }
  
  public UpdateSecurityGroup(O_Name, body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Sgpolicy objects */
  public ListSgpolicy():Observable<{body: ISecuritySgpolicyList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListSgpolicy') as Observable<{body: ISecuritySgpolicyList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Sgpolicy object */
  public AddSgpolicy(body: SecuritySgpolicy):Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSgpolicy') as Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Sgpolicy object */
  public GetSgpolicy(O_Name):Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSgpolicy') as Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Sgpolicy object */
  public DeleteSgpolicy(O_Name):Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSgpolicy') as Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}>;
  }
  
  public UpdateSgpolicy(O_Name, body: SecuritySgpolicy):Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSgpolicy') as Observable<{body: ISecuritySgpolicy | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy(body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}>;
  }
  
  public UpdateTrafficEncryptionPolicy(O_Name, body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to AppUser objects */
  public WatchAppUser():Observable<{body: ISecurityAutoMsgAppUserWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/app-users';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchAppUser') as Observable<{body: ISecurityAutoMsgAppUserWatchHelper | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to AppUserGrp objects */
  public WatchAppUserGrp():Observable<{body: ISecurityAutoMsgAppUserGrpWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/app-users-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchAppUserGrp') as Observable<{body: ISecurityAutoMsgAppUserGrpWatchHelper | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to Certificate objects */
  public WatchCertificate():Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchCertificate') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to SecurityGroup objects */
  public WatchSecurityGroup():Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchSecurityGroup') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to Sgpolicy objects */
  public WatchSgpolicy():Observable<{body: ISecurityAutoMsgSgpolicyWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/sgpolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchSgpolicy') as Observable<{body: ISecurityAutoMsgSgpolicyWatchHelper | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to TrafficEncryptionPolicy objects */
  public WatchTrafficEncryptionPolicy():Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchTrafficEncryptionPolicy') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus, statusCode: number}>;
  }
  
}