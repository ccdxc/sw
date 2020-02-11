import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ApiSampleComponent } from './api-sample.component';

const routes: Routes = [
  {
    path: '',
    component: ApiSampleComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ApiSampleRoutingModule { }
