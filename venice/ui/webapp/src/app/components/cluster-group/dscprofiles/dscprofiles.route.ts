import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DscprofilesComponent } from './dscprofiles.component';

const routes: Routes = [
  {
    path: '',
    component: DscprofilesComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DSCProfileRoutingModule { }
