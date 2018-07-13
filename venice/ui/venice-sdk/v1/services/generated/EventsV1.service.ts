import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IEventsEvent,IEventsEventList } from '../../models/generated/events';

@Injectable()
export class EventsV1Service extends AbstractService {
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

  /** http://<...>/events/v1/event/12345 will be translated to a gRPC query - GetEvent(uuid:"12345") */
  public GetEvent(UUID):Observable<{body: IEventsEvent | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/event/{UUID}';
    url = url.replace('{UUID}', UUID);
    return this.invokeAJAXGetCall(url, 'GetEvent') as Observable<{body: IEventsEvent | Error, statusCode: number}>;
  }
  
  /** http://<...>/events/v1/events */
  public GetEvents():Observable<{body: IEventsEventList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/events';
    return this.invokeAJAXGetCall(url, 'GetEvents') as Observable<{body: IEventsEventList | Error, statusCode: number}>;
  }
  
  /** http://<...>/events/v1/events */
  public GetEventsPost(body: any):Observable<{body: IEventsEventList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/events';
    return this.invokeAJAXPostCall(url, body.getValues(), 'GetEvents') as Observable<{body: IEventsEventList | Error, statusCode: number}>;
  }
  
}