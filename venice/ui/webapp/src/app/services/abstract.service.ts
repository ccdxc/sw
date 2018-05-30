import 'rxjs/add/observable/of';
import 'rxjs/add/operator/catch';
import 'rxjs/add/operator/delay';

import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs/Observable';
import { _throw } from 'rxjs/observable/throw';

import { MockDataUtil } from '../common/MockDataUtil';
import { Utility } from '../common/Utility';
import { LogService } from './logging/log.service';
import { environment } from '../../environments/environment';
import { Subscription } from 'rxjs/Subscription';
import { timer } from 'rxjs/observable/timer';

@Injectable()
export class AbstractService {

  protected pollingTimerSource: Observable<number>;
  protected pollingTimerSubscription: Subscription;

  protected logger: LogService;
  constructor() {
  }

  getLogger(): LogService {
    if (!this.logger) {
      this.logger = Utility.getInstance().getLogService();
    }
    return this.logger;
  }

  /**
   * Get the service class-name
  */
  getClassName(): string {
    return 'service';
  }

  // *************************
  // Public methods
  // *************************
  debug(msg: string, ...optionalParams: any[]) {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.debug(msg, caller, optionalParams);
  }

  info(msg: string, ...optionalParams: any[]) {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.info(msg, caller, optionalParams);
  }

  warn(msg: string, ...optionalParams: any[]) {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.warn(msg, caller, optionalParams);
  }

  error(msg: string, ...optionalParams: any[]) {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.error(msg, caller, optionalParams);
  }

  fatal(msg: string, ...optionalParams: any[]) {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.fatal(msg, caller, optionalParams);
  }

  log(msg: string, ...optionalParams: any[]) {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    const caller = this.getClassName();
    this.logger.log(msg, caller, optionalParams);
  }

  clear(): void {
    this.getLogger();
    if (!this.logger) {
      console.error('abstract.service.ts logger is null');
      return;
    }
    this.logger.clear();
  }

  /**
   * Handle any errors from the API
   */
  protected handleError(err: HttpErrorResponse): void {
    let errMsg: string;
    if (err.error instanceof Error) {
      // A client-side or network error occurred. Handle it accordingly.
      console.error('An error occurred:', err.error.message);
      errMsg = err.error.message;
    } else {
      // The backend returned an unsuccessful response code.
      // The response body may contain clues as to what went wrong,
      console.error(`Backend returned code ${err.status}, body was: ${err.error}`);
      errMsg = `Backend returned code ${err.status}, message was: ${err.message}`;
    }
    _throw(errMsg);
  }

  isOffLine(): boolean {
    return Utility.isOffLine;
  }

  /**
   * Get Headers
   */
  protected getHeaders(): Headers {
    const headers = new Headers();
    headers.append('Content-Type', 'application/json');
    headers.append('Accept', 'application/json; charset=utf-8');
    return headers;
  }

  protected publishAJAX(eventpayload: any) {
    const utility = Utility.getInstance();
    utility.publishAJAXStart(eventpayload);
  }

  /**
   * This is a fundational API of Pensando-UI-application to invoke server side REST API
   */
  protected invokeAJAXPostCall(url: string, payload: any, http: HttpClient, eventpayload: any): Observable<any> {

    this.publishAJAX(eventpayload);

    if (this.isToMockData()) {
      const method = 'POST';
      return this.handleOfflineAJAX(url, method, eventpayload);
    } else {
      return http.post(url, payload).catch((err: HttpErrorResponse) => {
        return _throw(this.handleError(err));
      });
    }
  }

  /**
   * Override-able api
   */
  public isToMockData(): boolean {
    return this.isOffLine();
  }

