import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IRolloutRolloutList,IApiStatus,IRolloutRollout,IRolloutAutoMsgRolloutWatchHelper } from '../../models/generated/rollout';

@Injectable()
export class Rolloutv1Service extends AbstractService {
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

  /** List Rollout objects */
  public ListRollout(queryParam: any = null):Observable<{body: IRolloutRolloutList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout';
    return this.invokeAJAXGetCall(url, queryParam, 'ListRollout') as Observable<{body: IRolloutRolloutList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Rollout object */
  public AddRollout(body: IRolloutRollout):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout';
    return this.invokeAJAXPostCall(url, body, 'AddRollout') as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Rollout object */
  public GetRollout(O_Name, queryParam: any = null):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetRollout') as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Rollout object */
  public DeleteRollout(O_Name):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteRollout') as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Rollout object */
  public UpdateRollout(O_Name, body: IRolloutRollout):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateRollout') as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Rollout objects */
  public WatchRollout(queryParam: any = null):Observable<{body: IRolloutAutoMsgRolloutWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/watch/rollout';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchRollout') as Observable<{body: IRolloutAutoMsgRolloutWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}