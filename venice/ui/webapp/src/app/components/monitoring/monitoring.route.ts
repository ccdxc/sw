import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { MonitoringComponent } from './monitoring.component';
import { TroubleshootingComponent } from './troubleshooting/troubleshooting.component';
import { AlertseventsComponent } from './alertsevents/alertsevents.component'

const routes: Routes = [
  {
    path: '',
    redirectTo: 'troubleshooting',
    pathMatch: 'full'
  },
  {
    path: 'troubleshooting',
    component: MonitoringComponent,
    children: [
      { path: '', component: TroubleshootingComponent }
    ]
  },
  {
    path: 'alertsevents',
    component: MonitoringComponent,
    children: [
      { path: '', component: AlertseventsComponent },
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MonitoringRoutingModule { }
