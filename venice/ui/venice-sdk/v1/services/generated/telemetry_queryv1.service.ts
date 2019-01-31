import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ITelemetry_queryMetricsQueryResponse,ITelemetry_queryMetricsQueryList } from '../../models/generated/telemetry_query';

@Injectable()
export class Telemetry_queryv1Service extends AbstractService {
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

  /** Metrics is the telemetry metrics query RPC, http://localhost:9000/telemetry/v1/metrics */
  public GetMetrics_1(queryParam: any = null):Observable<{body: ITelemetry_queryMetricsQueryResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/telemetry/v1/metrics';
    return this.invokeAJAXGetCall(url, queryParam, 'GetMetrics_1') as Observable<{body: ITelemetry_queryMetricsQueryResponse | Error, statusCode: number}>;
  }
  
  /** Metrics is the telemetry metrics query RPC, http://localhost:9000/telemetry/v1/metrics */
  public PostMetrics(body: ITelemetry_queryMetricsQueryList):Observable<{body: ITelemetry_queryMetricsQueryResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/telemetry/v1/metrics';
    return this.invokeAJAXPostCall(url, body, 'PostMetrics') as Observable<{body: ITelemetry_queryMetricsQueryResponse | Error, statusCode: number}>;
  }
  
}