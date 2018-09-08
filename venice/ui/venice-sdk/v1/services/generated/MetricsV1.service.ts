import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IMetrics_queryQueryResponse } from '../../models/generated/metrics_query';

@Injectable()
export class MetricsV1Service extends AbstractService {
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

  /** In the example below a query like
   http://<...>/venice/v1/metrics/query
 generates a RPC call Query */
  public Query-1():Observable<{body: IMetrics_queryQueryResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/metrics/v1/metrics';
    return this.invokeAJAXGetCall(url, 'Query-1') as Observable<{body: IMetrics_queryQueryResponse | Error, statusCode: number}>;
  }
  
}