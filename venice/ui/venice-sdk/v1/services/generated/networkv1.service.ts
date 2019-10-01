import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { INetworkIPAMPolicyList,NetworkIPAMPolicyList,IApiStatus,ApiStatus,INetworkIPAMPolicy,NetworkIPAMPolicy,INetworkNetworkInterfaceList,NetworkNetworkInterfaceList,INetworkNetworkInterface,NetworkNetworkInterface,INetworkNetworkList,NetworkNetworkList,INetworkNetwork,NetworkNetwork,INetworkAutoMsgIPAMPolicyWatchHelper,NetworkAutoMsgIPAMPolicyWatchHelper,INetworkAutoMsgNetworkInterfaceWatchHelper,NetworkAutoMsgNetworkInterfaceWatchHelper,INetworkAutoMsgNetworkWatchHelper,NetworkAutoMsgNetworkWatchHelper } from '../../models/generated/network';

@Injectable()
export class Networkv1Service extends AbstractService {
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

  /** List IPAMPolicy objects */
  public ListIPAMPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkIPAMPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/ipam-policies';
    const opts = {
      eventID: 'ListIPAMPolicy_1',
      objType: 'NetworkIPAMPolicyList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkIPAMPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create IPAMPolicy object */
  public AddIPAMPolicy_1(body: INetworkIPAMPolicy, stagingID: string = "", trimObject: boolean = true):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/ipam-policies';
    const opts = {
      eventID: 'AddIPAMPolicy_1',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new NetworkIPAMPolicy(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get IPAMPolicy object */
  public GetIPAMPolicy_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/ipam-policies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetIPAMPolicy_1',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete IPAMPolicy object */
  public DeleteIPAMPolicy_1(O_Name, stagingID: string = ""):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/ipam-policies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteIPAMPolicy_1',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update IPAMPolicy object */
  public UpdateIPAMPolicy_1(O_Name, body: INetworkIPAMPolicy, stagingID: string = "", previousVal: INetworkIPAMPolicy = null, trimObject: boolean = true):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/ipam-policies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateIPAMPolicy_1',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new NetworkIPAMPolicy(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List NetworkInterface objects */
  public ListNetworkInterface(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkInterfaceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networkinterfaces';
    const opts = {
      eventID: 'ListNetworkInterface',
      objType: 'NetworkNetworkInterfaceList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkNetworkInterfaceList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get NetworkInterface object */
  public GetNetworkInterface(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkInterface | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networkinterfaces/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetNetworkInterface',
      objType: 'NetworkNetworkInterface',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkNetworkInterface | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork_1(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    const opts = {
      eventID: 'ListNetwork_1',
      objType: 'NetworkNetworkList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetNetwork_1',
      objType: 'NetworkNetwork',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List IPAMPolicy objects */
  public ListIPAMPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkIPAMPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/ipam-policies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListIPAMPolicy',
      objType: 'NetworkIPAMPolicyList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkIPAMPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create IPAMPolicy object */
  public AddIPAMPolicy(body: INetworkIPAMPolicy, stagingID: string = "", trimObject: boolean = true):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/ipam-policies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'AddIPAMPolicy',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new NetworkIPAMPolicy(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get IPAMPolicy object */
  public GetIPAMPolicy(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/ipam-policies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetIPAMPolicy',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete IPAMPolicy object */
  public DeleteIPAMPolicy(O_Name, stagingID: string = ""):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/ipam-policies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteIPAMPolicy',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update IPAMPolicy object */
  public UpdateIPAMPolicy(O_Name, body: INetworkIPAMPolicy, stagingID: string = "", previousVal: INetworkIPAMPolicy = null, trimObject: boolean = true):Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/ipam-policies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateIPAMPolicy',
      objType: 'NetworkIPAMPolicy',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new NetworkIPAMPolicy(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: INetworkIPAMPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'ListNetwork',
      objType: 'NetworkNetworkList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetNetwork',
      objType: 'NetworkNetwork',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch IPAMPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchIPAMPolicy_1(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgIPAMPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/ipam-policies';
    const opts = {
      eventID: 'WatchIPAMPolicy_1',
      objType: 'NetworkAutoMsgIPAMPolicyWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkAutoMsgIPAMPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch NetworkInterface objects. Supports WebSockets or HTTP long poll */
  public WatchNetworkInterface(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgNetworkInterfaceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/networkinterfaces';
    const opts = {
      eventID: 'WatchNetworkInterface',
      objType: 'NetworkAutoMsgNetworkInterfaceWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkAutoMsgNetworkInterfaceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects. Supports WebSockets or HTTP long poll */
  public WatchNetwork_1(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/networks';
    const opts = {
      eventID: 'WatchNetwork_1',
      objType: 'NetworkAutoMsgNetworkWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch IPAMPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchIPAMPolicy(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgIPAMPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/ipam-policies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchIPAMPolicy',
      objType: 'NetworkAutoMsgIPAMPolicyWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkAutoMsgIPAMPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects. Supports WebSockets or HTTP long poll */
  public WatchNetwork(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    const opts = {
      eventID: 'WatchNetwork',
      objType: 'NetworkAutoMsgNetworkWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}