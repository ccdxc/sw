import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISecurityAppList,IApiStatus,ISecurityApp,ISecurityCertificateList,ISecurityCertificate,ISecuritySecurityGroupList,ISecuritySecurityGroup,ISecuritySGPolicyList,ISecuritySGPolicy,ISecurityTrafficEncryptionPolicy,ISecurityAutoMsgAppWatchHelper,ISecurityAutoMsgCertificateWatchHelper,ISecurityAutoMsgSecurityGroupWatchHelper,ISecurityAutoMsgSGPolicyWatchHelper,ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper } from '../../models/generated/security';

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
  public ListApp(queryParam: any = null):Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    return this.invokeAJAXGetCall(url, queryParam, 'ListApp') as Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create App object */
  public AddApp(body: ISecurityApp):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    return this.invokeAJAXPostCall(url, body, 'AddApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get App object */
  public GetApp(O_Name, queryParam: any = null):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete App object */
  public DeleteApp(O_Name):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update App object */
  public UpdateApp(O_Name, body: ISecurityApp):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Certificate objects */
  public ListCertificate_1(queryParam: any = null):Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    return this.invokeAJAXGetCall(url, queryParam, 'ListCertificate_1') as Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AddCertificate_1(body: ISecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    return this.invokeAJAXPostCall(url, body, 'AddCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public GetCertificate_1(O_Name, queryParam: any = null):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public DeleteCertificate_1(O_Name):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public UpdateCertificate_1(O_Name, body: ISecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup_1(queryParam: any = null):Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    return this.invokeAJAXGetCall(url, queryParam, 'ListSecurityGroup_1') as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup_1(body: ISecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    return this.invokeAJAXPostCall(url, body, 'AddSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup_1(O_Name, queryParam: any = null):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup_1(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup_1(O_Name, body: ISecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public ListSGPolicy_1(queryParam: any = null):Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    return this.invokeAJAXGetCall(url, queryParam, 'ListSGPolicy_1') as Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AddSGPolicy_1(body: ISecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    return this.invokeAJAXPostCall(url, body, 'AddSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public GetSGPolicy_1(O_Name, queryParam: any = null):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public DeleteSGPolicy_1(O_Name):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public UpdateSGPolicy_1(O_Name, body: ISecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Certificate objects */
  public ListCertificate(queryParam: any = null):Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListCertificate') as Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AddCertificate(body: ISecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public GetCertificate(O_Name, queryParam: any = null):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public DeleteCertificate(O_Name):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public UpdateCertificate(O_Name, body: ISecurityCertificate):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup(queryParam: any = null):Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListSecurityGroup') as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup(body: ISecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup(O_Name, queryParam: any = null):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup(O_Name):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup(O_Name, body: ISecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public ListSGPolicy(queryParam: any = null):Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListSGPolicy') as Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AddSGPolicy(body: ISecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public GetSGPolicy(O_Name, queryParam: any = null):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public DeleteSGPolicy(O_Name):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public UpdateSGPolicy(O_Name, body: ISecuritySGPolicy):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy(body: ISecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy(O_Name, queryParam: any = null):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public UpdateTrafficEncryptionPolicy(O_Name, body: ISecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy_1(body: ISecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy';
    return this.invokeAJAXPostCall(url, body, 'AddTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy_1(O_Name, queryParam: any = null):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy_1(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public UpdateTrafficEncryptionPolicy_1(O_Name, body: ISecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch App objects. Supports WebSockets or HTTP long poll */
  public WatchApp(queryParam: any = null):Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/apps';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchApp') as Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects. Supports WebSockets or HTTP long poll */
  public WatchCertificate_1(queryParam: any = null):Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/certificates';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchCertificate_1') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects. Supports WebSockets or HTTP long poll */
  public WatchSecurityGroup_1(queryParam: any = null):Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/security-groups';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSecurityGroup_1') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchSGPolicy_1(queryParam: any = null):Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/sgpolicies';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSGPolicy_1') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects. Supports WebSockets or HTTP long poll */
  public WatchCertificate(queryParam: any = null):Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchCertificate') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects. Supports WebSockets or HTTP long poll */
  public WatchSecurityGroup(queryParam: any = null):Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSecurityGroup') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchSGPolicy(queryParam: any = null):Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSGPolicy') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchTrafficEncryptionPolicy(queryParam: any = null):Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchTrafficEncryptionPolicy') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchTrafficEncryptionPolicy_1(queryParam: any = null):Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/trafficEncryptionPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchTrafficEncryptionPolicy_1') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}