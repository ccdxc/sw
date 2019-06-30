import { NgModule } from '@angular/core';
import { Routes, RouterModule, UrlSegment } from '@angular/router';
import { AlertseventspageComponent } from '@app/components/monitoring-group/alertsevents/alertseventspage.component';

export function destinationOrPolicies(url: UrlSegment[]) {
  return url.length === 1 && (url[0].path === ('alertpolicies') || url[0].path === ('alertdestinations'))  ? ({consumed: url}) : null;
}


const routes: Routes = [
  {
    path: '',
    component: AlertseventspageComponent
  },
  {
    matcher: destinationOrPolicies,
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
