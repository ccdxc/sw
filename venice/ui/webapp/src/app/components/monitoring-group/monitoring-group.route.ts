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
      },
      {
        path: 'alert/:alertname',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertseventspage.module#AlertsEventsPageModule'
      },
      {
        path: 'event/:eventname',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertseventspage.module#AlertsEventsPageModule'
      },
      {
        path: 'alert',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertseventspage.module#AlertsEventsPageModule'
      },
      {
        path: 'event',
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
    path: 'archive',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/archivelogs/archivelog.module#ArchivelogModule'
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