  /**
   *
   * @param url
   * @param http
   * @param eventpayload
   */
  protected invokeAJAXGetCall(url: string, http: HttpClient, eventpayload: any, forceReal: boolean = false): Observable<any> {
    this.publishAJAX(eventpayload);
    if (this.isToMockData() && !forceReal) {
      const method = 'GET';
      return this.handleOfflineAJAX(url, method, eventpayload);
    }
    return http.get(url).catch((err: HttpErrorResponse) => {
      return _throw(this.handleError(err));
    });
  }

  /**
   * This is the key API of Data-mocking framework
   * @param url
   * @param method
   * @param eventpayload
   */
  protected handleOfflineAJAX(url: string, method: string, eventpayload: any): Observable<any> {
    const mockedData = MockDataUtil.getMockedData(url, method, eventpayload);
    const fakeObservable = Observable.create(obs => {
      obs.next(mockedData);
      obs.complete();
    }).delay(1000);
    return fakeObservable;
  }

  /**
   * This API  enables one UI logical step to make multiple REST API calls.
   *
   * configs is [
   *  {
   *      "url": url1;
   *      "payload" : payloadJson1
   *   },
   *  {
   *      "url": url2;
   *      "payload" : payloadJson2
   *   }
   *
   * ]
   */
  protected invokeMultipleAJAXPostGetCalls(configs: any, http: HttpClient, eventpayload: any, methodString: string): Observable<any> {

    this.publishAJAX(eventpayload);
    const observables = []; // new Array <Observable>;
    for (let i = 0; i < configs.length; i++) {
      const url = configs[i]['url'];
      const payload = configs[i]['payload'];
      let obserable: Observable<any>;
      if (this.isToMockData()) {
        const method = methodString;
        return this.handleOfflineAJAX(url, method, eventpayload);
      } else {
        if (methodString === 'GET') {
          obserable = http.get(url, payload);
        } else if (methodString === 'POST') {
          obserable = http.post(url, payload);
        }
      }
      observables.push(obserable);
    }
    return Observable.forkJoin(observables);
  }

  /**
   * Get login user tenant information
   *
   */
  getTenant(): string {
    return Utility.getInstance().getTenant();
  }

  /**
   * This API build URL string
   * For example: given 'endpoints' , it will return endpoints/default/endpoints
   * @param token
   */
  buildURLHelper(token: string): string {
    return token + '/' + this.getTenant() + '/' + token;
  }

  isToUserRealData(): boolean {
    const isUseRealData = Utility.getInstance().getControllerService().useRealData;
    return (isUseRealData) ? isUseRealData : environment.isRESTAPIReady;
  }

  /**
   * Hook to be overridden in child class
   * If this returns false, the polling will terminate
   */
  protected pollingHasObservers() {
    return false;
  }

  /**
   * Hook to be overridden in child class
   * Called when the timer ticks to fetch data
   * @param useRealData
   */
  protected pollingFetchData(useRealData): void {
  }

  /**
   * Initates polling for data
   * Will automatically terminate if pollingHasObservers returns false
   * @param useRealData Currently here as not all of the data fetched is ready in the backend for workload service
   */
  public initiatePolling(useRealData, interval = 5000, initialDelay = 200): void {
    if (this.pollingTimerSource == null) {
      this.pollingTimerSource = timer(initialDelay, interval);
      this.pollingTimerSubscription = this.pollingTimerSource.subscribe(() => {
        // Check if there are any subscribers to our subjects
        // if not, we terminate this timer.
        if (!this.pollingHasObservers()) {
          this.terminatePolling();
          return;
        }
        if (useRealData) {
          // Setting up data to be polled
          this.pollingFetchData(useRealData);
        }
      });
      // Call fetch data once for fake data
      // This is because otherwise the data will keep dramatically fluctuating
      if (!useRealData) {
        this.pollingFetchData(useRealData);
      }
    }
  }

  /**
   * terminates the polling timer
   */
  protected terminatePolling() {
    if (this.pollingTimerSubscription != null) {
      this.pollingTimerSubscription.unsubscribe();
      this.pollingTimerSource = null;
      this.pollingTimerSubscription = null;
    }
  }
}
