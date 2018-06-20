import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { NetworkLbPolicyList,ApiStatus,NetworkLbPolicy,NetworkNetworkList,NetworkNetwork,NetworkServiceList,NetworkService } from '../../models/generated/network';

@Injectable()
export class NetworkV1Service extends AbstractService {
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

  /** Retreives a list of LbPolicy objects */
  public ListLbPolicy():Observable<{body: NetworkLbPolicyList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListLbPolicy') as Observable<{body: NetworkLbPolicyList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new LbPolicy object */
  public AddLbPolicy(body: NetworkLbPolicy):Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/lb-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddLbPolicy') as Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the LbPolicy object */
  public GetLbPolicy(O_Name):Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetLbPolicy') as Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the LbPolicy object */
  public DeleteLbPolicy(O_Name):Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteLbPolicy') as Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateLbPolicy(O_Name, body: NetworkLbPolicy):Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/lb-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateLbPolicy') as Observable<{body: NetworkLbPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Network objects */
  public ListNetwork():Observable<{body: NetworkNetworkList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListNetwork') as Observable<{body: NetworkNetworkList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Network object */
  public AddNetwork(body: NetworkNetwork):Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/networks';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddNetwork') as Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Network object */
  public GetNetwork(O_Name):Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetNetwork') as Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Network object */
  public DeleteNetwork(O_Name):Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteNetwork') as Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}>;
  }
  
  public UpdateNetwork(O_Name, body: NetworkNetwork):Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/networks/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateNetwork') as Observable<{body: NetworkNetwork | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Service objects */
  public ListService():Observable<{body: NetworkServiceList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListService') as Observable<{body: NetworkServiceList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new Service object */
  public AddService(body: NetworkService):Observable<{body: NetworkService | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/services';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddService') as Observable<{body: NetworkService | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Service object */
  public GetService(O_Name):Observable<{body: NetworkService | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetService') as Observable<{body: NetworkService | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the Service object */
  public DeleteService(O_Name):Observable<{body: NetworkService | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteService') as Observable<{body: NetworkService | ApiStatus, statusCode: number}>;
  }
  
  public UpdateService(O_Name, body: NetworkService):Observable<{body: NetworkService | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/network/{O.Tenant}/services/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateService') as Observable<{body: NetworkService | ApiStatus, statusCode: number}>;
  }
  
}