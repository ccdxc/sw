import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IWorkloadEndpointList,IApiStatus,IWorkloadEndpoint,WorkloadEndpoint,IWorkloadWorkloadList,IWorkloadWorkload,WorkloadWorkload,IWorkloadAutoMsgEndpointWatchHelper,IWorkloadAutoMsgWorkloadWatchHelper } from '../../models/generated/workload';

@Injectable()
export class Workloadv1Service extends AbstractService {
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

  /** List Endpoint objects */
  public ListEndpoint():Observable<{body: IWorkloadEndpointList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListEndpoint') as Observable<{body: IWorkloadEndpointList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Endpoint object */
  public AddEndpoint(body: WorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Endpoint object */
  public GetEndpoint(O_Name):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Endpoint object */
  public DeleteEndpoint(O_Name):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Endpoint object */
  public UpdateEndpoint(O_Name, body: WorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Workload objects */
  public ListWorkload():Observable<{body: IWorkloadWorkloadList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListWorkload') as Observable<{body: IWorkloadWorkloadList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Workload object */
  public AddWorkload(body: WorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Workload object */
  public GetWorkload(O_Name):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Workload object */
  public DeleteWorkload(O_Name):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Workload object */
  public UpdateWorkload(O_Name, body: WorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Endpoint objects */
  public WatchEndpoint():Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchEndpoint') as Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Workload objects */
  public WatchWorkload():Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchWorkload') as Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}