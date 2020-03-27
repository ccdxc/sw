import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { FirewallprofilesComponent } from '@app/components/security/firewallprofiles/firewallprofiles.component';

const routes: Routes = [
  {
    path: '',
    component: FirewallprofilesComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FirewallprofilesRoutingModule { }
