import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { FwlogpoliciesComponent } from './fwlogpolicies.component';


const routes: Routes = [
  {
    path: '',
    component: FwlogpoliciesComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FwlogpoliciesRoutingModule { }
