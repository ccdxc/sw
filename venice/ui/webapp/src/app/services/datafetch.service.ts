import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs/Observable';
import 'rxjs/add/observable/forkJoin';
import { AbstractService } from './abstract.service';
import { environment } from '../../environments/environment';
import { Utility } from '../common/Utility';
import { Eventtypes } from '../enum/eventtypes.enum';

@Injectable()
export class DatafetchService extends AbstractService {
  constructor(private _http: HttpClient) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPutCall(url, payload,
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
