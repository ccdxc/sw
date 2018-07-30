import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IMonitoringMirrorSessionList,IApiStatus,IMonitoringMirrorSession,MonitoringMirrorSession,IMonitoringTroubleshootingSessionList,IMonitoringTroubleshootingSession,MonitoringTroubleshootingSession,IMonitoringAlertDestinationList,IMonitoringAlertDestination,MonitoringAlertDestination,IMonitoringAlertPolicyList,IMonitoringAlertPolicy,MonitoringAlertPolicy,IMonitoringAlertList,IMonitoringAlert,MonitoringAlert,IMonitoringEventPolicy,MonitoringEventPolicy,IMonitoringFlowExportPolicyList,IMonitoringFlowExportPolicy,MonitoringFlowExportPolicy,IMonitoringFwlogPolicyList,IMonitoringFwlogPolicy,MonitoringFwlogPolicy,IMonitoringStatsPolicyList,IMonitoringStatsPolicy,MonitoringStatsPolicy,IMonitoringAutoMsgMirrorSessionWatchHelper,IMonitoringAutoMsgAlertDestinationWatchHelper,IMonitoringAutoMsgAlertPolicyWatchHelper,IMonitoringAutoMsgAlertWatchHelper,IMonitoringAutoMsgEventPolicyWatchHelper,IMonitoringAutoMsgFlowExportPolicyWatchHelper,IMonitoringAutoMsgFwlogPolicyWatchHelper,IMonitoringAutoMsgStatsPolicyWatchHelper } from '../../models/generated/monitoring';

@Injectable()
export class Monitoringv1Service extends AbstractService {
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

  /** List MirrorSession objects */
  public ListMirrorSession():Observable<{body: IMonitoringMirrorSessionList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListMirrorSession') as Observable<{body: IMonitoringMirrorSessionList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create MirrorSession object */
  public AddMirrorSession(body: MonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get MirrorSession object */
  public GetMirrorSession(O_Name):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete MirrorSession object */
  public DeleteMirrorSession(O_Name):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update MirrorSession object */
  public UpdateMirrorSession(O_Name, body: MonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List TroubleshootingSession objects */
  public ListTroubleshootingSession():Observable<{body: IMonitoringTroubleshootingSessionList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSessionList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TroubleshootingSession object */
  public AddTroubleshootingSession(body: MonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TroubleshootingSession object */
  public GetTroubleshootingSession(O_Name):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TroubleshootingSession object */
  public DeleteTroubleshootingSession(O_Name):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TroubleshootingSession object */
  public UpdateTroubleshootingSession(O_Name, body: MonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List AlertDestination objects */
  public ListAlertDestination():Observable<{body: IMonitoringAlertDestinationList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAlertDestination') as Observable<{body: IMonitoringAlertDestinationList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AlertDestination object */
  public AddAlertDestination(body: MonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get AlertDestination object */
  public GetAlertDestination(O_Name):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete AlertDestination object */
  public DeleteAlertDestination(O_Name):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AlertDestination object */
  public UpdateAlertDestination(O_Name, body: MonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List AlertPolicy objects */
  public ListAlertPolicy():Observable<{body: IMonitoringAlertPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAlertPolicy') as Observable<{body: IMonitoringAlertPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AlertPolicy object */
  public AddAlertPolicy(body: MonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get AlertPolicy object */
  public GetAlertPolicy(O_Name):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete AlertPolicy object */
  public DeleteAlertPolicy(O_Name):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AlertPolicy object */
  public UpdateAlertPolicy(O_Name, body: MonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Alert objects */
  public ListAlert():Observable<{body: IMonitoringAlertList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alerts';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListAlert') as Observable<{body: IMonitoringAlertList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Alert object */
  public GetAlert(O_Name):Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alerts/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetAlert') as Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Alert object */
  public UpdateAlert(O_Name, body: MonitoringAlert):Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alerts/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateAlert') as Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create EventPolicy object */
  public AddEventPolicy(body: MonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get EventPolicy object */
  public GetEventPolicy(O_Name):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete EventPolicy object */
  public DeleteEventPolicy(O_Name):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update EventPolicy object */
  public UpdateEventPolicy(O_Name, body: MonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FlowExportPolicy objects */
  public ListFlowExportPolicy():Observable<{body: IMonitoringFlowExportPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create FlowExportPolicy object */
  public AddFlowExportPolicy(body: MonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body.getValues(), 'AddFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FlowExportPolicy object */
  public GetFlowExportPolicy(O_Name):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete FlowExportPolicy object */
  public DeleteFlowExportPolicy(O_Name):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FlowExportPolicy object */
  public UpdateFlowExportPolicy(O_Name, body: MonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FwlogPolicy objects */
  public ListFwlogPolicy():Observable<{body: IMonitoringFwlogPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/fwlogPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListFwlogPolicy') as Observable<{body: IMonitoringFwlogPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FwlogPolicy object */
  public GetFwlogPolicy(O_Name):Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetFwlogPolicy') as Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FwlogPolicy object */
  public UpdateFwlogPolicy(O_Name, body: MonitoringFwlogPolicy):Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateFwlogPolicy') as Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List StatsPolicy objects */
  public ListStatsPolicy():Observable<{body: IMonitoringStatsPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/statsPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'ListStatsPolicy') as Observable<{body: IMonitoringStatsPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get StatsPolicy object */
  public GetStatsPolicy(O_Name):Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/statsPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'GetStatsPolicy') as Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update StatsPolicy object */
  public UpdateStatsPolicy(O_Name, body: MonitoringStatsPolicy):Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/statsPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'UpdateStatsPolicy') as Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch MirrorSession objects */
  public WatchMirrorSession():Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchMirrorSession') as Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AlertDestination objects */
  public WatchAlertDestination():Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchAlertDestination') as Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AlertPolicy objects */
  public WatchAlertPolicy():Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchAlertPolicy') as Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Alert objects */
  public WatchAlert():Observable<{body: IMonitoringAutoMsgAlertWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/alerts';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchAlert') as Observable<{body: IMonitoringAutoMsgAlertWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch EventPolicy objects */
  public WatchEventPolicy():Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/event-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchEventPolicy') as Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FlowExportPolicy objects */
  public WatchFlowExportPolicy():Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchFlowExportPolicy') as Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FwlogPolicy objects */
  public WatchFwlogPolicy():Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/fwlogPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchFwlogPolicy') as Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch StatsPolicy objects */
  public WatchStatsPolicy():Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/statsPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, 'WatchStatsPolicy') as Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}