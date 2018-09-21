import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SgpoliciesComponent } from '@app/components/security/sgpolicies/sgpolicies.component';
import { SgpolicydetailComponent } from '@app/components/security/sgpolicies/sgpolicydetail/sgpolicydetail.component';


const routes: Routes = [
  {
    path: '',
    component: SgpoliciesComponent
  },
  {
    path: ':id',
    component: SgpolicydetailComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SgpoliciesRoutingModule { }
