import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'sgpolicies',
    pathMatch: 'full'
  },
  {
    path: 'sgpolicies',
    children: [
      {
        path: '',
        loadChildren: '@app/components/security/sgpolicies/sgpolicies.module#SgpoliciesModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SecurityRoutingModule { }
