import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IStagingBufferList,IStagingBuffer,StagingBuffer,IStagingClearAction,IStagingCommitAction } from '../../models/generated/staging';

@Injectable()
export class StagingV1Service extends AbstractService {
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
  public AutoListBuffer-1():Observable<{body: IStagingBufferList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers';
    return this.invokeAJAXGetCall(url, 'AutoListBuffer-1') as Observable<{body: IStagingBufferList | Error, statusCode: number}>;
  }
  
  /** Create Buffer object */
  public AutoAddBuffer-1(body: StagingBuffer):Observable<{body: IStagingBuffer | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddBuffer-1') as Observable<{body: IStagingBuffer | Error, statusCode: number}>;
  }
  
  /** Get Buffer object */
  public AutoGetBuffer-1(O_Name):Observable<{body: IStagingBuffer | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetBuffer-1') as Observable<{body: IStagingBuffer | Error, statusCode: number}>;
  }
  
  /** Delete Buffer object */
  public AutoDeleteBuffer-1(O_Name):Observable<{body: IStagingBuffer | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteBuffer-1') as Observable<{body: IStagingBuffer | Error, statusCode: number}>;
  }
  
  public Clear-1(O_Name, body: ):Observable<{body: IStagingClearAction | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}/clear';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPostCall(url, body.getValues(), 'Clear-1') as Observable<{body: IStagingClearAction | Error, statusCode: number}>;
  }
  
  public Commit-1(O_Name, body: ):Observable<{body: IStagingCommitAction | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/staging/v1/buffers/{O.Name}/commit';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPostCall(url, body.getValues(), 'Commit-1') as Observable<{body: IStagingCommitAction | Error, statusCode: number}>;
  }
  
}