import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISearchPolicySearchResponse,ISearchPolicySearchRequest,ISearchSearchResponse,ISearchSearchRequest } from '../../models/generated/search';

@Injectable()
export class Searchv1Service extends AbstractService {
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
  public GetPolicyQuery_1(queryParam: any = null):Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/policy-query';
    return this.invokeAJAXGetCall(url, queryParam, 'GetPolicyQuery_1') as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** Security Policy Query */
  public PostPolicyQuery(body: ISearchPolicySearchRequest):Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/policy-query';
    return this.invokeAJAXPostCall(url, body, 'PostPolicyQuery') as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** In the example below a query like
   http://<...>/venice/v1/search/query?QueryString=XXXXX&MaxResults=100
 generates a RPC call Query with the parameter as
 SearchRequest{ QueryString: "XXXXX", MaxResults:100} */
  public GetQuery_1(queryParam: any = null):Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    return this.invokeAJAXGetCall(url, queryParam, 'GetQuery_1') as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
  /** In the example below a query like
   http://<...>/venice/v1/search/query?QueryString=XXXXX&MaxResults=100
 generates a RPC call Query with the parameter as
 SearchRequest{ QueryString: "XXXXX", MaxResults:100} */
  public PostQuery(body: ISearchSearchRequest):Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    return this.invokeAJAXPostCall(url, body, 'PostQuery') as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
}