import { HttpClient, HttpHeaders, HttpResponse } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';
import { environment } from '../../environments/environment';
import { AbstractService } from './abstract.service';
import { Utility } from '@app/common/Utility';


@Injectable()
export class AuthService extends AbstractService {
  // private authUrl: string;

  constructor(private _http: HttpClient) {
    super();
  }

  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPostCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'AuthService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * Login URL.
   * It detects if it is in production enviornment and build URL.
   * In Dev, UI localhost:4200 is calling venice server, we use proxy to handle CORS situation.
   */
  private _generateLoginURL(): string {
    if (!environment.production) {
      if (!this.isToMockData()) {
        return '/login';
      }
    }
    // e.g 'http://192.168.30.11:9000/v1/login/';
    return Utility.getRESTAPIServerAndPort() + environment.version_api_string + environment.login + '/';
  }

  /**
   * Log the user in
   */
  login(payload: string): Observable<any> {
    const url = this._generateLoginURL();
    return this.invokeAJAXPostCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'AuthService-ajax', 'url': url });
  }

  public isToMockData(): boolean {
    const boolUseRealData = !environment.isRESTAPIReady;
    return boolUseRealData;
  }

  /**
   *  Override super's API.
   *  As RBAC call returns "x-csrf-token" in response header, we need special handling.
   *  See:
   *   https://angular.io/guide/http#security-xsrf-protection (app.module.ts has HttpClientXsrfModule.withOptions)
   *   https://github.com/angular/angular-cli/blob/master/docs/documentation/stories/proxy.md (proxy.config.json)
   */
  protected invokeAJAXPostCall(url: string, payload: any, http: HttpClient, eventpayload: any): Observable<any> {

    this.publishAJAX(eventpayload);

    if (this.isToMockData()) {
      const method = 'POST';
      return this.handleOfflineAJAX(url, method, eventpayload);
    } else {
      const httpOptions = {
        headers: new HttpHeaders({
          'Content-Type': 'application/json'
        }),
        observe: 'response' as 'body'
      };
      return http.post<HttpResponse<any>>(url, payload, httpOptions);
    }
  }

}
