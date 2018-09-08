import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISecurityCertificateList,ISecurityCertificate,SecurityCertificate,ISecuritySecurityGroupList,ISecuritySecurityGroup,SecuritySecurityGroup,ISecuritySGPolicyList,ISecuritySGPolicy,SecuritySGPolicy,ISecurityTrafficEncryptionPolicy,SecurityTrafficEncryptionPolicy,ISecurityAutoMsgCertificateWatchHelper,ISecurityAutoMsgSecurityGroupWatchHelper,ISecurityAutoMsgSGPolicyWatchHelper,ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper } from '../../models/generated/security';

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

  /** List Certificate objects */
  public AutoListCertificate-1():Observable<{body: ISecurityCertificateList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    return this.invokeAJAXGetCall(url, 'AutoListCertificate-1') as Observable<{body: ISecurityCertificateList | Error, statusCode: number}>;
  }
  
  /** Create Certificate object */
  public AutoAddCertificate-1(body: SecurityCertificate):Observable<{body: ISecurityCertificate | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddCertificate-1') as Observable<{body: ISecurityCertificate | Error, statusCode: number}>;
  }
  
  /** Get Certificate object */
  public AutoGetCertificate-1(O_Name):Observable<{body: ISecurityCertificate | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetCertificate-1') as Observable<{body: ISecurityCertificate | Error, statusCode: number}>;
  }
  
  /** Delete Certificate object */
  public AutoDeleteCertificate-1(O_Name):Observable<{body: ISecurityCertificate | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteCertificate-1') as Observable<{body: ISecurityCertificate | Error, statusCode: number}>;
  }
  
  /** Update Certificate object */
  public AutoUpdateCertificate-1(O_Name, body: SecurityCertificate):Observable<{body: ISecurityCertificate | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/certificates/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateCertificate-1') as Observable<{body: ISecurityCertificate | Error, statusCode: number}>;
  }
  
  /** List SecurityGroup objects */
  public AutoListSecurityGroup-1():Observable<{body: ISecuritySecurityGroupList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    return this.invokeAJAXGetCall(url, 'AutoListSecurityGroup-1') as Observable<{body: ISecuritySecurityGroupList | Error, statusCode: number}>;
  }
  
  /** Create SecurityGroup object */
  public AutoAddSecurityGroup-1(body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddSecurityGroup-1') as Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}>;
  }
  
  /** Get SecurityGroup object */
  public AutoGetSecurityGroup-1(O_Name):Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetSecurityGroup-1') as Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}>;
  }
  
  /** Delete SecurityGroup object */
  public AutoDeleteSecurityGroup-1(O_Name):Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteSecurityGroup-1') as Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}>;
  }
  
  /** Update SecurityGroup object */
  public AutoUpdateSecurityGroup-1(O_Name, body: SecuritySecurityGroup):Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/security-groups/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateSecurityGroup-1') as Observable<{body: ISecuritySecurityGroup | Error, statusCode: number}>;
  }
  
  /** List SGPolicy objects */
  public AutoListSGPolicy-1():Observable<{body: ISecuritySGPolicyList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    return this.invokeAJAXGetCall(url, 'AutoListSGPolicy-1') as Observable<{body: ISecuritySGPolicyList | Error, statusCode: number}>;
  }
  
  /** Create SGPolicy object */
  public AutoAddSGPolicy-1(body: SecuritySGPolicy):Observable<{body: ISecuritySGPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddSGPolicy-1') as Observable<{body: ISecuritySGPolicy | Error, statusCode: number}>;
  }
  
  /** Get SGPolicy object */
  public AutoGetSGPolicy-1(O_Name):Observable<{body: ISecuritySGPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetSGPolicy-1') as Observable<{body: ISecuritySGPolicy | Error, statusCode: number}>;
  }
  
  /** Delete SGPolicy object */
  public AutoDeleteSGPolicy-1(O_Name):Observable<{body: ISecuritySGPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteSGPolicy-1') as Observable<{body: ISecuritySGPolicy | Error, statusCode: number}>;
  }
  
  /** Update SGPolicy object */
  public AutoUpdateSGPolicy-1(O_Name, body: SecuritySGPolicy):Observable<{body: ISecuritySGPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/sgpolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateSGPolicy-1') as Observable<{body: ISecuritySGPolicy | Error, statusCode: number}>;
  }
  
  /** Create TrafficEncryptionPolicy object */
  public AutoAddTrafficEncryptionPolicy-1(body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddTrafficEncryptionPolicy-1') as Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}>;
  }
  
  /** Get TrafficEncryptionPolicy object */
  public AutoGetTrafficEncryptionPolicy-1(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetTrafficEncryptionPolicy-1') as Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}>;
  }
  
  /** Delete TrafficEncryptionPolicy object */
  public AutoDeleteTrafficEncryptionPolicy-1(O_Name):Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteTrafficEncryptionPolicy-1') as Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}>;
  }
  
  /** Update TrafficEncryptionPolicy object */
  public AutoUpdateTrafficEncryptionPolicy-1(O_Name, body: SecurityTrafficEncryptionPolicy):Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/trafficEncryptionPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateTrafficEncryptionPolicy-1') as Observable<{body: ISecurityTrafficEncryptionPolicy | Error, statusCode: number}>;
  }
  
  /** Watch Certificate objects */
  public AutoWatchCertificate-1():Observable<{body: ISecurityAutoMsgCertificateWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/certificates';
    return this.invokeAJAXGetCall(url, 'AutoWatchCertificate-1') as Observable<{body: ISecurityAutoMsgCertificateWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch SecurityGroup objects */
  public AutoWatchSecurityGroup-1():Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/security-groups';
    return this.invokeAJAXGetCall(url, 'AutoWatchSecurityGroup-1') as Observable<{body: ISecurityAutoMsgSecurityGroupWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch SGPolicy objects */
  public AutoWatchSGPolicy-1():Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/sgpolicies';
    return this.invokeAJAXGetCall(url, 'AutoWatchSGPolicy-1') as Observable<{body: ISecurityAutoMsgSGPolicyWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch TrafficEncryptionPolicy objects */
  public AutoWatchTrafficEncryptionPolicy-1():Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/security/v1/watch/trafficEncryptionPolicy';
    return this.invokeAJAXGetCall(url, 'AutoWatchTrafficEncryptionPolicy-1') as Observable<{body: ISecurityAutoMsgTrafficEncryptionPolicyWatchHelper | Error, statusCode: number}>;
  }
  
}