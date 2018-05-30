import 'rxjs/add/observable/forkJoin';

import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';

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

  /**
   * Get url
   * e.g 'http://192.168.30.11:9000/v1/cmd/nodes';
   */
  getGlobalSearchURL(): string {
    return environment.server_url + ':' + environment.server_port + environment.version_api_string + environment.venice_nodes;
  }

  /**
   *  Invoke search
   */
  public globalSearch(payload: any): Observable<any> {

    const url = this.getGlobalSearchURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'DatafetchService-ajax', 'url': url });
  }

}
