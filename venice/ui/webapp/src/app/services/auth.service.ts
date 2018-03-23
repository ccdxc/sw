import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';

import { AbstractService } from './abstract.service';

@Injectable()
export class AuthService extends AbstractService {
  // private authUrl: string;

  constructor(private _http: HttpClient) {
    super();
  }

  protected callServer(url: string, payload: any) {
    return this.invokeAJAXPutCall(url, payload,
      this._http, { 'ajax': 'start', 'name': 'AuthService-ajax', 'url': url });
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
      return this.constructor.name;
  }

  private _generateLoginURL(): string {

    return '../data/venice-signin-response.json';

  }

  /**
   * Log the user in
   */
  login(payload: string): Observable<any> {
    // this.authUrl = this._generateLoginURL();
    const url = this._generateLoginURL();

    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'AuthService-ajax', 'url': url });

  }

}
