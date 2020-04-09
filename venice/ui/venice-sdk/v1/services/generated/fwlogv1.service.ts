import { AbstractService, ServerEvent } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { IFwlogFwLogList,FwlogFwLogList,FwlogFwLogQuery,IFwlogFwLogQuery } from '../../models/generated/fwlog';

@Injectable()
export class Fwlogv1Service extends AbstractService {
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

  /** Queries firewall logs */
  public GetGetLogs_1(queryParam: any = null):Observable<{body: IFwlogFwLogList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/fwlog/v1/query';
    const opts = {
      eventID: 'GetGetLogs_1',
      objType: 'FwlogFwLogList',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IFwlogFwLogList | Error, statusCode: number}>;
  }
  
  /** Queries firewall logs */
  public PostGetLogs(body: IFwlogFwLogQuery, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IFwlogFwLogList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/fwlog/v1/query';
    const opts = {
      eventID: 'PostGetLogs',
      objType: 'FwlogFwLogList',
      isStaging: false,
    }
    body = TrimUIFields(body)
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new FwlogFwLogQuery(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IFwlogFwLogList | Error, statusCode: number}>;
  }
  
}