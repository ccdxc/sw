import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IClusterCluster,IApiStatus,IClusterClusterAuthBootstrapRequest,IClusterHostList,IClusterHost,IClusterNodeList,IClusterNode,IClusterSmartNICList,IClusterSmartNIC,IClusterTenantList,IClusterTenant,IClusterAutoMsgClusterWatchHelper,IClusterAutoMsgHostWatchHelper,IClusterAutoMsgNodeWatchHelper,IClusterAutoMsgSmartNICWatchHelper,IClusterAutoMsgTenantWatchHelper } from '../../models/generated/cluster';

@Injectable()
export class Clusterv1Service extends AbstractService {
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

  /** Get Cluster object */
  public GetCluster(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetCluster') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Cluster object */
  public DeleteCluster(stagingID: string = ""):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteCluster') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Cluster object */
  public UpdateCluster(body: IClusterCluster, stagingID: string = ""):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateCluster') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  public AuthBootstrapComplete(body: IClusterClusterAuthBootstrapRequest, stagingID: string = ""):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster/AuthBootstrapComplete';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AuthBootstrapComplete') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Host objects */
  public ListHost(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterHostList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListHost') as Observable<{body: IClusterHostList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Host object */
  public AddHost(body: IClusterHost, stagingID: string = ""):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Host object */
  public GetHost(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Host object */
  public DeleteHost(O_Name, stagingID: string = ""):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Host object */
  public UpdateHost(O_Name, body: IClusterHost, stagingID: string = ""):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Node objects */
  public ListNode(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterNodeList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListNode') as Observable<{body: IClusterNodeList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Node object */
  public AddNode(body: IClusterNode, stagingID: string = ""):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Node object */
  public GetNode(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Node object */
  public DeleteNode(O_Name, stagingID: string = ""):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Node object */
  public UpdateNode(O_Name, body: IClusterNode, stagingID: string = ""):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SmartNIC objects */
  public ListSmartNIC(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterSmartNICList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListSmartNIC') as Observable<{body: IClusterSmartNICList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SmartNIC object */
  public GetSmartNIC(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SmartNIC object */
  public DeleteSmartNIC(O_Name, stagingID: string = ""):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SmartNIC object */
  public UpdateSmartNIC(O_Name, body: IClusterSmartNIC, stagingID: string = ""):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Tenant objects */
  public ListTenant(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterTenantList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'ListTenant') as Observable<{body: IClusterTenantList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Tenant object */
  public AddTenant(body: IClusterTenant, stagingID: string = ""):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'AddTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Tenant object */
  public GetTenant(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Tenant object */
  public DeleteTenant(O_Name, stagingID: string = ""):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXDeleteCall(url, 'DeleteTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Tenant object */
  public UpdateTenant(O_Name, body: IClusterTenant, stagingID: string = ""):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPutCall(url, body, 'UpdateTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Cluster objects. Supports WebSockets or HTTP long poll */
  public WatchCluster(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgClusterWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/cluster';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchCluster') as Observable<{body: IClusterAutoMsgClusterWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Host objects. Supports WebSockets or HTTP long poll */
  public WatchHost(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgHostWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/hosts';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchHost') as Observable<{body: IClusterAutoMsgHostWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Node objects. Supports WebSockets or HTTP long poll */
  public WatchNode(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgNodeWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/nodes';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchNode') as Observable<{body: IClusterAutoMsgNodeWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SmartNIC objects. Supports WebSockets or HTTP long poll */
  public WatchSmartNIC(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgSmartNICWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/smartnics';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSmartNIC') as Observable<{body: IClusterAutoMsgSmartNICWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Tenant objects. Supports WebSockets or HTTP long poll */
  public WatchTenant(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgTenantWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/tenants';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'WatchTenant') as Observable<{body: IClusterAutoMsgTenantWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}