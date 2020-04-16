import { HttpClient, HttpErrorResponse, HttpResponse } from '@angular/common/http';
import { MockDataUtil } from '@app/common/MockDataUtil';
import { Utility } from '@app/common/Utility';
import { AUTH_KEY } from '@app/core/auth/auth.reducer';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { VeniceResponse } from '@app/models/frontend/shared/veniceresponse.interface';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { MethodOpts, ServerEvent } from '@sdk/v1/services/generated/abstract.service';
import * as oboe from 'oboe';
import { NEVER, Observable, Subject, Subscriber, Subscription, TeardownLogic, BehaviorSubject, ReplaySubject } from 'rxjs';
import { WebSocketSubject } from 'rxjs/observable/dom/WebSocketSubject';
import { delay, publishReplay, refCount, bufferTime } from 'rxjs/operators';
import { HttpEventUtility } from '@app/common/HttpEventUtility';

// VS-1306 experimental code. See if we can preserve data to re-run createDataCache() if ListXXXCache() fails
export interface CreateCacheConfig {
  constructor: any;
  key: string;
  listFn: () => Observable<VeniceResponse>;
  watchFn: (query: any) => Observable<VeniceResponse>;
}

export interface CreateCacheConfigMap {
  [key: string]: CreateCacheConfig;
}

/**
 * This class is the core component of invoking REST API.  All *.service.ts use this class.
 * (e.g. see src/app/services/generated/auth.service.ts this.serviceUtility = new GenServiceUtility(..) ...  this.serviceUtility.setId(this.getClassName());)
 *
 * key API:
 * this.invokeAJAX() is the key API
 *        --> in case it is a "WATCH" web-socket call, --> handleWatchRequest()
 *
 * this.handleWatchRequest() is the watch response handler.  It also tries to keep web-socket connections alive.
 *
 * this.teardown() release all resources.
 *
 *
 * Misc:
 *    oboeObserverCreate() web-socket response handler.
 *    isAllowed() controls whether to invoke the REST call.
 *    setId() is used in *.service.ts. Using,  console.log('GenUtilty.subcribeToEvents() ' + this.id); we know which servie class is using this GenUtility instance.
 */
export class GenServiceUtility {

  protected _http;
  protected urlServiceMap: { [method: string]: Observable<VeniceResponse> } = {};
  protected dataCacheMap: { [method: string]: WebSocketSubject<any> } = {};
  protected urlWsMap: { [method: string]: WebSocketSubject<any> } = {};
  protected cacheMap: { [method: string]: ReplaySubject<ServerEvent<any>> } = {};
  protected ajaxStartCallback: (payload: any) => void;
  protected ajaxEndCallback: (payload: any) => void;
  protected useWebSockets: boolean;

  pingServerTimerMap: { [url: string]: any } = {};
  subscriptions: Subscription[] = [];
  logoutSubscription: Subscription = null;

  id: string = null;

  constructor(http: HttpClient, ajaxStartCallback, ajaxEndCallback, useWebSockets = true) {
    this._http = http;
    this.ajaxStartCallback = ajaxStartCallback;
    this.ajaxEndCallback = ajaxEndCallback;
    this.useWebSockets = useWebSockets;
    window.addEventListener('unload', (event) => {
      this.teardown();
    });
  }


  setId(inputId: string) {
    this.id = (inputId) ? inputId : this.id;
  }

  /**
   * Listen to logout event.
   * We subscribe only once.
   */
  subcribeToEvents() {
    if (!this.logoutSubscription && Utility.getInstance().getControllerService()) {
      // for debug: // console.log('GenUtilty.subcribeToEvents() ' + this.id);
      this.logoutSubscription = Utility.getInstance().getControllerService().subscribe(Eventtypes.LOGOUT, (payload) => {
        this.onLogout(payload);
      });
    }
  }

  clearOneTimer(pingServerTimer: any) {
    if (pingServerTimer) {
      clearInterval(pingServerTimer);
    }
  }

  clearAllTimers() {
    for (const key in this.pingServerTimerMap) {
      if (this.pingServerTimerMap.hasOwnProperty(key)) {
        this.clearOneTimer(this.pingServerTimerMap[key]);
      }
    }
  }

