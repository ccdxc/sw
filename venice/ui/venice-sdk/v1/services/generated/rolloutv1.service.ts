import { AbstractService, ServerEvent } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { IRolloutRolloutList,RolloutRolloutList,IApiStatus,ApiStatus,IRolloutRollout,RolloutRollout,IRolloutAutoMsgRolloutWatchHelper,RolloutAutoMsgRolloutWatchHelper } from '../../models/generated/rollout';

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
  public ListRollout(queryParam: any = null, stagingID: string = ""):Observable<{body: IRolloutRolloutList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout';
    const opts = {
      eventID: 'ListRollout',
      objType: 'RolloutRolloutList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IRolloutRolloutList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Start Rollout operation */
  public CreateRollout(body: IRolloutRollout, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/CreateRollout';
    const opts = {
      eventID: 'CreateRollout',
      objType: 'RolloutRollout',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new RolloutRollout(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Remove a Rollout */
  public RemoveRollout(body: IRolloutRollout, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/RemoveRollout';
    const opts = {
      eventID: 'RemoveRollout',
      objType: 'RolloutRollout',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new RolloutRollout(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Stop a Rollout operation */
  public StopRollout(body: IRolloutRollout, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/StopRollout';
    const opts = {
      eventID: 'StopRollout',
      objType: 'RolloutRollout',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new RolloutRollout(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Rollout configuration */
  public UpdateRollout(body: IRolloutRollout, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/UpdateRollout';
    const opts = {
      eventID: 'UpdateRollout',
      objType: 'RolloutRollout',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new RolloutRollout(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Rollout object */
  public GetRollout(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/rollout/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetRollout',
      objType: 'RolloutRollout',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IRolloutRollout | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Rollout objects. Supports WebSockets or HTTP long poll */
  public WatchRollout(queryParam: any = null, stagingID: string = ""):Observable<{body: IRolloutAutoMsgRolloutWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/rollout/v1/watch/rollout';
    const opts = {
      eventID: 'WatchRollout',
      objType: 'RolloutAutoMsgRolloutWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IRolloutAutoMsgRolloutWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  protected createListRolloutCache(): Observable<ServerEvent<RolloutRollout>> {
    return this.createDataCache<RolloutRollout>(RolloutRollout, `RolloutRollout`, () => this.ListRollout(), (body: any) => this.WatchRollout(body));
  }

  public ListRolloutCache(): Observable<ServerEvent<RolloutRollout>> {
    return this.getFromDataCache(`RolloutRollout`, () => { return this.createListRolloutCache() });
  }
  
}