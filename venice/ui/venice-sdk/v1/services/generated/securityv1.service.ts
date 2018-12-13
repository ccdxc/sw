import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISecurityAppList,IApiStatus,ISecurityApp,ISecurityCertificateList,ISecurityCertificate,ISecurityFirewallProfileList,ISecurityFirewallProfile,ISecuritySecurityGroupList,ISecuritySecurityGroup,ISecuritySGPolicyList,ISecuritySGPolicy,ISecurityTrafficEncryptionPolicy,ISecurityAutoMsgAppWatchHelper,ISecurityAutoMsgCertificateWatchHelper,ISecurityAutoMsgFirewallProfileWatchHelper,ISecurityAutoMsgSecurityGroupWatchHelper,ISecurityAutoMsgSGPolicyWatchHelper,ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper } from '../../models/generated/security';

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
  public ListApp_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListApp_1') as Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create App object */
  public AddApp_1(body: ISecurityApp, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddApp_1') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get App object */
  public GetApp_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetApp_1') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete App object */
  public DeleteApp_1(O_Name, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteApp_1') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update App object */
  public UpdateApp_1(O_Name, body: ISecurityApp, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateApp_1') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Certificate objects */
  public ListCertificate_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListCertificate_1') as Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AddCertificate_1(body: ISecurityCertificate, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public GetCertificate_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public DeleteCertificate_1(O_Name, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public UpdateCertificate_1(O_Name, body: ISecurityCertificate, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateCertificate_1') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FirewallProfile objects */
  public ListFirewallProfile_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListFirewallProfile_1') as Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create FirewallProfile object */
  public AddFirewallProfile_1(body: ISecurityFirewallProfile, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddFirewallProfile_1') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FirewallProfile object */
  public GetFirewallProfile_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetFirewallProfile_1') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete FirewallProfile object */
  public DeleteFirewallProfile_1(O_Name, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteFirewallProfile_1') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FirewallProfile object */
  public UpdateFirewallProfile_1(O_Name, body: ISecurityFirewallProfile, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateFirewallProfile_1') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListSecurityGroup_1') as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup_1(body: ISecuritySecurityGroup, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup_1(O_Name, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup_1(O_Name, body: ISecuritySecurityGroup, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateSecurityGroup_1') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public ListSGPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListSGPolicy_1') as Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AddSGPolicy_1(body: ISecuritySGPolicy, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public GetSGPolicy_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public DeleteSGPolicy_1(O_Name, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public UpdateSGPolicy_1(O_Name, body: ISecuritySGPolicy, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateSGPolicy_1') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List App objects */
  public ListApp(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListApp') as Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create App object */
  public AddApp(body: ISecurityApp, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get App object */
  public GetApp(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete App object */
  public DeleteApp(O_Name, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update App object */
  public UpdateApp(O_Name, body: ISecurityApp, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateApp') as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Certificate objects */
  public ListCertificate(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListCertificate') as Observable<{body: ISecurityCertificateList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AddCertificate(body: ISecurityCertificate, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public GetCertificate(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public DeleteCertificate(O_Name, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public UpdateCertificate(O_Name, body: ISecurityCertificate, stagingID: string = ""):Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/certificates/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateCertificate') as Observable<{body: ISecurityCertificate | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FirewallProfile objects */
  public ListFirewallProfile(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListFirewallProfile') as Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create FirewallProfile object */
  public AddFirewallProfile(body: ISecurityFirewallProfile, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddFirewallProfile') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FirewallProfile object */
  public GetFirewallProfile(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetFirewallProfile') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete FirewallProfile object */
  public DeleteFirewallProfile(O_Name, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteFirewallProfile') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FirewallProfile object */
  public UpdateFirewallProfile(O_Name, body: ISecurityFirewallProfile, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateFirewallProfile') as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListSecurityGroup') as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup(body: ISecuritySecurityGroup, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup(O_Name, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup(O_Name, body: ISecuritySecurityGroup, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateSecurityGroup') as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public ListSGPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListSGPolicy') as Observable<{body: ISecuritySGPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AddSGPolicy(body: ISecuritySGPolicy, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public GetSGPolicy(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public DeleteSGPolicy(O_Name, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public UpdateSGPolicy(O_Name, body: ISecuritySGPolicy, stagingID: string = ""):Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/sgpolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateSGPolicy') as Observable<{body: ISecuritySGPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy(body: ISecurityTrafficEncryptionPolicy, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy(O_Name, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public UpdateTrafficEncryptionPolicy(O_Name, body: ISecurityTrafficEncryptionPolicy, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateTrafficEncryptionPolicy') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AddTrafficEncryptionPolicy_1(body: ISecurityTrafficEncryptionPolicy, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public GetTrafficEncryptionPolicy_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public DeleteTrafficEncryptionPolicy_1(O_Name, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public UpdateTrafficEncryptionPolicy_1(O_Name, body: ISecurityTrafficEncryptionPolicy, stagingID: string = ""):Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateTrafficEncryptionPolicy_1') as Observable<{body: ISecurityTrafficEncryptionPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch App objects. Supports WebSockets or HTTP long poll */
  public WatchApp_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/apps';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchApp_1') as Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects. Supports WebSockets or HTTP long poll */
  public WatchCertificate_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/certificates';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchCertificate_1') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FirewallProfile objects. Supports WebSockets or HTTP long poll */
  public WatchFirewallProfile_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/firewallprofiles';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchFirewallProfile_1') as Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects. Supports WebSockets or HTTP long poll */
  public WatchSecurityGroup_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/security-groups';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSecurityGroup_1') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchSGPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/sgpolicies';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSGPolicy_1') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch App objects. Supports WebSockets or HTTP long poll */
  public WatchApp(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/apps';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchApp') as Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects. Supports WebSockets or HTTP long poll */
  public WatchCertificate(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/certificates';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchCertificate') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FirewallProfile objects. Supports WebSockets or HTTP long poll */
  public WatchFirewallProfile(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/firewallprofiles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchFirewallProfile') as Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects. Supports WebSockets or HTTP long poll */
  public WatchSecurityGroup(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSecurityGroup') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchSGPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/sgpolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSGPolicy') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchTrafficEncryptionPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/trafficEncryptionPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchTrafficEncryptionPolicy') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchTrafficEncryptionPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/trafficEncryptionPolicy';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchTrafficEncryptionPolicy_1') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}