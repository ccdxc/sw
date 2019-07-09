import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { IClusterCluster,ClusterCluster,IApiStatus,ApiStatus,ClusterClusterAuthBootstrapRequest,IClusterClusterAuthBootstrapRequest,ClusterUpdateTLSConfigRequest,IClusterUpdateTLSConfigRequest,IClusterHostList,ClusterHostList,IClusterHost,ClusterHost,IClusterNodeList,ClusterNodeList,IClusterNode,ClusterNode,IClusterSmartNICList,ClusterSmartNICList,IClusterSmartNIC,ClusterSmartNIC,IClusterTenantList,ClusterTenantList,IClusterTenant,ClusterTenant,IClusterVersion,ClusterVersion,IClusterAutoMsgClusterWatchHelper,ClusterAutoMsgClusterWatchHelper,IClusterAutoMsgHostWatchHelper,ClusterAutoMsgHostWatchHelper,IClusterAutoMsgNodeWatchHelper,ClusterAutoMsgNodeWatchHelper,IClusterAutoMsgSmartNICWatchHelper,ClusterAutoMsgSmartNICWatchHelper,IClusterAutoMsgTenantWatchHelper,ClusterAutoMsgTenantWatchHelper,IClusterAutoMsgVersionWatchHelper,ClusterAutoMsgVersionWatchHelper } from '../../models/generated/cluster';

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
    const opts = {
      eventID: 'GetCluster',
      objType: 'ClusterCluster',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Cluster object */
  public UpdateCluster(body: IClusterCluster, stagingID: string = "", previousVal: IClusterCluster = null, trimObject: boolean = true):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster';
    const opts = {
      eventID: 'UpdateCluster',
      objType: 'ClusterCluster',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterCluster(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  public AuthBootstrapComplete(body: IClusterClusterAuthBootstrapRequest, stagingID: string = "", trimObject: boolean = true):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster/AuthBootstrapComplete';
    const opts = {
      eventID: 'AuthBootstrapComplete',
      objType: 'ClusterCluster',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterClusterAuthBootstrapRequest(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  public UpdateTLSConfig(body: IClusterUpdateTLSConfigRequest, stagingID: string = "", trimObject: boolean = true):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/cluster/UpdateTLSConfig';
    const opts = {
      eventID: 'UpdateTLSConfig',
      objType: 'ClusterCluster',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterUpdateTLSConfigRequest(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Host objects */
  public ListHost(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterHostList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts';
    const opts = {
      eventID: 'ListHost',
      objType: 'ClusterHostList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterHostList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Host object */
  public AddHost(body: IClusterHost, stagingID: string = "", trimObject: boolean = true):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts';
    const opts = {
      eventID: 'AddHost',
      objType: 'ClusterHost',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterHost(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Host object */
  public GetHost(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetHost',
      objType: 'ClusterHost',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Host object */
  public DeleteHost(O_Name, stagingID: string = ""):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteHost',
      objType: 'ClusterHost',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Host object */
  public UpdateHost(O_Name, body: IClusterHost, stagingID: string = "", previousVal: IClusterHost = null, trimObject: boolean = true):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateHost',
      objType: 'ClusterHost',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterHost(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Node objects */
  public ListNode(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterNodeList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes';
    const opts = {
      eventID: 'ListNode',
      objType: 'ClusterNodeList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterNodeList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Node object */
  public AddNode(body: IClusterNode, stagingID: string = "", trimObject: boolean = true):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes';
    const opts = {
      eventID: 'AddNode',
      objType: 'ClusterNode',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterNode(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Node object */
  public GetNode(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetNode',
      objType: 'ClusterNode',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Node object */
  public DeleteNode(O_Name, stagingID: string = ""):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteNode',
      objType: 'ClusterNode',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Node object */
  public UpdateNode(O_Name, body: IClusterNode, stagingID: string = "", previousVal: IClusterNode = null, trimObject: boolean = true):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateNode',
      objType: 'ClusterNode',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterNode(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List SmartNIC objects */
  public ListSmartNIC(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterSmartNICList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics';
    const opts = {
      eventID: 'ListSmartNIC',
      objType: 'ClusterSmartNICList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterSmartNICList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SmartNIC object */
  public GetSmartNIC(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetSmartNIC',
      objType: 'ClusterSmartNIC',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete SmartNIC object */
  public DeleteSmartNIC(O_Name, stagingID: string = ""):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteSmartNIC',
      objType: 'ClusterSmartNIC',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update SmartNIC object */
  public UpdateSmartNIC(O_Name, body: IClusterSmartNIC, stagingID: string = "", previousVal: IClusterSmartNIC = null, trimObject: boolean = true):Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateSmartNIC',
      objType: 'ClusterSmartNIC',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterSmartNIC(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterSmartNIC | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Tenant objects */
  public ListTenant(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterTenantList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants';
    const opts = {
      eventID: 'ListTenant',
      objType: 'ClusterTenantList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterTenantList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Tenant object */
  public AddTenant(body: IClusterTenant, stagingID: string = "", trimObject: boolean = true):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants';
    const opts = {
      eventID: 'AddTenant',
      objType: 'ClusterTenant',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterTenant(body))
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Tenant object */
  public GetTenant(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetTenant',
      objType: 'ClusterTenant',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Tenant object */
  public DeleteTenant(O_Name, stagingID: string = ""):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteTenant',
      objType: 'ClusterTenant',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Tenant object */
  public UpdateTenant(O_Name, body: IClusterTenant, stagingID: string = "", previousVal: IClusterTenant = null, trimObject: boolean = true):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateTenant',
      objType: 'ClusterTenant',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterTenant(body), previousVal)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Version object */
  public GetVersion(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterVersion | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/version';
    const opts = {
      eventID: 'GetVersion',
      objType: 'ClusterVersion',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterVersion | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Cluster objects. Supports WebSockets or HTTP long poll */
  public WatchCluster(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgClusterWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/cluster';
    const opts = {
      eventID: 'WatchCluster',
      objType: 'ClusterAutoMsgClusterWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgClusterWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Host objects. Supports WebSockets or HTTP long poll */
  public WatchHost(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgHostWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/hosts';
    const opts = {
      eventID: 'WatchHost',
      objType: 'ClusterAutoMsgHostWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgHostWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Node objects. Supports WebSockets or HTTP long poll */
  public WatchNode(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgNodeWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/nodes';
    const opts = {
      eventID: 'WatchNode',
      objType: 'ClusterAutoMsgNodeWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgNodeWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch SmartNIC objects. Supports WebSockets or HTTP long poll */
  public WatchSmartNIC(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgSmartNICWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/smartnics';
    const opts = {
      eventID: 'WatchSmartNIC',
      objType: 'ClusterAutoMsgSmartNICWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgSmartNICWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Tenant objects. Supports WebSockets or HTTP long poll */
  public WatchTenant(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgTenantWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/tenants';
    const opts = {
      eventID: 'WatchTenant',
      objType: 'ClusterAutoMsgTenantWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgTenantWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Version objects. Supports WebSockets or HTTP long poll */
  public WatchVersion(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgVersionWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/version';
    const opts = {
      eventID: 'WatchVersion',
      objType: 'ClusterAutoMsgVersionWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgVersionWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}