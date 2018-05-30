import { Injectable } from '@angular/core';
import { HttpEvent, HttpInterceptor, HttpHandler, HttpRequest, HttpErrorResponse, HttpResponse } from '@angular/common/http';
import { Observable } from 'rxjs/Observable';
import { _throw } from 'rxjs/observable/throw';
import 'rxjs/add/operator/catch';
import 'rxjs/add/operator/do';

import { Utility } from '@app/common/Utility';
import { LogService } from '@app/services/logging/log.service';

/**
 * This is an angular http-client inteceptor
 * see https://angular.io/guide/http
 *
 * It logs how long a request take and capture errors
 *
 * app.module.ts registers this inteceptor.
 *
 * note. abstract.service.ts also handle error of AJAX calls
 */
@Injectable()
export class VeniceUIHttpInterceptor implements HttpInterceptor {

  protected logger: LogService;

  constructor() {

  }

  getLogger(): LogService {
    if (!this.logger) {
      this.logger = Utility.getInstance().getLogService();
    }
    return this.logger;
  }

  getClassName(): string {
    return this.constructor.name;
  }

  private isURLlogin(url: string): boolean {
    return (url.indexOf('login') >= 0);
  }

  /**
   * This api intercept all REST APIs.
   * As mentioned in https://angular.io/guide/http#security-xsrf-protection, Angular httpClient does not use XSRF token in GET/HEAD method,
   * it will add XSRFToken to http-request headers
   *
   * @param req
   * @param next
   */
  intercept(req: HttpRequest<any>, next: HttpHandler): Observable<HttpEvent<any>> {
    const started = Date.now();
    this.getLogger();
    const headerName = Utility.XSRF_NAME;
    const token = Utility.getInstance().getXSRFtoken();
    if (!this.isURLlogin(req.url)) {
      if (!req.headers.has(headerName)) {
        req = req.clone({ headers: req.headers.set(headerName, token) });
      }
    }
    return next
      .handle(req)
      .do(event => {
        if (event instanceof HttpResponse) {
          const elapsed = Date.now() - started;
          this.logger.log(`VeniceUIHttpInterceptor log: Request for ${req.urlWithParams} took ${elapsed} ms.`, this.getClassName());
        }
      }).catch(errorReponse => {
        let errMsg: string;
        const elapsed = Date.now() - started;
        if (errorReponse instanceof HttpErrorResponse) {
          const err = errorReponse.message || JSON.stringify(errorReponse.error);
          errMsg = `${errorReponse.status} - ${errorReponse.statusText || ''} Details: ${err}`;
        } else {
          errMsg = errorReponse.message ? errorReponse.message : errorReponse.toString();
        }
        this.logger.error(`VeniceUIHttpInterceptor log: Request for ${req.urlWithParams} took ${elapsed} ms.`, this.getClassName());
        this.logger.error('VeniceUIHttpInterceptor log ' + errMsg, this.getClassName());
        return _throw(errorReponse);
      });
  }
}