  /**
   * Clean up.
   */
  teardown() {
    this.clearAllTimers();
    if (this.logoutSubscription) {
      this.logoutSubscription.unsubscribe();
      this.logoutSubscription = null;
    }
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
    for (const key in this.urlWsMap) {
      if (this.urlWsMap.hasOwnProperty(key)) {
        try {
          this.urlWsMap[key].unsubscribe();
        } catch (e) {
          console.error(e);
        }
      }
    }
    for (const key in this.urlServiceMap) {
      if (this.urlServiceMap.hasOwnProperty(key)) {
        try {
          delete this.urlServiceMap[key];
        } catch (e) {
          console.error(e);
        }
      }
    }
    for (const key in this.cacheMap) {
      if (this.cacheMap.hasOwnProperty(key)) {
        try {
          delete this.cacheMap[key];
        } catch (e) {
          console.error(e);
        }
      }
    }
  }

  onLogout(payload: any) {
    this.teardown();
  }

  convertEventID(opts: MethodOpts): UIRolePermissions {
    let action;
    if (opts.eventID.startsWith('Watch')) {
      action = 'read';
      opts.objType = opts.objType.replace('AutoMsg', '');
      opts.objType = opts.objType.replace('WatchHelper', '');
    } else if (opts.eventID.startsWith('List')) {
      action = 'read';
      opts.objType = opts.objType.replace('List', '');
    } else if (opts.eventID.startsWith('Get')) {
      action = 'read';
    } else if (opts.eventID.startsWith('Update')) {
      action = 'update';
    } else if (opts.eventID.startsWith('Delete')) {
      if (opts.isStaging) {
        action = 'clear';
      } else {
        action = 'delete';
      }
    } else if (opts.eventID.startsWith('Add')) {
      if (opts.isStaging) {
        action = 'commit';
      } else {
        action = 'create';
      }
    } else {
      const catKind = Utility.actionMethodNameToObject(opts.eventID);
      action = 'create';
      opts.objType = catKind.category + catKind.kind;
    }
    const key = opts.objType.toLowerCase() + '_' + action;
    return UIRolePermissions[key];
  }


  /**
   * Function to create an observer that starts an oboe instance
   * and emits whenever oboe receives a full object. When an observer unsubscribes,
   * it will close the connection
   *
   * @param url URL for oboe to listen to
   * @param payload Query param object to send with the request
   * @param eventPayload payload to publish on connection end
   */
  protected oboeObserverCreate(url: string, payload: any, eventPayload: any): ((this: Observable<any>, subscriber: Subscriber<any>) => TeardownLogic) {
    return (observer) => {
      const headers = {};
      headers[AUTH_KEY] = Utility.getInstance().getXSRFtoken();

      const config = {
        'url': url,
        'method': 'GET',
        headers: headers
      };
      const oboeService = oboe(config);

      // The '!' will only consume complete json objects
      oboeService.node('!', (data) => {
        observer.next({ body: data, statusCode: null });
      });

      // If the connection fails for any reason
      oboeService.fail(result => {
        observer.error({ body: result.jsonBody, statusCode: result.statusCode });
      });

      // When the consumer unsubscribes, close the connection and publish ajax end
      return {
        unsubscribe: () => {
          oboeService.abort();
          this.ajaxEndCallback(eventPayload);
          delete this.urlServiceMap[url];
        }
      };
    };
  }

