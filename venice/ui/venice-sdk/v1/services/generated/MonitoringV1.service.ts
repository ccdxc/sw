import { AbstractService } from './abstract.service';
import { HttpClient } from '../../../../webapp/node_modules/@angular/common/http';
import { Observable } from '../../../../webapp/node_modules/rxjs/Observable';
import { Injectable } from '../../../../webapp/node_modules/@angular/core';

import { IMonitoringMirrorSessionList,IMonitoringMirrorSession,MonitoringMirrorSession,IMonitoringTroubleshootingSessionList,IMonitoringTroubleshootingSession,MonitoringTroubleshootingSession,IMonitoringAlertDestinationList,IMonitoringAlertDestination,MonitoringAlertDestination,IMonitoringAlertPolicyList,IMonitoringAlertPolicy,MonitoringAlertPolicy,IMonitoringAlertList,IMonitoringAlert,MonitoringAlert,IMonitoringEventPolicy,MonitoringEventPolicy,IMonitoringFlowExportPolicyList,IMonitoringFlowExportPolicy,MonitoringFlowExportPolicy,IMonitoringFwlogPolicyList,IMonitoringFwlogPolicy,MonitoringFwlogPolicy,IMonitoringStatsPolicyList,IMonitoringStatsPolicy,MonitoringStatsPolicy,IMonitoringAutoMsgMirrorSessionWatchHelper,IMonitoringAutoMsgAlertDestinationWatchHelper,IMonitoringAutoMsgAlertPolicyWatchHelper,IMonitoringAutoMsgAlertWatchHelper,IMonitoringAutoMsgEventPolicyWatchHelper,IMonitoringAutoMsgFlowExportPolicyWatchHelper,IMonitoringAutoMsgFwlogPolicyWatchHelper,IMonitoringAutoMsgStatsPolicyWatchHelper } from '../../models/generated/monitoring';

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

  /** List MirrorSession objects */
  public AutoListMirrorSession-1():Observable<{body: IMonitoringMirrorSessionList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession';
    return this.invokeAJAXGetCall(url, 'AutoListMirrorSession-1') as Observable<{body: IMonitoringMirrorSessionList | Error, statusCode: number}>;
  }
  
  /** Create MirrorSession object */
  public AutoAddMirrorSession-1(body: MonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddMirrorSession-1') as Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}>;
  }
  
  /** Get MirrorSession object */
  public AutoGetMirrorSession-1(O_Name):Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetMirrorSession-1') as Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}>;
  }
  
  /** Delete MirrorSession object */
  public AutoDeleteMirrorSession-1(O_Name):Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteMirrorSession-1') as Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}>;
  }
  
  /** Update MirrorSession object */
  public AutoUpdateMirrorSession-1(O_Name, body: MonitoringMirrorSession):Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/MirrorSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateMirrorSession-1') as Observable<{body: IMonitoringMirrorSession | Error, statusCode: number}>;
  }
  
  /** List TroubleshootingSession objects */
  public AutoListTroubleshootingSession-1():Observable<{body: IMonitoringTroubleshootingSessionList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession';
    return this.invokeAJAXGetCall(url, 'AutoListTroubleshootingSession-1') as Observable<{body: IMonitoringTroubleshootingSessionList | Error, statusCode: number}>;
  }
  
  /** Create TroubleshootingSession object */
  public AutoAddTroubleshootingSession-1(body: MonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddTroubleshootingSession-1') as Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}>;
  }
  
  /** Get TroubleshootingSession object */
  public AutoGetTroubleshootingSession-1(O_Name):Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetTroubleshootingSession-1') as Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}>;
  }
  
  /** Delete TroubleshootingSession object */
  public AutoDeleteTroubleshootingSession-1(O_Name):Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteTroubleshootingSession-1') as Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}>;
  }
  
  /** Update TroubleshootingSession object */
  public AutoUpdateTroubleshootingSession-1(O_Name, body: MonitoringTroubleshootingSession):Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/TroubleshootingSession/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateTroubleshootingSession-1') as Observable<{body: IMonitoringTroubleshootingSession | Error, statusCode: number}>;
  }
  
  /** List AlertDestination objects */
  public AutoListAlertDestination-1():Observable<{body: IMonitoringAlertDestinationList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations';
    return this.invokeAJAXGetCall(url, 'AutoListAlertDestination-1') as Observable<{body: IMonitoringAlertDestinationList | Error, statusCode: number}>;
  }
  
  /** Create AlertDestination object */
  public AutoAddAlertDestination-1(body: MonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddAlertDestination-1') as Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}>;
  }
  
  /** Get AlertDestination object */
  public AutoGetAlertDestination-1(O_Name):Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetAlertDestination-1') as Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}>;
  }
  
  /** Delete AlertDestination object */
  public AutoDeleteAlertDestination-1(O_Name):Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteAlertDestination-1') as Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}>;
  }
  
  /** Update AlertDestination object */
  public AutoUpdateAlertDestination-1(O_Name, body: MonitoringAlertDestination):Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertDestinations/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateAlertDestination-1') as Observable<{body: IMonitoringAlertDestination | Error, statusCode: number}>;
  }
  
  /** List AlertPolicy objects */
  public AutoListAlertPolicy-1():Observable<{body: IMonitoringAlertPolicyList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies';
    return this.invokeAJAXGetCall(url, 'AutoListAlertPolicy-1') as Observable<{body: IMonitoringAlertPolicyList | Error, statusCode: number}>;
  }
  
  /** Create AlertPolicy object */
  public AutoAddAlertPolicy-1(body: MonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddAlertPolicy-1') as Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}>;
  }
  
  /** Get AlertPolicy object */
  public AutoGetAlertPolicy-1(O_Name):Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetAlertPolicy-1') as Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}>;
  }
  
  /** Delete AlertPolicy object */
  public AutoDeleteAlertPolicy-1(O_Name):Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteAlertPolicy-1') as Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}>;
  }
  
  /** Update AlertPolicy object */
  public AutoUpdateAlertPolicy-1(O_Name, body: MonitoringAlertPolicy):Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alertPolicies/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateAlertPolicy-1') as Observable<{body: IMonitoringAlertPolicy | Error, statusCode: number}>;
  }
  
  /** List Alert objects */
  public AutoListAlert-1():Observable<{body: IMonitoringAlertList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alerts';
    return this.invokeAJAXGetCall(url, 'AutoListAlert-1') as Observable<{body: IMonitoringAlertList | Error, statusCode: number}>;
  }
  
  /** Get Alert object */
  public AutoGetAlert-1(O_Name):Observable<{body: IMonitoringAlert | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alerts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetAlert-1') as Observable<{body: IMonitoringAlert | Error, statusCode: number}>;
  }
  
  /** Update Alert object */
  public AutoUpdateAlert-1(O_Name, body: MonitoringAlert):Observable<{body: IMonitoringAlert | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/alerts/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateAlert-1') as Observable<{body: IMonitoringAlert | Error, statusCode: number}>;
  }
  
  /** Create EventPolicy object */
  public AutoAddEventPolicy-1(body: MonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddEventPolicy-1') as Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}>;
  }
  
  /** Get EventPolicy object */
  public AutoGetEventPolicy-1(O_Name):Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetEventPolicy-1') as Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}>;
  }
  
  /** Delete EventPolicy object */
  public AutoDeleteEventPolicy-1(O_Name):Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteEventPolicy-1') as Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}>;
  }
  
  /** Update EventPolicy object */
  public AutoUpdateEventPolicy-1(O_Name, body: MonitoringEventPolicy):Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/event-policy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateEventPolicy-1') as Observable<{body: IMonitoringEventPolicy | Error, statusCode: number}>;
  }
  
  /** List FlowExportPolicy objects */
  public AutoListFlowExportPolicy-1():Observable<{body: IMonitoringFlowExportPolicyList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy';
    return this.invokeAJAXGetCall(url, 'AutoListFlowExportPolicy-1') as Observable<{body: IMonitoringFlowExportPolicyList | Error, statusCode: number}>;
  }
  
  /** Create FlowExportPolicy object */
  public AutoAddFlowExportPolicy-1(body: MonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy';
    return this.invokeAJAXPostCall(url, body.getValues(), 'AutoAddFlowExportPolicy-1') as Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}>;
  }
  
  /** Get FlowExportPolicy object */
  public AutoGetFlowExportPolicy-1(O_Name):Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetFlowExportPolicy-1') as Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}>;
  }
  
  /** Delete FlowExportPolicy object */
  public AutoDeleteFlowExportPolicy-1(O_Name):Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXDeleteCall(url, 'AutoDeleteFlowExportPolicy-1') as Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}>;
  }
  
  /** Update FlowExportPolicy object */
  public AutoUpdateFlowExportPolicy-1(O_Name, body: MonitoringFlowExportPolicy):Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/flowExportPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateFlowExportPolicy-1') as Observable<{body: IMonitoringFlowExportPolicy | Error, statusCode: number}>;
  }
  
  /** List FwlogPolicy objects */
  public AutoListFwlogPolicy-1():Observable<{body: IMonitoringFwlogPolicyList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/fwlogPolicy';
    return this.invokeAJAXGetCall(url, 'AutoListFwlogPolicy-1') as Observable<{body: IMonitoringFwlogPolicyList | Error, statusCode: number}>;
  }
  
  /** Get FwlogPolicy object */
  public AutoGetFwlogPolicy-1(O_Name):Observable<{body: IMonitoringFwlogPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetFwlogPolicy-1') as Observable<{body: IMonitoringFwlogPolicy | Error, statusCode: number}>;
  }
  
  /** Update FwlogPolicy object */
  public AutoUpdateFwlogPolicy-1(O_Name, body: MonitoringFwlogPolicy):Observable<{body: IMonitoringFwlogPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/fwlogPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateFwlogPolicy-1') as Observable<{body: IMonitoringFwlogPolicy | Error, statusCode: number}>;
  }
  
  /** List StatsPolicy objects */
  public AutoListStatsPolicy-1():Observable<{body: IMonitoringStatsPolicyList | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/statsPolicy';
    return this.invokeAJAXGetCall(url, 'AutoListStatsPolicy-1') as Observable<{body: IMonitoringStatsPolicyList | Error, statusCode: number}>;
  }
  
  /** Get StatsPolicy object */
  public AutoGetStatsPolicy-1(O_Name):Observable<{body: IMonitoringStatsPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/statsPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXGetCall(url, 'AutoGetStatsPolicy-1') as Observable<{body: IMonitoringStatsPolicy | Error, statusCode: number}>;
  }
  
  /** Update StatsPolicy object */
  public AutoUpdateStatsPolicy-1(O_Name, body: MonitoringStatsPolicy):Observable<{body: IMonitoringStatsPolicy | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/statsPolicy/{O.Name}';
    url = url.replace('{O.Name}', O_Name);
    return this.invokeAJAXPutCall(url, body.getValues(), 'AutoUpdateStatsPolicy-1') as Observable<{body: IMonitoringStatsPolicy | Error, statusCode: number}>;
  }
  
  /** Watch MirrorSession objects */
  public AutoWatchMirrorSession-1():Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/MirrorSession';
    return this.invokeAJAXGetCall(url, 'AutoWatchMirrorSession-1') as Observable<{body: IMonitoringAutoMsgMirrorSessionWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch AlertDestination objects */
  public AutoWatchAlertDestination-1():Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/alertDestinations';
    return this.invokeAJAXGetCall(url, 'AutoWatchAlertDestination-1') as Observable<{body: IMonitoringAutoMsgAlertDestinationWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch AlertPolicy objects */
  public AutoWatchAlertPolicy-1():Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/alertPolicies';
    return this.invokeAJAXGetCall(url, 'AutoWatchAlertPolicy-1') as Observable<{body: IMonitoringAutoMsgAlertPolicyWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch Alert objects */
  public AutoWatchAlert-1():Observable<{body: IMonitoringAutoMsgAlertWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/alerts';
    return this.invokeAJAXGetCall(url, 'AutoWatchAlert-1') as Observable<{body: IMonitoringAutoMsgAlertWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch EventPolicy objects */
  public AutoWatchEventPolicy-1():Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/event-policy';
    return this.invokeAJAXGetCall(url, 'AutoWatchEventPolicy-1') as Observable<{body: IMonitoringAutoMsgEventPolicyWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch FlowExportPolicy objects */
  public AutoWatchFlowExportPolicy-1():Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/flowExportPolicy';
    return this.invokeAJAXGetCall(url, 'AutoWatchFlowExportPolicy-1') as Observable<{body: IMonitoringAutoMsgFlowExportPolicyWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch FwlogPolicy objects */
  public AutoWatchFwlogPolicy-1():Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/fwlogPolicy';
    return this.invokeAJAXGetCall(url, 'AutoWatchFwlogPolicy-1') as Observable<{body: IMonitoringAutoMsgFwlogPolicyWatchHelper | Error, statusCode: number}>;
  }
  
  /** Watch StatsPolicy objects */
  public AutoWatchStatsPolicy-1():Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | Error, statusCode: number}> {
    let url = this['baseUrlAndPort'] + '/configs/monitoring/v1/watch/statsPolicy';
    return this.invokeAJAXGetCall(url, 'AutoWatchStatsPolicy-1') as Observable<{body: IMonitoringAutoMsgStatsPolicyWatchHelper | Error, statusCode: number}>;
  }
  
}