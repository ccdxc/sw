import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { IOrchestrationOrchestratorList,OrchestrationOrchestratorList,IApiStatus,ApiStatus,IOrchestrationOrchestrator,OrchestrationOrchestrator,IOrchestrationAutoMsgOrchestratorWatchHelper,OrchestrationAutoMsgOrchestratorWatchHelper } from '../../models/generated/orchestration';

@Injectable()
export class Orchestrationv1Service extends AbstractService {
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

  /** List Orchestrator objects */
  public ListOrchestrator(queryParam: any = null, stagingID: string = ""):Observable<{body: IOrchestrationOrchestratorList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/orchestration/v1/orchestrator';
    const opts = {
      eventID: 'ListOrchestrator',
      objType: 'OrchestrationOrchestratorList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IOrchestrationOrchestratorList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create Orchestrator object */
  public AddOrchestrator(body: IOrchestrationOrchestrator, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/orchestration/v1/orchestrator';
    const opts = {
      eventID: 'AddOrchestrator',
      objType: 'OrchestrationOrchestrator',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new OrchestrationOrchestrator(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Orchestrator object */
  public GetOrchestrator(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/orchestration/v1/orchestrator/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetOrchestrator',
      objType: 'OrchestrationOrchestrator',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Orchestrator object */
  public DeleteOrchestrator(O_Name, stagingID: string = ""):Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/orchestration/v1/orchestrator/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'DeleteOrchestrator',
      objType: 'OrchestrationOrchestrator',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXDeleteCall(url, opts) as Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Orchestrator object */
  public UpdateOrchestrator(O_Name, body: IOrchestrationOrchestrator, stagingID: string = "", previousVal: IOrchestrationOrchestrator = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/orchestration/v1/orchestrator/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateOrchestrator',
      objType: 'OrchestrationOrchestrator',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new OrchestrationOrchestrator(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IOrchestrationOrchestrator | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Orchestrator objects. Supports WebSockets or HTTP long poll */
  public WatchOrchestrator(queryParam: any = null, stagingID: string = ""):Observable<{body: IOrchestrationAutoMsgOrchestratorWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/orchestration/v1/watch/orchestrator';
    const opts = {
      eventID: 'WatchOrchestrator',
      objType: 'OrchestrationAutoMsgOrchestratorWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IOrchestrationAutoMsgOrchestratorWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}