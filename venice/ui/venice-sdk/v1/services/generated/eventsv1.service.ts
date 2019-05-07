import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields } from '../../../v1/utils/utility';

import { IEventsEventList,EventsEventList,IEventsEvent,EventsEvent } from '../../models/generated/events';

@Injectable()
export class Eventsv1Service extends AbstractService {
  constructor(protected _http: HttpClient) {
    super(_http);
  }

  /**
   * Override super
   * Get the service class-name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  /** http://<...>/events/v1/events */
  public GetGetEvents_1(queryParam: any = null):Observable<{body: IEventsEventList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/events';
    const opts = {
      eventID: 'GetGetEvents_1',
      objType: 'EventsEventList',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IEventsEventList | Error, statusCode: number}>;
  }
  
  /** http://<...>/events/v1/events */
  public PostGetEvents(body: any, trimObject: boolean = true):Observable<{body: IEventsEventList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/events';
    const opts = {
      eventID: 'PostGetEvents',
      objType: 'EventsEventList',
      isStaging: false,
    }
    return this.invokeAJAXPostCall(url, body, opts) as Observable<{body: IEventsEventList | Error, statusCode: number}>;
  }
  
  /** http://<...>/events/v1/events/12345 will be translated to a gRPC query - GetEvent(uuid:"12345") */
  public GetGetEvent(UUID, queryParam: any = null):Observable<{body: IEventsEvent | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/events/{UUID}';
    url = url.replace('{UUID}', UUID);
    const opts = {
      eventID: 'GetGetEvent',
      objType: 'EventsEvent',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IEventsEvent | Error, statusCode: number}>;
  }
  
}