import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { IClusterCluster,ClusterCluster,IApiStatus,ApiStatus,ClusterClusterAuthBootstrapRequest,IClusterClusterAuthBootstrapRequest,ClusterUpdateTLSConfigRequest,IClusterUpdateTLSConfigRequest,IClusterSnapshotRestore,ClusterSnapshotRestore,IClusterConfigurationSnapshot,ClusterConfigurationSnapshot,ClusterConfigurationSnapshotRequest,IClusterConfigurationSnapshotRequest,IClusterDistributedServiceCardList,ClusterDistributedServiceCardList,IClusterDistributedServiceCard,ClusterDistributedServiceCard,IClusterHostList,ClusterHostList,IClusterHost,ClusterHost,IClusterNodeList,ClusterNodeList,IClusterNode,ClusterNode,IClusterTenantList,ClusterTenantList,IClusterTenant,ClusterTenant,IClusterVersion,ClusterVersion,IClusterAutoMsgClusterWatchHelper,ClusterAutoMsgClusterWatchHelper,IClusterAutoMsgConfigurationSnapshotWatchHelper,ClusterAutoMsgConfigurationSnapshotWatchHelper,IClusterAutoMsgDistributedServiceCardWatchHelper,ClusterAutoMsgDistributedServiceCardWatchHelper,IClusterAutoMsgHostWatchHelper,ClusterAutoMsgHostWatchHelper,IClusterAutoMsgNodeWatchHelper,ClusterAutoMsgNodeWatchHelper,IClusterAutoMsgTenantWatchHelper,ClusterAutoMsgTenantWatchHelper,IClusterAutoMsgVersionWatchHelper,ClusterAutoMsgVersionWatchHelper } from '../../models/generated/cluster';

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
  public UpdateCluster(body: IClusterCluster, stagingID: string = "", previousVal: IClusterCluster = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterCluster(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Mark bootstrapping as complete for the cluster */
  public AuthBootstrapComplete(body: IClusterClusterAuthBootstrapRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterClusterAuthBootstrapRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TLS Configuration for cluster */
  public UpdateTLSConfig(body: IClusterUpdateTLSConfigRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterUpdateTLSConfigRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterCluster | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get SnapshotRestore object */
  public GetSnapshotRestore(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterSnapshotRestore | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-restore';
    const opts = {
      eventID: 'GetSnapshotRestore',
      objType: 'ClusterSnapshotRestore',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterSnapshotRestore | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Restore Configuration */
  public Restore(body: IClusterSnapshotRestore, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterSnapshotRestore | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-restore/restore';
    const opts = {
      eventID: 'Restore',
      objType: 'ClusterSnapshotRestore',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterSnapshotRestore(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterSnapshotRestore | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get ConfigurationSnapshot object */
  public GetConfigurationSnapshot(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-snapshot';
    const opts = {
      eventID: 'GetConfigurationSnapshot',
      objType: 'ClusterConfigurationSnapshot',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete ConfigurationSnapshot object */
  public DeleteConfigurationSnapshot(stagingID: string = ""):Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-snapshot';
    const opts = {
      eventID: 'DeleteConfigurationSnapshot',
      objType: 'ClusterConfigurationSnapshot',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create ConfigurationSnapshot object */
  public AddConfigurationSnapshot(body: IClusterConfigurationSnapshot, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-snapshot';
    const opts = {
      eventID: 'AddConfigurationSnapshot',
      objType: 'ClusterConfigurationSnapshot',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterConfigurationSnapshot(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update ConfigurationSnapshot object */
  public UpdateConfigurationSnapshot(body: IClusterConfigurationSnapshot, stagingID: string = "", previousVal: IClusterConfigurationSnapshot = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-snapshot';
    const opts = {
      eventID: 'UpdateConfigurationSnapshot',
      objType: 'ClusterConfigurationSnapshot',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterConfigurationSnapshot(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Perform a Configuation Snapshot */
  public Save(body: IClusterConfigurationSnapshotRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/config-snapshot/save';
    const opts = {
      eventID: 'Save',
      objType: 'ClusterConfigurationSnapshot',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterConfigurationSnapshotRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IClusterConfigurationSnapshot | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List DistributedServiceCard objects */
  public ListDistributedServiceCard(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterDistributedServiceCardList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/distributedservicecards';
    const opts = {
      eventID: 'ListDistributedServiceCard',
      objType: 'ClusterDistributedServiceCardList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterDistributedServiceCardList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get DistributedServiceCard object */
  public GetDistributedServiceCard(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterDistributedServiceCard | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/distributedservicecards/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetDistributedServiceCard',
      objType: 'ClusterDistributedServiceCard',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterDistributedServiceCard | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete DistributedServiceCard object */
  public DeleteDistributedServiceCard(O_Name, stagingID: string = ""):Observable<{body: IClusterDistributedServiceCard | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/distributedservicecards/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteDistributedServiceCard',
      objType: 'ClusterDistributedServiceCard',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IClusterDistributedServiceCard | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update DistributedServiceCard object */
  public UpdateDistributedServiceCard(O_Name, body: IClusterDistributedServiceCard, stagingID: string = "", previousVal: IClusterDistributedServiceCard = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterDistributedServiceCard | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/distributedservicecards/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateDistributedServiceCard',
      objType: 'ClusterDistributedServiceCard',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ClusterDistributedServiceCard(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterDistributedServiceCard | IApiStatus | Error, statusCode: number}>;
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
  public AddHost(body: IClusterHost, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterHost(body), null, trimDefaults)
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
  public UpdateHost(O_Name, body: IClusterHost, stagingID: string = "", previousVal: IClusterHost = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterHost | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterHost(body), previousVal, trimDefaults)
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
  public AddNode(body: IClusterNode, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterNode(body), null, trimDefaults)
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
  public UpdateNode(O_Name, body: IClusterNode, stagingID: string = "", previousVal: IClusterNode = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterNode(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IClusterNode | IApiStatus | Error, statusCode: number}>;
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
  public AddTenant(body: IClusterTenant, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterTenant(body), null, trimDefaults)
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
  public UpdateTenant(O_Name, body: IClusterTenant, stagingID: string = "", previousVal: IClusterTenant = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IClusterTenant | IApiStatus | Error, statusCode: number}> {
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
      body = TrimDefaultsAndEmptyFields(body, new ClusterTenant(body), previousVal, trimDefaults)
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
  
  /** Watch ConfigurationSnapshot objects. Supports WebSockets or HTTP long poll */
  public WatchConfigurationSnapshot(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgConfigurationSnapshotWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/config-snapshot';
    const opts = {
      eventID: 'WatchConfigurationSnapshot',
      objType: 'ClusterAutoMsgConfigurationSnapshotWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgConfigurationSnapshotWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch DistributedServiceCard objects. Supports WebSockets or HTTP long poll */
  public WatchDistributedServiceCard(queryParam: any = null, stagingID: string = ""):Observable<{body: IClusterAutoMsgDistributedServiceCardWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/cluster/v1/watch/distributedservicecards';
    const opts = {
      eventID: 'WatchDistributedServiceCard',
      objType: 'ClusterAutoMsgDistributedServiceCardWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IClusterAutoMsgDistributedServiceCardWatchHelper | IApiStatus | Error, statusCode: number}>;
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