  /**
   * This is the key function for object cache
   *
   * This API has magic to return object list and websocket events.
   * The return object looks like:
   *
   * {
   * "data": [
   *  {
   *    "kind": "Host"  // host event
   *  }
   * ],
   * "events": [
   *  {
   *    "type": "Deleted",  // event.type
   *    "object": {
   *       "kind": "Host"
   *     }
   *   }
   * ],
   * "connIsErrorState": false
   * }
   *
   *  Service objects (such as cluster.service.ts or workload.service.ts) invoke this function by passing in listXXX function and watchXXX function
   *  This API will first fetch the object list, then use websocket to get create/update/delete events.
   *
   *  Say listXXX call get objest of item.meta.['resource-version'] = 1. Websocket will only watch item.meta.['resource-version']>1
   *
   *   returnObject.data always contains the latest objects.  returnObject.data.events[ ... ] contains the ws-events.  event.type  is of create/update/delete
   *
   *  See hosts.component.ts for example
   */
  public createDataCache<T>(constructor: any, key: string, listFn: () => Observable<VeniceResponse>, watchFn: (query: any) => Observable<VeniceResponse>): Observable<ServerEvent<T>> {
    let observer = new ReplaySubject<ServerEvent<T>>(1);
    if (this.cacheMap[key] != null) {
      // Fetch same observable that we have given out
      observer = this.cacheMap[key];
    }

    // We start the watch call first so that we don't miss any events
    // while the list call is being returned to us.
    // Watch events are processed as they come, but the resulting array
    // won't be sent to clients until the list has completed.
    // HttpEventUtility will not process an event for an object that has an older resource version
    // so old data in the list will not overwrite the watch event, and vice-versa

    // Only replay the last emitted event to new subscribers
    const eventUtility = new HttpEventUtility<T>(constructor);
      // TODO: the retry should be replaced with an observable retry
     let listDone = false;
     const watchMethod = () => {
        const watchBody = {};
        watchBody['O.resource-version'] = '-1';
        const watchSub = watchFn(watchBody).subscribe(watchResp => {
          const evts = (<any>watchResp).events;
          eventUtility.processEvents({
            events: evts,
          });
          if (!listDone) {
            // Don't emit until list call is done
            return;
          }
          observer.next({
            data: eventUtility.array,
            events: evts,
            connIsErrorState: false,
          });
        },
          (error) => {
            const controller = Utility.getInstance().getControllerService();
            controller.webSocketErrorHandler('Failed to get ' + key)(error);
            observer.next({
              data: eventUtility.array,
              events: [],
              connIsErrorState: true,
            });
            setTimeout(() => {
              watchMethod();
            }, 5000);
          });
        this.subscriptions.push(watchSub);
      };
    watchMethod();

    const sub = listFn().subscribe(resp => {
      const body = resp.body;
      if (body && body.items) {
        const events = body.items.map(item => {
          return {
            type: 'Created',
            object: item,
          };
        });
        eventUtility.processEvents({ events: events });
      }
      observer.next({
        data: eventUtility.array,
        events: [],
        connIsErrorState: false,
      });
      listDone = true;
    },
      (error) => {
        const controller = Utility.getInstance().getControllerService();
        controller.invokeRESTErrorToaster('Error', 'Failed to get ' + key);
        observer.next({
          data: eventUtility.array,
          events: [],
          connIsErrorState: true,
        });
        setTimeout(() => {
          // Rerun cache
          this.createDataCache(constructor, key, listFn, watchFn);
        }, 5000);
      });
    this.subscriptions.push(sub);
    this.cacheMap[key] = observer;
    return observer;
  }

  public handleListFromCache(key: string, createCacheFn: any): Observable<ServerEvent<any>> {
    if (this.cacheMap[key]) {
      return this.cacheMap[key];
    } else {
      // VS-1306. If browser is idle for a while. User is logged out. But Venice-UI is on browser and ClusterService object exists. We try to re-build the cache.
      if (createCacheFn != null) {
        return createCacheFn();
      } else {
        if (Utility.getInstance().getControllerService()) {
          Utility.getInstance().getControllerService().invokeErrorToaster('Session Expired', 'Please refresh your browser');
        } else {
          alert('Session Expired. Please refresh your browser');
        }
      }
    }
  }

  /**
   * If no watch connection exists for the given method, we create a new one and return the observable.
   * If one exists, we return the observable. Since we use a replay subject, and late
   * subscribers will receive all events that have happened in the connection.
   *
   * @param method HTTP method
   * @param url Url to issue a GET request against, used to uniquely idenitfy the observer objects
   * @param eventPayload EventPayload that will be published on AJAX End
   */
  protected handleWatchRequest(url: string, payload: any, eventPayload: any): Observable<VeniceResponse> {
    if (payload != null) {
      // we add the query params to the url
      url += '?' + Utility.getJQuery().param(payload);
    }
    if (this.useWebSockets) {
      if (this.urlWsMap[url] == null) {
        url = url.replace('http://', 'ws://');
        url = url.replace('https://', 'wss://');
        const observer = new WebSocketSubject({
          url: url,
        });
        // In this code block, we use a timer to invoke _observer.next() in order to keep UI page which subscribes to watch response active. (prevent browser closes ws due to ws idle)
        const pingServerTimer = setInterval(() => {
          const _observer = this.urlWsMap[url];
          if (_observer != null) {
            if (Utility.getInstance().getControllerService().isUserLogin()) {
              // for debug: // console.log('GenUtil.handleWatchRequest() ping server timer ' + this.id + ' ' + url);
              _observer.next(true);  // fire up observer to keep ws active
            } else {
              // for debug // console.log('GenUtil.handleWatchRequest() ping server timer/logout ' + this.id + ' ' + url);
              // if user is logout, we teardown everything.
              this.teardown();
            }
          } else {
            this.clearOneTimer(this.pingServerTimerMap[url]);
          }
        }, 60000);
        this.pingServerTimerMap[url] = pingServerTimer;
        this.urlWsMap[url] = observer;
      }
      return this.urlWsMap[url];
    }

    // Watch
    if (this.urlServiceMap[url] == null) {
      // Creating cold observer that emits events when oboe receives new data
      const oboeObserver: Observable<any> = new Observable(this.oboeObserverCreate(url, payload, eventPayload));
      // Creating a replay subject that subscribes and unsubscribes from the oboeObserver source
      // only if it has subscribers.
      // The connection will only be open if there is a listener, and closed as soon as there
      // are no more listeners
      const observer = oboeObserver.pipe(publishReplay(), refCount());
      this.urlServiceMap[url] = observer;
    }
    return this.urlServiceMap[url];
  }



