import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AlertseventspageComponent } from '@app/components/monitoring-group/alertsevents/alertseventspage.component';


const routes: Routes = [
  {
    path: '',
    component: AlertseventspageComponent
  },
  {
    path: 'alertpolicies',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/alertsevents/alertpolicies/alertpolicies.module#AlertPoliciesModule'
      }
    ]
  },
  {
    path: 'eventpolicy',
    children: [
      {
        path: '',
        loadChildren: '@app/components/monitoring-group/alertsevents/eventpolicy/eventpolicy.module#EventPolicyModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AlertsEventsPageRoutingModule { }
