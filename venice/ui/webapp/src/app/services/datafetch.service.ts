import 'rxjs/add/observable/forkJoin';

import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';

import { Utility } from '../common/Utility';
import { environment } from '../../environments/environment';
import { AbstractService } from './abstract.service';

@Injectable()
export class DatafetchService extends AbstractService {
  constructor(private _http: HttpClient) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPostCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'DatafetchService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  public isToMockData(): boolean {
    return  !environment.isRESTAPIReady;
  }

  /**
   * Get url
   * e.g http://192.168.69.189:10001/v1/search/query
   */
  getGlobalSearchURL(): string {
    if (!environment.production) {
      if (!this.isToMockData()) {
        return '/search';
      }
    }
    return Utility.getRESTAPIServerAndPort() + environment.version_api_string +   'search/query';
  }

  /**
   *  Invoke search
   */
  public globalSearch(payload: any): Observable<any> {
    const url = this.getGlobalSearchURL();
    return this.invokeAJAXPostCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'DatafetchService-ajax', 'url': url });
  }

}
