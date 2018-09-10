import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISecurityAppList,IApiStatus,ISecurityApp,SecurityApp,ISecurityCertificateList,ISecurityCertificate,SecurityCertificate,ISecuritySecurityGroupList,ISecuritySecurityGroup,SecuritySecurityGroup,ISecuritySGPolicyList,ISecuritySGPolicy,SecuritySGPolicy,ISecurityTrafficEncryptionPolicy,SecurityTrafficEncryptionPolicy,ISecurityAutoMsgAppWatchHelper,ISecurityAutoMsgCertificateWatchHelper,ISecurityAutoMsgSecurityGroupWatchHelper,ISecurityAutoMsgSGPolicyWatchHelper,ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper } from '../../models/generated/security';

@Injectable()
export class Securityv1Service extends AbstractService {
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

  /** List App objects */
  public ListApp():Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    return this.invokeAJAXGetCall(url, 'ListApp') as Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create App object */
  public AddApp(body: SecurityApp):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get App object */
  public GetApp(O_Name):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete App object */
  public DeleteApp(O_Name):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update App object */
  public UpdateApp(O_Name, body: SecurityApp):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Certificate objects */
  public ListCertificate_1():Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    return this.invokeAJAXGetCall(url, 'ListCertificate_1') as Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AddCertificate_1(body: SecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public GetCertificate_1(O_Name):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public DeleteCertificate_1(O_Name):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public UpdateCertificate_1(O_Name, body: SecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup_1():Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    return this.invokeAJAXGetCall(url, 'ListSecurityGroup_1') as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup_1(body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup_1(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup_1(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup_1(O_Name, body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public ListSGPolicy_1():Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    return this.invokeAJAXGetCall(url, 'ListSGPolicy_1') as Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AddSGPolicy_1(body: SecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public GetSGPolicy_1(O_Name):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public DeleteSGPolicy_1(O_Name):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public UpdateSGPolicy_1(O_Name, body: SecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Certificate objects */
  public ListCertificate():Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListCertificate') as Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AddCertificate(body: SecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public GetCertificate(O_Name):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public DeleteCertificate(O_Name):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public UpdateCertificate(O_Name, body: SecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup():Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListSecurityGroup') as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup(body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup(O_Name, body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public ListSGPolicy():Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListSGPolicy') as Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AddSGPolicy(body: SecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public GetSGPolicy(O_Name):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public DeleteSGPolicy(O_Name):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public UpdateSGPolicy(O_Name, body: SecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy(body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public UpdateTrafficEncryptionPolicy(O_Name, body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy_1(body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy_1(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy_1(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public UpdateTrafficEncryptionPolicy_1(O_Name, body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch App objects */
  public WatchApp():Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/apps';
    return this.invokeAJAXGetCall(url, 'WatchApp') as Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects */
  public WatchCertificate_1():Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/certificates';
    return this.invokeAJAXGetCall(url, 'WatchCertificate_1') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects */
  public WatchSecurityGroup_1():Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/security-groups';
    return this.invokeAJAXGetCall(url, 'WatchSecurityGroup_1') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects */
  public WatchSGPolicy_1():Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/sgpolicies';
    return this.invokeAJAXGetCall(url, 'WatchSGPolicy_1') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects */
  public WatchCertificate():Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchCertificate') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects */
  public WatchSecurityGroup():Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchSecurityGroup') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects */
  public WatchSGPolicy():Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchSGPolicy') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects */
  public WatchTrafficEncryptionPolicy():Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchTrafficEncryptionPolicy') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects */
  public WatchTrafficEncryptionPolicy_1():Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/trafficEncryptionPolicy';
    return this.invokeAJAXGetCall(url, 'WatchTrafficEncryptionPolicy_1') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}