import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AuthpolicyComponent } from '@app/components/settings-group/authpolicy/authpolicy.component';


const routes: Routes = [
  {
    path: '',
    component: AuthpolicyComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AuthPolicyRoutingModule { }
