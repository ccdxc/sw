import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IWorkloadEndpointList,IApiStatus,IWorkloadEndpoint,WorkloadEndpoint,IWorkloadWorkloadList,IWorkloadWorkload,WorkloadWorkload,IWorkloadAutoMsgEndpointWatchHelper,IWorkloadAutoMsgWorkloadWatchHelper } from '../../models/generated/workload';

@Injectable()
export class WorkloadV1Service extends AbstractService {
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

  /** Retreives a list of Endpoint objects */
  public ListEndpoint():Observable<{body: IWorkloadEndpointList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListEndpoint') as Observable<{body: IWorkloadEndpointList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Endpoint object */
  public AddEndpoint(body: WorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Endpoint object */
  public GetEndpoint(O_Name):Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Endpoint object */
  public DeleteEndpoint(O_Name):Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}>;
  }
  
  public UpdateEndpoint(O_Name, body: WorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Workload objects */
  public ListWorkload():Observable<{body: IWorkloadWorkloadList | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListWorkload') as Observable<{body: IWorkloadWorkloadList | IApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Workload object */
  public AddWorkload(body: WorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Workload object */
  public GetWorkload(O_Name):Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Workload object */
  public DeleteWorkload(O_Name):Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}>;
  }
  
  public UpdateWorkload(O_Name, body: WorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to Endpoint objects */
  public WatchEndpoint():Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchEndpoint') as Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus, statusCode: number}>;
  }
  
  /** Watch for changes to Workload objects */
  public WatchWorkload():Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchWorkload') as Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus, statusCode: number}>;
  }
  
}