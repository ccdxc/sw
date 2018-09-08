import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IWorkloadEndpointList,IWorkloadEndpoint,WorkloadEndpoint,IWorkloadAutoMsgEndpointWatchHelper,IWorkloadAutoMsgWorkloadWatchHelper,IWorkloadWorkloadList,IWorkloadWorkload,WorkloadWorkload } from '../../models/generated/workload';

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

  /** List Endpoint objects */
  public AutoListEndpoint-1():Observable<{body: IWorkloadEndpointList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints';
    return this.invokeAJAXGetCall(url, 'AutoListEndpoint-1') as Observable<{body: IWorkloadEndpointList | Error, statusCode: number}>;
  }
  
  /** Create Endpoint object */
  public AutoAddEndpoint-1(body: WorkloadEndpoint):Observable<{body: IWorkloadEndpoint | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddEndpoint-1') as Observable<{body: IWorkloadEndpoint | Error, statusCode: number}>;
  }
  
  /** Get Endpoint object */
  public AutoGetEndpoint-1(O_Name):Observable<{body: IWorkloadEndpoint | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetEndpoint-1') as Observable<{body: IWorkloadEndpoint | Error, statusCode: number}>;
  }
  
  /** Delete Endpoint object */
  public AutoDeleteEndpoint-1(O_Name):Observable<{body: IWorkloadEndpoint | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteEndpoint-1') as Observable<{body: IWorkloadEndpoint | Error, statusCode: number}>;
  }
  
  /** Update Endpoint object */
  public AutoUpdateEndpoint-1(O_Name, body: WorkloadEndpoint):Observable<{body: IWorkloadEndpoint | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/endpoints/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateEndpoint-1') as Observable<{body: IWorkloadEndpoint | Error, statusCode: number}>;
  }
  
  /** Watch Endpoint objects */
  public AutoWatchEndpoint-1():Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/endpoints';
    return this.invokeAJAXGetCall(url, 'AutoWatchEndpoint-1') as Observable<{body: IWorkloadAutoMsgEndpointWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch Workload objects */
  public AutoWatchWorkload-1():Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/watch/workloads';
    return this.invokeAJAXGetCall(url, 'AutoWatchWorkload-1') as Observable<{body: IWorkloadAutoMsgWorkloadWatchHelper | Error, statusCode: number}>;
  }
  
  /** List Workload objects */
  public AutoListWorkload-1():Observable<{body: IWorkloadWorkloadList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads';
    return this.invokeAJAXGetCall(url, 'AutoListWorkload-1') as Observable<{body: IWorkloadWorkloadList | Error, statusCode: number}>;
  }
  
  /** Create Workload object */
  public AutoAddWorkload-1(body: WorkloadWorkload):Observable<{body: IWorkloadWorkload | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddWorkload-1') as Observable<{body: IWorkloadWorkload | Error, statusCode: number}>;
  }
  
  /** Get Workload object */
  public AutoGetWorkload-1(O_Name):Observable<{body: IWorkloadWorkload | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetWorkload-1') as Observable<{body: IWorkloadWorkload | Error, statusCode: number}>;
  }
  
  /** Delete Workload object */
  public AutoDeleteWorkload-1(O_Name):Observable<{body: IWorkloadWorkload | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteWorkload-1') as Observable<{body: IWorkloadWorkload | Error, statusCode: number}>;
  }
  
  /** Update Workload object */
  public AutoUpdateWorkload-1(O_Name, body: WorkloadWorkload):Observable<{body: IWorkloadWorkload | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/workload/v1/workloads/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateWorkload-1') as Observable<{body: IWorkloadWorkload | Error, statusCode: number}>;
  }
  
}