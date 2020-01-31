import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';
import { TrimDefaultsAndEmptyFields, TrimUIFields } from '../../../v1/utils/utility';

import { ITokenauthNodeTokenResponse,TokenauthNodeTokenResponse } from '../../models/generated/tokenauth';

@Injectable()
export class Tokenauthv1Service extends AbstractService {
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

  public GetGenerateNodeToken(queryParam: any = null):Observable<{body: ITokenauthNodeTokenResponse | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/tokenauth/v1/node';
    const opts = {
      eventID: 'GetGenerateNodeToken',
      objType: 'TokenauthNodeTokenResponse',
      isStaging: false,
    }
    return this.invokeAJAXGetCall(url, queryParam, opts) as Observable<{body: ITokenauthNodeTokenResponse | Error, statusCode: number}>;
  }
  
}