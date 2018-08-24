import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AlertpoliciesComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/alertpolicies.component';


const routes: Routes = [
  {
    path: '',
    component: AlertpoliciesComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AlertPoliciesRoutingModule { }
