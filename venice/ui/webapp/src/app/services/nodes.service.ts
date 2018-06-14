import 'rxjs/add/observable/forkJoin';

import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';

import { environment } from '../../environments/environment';
import { AbstractService } from './abstract.service';

@Injectable()
export class NodesService extends AbstractService {
  constructor(private _http: HttpClient) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPostCall(url, payload, this._http,
      { 'ajax': 'start', 'name': 'NodesService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  getNodesURL(): string {
    if (!environment.production) {
      return '/nodes';
    }
    return environment.server_url + ':' + environment.server_port + '/cluster/nodes';
  }

  public getNodes(): Observable<any> {
    const url = this.getNodesURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'NodesService-ajax', 'url': url }, true);
  }
}
