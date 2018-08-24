import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { MonitoringGroupComponent } from './monitoring-group.component';
import { TroubleshootingComponent } from './troubleshooting/troubleshooting.component';

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
  {
    path: 'alertsevents',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertseventspage.module#AlertsEventsPageModule'
      }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MonitoringRoutingModule { }
