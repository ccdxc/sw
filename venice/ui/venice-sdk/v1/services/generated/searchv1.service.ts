import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { ISearchPolicySearchResponse,SearchPolicySearchRequest,ISearchSearchResponse,SearchSearchRequest } from '../../models/generated/search';

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
  public GetPolicyQuery_1():Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/policy-query';
    return this.invokeAJAXGetCall(url, 'GetPolicyQuery_1') as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** Security Policy Query */
  public PostPolicyQuery(body: SearchPolicySearchRequest):Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/policy-query';
    return this.invokeAJAXPostCall(url, body.getValues(), 'PostPolicyQuery') as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** In the example below a query like
   http://<...>/venice/v1/search/query?QueryString=XXXXX&MaxResults=100
 generates a RPC call Query with the parameter as
 SearchRequest{ QueryString: "XXXXX", MaxResults:100} */
  public GetQuery_1():Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    return this.invokeAJAXGetCall(url, 'GetQuery_1') as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
  /** In the example below a query like
   http://<...>/venice/v1/search/query?QueryString=XXXXX&MaxResults=100
 generates a RPC call Query with the parameter as
 SearchRequest{ QueryString: "XXXXX", MaxResults:100} */
  public PostQuery(body: SearchSearchRequest):Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    return this.invokeAJAXPostCall(url, body.getValues(), 'PostQuery') as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
}