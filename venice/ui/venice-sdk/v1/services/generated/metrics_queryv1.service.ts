import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IMetrics_queryQueryResponse,IMetrics_queryQuerySpec } from '../../models/generated/metrics_query';

@Injectable()
export class Metrics_queryv1Service extends AbstractService {
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

  /** Query is the telemetry metrics query RPC, http://localhost:9000/metrics/v1/query */
  public GetQuery_1(queryParam: any = null):Observable<{body: IMetrics_queryQueryResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/metrics/v1/query';
    return this.invokeAJAXGetCall(url, queryParam, 'GetQuery_1') as Observable<{body: IMetrics_queryQueryResponse | Error, statusCode: number}>;
  }
  
  /** Query is the telemetry metrics query RPC, http://localhost:9000/metrics/v1/query */
  public PostQuery(body: IMetrics_queryQuerySpec):Observable<{body: IMetrics_queryQueryResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/metrics/v1/query';
    return this.invokeAJAXPostCall(url, body, 'PostQuery') as Observable<{body: IMetrics_queryQueryResponse | Error, statusCode: number}>;
  }
  
}