import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { MonitoringGroupComponent } from './monitoring-group.component';
import { TroubleshootingComponent } from './troubleshooting/troubleshooting.component';
import { AlertseventsComponent } from './alertsevents/alertsevents.component';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'troubleshooting',
    pathMatch: 'full'
  },
  {
    path: 'troubleshooting',
    component: MonitoringGroupComponent,
    children: [
      { path: '', component: TroubleshootingComponent }
    ]
  },
  // {
  //   path: 'alertsevents',
  //   component: MonitoringGroupComponent,
  //   children: [
  //     { path: '', component: AlertseventsComponent },
  //   ]
  // },
  {
    path: 'alertsevents',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertsevents.module#AlertsEventsModule'
      }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MonitoringRoutingModule { }