  public isAllowed(eventPayloadID: any) {
    // checking maintenance mode (but get version object)
    const blockCondition = (Utility.getInstance().getMaintenanceMode() && eventPayloadID !== 'WatchVersion' && eventPayloadID !== 'WatchRollout' && eventPayloadID !== 'GetRollout' && eventPayloadID !== 'WatchDistributedServiceCard' && eventPayloadID !== 'StopRollout');

    if (blockCondition) {
      return false;
    }
    return true;
  }

  public invokeAJAX(method: string, url: string, payload: any,
    eventPayloadID: any, isOnline: boolean = false): Observable<VeniceResponse> {
    if (!this.isAllowed(eventPayloadID)) {
      return NEVER;
    }
    this.subcribeToEvents();  // subscribe to event in the very first REST call.
    // Removing time fields as null values will be attempted to be parsed
    if (payload != null) {
      if (payload.meta != null) {
        delete payload.meta['creation-time'];
        delete payload.meta['mod-time'];
      }
      // Removing time fields from api list watch options
      if (payload['creation-time'] == null) {
        delete payload['creation-time'];
      }
      if (payload['mod-time'] == null) {
        delete payload['mod-time'];
      }
    }

    const eventPayload = { 'ajax': 'start', 'method': method, 'name': eventPayloadID + '-ajax', 'url': url };
    this.ajaxStartCallback(eventPayload);

    if (!isOnline) {
      return this.handleOfflineAJAX(url, method, eventPayload);
    }

    if (url.indexOf('watch') >= 0) {
      // We use oboe to load the chunked responses if it is a watch request
      return this.handleWatchRequest(url, payload, eventPayload);
    }

    let observer: Observable<HttpResponse<Object>>;
    switch (method) {
      case 'GET':
        if (payload == null) {
          observer = this._http.get(url, { observe: 'response' });
        } else {
          observer = this._http.get(url, { params: payload, observe: 'response' });
        }
        break;
      case 'POST':
        observer = this._http.post(url, payload, { observe: 'response' });
        break;
      case 'PUT':
        observer = this._http.put(url, payload, { observe: 'response' });
        break;
      case 'DELETE':
        observer = this._http.delete(url, { observe: 'response' });
        break;
      default:
        break;
    }

    const retObserver = new Subject<VeniceResponse>();

    observer.subscribe(
      (response: HttpResponse<any>) => {
        this.ajaxEndCallback(eventPayload);
        const statusCode = response.status;
        const body = response.body;
        retObserver.next({ body: body, statusCode: statusCode });
        retObserver.complete();
      },
      (error: HttpErrorResponse) => {
        this.ajaxEndCallback(eventPayload);
        retObserver.error({ body: error.error, statusCode: error.status });
      }
    );

    return retObserver;
  }

  /**
   * This is the key API of Data-mocking framework
   * @param url
   * @param method
   * @param eventpayload
   */
  protected handleOfflineAJAX(url: string, method: string, eventpayload: any): Observable<VeniceResponse> {
    const mockedData = MockDataUtil.getMockedData(url, method, eventpayload);
    const fakeObservable = new Observable<VeniceResponse>(obs => {
      obs.next({ body: mockedData, statusCode: 200 });
      obs.complete();
    }).pipe(delay(1000));
    return fakeObservable;
  }

}

