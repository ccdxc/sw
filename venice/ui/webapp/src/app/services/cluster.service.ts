import 'rxjs/add/observable/forkJoin';

import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';

import { environment } from '../../environments/environment';
import { AbstractService } from './abstract.service';

@Injectable()
export class ClusterService extends AbstractService {
  constructor(private _http: HttpClient) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPostCall(url, payload, this._http,
      { 'ajax': 'start', 'name': 'ClusterService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  getClusterURL(): string {
    return environment.server_url + ':' + environment.server_port + '/cluster/cluster';
  }

  public getCluster(): Observable<any> {
    const url = this.getClusterURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'ClusterService-ajax', 'url': url }, true);
  }
}
