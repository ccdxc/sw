import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { IDiagnosticsModuleList,DiagnosticsModuleList,IApiStatus,ApiStatus,IDiagnosticsModule,DiagnosticsModule,IDiagnosticsDiagnosticsResponse,DiagnosticsDiagnosticsResponse,DiagnosticsDiagnosticsRequest,IDiagnosticsDiagnosticsRequest,ApiLabel,IApiLabel,IDiagnosticsAutoMsgModuleWatchHelper,DiagnosticsAutoMsgModuleWatchHelper } from '../../models/generated/diagnostics';

@Injectable()
export class Diagnosticsv1Service extends AbstractService {
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

  /** List Module objects */
  public ListModule(queryParam: any = null, stagingID: string = ""):Observable<{body: IDiagnosticsModuleList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/diagnostics/v1/modules';
    const opts = {
      eventID: 'ListModule',
      objType: 'DiagnosticsModuleList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IDiagnosticsModuleList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Module object */
  public GetModule(O_Name, queryParam: any = null, stagingID: string = ""):Observable<{body: IDiagnosticsModule | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/diagnostics/v1/modules/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetModule',
      objType: 'DiagnosticsModule',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IDiagnosticsModule | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Module object */
  public UpdateModule(O_Name, body: IDiagnosticsModule, stagingID: string = "", previousVal: IDiagnosticsModule = null, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IDiagnosticsModule | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/diagnostics/v1/modules/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'UpdateModule',
      objType: 'DiagnosticsModule',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new DiagnosticsModule(body), previousVal, trimDefaults)
    }
    return this.invokeAJAXPutCall(url, body, opts) as Observable<{body: IDiagnosticsModule | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Request Diagnostics information for a module */
  public Debug(O_Name, body: IDiagnosticsDiagnosticsRequest, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IDiagnosticsDiagnosticsResponse | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/diagnostics/v1/modules/{O.Name}/Debug';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'Debug',
      objType: 'DiagnosticsDiagnosticsResponse',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new DiagnosticsDiagnosticsRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IDiagnosticsDiagnosticsResponse | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Label Module object */
  public LabelModule(O_Name, body: IApiLabel, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IDiagnosticsModule | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/diagnostics/v1/modules/{O.Name}/label';
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'LabelModule',
      objType: 'DiagnosticsModule',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new ApiLabel(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IDiagnosticsModule | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Module objects. Supports WebSockets or HTTP long poll */
  public WatchModule(queryParam: any = null, stagingID: string = ""):Observable<{body: IDiagnosticsAutoMsgModuleWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/diagnostics/v1/watch/modules';
    const opts = {
      eventID: 'WatchModule',
      objType: 'DiagnosticsAutoMsgModuleWatchHelper',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IDiagnosticsAutoMsgModuleWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}