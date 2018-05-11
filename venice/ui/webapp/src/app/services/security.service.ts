import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';
import 'rxjs/add/observable/forkJoin';
import { environment } from '../../environments/environment';
import { Utility } from '../common/Utility';
import { AbstractService } from './abstract.service';

@Injectable()
export class SecurityService  extends AbstractService {
  constructor(private _http: HttpClient) {
    super();
  }

  /**
   * Override super
  */
  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPutCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'SecurityService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  getSecuritygroupsURL(): string {
    return environment.server_url + ':' + environment.server_port + environment.version_api_string + this.buildURLHelper(environment.venice_securitygroups); // 'security-groups';
  }

  getSGPoliciesURL(): string {
    return environment.server_url + ':' + environment.server_port + environment.version_api_string + this.buildURLHelper(environment.venice_sgpolicy ); // 'sgpolicy';
  }

  isToUserRealData(): boolean {
    const isUseRealData = Utility.getInstance().getControllerService().useRealData;
    return (isUseRealData) ? isUseRealData : environment.isRESTAPIReady;
  }

  public getSecuritygroups(payload: any): Observable<any> {
    const url = this.getSecuritygroupsURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'SecurityService-ajax', 'url': url }, this.isToUserRealData());
  }

  public getSGPolicies(payload: any): Observable<any> {
    const url = this.getSGPoliciesURL();
    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'SecurityService-ajax', 'url': url }, this.isToUserRealData());
  }

}
