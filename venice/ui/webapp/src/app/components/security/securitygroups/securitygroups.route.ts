import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SecuritygroupsComponent } from '@app/components/security/securitygroups/securitygroups.component';
import { NewsecuritygroupComponent } from '@app/components/security/securitygroups/newsecuritygroup/newsecuritygroup.component';


const routes: Routes = [
  {
    path: '',
    component: SecuritygroupsComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SecuritygroupsRoutingModule { }
