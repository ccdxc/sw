import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { INetworkLbPolicyList,IApiStatus,INetworkLbPolicy,INetworkNetworkList,INetworkNetwork,INetworkServiceList,INetworkService,INetworkAutoMsgLbPolicyWatchHelper,INetworkAutoMsgNetworkWatchHelper,INetworkAutoMsgServiceWatchHelper } from '../../models/generated/network';

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

  /** List LbPolicy objects */
  public ListLbPolicy_1(queryParam: any = null):Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy';
    return this.invokeAJAXGetCall(url, queryParam, 'ListLbPolicy_1') as Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create LbPolicy object */
  public AddLbPolicy_1(body: INetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy';
    return this.invokeAJAXPostCall(url, body, 'AddLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get LbPolicy object */
  public GetLbPolicy_1(O_Name, queryParam: any = null):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete LbPolicy object */
  public DeleteLbPolicy_1(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update LbPolicy object */
  public UpdateLbPolicy_1(O_Name, body: INetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork_1(queryParam: any = null):Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    return this.invokeAJAXGetCall(url, queryParam, 'ListNetwork_1') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Network object */
  public AddNetwork_1(body: INetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    return this.invokeAJAXPostCall(url, body, 'AddNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork_1(O_Name, queryParam: any = null):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Network object */
  public DeleteNetwork_1(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Network object */
  public UpdateNetwork_1(O_Name, body: INetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Service objects */
  public ListService_1(queryParam: any = null):Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services';
    return this.invokeAJAXGetCall(url, queryParam, 'ListService_1') as Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Service object */
  public AddService_1(body: INetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services';
    return this.invokeAJAXPostCall(url, body, 'AddService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Service object */
  public GetService_1(O_Name, queryParam: any = null):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Service object */
  public DeleteService_1(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Service object */
  public UpdateService_1(O_Name, body: INetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List LbPolicy objects */
  public ListLbPolicy(queryParam: any = null):Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListLbPolicy') as Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create LbPolicy object */
  public AddLbPolicy(body: INetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get LbPolicy object */
  public GetLbPolicy(O_Name, queryParam: any = null):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete LbPolicy object */
  public DeleteLbPolicy(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update LbPolicy object */
  public UpdateLbPolicy(O_Name, body: INetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork(queryParam: any = null):Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListNetwork') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Network object */
  public AddNetwork(body: INetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork(O_Name, queryParam: any = null):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Network object */
  public DeleteNetwork(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Network object */
  public UpdateNetwork(O_Name, body: INetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Service objects */
  public ListService(queryParam: any = null):Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListService') as Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Service object */
  public AddService(body: INetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Service object */
  public GetService(O_Name, queryParam: any = null):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Service object */
  public DeleteService(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Service object */
  public UpdateService(O_Name, body: INetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch LbPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchLbPolicy_1(queryParam: any = null):Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/lb-policy';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchLbPolicy_1') as Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects. Supports WebSockets or HTTP long poll */
  public WatchNetwork_1(queryParam: any = null):Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/networks';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchNetwork_1') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Service objects. Supports WebSockets or HTTP long poll */
  public WatchService_1(queryParam: any = null):Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/services';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchService_1') as Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch LbPolicy objects. Supports WebSockets or HTTP long poll */
  public WatchLbPolicy(queryParam: any = null):Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchLbPolicy') as Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects. Supports WebSockets or HTTP long poll */
  public WatchNetwork(queryParam: any = null):Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchNetwork') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Service objects. Supports WebSockets or HTTP long poll */
  public WatchService(queryParam: any = null):Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchService') as Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}