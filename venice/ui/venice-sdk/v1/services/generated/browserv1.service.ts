import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IBrowserBrowseResponse,IBrowserBrowseRequest } from '../../models/generated/browser';

@Injectable()
export class Browserv1Service extends AbstractService {
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

  public GetReferrers(queryParam: any = null, stagingID: string = ""):Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/dependedby/**';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetReferrers') as Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}>;
  }
  
  public GetReferences(queryParam: any = null, stagingID: string = ""):Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/dependencies/**';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetReferences') as Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}>;
  }
  
  public GetQuery_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/query';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXGetCall(url, queryParam, 'GetQuery_1') as Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}>;
  }
  
  public PostQuery(body: IBrowserBrowseRequest, stagingID: string = ""):Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/query';
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
    }
    return this.invokeAJAXPostCall(url, body, 'PostQuery') as Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}>;
  }
  
}