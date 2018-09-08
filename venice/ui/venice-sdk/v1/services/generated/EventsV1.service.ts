import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IEventsEventList } from '../../models/generated/events';

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

  /** http://<...>/events/v1/events */
  public GetEvents-1():Observable<{body: IEventsEventList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/events/v1/events';
    return this.invokeAJAXGetCall(url, 'GetEvents-1') as Observable<{body: IEventsEventList | Error, statusCode: number}>;
  }
  
}