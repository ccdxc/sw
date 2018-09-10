import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { INetworkLbPolicyList,IApiStatus,INetworkLbPolicy,NetworkLbPolicy,INetworkNetworkList,INetworkNetwork,NetworkNetwork,INetworkServiceList,INetworkService,NetworkService,INetworkAutoMsgLbPolicyWatchHelper,INetworkAutoMsgNetworkWatchHelper,INetworkAutoMsgServiceWatchHelper } from '../../models/generated/network';

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
  public ListLbPolicy_1():Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy';
    return this.invokeAJAXGetCall(url, 'ListLbPolicy_1') as Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create LbPolicy object */
  public AddLbPolicy_1(body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get LbPolicy object */
  public GetLbPolicy_1(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete LbPolicy object */
  public DeleteLbPolicy_1(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update LbPolicy object */
  public UpdateLbPolicy_1(O_Name, body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateLbPolicy_1') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork_1():Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    return this.invokeAJAXGetCall(url, 'ListNetwork_1') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Network object */
  public AddNetwork_1(body: NetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork_1(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Network object */
  public DeleteNetwork_1(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Network object */
  public UpdateNetwork_1(O_Name, body: NetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Service objects */
  public ListService_1():Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services';
    return this.invokeAJAXGetCall(url, 'ListService_1') as Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Service object */
  public AddService_1(body: NetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Service object */
  public GetService_1(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Service object */
  public DeleteService_1(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Service object */
  public UpdateService_1(O_Name, body: NetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateService_1') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List LbPolicy objects */
  public ListLbPolicy():Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListLbPolicy') as Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create LbPolicy object */
  public AddLbPolicy(body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get LbPolicy object */
  public GetLbPolicy(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete LbPolicy object */
  public DeleteLbPolicy(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update LbPolicy object */
  public UpdateLbPolicy(O_Name, body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork():Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListNetwork') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Network object */
  public AddNetwork(body: NetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Network object */
  public DeleteNetwork(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Network object */
  public UpdateNetwork(O_Name, body: NetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Service objects */
  public ListService():Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListService') as Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Service object */
  public AddService(body: NetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Service object */
  public GetService(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Service object */
  public DeleteService(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Service object */
  public UpdateService(O_Name, body: NetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch LbPolicy objects */
  public WatchLbPolicy_1():Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/lb-policy';
    return this.invokeAJAXGetCall(url, 'WatchLbPolicy_1') as Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects */
  public WatchNetwork_1():Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/networks';
    return this.invokeAJAXGetCall(url, 'WatchNetwork_1') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Service objects */
  public WatchService_1():Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/services';
    return this.invokeAJAXGetCall(url, 'WatchService_1') as Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch LbPolicy objects */
  public WatchLbPolicy():Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchLbPolicy') as Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects */
  public WatchNetwork():Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchNetwork') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Service objects */
  public WatchService():Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchService') as Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}