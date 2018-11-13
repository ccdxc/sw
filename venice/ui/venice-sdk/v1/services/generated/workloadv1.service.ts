import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IWorkloadEndpointList,IApiStatus,IWorkloadEndpoint,IWorkloadWorkloadList,IWorkloadWorkload,IWorkloadAutoMsgEndpointWatchHelper,IWorkloadAutoMsgWorkloadWatchHelper } from '../../models/generated/workload';

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
  public ListEndpoint_1(queryParam: any = null):Observable<{body: IWorkloadEndpointList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints';
    return this.invokeAJAXGetCall(url, queryParam, 'ListEndpoint_1') as Observable<{body: IWorkloadEndpointList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Endpoint object */
  public AddEndpoint_1(body: IWorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints';
    return this.invokeAJAXPostCall(url, body, 'AddEndpoint_1') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Endpoint object */
  public GetEndpoint_1(O_Name, queryParam: any = null):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetEndpoint_1') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Endpoint object */
  public DeleteEndpoint_1(O_Name):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEndpoint_1') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Endpoint object */
  public UpdateEndpoint_1(O_Name, body: IWorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateEndpoint_1') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Endpoint objects */
  public ListEndpoint(queryParam: any = null):Observable<{body: IWorkloadEndpointList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListEndpoint') as Observable<{body: IWorkloadEndpointList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Endpoint object */
  public AddEndpoint(body: IWorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Endpoint object */
  public GetEndpoint(O_Name, queryParam: any = null):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Endpoint object */
  public DeleteEndpoint(O_Name):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Endpoint object */
  public UpdateEndpoint(O_Name, body: IWorkloadEndpoint):Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateEndpoint') as Observable<{body: IWorkloadEndpoint | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Workload objects */
  public ListWorkload(queryParam: any = null):Observable<{body: IWorkloadWorkloadList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListWorkload') as Observable<{body: IWorkloadWorkloadList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Workload object */
  public AddWorkload(body: IWorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Workload object */
  public GetWorkload(O_Name, queryParam: any = null):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Workload object */
  public DeleteWorkload(O_Name):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Workload object */
  public UpdateWorkload(O_Name, body: IWorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/tenant/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateWorkload') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Endpoint objects */
  public WatchEndpoint_1(queryParam: any = null):Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/endpoints';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchEndpoint_1') as Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Endpoint objects */
  public WatchEndpoint(queryParam: any = null):Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/tenant/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchEndpoint') as Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Workload objects */
  public WatchWorkload(queryParam: any = null):Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/tenant/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchWorkload') as Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Workload objects */
  public WatchWorkload_1(queryParam: any = null):Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/workloads';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchWorkload_1') as Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Workload objects */
  public ListWorkload_1(queryParam: any = null):Observable<{body: IWorkloadWorkloadList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads';
    return this.invokeAJAXGetCall(url, queryParam, 'ListWorkload_1') as Observable<{body: IWorkloadWorkloadList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Workload object */
  public AddWorkload_1(body: IWorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads';
    return this.invokeAJAXPostCall(url, body, 'AddWorkload_1') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Workload object */
  public GetWorkload_1(O_Name, queryParam: any = null):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetWorkload_1') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Workload object */
  public DeleteWorkload_1(O_Name):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteWorkload_1') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Workload object */
  public UpdateWorkload_1(O_Name, body: IWorkloadWorkload):Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateWorkload_1') as Observable<{body: IWorkloadWorkload | IApiStatus | Error, statusCode: number}>;
  }
  
}