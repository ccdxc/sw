import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IClusterCluster,IApiStatus,ClusterCluster,ClusterClusterAuthBootstrapRequest,IClusterHostList,IClusterHost,ClusterHost,IClusterNodeList,IClusterNode,ClusterNode,IClusterSmartNICList,IClusterSmartNIC,ClusterSmartNIC,IClusterTenantList,IClusterTenant,ClusterTenant,IClusterAutoMsgClusterWatchHelper,IClusterAutoMsgHostWatchHelper,IClusterAutoMsgNodeWatchHelper,IClusterAutoMsgSmartNICWatchHelper,IClusterAutoMsgTenantWatchHelper } from '../../models/generated/cluster';

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
  public GetCluster(queryParam: any = null):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    return this.invokeAJAXGetCall(url, queryParam, 'GetCluster') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Cluster object */
  public DeleteCluster():Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    return this.invokeAJAXDeleteCall(url, 'DeleteCluster') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Cluster object */
  public UpdateCluster(body: ClusterCluster):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateCluster') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  public AuthBootstrapComplete(body: ClusterClusterAuthBootstrapRequest):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster/AuthBootstrapComplete';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AuthBootstrapComplete') as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Host objects */
  public ListHost(queryParam: any = null):Observable<{body: IClusterHostList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts';
    return this.invokeAJAXGetCall(url, queryParam, 'ListHost') as Observable<{body: IClusterHostList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Host object */
  public AddHost(body: ClusterHost):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Host object */
  public GetHost(O_Name, queryParam: any = null):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Host object */
  public DeleteHost(O_Name):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Host object */
  public UpdateHost(O_Name, body: ClusterHost):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateHost') as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Node objects */
  public ListNode(queryParam: any = null):Observable<{body: IClusterNodeList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes';
    return this.invokeAJAXGetCall(url, queryParam, 'ListNode') as Observable<{body: IClusterNodeList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Node object */
  public AddNode(body: ClusterNode):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Node object */
  public GetNode(O_Name, queryParam: any = null):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Node object */
  public DeleteNode(O_Name):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Node object */
  public UpdateNode(O_Name, body: ClusterNode):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateNode') as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SmartNIC objects */
  public ListSmartNIC(queryParam: any = null):Observable<{body: IClusterSmartNICList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics';
    return this.invokeAJAXGetCall(url, queryParam, 'ListSmartNIC') as Observable<{body: IClusterSmartNICList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create SmartNIC object */
  public AddSmartNIC(body: ClusterSmartNIC):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SmartNIC object */
  public GetSmartNIC(O_Name, queryParam: any = null):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SmartNIC object */
  public DeleteSmartNIC(O_Name):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SmartNIC object */
  public UpdateSmartNIC(O_Name, body: ClusterSmartNIC):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSmartNIC') as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Tenant objects */
  public ListTenant(queryParam: any = null):Observable<{body: IClusterTenantList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants';
    return this.invokeAJAXGetCall(url, queryParam, 'ListTenant') as Observable<{body: IClusterTenantList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Tenant object */
  public AddTenant(body: ClusterTenant):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Tenant object */
  public GetTenant(O_Name, queryParam: any = null):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Tenant object */
  public DeleteTenant(O_Name):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Tenant object */
  public UpdateTenant(O_Name, body: ClusterTenant):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTenant') as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Cluster objects */
  public WatchCluster(queryParam: any = null):Observable<{body: IClusterAutoMsgClusterWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/cluster';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchCluster') as Observable<{body: IClusterAutoMsgClusterWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Host objects */
  public WatchHost(queryParam: any = null):Observable<{body: IClusterAutoMsgHostWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/hosts';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchHost') as Observable<{body: IClusterAutoMsgHostWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Node objects */
  public WatchNode(queryParam: any = null):Observable<{body: IClusterAutoMsgNodeWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/nodes';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchNode') as Observable<{body: IClusterAutoMsgNodeWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SmartNIC objects */
  public WatchSmartNIC(queryParam: any = null):Observable<{body: IClusterAutoMsgSmartNICWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/smartnics';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchSmartNIC') as Observable<{body: IClusterAutoMsgSmartNICWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Tenant objects */
  public WatchTenant(queryParam: any = null):Observable<{body: IClusterAutoMsgTenantWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/tenants';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchTenant') as Observable<{body: IClusterAutoMsgTenantWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}