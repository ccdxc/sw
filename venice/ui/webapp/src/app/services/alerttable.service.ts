import 'rxjs/add/observable/forkJoin';

import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';

import { environment } from '../../environments/environment';
import { AbstractService } from './abstract.service';

@Injectable()
export class AlerttableService extends AbstractService {
  constructor(private _http: HttpClient) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPutCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'AlerttableService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  getAlertsURL(): string {
    return environment.server_url + ':' + environment.server_port + environment.version_api_string + 'alerttable';
  }

  getAlertListURL(): string {
    return environment.server_url + ':' + environment.server_port + environment.version_api_string + 'alertlist';
  }

  public getAlerts(payload: any): Observable<any> {
    const url = this.getAlertsURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'AlerttableService-ajax', 'url': url });
  }

  public getAlertList(payload: any): Observable<any> {
    const url = this.getAlertListURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'DatafetchService-ajax', 'url': url });
  }

}
