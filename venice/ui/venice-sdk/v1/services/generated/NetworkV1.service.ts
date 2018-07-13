import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { INetworkLbPolicyList,IApiStatus,INetworkLbPolicy,NetworkLbPolicy,INetworkNetworkList,INetworkNetwork,NetworkNetwork,INetworkServiceList,INetworkService,NetworkService,INetworkAutoMsgLbPolicyWatchHelper,INetworkAutoMsgNetworkWatchHelper,INetworkAutoMsgServiceWatchHelper } from '../../models/generated/network';

@Injectable()
export class NetworkV1Service extends AbstractService {
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

  /** Retreives a list of LbPolicy objects */
  public ListLbPolicy():Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListLbPolicy') as Observable<{body: INetworkLbPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Creates a new LbPolicy object */
  public AddLbPolicy(body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the LbPolicy object */
  public GetLbPolicy(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the LbPolicy object */
  public DeleteLbPolicy(O_Name):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateLbPolicy(O_Name, body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateLbPolicy') as Observable<{body: INetworkLbPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives a list of Network objects */
  public ListNetwork():Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListNetwork') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Creates a new Network object */
  public AddNetwork(body: NetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the Network object */
  public GetNetwork(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the Network object */
  public DeleteNetwork(O_Name):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateNetwork(O_Name, body: NetworkNetwork):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives a list of Service objects */
  public ListService():Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListService') as Observable<{body: INetworkServiceList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Creates a new Service object */
  public AddService(body: NetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Retreives the Service object */
  public GetService(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Deletes the Service object */
  public DeleteService(O_Name):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateService(O_Name, body: NetworkService):Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateService') as Observable<{body: INetworkService | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch for changes to LbPolicy objects */
  public WatchLbPolicy():Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchLbPolicy') as Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch for changes to Network objects */
  public WatchNetwork():Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchNetwork') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch for changes to Service objects */
  public WatchService():Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchService') as Observable<{body: INetworkAutoMsgServiceWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}