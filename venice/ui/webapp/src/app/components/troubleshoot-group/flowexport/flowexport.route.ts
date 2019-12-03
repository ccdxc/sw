import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { FlowexportComponent } from './flowexport.component';


const routes: Routes = [
  {
    path: '',
    component: FlowexportComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FlowexportRoutingModule { }
