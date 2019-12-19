import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { ISearchPolicySearchResponse,SearchPolicySearchResponse,SearchPolicySearchRequest,ISearchPolicySearchRequest,ISearchSearchResponse,SearchSearchResponse,SearchSearchRequest,ISearchSearchRequest } from '../../models/generated/search';

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
    const opts = {
      eventID: 'GetPolicyQuery_1',
      objType: 'SearchPolicySearchResponse',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** Security Policy Query */
  public PostPolicyQuery(body: ISearchPolicySearchRequest, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/policy-query';
    const opts = {
      eventID: 'PostPolicyQuery',
      objType: 'SearchPolicySearchResponse',
      isStaging: false,
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SearchPolicySearchRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISearchPolicySearchResponse | Error, statusCode: number}>;
  }
  
  /** Structured or free-form search */
  public GetQuery_1(queryParam: any = null):Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    const opts = {
      eventID: 'GetQuery_1',
      objType: 'SearchSearchResponse',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
  /** Structured or free-form search */
  public PostQuery(body: ISearchSearchRequest, trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: ISearchSearchResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/search/v1/query';
    const opts = {
      eventID: 'PostQuery',
      objType: 'SearchSearchResponse',
      isStaging: false,
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new SearchSearchRequest(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: ISearchSearchResponse | Error, statusCode: number}>;
  }
  
}