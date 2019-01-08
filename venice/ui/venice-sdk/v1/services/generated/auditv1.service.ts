import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IAuditEvent } from '../../models/generated/audit';

@Injectable()
export class Auditv1Service extends AbstractService {
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

  /** Fetches an audit event given its uuid- http://<...>/audit/v1/events/12345 */
  public GetGetEvent(UUID, queryParam: any = null):Observable<{body: IAuditEvent | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/audit/v1/events/{UUID}';
    url = url.replace('{UUID}', UUID);
    return this.invokeAJAXGetCall(url, queryParam, 'GetGetEvent') as Observable<{body: IAuditEvent | Error, statusCode: number}>;
  }
  
}