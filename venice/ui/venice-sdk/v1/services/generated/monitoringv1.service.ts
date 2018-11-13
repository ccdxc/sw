import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IMonitoringMirrorSessionList,IApiStatus,IMonitoringMirrorSession,IMonitoringTroubleshootingSessionList,IMonitoringTroubleshootingSession,IMonitoringAlertDestinationList,IMonitoringAlertDestination,IMonitoringAlertPolicyList,IMonitoringAlertPolicy,IMonitoringAlertList,IMonitoringAlert,IMonitoringEventPolicy,IMonitoringFlowExportPolicyList,IMonitoringFlowExportPolicy,IMonitoringFwlogPolicyList,IMonitoringFwlogPolicy,IMonitoringStatsPolicyList,IMonitoringStatsPolicy,IMonitoringAutoMsgMirrorSessionWatchHelper,IMonitoringAutoMsgAlertDestinationWatchHelper,IMonitoringAutoMsgAlertPolicyWatchHelper,IMonitoringAutoMsgAlertWatchHelper,IMonitoringAutoMsgEventPolicyWatchHelper,IMonitoringAutoMsgFlowExportPolicyWatchHelper,IMonitoringAutoMsgFwlogPolicyWatchHelper,IMonitoringAutoMsgStatsPolicyWatchHelper } from '../../models/generated/monitoring';

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
  public ListMirrorSession_1(queryParam: any = null):Observable<{body: IMonitoringMirrorSessionList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession';
    return this.invokeAJAXGetCall(url, queryParam, 'ListMirrorSession_1') as Observable<{body: IMonitoringMirrorSessionList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create MirrorSession object */
  public AddMirrorSession_1(body: IMonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession';
    return this.invokeAJAXPostCall(url, body, 'AddMirrorSession_1') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get MirrorSession object */
  public GetMirrorSession_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetMirrorSession_1') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete MirrorSession object */
  public DeleteMirrorSession_1(O_Name):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteMirrorSession_1') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update MirrorSession object */
  public UpdateMirrorSession_1(O_Name, body: IMonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateMirrorSession_1') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List TroubleshootingSession objects */
  public ListTroubleshootingSession_1(queryParam: any = null):Observable<{body: IMonitoringTroubleshootingSessionList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession';
    return this.invokeAJAXGetCall(url, queryParam, 'ListTroubleshootingSession_1') as Observable<{body: IMonitoringTroubleshootingSessionList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TroubleshootingSession object */
  public AddTroubleshootingSession_1(body: IMonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession';
    return this.invokeAJAXPostCall(url, body, 'AddTroubleshootingSession_1') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TroubleshootingSession object */
  public GetTroubleshootingSession_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetTroubleshootingSession_1') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TroubleshootingSession object */
  public DeleteTroubleshootingSession_1(O_Name):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTroubleshootingSession_1') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TroubleshootingSession object */
  public UpdateTroubleshootingSession_1(O_Name, body: IMonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateTroubleshootingSession_1') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List AlertDestination objects */
  public ListAlertDestination_1(queryParam: any = null):Observable<{body: IMonitoringAlertDestinationList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations';
    return this.invokeAJAXGetCall(url, queryParam, 'ListAlertDestination_1') as Observable<{body: IMonitoringAlertDestinationList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AlertDestination object */
  public AddAlertDestination_1(body: IMonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations';
    return this.invokeAJAXPostCall(url, body, 'AddAlertDestination_1') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get AlertDestination object */
  public GetAlertDestination_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetAlertDestination_1') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete AlertDestination object */
  public DeleteAlertDestination_1(O_Name):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertDestination_1') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AlertDestination object */
  public UpdateAlertDestination_1(O_Name, body: IMonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateAlertDestination_1') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List AlertPolicy objects */
  public ListAlertPolicy_1(queryParam: any = null):Observable<{body: IMonitoringAlertPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies';
    return this.invokeAJAXGetCall(url, queryParam, 'ListAlertPolicy_1') as Observable<{body: IMonitoringAlertPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AlertPolicy object */
  public AddAlertPolicy_1(body: IMonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies';
    return this.invokeAJAXPostCall(url, body, 'AddAlertPolicy_1') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get AlertPolicy object */
  public GetAlertPolicy_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetAlertPolicy_1') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete AlertPolicy object */
  public DeleteAlertPolicy_1(O_Name):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertPolicy_1') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AlertPolicy object */
  public UpdateAlertPolicy_1(O_Name, body: IMonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateAlertPolicy_1') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Alert objects */
  public ListAlert_1(queryParam: any = null):Observable<{body: IMonitoringAlertList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alerts';
    return this.invokeAJAXGetCall(url, queryParam, 'ListAlert_1') as Observable<{body: IMonitoringAlertList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Alert object */
  public GetAlert_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alerts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetAlert_1') as Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Alert object */
  public UpdateAlert_1(O_Name, body: IMonitoringAlert):Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alerts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateAlert_1') as Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create EventPolicy object */
  public AddEventPolicy_1(body: IMonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy';
    return this.invokeAJAXPostCall(url, body, 'AddEventPolicy_1') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get EventPolicy object */
  public GetEventPolicy_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetEventPolicy_1') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete EventPolicy object */
  public DeleteEventPolicy_1(O_Name):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEventPolicy_1') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update EventPolicy object */
  public UpdateEventPolicy_1(O_Name, body: IMonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateEventPolicy_1') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FlowExportPolicy objects */
  public ListFlowExportPolicy_1(queryParam: any = null):Observable<{body: IMonitoringFlowExportPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'ListFlowExportPolicy_1') as Observable<{body: IMonitoringFlowExportPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create FlowExportPolicy object */
  public AddFlowExportPolicy_1(body: IMonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy';
    return this.invokeAJAXPostCall(url, body, 'AddFlowExportPolicy_1') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FlowExportPolicy object */
  public GetFlowExportPolicy_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetFlowExportPolicy_1') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete FlowExportPolicy object */
  public DeleteFlowExportPolicy_1(O_Name):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteFlowExportPolicy_1') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FlowExportPolicy object */
  public UpdateFlowExportPolicy_1(O_Name, body: IMonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateFlowExportPolicy_1') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FwlogPolicy objects */
  public ListFwlogPolicy_1(queryParam: any = null):Observable<{body: IMonitoringFwlogPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/fwlogPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'ListFwlogPolicy_1') as Observable<{body: IMonitoringFwlogPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FwlogPolicy object */
  public GetFwlogPolicy_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetFwlogPolicy_1') as Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FwlogPolicy object */
  public UpdateFwlogPolicy_1(O_Name, body: IMonitoringFwlogPolicy):Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateFwlogPolicy_1') as Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List StatsPolicy objects */
  public ListStatsPolicy_1(queryParam: any = null):Observable<{body: IMonitoringStatsPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/statsPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'ListStatsPolicy_1') as Observable<{body: IMonitoringStatsPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get StatsPolicy object */
  public GetStatsPolicy_1(O_Name, queryParam: any = null):Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/statsPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetStatsPolicy_1') as Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update StatsPolicy object */
  public UpdateStatsPolicy_1(O_Name, body: IMonitoringStatsPolicy):Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/statsPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateStatsPolicy_1') as Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List MirrorSession objects */
  public ListMirrorSession(queryParam: any = null):Observable<{body: IMonitoringMirrorSessionList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListMirrorSession') as Observable<{body: IMonitoringMirrorSessionList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create MirrorSession object */
  public AddMirrorSession(body: IMonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get MirrorSession object */
  public GetMirrorSession(O_Name, queryParam: any = null):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete MirrorSession object */
  public DeleteMirrorSession(O_Name):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update MirrorSession object */
  public UpdateMirrorSession(O_Name, body: IMonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/MirrorSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateMirrorSession') as Observable<{body: IMonitoringMirrorSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List TroubleshootingSession objects */
  public ListTroubleshootingSession(queryParam: any = null):Observable<{body: IMonitoringTroubleshootingSessionList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSessionList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create TroubleshootingSession object */
  public AddTroubleshootingSession(body: IMonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get TroubleshootingSession object */
  public GetTroubleshootingSession(O_Name, queryParam: any = null):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete TroubleshootingSession object */
  public DeleteTroubleshootingSession(O_Name):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update TroubleshootingSession object */
  public UpdateTroubleshootingSession(O_Name, body: IMonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateTroubleshootingSession') as Observable<{body: IMonitoringTroubleshootingSession | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List AlertDestination objects */
  public ListAlertDestination(queryParam: any = null):Observable<{body: IMonitoringAlertDestinationList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListAlertDestination') as Observable<{body: IMonitoringAlertDestinationList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AlertDestination object */
  public AddAlertDestination(body: IMonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get AlertDestination object */
  public GetAlertDestination(O_Name, queryParam: any = null):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete AlertDestination object */
  public DeleteAlertDestination(O_Name):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AlertDestination object */
  public UpdateAlertDestination(O_Name, body: IMonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertDestinations/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateAlertDestination') as Observable<{body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List AlertPolicy objects */
  public ListAlertPolicy(queryParam: any = null):Observable<{body: IMonitoringAlertPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListAlertPolicy') as Observable<{body: IMonitoringAlertPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create AlertPolicy object */
  public AddAlertPolicy(body: IMonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get AlertPolicy object */
  public GetAlertPolicy(O_Name, queryParam: any = null):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete AlertPolicy object */
  public DeleteAlertPolicy(O_Name):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update AlertPolicy object */
  public UpdateAlertPolicy(O_Name, body: IMonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alertPolicies/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateAlertPolicy') as Observable<{body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List Alert objects */
  public ListAlert(queryParam: any = null):Observable<{body: IMonitoringAlertList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alerts';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListAlert') as Observable<{body: IMonitoringAlertList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get Alert object */
  public GetAlert(O_Name, queryParam: any = null):Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alerts/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetAlert') as Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update Alert object */
  public UpdateAlert(O_Name, body: IMonitoringAlert):Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/alerts/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateAlert') as Observable<{body: IMonitoringAlert | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create EventPolicy object */
  public AddEventPolicy(body: IMonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get EventPolicy object */
  public GetEventPolicy(O_Name, queryParam: any = null):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete EventPolicy object */
  public DeleteEventPolicy(O_Name):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update EventPolicy object */
  public UpdateEventPolicy(O_Name, body: IMonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/event-policy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateEventPolicy') as Observable<{body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FlowExportPolicy objects */
  public ListFlowExportPolicy(queryParam: any = null):Observable<{body: IMonitoringFlowExportPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Create FlowExportPolicy object */
  public AddFlowExportPolicy(body: IMonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXPostCall(url, body, 'AddFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FlowExportPolicy object */
  public GetFlowExportPolicy(O_Name, queryParam: any = null):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Delete FlowExportPolicy object */
  public DeleteFlowExportPolicy(O_Name):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'DeleteFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FlowExportPolicy object */
  public UpdateFlowExportPolicy(O_Name, body: IMonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateFlowExportPolicy') as Observable<{body: IMonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List FwlogPolicy objects */
  public ListFwlogPolicy(queryParam: any = null):Observable<{body: IMonitoringFwlogPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/fwlogPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListFwlogPolicy') as Observable<{body: IMonitoringFwlogPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get FwlogPolicy object */
  public GetFwlogPolicy(O_Name, queryParam: any = null):Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetFwlogPolicy') as Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update FwlogPolicy object */
  public UpdateFwlogPolicy(O_Name, body: IMonitoringFwlogPolicy):Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateFwlogPolicy') as Observable<{body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** List StatsPolicy objects */
  public ListStatsPolicy(queryParam: any = null):Observable<{body: IMonitoringStatsPolicyList | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/statsPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'ListStatsPolicy') as Observable<{body: IMonitoringStatsPolicyList | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Get StatsPolicy object */
  public GetStatsPolicy(O_Name, queryParam: any = null):Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/statsPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, queryParam, 'GetStatsPolicy') as Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Update StatsPolicy object */
  public UpdateStatsPolicy(O_Name, body: IMonitoringStatsPolicy):Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/tenant/{O.Tenant}/statsPolicy/{O.Name}';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body, 'UpdateStatsPolicy') as Observable<{body: IMonitoringStatsPolicy | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch MirrorSession objects */
  public WatchMirrorSession_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/MirrorSession';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchMirrorSession_1') as Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AlertDestination objects */
  public WatchAlertDestination_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/alertDestinations';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAlertDestination_1') as Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AlertPolicy objects */
  public WatchAlertPolicy_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/alertPolicies';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAlertPolicy_1') as Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Alert objects */
  public WatchAlert_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgAlertWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/alerts';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAlert_1') as Observable<{body: IMonitoringAutoMsgAlertWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch EventPolicy objects */
  public WatchEventPolicy_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/event-policy';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchEventPolicy_1') as Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FlowExportPolicy objects */
  public WatchFlowExportPolicy_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/flowExportPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchFlowExportPolicy_1') as Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FwlogPolicy objects */
  public WatchFwlogPolicy_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/fwlogPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchFwlogPolicy_1') as Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch StatsPolicy objects */
  public WatchStatsPolicy_1(queryParam: any = null):Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/statsPolicy';
    return this.invokeAJAXGetCall(url, queryParam, 'WatchStatsPolicy_1') as Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch MirrorSession objects */
  public WatchMirrorSession(queryParam: any = null):Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/MirrorSession';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchMirrorSession') as Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AlertDestination objects */
  public WatchAlertDestination(queryParam: any = null):Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/alertDestinations';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAlertDestination') as Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch AlertPolicy objects */
  public WatchAlertPolicy(queryParam: any = null):Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/alertPolicies';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAlertPolicy') as Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch Alert objects */
  public WatchAlert(queryParam: any = null):Observable<{body: IMonitoringAutoMsgAlertWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/alerts';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchAlert') as Observable<{body: IMonitoringAutoMsgAlertWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch EventPolicy objects */
  public WatchEventPolicy(queryParam: any = null):Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/event-policy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchEventPolicy') as Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FlowExportPolicy objects */
  public WatchFlowExportPolicy(queryParam: any = null):Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/flowExportPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchFlowExportPolicy') as Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch FwlogPolicy objects */
  public WatchFwlogPolicy(queryParam: any = null):Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/fwlogPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchFwlogPolicy') as Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
  /** Watch StatsPolicy objects */
  public WatchStatsPolicy(queryParam: any = null):Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | IApiStatus | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/tenant/{O.Tenant}/statsPolicy';
    url = url.replace('{O.Tenant}', this['O_Tenant']);
    return this.invokeAJAXGetCall(url, queryParam, 'WatchStatsPolicy') as Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | IApiStatus | Error, statusCode: number}>;
  }
  
}