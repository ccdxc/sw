import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AlertseventsComponent } from '@app/components/monitoring-group/alertsevents/alertsevents.component';


const routes: Routes = [
  {
    path: '',
    component: AlertseventsComponent
  },
  {
    path: 'alertpolicies',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertpolicies/alertpolicies.module#AlertPoliciesModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AlertsEventsRoutingModule { }
