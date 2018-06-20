import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { MonitoringMirrorSessionList,ApiStatus,MonitoringMirrorSession,MonitoringAlertDestinationList,MonitoringAlertDestination,MonitoringAlertPolicyList,MonitoringAlertPolicy,MonitoringAlertList,MonitoringAlert,MonitoringEventPolicy,MonitoringEventList,MonitoringEvent,MonitoringFlowExportPolicyList,MonitoringFlowExportPolicy,MonitoringFwlogPolicyList,MonitoringFwlogPolicy,MonitoringStatsPolicyList,MonitoringStatsPolicy } from '../../models/generated/monitoring';

@Injectable()
export class MonitoringV1Service extends AbstractService {
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

  /** Retreives a list of MirrorSession objects */
  public ListMirrorSession():Observable<{body: MonitoringMirrorSessionList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListMirrorSession') as Observable<{body: MonitoringMirrorSessionList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new MirrorSession object */
  public AddMirrorSession(body: MonitoringMirrorSession):Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddMirrorSession') as Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the MirrorSession object */
  public GetMirrorSession(O_Name):Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetMirrorSession') as Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the MirrorSession object */
  public DeleteMirrorSession(O_Name):Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteMirrorSession') as Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}>;
  }
  
  public UpdateMirrorSession(O_Name, body: MonitoringMirrorSession):Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateMirrorSession') as Observable<{body: MonitoringMirrorSession | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of AlertDestination objects */
  public ListAlertDestination():Observable<{body: MonitoringAlertDestinationList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAlertDestination') as Observable<{body: MonitoringAlertDestinationList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new AlertDestination object */
  public AddAlertDestination(body: MonitoringAlertDestination):Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAlertDestination') as Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the AlertDestination object */
  public GetAlertDestination(O_Name):Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAlertDestination') as Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the AlertDestination object */
  public DeleteAlertDestination(O_Name):Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertDestination') as Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}>;
  }
  
  public UpdateAlertDestination(O_Name, body: MonitoringAlertDestination):Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAlertDestination') as Observable<{body: MonitoringAlertDestination | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of AlertPolicy objects */
  public ListAlertPolicy():Observable<{body: MonitoringAlertPolicyList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAlertPolicy') as Observable<{body: MonitoringAlertPolicyList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new AlertPolicy object */
  public AddAlertPolicy(body: MonitoringAlertPolicy):Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAlertPolicy') as Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the AlertPolicy object */
  public GetAlertPolicy(O_Name):Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAlertPolicy') as Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the AlertPolicy object */
  public DeleteAlertPolicy(O_Name):Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertPolicy') as Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateAlertPolicy(O_Name, body: MonitoringAlertPolicy):Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAlertPolicy') as Observable<{body: MonitoringAlertPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Alert objects */
  public ListAlert():Observable<{body: MonitoringAlertList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alerts';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAlert') as Observable<{body: MonitoringAlertList | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Alert object */
  public GetAlert(O_Name):Observable<{body: MonitoringAlert | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alerts/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAlert') as Observable<{body: MonitoringAlert | ApiStatus, statusCode: number}>;
  }
  
  public UpdateAlert(O_Name, body: MonitoringAlert):Observable<{body: MonitoringAlert | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/alerts/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAlert') as Observable<{body: MonitoringAlert | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new EventPolicy object */
  public AddEventPolicy(body: MonitoringEventPolicy):Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/eventPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddEventPolicy') as Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the EventPolicy object */
  public GetEventPolicy(O_Name):Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/eventPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetEventPolicy') as Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the EventPolicy object */
  public DeleteEventPolicy(O_Name):Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/eventPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEventPolicy') as Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateEventPolicy(O_Name, body: MonitoringEventPolicy):Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/eventPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateEventPolicy') as Observable<{body: MonitoringEventPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of Event objects */
  public ListEvent():Observable<{body: MonitoringEventList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/events';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListEvent') as Observable<{body: MonitoringEventList | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the Event object */
  public GetEvent(O_Name):Observable<{body: MonitoringEvent | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/events/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetEvent') as Observable<{body: MonitoringEvent | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of FlowExportPolicy objects */
  public ListFlowExportPolicy():Observable<{body: MonitoringFlowExportPolicyList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListFlowExportPolicy') as Observable<{body: MonitoringFlowExportPolicyList | ApiStatus, statusCode: number}>;
  }
  
  /** Creates a new FlowExportPolicy object */
  public AddFlowExportPolicy(body: MonitoringFlowExportPolicy):Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddFlowExportPolicy') as Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the FlowExportPolicy object */
  public GetFlowExportPolicy(O_Name):Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetFlowExportPolicy') as Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Deletes the FlowExportPolicy object */
  public DeleteFlowExportPolicy(O_Name):Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteFlowExportPolicy') as Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateFlowExportPolicy(O_Name, body: MonitoringFlowExportPolicy):Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateFlowExportPolicy') as Observable<{body: MonitoringFlowExportPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of FwlogPolicy objects */
  public ListFwlogPolicy():Observable<{body: MonitoringFwlogPolicyList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/fwlogPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListFwlogPolicy') as Observable<{body: MonitoringFwlogPolicyList | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the FwlogPolicy object */
  public GetFwlogPolicy(O_Name):Observable<{body: MonitoringFwlogPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetFwlogPolicy') as Observable<{body: MonitoringFwlogPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateFwlogPolicy(O_Name, body: MonitoringFwlogPolicy):Observable<{body: MonitoringFwlogPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateFwlogPolicy') as Observable<{body: MonitoringFwlogPolicy | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives a list of StatsPolicy objects */
  public ListStatsPolicy():Observable<{body: MonitoringStatsPolicyList | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/statsPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListStatsPolicy') as Observable<{body: MonitoringStatsPolicyList | ApiStatus, statusCode: number}>;
  }
  
  /** Retreives the StatsPolicy object */
  public GetStatsPolicy(O_Name):Observable<{body: MonitoringStatsPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/statsPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetStatsPolicy') as Observable<{body: MonitoringStatsPolicy | ApiStatus, statusCode: number}>;
  }
  
  public UpdateStatsPolicy(O_Name, body: MonitoringStatsPolicy):Observable<{body: MonitoringStatsPolicy | ApiStatus, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/v1/monitoring/{O.Tenant}/statsPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateStatsPolicy') as Observable<{body: MonitoringStatsPolicy | ApiStatus, statusCode: number}>;
  }
  
}