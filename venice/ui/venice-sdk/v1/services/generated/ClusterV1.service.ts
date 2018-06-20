import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ClusterClusterList,ApiStatus,ClusterCluster,ClusterHostList,ClusterHost,ClusterNodeList,ClusterNode,ClusterSmartNICList,ClusterSmartNIC,ClusterTenantList,ClusterTenant } from '../../models/generated/cluster';

@Injectable()
export class ClusterV1Service extends AbstractService {
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

  /** Retreives a list of Cluster objects */
  public ListCluster():Observable<{body: ClusterClusterList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/cluster';
    return this.invokeAJAXGetCall(url, 'ListCluster') as Observable<{body: ClusterClusterList | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Cluster object */
  public GetCluster(O_Name):Observable<{body: ClusterCluster | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/cluster/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetCluster') as Observable<{body: ClusterCluster | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Cluster object */
  public DeleteCluster(O_Name):Observable<{body: ClusterCluster | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/cluster/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteCluster') as Observable<{body: ClusterCluster | ApiStatus, statusCode: number}>;
  }
  
  public UpdateCluster(O_Name, body: ClusterCluster):Observable<{body: ClusterCluster | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/cluster/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateCluster') as Observable<{body: ClusterCluster | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Host objects */
  public ListHost():Observable<{body: ClusterHostList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/hosts';
    return this.invokeAJAXGetCall(url, 'ListHost') as Observable<{body: ClusterHostList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Host object */
  public AddHost(body: ClusterHost):Observable<{body: ClusterHost | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/hosts';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddHost') as Observable<{body: ClusterHost | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Host object */
  public GetHost(O_Name):Observable<{body: ClusterHost | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetHost') as Observable<{body: ClusterHost | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Host object */
  public DeleteHost(O_Name):Observable<{body: ClusterHost | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteHost') as Observable<{body: ClusterHost | ApiStatus, statusCode: number}>;
  }
  
  public UpdateHost(O_Name, body: ClusterHost):Observable<{body: ClusterHost | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/hosts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateHost') as Observable<{body: ClusterHost | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Node objects */
  public ListNode():Observable<{body: ClusterNodeList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/nodes';
    return this.invokeAJAXGetCall(url, 'ListNode') as Observable<{body: ClusterNodeList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Node object */
  public AddNode(body: ClusterNode):Observable<{body: ClusterNode | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/nodes';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddNode') as Observable<{body: ClusterNode | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Node object */
  public GetNode(O_Name):Observable<{body: ClusterNode | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetNode') as Observable<{body: ClusterNode | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Node object */
  public DeleteNode(O_Name):Observable<{body: ClusterNode | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNode') as Observable<{body: ClusterNode | ApiStatus, statusCode: number}>;
  }
  
  public UpdateNode(O_Name, body: ClusterNode):Observable<{body: ClusterNode | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/nodes/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateNode') as Observable<{body: ClusterNode | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of SmartNIC objects */
  public ListSmartNIC():Observable<{body: ClusterSmartNICList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/smartnics';
    return this.invokeAJAXGetCall(url, 'ListSmartNIC') as Observable<{body: ClusterSmartNICList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new SmartNIC object */
  public AddSmartNIC(body: ClusterSmartNIC):Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/smartnics';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddSmartNIC') as Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the SmartNIC object */
  public GetSmartNIC(O_Name):Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetSmartNIC') as Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the SmartNIC object */
  public DeleteSmartNIC(O_Name):Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteSmartNIC') as Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}>;
  }
  
  public UpdateSmartNIC(O_Name, body: ClusterSmartNIC):Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/smartnics/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateSmartNIC') as Observable<{body: ClusterSmartNIC | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Tenant objects */
  public ListTenant():Observable<{body: ClusterTenantList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/tenants';
    return this.invokeAJAXGetCall(url, 'ListTenant') as Observable<{body: ClusterTenantList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Tenant object */
  public AddTenant(body: ClusterTenant):Observable<{body: ClusterTenant | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/tenants';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTenant') as Observable<{body: ClusterTenant | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Tenant object */
  public GetTenant(O_Name):Observable<{body: ClusterTenant | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetTenant') as Observable<{body: ClusterTenant | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Tenant object */
  public DeleteTenant(O_Name):Observable<{body: ClusterTenant | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTenant') as Observable<{body: ClusterTenant | ApiStatus, statusCode: number}>;
  }
  
  public UpdateTenant(O_Name, body: ClusterTenant):Observable<{body: ClusterTenant | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/cluster/tenants/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTenant') as Observable<{body: ClusterTenant | ApiStatus, statusCode: number}>;
  }
  
}