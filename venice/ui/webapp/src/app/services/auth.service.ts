import { HttpClient, HttpHeaders, HttpResponse } from '@angular/common/http';
import { Injectable, OnDestroy } from '@angular/core';
import { Observable, Subscription } from 'rxjs';
import { environment } from '../../environments/environment';
import { AbstractService } from './abstract.service';
import { Utility } from '@app/common/Utility';
import { ControllerService } from './controller.service';
import { IAuthSubjectAccessReviewRequest, IApiStatus, IAuthUser } from '@sdk/v1/models/generated/auth';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { MethodOpts } from '@sdk/v1/services/generated/abstract.service';
import { VeniceResponse } from '@app/models/frontend/shared/veniceresponse.interface';
import { GenServiceUtility } from './generated/GenUtility';


@Injectable()
export class AuthService extends AbstractService implements OnDestroy {
  protected O_Tenant: string = this.getTenant(); protected baseUrlAndPort = window.location.protocol + '//' + window.location.hostname + ':' + window.location.port;
  redirectUrl: string;
  protected serviceUtility: GenServiceUtility;
  protected subscriptions: Subscription[] = [];

  constructor(private _http: HttpClient,
              protected _controllerService: ControllerService) {
    super();
    this.serviceUtility = new GenServiceUtility(_http,
      (payload) => { this.publishAJAXStart(payload); },
      (payload) => { this.publishAJAXEnd(payload); }
    );
    this.serviceUtility.setId(this.getClassName());
    const sub = this._controllerService.subscribe(Eventtypes.LOGOUT, (payload) => {
      // After logout, login should take user to dashboard
      this.redirectUrl = '';
    });
    this.subscriptions.push(sub);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(s => {
      s.unsubscribe();
    });
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
    return Utility.getRESTAPIServerAndPort() + environment.version_api_string + environment.login;
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
   * Methods that are always accessible to a user are copied from authv1.service.ts.
   * This is so UIConfigService can call them without causing a circular dependency.
   */

  /** Get User object */
  public GetUser(O_Name, queryParam: any = null, stagingID: string = ''): Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'GetUser',
      objType: 'AuthUser',
      isStaging: false,
    };
    if (stagingID != null && stagingID.length !== 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAX('GET', url, queryParam, opts) as Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}>;
  }

  public IsAuthorized(O_Name, body: IAuthSubjectAccessReviewRequest, stagingID: string = '', trimObject: boolean = true): Observable<{body: IAuthUser | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/auth/v1/tenant/{O.Tenant}/users/{O.Name}/IsAuthorized';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    const opts = {
      eventID: 'IsAuthorized',
      objType: 'AuthUser',
      isStaging: false,
    };
    if (stagingID != null && stagingID.length !== 0) {
      url = url.replace('configs', 'staging/' + stagingID);
      opts.isStaging = true;
    }
    return this.invokeAJAX('POST', url, body, opts, false);
  }

  protected publishAJAXStart(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_START, eventPayload);
  }

  protected publishAJAXEnd(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_END, eventPayload);
  }

  protected invokeAJAX(method: string, url: string, payload: any, opts: MethodOpts, forceReal: boolean = false): Observable<VeniceResponse> {
    const isOnline = !this.isToMockData() || forceReal;
    return this.serviceUtility.invokeAJAX(method, url, payload, opts.eventID, isOnline);
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
