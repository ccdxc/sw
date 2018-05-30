import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SgpolicyComponent } from './sgpolicy/sgpolicy.component';
import { SecurityComponent } from './security.component';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'sgpolicy',
    pathMatch: 'full'
  },
  {
    path: 'sgpolicy',
    component: SecurityComponent,
    children: [
      { path: '', component: SgpolicyComponent }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SecurityRoutingModule { }
