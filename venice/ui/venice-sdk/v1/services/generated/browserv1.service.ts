import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { IBrowserBrowseResponse,BrowserBrowseResponse,IBrowserBrowseResponseList,BrowserBrowseResponseList,BrowserBrowseRequestList,IBrowserBrowseRequestList } from '../../models/generated/browser';

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
    const opts = {
      eventID: 'GetReferrers',
      objType: 'BrowserBrowseResponse',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}>;
  }
  
  public GetReferences(queryParam: any = null, stagingID: string = ""):Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/dependencies/**';
    const opts = {
      eventID: 'GetReferences',
      objType: 'BrowserBrowseResponse',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IBrowserBrowseResponse | Error, statusCode: number}>;
  }
  
  public GetQuery_1(queryParam: any = null, stagingID: string = ""):Observable<{body: IBrowserBrowseResponseList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/query';
    const opts = {
      eventID: 'GetQuery_1',
      objType: 'BrowserBrowseResponseList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IBrowserBrowseResponseList | Error, statusCode: number}>;
  }
  
  public PostQuery(body: IBrowserBrowseRequestList, stagingID: string = "", trimObject: boolean = true, trimDefaults: boolean = true):Observable<{body: IBrowserBrowseResponseList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/browser/v1/query';
    const opts = {
      eventID: 'PostQuery',
      objType: 'BrowserBrowseResponseList',
      isStaging: false,
    }
    if (stagingID != null && stagingID.length != 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    if (trimObject) {
      body = TrimDefaultsAndEmptyFields(body, new BrowserBrowseRequestList(body), null, trimDefaults)
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IBrowserBrowseResponseList | Error, statusCode: number}>;
  }
  
}