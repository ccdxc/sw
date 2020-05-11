import { AbstractService, ServerEvent } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { IRoutingNeighborList,RoutingNeighborList,IApiStatus,ApiStatus } from '../../models/generated/routing';

@Injectable()
export class Routingv1Service extends AbstractService {
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

  /** List Neighbor objects */
  public ListNeighbor(queryParam: any = null):Observable<{body: IRoutingNeighborList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/routing/v1/neighbors';
    const opts = {
      eventID: 'ListNeighbor',
      objType: 'RoutingNeighborList',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IRoutingNeighborList | IApiStatus | Error, statusCode: number}>;
  }
  
}