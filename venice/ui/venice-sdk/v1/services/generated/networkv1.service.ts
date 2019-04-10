import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { INetworkNetworkList,IApiStatus,INetworkNetwork,INetworkAutoMsgNetworkWatchHelper } from '../../models/generated/network';

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

  /** List Network objects */
  public ListNetwork_1(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListNetwork_1') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork_1(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/networks/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetNetwork_1') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Network objects */
  public ListNetwork(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListNetwork') as Observable<{body: INetworkNetworkList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Network object */
  public GetNetwork(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/tenant/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetNetwork') as Observable<{body: INetworkNetwork | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects. Supports WebSockets or HTTP long poll */
  public WatchNetwork_1(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/networks';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchNetwork_1') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Network objects. Supports WebSockets or HTTP long poll */
  public WatchNetwork(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/tenant/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchNetwork') as Observable<{body: INetworkAutoMsgNetworkWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}