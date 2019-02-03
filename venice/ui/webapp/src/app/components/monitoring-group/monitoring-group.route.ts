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
    path: 'telemetry',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/telemetry/telemetry.module#TelemetryModule'
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
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MonitoringRoutingModule { }
