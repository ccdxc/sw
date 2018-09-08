import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISearchPolicySearchResponse,ISearchSearchResponse } from '../../models/generated/search';

@Injectable()
export class SearchV1Service extends AbstractService {
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

  /** Security Policy Query */
  public PolicyQuery-1():Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/policy-query';
    return this.invokeAJAXGetCall(url, 'PolicyQuery-1') as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** In the example below a query like
   http://<...>/venice/v1/search/query?QueryString=XXXXX&MaxResults=100
 generates a RPC call Query with the parameter as
 SearchRequest{ QueryString: "XXXXX", MaxResults:100} */
  public Query-1():Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    return this.invokeAJAXGetCall(url, 'Query-1') as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
}