import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { INetworkLbPolicyList,INetworkLbPolicy,NetworkLbPolicy,INetworkNetworkList,INetworkNetwork,NetworkNetwork,INetworkServiceList,INetworkService,NetworkService,INetworkAutoMsgLbPolicyWatchHelper,INetworkAutoMsgNetworkWatchHelper,INetworkAutoMsgServiceWatchHelper } from '../../models/generated/network';

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

  /** List LbPolicy objects */
  public AutoListLbPolicy-1():Observable<{body: INetworkLbPolicyList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy';
    return this.invokeAJAXGetCall(url, 'AutoListLbPolicy-1') as Observable<{body: INetworkLbPolicyList | Error, statusCode: number}>;
  }
  
  /** Create LbPolicy object */
  public AutoAddLbPolicy-1(body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddLbPolicy-1') as Observable<{body: INetworkLbPolicy | Error, statusCode: number}>;
  }
  
  /** Get LbPolicy object */
  public AutoGetLbPolicy-1(O_Name):Observable<{body: INetworkLbPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetLbPolicy-1') as Observable<{body: INetworkLbPolicy | Error, statusCode: number}>;
  }
  
  /** Delete LbPolicy object */
  public AutoDeleteLbPolicy-1(O_Name):Observable<{body: INetworkLbPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteLbPolicy-1') as Observable<{body: INetworkLbPolicy | Error, statusCode: number}>;
  }
  
  /** Update LbPolicy object */
  public AutoUpdateLbPolicy-1(O_Name, body: NetworkLbPolicy):Observable<{body: INetworkLbPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/lb-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateLbPolicy-1') as Observable<{body: INetworkLbPolicy | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public AutoListNetwork-1():Observable<{body: INetworkNetworkList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    return this.invokeAJAXGetCall(url, 'AutoListNetwork-1') as Observable<{body: INetworkNetworkList | Error, statusCode: number}>;
  }
  
  /** Create Network object */
  public AutoAddNetwork-1(body: NetworkNetwork):Observable<{body: INetworkNetwork | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddNetwork-1') as Observable<{body: INetworkNetwork | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public AutoGetNetwork-1(O_Name):Observable<{body: INetworkNetwork | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetNetwork-1') as Observable<{body: INetworkNetwork | Error, statusCode: number}>;
  }
  
  /** Delete Network object */
  public AutoDeleteNetwork-1(O_Name):Observable<{body: INetworkNetwork | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteNetwork-1') as Observable<{body: INetworkNetwork | Error, statusCode: number}>;
  }
  
  /** Update Network object */
  public AutoUpdateNetwork-1(O_Name, body: NetworkNetwork):Observable<{body: INetworkNetwork | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateNetwork-1') as Observable<{body: INetworkNetwork | Error, statusCode: number}>;
  }
  
  /** List Service objects */
  public AutoListService-1():Observable<{body: INetworkServiceList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services';
    return this.invokeAJAXGetCall(url, 'AutoListService-1') as Observable<{body: INetworkServiceList | Error, statusCode: number}>;
  }
  
  /** Create Service object */
  public AutoAddService-1(body: NetworkService):Observable<{body: INetworkService | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddService-1') as Observable<{body: INetworkService | Error, statusCode: number}>;
  }
  
  /** Get Service object */
  public AutoGetService-1(O_Name):Observable<{body: INetworkService | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetService-1') as Observable<{body: INetworkService | Error, statusCode: number}>;
  }
  
  /** Delete Service object */
  public AutoDeleteService-1(O_Name):Observable<{body: INetworkService | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteService-1') as Observable<{body: INetworkService | Error, statusCode: number}>;
  }
  
  /** Update Service object */
  public AutoUpdateService-1(O_Name, body: NetworkService):Observable<{body: INetworkService | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/services/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateService-1') as Observable<{body: INetworkService | Error, statusCode: number}>;
  }
  
  /** Watch LbPolicy objects */
  public AutoWatchLbPolicy-1():Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/lb-policy';
    return this.invokeAJAXGetCall(url, 'AutoWatchLbPolicy-1') as Observable<{body: INetworkAutoMsgLbPolicyWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch Network objects */
  public AutoWatchNetwork-1():Observable<{body: INetworkAutoMsgNetworkWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/networks';
    return this.invokeAJAXGetCall(url, 'AutoWatchNetwork-1') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch Service objects */
  public AutoWatchService-1():Observable<{body: INetworkAutoMsgServiceWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/services';
    return this.invokeAJAXGetCall(url, 'AutoWatchService-1') as Observable<{body: INetworkAutoMsgServiceWatchHelper | Error, statusCode: number}>;
  }
  
}