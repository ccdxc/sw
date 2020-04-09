import { AbstractService, ServerEvent } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { IAuditAuditEvent,AuditAuditEvent } from '../../models/generated/audit';

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

  /** Fetches an audit event given its uuid */
  public GetGetEvent(UUID, queryParam: any = null):Observable<{body: IAuditAuditEvent | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/audit/v1/events/{UUID}';
    url = url.replace('{UUID}', UUID);
    const opts = {
      eventID: 'GetGetEvent',
      objType: 'AuditAuditEvent',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: IAuditAuditEvent | Error, statusCode: number}>;
  }
  
}