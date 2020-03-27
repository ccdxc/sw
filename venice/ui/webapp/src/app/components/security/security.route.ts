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
  },
  {
    path: 'securityapps',
    children: [
      {
        path: '',
        loadChildren: '@app/components/security/securityapps/securityapps.module#SecurityappsModule'
      }
    ]
  },
  {
    path: 'securitygroups',
    children: [
      {
        path: '',
        loadChildren: '@app/components/security/securitygroups/securitygroups.module#SecuritygroupsModule'
      }
    ]
  },
  {
    path: 'firewallprofiles',
    children: [
      {
        path: '',
        loadChildren: '@app/components/security/firewallprofiles/firewallprofiles.module#FirewallprofilesModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SecurityRoutingModule { }
