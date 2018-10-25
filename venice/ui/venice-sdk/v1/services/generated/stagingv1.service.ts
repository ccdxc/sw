import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IStagingBufferList,IApiStatus,IStagingBuffer,IStagingClearAction,IStagingCommitAction } from '../../models/generated/staging';

@Injectable()
export class Stagingv1Service extends AbstractService {
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

  /** List Buffer objects */
  public ListBuffer_1(queryParam: any = null):Observable<{body: IStagingBufferList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers';
    return this.invokeAJAXGetCall(url, queryParam, 'ListBuffer_1') as Observable<{body: IStagingBufferList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Buffer object */
  public AddBuffer_1(body: IStagingBuffer):Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers';
    return this.invokeAJAXPostCall(url, body, 'AddBuffer_1') as Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Buffer object */
  public GetBuffer_1(O_Name, queryParam: any = null):Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetBuffer_1') as Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Buffer object */
  public DeleteBuffer_1(O_Name):Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteBuffer_1') as Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}>;
  }
  
  public Clear_1(O_Name, body: IStagingClearAction):Observable<{body: IStagingClearAction | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}/clear';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPostCall(url, body, 'Clear_1') as Observable<{body: IStagingClearAction | IApiStatus | Error, statusCode: number}>;
  }
  
  public Commit_1(O_Name, body: IStagingCommitAction):Observable<{body: IStagingCommitAction | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}/commit';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPostCall(url, body, 'Commit_1') as Observable<{body: IStagingCommitAction | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Buffer objects */
  public ListBuffer(queryParam: any = null):Observable<{body: IStagingBufferList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/tenant/{O.Tenant}/buffers';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListBuffer') as Observable<{body: IStagingBufferList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Buffer object */
  public AddBuffer(body: IStagingBuffer):Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/tenant/{O.Tenant}/buffers';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddBuffer') as Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Buffer object */
  public GetBuffer(O_Name, queryParam: any = null):Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/tenant/{O.Tenant}/buffers/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetBuffer') as Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Buffer object */
  public DeleteBuffer(O_Name):Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/tenant/{O.Tenant}/buffers/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteBuffer') as Observable<{body: IStagingBuffer | IApiStatus | Error, statusCode: number}>;
  }
  
  public Clear(O_Name, body: IStagingClearAction):Observable<{body: IStagingClearAction | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/tenant/{O.Tenant}/buffers/{O.Name}/clear';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPostCall(url, body, 'Clear') as Observable<{body: IStagingClearAction | IApiStatus | Error, statusCode: number}>;
  }
  
  public Commit(O_Name, body: IStagingCommitAction):Observable<{body: IStagingCommitAction | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/tenant/{O.Tenant}/buffers/{O.Name}/commit';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPostCall(url, body, 'Commit') as Observable<{body: IStagingCommitAction | IApiStatus | Error, statusCode: number}>;
  }
  
}