import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SecurityappsComponent } from '@app/components/security/securityapps/securityapps.component';

const routes: Routes = [
  {
    path: '',
    component: SecurityappsComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SecurityappsRoutingModule { }
