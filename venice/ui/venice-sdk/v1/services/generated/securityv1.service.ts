import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { ISecurityAppList,SecurityAppList,IApiStatus,ApiStatus,ISecurityApp,SecurityApp,ISecurityFirewallProfileList,SecurityFirewallProfileList,ISecurityFirewallProfile,SecurityFirewallProfile,ISecurityNetworkSecurityPolicyList,SecurityNetworkSecurityPolicyList,ISecurityNetworkSecurityPolicy,SecurityNetworkSecurityPolicy,ISecuritySecurityGroupList,SecuritySecurityGroupList,ISecuritySecurityGroup,SecuritySecurityGroup,ISecurityAutoMsgAppWatchHelper,SecurityAutoMsgAppWatchHelper,ISecurityAutoMsgFirewallProfileWatchHelper,SecurityAutoMsgFirewallProfileWatchHelper,ISecurityAutoMsgNetworkSecurityPolicyWatchHelper,SecurityAutoMsgNetworkSecurityPolicyWatchHelper,ISecurityAutoMsgSecurityGroupWatchHelper,SecurityAutoMsgSecurityGroupWatchHelper } from '../../models/generated/security';

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
    const opts = {
      eventID: 'ListApp_1',
      objType: 'SecurityAppList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create App object */
  public AddApp_1(body: ISecurityApp, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps';
    const opts = {
      eventID: 'AddApp_1',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityApp(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get App object */
  public GetApp_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetApp_1',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete App object */
  public DeleteApp_1(O_Name, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteApp_1',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update App object */
  public UpdateApp_1(O_Name, body: ISecurityApp, stagingID: string = "", previousVal: ISecurityApp = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/apps/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateApp_1',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityApp(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FirewallProfile objects */
  public ListFirewallProfile_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles';
    const opts = {
      eventID: 'ListFirewallProfile_1',
      objType: 'SecurityFirewallProfileList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FirewallProfile object */
  public GetFirewallProfile_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetFirewallProfile_1',
      objType: 'SecurityFirewallProfile',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FirewallProfile object */
  public UpdateFirewallProfile_1(O_Name, body: ISecurityFirewallProfile, stagingID: string = "", previousVal: ISecurityFirewallProfile = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/firewallprofiles/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateFirewallProfile_1',
      objType: 'SecurityFirewallProfile',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityFirewallProfile(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List NetworkSecurityPolicy objects */
  public ListNetworkSecurityPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityNetworkSecurityPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/networksecuritypolicies';
    const opts = {
      eventID: 'ListNetworkSecurityPolicy_1',
      objType: 'SecurityNetworkSecurityPolicyList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityNetworkSecurityPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create NetworkSecurityPolicy object */
  public AddNetworkSecurityPolicy_1(body: ISecurityNetworkSecurityPolicy, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/networksecuritypolicies';
    const opts = {
      eventID: 'AddNetworkSecurityPolicy_1',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityNetworkSecurityPolicy(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get NetworkSecurityPolicy object */
  public GetNetworkSecurityPolicy_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/networksecuritypolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetNetworkSecurityPolicy_1',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete NetworkSecurityPolicy object */
  public DeleteNetworkSecurityPolicy_1(O_Name, stagingID: string = ""):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/networksecuritypolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteNetworkSecurityPolicy_1',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update NetworkSecurityPolicy object */
  public UpdateNetworkSecurityPolicy_1(O_Name, body: ISecurityNetworkSecurityPolicy, stagingID: string = "", previousVal: ISecurityNetworkSecurityPolicy = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/networksecuritypolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateNetworkSecurityPolicy_1',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityNetworkSecurityPolicy(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    const opts = {
      eventID: 'ListSecurityGroup_1',
      objType: 'SecuritySecurityGroupList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup_1(body: ISecuritySecurityGroup, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    const opts = {
      eventID: 'AddSecurityGroup_1',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecuritySecurityGroup(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetSecurityGroup_1',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup_1(O_Name, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteSecurityGroup_1',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup_1(O_Name, body: ISecuritySecurityGroup, stagingID: string = "", previousVal: ISecuritySecurityGroup = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateSecurityGroup_1',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecuritySecurityGroup(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List App objects */
  public ListApp(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListApp',
      objType: 'SecurityAppList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAppList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create App object */
  public AddApp(body: ISecurityApp, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddApp',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityApp(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get App object */
  public GetApp(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetApp',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete App object */
  public DeleteApp(O_Name, stagingID: string = ""):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteApp',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update App object */
  public UpdateApp(O_Name, body: ISecurityApp, stagingID: string = "", previousVal: ISecurityApp = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/apps/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateApp',
      objType: 'SecurityApp',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityApp(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecurityApp | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FirewallProfile objects */
  public ListFirewallProfile(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListFirewallProfile',
      objType: 'SecurityFirewallProfileList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityFirewallProfileList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FirewallProfile object */
  public GetFirewallProfile(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetFirewallProfile',
      objType: 'SecurityFirewallProfile',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FirewallProfile object */
  public UpdateFirewallProfile(O_Name, body: ISecurityFirewallProfile, stagingID: string = "", previousVal: ISecurityFirewallProfile = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/firewallprofiles/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateFirewallProfile',
      objType: 'SecurityFirewallProfile',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityFirewallProfile(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecurityFirewallProfile | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List NetworkSecurityPolicy objects */
  public ListNetworkSecurityPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityNetworkSecurityPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/networksecuritypolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListNetworkSecurityPolicy',
      objType: 'SecurityNetworkSecurityPolicyList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityNetworkSecurityPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create NetworkSecurityPolicy object */
  public AddNetworkSecurityPolicy(body: ISecurityNetworkSecurityPolicy, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/networksecuritypolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddNetworkSecurityPolicy',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityNetworkSecurityPolicy(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get NetworkSecurityPolicy object */
  public GetNetworkSecurityPolicy(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/networksecuritypolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetNetworkSecurityPolicy',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete NetworkSecurityPolicy object */
  public DeleteNetworkSecurityPolicy(O_Name, stagingID: string = ""):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/networksecuritypolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteNetworkSecurityPolicy',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update NetworkSecurityPolicy object */
  public UpdateNetworkSecurityPolicy(O_Name, body: ISecurityNetworkSecurityPolicy, stagingID: string = "", previousVal: ISecurityNetworkSecurityPolicy = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/networksecuritypolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateNetworkSecurityPolicy',
      objType: 'SecurityNetworkSecurityPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecurityNetworkSecurityPolicy(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public ListSecurityGroup(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListSecurityGroup',
      objType: 'SecuritySecurityGroupList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecuritySecurityGroupList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AddSecurityGroup(body: ISecuritySecurityGroup, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddSecurityGroup',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecuritySecurityGroup(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public GetSecurityGroup(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetSecurityGroup',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public DeleteSecurityGroup(O_Name, stagingID: string = ""):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteSecurityGroup',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public UpdateSecurityGroup(O_Name, body: ISecuritySecurityGroup, stagingID: string = "", previousVal: ISecuritySecurityGroup = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/tenant/{O.Tenant}/security-groups/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateSecurityGroup',
      objType: 'SecuritySecurityGroup',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SecuritySecurityGroup(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: ISecuritySecurityGroup | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch App objects. Supports WebSockets or HTTP long poll */
  public WatchApp_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/apps';
    const opts = {
      eventID: 'WatchApp_1',
      objType: 'SecurityAutoMsgAppWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FirewallProfile objects. Supports WebSockets or HTTP long poll */
  public WatchFirewallProfile_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/firewallprofiles';
    const opts = {
      eventID: 'WatchFirewallProfile_1',
      objType: 'SecurityAutoMsgFirewallProfileWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch NetworkSecurityPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchNetworkSecurityPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgNetworkSecurityPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/networksecuritypolicies';
    const opts = {
      eventID: 'WatchNetworkSecurityPolicy_1',
      objType: 'SecurityAutoMsgNetworkSecurityPolicyWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgNetworkSecurityPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects. Supports WebSockets or HTTP long poll */
  public WatchSecurityGroup_1(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/security-groups';
    const opts = {
      eventID: 'WatchSecurityGroup_1',
      objType: 'SecurityAutoMsgSecurityGroupWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch App objects. Supports WebSockets or HTTP long poll */
  public WatchApp(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/apps';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchApp',
      objType: 'SecurityAutoMsgAppWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgAppWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FirewallProfile objects. Supports WebSockets or HTTP long poll */
  public WatchFirewallProfile(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/firewallprofiles';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchFirewallProfile',
      objType: 'SecurityAutoMsgFirewallProfileWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgFirewallProfileWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch NetworkSecurityPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchNetworkSecurityPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgNetworkSecurityPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/networksecuritypolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchNetworkSecurityPolicy',
      objType: 'SecurityAutoMsgNetworkSecurityPolicyWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgNetworkSecurityPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects. Supports WebSockets or HTTP long poll */
  public WatchSecurityGroup(queryParam: any = null, stagingID: string = ""):Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/tenant/{O.Tenant}/security-groups';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchSecurityGroup',
      objType: 'SecurityAutoMsgSecurityGroupWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}