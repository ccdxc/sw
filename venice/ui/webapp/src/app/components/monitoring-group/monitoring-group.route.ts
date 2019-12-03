import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'alertsevents',
    pathMatch: 'full'
  },
  {
    path: 'alertsevents',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertseventspage.module#AlertsEventsPageModule'
      }
    ]
  },
  {
    path: 'fwlogs',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/fwlogs/fwlogs.module#FwlogsModule'
      }
    ]
  },
  {
    path: 'auditevents',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/auditevents/auditevents.module#AuditeventsModule'
      }
    ]
  },
  {
    path: 'metrics',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/telemetry/telemetry.module#TelemetryModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MonitoringRoutingModule { }
