import { Injectable } from '@angular/core';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { Observable } from 'rxjs/Observable';
import { _throw } from 'rxjs/observable/throw';

import { AbstractService } from './abstract.service';
import { ControllerService } from './controller.service';

import { environment } from '../../environments/environment';
import { Logintypes } from '../enum/logintypes.enum';

@Injectable()
export class AuthService extends AbstractService {
  private authUrl: string;

  constructor(private _http: HttpClient, private _controllerService: ControllerService) {
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
    this.authUrl = this._generateLoginURL();
    const url = this._generateLoginURL();

    return this.invokeAJAXGetCall(url,
      this._http, { 'ajax': 'start', 'name': 'AuthService-ajax', 'url': url });

  }

}
