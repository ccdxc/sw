import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { WorkloadEndpointList,ApiStatus,WorkloadEndpoint,WorkloadWorkloadList,WorkloadWorkload } from '../../models/generated/workload';

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
  public ListEndpoint():Observable<{body: WorkloadEndpointList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListEndpoint') as Observable<{body: WorkloadEndpointList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Endpoint object */
  public AddEndpoint(body: WorkloadEndpoint):Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/endpoints';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddEndpoint') as Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Endpoint object */
  public GetEndpoint(O_Name):Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetEndpoint') as Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Endpoint object */
  public DeleteEndpoint(O_Name):Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEndpoint') as Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}>;
  }
  
  public UpdateEndpoint(O_Name, body: WorkloadEndpoint):Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/endpoints/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateEndpoint') as Observable<{body: WorkloadEndpoint | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Workload objects */
  public ListWorkload():Observable<{body: WorkloadWorkloadList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListWorkload') as Observable<{body: WorkloadWorkloadList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Workload object */
  public AddWorkload(body: WorkloadWorkload):Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/workloads';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddWorkload') as Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Workload object */
  public GetWorkload(O_Name):Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetWorkload') as Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Workload object */
  public DeleteWorkload(O_Name):Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteWorkload') as Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}>;
  }
  
  public UpdateWorkload(O_Name, body: WorkloadWorkload):Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/workload/{O.Tenant}/workloads/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateWorkload') as Observable<{body: WorkloadWorkload | ApiStatus, statusCode: number}>;
  }
  
}