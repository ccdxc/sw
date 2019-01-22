import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IObjstoreObjectList,IApiStatus,IObjstoreObject } from '../../models/generated/objstore';

@Injectable()
export class Objstorev1Service extends AbstractService {
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

  /** List Object objects */
  public ListObject(O_Namespace, queryParam: any = null):Observable<{body: IObjstoreObjectList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/objstore/v1/{O.Namespace}/objects';
    url = url.replace('{O.Namespace}', O_Namespace);
    return this.invokeAJAXGetCall(url, queryParam, 'ListObject') as Observable<{body: IObjstoreObjectList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Object object */
  public GetObject(O_Namespace,O_Name, queryParam: any = null):Observable<{body: IObjstoreObject | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/objstore/v1/{O.Namespace}/objects/{O.Name}';
    url = url.replace('{O.Namespace}', O_Namespace);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetObject') as Observable<{body: IObjstoreObject | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete Object object */
  public DeleteObject(O_Namespace,O_Name):Observable<{body: IObjstoreObject | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/objstore/v1/{O.Namespace}/objects/{O.Name}';
    url = url.replace('{O.Namespace}', O_Namespace);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteObject') as Observable<{body: IObjstoreObject | IApiStatus | Error, statusCode: number}>;
  }
  
}