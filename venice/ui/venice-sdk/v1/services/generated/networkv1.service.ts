import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { INetworkNetworkList,NetworkNetworkList,IApiStatus,ApiStatus,INetworkNetwork,NetworkNetwork,INetworkNetworkInterfaceList,NetworkNetworkInterfaceList,INetworkNetworkInterface,NetworkNetworkInterface,INetworkAutoMsgNetworkWatchHelper,NetworkAutoMsgNetworkWatchHelper,INetworkAutoMsgNetworkInterfaceWatchHelper,NetworkAutoMsgNetworkInterfaceWatchHelper } from '../../models/generated/network';

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
  
  /** List NetworkInterface objects */
  public ListNetworkInterface(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkNetworkInterfaceList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/smartnics';
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
    let url = this['baseUrlAndPort'] + '/configs/network/v1/smartnics/{O.Name}';
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
  
  /** Watch NetworkInterface objects. Supports WebSockets or HTTP long poll */
  public WatchNetworkInterface(queryParam: any = null, stagingID: string = ""):Observable<{body: INetworkAutoMsgNetworkInterfaceWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/network/v1/watch/smartnics';
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